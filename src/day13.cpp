// Copyright (C) 2018 David Holmes <dholmes@dholmes.us>. All rights reserved.

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

using std::array;
using std::optional;
using std::pair;
using std::string;
using std::uint16_t;
using std::uint8_t;
using std::vector;

const auto TrackSize = 50;
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
using Cart = pair<Coordinate, Direction>;
using MaybeCart = optional<Cart>;
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

// Stream insertion operator for Cart.
std::ostream & operator<<(std::ostream & stream, const Cart & cart)
{
    auto [coord, dir] = cart;
    stream << coord << ',' << dir;
    return stream;
}

InputChars ReadFile(std::istream & stream)
{
    InputChars out;
    for (auto & row : out) {
        if (!stream.getline(row.data(), row.size())) {
            break;
        }
    }
    return out;
}

pair<TrackTile, optional<Cart>> ProcessInputTile(const InputChars & input,
                                                 uint8_t x, uint8_t y)
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
    vector<Cart> carts;
    for (auto y = 0; y < TrackHeight; y++) {
        for (auto x = 0; x < TrackWidth; x++) {
            auto [tile, maybeCart] = ProcessInputTile(input, x, y);
            track[y][x] = tile;
            if (maybeCart) {
                carts.push_back(*maybeCart);
            }
        }
    }
    return {track, carts};
}

pair<Track, vector<Cart>> ReadInput(std::istream & stream)
{
    return ProcessInput(ReadFile(stream));
}

void PrintTrack(std::ostream & stream, const Track & track)
{
    for (auto & row : track) {
        stream << row << '\n';
    }
}

class MoveCursor
{
    friend std::ostream & operator<<(std::ostream & stream,
                                     const MoveCursor & moveCursor);

public:
    MoveCursor(Direction direction, uint16_t spaces)
        : direction(direction), spaces(spaces)
    {}

private:
    Direction direction;
    uint16_t spaces;
};

std::ostream & operator<<(std::ostream & stream, const MoveCursor & moveCursor)
{
    char dirChar = 'A';
    if (moveCursor.direction == North) {
        dirChar = 'A';
    } else if (moveCursor.direction == South) {
        dirChar = 'B';
    } else if (moveCursor.direction == East) {
        dirChar = 'C';
    } else if (moveCursor.direction == West) {
        dirChar = 'D';
    }
    stream << "\033[" << moveCursor.spaces << dirChar;
    return stream;
}

void DrawCart(std::ostream & stream, const Cart & cart)
{
    auto [coord, dir] = cart;
    auto [x, y] = coord;
    if (y > 0) {
        stream << MoveCursor(South, y);
    }
    if (x > 0) {
        stream << MoveCursor(East, x);
    }
    stream << dir << MoveCursor(West, x + 1);
    if (y > 0) {
        stream << MoveCursor(North, y);
    }
}

void DrawCarts(std::ostream & stream, const vector<Cart> & carts)
{
    for (auto & cart : carts) {
        DrawCart(stream, cart);
    }
    stream.flush();
}

int main(int /*argc*/, char ** /*argv*/)
{
    auto [track, carts] = ReadInput(std::cin);

    for (Cart cart : carts) {
        std::cout << cart << '\n';
    }

    PrintTrack(std::cout, track);
    std::cout << MoveCursor(North, TrackSize);

    DrawCarts(std::cout, carts);

    std::cout << MoveCursor(South, TrackSize);

    return 0;
}
