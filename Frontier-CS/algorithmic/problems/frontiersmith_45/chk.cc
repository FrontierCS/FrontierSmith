#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- Read input ----
    int n = inf.readInt();
    int m = inf.readInt();
    long long B = inf.readLong();

    int R = n - 1;

    struct Road {
        int a, b;
        long long t_e, c_e, d_e, h_e, g_e;
    };

    vector<Road> roads(R + 1);
    vector<vector<pair<int,int>>> adj(n + 1);

    for (int j = 1; j <= R; j++) {
        roads[j].a   = inf.readInt();
        roads[j].b   = inf.readInt();
        roads[j].t_e = inf.readLong();
        roads[j].c_e = inf.readLong();
        roads[j].d_e = inf.readLong();
        roads[j].h_e = inf.readLong();
        roads[j].g_e = inf.readLong();
        adj[roads[j].a].push_back({roads[j].b, j});
        adj[roads[j].b].push_back({roads[j].a, j});
    }

    struct Trip {
        long long tau;
        int x;
        long long L, w, p;
    };
    vector<Trip> trips(m);
    for (int i = 0; i < m; i++) {
        trips[i].tau = inf.readLong();
        trips[i].x   = inf.readInt();
        trips[i].L   = inf.readLong();
        trips[i].w   = inf.readLong();
        trips[i].p   = inf.readLong();
    }

    // ---- Build tree via BFS from root 1 ----
    vector<int> parent(n + 1, 0);
    vector<int> parent_road(n + 1, 0);
    vector<bool> visited(n + 1, false);
    {
        queue<int> q;
        q.push(1);
        visited[1] = true;
        while (!q.empty()) {
            int u = q.front(); q.pop();
            for (auto [v, ri] : adj[u]) {
                if (!visited[v]) {
                    visited[v] = true;
                    parent[v] = u;
                    parent_road[v] = ri;
                    q.push(v);
                }
            }
        }
    }

    // ---- Helper: compute OBJ for a given preserved set ----
    auto computeObj = [&](const vector<bool>& preserved) -> long long {
        long long obj = 0;
        for (int i = 0; i < m; i++) {
            int dest = trips[i].x;
            long long tau = trips[i].tau;
            long long L   = trips[i].L;
            long long w   = trips[i].w;
            long long p   = trips[i].p;

            long long total_time = 0;
            long long total_nos  = 0;

            int cur = dest;
            while (cur != 1) {
                int ri = parent_road[cur];
                bool is_dirt = preserved[ri] || (tau < roads[ri].t_e);
                if (is_dirt) {
                    total_time += roads[ri].d_e;
                    total_nos  += roads[ri].g_e;
                } else {
                    total_time += roads[ri].h_e;
                }
                cur = parent[cur];
            }

            long long lateness = max(0LL, total_time - L);
            obj += w * total_nos - p * lateness;
        }
        return obj;
    };

    // ---- Compute BASE (no roads preserved) ----
    vector<bool> no_preserved(R + 1, false);
    long long BASE = computeObj(no_preserved);

    // ---- Compute UPPER = sum over trips of w_i * total_g_i (all dirt, no penalty) ----
    long long UPPER = 0;
    for (int i = 0; i < m; i++) {
        int dest = trips[i].x;
        long long ww = trips[i].w;
        long long tg = 0;
        int cur = dest;
        while (cur != 1) {
            int ri = parent_road[cur];
            tg += roads[ri].g_e;
            cur = parent[cur];
        }
        UPPER += ww * tg;
    }

    // ---- Read participant output ----
    int k = ouf.readInt(0, R, "k must be between 0 and n-1");

    vector<int> chosen(k);
    for (int i = 0; i < k; i++) {
        chosen[i] = ouf.readInt(1, R, "road ID must be between 1 and n-1");
    }

    // ---- Strict EOF check: reject trailing garbage ----
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra output found after the %d road IDs", k);
    }

    // ---- Check distinctness ----
    {
        set<int> seen;
        for (int i = 0; i < k; i++) {
            if (seen.count(chosen[i])) {
                quitf(_wa, "Duplicate road ID %d in preservation list", chosen[i]);
            }
            seen.insert(chosen[i]);
        }
    }

    // ---- Compute cost and mark preserved ----
    vector<bool> preserved(R + 1, false);
    long long total_cost = 0;
    for (int i = 0; i < k; i++) {
        int ri = chosen[i];
        preserved[ri] = true;
        total_cost += roads[ri].c_e;
    }

    if (total_cost > B) {
        quitf(_wa, "Total preservation cost %lld exceeds budget %lld", total_cost, B);
    }

    // ---- Compute ANS ----
    long long ANS = computeObj(preserved);

    // ---- Compute score ratio per statement formula ----
    // Statement: floor(1_000_000 * clamp((ANS - BASE) / (UPPER - BASE), 0, 1))
    // quitp expects a value in [0,1]; the framework handles the integer scaling.
    if (UPPER == BASE) {
        // No room for improvement; any feasible plan gets full score
        quitp(1.0, "Ratio: 1.0 | BASE=UPPER=%lld ANS=%lld cost=%lld budget=%lld k=%d",
              BASE, ANS, total_cost, B, k);
    }

    double ratio = (double)(ANS - BASE) / (double)(UPPER - BASE);
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    quitp(ratio,
          "Ratio: %.9f | ANS=%lld BASE=%lld UPPER=%lld cost=%lld budget=%lld k=%d",
          ratio, ANS, BASE, UPPER, total_cost, B, k);

    return 0;
}