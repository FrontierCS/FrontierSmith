#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

// Compute bestCost for pattern (b_req, g_req) using given pack types.
// Returns LLONG_MAX if infeasible.
// We run Dijkstra on state (b_covered, g_covered) capped at (b_req, g_req).
// Augmented cost per pack j: A + W*(x_j + y_j)
// bestCost = dp[b_req][g_req] - W*(b_req + g_req)
long long computeBestCost(
    int b_req, int g_req,
    const vector<pair<int,int>>& packs,
    long long A, long long W)
{
    const long long INF_VAL = (long long)4e18;
    if (b_req == 0 && g_req == 0) return 0LL;

    vector<vector<long long>> dp(b_req + 1, vector<long long>(g_req + 1, INF_VAL));
    dp[0][0] = 0;

    // Dijkstra: state (cost, b, g)
    priority_queue<tuple<long long,int,int>,
                   vector<tuple<long long,int,int>>,
                   greater<tuple<long long,int,int>>> pq;
    pq.push({0LL, 0, 0});

    while (!pq.empty()) {
        auto [cost, b, g] = pq.top(); pq.pop();
        if (cost > dp[b][g]) continue;
        if (b == b_req && g == g_req) break;
        for (auto& [x, y] : packs) {
            long long nc = cost + A + W * (long long)(x + y);
            int nb = min(b + x, b_req);
            int ng = min(g + y, g_req);
            if (nc < dp[nb][ng]) {
                dp[nb][ng] = nc;
                pq.push({nc, nb, ng});
            }
        }
    }

    if (dp[b_req][g_req] == INF_VAL) return INF_VAL;
    return dp[b_req][g_req] - W * (long long)(b_req + g_req);
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- Read problem input ----
    int M = inf.readInt();
    int K = inf.readInt();
    int C = inf.readInt();
    long long F = inf.readLong();
    long long A = inf.readLong();
    long long W = inf.readLong();

    vector<int> b_arr(M), g_arr(M);
    vector<long long> w_arr(M);
    for (int i = 0; i < M; i++) {
        b_arr[i] = inf.readInt();
        g_arr[i] = inf.readInt();
        w_arr[i] = inf.readLong();
    }

    // ---- Read participant output ----
    int T = ouf.readInt();

    // Check T bounds
    if (T < 1 || T > K) {
        quitf(_wa, "T=%d is out of range [1,%d]", T, K);
    }

    vector<pair<int,int>> packs(T);
    set<pair<int,int>> seen;
    for (int j = 0; j < T; j++) {
        int x = ouf.readInt();
        int y = ouf.readInt();
        if (x < 0 || y < 0) {
            quitf(_wa, "Pack %d: negative value x=%d y=%d", j, x, y);
        }
        if (x + y < 1 || x + y > C) {
            quitf(_wa, "Pack %d: x+y=%d violates 1<=x+y<=%d", j, x+y, C);
        }
        if (seen.count({x, y})) {
            quitf(_wa, "Duplicate pack type (%d,%d)", x, y);
        }
        seen.insert({x, y});
        packs[j] = {x, y};
    }

    // Check EOF
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra data after the last pack type");
    }

    // ---- Compute participant objective ----
    const long long INF_VAL = (long long)4e18;
    long long obj = F * (long long)T;
    for (int i = 0; i < M; i++) {
        long long bc = computeBestCost(b_arr[i], g_arr[i], packs, A, W);
        if (bc == INF_VAL) {
            quitf(_wa, "Pattern %d (%d blue, %d red) cannot be served by given pack types", i+1, b_arr[i], g_arr[i]);
        }
        obj += w_arr[i] * bc;
    }

    // ---- Compute baseline objective using {(1,0),(0,1)} ----
    // For pattern (b,g): open b packs of (1,0) and g packs of (0,1)
    // cost = A*(b+g) + W*0 = A*(b+g)
    // This is always feasible since C>=1 and (1,0),(0,1) each have size 1.
    // But we must verify C>=1 (guaranteed by constraints: C>=1).
    vector<pair<int,int>> baseline_packs = {{1, 0}, {0, 1}};
    long long baselineObj = F * 2LL;
    for (int i = 0; i < M; i++) {
        long long bc = computeBestCost(b_arr[i], g_arr[i], baseline_packs, A, W);
        // baseline is always feasible
        baselineObj += w_arr[i] * bc;
    }

    // ---- Compute score ----
    // score = 100 * clamp(baselineObj / obj, 0, 3)
    // We use config score=300, so pass ratio/3 to quitp where ratio = clamp(B/O,0,3)
    // That gives (ratio/3)*300 = 100*ratio points.
    double ratio;
    if (obj <= 0) {
        ratio = 3.0;
    } else {
        ratio = (double)baselineObj / (double)obj;
    }
    if (ratio > 3.0) ratio = 3.0;
    if (ratio < 0.0) ratio = 0.0;

    // score_ratio for quitp must be in [0,1]
    // Since config score=300, quitp(r) gives r*300 points.
    // We want points = 100*ratio, so pass ratio/3.
    double score_ratio = ratio / 3.0;
    if (score_ratio > 1.0) score_ratio = 1.0;
    if (score_ratio < 0.0) score_ratio = 0.0;

    quitp(score_ratio,
          "Objective=%lld Baseline=%lld Ratio: %.9f",
          obj, baselineObj, score_ratio);

    return 0;
}