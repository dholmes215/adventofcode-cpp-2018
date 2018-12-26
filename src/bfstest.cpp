// Copyright (C) 2018 David Holmes <dholmes@dholmes.us>. All rights reserved.

#include "day15.hpp"
#include "day15test.hpp"

#include <iostream>

using namespace day15;

char PredecessorChar(const Coordinates coord, const Coordinates pred)
{
    if (coord + Coordinates(0, -1) == pred) {
        return '^';
    }
    if (coord + Coordinates(0, 1) == pred) {
        return 'v';
    }
    if (coord + Coordinates(-1, 0) == pred) {
        return '<';
    }
    if (coord + Coordinates(1, 0) == pred) {
        return '>';
    }
    return ' ';
}

void PrintBfs(const BfsResult & bfsResult)
{
    for (Row y = 0; y < MapSize; y++) {
        for (Column x = 0; x < MapSize; x++) {
            Coordinates coord = {x, y};
            optional<Coordinates> pred;
            if (Contains(bfsResult.predecessors, coord)) {
                pred = bfsResult.predecessors.at(coord);
            }
            if (pred) {
                std::cout << PredecessorChar(coord, *pred);
            } else {
                std::cout << ' ';
            }
        }
        std::cout << '\n';
    }
}

int main(int /*argc*/, char ** /*argv*/)
{
    // State state = surroundedState;
    const auto [map, state] = puzzleInput;

    BfsResult bfsResult = Bfs(map, state, {29, 2});
    PrintBfs(bfsResult);

    return 0;
}