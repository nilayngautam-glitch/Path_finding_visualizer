#include <cassert>
#include <cstdlib>
#include <iostream>
#include "algo/Bfs.h"
#include "algo/Dfs.h"

bool validPath(const Grid& g, const SearchResult& r) {
    if (r.path.empty() || r.path.front() != g.start || r.path.back() != g.end) return false;
    for (size_t i = 0; i < r.path.size(); i++) {
        if (g.cells[r.path[i]].wall) return false;
        if (i > 0) {
            int a = r.path[i - 1], b = r.path[i];
            int d = std::abs(a / g.cols - b / g.cols) + std::abs(a % g.cols - b % g.cols);
            if (d != 1) return false;
        }
    }
    return true;
}

int main() {
    Grid g(5, 5);                       // T1: BFS on open grid
    auto r = bfs(g);
    assert(r.found && r.path.size() == 9 && r.cost == 8);

    Grid h(3, 3);                       // T5: end walled in
    h.cells[h.id(1, 2)].wall = true;
    h.cells[h.id(2, 1)].wall = true;
    assert(!bfs(h).found && !bfs(h).visitOrder.empty());

    Grid s(4, 4); s.end = s.start;      // T6: start == end
    auto r3 = bfs(s);
    assert(r3.found && r3.path.size() == 1 && r3.cost == 0);

    // DFS tests
    auto d = dfs(g);                    // T2: valid path, not shorter than BFS
    assert(d.found && validPath(g, d) && d.path.size() >= r.path.size());

    auto d2 = dfs(h);                   // unreachable
    assert(!d2.found && !d2.visitOrder.empty());

    auto d3 = dfs(s);                   // start == end
    assert(d3.found && d3.path.size() == 1);

    std::cout << "all tests passed\n";
}