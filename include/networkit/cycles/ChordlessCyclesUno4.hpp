#ifndef NETWORKIT_CYCLES_CHORDLESS_CYCLES_UNO_4_HPP_
#define NETWORKIT_CYCLES_CHORDLESS_CYCLES_UNO_4_HPP_

#include <functional>

#include <networkit/cycles/ChordlessCycles.hpp>
#include <networkit/graph/Graph.hpp>

namespace NetworKit {

/**
 * Description
 */
class ChordlessCyclesUno4 final : public ChordlessCycles {

    using ChordlessCycles::ChordlessCycles;

    void reset() override;

private:
    void allChordlessPaths(const Graph &g, node start, node current,
                           const std::vector<node> &path) override;
    void allChordlessPathsFollow(const Graph &g, const std::vector<node> &path,
                                 uid_t currentPosition, std::vector<bool> &marks, count minLength);
};

} // namespace NetworKit

#endif // NETWORKIT_CYCLES_CHORDLESS_CYCLES_UNO_4_HPP_
