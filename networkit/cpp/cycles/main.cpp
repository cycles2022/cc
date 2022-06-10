#include <chrono>
#include <iostream>
#include <experimental/filesystem>

#include <regex>
#include "networkit/centrality/KatzCentrality.hpp"
#include "networkit/community/PLM.hpp"
#include "networkit/community/PLP.hpp"
#include "networkit/components/BiconnectedComponents.hpp"
#include "networkit/cycles/ChordlessCyclesUno41.hpp"
#include "networkit/cycles/PreprocessingForCycles.hpp"
#include "networkit/io/EdgeListWriter.hpp"
#include <networkit/graph/Graph.hpp>
#include <networkit/io/EdgeListReader.hpp>

using NetworKit::Edge;
using NetworKit::Graph;
using NetworKit::node;

using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;

namespace fs = std::experimental::filesystem;

auto timedPrepro(const std::string &name, const Graph &g) {
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    auto g_prepro = NetworKit::preprocessedGraph(g);
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    auto elapsedTime = end - begin;
    auto elapsedTimeMilliS = duration_cast<milliseconds>(elapsedTime);
    std::cout << ", " << name << ", " << elapsedTimeMilliS.count();
    return g_prepro;
}

template <typename T>
void printVector(std::vector<T> v) {
    std::cout << "Vector of size " << v.size() << ".  ";
    for (uint64_t i = 0; i < v.size(); i++) {
        std::cout << i << " : " << v[i] << ", ";
    }
    std::cout << std::endl;
}

template <typename T, typename U>
void printVector(std::vector<std::pair<T, U>> v) {
    std::cout << "Vector of size " << v.size() << ".  ";
    for (uint64_t i = 0; i < v.size(); i++) {
        std::cout << i << " : " << v[i].first << " " << v[i].second << ", ";
    }
    std::cout << std::endl;
}

auto timedSmallestFirstOrder(const std::string &name, const Graph &g) {
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    auto nodeOrder =
        NetworKit::fastSmallestFirstOrderedNodes(g, NetworKit::Partition(g.upperNodeIdBound(), 0));

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    auto elapsedTime = end - begin;
    auto elapsedTimeMilliS = duration_cast<milliseconds>(elapsedTime);

    std::cout << name << ", " << elapsedTimeMilliS.count() << std::endl;

    return nodeOrder;
}

void timedEnum(const std::string &name, const Graph &g, uint64_t numCycles, bool constructive,
               const std::vector<node> &nodeOrder, const std::string &also) {
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    NetworKit::ChordlessCyclesUno41 cc(g, nodeOrder, numCycles, constructive);
    cc.run();
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    auto elapsedTime = end - begin;
    auto elapsedTimeMilliS = duration_cast<milliseconds>(elapsedTime);
    auto res = cc.getChordlessCycles();

    // Lengths
    uint64_t max_length =
        std::max_element(res.begin(), res.end(), [](const auto &u, const auto &v) {
            return u.size() < v.size();
        })->size();
    std::vector<uint64_t> counts(max_length + 1, 0);

    for (const auto &e : res) {
        counts[e.size()] += 1;
    }

    std::cout << "Total: " << res.size() << ". Max length: " << max_length << ". ";
    for (uint64_t i = 0; i < counts.size(); i++) {
        std::cout << i << ": " << counts[i] << ", ";
    }
    std::cout << std::endl;

    std::cout << name << ", " << numCycles << ", " << constructive << ", "
              << elapsedTimeMilliS.count() << ", " << g.numberOfNodes() << ", " << g.numberOfEdges()
              << ", " << also << std::endl;
}

void timedEnumReport(const std::string &name, const Graph &g, uint64_t numCycles, bool constructive,
                     const std::vector<node> &nodeOrder, const std::string &also,
                     const uint64_t interval) {

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    NetworKit::count enumCycles = 0;
    NetworKit::ChordlessCyclesUno41 cc(
        g,
        [&begin, &enumCycles, &interval](const std::vector<node> &p) {
            enumCycles += 1;
            if (enumCycles % interval == 0) {
                std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
                auto elapsedTime = end - begin;
                auto elapsedTimeMilliS = duration_cast<milliseconds>(elapsedTime);
                std::cout << enumCycles << ", " << elapsedTimeMilliS.count() << std::endl;
            }
            return true;
        },
        nodeOrder, numCycles, constructive);

    cc.run();
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    auto elapsedTime = end - begin;
    auto elapsedTimeMilliS = duration_cast<milliseconds>(elapsedTime);

    std::cout << std::endl;

    std::cout << name << ", " << numCycles << ", " << constructive << ", "
              << elapsedTimeMilliS.count() << ", " << g.numberOfNodes() << ", " << g.numberOfEdges()
              << ", " << also << std::endl;
}

