#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

struct DSU {
    vector<int> p, rnk;
    DSU(int n) : p(n), rnk(n, 0) { iota(p.begin(), p.end(), 0); }
    int find(int x) { return p[x] == x ? x : p[x] = find(p[x]); }
    bool unite(int a, int b) {
        a = find(a); b = find(b);
        if (a == b) return false;
        if (rnk[a] < rnk[b]) swap(a, b);
        p[b] = a;
        if (rnk[a] == rnk[b]) rnk[a]++;
        return true;
    }
    bool same(int a, int b) { return find(a) == find(b); }
};

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    int T = inf.readInt();

    double total_score = 0.0;
    int total_cases = 0;

    for (int tc = 1; tc <= T; tc++) {
        // ---- Read problem input for this test case ----
        int N = inf.readInt();
        int M = inf.readInt();
        long long P = (long long)inf.readInt();

        vector<int> eu(M), ev(M);
        vector<long long> ew(M);
        for (int i = 0; i < M; i++) {
            eu[i] = inf.readInt() - 1;
            ev[i] = inf.readInt() - 1;
            ew[i] = (long long)inf.readInt();
        }

        // ---- Compute upper bound U ----
        long long U = 0;
        for (int i = 0; i < M; i++)
            if (ew[i] > 0) U += ew[i];

        // ---- Compute baseline B: max-weight spanning tree ----
        vector<int> order(M);
        iota(order.begin(), order.end(), 0);
        sort(order.begin(), order.end(), [&](int a, int b){ return ew[a] > ew[b]; });

        DSU dsu_mst(N);
        long long mst_w = 0;
        vector<int> deg_mst(N, 0);
        int mst_cnt = 0;
        for (int i : order) {
            if (dsu_mst.unite(eu[i], ev[i])) {
                mst_w += ew[i];
                deg_mst[eu[i]]++;
                deg_mst[ev[i]]++;
                mst_cnt++;
                if (mst_cnt == N - 1) break;
            }
        }
        long long odd_mst = 0;
        for (int i = 0; i < N; i++)
            if (deg_mst[i] % 2 == 1) odd_mst++;
        long long B = mst_w - P * odd_mst;

        total_cases++;

        // ---- Read participant output for this test case ----
        // We try to parse; on any parse/feasibility failure give 0 for this case.

        // Read N colors
        vector<int> color(N, -1);
        bool parse_ok = true;
        for (int i = 0; i < N && parse_ok; i++) {
            if (ouf.eof()) { parse_ok = false; break; }
            color[i] = ouf.readInt(0, 1, ("color_" + to_string(i+1)).c_str());
        }
        if (!parse_ok) {
            // infeasible: score 0 for this case, continue
            total_score += 0.0;
            continue;
        }

        // Read K
        if (ouf.eof()) { total_score += 0.0; continue; }
        int K = ouf.readInt(0, 2 * M + 10, "K");

        // Feasibility: K >= N-1
        if (K < N - 1) {
            total_score += 0.0;
            // Still need to drain K edge indices to stay in sync
            for (int i = 0; i < K && !ouf.eof(); i++)
                ouf.readInt();
            continue;
        }

        // Read K edge indices
        if (K > 2 * M + 10) {
            // Suspiciously large, treat as infeasible
            total_score += 0.0;
            continue;
        }

        vector<int> chosen(K, 0);
        bool dup_or_invalid = false;
        vector<bool> used(M + 1, false);
        for (int i = 0; i < K && parse_ok; i++) {
            if (ouf.eof()) { parse_ok = false; break; }
            int idx = ouf.readInt(1, M, ("edge_idx_" + to_string(i+1)).c_str());
            if (used[idx]) {
                dup_or_invalid = true;
                // drain remaining
                for (int j = i + 1; j < K && !ouf.eof(); j++)
                    ouf.readInt();
                break;
            }
            used[idx] = true;
            chosen[i] = idx;
        }

        if (!parse_ok || dup_or_invalid) {
            total_score += 0.0;
            continue;
        }

        // ---- Feasibility checks ----

        // Check bipartiteness and compute W, degrees
        vector<int> deg(N, 0);
        long long W = 0;
        bool feasible = true;
        for (int idx : chosen) {
            int i = idx - 1;
            if (color[eu[i]] == color[ev[i]]) {
                feasible = false;
                break;
            }
            deg[eu[i]]++;
            deg[ev[i]]++;
            W += ew[i];
        }

        if (!feasible) {
            total_score += 0.0;
            continue;
        }

        // Check connectivity
        DSU dsu2(N);
        for (int idx : chosen) {
            int i = idx - 1;
            dsu2.unite(eu[i], ev[i]);
        }
        int root = dsu2.find(0);
        for (int i = 1; i < N; i++) {
            if (dsu2.find(i) != root) {
                feasible = false;
                break;
            }
        }

        if (!feasible) {
            total_score += 0.0;
            continue;
        }

        // ---- Compute objective X ----
        long long odd_h = 0;
        for (int i = 0; i < N; i++)
            if (deg[i] % 2 == 1) odd_h++;
        long long X = W - P * odd_h;

        // ---- Per-case score ----
        long long denom = max(1LL, U - B);
        double ratio = (double)(X - B) / (double)denom;
        ratio = max(0.0, min(1.0, ratio));
        total_score += ratio;
    }

    // ---- Aggregate ----
    double avg_ratio = (total_cases > 0) ? (total_score / (double)total_cases) : 0.0;
    // clamp
    avg_ratio = max(0.0, min(1.0, avg_ratio));

    quitp(avg_ratio, "Ratio: %.9f (sum=%.6f over %d test cases)", avg_ratio, total_score, total_cases);
}