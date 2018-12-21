// Copyright (C) 2018 David Holmes <dholmes@dholmes.us>. All rights reserved.

#include "day15.hpp"

#include "ansiterm.hpp"
#include "termios.hpp"

#include <unistd.h> // For STDIN_FILENO

#include <array>
#include <bitset>
#include <cstdint>
#include <fstream>
#include <limits>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <utility>

using namespace day15;

using namespace std::chrono_literals;

using ansi::cursor;
using ansi::cursor_position;
using ansi::graphic;

ansi::cursor_position GetCursor()
{
    auto t = posix::scoped_termios::raw(STDIN_FILENO);

    auto maybeCursor = ansi::get_cursor_position(std::cin, std::cout);
    if (!maybeCursor) {
        std::cerr << "Failed to read cursor position.\n";
        std::exit(1);
    }
    return *maybeCursor;
}

void CheckUsage(int argc, char ** argv)
{
    if (argc != 2) {
        std::cerr << "USAGE: " << argv << " inputFileName.txt\n";
        std::exit(1);
    }
}

pair<Map, State> ReadInput(int argc, char ** argv)
{
    CheckUsage(argc, argv);
    std::ifstream input(argv[1]);
    return ReadInput(input);
}

void DrawMap(std::ostream & stream, ansi::cursor_position pos, const Map & map)
{
    stream << ansi::cup(pos);
    for (Row y = 0; y < MapSize; y++) {
        for (Column x = 0; x < MapSize; x++) {
            stream << (map[y][x] ? '.' : '#');
        }
        stream << '\n';
    }
}

void DrawEntities(std::ostream & stream, ansi::cursor_position areaPos,
                  const State & state)
{
    for (const auto [coords, id] : state.entitiesByLocation) {
        const auto & entity = state.entities.at(id);
        char c = ' ';
        graphic::color3 color = graphic::color3::white;
        if (entity.type == EntityType::Elf) {
            color = graphic::color3::green;
            c = 'E';
        } else {
            color = graphic::color3::red;
            c = 'G';
        }
        if (state.activeEntity == id) {
            stream << graphic::bold();
            // Draw path.
            for (auto pathCoords : entity.currentPath) {
                ansi::cursor_position pathCoordPos = areaPos;
                pathCoordPos.x += pathCoords.x;
                pathCoordPos.y += pathCoords.y;
                stream << ansi::cup(pathCoordPos) << '*';
            }
        }
        if (state.targetEntity == id) {
            stream << graphic::reverse_video();
        }
        ansi::cursor_position entityPos = areaPos;
        entityPos.x += coords.x;
        entityPos.y += coords.y;
        stream << ansi::cup(entityPos) << graphic::fg_color(color) << c
               << ansi::graphic::reset();
    }
}

struct DisplayRectangle
{
    cursor_position topLeft;
    cursor_position dimensions;
};

class Display
{
    friend Display CreateDisplay(std::istream &, std::ostream &, int);

public:
    Display(std::istream & in, std::ostream & out) : in(in), out(out) {}
    std::istream & in;
    std::ostream & out;
    DisplayRectangle screen;
    DisplayRectangle map;
    DisplayRectangle stats;
};

Display CreateDisplay(std::istream & in, std::ostream & out, int rows)
{
    Display disp(in, out);
    for (int i = 0; i < rows; i++) {
        out << '\n';
    }
    out << cursor(cursor::direction::up, rows) << std::flush;
    cursor_position cursor = GetCursor();
    cursor_position dimensions = {MapSize + 40, MapSize + 1};
    disp.screen = {cursor, dimensions};
    disp.map = {cursor + cursor_position(0, 1),
                cursor_position(MapSize, MapSize)};
    disp.stats = {cursor + cursor_position(MapSize, 0),
                  cursor_position(40, MapSize + 1)};
    return disp;
}

