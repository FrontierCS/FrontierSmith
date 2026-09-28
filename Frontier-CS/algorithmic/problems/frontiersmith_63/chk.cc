#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

// ---- Global problem data ----
static int n, m;
static vector<long long> p_val, es_val, ec_val;
static vector<int> eu_val, ev_val;

// ---- Max-weight matching on a tree via tree DP ----
// adj[v] = list of (neighbor, edge_id)
static vector<vector<pair<int,int>>> g_adj;
static vector<long long> dp0v, dp1v;

static void computeMatchingDP(int root) {
    // BFS order
    vector<int> order;
    vector<int> par(n + 1, -1);
    vector<bool> vis(n + 1, false);
    queue<int> bfsq;
    bfsq.push(root);
    vis[root] = true;
    while (!bfsq.empty()) {
        int v = bfsq.front(); bfsq.pop();
        order.push_back(v);
        for (auto &[u, eid] : g_adj[v]) {
            if (!vis[u]) {
                vis[u] = true;
                par[u] = v;
                bfsq.push(u);
            }
        }
    }
    dp0v.assign(n + 1, 0LL);
    dp1v.assign(n + 1, (long long)(-1e18));
    // process in reverse BFS (leaves first)
    for (int i = (int)order.size() - 1; i >= 0; i--) {
        int v = order[i];
        dp0v[v] = 0LL;
        dp1v[v] = (long long)(-1e18);
        long long best_gain = (long long)(-1e18);
        for (auto &[u, eid] : g_adj[v]) {
            if (u == par[v]) continue;
            long long child_best = max(dp0v[u], dp1v[u]);
            dp0v[v] += child_best;
            // gain if we match edge (v,u)
            long long gain = p_val[v] + p_val[u] + es_val[eid] + dp0v[u] - child_best;
            if (gain > best_gain) best_gain = gain;
        }
        if (best_gain > (long long)(-1e17)) {
            dp1v[v] = dp0v[v] + best_gain;
        }
    }
}

static long long computeObj(const vector<int> &chosen) {
    g_adj.assign(n + 1, {});
    long long cost = 0;
    for (int eid : chosen) {
        g_adj[eu_val[eid]].push_back({ev_val[eid], eid});
        g_adj[ev_val[eid]].push_back({eu_val[eid], eid});
        cost += ec_val[eid];
    }
    computeMatchingDP(1);
    long long match_val = max(dp0v[1], dp1v[1]);
    return match_val - cost;
}

int main(int argc, char *argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- Read input ----
    n = inf.readInt();
    m = inf.readInt();

    int totalEdges = (n - 1) + m;

    p_val.resize(n + 1, 0LL);
    for (int i = 1; i <= n; i++) p_val[i] = inf.readLong();

    eu_val.resize(totalEdges + 1);
    ev_val.resize(totalEdges + 1);
    es_val.resize(totalEdges + 1, 0LL);
    ec_val.resize(totalEdges + 1, 0LL);

    // original edges: IDs 1..n-1
    for (int i = 1; i <= n - 1; i++) {
        eu_val[i] = inf.readInt();
        ev_val[i] = inf.readInt();
        es_val[i] = inf.readLong();
        ec_val[i] = 0LL;
    }
    // optional edges: IDs n..n+m-1
    for (int i = n; i < n + m; i++) {
        eu_val[i] = inf.readInt();
        ev_val[i] = inf.readInt();
        es_val[i] = inf.readLong();
        ec_val[i] = inf.readLong();
    }

    // ---- Compute baseline (original tree: edges 1..n-1) ----
    vector<int> baseline;
    baseline.reserve(n - 1);
    for (int i = 1; i <= n - 1; i++) baseline.push_back(i);
    long long B = computeObj(baseline);

    // ---- Read participant output ----
    vector<int> chosen;
    while (!ouf.seekEof()) {
        int id = ouf.readInt();
        chosen.push_back(id);
    }

    // ---- Validate: count ----
    if ((int)chosen.size() != n - 1) {
        quitf(_wa, "Expected %d edge IDs, got %d", n - 1, (int)chosen.size());
    }

    // ---- Validate: range & uniqueness ----
    set<int> chosenSet;
    for (int id : chosen) {
        if (id < 1 || id > totalEdges) {
            quitf(_wa, "Invalid edge ID %d (valid range 1..%d)", id, totalEdges);
        }
        if (chosenSet.count(id)) {
            quitf(_wa, "Duplicate edge ID %d", id);
        }
        chosenSet.insert(id);
    }

    // ---- Validate: spanning tree (acyclic + connected) via DSU ----
    {
        vector<int> dsu(n + 1);
        iota(dsu.begin(), dsu.end(), 0);
        function<int(int)> find = [&](int x) -> int {
            return dsu[x] == x ? x : dsu[x] = find(dsu[x]);
        };
        for (int id : chosen) {
            int u = eu_val[id], v = ev_val[id];
            int pu = find(u), pv = find(v);
            if (pu == pv) {
                quitf(_wa, "Chosen edges form a cycle (edge ID %d creates cycle)", id);
            }
            dsu[pu] = pv;
        }
        int root = find(1);
        for (int i = 2; i <= n; i++) {
            if (find(i) != root) {
                quitf(_wa, "Chosen edges do not span all vertices (vertex %d disconnected)", i);
            }
        }
    }

    // ---- Compute objective ----
    long long O = computeObj(chosen);

    // ---- Scoring ----
    // Score = 1000 * clamp(0, 2, (O+1)/(B+1))
    // Map to ratio in [0, 1]: ratio = clamp(0, 1, (O+1)/(B+1) / 2)
    // So baseline (O==B) -> ratio = 0.5 -> 1000 pts
    // Twice baseline -> ratio = 1.0 -> 2000 pts
    // Zero / very bad -> ratio -> 0

    double ratio;
    long long denom = B + 1LL;
    if (denom <= 0LL) {
        // B is very negative; any non-negative O is much better
        if (O >= B) {
            ratio = 1.0;
        } else {
            // Both negative — use linear interpolation: 0 if O << B
            // Ratio based on how much worse O is vs B (both negative)
            // Use: ratio = clamp(0,1, 0.5 + (O - B) / (2.0 * fabs((double)B) + 1.0))
            double spread = 2.0 * (double)(llabs(B)) + 1.0;
            ratio = 0.5 + (double)(O - B) / spread;
            if (ratio < 0.0) ratio = 0.0;
            if (ratio > 1.0) ratio = 1.0;
        }
    } else {
        // Normal case
        double raw = (double)(O + 1LL) / (double)denom; // in [0, ~2+]
        // Divide by 2 to map to [0,1]
        ratio = raw / 2.0;
        if (ratio < 0.0) ratio = 0.0;
        if (ratio > 1.0) ratio = 1.0;
    }

    quitp(ratio, "OBJ=%lld Baseline=%lld Ratio: %.9f", O, B, ratio);
    return 0;
}