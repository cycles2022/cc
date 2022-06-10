// no-networkit-format
#include <iostream>
#include <utility>

#include "networkit/cycles/ChordlessCyclesUno.hpp"
#include "networkit/distance/BFS.hpp"

namespace NetworKit {

using paths = std::vector<node>;

void ChordlessCyclesUno::reset() {
    hasRun = false;
    result.clear();

    G_copy = Graph(*G);
    num_tree_nodes = 0;
    num_leafs = 0;
}

void ChordlessCyclesUno::run() {
    reset();
    findChordlessCycles();
    hasRun = true;
    report();
}

void ChordlessCyclesUno::findChordlessCycles() { // pick an edge u v
    for (auto u : node_order) {
        if (result.size() >= num_max_cyles) {
            break;
        }

        auto current_copy = Graph(G_copy);

        for (auto v : G_copy.neighborRange(u)) {
            current_copy.removeEdge(u, v);
            if (bfsReachable(current_copy, u, v)) {
                chordlessPath(current_copy, u, v);
            }
            current_copy.removeNode(v);
        }

        for (auto w : G_copy.neighborRange(u)) {
            G_copy.removeEdge(u, w);
        }
    }
}

// Find all chordless paths from u to v
void ChordlessCyclesUno::chordlessPath(const Graph &g, node u, node v) {
    std::vector<node> path;
    path.push_back(v);

    auto bfs = NetworKit::BFS(g, u, true, false, v);
    bfs.run();
    auto paths = bfs.getPath(v);
    if (!paths.empty())
        findChordlessPath(g, u, v, path);
}

void ChordlessCyclesUno::findChordlessPath(const Graph &g, node current, node target,
                                           const std::vector<node> &path) {
    if (result.size() >= num_max_cyles) {
        return;
    }
    num_tree_nodes += 1;

    if (current == target) {
        if (callback) {
            callback(path);
        } else {
            result.push_back(path);
        }
        num_leafs += 1;
        return;
    }

    if (g.degree(current) == 0)
        num_leafs += 1;

    // pick one neighbor
    // block all others
    // go deeper
    for (auto v : g.neighborRange(current)) {
        auto current_graph = Graph(g);
        for (auto u : current_graph.neighborRange(current)) {
            if (u != v)
                current_graph.removeNode(u);
        }
        current_graph.removeNode(current);

        if (bfsReachable(current_graph, v, target)) {
            auto new_path(path);
            new_path.push_back(v);
            findChordlessPath(current_graph, v, target, new_path);
        }
    }
}

} // namespace NetworKit
