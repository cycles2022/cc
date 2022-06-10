#ifndef NETWORKIT_CYCLES_CHORDLESS_CYCLES_UNO_41_HPP_
#define NETWORKIT_CYCLES_CHORDLESS_CYCLES_UNO_41_HPP_

#include <functional>

#include <networkit/cycles/ChordlessCycles.hpp>
#include <networkit/graph/Graph.hpp>

namespace NetworKit {

/**
 * Description
 */
class ChordlessCyclesUno41 final : public ChordlessCycles {

    using ChordlessCycles::ChordlessCycles;

    void reset() override;

protected:
    void destructiveEnumeration() override;
    void constructiveEnumeration() override;

private:
    void allChordlessPaths(const Graph &g, node start, node current,
                           const std::vector<node> &path, std::vector<count> blocked);

    void allChordlessPathsFollow(const Graph &g, const std::vector<node> &path,
                                 uid_t currentPosition,
                                 std::vector<bool> &marks,
                                 std::vector<count> &blocked,
                                 count minLength);
};

} // namespace NetworKit

#endif // NETWORKIT_CYCLES_CHORDLESS_CYCLES_UNO_41_HPP_
