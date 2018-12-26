// Copyright (C) 2018 David Holmes <dholmes@dholmes.us>. All rights reserved.

#include "aoc.hpp"

#include <algorithm>
#include <array>
#include <charconv>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <regex>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

using std::array;
using std::map;
using std::optional;
using std::pair;
using std::set;
using std::string;
using std::string_view;
using std::uint8_t;
using std::vector;

using Register = std::uint8_t;

enum Opcode : uint8_t
{
    addr, /// add register
    addi, /// add immediate
    mulr, /// multiply register
    muli, /// multiply immediate
    banr, /// bitwise AND register
    bani, /// bitwise AND immediate
    borr, /// bitwise OR register
    bori, /// bitwise OR immediate
    setr, /// set register
    seti, /// set immediate
    gtir, /// greater-than immediate/register
    gtri, /// greater-than register/immediate
    gtrr, /// greater-than register/register
    eqir, /// equal immediate/register
    eqri, /// equal register/immediate
    eqrr, /// equal register/register
};

const array<Opcode, 16> allOpcodes = {addr, addi, mulr, muli, banr, bani,
                                      borr, bori, setr, seti, gtir, gtri,
                                      gtrr, eqir, eqri, eqrr};

const array<const char * const, 16> opcodeStr = {
    "addr", "addi", "mulr", "muli", "banr", "bani", "borr", "bori",
    "setr", "seti", "gtir", "gtri", "gtrr", "eqir", "eqri", "eqrr"};

struct Cpu
{
    /// Registers.
    array<Register, 4> r;

    bool operator==(const Cpu & that) const { return r == that.r; }
    bool operator!=(const Cpu & that) const { return r != that.r; }
};

struct Instruction
{
    uint8_t opcode;
    uint8_t a; /// Input A
    uint8_t b; /// Input B
    uint8_t c; /// Output C
};

struct Sample
{
    Cpu before;
    Cpu after;
    Instruction instruction;
};

Cpu AddRInst(const Cpu before, const Instruction inst)
{
    Cpu after = before;
    auto [o, a, b, c] = inst;
    after.r[c] = after.r[a] + after.r[b];
    return after;
}

Cpu AddIInst(const Cpu before, const Instruction inst)
{
    Cpu after = before;
    auto [o, a, b, c] = inst;
    after.r[c] = after.r[a] + b;
    return after;
}

Cpu MulRInst(const Cpu before, const Instruction inst)
{
    Cpu after = before;
    auto [o, a, b, c] = inst;
    after.r[c] = after.r[a] * after.r[b];
    return after;
}

Cpu MulIInst(const Cpu before, const Instruction inst)
{
    Cpu after = before;
    auto [o, a, b, c] = inst;
    after.r[c] = after.r[a] * b;
    return after;
}

Cpu BanRInst(const Cpu before, const Instruction inst)
{
    Cpu after = before;
    auto [o, a, b, c] = inst;
    after.r[c] = after.r[a] & after.r[b];
    return after;
}

Cpu BanIInst(const Cpu before, const Instruction inst)
{
    Cpu after = before;
    auto [o, a, b, c] = inst;
    after.r[c] = after.r[a] & b;
    return after;
}

Cpu BorRInst(const Cpu before, const Instruction inst)
{
    Cpu after = before;
    auto [o, a, b, c] = inst;
    after.r[c] = after.r[a] | after.r[b];
    return after;
}

Cpu BorIInst(const Cpu before, const Instruction inst)
{
    Cpu after = before;
    auto [o, a, b, c] = inst;
    after.r[c] = after.r[a] | b;
    return after;
}

Cpu SetRInst(const Cpu before, const Instruction inst)
{
    Cpu after = before;
    auto [o, a, b, c] = inst;
    after.r[c] = after.r[a];
    return after;
}

Cpu SetIInst(const Cpu before, const Instruction inst)
{
    Cpu after = before;
    auto [o, a, b, c] = inst;
    after.r[c] = a;
    return after;
}

Cpu GtIRInst(const Cpu before, const Instruction inst)
{
    Cpu after = before;
    auto [o, a, b, c] = inst;
    after.r[c] = a > after.r[b];
    return after;
}

Cpu GtRIInst(const Cpu before, const Instruction inst)
{
    Cpu after = before;
    auto [o, a, b, c] = inst;
    after.r[c] = after.r[a] > b;
    return after;
}

