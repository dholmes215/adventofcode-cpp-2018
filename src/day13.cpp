// Copyright (C) 2018 David Holmes <dholmes@dholmes.us>. All rights reserved.

#include "ansiterm.hpp"

#include <algorithm>
#include <array>
#include <bitset>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <optional>
#include <string>
#include <thread>
#include <utility>
#include <variant>
#include <vector>

using namespace std::chrono_literals;

using ansi::cursor;
using ansi::graphic;

using std::array;
using std::optional;
using std::pair;
using std::string;
using std::uint16_t;
using std::uint8_t;
using std::vector;

const auto TrackSize = 150;
const auto TrackWidth = TrackSize;
const auto TrackHeight = TrackSize;

enum Direction : std::uint8_t
{
    North = 1,
    East = 2,
    South = 4,
    West = 8,
};

using Directions = uint8_t;
using Coordinate = pair<uint8_t, uint8_t>;
using TrackTile = std::bitset<4>;
using TrackRow = array<TrackTile, TrackWidth>;
using Track = array<TrackRow, TrackHeight>;
using CartId = uint8_t;
using MaybeCart = optional<pair<Coordinate, Direction>>;
using ErrorString = std::string;

using InputChars = array<array<char, TrackWidth>, TrackHeight>;

char TrackTileToChar(const TrackTile tile)
{
    if (tile == TrackTile()) {
        return ' ';
    } else if (tile == TrackTile(North | South)) {
        return '|';
    } else if (tile == TrackTile(East | West)) {
        return '-';
    } else if (tile == TrackTile(North | West)) {
        return '/';
    } else if (tile == TrackTile(South | East)) {
        return '/';
    } else if (tile == TrackTile(North | East)) {
        return '\\';
    } else if (tile == TrackTile(South | West)) {
        return '\\';
    } else if (tile == TrackTile(North | South | East | West)) {
        return '+';
    }
    return 'X';
}

// Stream insertion operator for Coordinate.
std::ostream & operator<<(std::ostream & stream, const Coordinate & coord)
{
    stream << static_cast<int>(coord.first) << ','
           << static_cast<int>(coord.second);
    return stream;
}

// Stream insertion operator for TrackTile.
std::ostream & operator<<(std::ostream & stream, const TrackTile & tile)
{
    stream << TrackTileToChar(tile);
    return stream;
}

// Stream insertion operator for TrackRow.
std::ostream & operator<<(std::ostream & stream, const TrackRow & row)
{
    for (auto tile : row) {
        stream << tile;
    }
    return stream;
}

char DirectionToChar(Direction direction)
{
    switch (direction) {
    case North:
        return '^';
    case South:
        return 'v';
    case East:
        return '>';
    case West:
        return '<';
    }

    assert(false);
}

// Stream insertion operator for Direction.
std::ostream & operator<<(std::ostream & stream, const Direction & direction)
{
    stream << DirectionToChar(direction);
    return stream;
}

class Cart
{

public:
    Cart(CartId id, Coordinate coordinates, Direction direction)
        : id(id), coordinates(coordinates), direction(direction),
          nextDecision(TurnDecision::Left)
    {}

    CartId GetId() const { return id; }
    Coordinate GetCoordinates() const { return coordinates; }
    Direction GetDirection() const { return direction; }

    void Move()
    {
        auto & [x, y] = coordinates;
        if (direction == Direction::North) {
            y--;
        } else if (direction == Direction::South) {
            y++;
        } else if (direction == Direction::West) {
            x--;
        } else if (direction == Direction::East) {
            x++;
        }
    }

    void Turn(TrackTile tile)
    {
        if (tile == TrackTile(North | South) ||
            tile == TrackTile(East | West)) {
            return;
        } else if (tile == TrackTile(North | East)) {
            if (direction == South) {
                direction = East;
            } else if (direction == West) {
                direction = North;
            }
            // TODO bad
        } else if (tile == TrackTile(South | West)) {
            if (direction == North) {
                direction = West;
            } else if (direction == East) {
                direction = South;
            }
            // TODO bad
        } else if (tile == TrackTile(North | West)) {
            if (direction == South) {
                direction = West;
            } else if (direction == East) {
                direction = North;
            }
            // TODO bad
        } else if (tile == TrackTile(South | East)) {
            if (direction == North) {
                direction = East;
            } else if (direction == West) {
                direction = South;
            }
            // TODO bad
        } else if (tile == TrackTile(North | South | West | East)) {
            direction = NewDirection(direction, nextDecision);
            nextDecision = NextTurnDecision(nextDecision);
        }
    }

private:
    enum class TurnDecision : uint8_t
    {
        Left = 0,
        Straight = 1,
        Right = 2,
    };

