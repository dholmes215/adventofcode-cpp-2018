// Copyright (C) 2018 David Holmes <dholmes@dholmes.us>. All rights reserved.

#include <algorithm>
#include <array>
#include <bitset>
#include <cassert>
#include <charconv>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <regex>
#include <set>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <vector>

using namespace std::chrono_literals;

const auto maxDisplayDimension = 64;

using SceneRow = std::bitset<maxDisplayDimension>;
using Scene = std::array<SceneRow, maxDisplayDimension>;

struct Position
{
    std::int32_t x;
    std::int32_t y;

    bool operator<(const Position & other) const
    {
        return std::tie(x, y) < std::tie(other.x, other.y);
    }
    bool operator>(const Position & other) const { return other < *this; }
    bool operator<=(const Position & other) const { return !(other < *this); }
    bool operator>=(const Position & other) const { return !(other > *this); }

    bool operator==(const Position & other) const
    {
        return std::tie(x, y) == std::tie(other.x, other.y);
    }
    bool operator!=(const Position & other) const { return !(*this == other); }
};

std::ostream & operator<<(std::ostream & stream, const Position & pos)
{
    stream << "position=<" << std::setw(6) << pos.x << ", " << std::setw(6)
           << pos.y << '>';
    return stream;
}

struct Velocity
{
    std::int8_t dx;
    std::int8_t dy;
};

std::ostream & operator<<(std::ostream & stream, const Velocity & vel)
{
    stream << "velocity=<" << std::setw(2) << static_cast<std::int32_t>(vel.dx)
           << ", " << std::setw(2) << static_cast<std::int32_t>(vel.dy) << '>';
    return stream;
}

struct Point
{
    Position pos;
    Velocity vel;
};

std::ostream & operator<<(std::ostream & stream, const Point & point)
{
    stream << point.pos << ", " << point.vel;
    return stream;
}

Point ParsePoint(std::string_view str)
{
    static const std::regex pointRegex(
        "position=<\\s*(-?\\d+),\\s*(-?\\d+)>\\s*velocity=<\\s*(-?\\d+),\\s*(-?"
        "\\d+)>");
    std::cmatch match;
    std::regex_search(str.begin(), str.end(), match, pointRegex);
    Point p;
    std::from_chars(match[1].first, match[1].second, p.pos.x);
    std::from_chars(match[2].first, match[2].second, p.pos.y);
    std::from_chars(match[3].first, match[3].second, p.vel.dx);
    std::from_chars(match[4].first, match[4].second, p.vel.dy);
    return p;
}

std::vector<Point> ReadPoints(std::istream & stream)
{

    std::vector<Point> out;

    std::string line;
    while (std::getline(stream, line)) {
        out.push_back(ParsePoint(line));
    }

    return out;
}

struct Area
{
    Position nwCorner;
    Position seCorner;
};

Area GetBoundaries(const std::vector<Point> & points)
{
    if (points.empty()) {
        return {{0, 0}, {0, 0}};
    }
    Area area;
    auto max = std::numeric_limits<decltype(area.nwCorner.x)>::max();
    auto min = std::numeric_limits<decltype(area.nwCorner.x)>::min();
    area.nwCorner = {max, max};
    area.seCorner = {min, min};
    for (auto & p : points) {
        Position pos = p.pos;
        area.nwCorner.x = std::min(area.nwCorner.x, pos.x);
        area.nwCorner.y = std::min(area.nwCorner.y, pos.y);
        area.seCorner.x = std::max(area.seCorner.x, pos.x);
        area.seCorner.y = std::max(area.seCorner.y, pos.y);
    }
    return area;
}

const std::array<const char *, 16> quadrantStrings = {
    " ", "▘", "▝", "▀", "▖", "▌", "▞", "▛",
    "▗", "▚", "▐", "▜", "▄", "▙", "▟", "█"};

inline constexpr std::string_view
QuadrantsToStr(bool upperLeft, bool upperRight, bool lowerLeft, bool lowerRight)
{
    return quadrantStrings[upperLeft | upperRight << 1 | lowerLeft << 2 |
                           lowerRight << 3];
}

void PrintScene(std::ostream & stream, Scene & scene)
{

    for (int y = 0; y < maxDisplayDimension - 1; y += 2) {
        for (int x = 0; x < maxDisplayDimension - 1; x += 2) {
            stream << QuadrantsToStr(scene[y][x], scene[y][x + 1],
                                     scene[y + 1][x], scene[y + 1][x + 1]);
        }
        stream << '\n';
    }
}

/// Draws the scene, at (1/(scaleFactor+1)) scale.
void DrawSceneScale(std::ostream & stream, const std::vector<Point> & points,
                    std::int32_t scaleExponent)
{
    const Area area = GetBoundaries(points);
    Area scaledArea;
    scaledArea.nwCorner = {area.nwCorner.x >> scaleExponent,
                           area.nwCorner.y >> scaleExponent};
    scaledArea.seCorner = {area.seCorner.x >> scaleExponent,
                           area.seCorner.y >> scaleExponent};

    using SceneRow = std::bitset<maxDisplayDimension>;
    using Scene = std::array<SceneRow, maxDisplayDimension>;
    Scene scene = {0};

    for (auto & p : points) {
        Position pos = p.pos;
        Position scaledPos = {pos.x >> scaleExponent, pos.y >> scaleExponent};

        assert(scaledPos.x >= scaledArea.nwCorner.x);
        assert(scaledPos.y >= scaledArea.nwCorner.y);
        assert(scaledPos.x <= scaledArea.seCorner.x);
        assert(scaledPos.y <= scaledArea.seCorner.y);

        Position shiftedScaledPos = {scaledPos.x - scaledArea.nwCorner.x,
                                     scaledPos.y - scaledArea.nwCorner.y};

        assert(shiftedScaledPos.x >= 0);
        assert(shiftedScaledPos.y >= 0);
        assert(shiftedScaledPos.x < maxDisplayDimension);
        assert(shiftedScaledPos.y < maxDisplayDimension);

        scene[shiftedScaledPos.y][shiftedScaledPos.x] = true;
    }

    PrintScene(stream, scene);
}

int main(int /*argc*/, char ** /*argv*/)
{
    const std::vector<Point> inputPoints = ReadPoints(std::cin);
    std::vector<Point> points = inputPoints;

    DrawSceneScale(std::cout, points, 11);

    for (int i = 1; i < 11000; i++) {
        for (auto & [pos, vel] : points) {
            pos.x += vel.dx * 1;
            pos.y += vel.dy * 1;
        }
        if (i > 10354 && i < 10356) {
            std::this_thread::sleep_for(500ms);
            std::cout << i << '\n';
            DrawSceneScale(std::cout, points, 0);
        }
    }

    return 0;
}
