#include <cassert>
#include <iostream>
#include "algo/Bfs.h"

int main() {
    Grid g(5, 5);                       // T1: open 5x5, corner to corner
    auto r = bfs(g);
    assert(r.found && r.path.size() == 9 && r.cost == 8);

    Grid h(3, 3);                       // T5: end walled in
    h.cells[h.id(1, 2)].wall = true;
    h.cells[h.id(2, 1)].wall = true;
    auto r2 = bfs(h);
    assert(!r2.found && !r2.visitOrder.empty());

    Grid s(4, 4); s.end = s.start;      // T6: start == end
    auto r3 = bfs(s);
    assert(r3.found && r3.path.size() == 1 && r3.cost == 0);

    std::cout << "all tests passed\n";
}