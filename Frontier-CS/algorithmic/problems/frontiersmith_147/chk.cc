#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ── Read problem input ──────────────────────────────────────────────────
    int H = inf.readInt();
    int W = inf.readInt();
    int K = inf.readInt();
    long long B = inf.readLong();

    vector<string> grid(H);
    for (int i = 0; i < H; i++)
        grid[i] = inf.readToken();

    vector<vector<int>> T(H, vector<int>(W, 0));
    for (int i = 0; i < H; i++)
        for (int j = 0; j < W; j++)
            T[i][j] = inf.readInt();

    vector<vector<long long>> C(H, vector<long long>(W, 0LL));
    for (int i = 0; i < H; i++)
        for (int j = 0; j < W; j++)
            C[i][j] = inf.readLong();

    // Routes (convert to 0-indexed)
    vector<int> rs(K), cs(K), rg(K), cg(K);
    vector<long long> w(K);
    set<pair<int,int>> endpoint_cells;
    for (int t = 0; t < K; t++) {
        rs[t] = inf.readInt() - 1;
        cs[t] = inf.readInt() - 1;
        rg[t] = inf.readInt() - 1;
        cg[t] = inf.readInt() - 1;
        w[t]  = inf.readLong();
        endpoint_cells.insert({rs[t], cs[t]});
        endpoint_cells.insert({rg[t], cg[t]});
    }

    long long P = 9LL * H * W + 1;

    // ── Dijkstra helper (entry-cost model) ─────────────────────────────────
    const int dx[] = {0, 0, 1, -1};
    const int dy[] = {1, -1, 0, 0};

    auto dijkstra = [&](int sr, int sc, int gr, int gc,
                        const vector<vector<bool>>& blocked) -> long long {
        vector<vector<long long>> dist(H, vector<long long>(W, (long long)4e18));
        priority_queue<tuple<long long,int,int>,
                       vector<tuple<long long,int,int>>,
                       greater<>> pq;
        dist[sr][sc] = 0;
        pq.push({0LL, sr, sc});
        while (!pq.empty()) {
            auto [d, r, c] = pq.top(); pq.pop();
            if (d > dist[r][c]) continue;
            if (r == gr && c == gc) return d;
            for (int di = 0; di < 4; di++) {
                int nr = r + dx[di];
                int nc = c + dy[di];
                if (nr < 0 || nr >= H || nc < 0 || nc >= W) continue;
                if (grid[nr][nc] == '#') continue;
                if (blocked[nr][nc]) continue;
                long long nd = d + (long long)T[nr][nc];
                if (nd < dist[nr][nc]) {
                    dist[nr][nc] = nd;
                    pq.push({nd, nr, nc});
                }
            }
        }
        return P; // unreachable
    };

    // ── Compute BASE (no barricades) ────────────────────────────────────────
    vector<vector<bool>> no_barricades(H, vector<bool>(W, false));
    long long BASE = 0;
    for (int t = 0; t < K; t++)
        BASE += w[t] * dijkstra(rs[t], cs[t], rg[t], cg[t], no_barricades);

    // ── Read participant output ─────────────────────────────────────────────
    // Read M (number of barricades)
    int M = ouf.readInt(0, H * W, "M");

    vector<vector<bool>> barricaded(H, vector<bool>(W, false));
    long long total_cost = 0;
    set<pair<int,int>> seen;

    for (int x = 0; x < M; x++) {
        int r = ouf.readInt(1, H, "r") - 1;
        int c = ouf.readInt(1, W, "c") - 1;

        if (seen.count({r, c}))
            quitf(_wa, "Duplicate barricade cell (%d, %d)", r+1, c+1);
        seen.insert({r, c});

        if (grid[r][c] != '.')
            quitf(_wa, "Cell (%d, %d) is not a '.' cell (type='%c')", r+1, c+1, grid[r][c]);

        if (endpoint_cells.count({r, c}))
            quitf(_wa, "Cell (%d, %d) is a route endpoint and cannot be barricaded", r+1, c+1);

        total_cost += C[r][c];
        barricaded[r][c] = true;
    }

    if (total_cost > B)
        quitf(_wa, "Total barricade cost %lld exceeds budget %lld", total_cost, B);

    // NOTE: We intentionally do NOT call ouf.readEof() here.
    // Trailing whitespace/newlines in the participant output are acceptable
    // for this optimization problem format.

    // ── Compute OBJ (with barricades) ───────────────────────────────────────
    long long OBJ = 0;
    for (int t = 0; t < K; t++)
        OBJ += w[t] * dijkstra(rs[t], cs[t], rg[t], cg[t], barricaded);

    // ── Compute score ratio ─────────────────────────────────────────────────
    // Statement formula: score = min(10^9, floor(10^6 * max(0, OBJ - BASE) / BASE))
    // We map this to [0, 1] by dividing by 10^9.
    //
    // BASE >= 1 is guaranteed: all travel times >= 1, start != goal,
    // and disconnected routes get penalty P >= 1.
    double ratio = 0.0;
    if (BASE > 0 && OBJ > BASE) {
        // Use __int128 to avoid overflow in intermediate products
        __int128 diff    = (__int128)(OBJ - BASE);
        __int128 base128 = (__int128)BASE;
        __int128 scale   = (__int128)1000000LL;    // 10^6
        __int128 cap     = (__int128)1000000000LL; // 10^9

        __int128 score_val = diff * scale / base128; // floor division
        if (score_val > cap) score_val = cap;

        ratio = (double)score_val / 1000000000.0;
        if (ratio > 1.0) ratio = 1.0;
        if (ratio < 0.0) ratio = 0.0;
    }

    quitp(ratio,
          "BASE=%lld OBJ=%lld barricades=%d cost=%lld Ratio: %.9f",
          BASE, OBJ, M, total_cost, ratio);

    return 0;
}