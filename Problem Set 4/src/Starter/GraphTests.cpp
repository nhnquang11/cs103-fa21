#include "GUI/SimpleTest.h"
#include "GraphEditor/Utilities/JSON.h"
#include <set>
#include <string>
#include <utility>
#include <fstream>
using namespace std;

namespace {
    using Graph = pair<set<string>, set<pair<string, string>>>;

    Graph decodeGraph(const string& filename) {
        ifstream input(filename);
        if (!input) SHOW_ERROR("Cannot open file " + filename);

        /* Raw JSON data. */
        JSON j = JSON::parse(input);

        Graph result;

        /* Map from index to name. */
        map<size_t, string> toName;
        for (JSON node: j["nodes"]) {
            toName[node["index"].asInteger()] = node["label"].asString();
            result.first.insert(node["label"].asString());
        }

        /* Load edges, bidirectionally. */
        for (JSON edge: j["edges"]) {
            auto e = make_pair(toName.at(edge["from"].asInteger()), toName.at(edge["to"].asInteger()));
            if (e.first == e.second) SHOW_ERROR("Undirected graphs cannot have self-loops.");

            result.second.insert(e);
            result.second.insert(make_pair(e.second, e.first));
        }

        return result;
    }

    /* Utility: Is there an edge between these nodes? */
    bool edgeBetween(const Graph& g, const string& u, const string& v) {
        return g.second.count(make_pair(u, v));
    }

    /* Utility: Degree of a node. */
    size_t degreeOf(const Graph& g, const string& v) {
        size_t result = 0;
        for (auto entry: g.second) {
            /* Only check one direction; other direction will double the total. */
            if (entry.first == v) {
                result++;
            }
        }
        return result;
    }

    /* Literal translation of the FOL definition. */
    bool isHighlyIrregular(const Graph& graph) {
        /*
         * ∀v ∈ V.
         *    ∀x ∈ V.
         *       ∀y ∈ V. (x ≠ y  ∧  {v, x} ∈ E  ∧  {v, y} ∈ E  →  deg(x) ≠ deg(y)).
         */
        for (auto v: graph.first) {
            for (auto x: graph.first) {
                for (auto y: graph.first) {
                    if (x != y && edgeBetween(graph, v, x) && edgeBetween(graph, v, y) &&
                        degreeOf(graph, x) == degreeOf(graph, y)) {
                        return false;
                    }
                }
            }
        }

        return true;
    }
}

PROVIDED_TEST("Linkage.graph") {
    /* For any nodes u, v ∈ V where u ≠ v,
     *    there is exactly one node z ∈ V where
     *        {u, z} ∈ E and {v, z} ∈ E.
     */
    auto graph = decodeGraph("res/Linkage.graph");
    if (graph.first.size() != 7) {
        SHOW_ERROR("Graph does not have seven nodes.");
    }

    for (auto u: graph.first) {
        for (auto v: graph.first) {
            if (u != v) {
                size_t count = 0;
                for (auto z: graph.first) {
                    if (edgeBetween(graph, u, z) && edgeBetween(graph, v, z)) {
                        count++;
                    }
                }
                if (count != 1) {
                    SHOW_ERROR("Graph is not a linkage graph.");
                }
            }
        }
    }
}

#ifdef USE_HIGHLY_IRREGULAR_GRAPHS
PROVIDED_TEST("HighlyIrregular.graph") {
    auto graph = decodeGraph("res/HighlyIrregular.graph");

    if (!isHighlyIrregular(graph)) {
        SHOW_ERROR("Graph is not highly irregular.");
    }

    /* Has a triangle. */
    bool hasTriangle = false;
    for (auto u: graph.first) {
        for (auto v: graph.first) {
            for (auto x: graph.first) {
                if (edgeBetween(graph, u, v) &&
                    edgeBetween(graph, v, x) &&
                    edgeBetween(graph, x, u)) {
                    hasTriangle = true;
                }
            }
        }
    }

    if (!hasTriangle) SHOW_ERROR("Graph has no triangles.");

    /* Not too many nodes. */
    if (graph.first.size() > 10) {
        SHOW_ERROR("This graph works, but you need to use 10 or fewer nodes to receive credit.");
    }
}
#endif
