// Copyright (C) 2018 David Holmes <dholmes@dholmes.us>. All rights reserved.

#include "day15test.hpp"
#include "day15.hpp"

#include "gtest/gtest.h"

TEST(SelectAdjacentTargetTest, AllEqualHp)
{
    State state = surroundedState;
    EntityId two = 2;
    ASSERT_EQ(
        two, *SelectAdjacentTarget(state, surroundedSource, surroundedTargets));
}

TEST(SelectAdjacentTargetTest, NorthLowestHp)
{
    State state = surroundedState;
    state.entities.at(2).hp--;
    EntityId two = 2;
    ASSERT_EQ(
        two, *SelectAdjacentTarget(state, surroundedSource, surroundedTargets));
}

TEST(SelectAdjacentTargetTest, WestLowestHp)
{
    State state = surroundedState;
    state.entities.at(3).hp--;
    EntityId three = 3;
    ASSERT_EQ(three, *SelectAdjacentTarget(state, surroundedSource,
                                           surroundedTargets));
}

TEST(SelectAdjacentTargetTest, EastLowestHp)
{
    State state = surroundedState;
    state.entities.at(4).hp--;
    EntityId four = 4;
    ASSERT_EQ(four, *SelectAdjacentTarget(state, surroundedSource,
                                          surroundedTargets));
}

TEST(SelectAdjacentTargetTest, SouthLowestHp)
{
    State state = surroundedState;
    state.entities.at(5).hp--;
    EntityId five = 5;
    ASSERT_EQ(five, *SelectAdjacentTarget(state, surroundedSource,
                                          surroundedTargets));
}

TEST(SelectAdjacentTargetTest, EastWestTie)
{
    State state = surroundedState;
    state.entities.at(3).hp--;
    EntityId three = 3;
    state.entities.at(4).hp--;
    ASSERT_EQ(three, *SelectAdjacentTarget(state, surroundedSource,
                                           surroundedTargets));
}

TEST(SearchForTargetTest, RedditExample1)
{
    // First test case from comment at: https://redd.it/a7fhax
    const char * const example1InputStr = "########\n"
                                          "#.E....#\n"
                                          "#......#\n"
                                          "#....G.#\n"
                                          "#...G..#\n"
                                          "#G.....#\n"
                                          "########\n";

    const auto [map, state] = [=]() {
        std::stringstream ss;
        ss << example1InputStr;
        return ReadInput(ss);
    }();

    const set<Coordinates> targets = GetTargets(state, EntityType::Goblin);

    const Coordinates elf = {2,1};
    const auto maybePath = SearchForTarget(map, state, elf, targets);

    const Path expectedPath = {{3,1}, {4,1}, {5,1}, {5,2}};
    
    ASSERT_EQ(*maybePath, expectedPath);
}

TEST(SearchForTargetTest, RedditExample2)
{
    // Second test case from comment at: https://redd.it/a7fhax
    const char * const example2InputStr = "######\n"
                                          "#.G..#\n"
                                          "##..##\n"
                                          "#...E#\n"
                                          "#E...#\n"
                                          "######\n";

    const auto [map, state] = [=]() {
        std::stringstream ss;
        ss << example2InputStr;
        return ReadInput(ss);
    }();

    const set<Coordinates> targets = GetTargets(state, EntityType::Elf);

    const Coordinates goblin = {2,1};
    const auto maybePath = SearchForTarget(map, state, goblin, targets);

    const Path expectedPath = {{2,2}, {2,3}, {1,3}};
    
    ASSERT_EQ(*maybePath, expectedPath);
}

int main(int argc, char ** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
