#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

struct DSU {
    vector<int> parent, rnk;
    DSU(int n) : parent(n), rnk(n, 0) { iota(parent.begin(), parent.end(), 0); }
    int find(int x) { return parent[x] == x ? x : parent[x] = find(parent[x]); }
    bool unite(int a, int b) {
        a = find(a); b = find(b);
        if (a == b) return false;
        if (rnk[a] < rnk[b]) swap(a, b);
        parent[b] = a;
        if (rnk[a] == rnk[b]) rnk[a]++;
        return true;
    }
    bool same(int a, int b) { return find(a) == find(b); }
};

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---------- read input ----------
    int n = inf.readInt();
    int m = inf.readInt();
    long long A = inf.readLong();

    vector<long long> w(n + 1);
    for (int i = 1; i <= n; i++) w[i] = inf.readLong();

    vector<int> eu(m + 1), ev(m + 1);
    vector<long long> ec(m + 1);
    vector<vector<int>> adj(n + 1); // adjacency list (neighbor vertices)

    for (int j = 1; j <= m; j++) {
        eu[j] = inf.readInt();
        ev[j] = inf.readInt();
        ec[j] = inf.readLong();
        adj[eu[j]].push_back(ev[j]);
        adj[ev[j]].push_back(eu[j]);
    }

    // ---------- read participant output: first line r e ----------
    int r = ouf.readInt();
    int e = ouf.readInt();
    ouf.readEoln(); // consume newline after "r e"

    if (r < 0) quitf(_wa, "r=%d is negative", r);
    if (e < 0) quitf(_wa, "e=%d is negative", e);
    if (r > n) quitf(_wa, "r=%d exceeds n=%d", r, n);
    if (e > m) quitf(_wa, "e=%d exceeds m=%d", e, m);

    // ---------- read hideout vertices (second line, may be empty) ----------
    vector<int> hideouts(r);
    set<int> hideoutSet;
    for (int i = 0; i < r; i++) {
        int v = ouf.readInt();
        if (v < 1 || v > n)
            quitf(_wa, "Hideout vertex %d out of range [1,%d]", v, n);
        if (!hideoutSet.insert(v).second)
            quitf(_wa, "Duplicate hideout vertex %d", v);
        hideouts[i] = v;
    }
    ouf.readEoln(); // consume newline after hideout list (even if empty line)

    // ---------- read selected edge IDs (one per line) ----------
    vector<int> edgeIds(e);
    set<int> edgeSet;
    for (int i = 0; i < e; i++) {
        int eid = ouf.readInt();
        ouf.readEoln();
        if (eid < 1 || eid > m)
            quitf(_wa, "Edge ID %d out of range [1,%d]", eid, m);
        if (!edgeSet.insert(eid).second)
            quitf(_wa, "Duplicate edge ID %d", eid);
        edgeIds[i] = eid;
    }

    // Ensure no trailing tokens
    ouf.readEof();

    // ---------- feasibility check 1: hideouts are independent set ----------
    set<int> hideoutSetCheck(hideoutSet);
    for (int h : hideouts) {
        for (int nb : adj[h]) {
            if (hideoutSetCheck.count(nb))
                quitf(_wa, "Hideouts %d and %d are adjacent in the original graph", h, nb);
        }
    }

    // ---------- feasibility check 2: selected edges form a forest ----------
    DSU dsu(n + 1);
    for (int j : edgeIds) {
        if (!dsu.unite(eu[j], ev[j]))
            quitf(_wa, "Selected edges contain a cycle (edge %d creates one)", j);
    }

    // ---------- feasibility check 3: V' and each component has exactly one hideout ----------
    // V' = hideout vertices + endpoints of selected edges
    set<int> Vprime(hideoutSetCheck);
    for (int j : edgeIds) {
        Vprime.insert(eu[j]);
        Vprime.insert(ev[j]);
    }

    // count hideouts per DSU component (among vertices in V')
    map<int, int> compHideout;
    for (int v : Vprime) {
        compHideout[dsu.find(v)] = 0;
    }
    for (int h : hideouts) {
        compHideout[dsu.find(h)]++;
    }
    for (auto& [root, cnt] : compHideout) {
        if (cnt != 1)
            quitf(_wa, "A connected component has %d hideout(s) instead of exactly 1", cnt);
    }

    // ---------- compute objective O ----------
    long long sumW = 0;
    for (int v : Vprime) sumW += w[v];
    long long sumC = 0;
    for (int j : edgeIds) sumC += ec[j];
    long long O = sumW - sumC - A * (long long)r;

    // If O < 0 we still allow it (feasible), but clamp score to >= 0
    // (empty solution always scores 0, which is fine)

    // ---------- compute W = sum of all vertex weights ----------
    long long W = 0;
    for (int i = 1; i <= n; i++) W += w[i];

    // ---------- compute baseline B ----------
    vector<int> order(n);
    iota(order.begin(), order.end(), 1);
    sort(order.begin(), order.end(), [&](int a, int b) {
        return w[a] != w[b] ? w[a] > w[b] : a < b;
    });

    vector<bool> blocked(n + 1, false);
    long long B = 0;
    for (int v : order) {
        if (w[v] <= A) continue;
        if (blocked[v]) continue;
        B += w[v] - A;
        for (int nb : adj[v]) blocked[nb] = true;
        blocked[v] = true;
    }

    // ---------- scoring: continuous in [0, 100] ----------
    double score;
    if (O <= 0) {
        // Worse than or equal to empty solution
        score = 0.0;
    } else {
        long long denom1 = max(1LL, B);
        long long denom2 = max(1LL, W - B);
        if (O <= B) {
            score = 50.0 * (double)O / (double)denom1;
        } else {
            score = 50.0 + 50.0 * (double)(O - B) / (double)denom2;
        }
    }
    // clamp to [0, 100]
    if (score < 0.0) score = 0.0;
    if (score > 100.0) score = 100.0;

    double ratio = score / 100.0;

    quitp(ratio, "Ratio: %.6f (O=%lld, B=%lld, W=%lld, score=%.4f/100)",
          ratio, O, B, W, score);
}