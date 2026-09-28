#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- Read input ----
    int n = inf.readInt();
    int m = inf.readInt();

    vector<long long> x(n + 1);
    for (int i = 1; i <= n; i++)
        x[i] = inf.readLong();

    vector<long long> P(m + 1), B(m + 1), T(m + 1), W(m + 1), L(m + 1), U(m + 1);
    for (int j = 1; j <= m; j++) {
        P[j] = inf.readLong();
        B[j] = inf.readLong();
        T[j] = inf.readLong();
        W[j] = inf.readLong();
        L[j] = inf.readLong();
        U[j] = inf.readLong();
    }

    // ---- Read participant output ----
    vector<vector<int>> asgn(m + 1);
    for (int j = 1; j <= m; j++) {
        int k = ouf.readInt(0, n,
            ("Basket " + to_string(j) + ": invalid count").c_str());
        asgn[j].resize(k);
        for (int t = 0; t < k; t++) {
            asgn[j][t] = ouf.readInt(1, n,
                ("Basket " + to_string(j) + ": ball index out of range").c_str());
        }
    }

    // ---- EOF check: reject trailing tokens ----
    if (!ouf.seekEof())
        quitf(_wa, "Extra output after the last basket line");

    // ---- Feasibility checks ----
    vector<bool> used(n + 1, false);
    for (int j = 1; j <= m; j++) {
        long long kj = (long long)asgn[j].size();
        if (kj < L[j] || kj > U[j])
            quitf(_wa, "Basket %d: count=%lld not in [%lld,%lld]", j, kj, L[j], U[j]);
        for (int ball : asgn[j]) {
            if (used[ball])
                quitf(_wa, "Ball %d is used more than once", ball);
            used[ball] = true;
        }
    }
    for (int i = 1; i <= n; i++) {
        if (!used[i])
            quitf(_wa, "Ball %d is never used", i);
    }

    // ---- Compute participant error E_part ----
    long long E_part = 0;
    for (int j = 1; j <= m; j++) {
        long long v = 1;
        for (int ball : asgn[j]) {
            long long r = ((x[ball] % P[j]) + P[j]) % P[j];
            v = ((__int128)v * r + B[j]) % P[j];
        }
        long long d1 = (v - T[j] + P[j]) % P[j];
        long long d2 = (T[j] - v + P[j]) % P[j];
        E_part += W[j] * min(d1, d2);
    }

    // ---- Compute baseline error E_base ----
    // Baseline: greedy, process balls 1..n in input order.
    // For ball i, pick the available basket j minimizing W_j * circ_dist(v_j', T_j).
    // Available means: c_j < U_j, and remaining balls after assigning this one
    // are still >= sum of remaining lower-bounds for other baskets.
    vector<long long> bv(m + 1, 1LL);
    vector<long long> bc(m + 1, 0LL);

    for (int i = 1; i <= n; i++) {
        // Remaining balls after assigning ball i: n - i (balls i+1..n)
        // Total remaining lower bounds if we assign ball i to basket j:
        //   sum_{k != j} max(0, L[k] - bc[k])
        // We need n - i >= that sum.

        int best_j = -1;
        long long best_cost = -1;
        long long best_cnt = -1;

        for (int j = 1; j <= m; j++) {
            if (bc[j] >= U[j]) continue;

            // Check feasibility: remaining balls = n - i
            // lower bounds sum excluding j (with j having one more)
            long long need = 0;
            for (int k = 1; k <= m; k++) {
                long long ck = bc[k] + (k == j ? 1 : 0);
                if (L[k] > ck) need += L[k] - ck;
            }
            long long remaining = (long long)(n - i);
            if (remaining < need) continue;

            long long r = ((x[i] % P[j]) + P[j]) % P[j];
            long long vp = ((__int128)bv[j] * r + B[j]) % P[j];
            long long d1 = (vp - T[j] + P[j]) % P[j];
            long long d2 = (T[j] - vp + P[j]) % P[j];
            long long cost = W[j] * min(d1, d2);

            bool better = false;
            if (best_j == -1) {
                better = true;
            } else if (cost < best_cost) {
                better = true;
            } else if (cost == best_cost && bc[j] < best_cnt) {
                better = true;
            } else if (cost == best_cost && bc[j] == best_cnt && j < best_j) {
                better = true;
            }

            if (better) {
                best_j    = j;
                best_cost = cost;
                best_cnt  = bc[j];
            }
        }

        if (best_j == -1)
            quitf(_fail, "Baseline greedy: no available basket for ball %d", i);

        long long r = ((x[i] % P[best_j]) + P[best_j]) % P[best_j];
        bv[best_j] = ((__int128)bv[best_j] * r + B[best_j]) % P[best_j];
        bc[best_j]++;
    }

    long long E_base = 0;
    for (int j = 1; j <= m; j++) {
        long long d1 = (bv[j] - T[j] + P[j]) % P[j];
        long long d2 = (T[j] - bv[j] + P[j]) % P[j];
        E_base += W[j] * min(d1, d2);
    }

    // ---- Score formula (exact per statement) ----
    // Score = min(2,000,000, floor(1,000,000 * (E_base+1) / (E_part+1)))
    // Use integer arithmetic for the floor.
    long long score_int = 1000000LL * (E_base + 1) / (E_part + 1);
    if (score_int > 2000000LL) score_int = 2000000LL;
    if (score_int < 0LL) score_int = 0LL;

    // quitp expects a ratio in [0,1] relative to the max score (2,000,000).
    double score_ratio = (double)score_int / 2000000.0;
    if (score_ratio > 1.0) score_ratio = 1.0;
    if (score_ratio < 0.0) score_ratio = 0.0;

    quitp(score_ratio,
          "E_participant=%lld E_base=%lld score=%lld Ratio: %.9f",
          E_part, E_base, score_int, score_ratio);

    return 0;
}