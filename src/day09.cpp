// Copyright (C) 2018 David Holmes <dholmes@dholmes.us>. All rights reserved.

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <list>
#include <regex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

using std::pair;
using std::uint32_t;

using Player = std::uint16_t;
using Marble = std::uint32_t;
using Score = std::uint32_t;
using Circle = std::list<Marble>;
using Scoreboard = std::map<Player, Score>;

void PrintCircle(std::ostream & stream, const Circle & circle,
                 const Circle::const_iterator & iter)
{
    for (Marble m : circle) {
        if (*iter == m) {
            stream << "(" << std::setw(2) << m << ')';
        } else {
            stream << " " << std::setw(2) << m << ' ';
        }
    }
}

void PrintGameState(std::ostream & stream, const Circle & circle,
                    Circle::const_iterator currentMarbleIter,
                    Player currentPlayer)
{
    if (currentPlayer == 0) {
        stream << '[' << std::setw(4) << '-' << ']';
    } else {
        stream << '[' << std::setw(4) << currentPlayer << ']';
    }
    PrintCircle(stream, circle, currentMarbleIter);
    stream << '\n';
}

void PrintScoreboard(std::ostream & stream, const Scoreboard & scoreboard)
{
    for (auto & [player, score] : scoreboard) {
        stream << "Player " << std::setw(4) << player << ": " << score << '\n';
    }
}

pair<Player, Score> GetHighScore(const Scoreboard & scoreboard)
{
    Player highestPlayer = 1;
    Score highestScore = scoreboard.at(1);
    for (auto & [player, score] : scoreboard) {
        if (score > highestScore) {
            highestPlayer = player;
            highestScore = score;
        }
    }
    return std::make_pair(highestPlayer, highestScore);
}

pair<Player, Marble> ParseInput(std::string_view str)
{
    static const std::regex inputRegex(
        "(\\d+) players; last marble is worth (\\d+) points");
    std::cmatch match;
    std::regex_search(str.begin(), str.end(), match, inputRegex);
    return std::make_pair(std::stol(match[1]), std::stol(match[2]));
}

pair<Player, Marble> ReadInput(std::istream & stream)
{
    std::string line;
    std::getline(stream, line);
    return ParseInput(line);
}

Circle::iterator MoveIterBack(Circle & circle, Circle::iterator iter, int count)
{
    for (int i = 0; i < count; i++) {
        if (iter == circle.begin()) {
            iter = circle.end();
        }
        iter--;
    }
    return iter;
}

Circle::iterator MoveIterForward(Circle & circle, Circle::iterator iter,
                                 int count)
{
    for (int i = 0; i < count; i++) {
        iter++;
        if (iter == circle.end()) {
            iter = circle.begin();
        }
    }
    return iter;
}

Circle::iterator InsertMarble(Circle & circle, Circle::iterator iter,
                              Marble marble)
{
    if (iter == circle.begin()) {
        iter = circle.end();
    }
    return circle.insert(iter, marble);
}

void RunGame(Player playerCount, Marble lastMarble)
{
    std::cout << playerCount << " players; last marble is worth " << lastMarble
              << " points" << '\n';

    Circle circle;
    Scoreboard scoreboard;
    // Add all players to scoreboard.
    for (Player p = 1; p <= playerCount; p++) {
        scoreboard[p] = 0;
    }
    Player currentPlayer = 1;
    Marble nextMarble = 0;
    Circle::iterator currentMarbleIter =
        circle.insert(circle.end(), nextMarble++);

    // Place first marble.

    // PrintGameState(std::cout, circle, currentMarbleIter, currentPlayer);

    while (nextMarble <= lastMarble) {

        if (nextMarble % 23 == 0) {
            scoreboard[currentPlayer] += nextMarble;
            currentMarbleIter = MoveIterBack(circle, currentMarbleIter, 7);
            scoreboard[currentPlayer] += *currentMarbleIter;
            currentMarbleIter = circle.erase(currentMarbleIter);
        } else {
            currentMarbleIter = InsertMarble(
                circle, MoveIterForward(circle, currentMarbleIter, 2),
                nextMarble);
        }

        // PrintGameState(std::cout, circle, currentMarbleIter, currentPlayer);

        ++nextMarble;
        ++currentPlayer;
        if (currentPlayer > playerCount) {
            currentPlayer = 1;
        }
    }

    // PrintScoreboard(std::cout, scoreboard);
    auto [highScorePlayer, highScore] = GetHighScore(scoreboard);

    std::cout << "High score: Player " << highScorePlayer << " (" << highScore
              << " points)\n";
}

int main(int /*argc*/, char ** /*argv*/)
{
    const auto [playerCount, lastMarble] = ReadInput(std::cin);
    RunGame(playerCount, lastMarble);
    RunGame(playerCount, lastMarble * 100);

    return 0;
}
