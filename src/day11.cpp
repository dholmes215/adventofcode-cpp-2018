// Copyright (C) 2018 David Holmes <dholmes@dholmes.us>. All rights reserved.

#include <array>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <utility>
#include <vector>

using std::array;
using std::int16_t;
using std::int32_t;
using std::ostream;
using std::pair;
using std::size_t;
using std::uint16_t;
using std::vector;
using SerialNumber = int16_t;
using PowerLevel = int32_t;
using Coordinate = pair<uint16_t, uint16_t>;
using Size = uint16_t;
using Square = pair<Coordinate, Size>;

const Size gridSize = 300;

// Generic vector stream insertion operator
template <typename ValueType>
ostream & operator<<(ostream & stream, const vector<ValueType> & container)
{
    auto iter = container.begin();
    stream << '{' << *iter++;
    for (; iter < container.end(); iter++) {
        stream << "," << *iter;
    }
    stream << '}';
    return stream;
}

// Coordinate stream insertion operator
ostream & operator<<(ostream & stream, const Coordinate & coord)
{
    auto [x, y] = coord;
    stream << '{' << x << ',' << y << '}';
    return stream;
}

// Square stream insertion operator
ostream & operator<<(ostream & stream, const Square & square)
{
    auto [coord, size] = square;
    auto [x, y] = coord;
    stream << '{' << x << ',' << y << ',' << size << '}';
    return stream;
}

class Grid
{
public:
    Grid() = default;

    Grid(SerialNumber serialNumber) : grid()
    {
        for (int y = 1; y <= gridSize; y++) {
            for (int x = 1; x <= gridSize; x++) {
                std::uint16_t rackId = x + 10;
                PowerLevel powerLevel = rackId * y;
                powerLevel += serialNumber;
                powerLevel *= rackId;
                powerLevel = (powerLevel / 100) % 10;
                powerLevel -= 5;
                (*this)[{x, y}] = powerLevel;
            }
        }
    }

    PowerLevel & operator[](Coordinate coord)
    {
        auto [x, y] = coord;
        return grid[y - 1][x - 1];
    }

    const PowerLevel & operator[](Coordinate coord) const
    {
        auto [x, y] = coord;
        return grid[y - 1][x - 1];
    }

    void Print(std::ostream & stream) const
    {
        for (uint16_t y = 1; y <= gridSize; y++) {
            for (uint16_t x = 1; x <= gridSize; x++) {
                stream << std::setw(4) << (*this)[{x, y}];
            }
            stream << '\n';
        }
    }

private:
    array<array<PowerLevel, gridSize>, gridSize> grid;
};

using SubproblemGrids = array<Grid, gridSize>;

PowerLevel GetSquarePowerLevelDynamic(const SubproblemGrids & grids,
                                      Square square)
{
    PowerLevel powerLevel = 0;
    auto [coord, size] = square;
    auto [x, y] = coord;
    if (size % 2 == 0) {
        Size halfSize = size / 2;
        Coordinate topLeft = {x, y};
        Coordinate topRight = {x + halfSize, y};
        Coordinate bottomLeft = {x, y + halfSize};
        Coordinate bottomRight = {x + halfSize, y + halfSize};
        powerLevel += grids[halfSize - 1][topLeft];
        powerLevel += grids[halfSize - 1][topRight];
        powerLevel += grids[halfSize - 1][bottomLeft];
        powerLevel += grids[halfSize - 1][bottomRight];
    } else {
        powerLevel += grids[size - 2][coord];
        for (uint16_t y2 = y; y2 < y + size; y2++) {
            uint16_t x2 = x + size - 1;
            powerLevel += grids[0][{x2, y2}];
        }
        for (uint16_t x2 = x; x2 < x + size - 1; x2++) {
            uint16_t y2 = y + size - 1;
            powerLevel += grids[0][{x2, y2}];
        }
    }
    return powerLevel;
}

void PopulateSubproblemGridDynamic(SubproblemGrids & grids, Size size)
{
    size_t gridIndex = size - 1;

    for (uint16_t y = 1; y <= gridSize - size + 1; y++) {
        for (uint16_t x = 1; x <= gridSize - size + 1; x++) {
            Square square = {{x, y}, size};
            grids[gridIndex][{x, y}] =
                GetSquarePowerLevelDynamic(grids, square);
        }
    }
}

pair<Coordinate, PowerLevel> FindHighestPowerCell(const Grid & grid, Size size)
{
    Coordinate highestCell;
    PowerLevel highestPower = std::numeric_limits<PowerLevel>::min();
    for (uint16_t y = 1; y <= size; y++) {
        for (uint16_t x = 1; x <= size; x++) {
            Coordinate cell = {x, y};
            PowerLevel power = grid[cell];
            if (power > highestPower) {
                highestPower = power;
                highestCell = cell;
            }
        }
    }

    return {highestCell, highestPower};
}

pair<Coordinate, PowerLevel>
FindHighestPowerSquareDynamic(const SubproblemGrids & subproblemGrids,
                              Size size)
{
    return FindHighestPowerCell(subproblemGrids[size - 1], gridSize - size + 1);
}

pair<Square, PowerLevel>
FindHighestPowerSquareDynamic(const SubproblemGrids & subproblemGrids)
{
    Square highestPowerSquare;
    PowerLevel highestPower = std::numeric_limits<PowerLevel>::min();
    for (Size size = 1; size <= gridSize; size++) {
        auto [coord, power] =
            FindHighestPowerSquareDynamic(subproblemGrids, size);
        Square square = {coord, size};
        if (power > highestPower) {
            highestPowerSquare = square;
            highestPower = power;
        }
    }

    return {highestPowerSquare, highestPower};
}

int main(int /*argc*/, char ** /*argv*/)
{
    SerialNumber serialNumber = 0;
    std::cin >> serialNumber;

    // Note that sizes are indexed from 1, but this array is indexed from 0.
    auto pSubproblemGrids = std::make_unique<SubproblemGrids>();
    SubproblemGrids & subproblemGrids = *pSubproblemGrids;
    subproblemGrids[0] = Grid(serialNumber);

    for (Size size = 2; size <= gridSize; size++) {
        PopulateSubproblemGridDynamic(subproblemGrids, size);
    }

    auto [square1, power1] = FindHighestPowerSquareDynamic(subproblemGrids, 3);
    std::cout << "Part 1: 3x3 square " << square1
              << " has highest total power level (" << power1 << ")\n";

    // for (auto & grid : subproblemGrids) {
    //     std::cout << '\n';
    //     grid.Print(std::cout);
    // }

    auto [square2, power2] = FindHighestPowerSquareDynamic(subproblemGrids);
    auto [coords, size] = square2;
    std::cout << "Part 2: " << size << "x" << size << " square " << square2
              << " has highest total power level (" << power2 << ")\n";
    return 0;
}
