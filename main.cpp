#include <bits/stdc++.h>
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#pragma comment(lib, "Psapi.lib")
#endif

using namespace std;

struct Cell { int row, col; };

static bool operator==(const Cell &a, const Cell &b) { return a.row == b.row && a.col == b.col; }

// Read maze from maze.txt (format matches your generator)
// First line: height width
// Second: start_row start_col
// Third: end_row end_col
// Then height lines of maze using '1','0','S','E'
struct MazeData {
    int height{}, width{};
    int sr{}, sc{}, er{}, ec{};
    vector<string> grid;
};

static bool readMaze(const string &path, MazeData &m) {
    ifstream in(path);
    if (!in.is_open()) return false;
    in >> m.height >> m.width;
    in >> m.sr >> m.sc;
    in >> m.er >> m.ec;
    string line; getline(in, line);
    m.grid.resize(m.height);
    for (int i = 0; i < m.height; ++i) getline(in, m.grid[i]);
    return true;
}

static bool isFree(const MazeData &m, int r, int c) {
    if (r < 0 || r >= m.height || c < 0 || c >= m.width) return false;
    return m.grid[r][c] != '1';
}

// Pretty print maze with a path
static void printMazeWithPath(const MazeData &m, const vector<Cell> &path) {
    vector<string> disp = m.grid;
    for (const auto &p : path) {
        if (p.row == m.sr && p.col == m.sc) disp[p.row][p.col] = 'S';
        else if (p.row == m.er && p.col == m.ec) disp[p.row][p.col] = 'E';
        else disp[p.row][p.col] = '*';
    }
    cout << "\nMaze with path (S,E,*,1,0):\n";
    for (auto &row : disp) cout << row << '\n';
}

#ifdef _WIN32
static size_t getCurrentMemoryBytes() {
    PROCESS_MEMORY_COUNTERS pmc{};
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return static_cast<size_t>(pmc.WorkingSetSize);
    }
    return 0;
}
#else
static size_t getCurrentMemoryBytes() { return 0; }
#endif

// Dijkstra (uniform cost)
static vector<Cell> solveDijkstra(const MazeData &m) {
    const int dr[4] = {-1,0,1,0};
    const int dc[4] = {0,1,0,-1};
    const int H = m.height, W = m.width;
    const int INF = INT_MAX/4;
    vector<vector<int>> dist(H, vector<int>(W, INF));
    vector<vector<Cell>> parent(H, vector<Cell>(W, Cell{-1,-1}));
    struct Node { int d,r,c; bool operator>(const Node&o) const {return d>o.d;} };
    priority_queue<Node, vector<Node>, greater<Node>> pq;
    dist[m.sr][m.sc] = 0; pq.push({0,m.sr,m.sc});
    while(!pq.empty()){
        auto cur = pq.top(); pq.pop();
        if (cur.d != dist[cur.r][cur.c]) continue;
        if (cur.r == m.er && cur.c == m.ec) break;
        for(int k=0;k<4;k++){
            int nr = cur.r+dr[k], nc = cur.c+dc[k];
            if(!isFree(m,nr,nc)) continue;
            int nd = cur.d+1;
            if(nd<dist[nr][nc]){ dist[nr][nc]=nd; parent[nr][nc]=Cell{cur.r,cur.c}; pq.push({nd,nr,nc}); }
        }
    }
    if (dist[m.er][m.ec] >= INF) return {};
    vector<Cell> path; Cell at{m.er,m.ec};
    for(;;){ path.push_back(at); if(at.row==m.sr && at.col==m.sc) break; at=parent[at.row][at.col]; }
    reverse(path.begin(), path.end());
    return path;
}

// BFS shortest path
static vector<Cell> solveBFS(const MazeData &m){
    const int dr[4] = {-1,0,1,0};
    const int dc[4] = {0,1,0,-1};
    const int H=m.height,W=m.width;
    vector<vector<int>> dist(H, vector<int>(W,-1));
    vector<vector<Cell>> parent(H, vector<Cell>(W, Cell{-1,-1}));
    queue<Cell> q; q.push({m.sr,m.sc}); dist[m.sr][m.sc]=0;
    while(!q.empty()){
        auto cur=q.front(); q.pop();
        if(cur.row==m.er && cur.col==m.ec) break;
        for(int k=0;k<4;k++){
            int nr=cur.row+dr[k], nc=cur.col+dc[k];
            if(!isFree(m,nr,nc) || dist[nr][nc]!=-1) continue;
            dist[nr][nc]=dist[cur.row][cur.col]+1; parent[nr][nc]=cur; q.push({nr,nc});
        }
    }
    if(dist[m.er][m.ec]==-1) return {};
    vector<Cell> path; Cell at{m.er,m.ec};
    for(;;){ path.push_back(at); if(at.row==m.sr && at.col==m.sc) break; at=parent[at.row][at.col]; }
    reverse(path.begin(), path.end());
    return path;
}

// DFS (stack) - may not be shortest
static vector<Cell> solveDFS(const MazeData &m){
    const int dr[4] = {-1,0,1,0};
    const int dc[4] = {0,1,0,-1};
    const int H=m.height,W=m.width;
    vector<vector<int>> vis(H, vector<int>(W,0));
    vector<vector<Cell>> parent(H, vector<Cell>(W, Cell{-1,-1}));
    stack<Cell> st; st.push({m.sr,m.sc}); vis[m.sr][m.sc]=1;
    while(!st.empty()){
        auto cur=st.top(); st.pop();
        if(cur.row==m.er && cur.col==m.ec) break;
        for(int k=0;k<4;k++){
            int nr=cur.row+dr[k], nc=cur.col+dc[k];
            if(!isFree(m,nr,nc) || vis[nr][nc]) continue;
            vis[nr][nc]=1; parent[nr][nc]=cur; st.push({nr,nc});
        }
    }
    if(!vis[m.er][m.ec]) return {};
    vector<Cell> path; Cell at{m.er,m.ec};
    for(;;){ path.push_back(at); if(at.row==m.sr && at.col==m.sc) break; at=parent[at.row][at.col]; }
    reverse(path.begin(), path.end());
    return path;
}

