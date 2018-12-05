// Copyright (C) 2018 David Holmes <dholmes@dholmes.us>. All rights reserved.

#include <bitset>
#include <cassert>
#include <charconv>
#include <cstdint>
#include <iostream>
#include <regex>
#include <stdexcept>
#include <string>
#include <string_view>

using namespace std::string_literals;

struct Claim
{
public:
    Claim(std::string_view str);

    std::uint16_t claimNumber;
    std::uint16_t x;
    std::uint16_t y;
    std::uint16_t h;
    std::uint16_t w;

    class BadInput : public std::runtime_error
    {

    public:
        BadInput(std::string_view str) : std::runtime_error(std::string(str)) {}
    };
};

std::uint16_t submatchToInt(const std::csub_match & submatch)
{
    std::uint16_t i;
    auto ec = std::from_chars(submatch.first, submatch.second, i).ec;

    // We know it's valid because it matched our regex.
    assert(ec != std::errc::invalid_argument);

    if (ec == std::errc::result_out_of_range) {
        throw Claim::BadInput("Value out of range: \""s +
                              std::string(submatch.first, submatch.second) +
                              '\"');
    }

    return i;
}

Claim::Claim(std::string_view str)
{
    static std::regex claimRegex(
        "\\#(\\d+) \\@ (\\d+),(\\d+)\\: (\\d+)x(\\d+)");
    std::cmatch match;
    if (!std::regex_search(str.begin(), str.end(), match, claimRegex)) {
        throw BadInput("Invalid Claim text: \""s + std::string(str) + '\"');
    }

    claimNumber = submatchToInt(match[1]);
    x = submatchToInt(match[2]);
    y = submatchToInt(match[3]);
    w = submatchToInt(match[4]);
    h = submatchToInt(match[5]);
}

int main(int /*argc*/, char ** /*argv*/)
{
    using Fabric = std::array<std::bitset<1000>, 1000>;
    std::string line;
    Fabric fabric;
    Fabric overlaps;
    std::vector<Claim> claims;

    while (std::getline(std::cin, line)) {
        Claim claim(line);
        claims.push_back(claim);
        for (int x = claim.x; x < claim.x + claim.w; x++) {
            for (int y = claim.y; y < claim.y + claim.h; y++) {
                if (fabric[y][x]) {
                    overlaps[y][x] = true;
                }
                fabric[y][x] = true;
            }
        }
    }

    std::uint32_t overlapSquareInches = 0;
    for (auto & row : overlaps) {
        overlapSquareInches += row.count();
    }

    std::cout << "Overlapping square inches: " << overlapSquareInches << '\n';

    for (Claim & claim : claims) {

        bool thisClaimOverlaps = false;
        for (int x = claim.x; x < claim.x + claim.w; x++) {
            for (int y = claim.y; y < claim.y + claim.h; y++) {
                if (overlaps[y][x]) {
                    thisClaimOverlaps = true;
                }
            }
        }

        if (!thisClaimOverlaps) {
            std::cout << "Claim #" << claim.claimNumber
                      << " does not overlap!\n";
        }
    }

    return 0;
}
