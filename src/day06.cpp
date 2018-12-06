// Copyright (C) 2018 David Holmes <dholmes@dholmes.us>. All rights reserved.

#include <algorithm>
#include <array>
#include <bitset>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <map>
#include <regex>
#include <set>
#include <string>

using Coordinate = std::pair<std::int32_t, std::int32_t>;
using CoordinateId = std::uint8_t;
using CoordinateWithId = std::pair<Coordinate, CoordinateId>;
using CoordinateMap = std::map<Coordinate, CoordinateId>;

const CoordinateId none = 255;
const std::size_t mapSize = 400;
using TerritoryMap = std::array<std::array<CoordinateId, mapSize>, mapSize>;
using SafeRegionMap = std::array<std::bitset<mapSize>, mapSize>;

Coordinate CoordinateFromStr(std::string_view str)
{
    static const std::regex coordRegex("(\\d+),\\s*(\\d+)");
    std::cmatch match;
    std::regex_search(str.begin(), str.end(), match, coordRegex);
    return std::make_pair(std::stoi(match[1]), std::stoi(match[2]));
}

std::ostream & operator<<(std::ostream & stream, const Coordinate & coord)
{
    stream << '(' << coord.first << "," << coord.second << ')';
    return stream;
}

std::uint16_t ManhattanDistance(const Coordinate & a, const Coordinate & b)
{
    return std::abs(a.first - b.first) + std::abs(a.second - b.second);
}

CoordinateId FindNearest(const Coordinate & coordinate,
                         const CoordinateMap & map)
{
    std::vector<CoordinateWithId> coordVector(map.begin(), map.end());
    // Sort vector by distance to coordinate.
    std::sort(coordVector.begin(), coordVector.end(),
              [&](const CoordinateWithId & a, const CoordinateWithId & b) {
                  return ManhattanDistance(coordinate, a.first) <
                         ManhattanDistance(coordinate, b.first);
              });

    bool tie = ManhattanDistance(coordinate, coordVector[0].first) ==
               ManhattanDistance(coordinate, coordVector[1].first);
    return tie ? none : coordVector[0].second;
}

char IdToCharacter(const CoordinateId id)
{
    char ch = ' ';
    if (id < 26) {
        ch = 'A' + id;
    } else if (id < 52) {
        ch = 'a' + id - 26;
    }
    return ch;
}

void PrintTerritoryMap(const TerritoryMap & territoryMap,
                       const CoordinateMap & coordinateMap)
{
    for (std::uint32_t y = 0; y < mapSize; y++) {
        for (std::uint32_t x = 0; x < mapSize; x++) {
            auto id = territoryMap[y][x];
            auto find = coordinateMap.find({x, y});
            if (find != coordinateMap.end()) {
                char ch = IdToCharacter(id);
                std::uint16_t color = id;
                std::cout << "\033[48;5;" << color << "m" << ch << "\033[0m";
            } else {
                if (id == none) {
                    std::cout << '.';
                } else {
                    std::uint16_t color = id;
                    std::cout << "\033[48;5;" << color << "m.\033[0m";
                }
            }
        }
        std::cout << '\n';
    }
}

TerritoryMap CalculateTerritoryMap(const CoordinateMap & coordMap)
{
    TerritoryMap out;

    for (std::uint32_t y = 0; y < mapSize; y++) {
        for (std::uint32_t x = 0; x < mapSize; x++) {
            out[y][x] = FindNearest({x, y}, coordMap);
        }
    }

    return out;
}

SafeRegionMap CalculateSafeRegionMap(const CoordinateMap & coordMap)
{
    const auto maxSafeDistanceSum = 10000;
    SafeRegionMap out;

    for (std::size_t y = 0; y < mapSize; y++) {
        for (std::size_t x = 0; x < mapSize; x++) {
            const Coordinate coord = {x, y};
            std::uint32_t distanceSum = 0;
            for (const auto & coord2 : coordMap) {
                distanceSum += ManhattanDistance(coord, coord2.first);
            }
            out[y][x] = (distanceSum < maxSafeDistanceSum);
        }
    }

    return out;
}

void PrintSafeRegionMap(const SafeRegionMap & safeRegionMap,
                        const CoordinateMap & coordinateMap)
{
    for (std::uint32_t y = 0; y < mapSize; y++) {
        for (std::uint32_t x = 0; x < mapSize; x++) {
            auto find = coordinateMap.find({x, y});
            if (find != coordinateMap.end()) {
                char ch = IdToCharacter(find->second);
                std::uint16_t color = find->second;
                std::cout << "\033[48;5;" << color << "m" << ch << "\033[0m";
            } else {
                if (safeRegionMap[y][x]) {
                    std::cout << "\033[48;5;2m.\033[0m";
                } else {
                    std::cout << '.';
                }
            }
        }
        std::cout << '\n';
    }
}

int main(int /*argc*/, char ** /*argv*/)
{
    std::cout << "PROTIP: For best results, run with something like:\n $ day06 "
                 "< day06input.txt | less -RS -#20\n";

    std::string line;
    CoordinateMap coordMap;
    std::map<CoordinateId, Coordinate> coordsById;
    CoordinateId id = 0;
    while (std::getline(std::cin, line)) {
        Coordinate coord = CoordinateFromStr(line);
        coordMap.emplace(std::make_pair(coord, id));
        coordsById.emplace(std::make_pair(id, coord));
        ++id;
    }

    const auto territoryMap = CalculateTerritoryMap(coordMap);
    PrintTerritoryMap(territoryMap, coordMap);
    // Coordinates on the edge of the map have "infinite" territory
    std::set<CoordinateId> edgeIds;
    for (std::size_t i = 0; i < mapSize; i++) {
        edgeIds.insert(territoryMap[i][0]);
        edgeIds.insert(territoryMap[i][mapSize - 1]);
        edgeIds.insert(territoryMap[0][i]);
        edgeIds.insert(territoryMap[mapSize - 1][i]);
    }

    std::map<CoordinateId, std::uint32_t> territorySizes;
    for (std::size_t y = 0; y < mapSize; y++) {
        for (std::size_t x = 0; x < mapSize; x++) {
            auto id = territoryMap[y][x];
            bool isEdge = edgeIds.find(id) != edgeIds.end();
            if (!isEdge) {
                territorySizes[id]++;
            }
        }
    }

    auto [maxTerritoryId, maxTerritorySize] =
        *std::max_element(territorySizes.begin(), territorySizes.end(),
                          [](auto a, auto b) { return a.second < b.second; });
    auto maxTerritoryCoord = coordsById[maxTerritoryId];

    auto safeRegionMap = CalculateSafeRegionMap(coordMap);
    PrintSafeRegionMap(safeRegionMap, coordMap);

    std::uint32_t safeRegionSize = 0;
    for (auto & row : safeRegionMap) {
        safeRegionSize += row.count();
    }

    std::cout << "Coordinate " << maxTerritoryCoord << " ('"
              << IdToCharacter(maxTerritoryId)
              << "') has largest area: " << maxTerritorySize << '\n';

    std::cout << "Safe Region Size: " << safeRegionSize << '\n';

    return 0;
}
