#ifndef NETWORKIT_CYCLES_CHORDLESS_CYCLES_HPP_
#define NETWORKIT_CYCLES_CHORDLESS_CYCLES_HPP_

#include <functional>

#include <networkit/base/Algorithm.hpp>
#include <networkit/graph/Graph.hpp>

namespace NetworKit {

/**
 * Base implementation of naive chordless cycle enumeration
 */
class ChordlessCycles : public Algorithm {

public:
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
    ChordlessCycles(const Graph &G, std::function<bool(const std::vector<node> &)> callback,
                    const std::vector<node> &node_order,
                    unsigned int num_max_cycles = std::numeric_limits<int>::max(),
                    bool constructive = true);
    /**
     * Construct the chordless cycle algorithm with the given graph.
     * @param G The graph to list the chordless cycles for.
     */
    ChordlessCycles(const Graph &G, unsigned int num_max_cycles = std::numeric_limits<int>::max(),
                    bool constructive = true);

    /**
     * Construct the chordless cycle algorithm with the given graph.
     * @param G The graph to list the chordless cycles for.
     */
    ChordlessCycles(const Graph &G, const std::vector<node> &node_order,
                    unsigned int num_max_cycles = std::numeric_limits<int>::max(),
                    bool constructive = true);

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

    /**
     * Print a basic report of hardware indepent metrics for last run
     */
    void report();

protected:
    const Graph *G;
    Graph G_copy;

    // Runtime metrics
    unsigned int num_outer_loops = 0;
    unsigned int num_inner_loops = 0;
    unsigned int num_tree_nodes = 0;
    unsigned int num_leafs = 0;
    unsigned int bfsCalls = 0;
    unsigned int unsuccessfulBfsCalls = 0;
    unsigned int bfsNeighborQueries = 0;

    // Parameter
    std::vector<node> node_order;
    const unsigned int num_max_cyles;
    bool constructive;

    std::vector<std::vector<node>> result;
    std::function<bool(const std::vector<node> &)> callback;

    virtual void destructiveEnumeration();
    virtual void constructiveEnumeration();

    virtual void reset();
    bool bfsReachable(const Graph &g, node start, node target);
    std::vector<node> bfsPaths(const Graph &g, node start, node target, bool forward = true);
    std::vector<node> bfsPaths(const Graph &g, node start, node target,
                               const std::vector<count> &blocked, bool forward = true);
    void biConReport(const Graph &g);
    std::vector<node> allBfsReachable(const Graph &g, node start,
                                      const std::vector<count> &blocked);
    std::vector<bool> bfsReachableMask(const Graph &g, node start);
    std::vector<bool> bfsReachableMask(const Graph &g, node start,
                                       const std::vector<count> &blocked);

private:
    virtual void allChordlessPaths(const Graph &g, node current, node target,
                                   const std::vector<node> &path);
};

} // namespace NetworKit

#endif // NETWORKIT_CYCLES_CHORDLESS_CYCLES_HPP_
