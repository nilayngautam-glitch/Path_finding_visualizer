#pragma once
#include <vector>
#include <cstdint>

struct Cell { bool wall = false; uint8_t cost = 1; };

struct Grid {
    int rows, cols, start, end;
    std::vector<Cell> cells;
    Grid(int r, int c) : rows(r), cols(c), start(0), end(r * c - 1), cells(r * c) {}
    int id(int r, int c) const { return r * cols + c; }

    // order is fixed: up, right, down, left
    std::vector<int> neighbors(int u) const {
        static const int dr[] = {-1, 0, 1, 0}, dc[] = {0, 1, 0, -1};
        std::vector<int> out;
        int r = u / cols, c = u % cols;
        for (int k = 0; k < 4; k++) {
            int nr = r + dr[k], nc = c + dc[k];
            if (nr < 0 || nc < 0 || nr >= rows || nc >= cols) continue;
            int v = id(nr, nc);
            if (!cells[v].wall) out.push_back(v);
        }
        return out;
    }
};