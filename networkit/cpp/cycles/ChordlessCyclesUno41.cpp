// no-networkit-format

#include "networkit/cycles/ChordlessCyclesUno41.hpp"
#include <iostream>
#include "networkit/clique/MaximalCliques.hpp"

namespace NetworKit {

using paths = std::vector<node>;

void ChordlessCyclesUno41::reset() {
    hasRun = false;
    result.clear();

    G_copy = Graph(*G);
    num_tree_nodes = 0;
    num_leafs = 0;
}

void ChordlessCyclesUno41::allChordlessPaths(const Graph &g, node current, node target,
                                             const std::vector<node> &path,
                                             std::vector<count> blocked) {
    auto suggestedPath = bfsPaths(g, current, target, blocked, true);
    if (suggestedPath.empty()) {
        unsuccessfulBfsCalls += 1;
        return;
    } else {
        std::vector<bool> marks(G->upperNodeIdBound(), true);
        allChordlessPathsFollow(g, suggestedPath, 0, marks, blocked, 100);
    }
}

void ChordlessCyclesUno41::destructiveEnumeration() {
    std::vector<uint64_t> blocked(G->upperNodeIdBound(), 0);

    for (auto u : node_order) {
        num_outer_loops += 1;

        if (result.size() >= num_max_cyles) {
            break;
        }

        std::vector<node> neighbors;
        for (auto v : G_copy.neighborRange(u)) {
            if (blocked[v] == 0)
                neighbors.push_back(v);
        }

        for (auto v : neighbors) {
            num_inner_loops += 1;
            G_copy.removeEdge(u, v);
            std::vector<node> path;
            path.push_back(u);
            // biConReport(G_copy);
            allChordlessPaths(G_copy, u, v, path, blocked);
            blocked[v] += 1;
        }

        blocked[u] += 1;
        for (auto v : neighbors) {
            G_copy.addEdge(u, v);
            blocked[v] -= 1;
        }
    }
}

void ChordlessCyclesUno41::constructiveEnumeration() {
    // no node added in the beginning
    std::vector<uint64_t> blocked(G->upperNodeIdBound(), 1);

    for (auto u : node_order) {
        num_outer_loops += 1;
        count numBefore = result.size();

        if (result.size() >= num_max_cyles) {
            break;
        }

        // add node u and available edges
        blocked[u] -= 1;

        // enum all chordless paths through u
        std::vector<node> neighbors;
        for (auto v : G_copy.neighborRange(u)) {
            if (blocked[v] == 0)
                neighbors.push_back(v);
        }

        for (auto v : neighbors) {
            num_inner_loops += 1;
            G_copy.removeEdge(u, v);
            std::vector<node> path;
            path.push_back(u);
            // biConReport(G_copy);
            allChordlessPaths(G_copy, u, v, path, blocked);
            blocked[v] += 1;
        }

        for (auto v : neighbors) {
            G_copy.addEdge(u, v);
            blocked[v] -= 1;
        }
        count numAfter = result.size();
        //std::cout << numAfter - numBefore << ", ";
    }
    //std::cout << std::endl;
}

void inline blockNeighbors(const Graph &g, node u, std::vector<count> &blocked, node exception) {
    for (auto v : g.neighborRange(u)) {
        if (v != exception)
            blocked[v] += 1;
    }
}

void inline unblockNeighbors(const Graph &g, node u, std::vector<count> &blocked, node exception) {
    for (auto v : g.neighborRange(u)) {
        if (v != exception)
            blocked[v] -= 1;
    }
}

void ChordlessCyclesUno41::allChordlessPathsFollow(const Graph &g, const std::vector<node> &path,
                                                   uid_t currentPosition, std::vector<bool> &marks,
                                                   std::vector<count> &blocked,
                                                   count minLength = 29) {
    assert(currentPosition < path.size() - 1);
    auto currentNode = path[currentPosition];
    assert(g.degree(currentNode) > 0);
    assert(blocked[currentNode] == 0);
    auto nextNode = path[currentPosition + 1];
    auto targetNode = path[path.size() - 1];

    if (result.size() >= num_max_cyles) {
        return;
    }
    num_tree_nodes += 1;
    blocked[currentNode] += 1;

    if (currentPosition >= path.size() - 2) {
        if (callback) {
            bool goOn = callback(path);
        } else {
            result.push_back(path);
        }
        num_tree_nodes += 1;
        num_leafs += 1;

        // set marks for use in parent
        // graph search from t but all nodes on path and their neighbors are disallowed
        auto newMarks = bfsReachableMask(g, targetNode, blocked);
        for (count i = 0; i < marks.size(); i++) {
            marks[i] = newMarks[i];
        }

        // unblock current before going up
        blocked[currentNode] -= 1;
        return;
    }

    // first recursion: follow suggested paths
    {
        blockNeighbors(g, currentNode, blocked, nextNode);
        allChordlessPathsFollow(g, path, currentPosition + 1, marks, blocked);
        unblockNeighbors(g, currentNode, blocked, nextNode);
    }

    // remaining recursions: follow other paths
    for (auto v : g.neighborRange(currentNode)) {
        auto neighbors = g.neighborRange(v);
        if ((v == nextNode) || (blocked[v] > 0)
            || !std::any_of(neighbors.begin(), neighbors.end(),
                            [marks](node e) { return marks[e]; }))
            continue;

        // create G \ (N(s) \ v) via blocking
        blockNeighbors(g, currentNode, blocked, v);

        auto suggestedSuffix = bfsPaths(g, v, targetNode, blocked);
        if (!suggestedSuffix.empty()) {
            // The start node should not be part of the paths to follow
            std::vector<node> newPath = {path.begin(), path.begin() + currentPosition + 1};
            std::move(suggestedSuffix.begin(), suggestedSuffix.end(), std::back_inserter(newPath));
            std::vector<bool> newMarks(G->upperNodeIdBound(), true);
            // if (currentPosition + 1 + cliqueBound(currentGraph) >= minLength)
            allChordlessPathsFollow(g, newPath, currentPosition + 1, newMarks, blocked);
        } else {
            unsuccessfulBfsCalls += 1;
            // assert(false);
        }

        // always unblock before next loop
        unblockNeighbors(g, currentNode, blocked, v);
    }

    auto fakeBlocked = std::vector<node>(blocked);
    for (auto u : g.nodeRange()) {
        if (marks[u]) {
            fakeBlocked[u] += 1;
        }
    }
    for (auto v : g.neighborRange(currentNode)) {
        auto neighbors = g.neighborRange(v);
        if ((fakeBlocked[v] == 0)
            & std::any_of(neighbors.begin(), neighbors.end(),
                          [marks](node e) { return marks[e]; })) {
            // graph explore from node, but skip all marked or blocked
            // update marks of newly reachable nodes
            for (auto u : allBfsReachable(g, v, fakeBlocked)) {
                marks[u] = true;
                fakeBlocked[u] += 1;
            }
        }
    }

    // unblock current before going up
    blocked[currentNode] -= 1;
    return;
}

} // namespace NetworKit
