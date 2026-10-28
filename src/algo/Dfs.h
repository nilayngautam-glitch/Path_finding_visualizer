#pragma once
#include <stack>
#include <utility>
#include <chrono>
#include <algorithm>
#include "algo/SearchResult.h"

// DFS with an explicit stack of (cell, parentCandidate) pairs.
// A cell is marked seen when POPPED, and its parent is fixed at that moment.
// Marking on push would break true depth-first order and could give a wrong parent.
// No shortest-path guarantee. O(V+E).
inline SearchResult dfs(const Grid& g) {
    SearchResult res;
    int n = g.rows * g.cols;
    res.parent.assign(n, -1);
    std::vector<char> seen(n, 0);
    std::stack<std::pair<int, int>> st;

    auto t0 = std::chrono::steady_clock::now();
    st.push({g.start, -1});
    while (!st.empty()) {
        res.peakFrontier = std::max(res.peakFrontier, (int)st.size());
        auto [u, p] = st.top();
        st.pop();
        if (seen[u]) continue;          // stale duplicate, skip
        seen[u] = 1;
        res.parent[u] = p;
        res.visitOrder.push_back(u);
        if (u == g.end) { res.found = true; break; }

        auto nb = g.neighbors(u);       // up, right, down, left
        for (auto it = nb.rbegin(); it != nb.rend(); ++it)   // reversed so "up" is popped first
            if (!seen[*it]) st.push({*it, u});
    }
    auto t1 = std::chrono::steady_clock::now();
    res.microseconds = std::chrono::duration<double, std::micro>(t1 - t0).count();
    buildPath(g, res);
    return res;
}