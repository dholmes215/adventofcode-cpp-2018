// Copyright (C) 2018 David Holmes <dholmes@dholmes.us>. All rights reserved.

#include "ansiterm.hpp"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>

using std::uint32_t;
using std::uint64_t;
using std::uint8_t;
using std::vector;

using RecipeScore = uint8_t;
using ScoreIndex = vector<RecipeScore>::size_type;

void AppendNewScores(vector<RecipeScore> & scores, RecipeScore elf1,
                     RecipeScore elf2)
{
    auto sum = elf1 + elf2;
    if (sum == 0) {
        scores.push_back(0);
        return;
    }
    if (sum >= 10) {
        scores.push_back(1);
    }
    scores.push_back(sum % 10);
}

vector<RecipeScore> ScoresFromInteger(uint32_t i)
{
    vector<RecipeScore> out;
    if (i == 0) {
        return {0};
    }
    while (i > 0) {
        out.insert(out.begin(), i % 10);
        i /= 10;
    }
    return out;
}

uint32_t ReadInput(std::istream & stream)
{
    uint32_t input;
    stream >> input;
    return input;
}

void PrintScores(std::ostream & stream, const vector<RecipeScore> & scores,
                 ScoreIndex elf1, ScoreIndex elf2)
{
    for (ScoreIndex i = 0; i < scores.size(); i++) {
        if (i == elf1) {
            stream << ansi::graphic::fg_color(ansi::graphic::color3::red);
        } else if (i == elf2) {
            stream << ansi::graphic::fg_color(ansi::graphic::color3::green);
        }
        stream << static_cast<int>(scores[i]);
        stream << ansi::graphic::reset();
    }
    stream << '\n';
}

int main(int /*argc*/, char ** /*argv*/)
{
    vector<RecipeScore> scores = {3, 7};
    auto input = ReadInput(std::cin);
    vector<RecipeScore> part2TargetScores = ScoresFromInteger(input);
    ScoreIndex elf1 = 0;
    ScoreIndex elf2 = 1;

    bool part2Solved = false;
    ScoreIndex part2Solution = 0;

    while (!part2Solved) {

        for (int i = 0; i < 1000000; i++) {

            AppendNewScores(scores, scores[elf1], scores[elf2]);
            auto scoresSize = scores.size();
            elf1 = ((elf1 + 1 + scores[elf1]) % scoresSize);
            elf2 = ((elf2 + 1 + scores[elf2]) % scoresSize);
            // PrintScores(std::cout, scores, elf1, elf2);
        }

        auto s =
            std::search(scores.begin(), scores.end(), part2TargetScores.begin(),
                        part2TargetScores.end());
        if (s != scores.end()) {
            part2Solved = true;
            part2Solution = s - scores.begin();
        }
    }

    std::cout << "Next ten scores after " << input << " recipes: ";
    for (ScoreIndex i = input; i < input + 10; i++) {
        std::cout << static_cast<int>(scores[i]);
    }
    std::cout << '\n';

    std::cout << "Number of recipes to the left of \"" << input
              << "\": " << part2Solution << '\n';

    return 0;
}
