// Copyright (C) 2018 David Holmes <dholmes@dholmes.us>. All rights reserved.

#ifndef AOC_DAY15_HPP
#define AOC_DAY15_HPP

#include <array>
#include <bitset>
#include <cstdint>
#include <deque>
#include <iomanip>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <vector>

namespace day15 {

const auto MapSize = 32;

using std::array;
using std::bitset;
using std::deque;
using std::optional;
using std::pair;
using std::set;
using std::string;
using std::string_view;
using std::vector;

using std::int32_t;
using std::uint32_t;

using MapRow = bitset<MapSize>;
using Map = array<MapRow, MapSize>;
using Column = int32_t;
using Row = int32_t;
using EntityId = uint32_t;
using Round = uint32_t;
using HitPoints = int32_t;
using AttackPower = uint32_t;


template <typename Key, typename Value>
bool Contains(const std::map<Key, Value> & m, Key key)
{
    return m.find(key) != m.end();
}

template <typename Key> bool Contains(const set<Key> & s, Key key)
{
    return s.find(key) != s.end();
}

struct Coordinates
{
    Coordinates(Column x, Row y) : x(x), y(y) {}

    Column x;
    Row y;

    bool operator<(const Coordinates & other) const
    {
        return std::tie(y, x) < std::tie(other.y, other.x);
    }
    bool operator>(const Coordinates & other) const { return other < *this; }
    bool operator<=(const Coordinates & other) const
    {
        return !(other < *this);
    }
    bool operator>=(const Coordinates & other) const
    {
        return !(other > *this);
    }

    bool operator==(const Coordinates & other) const
    {
        return std::tie(x, y) == std::tie(other.x, other.y);
    }
    bool operator!=(const Coordinates & other) const
    {
        return !(*this == other);
    }

    Coordinates operator+(const Coordinates & other)
    {
        Coordinates out = other;
        out.x += x;
        out.y += y;
        return out;
    }
};

using Path = vector<Coordinates>;

enum class EntityType
{
    Elf,
    Goblin,
};

// Stream insertion operator for EntityType.
std::ostream & operator<<(std::ostream & stream, const EntityType & type)
{
    stream << (type == EntityType::Elf ? "Elf" : "Goblin");
    return stream;
}

EntityType EnemyType(EntityType type)
{
    if (type == EntityType::Elf) {
        return EntityType::Goblin;
    } else {
        return EntityType::Elf;
    }
}

struct Entity
{
    Entity(EntityId id, EntityType type, Coordinates coords)
        : id(id), type(type), coords(coords), hp(200), attackPower(3),
          status(){};
    EntityId id;
    EntityType type;
    Coordinates coords;
    HitPoints hp;
    AttackPower attackPower;
    string status;
    Path currentPath;
};

struct State
{
    Round round = 1;
    EntityId activeEntity = 0;
    EntityId targetEntity = 0;
    std::map<EntityId, Entity> entities;
    std::map<Coordinates, EntityId> entitiesByLocation;
};

pair<Map, State> ReadInput(std::istream & stream)
{
    Map map;
    State state;
    EntityId nextEntityId = 1;
    string line;
    Row y = 0;
    while (std::getline(stream, line)) {
        if (y > MapSize) {
            std::cerr << "Input file is too long (max = " << MapSize
                      << " lines).\n";
        }
        Column lineSize = line.size();
        if (line.size() > MapSize) {
            std::cerr << "Input line " << y << " is too long (max = " << MapSize
                      << " characters).\n";
            std::exit(1);
        }

        for (Column x = 0; x < lineSize; x++) {
            if (line[x] != '#') {
                map[y][x] = true;
                if (line[x] == 'G') {
                    const auto id = nextEntityId++;
                    const Coordinates coords = {x, y};
                    state.entities.emplace(
                        id, Entity(id, EntityType::Goblin, coords));
                    state.entitiesByLocation.emplace(coords, id);
                } else if (line[x] == 'E') {
                    const auto id = nextEntityId++;
                    const Coordinates coords = {x, y};
                    state.entities.emplace(id,
                                           Entity(id, EntityType::Elf, coords));
                    state.entitiesByLocation.emplace(coords, id);
                }
            }
        }

        y++;
    }

    return {map, state};
}

// Returns adjacent squares in "reading order";
vector<Coordinates> GetAdjacentSquares(Coordinates coords)
{
    vector<Coordinates> out;

    const Coordinates north = {0, -1};
    const Coordinates west = {-1, 0};
    const Coordinates east = {1, 0};
    const Coordinates south = {0, 1};

    const array<Coordinates, 4> neighbors = {north, west, east, south};
    for (auto neighbor : neighbors) {
        out.push_back(neighbor + coords);
    }

    return out;
};

optional<EntityId> SelectAdjacentTarget(const State & state, Coordinates source,
                                        const set<Coordinates> & targets)
{
    optional<EntityId> attackThisTarget = {};
    HitPoints lowestHp = std::numeric_limits<HitPoints>::max();
    for (auto neighbor : GetAdjacentSquares(source)) {
        if (Contains(targets, neighbor)) {
            const EntityId enemyId = state.entitiesByLocation.at(neighbor);
            const Entity & enemy = state.entities.at(enemyId);
            if (enemy.hp < lowestHp) {
                lowestHp = enemy.hp;
                attackThisTarget = enemyId;
            }
        }
    }
    return attackThisTarget;
}

} // namespace day15

#endif // AOC_DAY15_HPP