    static TurnDecision NextTurnDecision(TurnDecision oldDecision)
    {
        return static_cast<TurnDecision>(
            (static_cast<uint8_t>(oldDecision) + 1) % 3);
    }

    static Direction NewDirection(Direction oldDirection,
                                  TurnDecision turnDecision)
    {
        if (turnDecision == TurnDecision::Straight) {
            return oldDirection;
        } else if (turnDecision == TurnDecision::Left) {
            if (oldDirection == North) {
                return West;
            } else if (oldDirection == West) {
                return South;
            } else if (oldDirection == South) {
                return East;
            } else if (oldDirection == East) {
                return North;
            }
        } else if (turnDecision == TurnDecision::Right) {
            if (oldDirection == North) {
                return East;
            } else if (oldDirection == East) {
                return South;
            } else if (oldDirection == South) {
                return West;
            } else if (oldDirection == West) {
                return North;
            }
        }
        assert(false);
    }

    CartId id;
    Coordinate coordinates;
    Direction direction;
    TurnDecision nextDecision;
};

// Stream insertion operator for Cart.
std::ostream & operator<<(std::ostream & stream, const Cart & cart)
{
    stream << cart.GetCoordinates() << ',' << cart.GetDirection();
    return stream;
}

InputChars ReadFile(std::istream & stream)
{
    InputChars out;
    string line;
    for (auto & row : out) {
        std::fill(row.begin(), row.end(), ' ');
        std::getline(stream, line);
        std::copy(line.begin(), line.end(), row.begin());
    }
    return out;
}

pair<TrackTile, MaybeCart> ProcessInputTile(const InputChars & input, uint8_t x,
                                            uint8_t y)
{
    Coordinate coordinate = {x, y};
    char tileChar = input[y][x];
    if (tileChar == ' ' || tileChar == '\0') {
        return {};
    } else if (tileChar == '-') {
        return {TrackTile(East | West), {}};
    } else if (tileChar == '|') {
        return {TrackTile(North | South), {}};
    } else if (tileChar == '+') {
        return {TrackTile(North | South | East | West), {}};
    } else if (tileChar == '^') {
        return {TrackTile(North | South), MaybeCart({coordinate, North})};
    } else if (tileChar == 'v') {
        return {TrackTile(North | South), MaybeCart({coordinate, South})};
    } else if (tileChar == '<') {
        return {TrackTile(West | East), MaybeCart({coordinate, West})};
    } else if (tileChar == '>') {
        return {TrackTile(West | East), MaybeCart({coordinate, East})};
    }

    // To determine the track tile of an input character, in the case of '/' and
    // '\', we also need to consider the eight surrounding tiles.
    char nc = (y - 1) < 0 ? ' ' : input[y - 1][x];
    char sc = (y + 1) >= TrackHeight ? ' ' : input[y + 1][x];
    char wc = (x - 1) < 0 ? ' ' : input[y][x - 1];
    char ec = (x + 1) >= TrackWidth ? ' ' : input[y][x + 1];

    bool n = (nc == '|') | (nc == '+') | (nc == '^') | (nc == 'v');
    bool s = (sc == '|') | (sc == '+') | (sc == '^') | (sc == 'v');
    bool w = (wc == '-') | (wc == '+') | (wc == '<') | (wc == '>');
    bool e = (ec == '-') | (ec == '+') | (ec == '<') | (ec == '>');

    if (tileChar == '/') {
        if (n && w && !s && !e) {
            return {TrackTile(North | West), {}};
        } else if (s && e && !n && !w) {
            return {TrackTile(South | East), {}};
        } else {
            std::cerr << "Invalid input at " << coordinate
                      << ": '/' must be connected to either north and west, or "
                         "south and east."
                      << '\n';
            std::exit(-1);
        }
    } else if (tileChar == '\\') {
        if (n && e && !s && !w) {
            return {TrackTile(North | East), {}};
        } else if (s && w && !n && !e) {
            return {TrackTile(South | West), {}};
        } else {
            std::cerr << "Invalid input at " << coordinate
                      << ": '\\' must be connected to either north and east, "
                         "or south and west."
                      << '\n';
            std::exit(-1);
        }
    }

    std::cerr << "Unrecognized character '" << tileChar << "'\n";
    std::exit(-1);
}

