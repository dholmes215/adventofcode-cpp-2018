// Copyright (C) 2018 David Holmes <dholmes@dholmes.us>. All rights reserved.

#include <array>
#include <bitset>
#include <cstdint>
#include <iostream>
#include <set>
#include <string>
#include <utility>

using std::int32_t;
using std::uint32_t;

using std::array;
using std::bitset;
using std::pair;
using std::set;
using std::string;

const auto MapSize = 32;

using MapRow = bitset<MapSize>;
using Map = array<MapRow, MapSize>;
using Column = int32_t;
using Row = int32_t;
using Coordinates = pair<Column, Row>;
using Round = uint32_t;

Map ReadInput(std::istream & stream)
{
    Map out;
    string line;
    Row y = 0;
    while (std::getline(stream, line)) {
        if (y > MapSize) {
            std::cerr << "Input file is too long (max = " << MapSize
                      << " lines).\n";
        }
        Column lineSize = line.size();
        if (line.size() > MapSize) {
            std::cerr << "Input line " << y << " is too long (max = " << MapSize
                      << " characters).\n";
            std::exit(1);
        }

        for (Column x = 0; x < lineSize; x++) {
            if (line[x] != '#') {
                out[y][x] = true;
            }
        }

        y++;
    }

    return out;
}

void PrintMap(std::ostream & stream, const Map & map)
{
    for (Row y = 0; y < MapSize; y++) {
        for (Column x = 0; x < MapSize; x++) {
            stream << (map[y][x] ? '.' : '#');
        }
        stream << '\n';
    }
}

int main(int /*argc*/, char ** /*argv*/)
{
    const Map map = ReadInput(std::cin);
    PrintMap(std::cout, map);
    return 0;
}
