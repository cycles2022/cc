// no-networkit-format

#include "networkit/cycles/ChordlessCyclesUno4.hpp"
#include <iostream>
#include "networkit/clique/MaximalCliques.hpp"

namespace NetworKit {

using paths = std::vector<node>;

unsigned long cliqueBound(const Graph &g) {
    auto c = MaximalCliques(g);
    c.run();

    std::vector<uint32_t> nodeCliques(g.numberOfNodes(), -1);
    auto cResult = c.getCliques();
    std::sort(cResult.begin(), cResult.end(), [](auto x, auto y) { return x.size() > y.size(); });
    count numCliques = 0;
    for (count i=0; i<cResult.size(); i++) {
        auto curClique = cResult[i];
        bool allNodesFree = true;
        for (auto u : curClique) {
            allNodesFree = allNodesFree & (nodeCliques[u] == -1);
        }

        if (allNodesFree) {
            numCliques += 1;
            for (auto u : curClique) {
                nodeCliques[u] = i;
            }
        }
    }

    count notCovered = 0;
    for (auto p : nodeCliques) {
        if (p == -1) notCovered += 1;
    }

    return numCliques * 2 + notCovered;
}

void ChordlessCyclesUno4::reset() {
    hasRun = false;
    result.clear();

    G_copy = Graph(*G);
    num_tree_nodes = 0;
    num_leafs = 0;
}

void ChordlessCyclesUno4::allChordlessPaths(const Graph &g, node current, node target,
                                            const std::vector<node> &path) {
    auto suggestedPath = bfsPaths(g, current, target, true);
    if (suggestedPath.empty()) {
        unsuccessfulBfsCalls += 1;
        return;
    } else {
        // The start node should not be part of the paths to follow
        std::vector<bool> marks(G->numberOfNodes(), true);
        allChordlessPathsFollow(g, suggestedPath, 0, marks, 100);
    }
}

void ChordlessCyclesUno4::allChordlessPathsFollow(const Graph &g, const std::vector<node> &path,
                                                  uid_t currentPosition, std::vector<bool> &marks,
                                                  count minLength = 29) {
    assert(currentPosition < path.size() - 1);
    auto currentNode = path[currentPosition];
    assert(g.degree(currentNode) > 0);
    auto nextNode = path[currentPosition + 1];
    auto targetNode = path[path.size() - 1];

    if (result.size() >= num_max_cyles) {
        return;
    }
    num_tree_nodes += 1;

    if (currentPosition >= path.size() - 2) {
        if (callback) {
            callback(path);
        } else {
            result.push_back(path);
        }
        num_tree_nodes += 1;
        num_leafs += 1;

        // set marks for use in parent
        // graph search from t but all nodes on path and their neighbors are disallowed
        auto currentGraph = Graph(g);
        currentGraph.removeNode(currentNode);
        auto newMarks = bfsReachableMask(currentGraph, targetNode);
        for (count i = 0; i < marks.size(); i++) {
            marks[i] = newMarks[i];
        }
        return;
    }

    // first recursion: follow suggested paths
    {
        auto currentGraph = Graph(g);
        for (auto u : currentGraph.neighborRange(currentNode)) {
            if (u != nextNode)
                currentGraph.removeNode(u);
        }
        currentGraph.removeNode(currentNode);
        //if (currentPosition + 1 + cliqueBound(currentGraph) >= minLength)
        allChordlessPathsFollow(currentGraph, path, currentPosition + 1, marks);
    }

    // remaining recursions: follow other paths
    for (auto v : g.neighborRange(currentNode)) {
        auto neighbors = g.neighborRange(v);
        if (v == nextNode || !std::any_of(neighbors.begin(), neighbors.end(), [marks](node e) {
                return marks[e];
            }))
            continue;

        auto currentGraph = Graph(g);

        // create G \ (N(s) \ v)
        for (auto u : currentGraph.neighborRange(currentNode)) {
            if (u != v)
                currentGraph.removeNode(u);
        }
        currentGraph.removeNode(currentNode);

        auto suggestedSuffix = bfsPaths(currentGraph, v, targetNode, true);
        if (!suggestedSuffix.empty()) {
            // The start node should not be part of the paths to follow
            std::vector<node> newPath = {path.begin(), path.begin() + currentPosition + 1};
            std::move(suggestedSuffix.begin(), suggestedSuffix.end(), std::back_inserter(newPath));
            std::vector<bool> newMarks(G->numberOfNodes(), true);
            //if (currentPosition + 1 + cliqueBound(currentGraph) >= minLength)
            allChordlessPathsFollow(currentGraph, newPath, currentPosition + 1, newMarks);
        } else {
            unsuccessfulBfsCalls += 1;
            //assert(false);
        }
    }

    auto currentGraph = Graph(g);
    currentGraph.removeNode(currentNode);
    auto pseudoNode = currentGraph.addNode();
    // go up, update marks
    for (auto v : g.neighborRange(currentNode)) {
        auto neighbors = g.neighborRange(v);
        if (std::any_of(neighbors.begin(), neighbors.end(), [marks](node e) { return marks[e]; }))
            currentGraph.addEdge(v, pseudoNode);
    }
    for (auto v : g.nodeRange()) {
        if (marks[v] & currentGraph.hasNode(v))
            currentGraph.removeNode(v);
    }
    auto newMarks = bfsReachableMask(currentGraph, pseudoNode);
    for (count i = 0; i < marks.size(); i++) {
        marks[i] = marks[i] | newMarks[i];
    }
}

} // namespace NetworKit
