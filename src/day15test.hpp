// Copyright (C) 2018 David Holmes <dholmes@dholmes.us>. All rights reserved.

#ifndef AOC_DAY15TEST_HPP
#define AOC_DAY15TEST_HPP

#include "day15.hpp"

#include <sstream>

using namespace day15;

const char * const exampleInputStr = "#######\n"
                                     "#.G...#\n"
                                     "#...EG#\n"
                                     "#.#.#G#\n"
                                     "#..G#E#\n"
                                     "#.....#\n"
                                     "#######\n";

const pair<Map, State> exampleInput = []() {
    std::stringstream ss;
    ss << exampleInputStr;
    return ReadInput(ss);
}();

const char * const puzzleInputStr = "################################\n"
                                    "#########.####...#####.#########\n"
                                    "#####...#G...#.G.##.#...##.##..#\n"
                                    "####...G####.G....#..........E.#\n"
                                    "#####..#######.................#\n"
                                    "#####..###########.............#\n"
                                    "#GG...############.............#\n"
                                    "#...#.#.##..######..........#..#\n"
                                    "##............#G.#..E.......####\n"
                                    "##.G........#....#.........#####\n"
                                    "###....G...................#####\n"
                                    "###............G.....G.......###\n"
                                    "#####.....#..G#####..........###\n"
                                    "####..#......#######G...#.E..E##\n"
                                    "####G##.G...#########.#.......##\n"
                                    "###..###....#########...E....###\n"
                                    "##...G......#########.E...######\n"
                                    "##G.........#########......#####\n"
                                    "##...#.G....#########.#...######\n"
                                    "##...#.......#######E.##########\n"
                                    "####.#........#####...##########\n"
                                    "#######............E..##########\n"
                                    "####..#...........E#############\n"
                                    "##...G#...........##############\n"
                                    "##........#.......##############\n"
                                    "#####G..###..E..################\n"
                                    "##########......################\n"
                                    "##########.....#################\n"
                                    "#########......#################\n"
                                    "###########.....################\n"
                                    "###########...##################\n"
                                    "################################\n";

const pair<Map, State> puzzleInput = []() {
    std::stringstream ss;
    ss << puzzleInputStr;
    return ReadInput(ss);
}();

const State surroundedState = []() {
    State state;

    state.entities.insert({1, {1, EntityType::Elf, {2, 2}}});
    state.entitiesByLocation.insert({{2, 2}, 1});
    state.entities.at(1).hp = 200;

    state.entities.insert({2, {2, EntityType::Goblin, {2, 1}}});
    state.entitiesByLocation.insert({{2, 1}, 2});
    state.entities.at(2).hp = 200;

    state.entities.insert({3, {3, EntityType::Goblin, {1, 2}}});
    state.entitiesByLocation.insert({{1, 2}, 3});
    state.entities.at(3).hp = 200;

    state.entities.insert({4, {4, EntityType::Goblin, {3, 2}}});
    state.entitiesByLocation.insert({{3, 2}, 4});
    state.entities.at(4).hp = 200;

    state.entities.insert({5, {5, EntityType::Goblin, {2, 3}}});
    state.entitiesByLocation.insert({{2, 3}, 5});
    state.entities.at(5).hp = 200;

    return state;
}();

const Coordinates surroundedSource = {2, 2};

const set<Coordinates> surroundedTargets = []() {
    set<Coordinates> targets;
    targets.insert({2, 1});
    targets.insert({1, 2});
    targets.insert({3, 2});
    targets.insert({2, 3});
    return targets;
}();

#endif // AOC_DAY15TEST_HPP
