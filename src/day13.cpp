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
using CartId = uint8_t;
using Cart = pair<Coordinate, Direction>;
using CartWithId = pair<CartId, Cart>;
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

pair<Track, vector<CartWithId>> ProcessInput(const InputChars & input)
{
    Track track;
    CartId cartId = 0;
    vector<CartWithId> carts;
    for (auto y = 0; y < TrackHeight; y++) {
        for (auto x = 0; x < TrackWidth; x++) {
            auto [tile, maybeCart] = ProcessInputTile(input, x, y);
            track[y][x] = tile;
            if (maybeCart) {
                carts.emplace_back(cartId++, *maybeCart);
            }
        }
    }
    return {track, carts};
}

pair<Track, vector<CartWithId>> ReadInput(std::istream & stream)
{
    return ProcessInput(ReadFile(stream));
}

void PrintTrack(std::ostream & stream, const Track & track)
{
    for (auto & row : track) {
        stream << row << '\n';
    }
}

ansi::graphic::color3 CartIdToColor(CartId id)
{
    if (id < 7) {
        return static_cast<ansi::graphic::color3>(id + 31);
    }
    return ansi::graphic::color3::white;
}

void DrawCart(std::ostream & stream, const CartWithId & cartWithId)
{
    using ansi::cursor;
    using ansi::graphic;
    auto [id, cart] = cartWithId;
    auto [coord, dir] = cart;
    auto [x, y] = coord;
    if (y > 0) {
        stream << cursor(cursor::direction::down, y);
    }
    if (x > 0) {
        stream << cursor(cursor::direction::right, x);
    }
    stream << graphic::fg_color(CartIdToColor(id));
    stream << dir;
    stream << graphic::reset();
    stream << cursor(cursor::direction::left, x + 1);
    if (y > 0) {
        stream << cursor(cursor::direction::up, y);
    }
}

void DrawCarts(std::ostream & stream, const vector<CartWithId> & cartsWithIds)
{
    for (auto & cartWithId : cartsWithIds) {
        DrawCart(stream, cartWithId);
    }
    stream.flush();
}

void SortCarts(vector<CartWithId> & cartsWithIds)
{
    std::sort(cartsWithIds.begin(), cartsWithIds.end(),
              [](auto & cart1, auto & cart2) {
                  auto [x1, y1] = cart1.second;
                  auto [x2, y2] = cart2.second;
                  return std::tie(y1, x1) < std::tie(y2, x2);
              });
}

using Crashed = bool;
Crashed SortAndMoveCarts(const Track & track, vector<CartWithId> & cartsWithIds)
{
    SortCarts(cartsWithIds);

    for (auto & cartWithId : cartsWithIds) {
        auto [id, cart] = cartWithId;
        auto [coord, direction] = cart;
    }

    return false;
}

int main(int /*argc*/, char ** /*argv*/)
{
    using ansi::cursor;
    auto [track, cartsWithIds] = ReadInput(std::cin);

    for (CartWithId cartWithId : cartsWithIds) {
        auto [id, cart] = cartWithId;
        std::cout << cart << '\n';
    }

    PrintTrack(std::cout, track);
    std::cout << cursor(cursor::direction::up, TrackSize);

    DrawCarts(std::cout, cartsWithIds);

    bool crashed = false;
    while (!crashed) {
        SortAndMoveCarts(cartsWithIds);
        DrawCarts(std::cout, cartsWithIds);
    }

    std::cout << cursor(cursor::direction::down, TrackSize);

    return 0;
}
