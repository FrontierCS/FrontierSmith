#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

// BFS returning dist array (1-indexed vertices); -1 if unreachable
vector<int> bfsDist(int src, int n, const vector<vector<pair<int,int>>>& adj) {
    vector<int> dist(n + 1, -1);
    queue<int> qu;
    dist[src] = 0;
    qu.push(src);
    while (!qu.empty()) {
        int u = qu.front(); qu.pop();
        for (auto& [v, idx] : adj[u]) {
            if (dist[v] == -1) {
                dist[v] = dist[u] + 1;
                qu.push(v);
            }
        }
    }
    return dist;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- Read problem input ----
    int n = inf.readInt();
    int m = inf.readInt();
    int q = inf.readInt();
    int B = inf.readInt();

    vector<pair<int,int>> edges(m + 1);
    vector<vector<pair<int,int>>> adjG(n + 1); // (neighbor, edge_index)

    for (int i = 1; i <= m; i++) {
        int u = inf.readInt(), v = inf.readInt();
        edges[i] = {u, v};
        adjG[u].push_back({v, i});
        adjG[v].push_back({u, i});
    }

    vector<int> roots(q), wts(q);
    for (int t = 0; t < q; t++) {
        roots[t] = inf.readInt();
        wts[t]   = inf.readInt();
    }

    // ---- BFS distances in original graph for each depot ----
    vector<vector<int>> distG(q);
    for (int t = 0; t < q; t++) {
        distG[t] = bfsDist(roots[t], n, adjG);
    }

    // ---- Compute Q_max ----
    long long sumW = 0;
    for (int t = 0; t < q; t++) sumW += wts[t];
    long long Q_max = (long long)n * n * sumW;

    // ---- Build baseline backbone ----
    // Step 1: BFS tree from roots[0], parent = smallest-numbered vertex u
    //         such that edge(u,v) exists and d[u]+1=d[v]
    vector<int> d1 = distG[0];  // distances from roots[0] in G

    // Map (u,v) -> edge index (canonical, u<v not required since we store both)
    map<pair<int,int>, int> edgeMap;
    for (int i = 1; i <= m; i++) {
        edgeMap[{edges[i].first,  edges[i].second}] = i;
        edgeMap[{edges[i].second, edges[i].first}]  = i;
    }

    set<int> baseSet;
    for (int v = 1; v <= n; v++) {
        if (v == roots[0]) continue;
        int bestU = -1;
        for (auto& [u, idx] : adjG[v]) {
            if (d1[u] != -1 && d1[u] + 1 == d1[v]) {
                if (bestU == -1 || u < bestU) bestU = u;
            }
        }
        if (bestU != -1) {
            int ei = edgeMap[{bestU, v}];
            baseSet.insert(ei);
        }
    }

    // Step 2: If B > n-1, add unused edges in increasing index order
    for (int i = 1; i <= m && (int)baseSet.size() < B; i++) {
        baseSet.insert(i);
    }

    // ---- Compute Q_base ----
    vector<vector<pair<int,int>>> adjBase(n + 1);
    for (int ei : baseSet) {
        int u = edges[ei].first, v = edges[ei].second;
        adjBase[u].push_back({v, ei});
        adjBase[v].push_back({u, ei});
    }

    long long Q_base = 0;
    for (int t = 0; t < q; t++) {
        vector<int> dH = bfsDist(roots[t], n, adjBase);
        for (int v = 1; v <= n; v++) {
            int delta = dH[v] - distG[t][v];  // >= 0
            Q_base += (long long)wts[t] * (n - delta);
        }
    }

    // ---- Read participant output ----
    set<int> chosenSet;
    for (int i = 0; i < B; i++) {
        int ei = ouf.readInt();
        if (ei < 1 || ei > m) {
            quitf(_wa, "Edge index %d is out of range [1, %d]", ei, m);
        }
        if (!chosenSet.insert(ei).second) {
            quitf(_wa, "Duplicate edge index %d in output", ei);
        }
    }
    // Ensure no trailing tokens
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra tokens found after reading %d edge indices", B);
    }

    // ---- Validate connectivity ----
    vector<vector<pair<int,int>>> adjH(n + 1);
    for (int ei : chosenSet) {
        int u = edges[ei].first, v = edges[ei].second;
        adjH[u].push_back({v, ei});
        adjH[v].push_back({u, ei});
    }

    vector<int> compDist = bfsDist(1, n, adjH);
    for (int v = 1; v <= n; v++) {
        if (compDist[v] == -1) {
            quitf(_wa, "Backbone is not connected: vertex %d is unreachable", v);
        }
    }

    // ---- Compute Q_sub ----
    long long Q_sub = 0;
    for (int t = 0; t < q; t++) {
        vector<int> dH = bfsDist(roots[t], n, adjH);
        for (int v = 1; v <= n; v++) {
            int delta = dH[v] - distG[t][v];  // >= 0 since H is subgraph
            Q_sub += (long long)wts[t] * (n - delta);
        }
    }

    // ---- Compute and report score ----
    if (Q_max == Q_base) {
        // Baseline already achieves theoretical maximum; any feasible solution scores full
        quitp(1.0, "Q_sub=%lld Q_base=%lld Q_max=%lld (trivial case) Ratio: 1.000000", Q_sub, Q_base, Q_max);
    }

    double ratio = (double)(Q_sub - Q_base) / (double)(Q_max - Q_base);
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    quitp(ratio,
          "Q_sub=%lld Q_base=%lld Q_max=%lld Ratio: %.9f",
          Q_sub, Q_base, Q_max, ratio);

    return 0;
}