#ifndef NETWORKIT_CYCLES_CHORDLESS_CYCLES_DIAS_HPP_
#define NETWORKIT_CYCLES_CHORDLESS_CYCLES_DIAS_HPP_

#include <functional>

#include <networkit/base/Algorithm.hpp>
#include <networkit/graph/Graph.hpp>

namespace NetworKit {

/**
 * Description
 */
class ChordlessCyclesDias final : public Algorithm {

public:
    /**
     * Construct the chordless cycle algorithm with the given graph.
     * @param G The graph to list the chordless cycles for.
     */
    ChordlessCyclesDias(const Graph &G);

    /**
     * Construct the chordless cycle algorithm with the given graph and a callback.
     *
     * The callback is called once for each found chordless cycle
     * with a reference to the chordless cycle.
     * Note that the reference is to an internal object, the callback should not assume that
     * this reference is still valid after it returned.
     *
     * @param G The graph to list all chordless cycles for.
     * @param callback The callback to call for each chordless cycle.
     */
    ChordlessCyclesDias(const Graph &G, std::function<void(const std::vector<node> &)> callback);

    /**
     * Execute the chordless cycle listing algorithm.
     */
    void run() override;

    /**
     * Return all found chordless cycles unless a callback was given.
     *
     * This method will throw if a callback was given and thus the chordless cycles were not stored.
     *
     * @return a vector of chordless cycles, each being represented as a vector of nodes.
     */
    const std::vector<std::vector<node>> &getChordlessCycles() const;

private:
    const Graph *G;

    std::vector<std::vector<node>> result;

    std::function<void(const std::vector<node> &)> callback;
};

} // namespace NetworKit

#endif // NETWORKIT_CYCLES_CHORDLESS_CYCLES_DIAS_HPP_
