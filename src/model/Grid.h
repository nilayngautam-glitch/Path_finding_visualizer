#pragma once
#include<bits/stdc++.h>
using namespace std;

struct Cell {
    bool wall = false; // wheather a cell is blocked    
    uint8_t cost = 1;  
};

struct Grid {
    int rows, cols, start, end;
    vector<Cell> cells;

    Grid(int r, int c){
        rows = r;
        cols = c;
        start = 0;
        end = r*c-1;
        cells.resize(r*c);
    }

    int id(int r, int c) const { return r * cols + c; }

    // order is fixed: up, right, down, left
    vector<int> neighbors(int u) const { 
        static const int dr[] = {-1, 0, 1, 0};  // change in row
        static const int dc[] = {0, 1, 0, -1};  // change in column 
        /*
          k	  dr[k]	 dc[k]	 Direction
          0	    -1	    0	    Up
          1	     0	    1	   Right
          2	     1 	    0	   Down
          3	     0	   -1	   Left
        */
        vector<int> out;
         // u is a cell ID
        int r = u / cols;  // Both r and c convert cell ID's back to co-ordinates
        int c = u % cols; 
        for (int k = 0; k < 4; k++) {
            int nr = r + dr[k], nc = c + dc[k];
            if (nr < 0 || nc < 0 || nr >= rows || nc >= cols) continue; // Skips the iteration if neighbours are outside the grid
            int v = id(nr, nc);  // Converts neighbour cell to ID's
            if (!cells[v].wall) out.push_back(v);
        }
        return out;
    }
};