// Copyright (C) 2018 David Holmes <dholmes@dholmes.us>. All rights reserved.

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <map>
#include <regex>
#include <set>
#include <string>
#include <utility>

using Node = char;
const Node noTask = 0;
using Edge = std::pair<char, char>;
using Second = std::uint32_t;
using WorkerId = std::uint8_t;

Edge StrToEdge(std::string_view str)
{
    static std::regex edgeRegex(
        "Step (.) must be finished before step (.) can begin");

    std::cmatch match;
    std::regex_search(str.begin(), str.end(), match, edgeRegex);
    return std::make_pair(*match[1].first, *match[2].first);
}

std::vector<Edge> ReadEdgeList(std::istream & stream)
{
    std::vector<Edge> edgeList;
    std::string line;
    while (std::getline(stream, line)) {
        edgeList.push_back(StrToEdge(line));
    }
    std::sort(edgeList.begin(), edgeList.end());
    return edgeList;
}

std::vector<Node> CreateNodeList(const std::vector<Edge> & edgeList)
{
    std::vector<Node> nodeList;
    for (auto [a, b] : edgeList) {
        nodeList.push_back(a);
        nodeList.push_back(b);
    }
    std::sort(nodeList.begin(), nodeList.end());
    nodeList.erase(std::unique(nodeList.begin(), nodeList.end()),
                   nodeList.end());
    return nodeList;
}

template <typename KeyType, typename ValueType>
std::vector<ValueType>
MultimapGetValues(const std::multimap<KeyType, ValueType> & multimap,
                  KeyType & key)
{
    std::vector<ValueType> out;
    const auto lower = multimap.lower_bound(key);
    const auto upper = multimap.upper_bound(key);
    for (auto iter = lower; iter != upper; iter++) {
        out.push_back(iter->second);
    }
    return out;
}

bool AreDependenciesDone(const std::multimap<Node, Node> & dependencies,
                         const std::set<Node> & completedNodes, Node key)
{
    auto keyDependencies = MultimapGetValues(dependencies, key);
    for (auto dep : keyDependencies) {
        if (completedNodes.find(dep) == completedNodes.end()) {
            return false;
        }
    }
    return true;
}

Node SelectNextNode(const std::vector<Node> & nodeList,
                    const std::multimap<Node, Node> & dependencies,
                    const std::set<Node> & availableTasks,
                    const std::set<Node> & completedTasks)
{
    assert(!availableTasks.empty());
    assert(completedTasks.size() != nodeList.size());

    for (Node node : nodeList) {
        bool available = availableTasks.find(node) != availableTasks.end();
        bool depsDone = AreDependenciesDone(dependencies, completedTasks, node);
        if (available && depsDone) {
            return node;
        }
    }

    return noTask;
}

std::uint8_t TaskDuration(Node node) { return node - 'A' + 61; }

template <typename WorkerArray>
void LogSecond(std::ostream & stream, Second second, WorkerArray & workers,
               std::map<WorkerId, Node> & currentTasks,
               std::string_view taskSequence)
{
    stream << std::setw(8) << second;
    for (WorkerId worker : workers) {
        Node task = currentTasks[worker];
        char ch = task;
        if (task == noTask) {
            ch = '.';
        }
        stream << std::setw(8) << ch;
    }
    stream << std::setw(8) << taskSequence << '\n';
}

void SimulateWork(const std::vector<Node> & taskList,
                  const std::multimap<Node, Node> & dependencies)
{

    std::string taskSequence;
    std::set<Node> completedTasks;
    std::set<Node> availableTasks(taskList.begin(), taskList.end());

    const std::array<WorkerId, 5> workers = {1, 2, 3, 4, 5};
    std::map<WorkerId, Node> currentTasks;
    std::map<Node, Second> workDone;
    Second second = 0;
    bool done = false;
    LogSecond(std::cout, second, workers, currentTasks, taskSequence);
    while (!done) {
        // Beginning of the second.
        for (WorkerId worker : workers) {
            if (currentTasks[worker] == noTask && !availableTasks.empty()) {
                Node task = SelectNextNode(taskList, dependencies,
                                           availableTasks, completedTasks);
                if (task != noTask) {
                    currentTasks[worker] = task;
                    availableTasks.erase(task);
                }
            }
        }

        // Do the work.
        for (WorkerId worker : workers) {
            Node task = currentTasks[worker];
            if (task != noTask) {
                workDone[task]++;
            }
        }

        // End of the second.
        for (WorkerId worker : workers) {
            Node task = currentTasks[worker];
            if (task != noTask && workDone[task] == TaskDuration(task)) {
                completedTasks.insert(task);
                currentTasks[worker] = noTask;
                taskSequence.push_back(task);
            }
        }
        ++second;

        LogSecond(std::cout, second, workers, currentTasks, taskSequence);
        done = completedTasks.size() == taskList.size();
    }
}

int main(int /*argc*/, char ** /*argv*/)
{
    const std::vector<Edge> edgeList = ReadEdgeList(std::cin);
    const std::multimap<Node, Node> dependents(edgeList.begin(),
                                               edgeList.end());
    const std::multimap<Node, Node> dependencies = [&dependents]() {
        std::multimap<Node, Node> out;
        for (auto [a, b] : dependents) {
            out.insert({b, a});
        }
        return out;
    }();
    const auto nodeList = CreateNodeList(edgeList);

    // std::cout << "digraph {\n";
    // for (auto edge : edgeList) {
    //     std::cout << "    " << edge.first << " -> " << edge.second << ";\n";
    // }
    // std::cout << "}\n";

    std::string nodeSequence;
    std::set<Node> availableTasks(nodeList.begin(), nodeList.end());
    std::set<Node> completedTasks;

    while (availableTasks.size() > 0) {
        auto node = SelectNextNode(nodeList, dependencies, availableTasks,
                                   completedTasks);
        assert(node != noTask);
        nodeSequence.push_back(node);
        availableTasks.erase(node);
        completedTasks.insert(node);
    }

    std::cout << "Sequence: " << nodeSequence << std::endl;

    // Part 2
    SimulateWork(nodeList, dependencies);

    return 0;
}