std::vector<std::string> collectGraphs(const std::string &path, bool original = false) {
    std::vector<std::string> graphs;

    for (const auto &entry : fs::directory_iterator(path)) {
        const fs::path &filePath(entry);
        if ((filePath.extension() == ".el")) {
            if (original && (filePath.string().find("bbicon") == std::string::npos)
                && (filePath.string().find("prep") == std::string::npos)) {
                graphs.push_back(entry.path());
            }
            if (!original && (filePath.string().find("bbicon") != std::string::npos)) {
                graphs.push_back(entry.path());
            }
        }
    }
    return graphs;
}

/*
 * Sorry, because of last minute changes this is somewhat messy
 */
int main(int argc, char *argv[]) {


    NetworKit::EdgeListReader r('\t', 0, "#");
    NetworKit::EdgeListWriter w(' ', 0);

    std::string smallPaths = "/home/";
    std::string mediumPaths = "/home/";
    std::string largePaths = "/home/";
    std::string micpPaths = "/home/";

    std::vector<std::string> smallGraphsOrig = collectGraphs(smallPaths, true);
    std::vector<std::string> mediumGraphsOrig = collectGraphs(mediumPaths, true);
    std::vector<std::string> largeGraphsOrig = collectGraphs(largePaths, true);
    std::vector<std::string> micpGraphsOrig = collectGraphs(micpPaths, true);

    std::vector<std::string> smallGraphsPre = collectGraphs(smallPaths, false);
    std::vector<std::string> mediumGraphsPre = collectGraphs(mediumPaths, false);
    std::vector<std::string> largeGraphsPre = collectGraphs(largePaths, false);

    std::vector<std::pair<std::string, uint64_t>> origExperiments;
    std::vector<std::pair<std::string, uint64_t>> preExperiments;

    for (auto &p : smallGraphsOrig) {
        origExperiments.emplace_back(p, 4000000);
    }
    for (auto &p : mediumGraphsOrig) {
        break;
        origExperiments.emplace_back(p, 4000000);
    }
    for (auto &p : largeGraphsOrig) {
        break;
        origExperiments.emplace_back(p, 5000);
    }

    for (auto &p : smallGraphsPre) {
        preExperiments.emplace_back(p, 4000000);
    }
    for (auto &p : mediumGraphsPre) {
        break;
        preExperiments.emplace_back(p, 4000000);
    }
    for (auto &p : largeGraphsPre) {
        break;
        preExperiments.emplace_back(p, 5000);
    }
    for (auto &p : micpGraphsOrig) {
        break;
        preExperiments.emplace_back(p, 4000000);
    }

    printVector(origExperiments);
    printVector(preExperiments);

    if (true) {
        for (const auto &exp : origExperiments) {
            auto graphPath = exp.first;
            auto numCycles = exp.second;

            if (graphPath.find("polbooks") == std::string::npos)
                continue;

            std::cout << "---  " << graphPath << "  ---" << std::endl;

            Graph g = r.read(graphPath);
            g.shrinkToFit();
            g.sortEdges();
            g.indexEdges();

            for (auto i : {0, 1, 2, 3, 4}) {
                timedEnum(graphPath, g, numCycles, false, NetworKit::randomOrderedNodes(g, i),
                          "random, " + std::to_string(i));
            }
        }
    }

    std::reverse(preExperiments.begin(), preExperiments.end());
    for (const auto &exp : preExperiments) {
        auto graphPath = exp.first;
        auto numCycles = exp.second;

        if (graphPath.find("polbooks") == std::string::npos)
            continue;

        std::cout << "---  " << graphPath << "  ---" << std::endl;

        Graph g = r.read(graphPath);
        g.shrinkToFit();
        g.sortEdges();
        g.indexEdges();

        auto smallestFirstNodeOrder = timedSmallestFirstOrder(graphPath, g);
        std::vector<node> smallestFirstReverseNodeOrder(smallestFirstNodeOrder.rbegin(),
                                                        smallestFirstNodeOrder.rend());



        timedEnum(graphPath, g, numCycles, true, smallestFirstReverseNodeOrder,
                  "smallest_first, reverse");

        continue ;

        timedEnum(graphPath, g, numCycles, false, smallestFirstReverseNodeOrder,
                  "smallest_first, reverse");

        for (auto i : {0, 1, 2}) {
            timedEnum(graphPath, g, numCycles, false, NetworKit::randomOrderedNodes(g, i),
                      "random, " + std::to_string(i));
            timedEnum(graphPath, g, numCycles, true, NetworKit::randomOrderedNodes(g, i),
                      "random, " + std::to_string(i));
        }
    }


}
