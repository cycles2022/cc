// no-networkit-format

#include "networkit/cycles/ChordlessCyclesUno3.hpp"

namespace NetworKit {

using paths = std::vector<node>;

void ChordlessCyclesUno3::reset() {
    hasRun = false;
    result.clear();

    G_copy = Graph(*G);
    num_tree_nodes = 0;
    num_leafs = 0;
}

void ChordlessCyclesUno3::allChordlessPaths(const Graph &g, node current, node target,
                                            const std::vector<node> &path) {
    auto suggestedPathReverse = bfsPaths(g, current, target, false);
    if (suggestedPathReverse.empty()) {
        unsuccessfulBfsCalls += 1;
        return;
    } else {
        // The start node should not be part of the paths to follow
        suggestedPathReverse.pop_back();
        allChordlessPathsFollow(g, current, target, path, suggestedPathReverse);
    }
}

void ChordlessCyclesUno3::allChordlessPathsFollow(const Graph &g, node current, node target,
                                                  const std::vector<node> &path,
                                                  std::vector<node> &suggestedPathsReverse) {
    assert(path.back() == current);
    assert(suggestedPathsReverse.front() == target);
    assert(g.degree(current) > 0);

    if (result.size() >= num_max_cyles) {
        return;
    }
    num_tree_nodes += 1;

    auto nextNode = suggestedPathsReverse.back();
    suggestedPathsReverse.pop_back();

    if (suggestedPathsReverse.empty()) {
        auto fullPaths = std::vector<node>(path);
        fullPaths.push_back(nextNode);
        if (callback) {
            callback(fullPaths);
        } else {
            result.push_back(fullPaths);
        }
        num_tree_nodes += 1;
        num_leafs += 1;
        return;
    }

    // first recursion: follow suggested paths
    {
        auto currentGraph = Graph(g);
        for (auto u : currentGraph.neighborRange(current)) {
            if (u != nextNode)
                currentGraph.removeNode(u);
        }
        auto newPath(path);
        newPath.push_back(nextNode);
        allChordlessPathsFollow(currentGraph, nextNode, target, newPath, suggestedPathsReverse);
    }

    // remaining recursions: follow other paths
    for (auto v : g.neighborRange(current)) {
        if (v == nextNode)
            continue;

        auto currentGraph = Graph(g);

        // create G \ (N(s) \ v)
        for (auto u : currentGraph.neighborRange(current)) {
            if (u != v)
                currentGraph.removeNode(u);
        }
        currentGraph.removeNode(current);

        auto newSuggestedPathReverse = bfsPaths(currentGraph, v, target, false);
        if (!newSuggestedPathReverse.empty()) {
            // The start node should not be part of the paths to follow
            newSuggestedPathReverse.pop_back();
            auto newPath(path);
            newPath.push_back(v);
            allChordlessPathsFollow(currentGraph, v, target, newPath, newSuggestedPathReverse);
        } else {
            unsuccessfulBfsCalls += 1;
        }

        // re-add edges
    }
}

} // namespace NetworKit