Cpu GtRRInst(const Cpu before, const Instruction inst)
{
    Cpu after = before;
    auto [o, a, b, c] = inst;
    after.r[c] = after.r[a] > after.r[b];
    return after;
}

Cpu EqIRInst(const Cpu before, const Instruction inst)
{
    Cpu after = before;
    auto [o, a, b, c] = inst;
    after.r[c] = (a == after.r[b]);
    return after;
}

Cpu EqRIInst(const Cpu before, const Instruction inst)
{
    Cpu after = before;
    auto [o, a, b, c] = inst;
    after.r[c] = (after.r[a] == b);
    return after;
}

Cpu EqRRInst(const Cpu before, const Instruction inst)
{
    Cpu after = before;
    auto [o, a, b, c] = inst;
    after.r[c] = (after.r[a] == after.r[b]);
    return after;
}

typedef Cpu (*InstFunc)(const Cpu, const Instruction inst);

InstFunc OpcodeToFunc(const Opcode opcode)
{
    // Since we don't yet know the real numeric values of opcodes, the indexes
    // of this array are the opcodes ordered in the enumeration;
    static const array<InstFunc, 16> instFuncs = {
        AddRInst, AddIInst, MulRInst, MulIInst, BanRInst, BanIInst,
        BorRInst, BorIInst, SetRInst, SetIInst, GtIRInst, GtRIInst,
        GtRRInst, EqIRInst, EqRIInst, EqRRInst};

    return instFuncs[static_cast<size_t>(opcode)];
}

using Program = vector<Instruction>;

void CheckUsage(int argc, char ** argv)
{
    if (argc != 2) {
        std::cerr << "USAGE: " << argv << " inputFileName.txt\n";
        std::exit(1);
    }
}

optional<Cpu> ParseCpuState(string_view line)
{
    static const std::regex cpuStateRegex(
        ".+\\[(\\d+), (\\d+), (\\d+), (\\d+)\\]");

    std::cmatch match;
    if (!std::regex_match(line.begin(), line.end(), match, cpuStateRegex)) {
        return {};
    }

    Cpu cpu;
    std::from_chars(match[1].first, match[1].second, cpu.r[0]);
    std::from_chars(match[2].first, match[2].second, cpu.r[1]);
    std::from_chars(match[3].first, match[3].second, cpu.r[2]);
    std::from_chars(match[4].first, match[4].second, cpu.r[3]);
    return cpu;
}

optional<Instruction> ParseInstruction(string_view line)
{
    static const std::regex instructionRegex("(\\d+) (\\d+) (\\d+) (\\d+)");

    std::cmatch match;
    if (!std::regex_match(line.begin(), line.end(), match, instructionRegex)) {
        return {};
    }

    Instruction i;
    std::from_chars(match[1].first, match[1].second, i.opcode);
    std::from_chars(match[2].first, match[2].second, i.a);
    std::from_chars(match[3].first, match[3].second, i.b);
    std::from_chars(match[4].first, match[4].second, i.c);
    return i;
}

pair<vector<Sample>, Program> ReadInput(std::istream & stream)
{
    vector<Sample> samples;
    Program program;

    string line;
    while (std::getline(stream, line)) {
        if (line.empty()) {
            continue;
        }

        // Try to read a sample.
        auto maybeCpu = ParseCpuState(line);
        if (!maybeCpu) {
            // This isn't a sample; move on to the program.
            break;
        }

        Sample sample;
        sample.before = *maybeCpu;

        if (!std::getline(stream, line)) {
            std::cerr << "Unexpected end of file parsing sample.\n";
            std::exit(1);
        }

        auto maybeInstruction = ParseInstruction(line);
        if (!maybeInstruction) {
            std::cerr << "Unexpected line while parsing sample: " << line
                      << '\n';
            std::exit(1);
        }

        sample.instruction = *maybeInstruction;

        if (!std::getline(stream, line)) {
            std::cerr << "Unexpected end of file parsing sample.\n";
            std::exit(1);
        }

        maybeCpu = ParseCpuState(line);
        if (!maybeCpu) {
            std::cerr << "Missing 'after' state parsing sample: " << line
                      << '\n';
            std::exit(1);
        }

        sample.after = *maybeCpu;

        samples.push_back(sample);
    }

    do {
        if (line.empty()) {
            continue;
        }

        auto maybeInstruction = ParseInstruction(line);
        if (!maybeInstruction) {
            // Unexpectedly found a non-empty non-instruction line.
            std::cerr << "Unexpected line: " << line << '\n';
            std::exit(1);
        }

        program.push_back(*maybeInstruction);
    } while (std::getline(stream, line));

    return {samples, program};
}

