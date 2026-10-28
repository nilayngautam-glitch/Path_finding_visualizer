#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <optional>
#include <random>
#include <sstream>
#include <string>
#include <vector>
#include "algo/Bfs.h"
#include "algo/Dfs.h"
#include "algo/Dijkstra.h"

namespace {
constexpr int ROWS = 25, COLS = 41, CELL = 23, HUD = 120;   // ROWS, COLS odd (needed by maze)
constexpr int TERRAIN_COST = 5;

enum class St { Idle, Running, Paused, Finished };
enum class Drag { None, Start, End, Paint, Erase };

const char* NAMES[3] = {"BFS (queue)", "DFS (stack)", "Dijkstra (min-heap)"};

const char* stateName(St s) {
    switch (s) {
        case St::Idle: return "Idle";
        case St::Running: return "Running";
        case St::Paused: return "Paused";
        default: return "Finished";
    }
}

std::string statsLine(const char* name, const SearchResult& r) {
    std::ostringstream o;
    o << name << ": ";
    if (!r.found) o << "NO PATH | visited " << r.visitOrder.size();
    else o << "visited " << r.visitOrder.size() << " | length " << r.path.size() - 1
           << " | cost " << r.cost << " | peak frontier " << r.peakFrontier;
    o << " | " << std::fixed << std::setprecision(1) << r.microseconds << " us";
    return o.str();
}

// Recursive backtracking with an explicit stack, then "braiding" (opens ~8% of walls)
// so several routes exist and the algorithms can be compared.
void makeMaze(Grid& g, std::mt19937& rng) {
    const int R = g.rows, C = g.cols;
    for (auto& c : g.cells) { c.wall = true; c.cost = 1; }
    const int dr[4] = {-2, 0, 2, 0}, dc[4] = {0, 2, 0, -2};
    std::vector<int> st;
    g.cells[g.id(1, 1)].wall = false;
    st.push_back(g.id(1, 1));
    while (!st.empty()) {
        int u = st.back(), r = u / C, c = u % C;
        int opts[4], k = 0;
        for (int d = 0; d < 4; d++) {
            int nr = r + dr[d], nc = c + dc[d];
            if (nr > 0 && nc > 0 && nr < R - 1 && nc < C - 1 && g.cells[g.id(nr, nc)].wall) opts[k++] = d;
        }
        if (k == 0) { st.pop_back(); continue; }
        int d = opts[rng() % k];
        g.cells[g.id(r + dr[d] / 2, c + dc[d] / 2)].wall = false;
        g.cells[g.id(r + dr[d], c + dc[d])].wall = false;
        st.push_back(g.id(r + dr[d], c + dc[d]));
    }
    for (int r = 1; r < R - 1; r++)
        for (int c = 1; c < C - 1; c++)
            if ((r + c) % 2 == 1 && g.cells[g.id(r, c)].wall && rng() % 100 < 8)
                g.cells[g.id(r, c)].wall = false;
    g.start = g.id(1, 1);
    g.end = g.id(R - 2, C - 2);
}
}  // namespace

