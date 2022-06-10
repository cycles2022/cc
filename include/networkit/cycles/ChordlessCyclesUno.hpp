#ifndef NETWORKIT_CYCLES_CHORDLESS_CYCLES_UNO_HPP_
#define NETWORKIT_CYCLES_CHORDLESS_CYCLES_UNO_HPP_

#include <functional>

#include <networkit/cycles/ChordlessCycles.hpp>
#include <networkit/graph/Graph.hpp>

namespace NetworKit {

/**
 * Description
 */
class ChordlessCyclesUno final : public ChordlessCycles {
    using ChordlessCycles::ChordlessCycles;

public:
    void run() override;
    void reset() override;

private:
    void findChordlessCycles();
    void chordlessPath(const Graph &g, node u, node v);
    void findChordlessPath(const Graph &g, node current, node target,
                           const std::vector<node> &path);
};

} // namespace NetworKit

#endif // NETWORKIT_CYCLES_CHORDLESS_CYCLES_UNO_HPP_
