// Copyright (C) 2018 David Holmes <dholmes@dholmes.us>. All rights reserved.

#include <algorithm>
#include <cassert>
#include <charconv>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <map>
#include <regex>
#include <set>
#include <sstream>
#include <string>

using Timestamp = std::string;
using GuardId = std::uint16_t;
using MonthAndDay = std::string;
using Minute = std::uint8_t;
using Asleep = bool;
using AsleepMinutes = std::bitset<60>;

std::ostream & operator<<(std::ostream & stream, const AsleepMinutes & minutes)
{
    for (int i = 0; i < 60; i++) {
        stream << (minutes[i] ? '#' : '.');
    }
    return stream;
}

int main(int /*argc*/, char ** /*argv*/)
{
    static std::regex guardRegex(
        ".................. Guard \\#(\\d+) begins shift");

    std::map<Timestamp, GuardId> shiftStartTimes;
    std::map<Timestamp, Asleep> allSleepWakeTimes;
    std::map<GuardId, std::map<MonthAndDay, AsleepMinutes>> guardAsleepMinutes;
    std::map<GuardId, std::uint16_t> guardTotalSleepMinutes;
    std::set<GuardId> guardIds;

    std::string line;
    while (std::getline(std::cin, line)) {
        Timestamp timestamp = line.substr(1, 16);
        std::smatch match;
        if (std::regex_search(line, match, guardRegex)) {
            std::uint16_t guardId =
                std::stoi(std::string(match[1].first, match[1].second));
            shiftStartTimes[timestamp] = guardId;
            guardIds.insert(guardId);
        } else {
            Asleep asleep = (line.find("falls asleep") != std::string::npos);
            allSleepWakeTimes[timestamp] = asleep;
        }
    }

    for (auto & [timestamp, guardId] : shiftStartTimes) {
        auto fallAsleepIter = allSleepWakeTimes.lower_bound(timestamp);
        auto [fallAsleepTime, asleep] = *fallAsleepIter;
        assert(asleep);

        MonthAndDay date = fallAsleepTime.substr(5, 5);
        std::string minuteTimestampPrefix = fallAsleepTime.substr(0, 14);

        AsleepMinutes & minutes = guardAsleepMinutes[guardId][date];
        for (Minute minute = 0; minute < 60; minute++) {

            std::uint16_t tens = minute / 10;
            std::uint16_t ones = minute % 10;
            std::stringstream thisMinuteTimestampStringstream;
            thisMinuteTimestampStringstream << minuteTimestampPrefix << tens
                                            << ones;
            std::string thisMinuteTimestamp =
                thisMinuteTimestampStringstream.str();
            // Look up whether we were asleep at this minute.
            minutes[minute] =
                (--allSleepWakeTimes.upper_bound(thisMinuteTimestamp))->second;
        }

        guardTotalSleepMinutes[guardId] += minutes.count();
        std::cout << date << " #" << std::setw(4) << guardId << " " << minutes
                  << '\n';
    }

    auto [sleepiestGuard, totalMinutesAsleep] = *std::max_element(
        guardTotalSleepMinutes.begin(), guardTotalSleepMinutes.end(),
        [](const auto & a, const auto & b) { return a.second < b.second; });

    std::cout << "Sleepiest guard: " << sleepiestGuard << " ("
              << totalMinutesAsleep << " minutes)\n";

    // Returns {minute, timesAsleep} pair.
    auto findMinuteMostOftenSleptByGuard = [&](GuardId guardId) {
        std::array<std::uint8_t, 60> nightsMinuteWasSlept = {0};
        for (auto & [date, asleepMinutes] : guardAsleepMinutes[guardId]) {
            for (int i = 0; i < 60; i++) {
                if (asleepMinutes[i]) {
                    nightsMinuteWasSlept[i]++;
                }
            }
        }

        auto search = std::max_element(nightsMinuteWasSlept.begin(),
                                       nightsMinuteWasSlept.end());
        std::uint16_t timesAsleep = *search;
        std::uint16_t minute = search - nightsMinuteWasSlept.begin();

        return std::make_pair(minute, timesAsleep);
    };

    using Result = std::pair<std::uint16_t, std::uint16_t>;
    Result sleepiestGuardResult =
        findMinuteMostOftenSleptByGuard(sleepiestGuard);
    auto [minute, timesAsleep] = sleepiestGuardResult;
    auto multiple = sleepiestGuard * minute;
    std::cout << "Guard " << sleepiestGuard
              << " was most often asleep at minute " << minute << " ("
              << timesAsleep << " times) (multiple=" << multiple << ")\n";

    GuardId finalGuard = sleepiestGuard;
    Result finalResult = sleepiestGuardResult;
    for (GuardId guard : guardIds) {
        Result candidateResult = findMinuteMostOftenSleptByGuard(guard);
        if (candidateResult.second > finalResult.second) {
            finalGuard = guard;
            finalResult = candidateResult;
        }
    }

    auto multiple2 = finalGuard * finalResult.first;
    std::cout << "Guard " << finalGuard << " was most often asleep at minute "
              << finalResult.first << " (" << finalResult.second
              << " times) (multiple=" << multiple2 << ")\n";

    return 0;
}
