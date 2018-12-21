// Copyright (C) 2018 David Holmes <dholmes@dholmes.us>. All rights reserved.

#include "day15.hpp"

#include "gtest/gtest.h"

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

TEST(SelectAdjacentTargetTest, AllEqualHp)
{
    State state = surroundedState;
    EntityId two = 2;
    ASSERT_EQ(two, *SelectAdjacentTarget(state, surroundedSource, surroundedTargets));
}

TEST(SelectAdjacentTargetTest, NorthLowestHp)
{
    State state = surroundedState;
    state.entities.at(2).hp--;
    EntityId two = 2;
    ASSERT_EQ(two, *SelectAdjacentTarget(state, surroundedSource, surroundedTargets));
}

TEST(SelectAdjacentTargetTest, WestLowestHp)
{
    State state = surroundedState;
    state.entities.at(3).hp--;
    EntityId three = 3;
    ASSERT_EQ(three, *SelectAdjacentTarget(state, surroundedSource, surroundedTargets));
}

TEST(SelectAdjacentTargetTest, EastLowestHp)
{
    State state = surroundedState;
    state.entities.at(4).hp--;
    EntityId four = 4;
    ASSERT_EQ(four, *SelectAdjacentTarget(state, surroundedSource, surroundedTargets));
}

TEST(SelectAdjacentTargetTest, SouthLowestHp)
{
    State state = surroundedState;
    state.entities.at(5).hp--;
    EntityId five = 5;
    ASSERT_EQ(five, *SelectAdjacentTarget(state, surroundedSource, surroundedTargets));
}

TEST(SelectAdjacentTargetTest, EastWestTie)
{
    State state = surroundedState;
    state.entities.at(3).hp--;
    EntityId three = 3;
    state.entities.at(4).hp--;
    ASSERT_EQ(three, *SelectAdjacentTarget(state, surroundedSource, surroundedTargets));
}
int main(int argc, char ** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
