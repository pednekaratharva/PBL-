#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <queue>
#include <limits>

using namespace std;

// Structure to represent a cell in the maze
struct Cell {
    int row, col;
    Cell(int r = 0, int c = 0) : row(r), col(c) {}
    bool operator==(const Cell& other) const {
        return row == other.row && col == other.col;
    }
};

// (Visualization and wall-following removed)

void printMazeWithPath(vector<string>& maze, vector<Cell>& path, int startRow, int startCol, int endRow, int endCol) {
    int height = maze.size();
    int width = maze[0].size();
    
    // Create a copy of the maze for visualization
    vector<string> display = maze;
    
    // Mark path cells
    for (const Cell& cell : path) {
        if (cell.row == startRow && cell.col == startCol) {
            display[cell.row][cell.col] = 'S';
        } else if (cell.row == endRow && cell.col == endCol) {
            display[cell.row][cell.col] = 'E';
        } else {
            display[cell.row][cell.col] = '*';
        }
    }
    
    // Print the maze
    cout << "\nMaze with path (S=Start, E=End, *=Path, 1=Wall, 0=Empty):\n";
    for (int i = 0; i < height; i++) {
        cout << display[i] << endl;
    }
}

// Dijkstra's shortest path on a grid maze (cost 1 per move)
// Returns the shortest path from start to end
vector<Cell> solveMazeDijkstra(vector<string>& maze, int startRow, int startCol,
                               int endRow, int endCol) {
    const int height = static_cast<int>(maze.size());
    const int width = height > 0 ? static_cast<int>(maze[0].size()) : 0;
    if (height == 0 || width == 0) return {};

    // Movements: up, right, down, left
    const int dr[4] = {-1, 0, 1, 0};
    const int dc[4] = {0, 1, 0, -1};

    const int INF = std::numeric_limits<int>::max();
    vector<vector<int>> dist(height, vector<int>(width, INF));
    vector<vector<Cell>> parent(height, vector<Cell>(width, Cell(-1, -1)));

    struct Node {
        int dist;
        int row;
        int col;
        bool operator>(const Node& other) const { return dist > other.dist; }
    };

    auto isFree = [&](int r, int c) -> bool {
        if (r < 0 || r >= height || c < 0 || c >= width) return false;
        // Treat any non-'1' as traversable (includes '0', 'S', 'E')
        return maze[r][c] != '1';
    };

    priority_queue<Node, vector<Node>, greater<Node>> pq;
    dist[startRow][startCol] = 0;
    pq.push(Node{0, startRow, startCol});

    while (!pq.empty()) {
        Node cur = pq.top();
        pq.pop();
        if (cur.dist != dist[cur.row][cur.col]) continue; // stale
        if (cur.row == endRow && cur.col == endCol) break;

        for (int k = 0; k < 4; k++) {
            int nr = cur.row + dr[k];
            int nc = cur.col + dc[k];
            if (!isFree(nr, nc)) continue;
            int nd = cur.dist + 1; // uniform cost
            if (nd < dist[nr][nc]) {
                dist[nr][nc] = nd;
                parent[nr][nc] = Cell(cur.row, cur.col);
                pq.push(Node{nd, nr, nc});
            }
        }
    }

    if (dist[endRow][endCol] == INF) return {};

    // Reconstruct shortest path
    vector<Cell> path;
    for (Cell at = Cell(endRow, endCol); at.row != -1; at = parent[at.row][at.col]) {
        path.push_back(at);
        if (at.row == startRow && at.col == startCol) break;
    }
    reverse(path.begin(), path.end());

    return path;
}

int main() {
    ifstream file("maze.txt");
    if (!file.is_open()) {
        cerr << "Error: Could not open maze.txt" << endl;
        return 1;
    }
    
    int height, width;
    int startRow, startCol;
    int endRow, endCol;
    
    // Read dimensions
    file >> height >> width;
    // Read start position
    file >> startRow >> startCol;
    // Read end position
    file >> endRow >> endCol;
    
    // Read the maze
    vector<string> maze(height);
    string line;
    getline(file, line); // Consume the newline after end position
    
    for (int i = 0; i < height; i++) {
        getline(file, maze[i]);
    }
    file.close();
    
    cout << "Maze dimensions: " << height << " x " << width << endl;
    cout << "Start: (" << startRow << ", " << startCol << ")" << endl;
    cout << "End: (" << endRow << ", " << endCol << ")" << endl;
    // Solve using Dijkstra's shortest path
    vector<Cell> shortestPath = solveMazeDijkstra(maze, startRow, startCol, endRow, endCol);

    if (!shortestPath.empty()) {
        cout << "\n=== Dijkstra's Results ===" << endl;
        cout << "Shortest path found! Length: " << shortestPath.size() << " cells" << endl;
        printMazeWithPath(maze, shortestPath, startRow, startCol, endRow, endCol);
    } else {
        cout << "No path found by Dijkstra's algorithm!" << endl;
    }
    
    return 0;
}
