// Copyright (C) 2018 David Holmes <dholmes@dholmes.us>. All rights reserved.

#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <vector>

int main(int /*argc*/, char ** /*argv*/)
{
    std::vector<std::string> lines;
    std::string line;
    while (std::cin >> line) {
        lines.emplace_back(std::move(line));
    }

    for (auto & id1 : lines) {
        for (auto & id2 : lines) {
            int differingCharacters = 0;
            for (std::size_t i = 0; i < id1.size(); i++) {
                if (id1[i] != id2[i]) {
                    differingCharacters++;
                }
            }
            if (differingCharacters == 1) {
                std::string commonCharacters;
                for (std::size_t i = 0; i < id1.size(); i++) {
                    if (id1[i] == id2[i]) {
                        commonCharacters.push_back(id1[i]);
                    }
                }
                std::cout << "Differs by one character: " << id1 << '\n';
                std::cout << "  Common characters: " << commonCharacters
                          << '\n';

                return 0;
            }
        }
    }

    return 1;
}
