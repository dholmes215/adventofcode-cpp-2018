// Copyright (C) 2018 David Holmes <dholmes@dholmes.us>. All rights reserved.

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <regex>
#include <string>
#include <string_view>
#include <utility>

using PlantIndex = std::int64_t;
using Generation = std::uint64_t;
using FivePlants = std::uint8_t;

const std::int32_t maxPlants = 1024;
const PlantIndex plantZeroIndex = maxPlants / 2;

using PlantBitset = std::bitset<maxPlants>;

const auto lastGeneration = 200;

class PlantCollection
{
public:
    static const PlantIndex min = plantZeroIndex - maxPlants;
    static const PlantIndex max = maxPlants - plantZeroIndex;

    bool Get(PlantIndex pos) const { return plants[pos + plantZeroIndex]; }
    void Set(PlantIndex pos, bool val) { plants[pos + plantZeroIndex] = val; }
    FivePlants GetFivePlants(PlantIndex middle) const
    {
        FivePlants out = 0;
        for (PlantIndex i = middle - 2; i <= middle + 2; i++) {
            out <<= 1;
            out |= Get(i);
        }
        return out;
    }

    PlantIndex FindFirst() const
    {
        for (PlantIndex i = min; i < max; i++) {
            if (Get(i)) {
                return i;
            }
        }
        return max;
    }

    std::pair<PlantBitset, PlantIndex> GetBits() const
    {
        auto firstPlant = FindFirst();
        auto firstBit = firstPlant + plantZeroIndex;
        auto shifted = plants >> firstBit;
        return {shifted, firstPlant};
    }

private:
    PlantBitset plants;
};

void WritePlantCollectionRange(std::ostream & stream,
                               const PlantCollection & plants, PlantIndex first,
                               PlantIndex last)
{
    for (auto i = first; i <= last; i++) {
        stream << (plants.Get(i) ? '#' : '.');
    }
}

using Note = std::pair<FivePlants, bool>;
using NoteSet = std::bitset<32>;

PlantCollection ParseInitialState(std::string_view str)
{
    static const std::regex initialStateRegex("initial state: ([\\.\\#]+)");
    std::cmatch match;
    std::regex_search(str.begin(), str.end(), match, initialStateRegex);
    auto len = match[1].second - match[1].first;
    std::string_view stateStr(match[1].first, len);

    PlantCollection initialState;
    for (int i = 0; i < len; i++) {
        initialState.Set(i, stateStr[i] == '#');
    }
    return initialState;
}

PlantCollection NewGeneration(const PlantCollection & parent, NoteSet notes)
{
    PlantCollection out;
    for (auto i = PlantCollection::min + 2; i <= PlantCollection::max - 2;
         i++) {
        out.Set(i, notes[parent.GetFivePlants(i)]);
    }
    return out;
}

void PrintSpaces(std::ostream & stream, std::uint8_t count)
{
    for (int i = 0; i < count; i++) {
        stream << ' ';
    }
}

void PrintColumnNumbers(std::ostream & stream, PlantIndex first,
                        PlantIndex last, std::uint8_t offset)
{
    std::string firstString = std::to_string(first);
    std::string lastString = std::to_string(last);
    int rowCount = std::max(firstString.length(), lastString.length());

    for (int digit = rowCount; digit > 0; digit--) {
        PrintSpaces(stream, offset);
        for (PlantIndex column = first; column <= last; column++) {
            if (column % 10 != 0) {
                stream << ' ';
            } else {
                std::string colString = std::to_string(column);
                int offset = colString.length() - digit;
                char ch = (offset < 0) ? ' ' : colString[offset];
                stream << ch;
            }
        }
        stream << '\n';
    }
}

void PrintGeneration(std::ostream & stream, Generation generation,
                     std::uint8_t genColumnWidth,
                     const PlantCollection & plants, PlantIndex first,
                     PlantIndex last)
{
    stream << std::setw(genColumnWidth - 2) << generation << ": ";
    WritePlantCollectionRange(stream, plants, first, last);
    std::cout << '\n';
}

Note ParseNote(std::string_view str)
{
    static const std::regex noteRegex("([\\.\\#]{5})\\s*=>\\s*([\\.\\#])");
    std::cmatch match;
    std::regex_search(str.begin(), str.end(), match, noteRegex);

    auto len = match[1].second - match[1].first;
    std::string_view inputStr(match[1].first, len);
    std::uint8_t input = 0;
    for (char c : inputStr) {
        input <<= 1;
        input |= c == '#';
    }

    bool output = *match[2].first == '#';

    return {input, output};
}