pair<Track, vector<Cart>> ProcessInput(const InputChars & input)
{
    Track track;
    CartId cartId = 0;
    vector<Cart> carts;
    for (auto y = 0; y < TrackHeight; y++) {
        for (auto x = 0; x < TrackWidth; x++) {
            auto [tile, maybeCart] = ProcessInputTile(input, x, y);
            track[y][x] = tile;
            if (maybeCart) {
                auto [coord, dir] = *maybeCart;
                carts.emplace_back(cartId++, coord, dir);
            }
        }
    }
    return {track, carts};
}

pair<Track, vector<Cart>> ReadInput(std::istream & stream)
{
    return ProcessInput(ReadFile(stream));
}

struct View
{
    View() = default;
    bool Contains(const Coordinate & coord) const
    {
        auto [x1, y1] = coord;
        return (x1 >= x) && (x1 < (x + width)) && (y1 >= y) &&
               (y1 < (y + height));
    }
    int16_t x;
    int16_t y;
    int16_t width;
    int16_t height;
};

void PrintTrack(std::ostream & stream, const View & view, const Track & track)
{
    int16_t trackSize = track.size();
    int16_t rowSize = track[0].size();
    for (int16_t y = view.y; y < view.y + view.height; y++) {
        if (y < 0 || y >= trackSize) {
            stream << '\n';
        } else {
            auto & row = track[y];
            for (int16_t x = view.x; x < view.x + view.width; x++) {
                if (x < 0 || x >= rowSize) {
                    stream << ' ';
                } else {
                    stream << row[x];
                }
            }
        }
        stream << '\n';
    }
}

void DrawTrack(std::ostream & stream, const View & view, const Track & track)
{
    PrintTrack(stream, view, track);
    stream << cursor(cursor::direction::up, TrackSize);
}

graphic::color3 CartIdToColor(CartId id)
{
    if (id < 7) {
        return static_cast<graphic::color3>(id + 31);
    }
    return graphic::color3::white;
}

void DrawCart(std::ostream & stream, const View & view, const Cart & cart)
{
    if (view.Contains(cart.GetCoordinates())) {
        auto [x, y] = cart.GetCoordinates();
        if (y > 0) {
            stream << cursor(cursor::direction::down, y);
        }
        if (x > 0) {
            stream << cursor(cursor::direction::right, x);
        }
        stream << graphic::fg_color(CartIdToColor(cart.GetId()));
        stream << cart.GetDirection();
        stream << graphic::reset();
        stream << cursor(cursor::direction::left, x + 1);
        if (y > 0) {
            stream << cursor(cursor::direction::up, y);
        }
    }
}

void DrawCarts(std::ostream & stream, const View & view,
               const vector<Cart> & carts)
{
    for (auto & cart : carts) {
        DrawCart(stream, view, cart);
    }
    stream.flush();
}

void SortCarts(vector<Cart> & carts)
{
    std::sort(carts.begin(), carts.end(), [](auto & cart1, auto & cart2) {
        auto [x1, y1] = cart1.GetCoordinates();
        auto [x2, y2] = cart2.GetCoordinates();
        return std::tie(y1, x1) < std::tie(y2, x2);
    });
}

void MoveCarts(vector<Cart> & carts)
{
    for (auto & cart : carts) {
        cart.Move();
    }
}

void TurnCarts(const Track & track, vector<Cart> & carts)
{
    for (auto & cart : carts) {
        auto [x, y] = cart.GetCoordinates();
        cart.Turn(track[y][x]);
    }
}

int main(int /*argc*/, char ** /*argv*/)
{
    auto [track, carts] = ReadInput(std::cin);

    for (Cart & cart : carts) {
        std::cout << cart << '\n';
    }

    const View view = {0, 0, 150, 48};

    DrawTrack(std::cout, view, track);
    DrawCarts(std::cout, view, carts);

    bool crashed = false;
    while (!crashed) {
        SortCarts(carts);
        MoveCarts(carts);

        DrawTrack(std::cout, view, track);
        DrawCarts(std::cout, view, carts);
        std::this_thread::sleep_for(10ms);

        TurnCarts(track, carts);

        DrawTrack(std::cout, view, track);
        DrawCarts(std::cout, view, carts);
        std::this_thread::sleep_for(10ms);
    }

    std::cout << cursor(cursor::direction::down, TrackSize);

    return 0;
}
