#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

struct DSU {
    vector<int> parent, rnk;
    int components;
    DSU(int n) : parent(n+1), rnk(n+1, 0), components(n) {
        iota(parent.begin(), parent.end(), 0);
    }
    int find(int x) {
        return parent[x] == x ? x : parent[x] = find(parent[x]);
    }
    void unite(int a, int b) {
        a = find(a); b = find(b);
        if (a == b) return;
        --components;
        if (rnk[a] < rnk[b]) swap(a, b);
        parent[b] = a;
        if (rnk[a] == rnk[b]) ++rnk[a];
    }
};

long long computeF(int n,
                   const vector<tuple<int,int,int>>& edges,
                   const vector<int>& perm) {
    vector<int> pos(n + 1);
    for (int i = 0; i < n; i++) pos[perm[i]] = i;

    long long W = 0;
    DSU dsu(n);

    for (const auto& e : edges) {
        int u = get<0>(e), v = get<1>(e), w = get<2>(e);
        if (pos[u] < pos[v]) {
            W += w;
            dsu.unite(u, v);
        }
    }

    int C = dsu.components;
    long long mult = (long long)(n - C + 1);
    return W * mult;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    int T = inf.readInt();

    double totalRatio = 0.0;

    for (int t = 0; t < T; t++) {
        int n = inf.readInt();
        int m = inf.readInt();

        vector<tuple<int,int,int>> edges(m);
        vector<long long> balance(n + 1, 0LL);

        for (int i = 0; i < m; i++) {
            int u = inf.readInt();
            int v = inf.readInt();
            int w = inf.readInt();
            edges[i] = make_tuple(u, v, w);
            balance[u] += w;
            balance[v] -= w;
        }

        // Read participant's permutation for this test case
        vector<int> perm(n);
        vector<bool> seen(n + 1, false);
        for (int i = 0; i < n; i++) {
            int x = ouf.readInt(1, n);
            if (seen[x]) {
                quitf(_wa, "Test case %d: duplicate vertex %d in permutation", t + 1, x);
            }
            seen[x] = true;
            perm[i] = x;
        }
        for (int v2 = 1; v2 <= n; v2++) {
            if (!seen[v2]) {
                quitf(_wa, "Test case %d: vertex %d missing from permutation", t + 1, v2);
            }
        }

        // Compute baseline permutation:
        // Sort vertices by decreasing balance, break ties by smaller label
        vector<int> basePerm(n);
        iota(basePerm.begin(), basePerm.end(), 1); // 1..n
        sort(basePerm.begin(), basePerm.end(), [&](int a, int b) {
            if (balance[a] != balance[b]) return balance[a] > balance[b];
            return a < b;
        });

        long long F = computeF(n, edges, perm);
        long long B = computeF(n, edges, basePerm);

        // S = 10^6 * clamp(0, (F+1)/(B+1), 2)
        // per-test clamp_ratio in [0, 2]
        double clampRatio;
        if (B + 1 <= 0) {
            // degenerate: baseline is 0, any valid answer is perfect
            clampRatio = 1.0;
        } else {
            clampRatio = (double)(F + 1) / (double)(B + 1);
        }
        if (clampRatio < 0.0) clampRatio = 0.0;
        if (clampRatio > 2.0) clampRatio = 2.0;

        totalRatio += clampRatio;
    }

    // Strict EOF check: reject trailing garbage in participant output
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra output after all test cases");
    }

    // meanClampRatio in [0, 2]; divide by 2 to get scoreRatio in [0, 1]
    // (maximum possible S per test is 2e6, baseline gives 1e6 = half of max)
    double meanClampRatio = totalRatio / (double)T;
    double scoreRatio = meanClampRatio / 2.0;
    if (scoreRatio < 0.0) scoreRatio = 0.0;
    if (scoreRatio > 1.0) scoreRatio = 1.0;

    // "Ratio: <score_ratio>" tag is required by the judge (parsed by regex).
    // scoreRatio is in [0,1]: 0.5 = matches baseline, 1.0 = 2x baseline everywhere.
    quitp(scoreRatio, "Ratio: %.9f (mean_clamp_ratio=%.9f over %d tests)",
          scoreRatio, meanClampRatio, T);
}