// Stream insertion operator for Note
std::ostream & operator<<(std::ostream & stream, const Note & note)
{
    auto [input, output] = note;
    for (int i = 0; i < 5; i++) {
        bool bit = (input >> (4 - i)) & 1;
        stream << (bit ? '#' : '.');
    }
    stream << " >> " << (output ? '#' : '.');
    return stream;
}

void PrintNoteSet(std::ostream & stream, const NoteSet & noteSet)
{
    for (std::uint8_t input = 0; input < 32; input++) {
        Note note = {input, noteSet[input]};
        stream << note << '\n';
    }
}

std::int64_t PlantNumberSum(const PlantCollection & plants)
{
    std::int64_t sum = 0;
    for (PlantIndex i = PlantCollection::min; i < PlantCollection::max; i++) {
        if (plants.Get(i)) {
            sum += i;
        }
    }
    return sum;
}

std::int64_t PlantNumberSum(const PlantBitset & bitset, PlantIndex first)
{
    std::int64_t sum = 0;
    for (std::int64_t i = 0; i < static_cast<std::int64_t>(bitset.size());
         i++) {
        PlantIndex plantNumber = i + first;
        if (bitset[i]) {
            sum += plantNumber;
        }
    }
    return sum;
}

void PartTwo(std::array<PlantCollection, lastGeneration + 1> & plantGenerations)
{

    std::array<std::pair<PlantBitset, PlantIndex>, lastGeneration + 1>
        plantGenerationBitsets;

    for (Generation gen = 0; gen <= lastGeneration; gen++) {
        plantGenerationBitsets[gen] = plantGenerations[gen].GetBits();
    }

    // Find the first repeat.
    auto repeatStartGen = -1;
    // auto lastOriginalGen = -1;
    auto repeatFactor = -1;
    auto repeatShift = 0;
    for (Generation gen = 1; gen <= lastGeneration; gen++) {
        auto & [bits, first] = plantGenerationBitsets[gen];
        for (Generation prevGen = 0; prevGen < gen; prevGen++) {
            auto & [prevBits, prevFirst] = plantGenerationBitsets[prevGen];
            if (bits == prevBits) {
                std::cout << "Generation " << gen << " is a repeat of "
                          << prevGen << ".\n";
                repeatStartGen = prevGen;
                repeatFactor = gen - prevGen;
                repeatShift = first - prevFirst;

                auto targetGen = 50000000000LL;
                auto targetGenAdjusted = targetGen - repeatStartGen;
                auto repeatCount = targetGenAdjusted / repeatFactor;
                auto targetShift = repeatShift * repeatCount;

                std::cout << "Sum of plant numbers at generation #" << targetGen
                          << ": "
                          << PlantNumberSum(bits, first + targetShift - 1)
                          << '\n';

                return;
            }
        }
    }

    std::cout << "No repeats found!\n";
}

int main(int /*argc*/, char ** /*argv*/)
{
    std::string line;
    std::getline(std::cin, line);
    const PlantCollection initialState = ParseInitialState(line);

    const NoteSet noteSet = []() {
        NoteSet out;
        std::string line;
        while (std::getline(std::cin, line)) {
            if (line.empty()) {
                continue;
            }
            Note note = ParseNote(line);
            auto [input, output] = note;
            out[input] = output;
        }
        return out;
    }();

    std::cout << "Notes:\n";
    PrintNoteSet(std::cout, noteSet);

    std::array<PlantCollection, lastGeneration + 1> plantGenerations;
    plantGenerations[0] = initialState;
    for (Generation gen = 1; gen <= lastGeneration; gen++) {
        plantGenerations[gen] =
            NewGeneration(plantGenerations[gen - 1], noteSet);
    }

    PrintColumnNumbers(std::cout, -20, 140, 5);
    for (Generation gen = 0; gen <= lastGeneration; gen++) {
        PrintGeneration(std::cout, gen, 5, plantGenerations[gen], -20, 140);
    }

    std::cout << "Sum of plant numbers at generation #20: "
              << PlantNumberSum(plantGenerations[20]) << '\n';

    PartTwo(plantGenerations);

    return 0;
}
