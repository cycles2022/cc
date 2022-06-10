#include <algorithm>
#include <climits>
#include <random>

#include "networkit/centrality/EstimateBetweenness.hpp"
#include "networkit/centrality/PageRank.hpp"
#include "networkit/components/BiconnectedComponents.hpp"
#include "networkit/components/ConnectedComponents.hpp"
#include "networkit/cycles/PreprocessingForCycles.hpp"

namespace NetworKit {

std::vector<node> smallestFirstOrderedNodes(const Graph &g) {
    count M = 1000000000;

    auto gCopy = Graph(g);

    auto degrees = std::vector<count>(g.upperNodeIdBound(), M);
    node smallestDegreeNode = 0;
    count smallestDegree = g.degree(0);
    for (auto u : g.nodeRange()) {
        auto currentDegree = g.degree(u);
        degrees[u] = currentDegree;
        if (currentDegree < smallestDegree) {
            smallestDegreeNode = u;
            smallestDegree = currentDegree;
        }
    }

    std::vector<node> nodes(0);
    for (count i = 0; i < g.numberOfNodes(); i++) {
        auto minElementIndex = std::min_element(degrees.begin(), degrees.end()) - degrees.begin();
        assert(i != 0 || (degrees[minElementIndex] == smallestDegree));
        for (auto v : gCopy.neighborRange(minElementIndex)) {
            degrees[v] -= 1;
        }
        nodes.push_back(minElementIndex);
        degrees[minElementIndex] = M;
        gCopy.removeNode(minElementIndex);
    }

    return nodes;
}

std::vector<node> fastSmallestFirstOrderedNodes(const Graph &g, const Partition &p) {
    count M = 10000000000;

    auto gCopy = Graph(g);

    auto degrees = std::vector<count>(g.upperNodeIdBound(), M);

    count smallestDegree = g.degree(0);
    count largestDegree = g.degree(0);

    // collect degrees
    for (auto u : g.nodeRange()) {
        auto currentDegree = 0;
        for (auto v : g.neighborRange(u)) {
            if (p.inSameSubset(u, v)) {
                currentDegree += 1;
            }
        }

        degrees[u] = currentDegree;
        if (currentDegree < smallestDegree) {
            smallestDegree = currentDegree;
        }
        if (currentDegree > largestDegree) {
            largestDegree = currentDegree;
        }
    }

    // setup buckets
    std::vector<std::vector<node>> degreeBuckets = {};
    for (count i = 0; i <= largestDegree; i++) {
        degreeBuckets.emplace_back();
    }

    // fill buckets
    auto nodeInBucketPosition = std::vector<count>(g.upperNodeIdBound(), M);
    for (auto u : g.nodeRange()) {
        auto currentDegree = 0;
        for (auto v : g.neighborRange(u)) {
            if (p.inSameSubset(u, v)) {
                currentDegree += 1;
            }
        }
        assert(degrees[u] == currentDegree);
        nodeInBucketPosition[u] = degreeBuckets[currentDegree].size();
        degreeBuckets[currentDegree].push_back(u);
    }

    for (auto &b : degreeBuckets) {
        for (count i = 0; i < b.size(); i++) {
            node u = b[i];
            assert(nodeInBucketPosition[u] == i);
            assert(u < g.upperNodeIdBound());
        }
    }

    std::vector<node> nodes(0);
    for (count i = 0; i < g.numberOfNodes(); i++) {
        smallestDegree = 0;
        while (degreeBuckets[smallestDegree].empty()) {
            smallestDegree += 1;
        }
        assert(!degreeBuckets[smallestDegree].empty());
        node smallest = degreeBuckets[smallestDegree].back();
        assert(g.hasNode(smallest));
        degreeBuckets[smallestDegree].pop_back();
        degrees[smallest] = 0;
        nodes.push_back(smallest);
        for (node u : g.neighborRange(smallest)) {
            if ((degrees[u] > 0) && (p.inSameSubset(smallest, u))) {
                auto oldDegree = degrees[u];
                auto newDegree = oldDegree - 1;

                auto oldDegreeLenBefore = degreeBuckets[oldDegree].size();
                auto newDegreeLenBefore = degreeBuckets[oldDegree - 1].size();

                auto oldPosition = nodeInBucketPosition[u];
                auto newPosition = degreeBuckets[newDegree].size();

                assert(g.hasNode(degreeBuckets[oldDegree][oldPosition]));
                assert(degreeBuckets[oldDegree][oldPosition] == u);

                // Update for new degree
                degrees[u] = newDegree;
                nodeInBucketPosition[u] = newPosition;
                degreeBuckets[newDegree].push_back(u);
                assert(degreeBuckets[newDegree][newPosition] == u);
                assert(degreeBuckets[newDegree][nodeInBucketPosition[u]] == u);

                // Delete for old degree
                assert(!degreeBuckets[oldDegree].empty());
                node back = degreeBuckets[oldDegree].back();
                assert(g.hasNode(back));
                if (back != u) {
                    degreeBuckets[oldDegree][oldPosition] = back;
                    nodeInBucketPosition[back] = oldPosition;
                    assert(degreeBuckets[oldDegree][nodeInBucketPosition[back]] == back);
                }
                degreeBuckets[oldDegree].pop_back();

                assert((degreeBuckets[oldDegree].size() + 1) == oldDegreeLenBefore);
                assert((degreeBuckets[oldDegree - 1].size() - 1) == newDegreeLenBefore);
            }
        }
    }

    assert(nodes.size() == g.numberOfNodes());
    assert(smallestDegree == 0);

    return nodes;
}

std::vector<node> largestFirstOrderedNodes(const Graph &g) {
    int M = -10000;

    auto gCopy = Graph(g);

    auto degrees = std::vector<int>(g.upperNodeIdBound(), M);
    node largestDegreeNode = 0;
    count largestDegree = g.degree(0);
    for (auto u : g.nodeRange()) {
        auto currentDegree = g.degree(u);
        degrees[u] = static_cast<int>(currentDegree);
        if (currentDegree > largestDegree) {
            largestDegreeNode = u;
            largestDegree = currentDegree;
        }
    }

    std::vector<node> nodes(0);
    for (count i = 0; i < g.numberOfNodes(); i++) {
        auto maxElementIndex = std::max_element(degrees.begin(), degrees.end()) - degrees.begin();
        assert(i != 0 || (degrees[maxElementIndex] == largestDegree));
        for (auto v : gCopy.neighborRange(maxElementIndex)) {
            degrees[v] -= 1;
        }
        nodes.push_back(maxElementIndex);
        degrees[maxElementIndex] = M;
        gCopy.removeNode(maxElementIndex);
    }

    return nodes;
}

std::vector<node> randomOrderedNodes(const Graph &g, int seed) {
    Betweenness b(g);
    b.run();

    std::vector<node> nodes(0);
    for (auto u : g.nodeRange()) {
        nodes.push_back(u);
    }

    auto rng = std::default_random_engine(seed);
    std::shuffle(nodes.begin(), nodes.end(), rng);

    return nodes;
}

std::vector<node> betweennessOrderedNodes(const Graph &g) {
    Betweenness b(g);
    b.run();

    std::vector<node> nodes;
    nodes.reserve(g.numberOfNodes());
    for (auto v : g.nodeRange()) {
        nodes.emplace_back(v);
    }

    std::sort(nodes.begin(), nodes.end(), [&](node u, node v) { return b.score(u) < b.score(v); });

    return nodes;
}

std::vector<node> betweennessEstimatedOrderedNodes(const Graph &g, count nSamples) {
    EstimateBetweenness b(g, nSamples);
    b.run();

    std::vector<node> nodes;
    nodes.reserve(g.numberOfNodes());
    for (auto v : g.nodeRange()) {
        nodes.emplace_back(v);
    }

    std::sort(nodes.begin(), nodes.end(), [&](node u, node v) { return b.score(u) < b.score(v); });

    return nodes;
}

std::vector<node> pageRankOrderedNodes(const Graph &g) {
    PageRank b(g);
    b.run();

    std::vector<node> nodes;
    nodes.reserve(g.numberOfNodes());
    for (auto v : g.nodeRange()) {
        nodes.emplace_back(v);
    }

    std::sort(nodes.begin(), nodes.end(), [&](node u, node v) { return b.score(u) < b.score(v); });

    return nodes;
}

node deleteFromVector(std::vector<node> &vec, index i) {
    assert(i < vec.size());

    node movedNode = none;

    if (i < vec.size() - 1) {
        movedNode = vec.back();
        vec[i] = movedNode;
    }

    vec.pop_back();
    return movedNode;
}

Graph preprocessedGraph(const Graph &g) {
    Graph gCopy = Graph(g);

    auto bi_c = BiconnectedComponents(g);
    bi_c.run();
    auto res = bi_c.getComponents();

    // Bridges can be deleted
    count numBridges = 0;
    for (auto &curComponent : res) {
        if (curComponent.size() == 2) {
            unsigned long u = curComponent[0];
            unsigned long v = curComponent[1];
            gCopy.removeEdge(u, v);
            numBridges += 1;
        }
    }
    // std::cout << "Num bridges: " << numBridges << std::endl;

    // Nodes with no edges can be deleted
    count M = 10000000;

    auto degrees = std::vector<count>(gCopy.upperNodeIdBound(), M);
    auto degTwoNodes = std::vector<node>(0);

    count numDegreeZeroNodes = 0;
    for (auto u : g.nodeRange()) {
        count currentDegree = gCopy.degree(u);
        if (currentDegree == 0) {
            gCopy.removeNode(u);
            numDegreeZeroNodes += 1;
        } else if (currentDegree == 2) {
            degTwoNodes.push_back(u);
        }
        assert(currentDegree != 1);
        degrees[u] = currentDegree;
    }

    // As long as new degree two nodes exist, try contract and pre-enumerate
    count numDeletedNodes = 0;
    count numCCs = 0;
    count numInducedPaths = 0;
    while (!degTwoNodes.empty()) {
        auto currentNode = degTwoNodes.back();
        degTwoNodes.pop_back();

        if (degrees[currentNode] <= 0) {
            // Node already got taken care of
            continue;
        }

        assert(degrees[currentNode] == 2);
        node left = gCopy.getIthNeighbor(currentNode, 0);
        node right = gCopy.getIthNeighbor(currentNode, 1);
        assert(!((left == none) || (right == none)));

        node preLeft = currentNode;
        node preRight = currentNode;
        std::vector<node> middleNodes = {currentNode};

        bool successfulExtension = true;
        bool ccFound = false;

        while (successfulExtension & !ccFound) {
            if (gCopy.hasEdge(left, right)) {
                ccFound = true;
            } else {
                if (degrees[left] == 2) {
                    middleNodes.push_back(left);
                    node n1 = gCopy.getIthNeighbor(left, 0);
                    node n2 = gCopy.getIthNeighbor(left, 1);
                    auto tempLeft = left;
                    left = (n1 == preLeft) ? n2 : n1;
                    preLeft = tempLeft;
                } else if (degrees[right] == 2) {
                    middleNodes.push_back(right);
                    node n1 = gCopy.getIthNeighbor(right, 0);
                    node n2 = gCopy.getIthNeighbor(right, 1);
                    auto tempRight = right;
                    right = (n1 == preRight) ? n2 : n1;
                    preRight = tempRight;
                } else {
                    successfulExtension = false;
                }
            }
        }

        if (ccFound) {
            numCCs += 1;

            for (auto v : middleNodes) {
                degrees[v] = 0;
                numDeletedNodes += 1;
                gCopy.removeNode(v);
            }

            // std::cout << "CC length: " << middleNodes.size() + 2 << std::endl;
            if ((degrees[left] == 2) && (degrees[right] == 2)) {
                numDeletedNodes += 2;
                degrees[left] = 0;
                gCopy.removeNode(left);
                degrees[right] = 0;
                gCopy.removeNode(right);
            } else if (degrees[left] == 2) {
                numDeletedNodes += 1;
                degrees[left] = 0;
                degrees[right] -= 2;
                gCopy.removeNode(left);
            } else if (degrees[right] == 2) {
                numDeletedNodes += 1;
                degrees[right] = 0;
                degrees[left] -= 2;
                gCopy.removeNode(right);
            } else {
                auto newLeftDegree = degrees[left] - 1;
                assert(newLeftDegree != 1);
                degrees[left] = newLeftDegree;
                if (newLeftDegree == 2) {
                    degTwoNodes.push_back(left);
                }
                auto newRightDegree = degrees[right] - 1;
                assert(newRightDegree != 1);
                degrees[right] = newRightDegree;
                if (newRightDegree == 2) {
                    degTwoNodes.push_back(right);
                }
            }
        } else {
            if (middleNodes.size() > 1) {
                auto firstMiddleNode = middleNodes[0];
                assert(firstMiddleNode != none);
                for (auto v : middleNodes) {
                    if (v != firstMiddleNode) {
                        degrees[v] = 0;
                        gCopy.removeNode(v);
                    }
                }
                assert(degrees[firstMiddleNode] == 2);

                numDeletedNodes += middleNodes.size() - 1;
                gCopy.addEdge(left, firstMiddleNode);
                gCopy.addEdge(right, firstMiddleNode);
                numInducedPaths += 1;
                // std::cout << "Tunnel length: " << middleNodes.size() + 2 << std::endl;
            }
        }
    }

    std::cout << g.numberOfNodes() << ", " << g.numberOfEdges()
              << ", " << gCopy.numberOfNodes() << ", " << gCopy.numberOfEdges()
              << ", " << numBridges
              << ", " << numInducedPaths
              << ", " << numCCs;
    return gCopy;
}

Graph degTwoPreproGraph(const Graph &g) {
    Graph gCopy = Graph(g);

    count M = 10000000;

    auto degrees = std::vector<count>(g.upperNodeIdBound(), M);
    auto degTwoNodes = std::vector<node>(0);

    for (auto u : g.nodeRange()) {
        auto currentDegree = g.degree(u);
        degrees[u] = currentDegree;
        if (currentDegree == 2) {
            degTwoNodes.push_back(u);
        }
    }
    auto originalDegrees(degrees);

    count numCCs = 0;
    count numTunnel = 0;
    count numDeletedNodes = 0;

    for (auto u : degTwoNodes) {
        if (gCopy.hasNode(u) && degrees[u] == 2) {
            node left = gCopy.getIthNeighbor(u, 0);
            node right = gCopy.getIthNeighbor(u, 1);
            assert(!((left == none) || (right == none)));

            node preLeft = u;
            node preRight = u;
            std::vector<node> middleNodes = {u};

            bool successfulExtension = true;
            bool ccFound = false;

            while (successfulExtension & !ccFound) {
                if (g.hasEdge(left, right)) {
                    ccFound = true;
                } else {
                    if (originalDegrees[left] == 2) {
                        middleNodes.push_back(left);
                        node n1 = gCopy.getIthNeighbor(left, 0);
                        node n2 = gCopy.getIthNeighbor(left, 1);
                        auto tempLeft = left;
                        left = (n1 == preLeft) ? n2 : n1;
                        preLeft = tempLeft;
                    } else if (originalDegrees[right] == 2) {
                        middleNodes.push_back(right);
                        node n1 = gCopy.getIthNeighbor(right, 0);
                        node n2 = gCopy.getIthNeighbor(right, 1);
                        auto tempRight = right;
                        right = (n1 == preRight) ? n2 : n1;
                        preRight = tempRight;
                    } else {
                        successfulExtension = false;
                    }
                }
            }

            if (ccFound) {
                numCCs += 1;

                for (auto v : middleNodes) {
                    for (auto x : gCopy.neighborRange(v)) {
                        degrees[x] -= 1;
                    }
                    numDeletedNodes += 1;
                    gCopy.removeNode(v);
                }

                std::cout << "CC length: " << middleNodes.size() + 2 << std::endl;
                if ((originalDegrees[left] == 2) && (originalDegrees[right] == 2)) {
                    gCopy.removeNode(left);
                    gCopy.removeNode(right);
                }
            } else {
                if (middleNodes.size() > 1) {
                    auto firstMiddleNode = middleNodes[0];
                    for (auto v : middleNodes) {
                        for (auto x : gCopy.neighborRange(v)) {
                            degrees[x] -= 1;
                        }
                        numDeletedNodes += 1;
                        gCopy.removeNode(v);
                    }
                    gCopy.restoreNode(firstMiddleNode);
                    numDeletedNodes += middleNodes.size() - 1;
                    gCopy.addEdge(left, firstMiddleNode);
                    gCopy.addEdge(right, firstMiddleNode);
                    numTunnel += 1;
                    std::cout << "Tunnel length: " << middleNodes.size() + 2 << std::endl;
                }
            }
        }
    }

    std::cout << "Num CCs found: " << numCCs << std::endl;
    std::cout << "Num tunnels removed: " << numTunnel << std::endl;
    std::cout << "Num deleted nodes: " << numDeletedNodes << std::endl;

    return gCopy;
}

} // namespace NetworKit
