// Copyright (C) 2018 David Holmes <dholmes@dholmes.us>. All rights reserved.

#ifndef AOC_AOC_HPP
#define AOC_AOC_HPP

#include <map>
#include <set>

namespace aoc {

// TODO: Is it possible to remove "set" and "map" and make these truly generic?

template <typename Key, typename Value>
bool contains(const std::map<Key, Value> & m, Key key)
{
    return m.find(key) != m.end();
}

template <typename Key> bool contains(const std::set<Key> & s, Key key)
{
    return s.find(key) != s.end();
}

} // namespace aoc

#endif // AOC_AOC_HPP
