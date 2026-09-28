#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- Read the input ----
    int K = inf.readInt();
    int T = inf.readInt();

    vector<int> N(K), M(K);
    vector<vector<long long>> a(K);
    vector<set<pair<int,int>>> edgeSet(K);
    vector<vector<vector<int>>> adj(K);

    for (int i = 0; i < K; i++) {
        N[i] = inf.readInt();
        M[i] = inf.readInt();
        a[i].resize(N[i] + 1);
        for (int v = 1; v <= N[i]; v++) {
            a[i][v] = inf.readInt();
        }
        adj[i].resize(N[i] + 1);
        for (int j = 0; j < M[i]; j++) {
            int u = inf.readInt(), v = inf.readInt();
            edgeSet[i].insert({min(u,v), max(u,v)});
            adj[i][u].push_back(v);
            if (u != v) adj[i][v].push_back(u);
        }
        for (int v = 1; v <= N[i]; v++) {
            sort(adj[i][v].begin(), adj[i][v].end());
        }
    }

    // ---- Read and validate participant output ----
    vector<vector<int>> walks(K);
    bool feasible = true;
    string failReason;

    for (int i = 0; i < K; i++) {
        walks[i].resize(T + 1);
        for (int t = 0; t <= T; t++) {
            if (ouf.seekEof()) {
                feasible = false;
                failReason = "Walk " + to_string(i+1) + " truncated at step " + to_string(t);
                goto done_reading;
            }
            int v = ouf.readInt();
            if (v < 1 || v > N[i]) {
                feasible = false;
                failReason = "Walk " + to_string(i+1) + " vertex " + to_string(v)
                           + " out of range [1," + to_string(N[i]) + "] at step " + to_string(t);
                goto done_reading;
            }
            walks[i][t] = v;
        }
    }

    done_reading:;

    // ---- EOF check: no extra tokens allowed ----
    if (feasible) {
        if (!ouf.seekEof()) {
            quitf(_wa, "Extra output after the %d walks", K);
        }
    }

    if (feasible) {
        // Check starting vertices
        for (int i = 0; i < K; i++) {
            if (walks[i][0] != 1) {
                feasible = false;
                failReason = "Walk " + to_string(i+1) + " does not start at vertex 1 (got "
                           + to_string(walks[i][0]) + ")";
                break;
            }
        }
    }

    if (feasible) {
        // Check edge validity
        for (int i = 0; i < K && feasible; i++) {
            for (int t = 0; t < T && feasible; t++) {
                int u = walks[i][t], v = walks[i][t+1];
                if (edgeSet[i].find({min(u,v), max(u,v)}) == edgeSet[i].end()) {
                    feasible = false;
                    failReason = "Walk " + to_string(i+1) + ": no edge ("
                               + to_string(u) + "," + to_string(v) + ") at step " + to_string(t);
                }
            }
        }
    }

    if (!feasible) {
        quitf(_wa, "%s", failReason.c_str());
    }

    // ---- Compute participant objective ----
    set<vector<int>> visited;
    for (int t = 0; t <= T; t++) {
        vector<int> state(K);
        for (int i = 0; i < K; i++) state[i] = walks[i][t];
        visited.insert(state);
    }

    long double obj = 0;
    for (auto& state : visited) {
        long double prod = 1;
        for (int i = 0; i < K; i++) prod *= (long double)a[i][state[i]];
        obj += prod;
    }

    // ---- Compute baseline ----
    auto computeBaselineWalk = [&](int i) -> vector<int> {
        int n = N[i];
        // BFS to build tree with lex-smallest ordering
        vector<bool> vis(n + 1, false);
        vector<vector<int>> children(n + 1);
        queue<int> bq;
        bq.push(1);
        vis[1] = true;
        while (!bq.empty()) {
            int v = bq.front(); bq.pop();
            vector<int> nbrs = adj[i][v]; // already sorted
            for (int u : nbrs) {
                if (u != v && !vis[u]) {
                    vis[u] = true;
                    children[v].push_back(u);
                    bq.push(u);
                }
            }
        }
        // DFS Euler tour
        vector<int> tour;
        function<void(int)> dfs = [&](int v) {
            tour.push_back(v);
            for (int c : children[v]) {
                dfs(c);
                tour.push_back(v);
            }
        };
        dfs(1);
        // tour has 2*n-1 elements, tour[0]=tour.back()=1
        int L = (int)tour.size();
        int period = L - 1; // 2*(n-1), >= 1 since n>=2

        vector<int> walk(T + 1);
        for (int t = 0; t <= T; t++) {
            if (t < L) {
                walk[t] = tour[t];
            } else {
                walk[t] = tour[1 + (t - 1) % period];
            }
        }
        return walk;
    };

    vector<vector<int>> baseWalks(K);
    for (int i = 0; i < K; i++) {
        baseWalks[i] = computeBaselineWalk(i);
    }

    set<vector<int>> baseVisited;
    for (int t = 0; t <= T; t++) {
        vector<int> state(K);
        for (int i = 0; i < K; i++) state[i] = baseWalks[i][t];
        baseVisited.insert(state);
    }

    long double baseObj = 0;
    for (auto& state : baseVisited) {
        long double prod = 1;
        for (int i = 0; i < K; i++) prod *= (long double)a[i][state[i]];
        baseObj += prod;
    }

    // ---- Score formula ----
    // score = floor(1e6 * clamp((OBJ - BASE) / (4 * BASE), 0, 1))
    long double ratio = 0;
    if (baseObj > 0) {
        ratio = (obj - baseObj) / (4.0L * baseObj);
    }
    if (ratio < 0.0L) ratio = 0.0L;
    if (ratio > 1.0L) ratio = 1.0L;

    long long scoreInt = (long long)(ratio * 1000000.0L);

    quitp((double)ratio,
        "OBJ=%.0Lf BASE=%.0Lf Ratio: %.6Lf Score: %lld",
        obj, baseObj, ratio, scoreInt);

    return 0;
}