pair<vector<Sample>, Program> ReadInput(int argc, char ** argv)
{
    CheckUsage(argc, argv);
    std::ifstream input(argv[1]);
    return ReadInput(input);
}

using OpcodeValue = uint8_t;
using CandidateValues = set<OpcodeValue>;

std::ostream & operator<<(std::ostream & stream, const CandidateValues & values)
{
    for (auto val : values) {
        stream << ' ' << static_cast<int>(val);
    }
    return stream;
}

std::ostream & operator<<(std::ostream & stream, const Cpu & cpu)
{
    stream << static_cast<int>(cpu.r[0]) << ' ' << static_cast<int>(cpu.r[1])
           << ' ' << static_cast<int>(cpu.r[2]) << ' '
           << static_cast<int>(cpu.r[3]) << '\n';
    return stream;
}
void PrintInstruction(std::ostream & stream, const Opcode actualOpcode,
                      const Instruction & inst)
{
    stream << opcodeStr[actualOpcode] << ' ' << static_cast<int>(inst.a) << ' '
           << static_cast<int>(inst.b) << ' ' << static_cast<int>(inst.c)
           << '\n';
}

int main(int argc, char ** argv)
{
    const auto [samples, program] = ReadInput(argc, argv);

    // Part 1
    uint32_t samplesMatchingThreeOrMoreOpcodes = 0;
    for (const auto sample : samples) {
        set<Opcode> matchingOpcodes;
        // Test each opcode.
        for (const Opcode op : allOpcodes) {
            if (OpcodeToFunc(op)(sample.before, sample.instruction) ==
                sample.after) {
                matchingOpcodes.insert(op);
            }
        }
        if (matchingOpcodes.size() >= 3) {
            samplesMatchingThreeOrMoreOpcodes++;
        }
    }

    std::cout << "Samples matching three or more opcodes: "
              << samplesMatchingThreeOrMoreOpcodes << '\n';

    // Part 2
    CandidateValues allValuesSet(allOpcodes.begin(), allOpcodes.end());
    map<Opcode, CandidateValues> candidateValues;
    for (const Opcode op : allOpcodes) {
        candidateValues.insert({op, allValuesSet});
    }

    for (Opcode op : allOpcodes) {
        std::cout << opcodeStr[op] << ": " << candidateValues.at(op) << '\n';
    }

    for (const auto sample : samples) {
        // Test each opcode.
        for (const Opcode opToTestAgainstSample : allOpcodes) {
            auto & thisOpCandidates = candidateValues[opToTestAgainstSample];
            if (OpcodeToFunc(opToTestAgainstSample)(
                    sample.before, sample.instruction) != sample.after) {
                thisOpCandidates.erase(sample.instruction.opcode);
            }
        }
    }

    for (Opcode op : allOpcodes) {
        std::cout << opcodeStr[op] << ": " << candidateValues.at(op) << '\n';
    }

    map<Opcode, OpcodeValue> opcodeValues;
    set<OpcodeValue> identifiedValues;

    for (int i = 0; i < 16; i++) {
        for (Opcode op : allOpcodes) {
            if (candidateValues.at(op).size() == 1) {
                auto val = *candidateValues.at(op).begin();
                identifiedValues.insert(val);
                opcodeValues[op] = val;
            }
        }

        for (Opcode op : allOpcodes) {
            auto & thisOpCandidateValues = candidateValues.at(op);
            for (OpcodeValue val : identifiedValues) {
                thisOpCandidateValues.erase(val);
            }
        }
    }

    for (Opcode op : allOpcodes) {
        std::cout << opcodeStr[op] << ": " << static_cast<int>(opcodeValues[op])
                  << '\n';
    }

    std::array<Opcode, 16> opcodesByValue;

    for (auto [op, val] : opcodeValues) {
        opcodesByValue[val] = op;
    }

    Cpu cpu = {0};
    std::cout << cpu << '\n';
    for (Instruction inst : program) {
        Opcode op = opcodesByValue[inst.opcode];
        PrintInstruction(std::cout, op, inst);
        cpu = OpcodeToFunc(op)(cpu, inst);
        std::cout << cpu << '\n';
    }

    std::cout << "Register 0 after running program: "
              << static_cast<int>(cpu.r[0]) << '\n';

    return 0;
}
