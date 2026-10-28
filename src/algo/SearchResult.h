#pragma once
#include <vector>
#include "model/Grid.h"
#include <algorithm>
using namespace std;

struct SearchResult {
    bool found = false;  // if Bfs reached the destination 
    vector<int> visitOrder, path, parent;
    long long cost = 0;
    int peakFrontier = 0; // 	Maximum number of cells simultaneously waiting in the BFS queue, measured at the points where the code checks the queue size.
    double microseconds = 0;
};

inline void buildPath(const Grid& g, SearchResult& r) {
    if (!r.found) return;
    for (int v = g.end; v != -1; v = r.parent[v]) r.path.push_back(v);
    reverse(r.path.begin(), r.path.end());
    r.cost = 0;
    for (size_t i = 1; i < r.path.size(); i++) r.cost += g.cells[r.path[i]].cost;
}