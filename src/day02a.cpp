// Copyright (C) 2018 David Holmes <dholmes@dholmes.us>. All rights reserved.

#include <cstdint>
#include <iostream>
#include <map>
#include <string>

int main(int /*argc*/, char ** /*argv*/)
{
    std::string line;
    std::uint_least16_t containsTwoCount = 0;
    std::uint_least16_t containsThreeCount = 0;
    while (std::cin >> line) {
        std::map<char, std::uint8_t> charCounts;
        for (char c : line) {
            charCounts[c]++;
        }
        bool containsTwo = false;
        bool containsThree = false;
        for (auto &charCountPair : charCounts) {
            containsTwo |= charCountPair.second == 2;
            containsThree |= charCountPair.second == 3;
        }

        if (containsTwo) {
            containsTwoCount++;
        }

        if (containsThree) {
            containsThreeCount++;
        }

        std::cout << line << ": " << containsTwo << " " << containsThree
                  << '\n';

        for (auto [ch, count] : charCounts) {
            if (count >= 2) {
                std::cout << "  " << ch << ": " << static_cast<int>(count)
                          << '\n';
            }
        }
    }

    std::uint_least32_t checksum = containsTwoCount * containsThreeCount;
    std::cout << "Contains Two: " << containsTwoCount << '\n';
    std::cout << "Contains Three: " << containsThreeCount << '\n';
    std::cout << "Checksum: " << checksum << '\n';

    return 0;
}
