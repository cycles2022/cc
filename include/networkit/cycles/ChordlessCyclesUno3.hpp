#ifndef NETWORKIT_CYCLES_CHORDLESS_CYCLES_UNO_3_HPP_
#define NETWORKIT_CYCLES_CHORDLESS_CYCLES_UNO_3_HPP_

#include <functional>

#include <networkit/cycles/ChordlessCycles.hpp>
#include <networkit/graph/Graph.hpp>

namespace NetworKit {

/**
 * Description
 */
class ChordlessCyclesUno3 final : public ChordlessCycles {

    using ChordlessCycles::ChordlessCycles;

    void reset() override;

private:
    void allChordlessPaths(const Graph &g, node start, node current,
                           const std::vector<node> &path) override;
    void allChordlessPathsFollow(const Graph &g, node current, node target,
                                 const std::vector<node> &path,
                                 std::vector<node> &suggestedPathsReverse);
};

} // namespace NetworKit

#endif // NETWORKIT_CYCLES_CHORDLESS_CYCLES_UNO_3_HPP_
