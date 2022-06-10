// no-networkit-format
#include <algorithm>

#include <iostream>
#include <utility>
#include <networkit/cycles/ChordlessCyclesDias.hpp>

namespace {
// Private implementation namespace
using NetworKit::count;
using NetworKit::index;
using NetworKit::node;

using triplet = std::vector<node>;

class ChordlessCyclesDiasImpl {
private:
    const NetworKit::Graph *G;
    NetworKit::Graph G_copy;
    std::vector<std::vector<node>> *result;
    std::function<void(const std::vector<node> &)> &callback;
    int num_nodes_visited = 0;

public:
    ChordlessCyclesDiasImpl(const NetworKit::Graph &G, std::vector<std::vector<node>> &result,
                            std::function<void(const std::vector<node> &)> &callback)
        : G(&G), result(&result), callback(callback), G_copy(G) {}

    void run() { auto triplets = allTriplets(); }

private:
    std::vector<triplet> allTriplets() {
        auto all_triplets = std::vector<triplet>{};

        for (auto e : G->edgeRange()) {
            auto x = std::max(e.u, e.v);
            auto u = std::min(e.u, e.v);

            for (auto y : G->neighborRange(u)) {
                if (x < y) {
                    if (G->hasEdge(x, y)) {
                        // triangle
                        result->push_back(std::vector<node>{x, u, y});
                    } else {
                        // triplet
                        all_triplets.push_back(std::vector<node>{x, u, y});
                    }
                }
            }
        }

        return all_triplets;
    }
};

} // namespace

namespace NetworKit {

ChordlessCyclesDias::ChordlessCyclesDias(const Graph &G) : G(&G) {}

ChordlessCyclesDias::ChordlessCyclesDias(const Graph &G,
                                         std::function<void(const std::vector<node> &)> callback)
    : G(&G), callback(std::move(callback)) {}

const std::vector<std::vector<node>> &
ChordlessCyclesDias::ChordlessCyclesDias::getChordlessCycles() const {
    if (callback)
        throw std::runtime_error(
            "ChordlessCycles used with callback does not store chordless cycles");
    assureFinished();
    return result;
}

void ChordlessCyclesDias::run() {
    hasRun = false;
    result.clear();
    ChordlessCyclesDiasImpl(*G, result, callback).run();
    hasRun = true;
}

} // namespace NetworKit