// Linear scan (very simple dynamic reachability, not optimal but deterministic)
static vector<Cell> solveLinear(const MazeData &m){
    const int H=m.height,W=m.width;
    vector<vector<int>> reachable(H, vector<int>(W,0));
    vector<vector<Cell>> parent(H, vector<Cell>(W, Cell{-1,-1}));
    reachable[m.sr][m.sc]=1;
    // Sweep until no change
    bool changed=true; int iter=0; const int dr[4]={-1,0,1,0}; const int dc[4]={0,1,0,-1};
    while(changed && ++iter<=H*W){
        changed=false;
        for(int r=0;r<H;r++) for(int c=0;c<W;c++) if(isFree(m,r,c)){
            if(reachable[r][c]) continue;
            for(int k=0;k<4;k++){
                int pr=r+dr[k], pc=c+dc[k];
                if(pr<0||pr>=H||pc<0||pc>=W) continue;
                if(reachable[pr][pc] && isFree(m,pr,pc)){
                    reachable[r][c]=1; parent[r][c]=Cell{pr,pc}; changed=true; break;
                }
            }
        }
    }
    if(!reachable[m.er][m.ec]) return {};
    vector<Cell> path; Cell at{m.er,m.ec};
    for(;;){ path.push_back(at); if(at.row==m.sr && at.col==m.sc) break; at=parent[at.row][at.col]; }
    reverse(path.begin(), path.end());
    return path;
}

struct RunResult {
    string name;
    vector<Cell> path;
    long long ms{};
    size_t memBytes{};
};

template <class Solver>
static RunResult runOne(const string &name, const MazeData &m, Solver solver){
    size_t memBefore = getCurrentMemoryBytes();
    auto t0 = chrono::steady_clock::now();
    auto path = solver(m);
    auto t1 = chrono::steady_clock::now();
    size_t memAfter = getCurrentMemoryBytes();
    RunResult r; r.name=name; r.path=std::move(path); r.ms = chrono::duration_cast<chrono::milliseconds>(t1-t0).count(); r.memBytes = (memAfter?memAfter:0); // working set snapshot
    return r;
}

static void saveResults(const vector<RunResult> &all, const RunResult &fastest, const string &outPath, const MazeData &m){
    ofstream out(outPath);
    out << "Maze: " << m.height << "x" << m.width << " Start:("<<m.sr<<","<<m.sc<<") End:("<<m.er<<","<<m.ec<<")\n\n";
    for (const auto &r: all){
        out << "Algorithm: " << r.name << "\n";
        out << "Time(ms): " << r.ms << "\n";
        out << "Memory(bytes, working set): " << r.memBytes << "\n";
        out << "Path length: " << r.path.size() << "\n";
        out << "Path: ";
        for (size_t i=0;i<r.path.size();++i){ out << "("<<r.path[i].row<<","<<r.path[i].col<<")" << (i+1<r.path.size()?" -> ":""); }
        out << "\n\n";
    }
    out << "FASTEST: " << fastest.name << " (" << fastest.ms << " ms)\n";
    out << "Fastest path length: " << fastest.path.size() << "\n";
    out << "Fastest path: ";
    for (size_t i=0;i<fastest.path.size();++i){ out << "("<<fastest.path[i].row<<","<<fastest.path[i].col<<")" << (i+1<fastest.path.size()?" -> ":""); }
    out << "\n";
}

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    MazeData m; if(!readMaze("maze.txt", m)){ cerr << "Failed to read maze.txt" << '\n'; return 1; }
    cout << "Maze: " << m.height << "x" << m.width << " Start:("<<m.sr<<","<<m.sc<<") End:("<<m.er<<","<<m.ec<<")\n";

    vector<RunResult> results;
    results.push_back(runOne("Dijkstra (custom.cpp)", m, solveDijkstra));
    results.push_back(runOne("BFS (BFS.cpp)", m, solveBFS));
    results.push_back(runOne("DFS (stl.cpp)", m, solveDFS));
    results.push_back(runOne("Linear Scan (mazesequential.cpp)", m, solveLinear));

    for (const auto &r: results){
        cout << "\n["<< r.name << "]\n";
        cout << "Time: " << r.ms << " ms\n";
        cout << "Memory (working set bytes): " << r.memBytes << "\n";
        cout << "Path length: " << r.path.size() << "\n";
        printMazeWithPath(m, r.path);
    }

    // Fastest by time
    const RunResult *fastest = nullptr;
    for (const auto &r: results){ if(!fastest || r.ms < fastest->ms) fastest = &r; }
    if (fastest){
        cout << "\nFASTEST: " << fastest->name << " (" << fastest->ms << " ms)\n";
    }

    saveResults(results, *fastest, "results.txt", m);
    ofstream fastestOut("fastest.txt");
    if (fastest){
        fastestOut << fastest->name << "\n";
        for (size_t i=0;i<fastest->path.size();++i){
            fastestOut << fastest->path[i].row << " " << fastest->path[i].col << (i+1<fastest->path.size()?"\n":"");
        }
    }
    return 0;
}


