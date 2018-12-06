// Copyright (C) 2018 David Holmes <dholmes@dholmes.us>. All rights reserved.

#include <algorithm>
#include <cctype>
#include <exception>
#include <iostream>
#include <limits>
#include <string>

/// Flip the case of an ASCII letter.  Will not work for non-letter or non-ASCII
/// input.
char FlipCase(char c)
{
    if (c >= 'A' && c <= 'Z') {
        return std::tolower(c);
    } else if (c >= 'a' && c <= 'z') {
        return std::toupper(c);
    }
    throw std::invalid_argument("Invalid input to FlipCase()");
}

std::uint32_t DestroyUnits(std::string & polymer)
{
    // Replace destroyed units with underscores.
    std::uint32_t destroyedUnits = 0;
    auto polymerLength = polymer.size();
    for (std::size_t i = 0; i < polymerLength - 1; i++) {
        if (polymer[i] == FlipCase(polymer[i + 1])) {
            polymer[i] = '_';
            polymer[i + 1] = '_';
            ++i;
            destroyedUnits += 2;
        }
    }

    if (destroyedUnits > 0) {
        // Strip out the underscores.
        std::string replacementString;
        replacementString.reserve(polymer.size() - destroyedUnits);
        std::copy_if(polymer.begin(), polymer.end(),
                     std::back_inserter(replacementString),
                     [](auto c) { return c != '_'; });
        polymer = std::move(replacementString);
    }

    return destroyedUnits;
}

std::string StripPolymer(const char c, const std::string & polymer)
{
    std::string strippedPolymer;
    strippedPolymer.reserve(polymer.size());
    std::copy_if(polymer.begin(), polymer.end(),
                 std::back_inserter(strippedPolymer),
                 [c](auto c2) { return std::toupper(c2) != c; });
    return strippedPolymer;
}

/// React the polymer repeatedly until its length stops changing.
/// XXX This has worst-case complexity of n^2. I did it this way because I
/// thought I would get average-case performance that's much better, but the
/// clever/devious designer provided input data that's actually close to
/// worst-case. It's slow, but not slow enough for me to redo it.
void FullyReactPolymer(std::string & polymer)
{
    while (DestroyUnits(polymer) > 0) {
        // Do nothing
    }
}

int main(int /*argc*/, char ** /*argv*/)
{
    const auto initialStringSize = 50000;

    const auto originalPolymer = []() {
        std::string polymer;
        polymer.reserve(initialStringSize);
        std::cin >> polymer;
        return polymer;
    }();

    std::string polymer = originalPolymer;
    FullyReactPolymer(polymer);
    std::cout << "Part One polymer length: " << polymer.size() << '\n';

    auto bestResult = std::numeric_limits<std::size_t>::max();
    for (char c = 'A'; c <= 'Z'; c++) {
        std::string strippedPolymer = StripPolymer(c, originalPolymer);
        FullyReactPolymer(strippedPolymer);
        bestResult = std::min(bestResult, strippedPolymer.size());
    }
    std::cout << "Part Two polymer length: " << bestResult << '\n';

    return 0;
}
