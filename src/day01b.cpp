// Copyright (C) 2018 David Holmes <dholmes@dholmes.us>. All rights reserved.

#include <cstdint>
#include <iostream>
#include <set>
#include <vector>

int main(int /*argc*/, char ** /*argv*/)
{
    std::uint_least32_t currentFrequencyChange = 0;
    std::vector<std::uint_least32_t> frequencyChangeList;
    while (std::cin >> currentFrequencyChange) {
        frequencyChangeList.push_back(currentFrequencyChange);
    }

    std::set<std::uint_least32_t> frequenciesToDate;
    std::uint_least32_t frequency = 0;
    while (true) {
        for (auto frequencyChange : frequencyChangeList) {
            frequency += frequencyChange;
            bool inserted = frequenciesToDate.insert(frequency).second;
            if (!inserted) {
                std::cout << "Repeated Frequency: " << frequency << '\n';
                return 0;
            }
        }
    }
}
