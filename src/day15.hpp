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
using Distance = uint32_t;


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

    Coordinates operator+(const Coordinates & other) const
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

vector<Coordinates> GetAdjacentOpenSquares(const Map & map, const State & state,
                                           Coordinates coords)
{
    vector<Coordinates> out;

    for (Coordinates neighbor : GetAdjacentSquares(coords)) {
        auto [x, y] = neighbor;
        if (!map[y][x]) {
            continue;
        }
        if (!Contains(state.entitiesByLocation, neighbor)) {
            out.push_back(neighbor);
        }
    }

    return out;
}

set<Coordinates> GetAllOpenSquares(const Map & map, const State & state)
{
    set<Coordinates> out;
    for (Row y = 0; y < MapSize; y++) {
        for (Column x = 0; x < MapSize; x++) {
            Coordinates coords = {x, y};
            if (map[y][x] && !Contains(state.entitiesByLocation, coords)) {
                out.insert(coords);
            }
        }
    }
    return out;
}

struct BfsResult
{
    std::map<Coordinates, Distance> distances;
    std::map<Coordinates, optional<Coordinates>> predecessors;
};

BfsResult Bfs(const Map & map, const State & state, Coordinates s)
{
    enum class Color
    {
        White,
        Gray,
        Black,
    };

    set<Coordinates> vertexesExcludingSource = GetAllOpenSquares(map, state);
    set<Coordinates> vertexes = vertexesExcludingSource;
    vertexes.insert(s);
    std::map<Coordinates, Color> color;
    std::map<Coordinates, Distance> d;
    std::map<Coordinates, optional<Coordinates>> pi; // Predecessor
    deque<Coordinates> q;

    for (Coordinates u : vertexesExcludingSource) {
        d[u] = std::numeric_limits<Distance>::max();
        pi[u] = {};
    }
    color[s] = Color::Gray;
    d[s] = 0;
    pi[s] = {};
    q.push_back(s);
    while (!q.empty()) {
        Coordinates u = q.front();
        q.pop_front();
        for (Coordinates v : GetAdjacentOpenSquares(map, state, u)) {
            if (color[v] == Color::White) {
                color[v] = Color::Gray;
                d[v] = d[u] + 1;
                pi[v] = u;
                q.push_back(v);
            }
        }
        color[u] = Color::Black;
    }

    BfsResult result;
    result.distances = std::move(d);
    result.predecessors = std::move(pi);
    return result;
}

set<Coordinates> FindSquaresInRangeOfTargets(const Map & map,
                                             const State & state,
                                             const set<Coordinates> & targets)
{
    set<Coordinates> out;
    for (Coordinates target : targets) {
        for (auto neighbor : GetAdjacentOpenSquares(map, state, target)) {
            out.insert(neighbor);
        }
    }
    return out;
}


// Returns either the adjacent square we should move onto to reach the
// nearest target, or {} if there are no paths to targets.
optional<Path> SearchForTarget(const Map & map, const State & state,
                               const Coordinates entity,
                               const set<Coordinates> & targets)
{
    const BfsResult bfsToSource = Bfs(map, state, entity);

    // Set will automatically be ordered in "reading order", so the first of
    // each distance we find will be the priority.
    const set<Coordinates> squaresInRangeOfTargets =
        FindSquaresInRangeOfTargets(map, state, targets);
    if (squaresInRangeOfTargets.empty()) {
        return {};
    }
    Coordinates nearestDest = *squaresInRangeOfTargets.begin();
    Distance shortestDistance = bfsToSource.distances.at(nearestDest);
    for (Coordinates dest : squaresInRangeOfTargets) {
        Distance distance = bfsToSource.distances.at(dest);
        if (distance < shortestDistance) {
            shortestDistance = distance;
            nearestDest = dest;
        }
    }

    // If there was no path, shortestDistance will be "infinity".
    // XXX This is a horrible paradigm. Do something else.

    if (shortestDistance == std::numeric_limits<Distance>::max()) {
        return {};
    }


    BfsResult bfsToTarget = Bfs(map, state, nearestDest);

    // Our current location wasn't part of the BFS, so pick our best neighbor.
    auto openNeighbors = GetAdjacentOpenSquares(map, state, entity);
    Coordinates bestNeighbor = *openNeighbors.begin();
    Distance bestNeighborDistance = bfsToTarget.distances.at(bestNeighbor);
    for (auto neighbor : openNeighbors) {
        auto distance = bfsToTarget.distances.at(neighbor);
        if (distance < bestNeighborDistance) {
            bestNeighborDistance = distance;
            bestNeighbor = neighbor;
        }
    }

    Path out;
    Coordinates next = bestNeighbor;
    out.push_back(next);
    while (next != nearestDest) {
        next = *bfsToTarget.predecessors.at(next);
        out.push_back(next);
    }

    return out;
}

set<Coordinates> GetTargets(const State & state, EntityType targetType)
{
    set<Coordinates> out;
    for (auto & [id, entity] : state.entities) {
        if (entity.type == targetType && entity.hp > 0) {
            out.insert(entity.coords);
        }
    }
    return out;
}

} // namespace day15

#endif // AOC_DAY15_HPP
