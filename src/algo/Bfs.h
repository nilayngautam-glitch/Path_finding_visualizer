#pragma once
#include <queue>
#include <chrono>
#include "algo/SearchResult.h"

inline SearchResult bfs(const Grid& g) {
    SearchResult res;
    int n = g.rows * g.cols;
    res.parent.assign(n, -1);
    std::vector<char> seen(n, 0);
    std::queue<int> q;

    auto t0 = std::chrono::steady_clock::now();
    q.push(g.start); seen[g.start] = 1;
    while (!q.empty()) {
        res.peakFrontier = std::max(res.peakFrontier, (int)q.size());
        int u = q.front(); q.pop();
        res.visitOrder.push_back(u);
        if (u == g.end) { res.found = true; break; }
        for (int v : g.neighbors(u))
            if (!seen[v]) { seen[v] = 1; res.parent[v] = u; q.push(v); }
    }
    auto t1 = std::chrono::steady_clock::now();
    res.microseconds = std::chrono::duration<double, std::micro>(t1 - t0).count();
    buildPath(g, res);
    return res;
}