void DrawText(std::ostream & stream, ansi::cursor_position pos,
              string_view text)
{
    stream << ansi::cup(pos) << text;
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

using Distance = uint32_t;

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

optional<Path> GetPathFromBfs(BfsResult & bfsResult, Coordinates source,
                              Coordinates target)
{
    if (!bfsResult.predecessors.at(target)) {
        return {};
    }
    Path path;
    while (target != source) {
        path.push_back(target);
        target = *bfsResult.predecessors.at(target);
    }
    std::reverse(path.begin(), path.end());
    return path;
}

// Returns either the adjacent square we should move onto to reach the nearest
// target, or {} if there are no paths to targets.
optional<Path> SearchForTarget(const Map & map, const State & state,
                               Coordinates source,
                               const set<Coordinates> & targets)
{
    BfsResult bfsResult = Bfs(map, state, source);

    // Set will automatically be ordered in "reading order", so the first of
    // each distance we find will be the priority.
    set<Coordinates> squaresInRangeOfTargets =
        FindSquaresInRangeOfTargets(map, state, targets);
    if (squaresInRangeOfTargets.empty()) {
        return {};
    }
    Coordinates nearestDest = *squaresInRangeOfTargets.begin();
    Distance shortestDistance = bfsResult.distances[nearestDest];
    for (Coordinates dest : squaresInRangeOfTargets) {
        Distance distance = bfsResult.distances[dest];
        if (distance < shortestDistance) {
            shortestDistance = distance;
            nearestDest = dest;
        }
    }

    return GetPathFromBfs(bfsResult, source, nearestDest);
}

void DrawAllEntityStats(std::ostream & stream, cursor_position pos,
                        const State & state)
{
    using std::setw;
    cursor_position rowStart = pos;
    for (auto [id, entity] : state.entities) {
        stream << ansi::cup(rowStart);
        rowStart += {0, 1};
        if (id == state.activeEntity) {
            stream << graphic::bold() << graphic::underline();
        }
        if (entity.hp <= 0) {
            stream << graphic::crossed_out();
        }
        stream << setw(2) << id << setw(8) << entity.type << setw(4)
               << entity.hp << ' ' << entity.status << graphic::reset();
    }
}

void DrawEverything(const Map & map, State & state, Display & disp)
{
    std::stringstream ss;
    ss << "Round: " << state.round;
    DrawMap(std::cout, disp.map.topLeft, map);
    DrawText(std::cout, disp.screen.topLeft, ss.str());
    DrawEntities(std::cout, disp.map.topLeft, state);
    DrawAllEntityStats(std::cout, disp.stats.topLeft, state);
    std::cout << std::flush;
}

void TakeTurns(const Map & map, State & state, Display & disp)
{
    auto beginningOfTurnLocations = state.entitiesByLocation;
    for (auto & [coord, id] : beginningOfTurnLocations) {
        state.activeEntity = id;
        auto & entity = state.entities.at(id);

        set<Coordinates> targets = GetTargets(state, EnemyType(entity.type));

        auto attackThisTarget = SelectAdjacentTarget(state, coord, targets);

        if (!attackThisTarget) {

            // Search for the nearest target.
            auto maybePath = SearchForTarget(map, state, coord, targets);
            if (!maybePath) {
                // Don't move this turn.
                entity.status = "Not Moving";
            } else {
                entity.currentPath = *maybePath;

                DrawEverything(map, state, disp);
                std::this_thread::sleep_for(50ms);

                auto move = maybePath->at(0);
                // Move to an adjacent tile.
                entity.coords = move;
                state.entitiesByLocation.erase(coord);
                state.entitiesByLocation.insert({move, id});
                entity.status = "Moving";

                attackThisTarget =
                    SelectAdjacentTarget(state, entity.coords, targets);
            }
        }

        if (attackThisTarget) {
            // Attach the target.
            entity.status = "Attacking";
            auto & enemy = state.entities.at(*attackThisTarget);
            state.targetEntity = enemy.id;
            enemy.status = "Under Attack";
            enemy.hp -= entity.attackPower;
            if (enemy.hp <= 0) {
                enemy.hp = 0;
                enemy.status = "Dead";
                state.entitiesByLocation.erase(enemy.coords);
            }
        }
        entity.currentPath = {};

        DrawEverything(map, state, disp);
        std::this_thread::sleep_for(50ms);
        entity.status = "";
        state.targetEntity = 0;
    }
    state.activeEntity = 0;
}

int CountEntityHitPoints(const State & state, EntityType type)
{
    int out = 0;
    for (auto & [id, entity] : state.entities) {
        if (entity.type == type) {
            out += entity.hp;
        }
    }
    return out;
}

int main(int argc, char ** argv)
{
    auto [map, state] = ReadInput(argc, argv);

    Display disp = CreateDisplay(std::cin, std::cout, map.size() + 2);
    cursor_position displayEnd =
        disp.screen.topLeft + cursor_position(0, disp.screen.dimensions.y);

    DrawMap(std::cout, disp.map.topLeft, map);

    bool done = false;
    while (!done) {
        TakeTurns(map, state, disp);

        DrawEverything(map, state, disp);
        std::this_thread::sleep_for(200ms);

        int elfHp = CountEntityHitPoints(state, EntityType::Elf);
        int goblinHp = CountEntityHitPoints(state, EntityType::Goblin);
        if (elfHp == 0) {
            done = true;
            std::cout << "Goblins win! Round=" << state.round
                      << ", HP=" << goblinHp
                      << ", Outcome=" << (state.round * goblinHp) << '\n';
        } else if (goblinHp == 0) {
            done = true;
            std::cout << "Elves win! Round=" << state.round << ", HP=" << elfHp
                      << ", Outcome=" << (state.round * elfHp) << '\n';
        } else {
            state.round++;
        }
    }
    std::cout << ansi::cup(displayEnd) << '\n';

    return 0;
}
