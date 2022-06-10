// no-networkit-format
#include <iostream>
#include <utility>

#include "networkit/components/BiconnectedComponents.hpp"
#include "networkit/cycles/ChordlessCycles.hpp"
#include "networkit/distance/BFS.hpp"

namespace NetworKit {

ChordlessCycles::ChordlessCycles(const Graph &G, unsigned int num_max_cycles_, bool constructive_)
    : G(&G), G_copy(G), num_max_cyles(num_max_cycles_), constructive(constructive_) {
    for (auto v : G.nodeRange()) {
        node_order.push_back(v);
    }
}

ChordlessCycles::ChordlessCycles(const Graph &G, const std::vector<node> &node_order,
                                 unsigned int num_max_cycles_, bool constructive_)
    : G(&G), G_copy(G), node_order(node_order), num_max_cyles(num_max_cycles_),
      constructive(constructive_) {}

ChordlessCycles::ChordlessCycles(const Graph &G,
                                 std::function<bool(const std::vector<node> &)> callback,
                                 const std::vector<node> &node_order, unsigned int num_max_cycles_,
                                 bool constructive_)
    : G(&G), callback(std::move(callback)), G_copy(G), node_order(node_order),
      num_max_cyles(num_max_cycles_), constructive(constructive_) {}

const std::vector<std::vector<node>> &ChordlessCycles::ChordlessCycles::getChordlessCycles() const {
    if (callback)
        throw std::runtime_error(
            "ChordlessCycles used with callback does not store chordless cycles");
    assureFinished();
    return result;
}

bool ChordlessCycles::bfsReachable(const Graph &g, node start, node target) {
    bfsCalls += 1;
    auto bfs = NetworKit::BFS(g, start, false, false, target);
    bfs.run();
    bfsNeighborQueries += bfs.neighborQueries;
    return (int)bfs.distance(target) < g.numberOfNodes();
}

std::vector<node> ChordlessCycles::allBfsReachable(const Graph &g, node start,
                                                   const std::vector<count> &blocked) {
    bfsCalls += 1;
    auto bfs = NetworKit::BFS(g, start, blocked, true, false);
    bfs.run();
    bfsNeighborQueries += bfs.neighborQueries;

    auto allDistances = bfs.getDistances();
    auto reachableNodes = std::vector<node>();
    uint num_nodes = g.numberOfNodes();
    for (auto curNode : g.nodeRange()) {
        if (allDistances[curNode] < num_nodes) {
            reachableNodes.push_back(curNode);
        }
    }
    return reachableNodes;
}

std::vector<bool> ChordlessCycles::bfsReachableMask(const Graph &g, node start) {
    bfsCalls += 1;
    auto bfs = NetworKit::BFS(g, start, false, false);
    bfs.run();
    bfsNeighborQueries += bfs.neighborQueries;

    auto allDistances = bfs.getDistances();
    auto reachableMask = std::vector<bool>(allDistances.size(), false);
    uint num_nodes = g.numberOfNodes();
    for (auto curNode : g.nodeRange()) {
        reachableMask[curNode] = allDistances[curNode] < num_nodes;
    }

    return reachableMask;
}

std::vector<bool> ChordlessCycles::bfsReachableMask(const Graph &g, node start,
                                                    const std::vector<count> &blocked) {
    bfsCalls += 1;
    auto bfs = NetworKit::BFS(g, start, blocked, true, false);
    bfs.run();
    bfsNeighborQueries += bfs.neighborQueries;

    auto allDistances = bfs.getDistances();
    auto reachableMask = std::vector<bool>(allDistances.size(), false);
    uint num_nodes = g.numberOfNodes();
    for (auto curNode : g.nodeRange()) {
        reachableMask[curNode] = allDistances[curNode] < num_nodes;
    }

    return reachableMask;
}

std::vector<node> ChordlessCycles::bfsPaths(const Graph &g, node start, node target, bool forward) {
    bfsCalls += 1;
    auto bfs = NetworKit::BFS(g, start, true, false, target);
    bfs.run();
    bfsNeighborQueries += bfs.neighborQueries;
    return bfs.getPath(target, forward);
}

std::vector<node> ChordlessCycles::bfsPaths(const Graph &g, node start, node target,
                                            const std::vector<count> &blocked, bool forward) {
    bfsCalls += 1;
    auto bfs = NetworKit::BFS(g, start, blocked, true, false, target);
    bfs.run();
    bfsNeighborQueries += bfs.neighborQueries;
    return bfs.getPath(target, forward);
}

void ChordlessCycles::reset() {
    hasRun = false;
    result.clear();

    G_copy = Graph(*G);
    num_tree_nodes = 0;
    num_leafs = 0;
}

void ChordlessCycles::run() {
    reset();
    if (constructive) {
        constructiveEnumeration();
    } else {
        destructiveEnumeration();
    }
    hasRun = true;
    report();
}

void ChordlessCycles::destructiveEnumeration() {
    for (auto u : node_order) {
        num_outer_loops += 1;

        if (result.size() >= num_max_cyles) {
            break;
        }

        auto currentCopy = Graph(G_copy);
        for (auto v : G_copy.neighborRange(u)) {
            num_inner_loops += 1;
            currentCopy.removeEdge(u, v);
            std::vector<node> path;
            path.push_back(u);
            // biConReport(G_copy);
            allChordlessPaths(currentCopy, u, v, path);
            currentCopy.removeNode(v);
        }

        for (auto w : G_copy.neighborRange(u)) {
            G_copy.removeEdge(u, w);
        }
    }
}

void ChordlessCycles::constructiveEnumeration() {
    G_copy = Graph(G->numberOfNodes());

    // no node added in the beginning
    std::vector<bool> added_nodes;
    added_nodes.reserve(G->numberOfNodes());
    for (uint i = 0; i < G->numberOfNodes(); i++) {
        added_nodes.push_back(false);
    }

    for (auto u : node_order) {
        num_outer_loops += 1;

        if (result.size() >= num_max_cyles) {
            break;
        }

        // add node u and available edges
        added_nodes[u] = true;
        for (auto v : G->neighborRange(u)) {
            if (added_nodes[v]) {
                G_copy.addEdge(u, v);
            }
        }

        // enum all chordless paths through u
        Graph currentCopy = Graph(G_copy);
        for (auto v : G_copy.neighborRange(u)) {
            num_inner_loops += 1;
            currentCopy.removeEdge(u, v);
            std::vector<node> path;
            path.push_back(u);
            // biConReport(G_copy);
            allChordlessPaths(currentCopy, u, v, path);
            currentCopy.removeNode(v);
        }
    }
}

void ChordlessCycles::allChordlessPaths(const Graph &g, node current, node target,
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
            allChordlessPaths(current_graph, v, target, new_path);
        }
    }
}

void ChordlessCycles::report() {
    std::cout << "Num outer loops: " << num_outer_loops << std::endl;
    std::cout << "Num inner loops: " << num_inner_loops << std::endl;
    std::cout << "Num tree nodes: " << num_tree_nodes << std::endl;
    std::cout << "Num leafs: " << num_leafs << std::endl;
    std::cout << "Num ccs: " << result.size() << std::endl;
    std::cout << "Num bfs calls: " << bfsCalls << std::endl;
    std::cout << "Num unsuccessful bfs calls: " << unsuccessfulBfsCalls << std::endl;
    std::cout << "Num bfs neighbor queries: " << bfsNeighborQueries << std::endl;
}
void ChordlessCycles::biConReport(const Graph &g) {
    auto bi_c = BiconnectedComponents(g);
    bi_c.run();
    auto res = bi_c.getComponents();
    if (!res.empty()) {
        auto max_e = std::max_element(res.begin(), res.end(),
                                      [](auto a, auto b) { return a.size() < b.size(); });
        if (max_e != res.end())
            std::cout << res.size() << " " << max_e->size() << " " << result.size() << std::endl;
    }
}

} // namespace NetworKit
