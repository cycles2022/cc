#ifndef NETWORKIT_CYCLES_PREPROCESSING_CYCLES_HPP_
#define NETWORKIT_CYCLES_PREPROCESSING_CYCLES_HPP_

#include "iostream"
#include <functional>

#include "networkit/centrality/Betweenness.hpp"
#include "networkit/structures/Partition.hpp"
#include <networkit/base/Algorithm.hpp>
#include <networkit/graph/Graph.hpp>

namespace NetworKit {

std::vector<node> betweennessOrderedNodes(const Graph &g);
std::vector<node> betweennessEstimatedOrderedNodes(const Graph &g, count nSamples);
std::vector<node> randomOrderedNodes(const Graph &g, int seed);
std::vector<node> smallestFirstOrderedNodes(const Graph &g);
std::vector<node> fastSmallestFirstOrderedNodes(const Graph &g, const Partition &p);
std::vector<node> largestFirstOrderedNodes(const Graph &g);
std::vector<node> pageRankOrderedNodes(const Graph &g);
Graph preprocessedGraph(const Graph &g);
Graph degTwoPreproGraph(const Graph &g);

} // namespace NetworKit

#endif // NETWORKIT_CYCLES_PREPROCESSING_CYCLES_HPP_
