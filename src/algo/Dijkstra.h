#pragma once
#include <queue>
#include <vector>
#include <utility>
#include <functional>
#include <chrono>
#include <algorithm>
#include "algo/SearchResult.h"

// Dijkstra with a min-heap: std::priority_queue<pair<dist,id>, vector, greater>.
// Cost model: the cost of a move is the cost of the cell being ENTERED (start costs 0).
// std::priority_queue has no decrease-key, so we push duplicates and skip stale
// entries on pop (lazy deletion). Needs costs >= 1. O(E log V).
inline SearchResult dijkstra(const Grid& g) {
    SearchResult res;
    int n = g.rows * g.cols;
    const long long INF = (1LL << 60);
    res.parent.assign(n, -1);
    std::vector<long long> best(n, INF);
    using P = std::pair<long long, int>;
    std::priority_queue<P, std::vector<P>, std::greater<P>> pq;

    auto t0 = std::chrono::steady_clock::now();
    best[g.start] = 0;
    pq.push({0, g.start});
    while (!pq.empty()) {
        res.peakFrontier = std::max(res.peakFrontier, (int)pq.size());
        auto [d, u] = pq.top();
        pq.pop();
        if (d > best[u]) continue;          // stale entry, skip
        res.visitOrder.push_back(u);
        if (u == g.end) { res.found = true; break; }
        for (int v : g.neighbors(u)) {
            long long nd = d + g.cells[v].cost;
            if (nd < best[v]) {
                best[v] = nd;
                res.parent[v] = u;
                pq.push({nd, v});
            }
        }
    }
    auto t1 = std::chrono::steady_clock::now();
    res.microseconds = std::chrono::duration<double, std::micro>(t1 - t0).count();
    buildPath(g, res);
    return res;
}