int main() {
    Grid grid(ROWS, COLS);
    grid.start = grid.id(12, 5);
    grid.end = grid.id(12, 35);
    const int N = ROWS * COLS;
    std::mt19937 rng(std::random_device{}());

    int algo = 0, speed = 3;                 // speed = events revealed per frame
    bool terrainMode = false;
    St state = St::Idle;
    Drag drag = Drag::None;
    SearchResult res;
    size_t vi = 0, pi = 0;
    std::vector<char> mark(N, 0);            // 0 none, 1 visited, 2 path
    std::string stats[3];

    auto clearPath = [&]() {
        res = SearchResult();
        vi = pi = 0;
        std::fill(mark.begin(), mark.end(), 0);
        state = St::Idle;
    };
    auto boardChanged = [&]() { clearPath(); for (auto& s : stats) s.clear(); };
    auto startRun = [&]() {
        clearPath();
        res = (algo == 0) ? bfs(grid) : (algo == 1) ? dfs(grid) : dijkstra(grid);
        state = St::Running;
    };
    auto resetBoard = [&]() {
        for (auto& c : grid.cells) c = Cell();
        grid.start = grid.id(12, 5);
        grid.end = grid.id(12, 35);
        boardChanged();
    };
    auto cellAt = [&](sf::Vector2i p) -> int {
        if (p.x < 0 || p.y < 0 || p.x >= COLS * CELL || p.y >= ROWS * CELL) return -1;
        return grid.id(p.y / CELL, p.x / CELL);
    };
    auto applyDrag = [&](int cell) {
        if (state == St::Running || state == St::Paused) return;   // no editing mid-search
        if (drag == Drag::Start) {
            if (cell != grid.end && cell != grid.start) { grid.start = cell; grid.cells[cell].wall = false; boardChanged(); }
        } else if (drag == Drag::End) {
            if (cell != grid.start && cell != grid.end) { grid.end = cell; grid.cells[cell].wall = false; boardChanged(); }
        } else if (drag == Drag::Paint) {
            if (cell == grid.start || cell == grid.end) return;
            Cell& c = grid.cells[cell];
            bool wantWall = !terrainMode;
            uint8_t nc = terrainMode ? TERRAIN_COST : 1;
            if (c.wall != wantWall || c.cost != nc) { c.wall = wantWall; c.cost = nc; boardChanged(); }
        } else if (drag == Drag::Erase) {
            if (cell == grid.start || cell == grid.end) return;
            Cell& c = grid.cells[cell];
            if (c.wall || c.cost != 1) { c.wall = false; c.cost = 1; boardChanged(); }
        }
    };

    sf::RenderWindow window(sf::VideoMode({(unsigned)(COLS * CELL), (unsigned)(ROWS * CELL + HUD)}),
                            "Pathfinding Visualizer");
    window.setFramerateLimit(60);

    sf::Font font;
    bool haveFont = font.openFromFile("C:/Windows/Fonts/consola.ttf") ||
                    font.openFromFile("C:/Windows/Fonts/arial.ttf");
    sf::Text label(font, "", 14);
    label.setFillColor(sf::Color(230, 230, 235));

    sf::RectangleShape rect(sf::Vector2f(CELL - 1.f, CELL - 1.f));
    sf::RectangleShape hudBg(sf::Vector2f((float)(COLS * CELL), (float)HUD));
    hudBg.setPosition({0.f, (float)(ROWS * CELL)});
    hudBg.setFillColor(sf::Color(28, 30, 38));
    std::string lastTitle;

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            } else if (const auto* mb = event->getIf<sf::Event::MouseButtonPressed>()) {
                int c = cellAt(mb->position);
                if (c >= 0 && state != St::Running && state != St::Paused) {
                    if (mb->button == sf::Mouse::Button::Left)
                        drag = (c == grid.start) ? Drag::Start : (c == grid.end) ? Drag::End : Drag::Paint;
                    else if (mb->button == sf::Mouse::Button::Right)
                        drag = Drag::Erase;
                }
            } else if (event->is<sf::Event::MouseButtonReleased>()) {
                drag = Drag::None;
            } else if (const auto* kp = event->getIf<sf::Event::KeyPressed>()) {
                const auto k = kp->code;
                bool editable = (state == St::Idle || state == St::Finished);
                if (k == sf::Keyboard::Key::Escape) window.close();
                else if (k == sf::Keyboard::Key::Num1 && editable) { clearPath(); algo = 0; }
                else if (k == sf::Keyboard::Key::Num2 && editable) { clearPath(); algo = 1; }
                else if (k == sf::Keyboard::Key::Num3 && editable) { clearPath(); algo = 2; }
                else if (k == sf::Keyboard::Key::Space) {
                    if (editable) startRun();
                    else if (state == St::Running) state = St::Paused;
                    else if (state == St::Paused) state = St::Running;
                }
                else if (k == sf::Keyboard::Key::C) clearPath();
                else if (k == sf::Keyboard::Key::R) resetBoard();
                else if (k == sf::Keyboard::Key::M && editable) { makeMaze(grid, rng); boardChanged(); }
                else if (k == sf::Keyboard::Key::T) terrainMode = !terrainMode;
                else if (k == sf::Keyboard::Key::Up) speed = std::min(256, speed * 2);
                else if (k == sf::Keyboard::Key::Down) speed = std::max(1, speed / 2);
            }
        }

        if (drag != Drag::None) {
            int c = cellAt(sf::Mouse::getPosition(window));
            if (c >= 0) applyDrag(c);
        }

        // Replay: visited cells first, then the final path
        if (state == St::Running) {
            for (int k = 0; k < speed && state == St::Running; k++) {
                if (vi < res.visitOrder.size()) mark[res.visitOrder[vi++]] = 1;
                else if (pi < res.path.size()) mark[res.path[pi++]] = 2;
                else { state = St::Finished; stats[algo] = statsLine(NAMES[algo], res); }
            }
        }

        window.clear(sf::Color(190, 190, 196));
        for (int r = 0; r < ROWS; r++) {
            for (int c = 0; c < COLS; c++) {
                int id = grid.id(r, c);
                const Cell& cell = grid.cells[id];
                sf::Color col = cell.wall ? sf::Color(30, 30, 40)
                              : cell.cost > 1 ? sf::Color(176, 132, 82)
                              : sf::Color(245, 245, 248);
                rect.setPosition({(float)(c * CELL), (float)(r * CELL)});
                rect.setFillColor(col);
                window.draw(rect);
                if (mark[id] == 1) { rect.setFillColor(sf::Color(60, 140, 255, 160)); window.draw(rect); }
                else if (mark[id] == 2) { rect.setFillColor(sf::Color(255, 214, 10, 240)); window.draw(rect); }
                if (id == grid.start) { rect.setFillColor(sf::Color(46, 184, 92)); window.draw(rect); }
                else if (id == grid.end) { rect.setFillColor(sf::Color(230, 57, 70)); window.draw(rect); }
            }
        }

        std::vector<std::string> lines;
        lines.push_back(std::string("Algorithm: ") + NAMES[algo] + "   Brush: " +
                        (terrainMode ? "Terrain (cost 5)" : "Wall") + "   Speed: " +
                        std::to_string(speed) + "   State: " + stateName(state));
        lines.push_back("1 BFS | 2 DFS | 3 Dijkstra | Space run/pause | C clear path | R reset | M maze | T terrain | Up/Down speed");
        lines.push_back("Left-drag: draw (drag the green/red cells to move start/end) | Right-drag: erase");
        for (int i = 0; i < 3; i++)
            lines.push_back(stats[i].empty() ? std::string(NAMES[i]) + ": -" : stats[i]);

        window.draw(hudBg);
        if (haveFont) {
            for (size_t i = 0; i < lines.size(); i++) {
                label.setString(lines[i]);
                label.setPosition({8.f, (float)(ROWS * CELL + 6 + i * 16)});
                window.draw(label);
            }
        } else if (lines[0] != lastTitle) {       // no font found: show status in the title bar
            window.setTitle(lines[0]);
            lastTitle = lines[0];
        }
        window.display();
    }
    return 0;
}