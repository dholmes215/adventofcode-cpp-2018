// Copyright (C) 2018 David Holmes <dholmes@dholmes.us>. All rights reserved.

#include <cstdint>
#include <iostream>

int main(int /*argc*/, char ** /*argv*/)
{
    std::uint_least32_t frequency = 0;
    std::uint_least32_t frequencyChange = 0;
    while (std::cin >> frequencyChange) {
        frequency += frequencyChange;
    }

    std::cout << "Resulting Frequency: " << frequency << '\n';

    return 0;
}
