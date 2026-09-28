#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- read problem input ----
    int N = inf.readInt();
    long long C = inf.readLong();
    long long B = inf.readLong();

    vector<long long> S(N+1), W(N+1), A(N+1), R(N+1);
    for (int i = 1; i <= N; i++) S[i] = inf.readLong();
    for (int i = 1; i <= N; i++) W[i] = inf.readLong();
    for (int i = 1; i <= N; i++) A[i] = inf.readLong();
    for (int i = 1; i <= N; i++) R[i] = inf.readLong();

    // ---- read participant output ----
    int K = ouf.readInt(0, N, "K must be in [0,N]");

    vector<bool> used(N+1, false);
    long long totalFatigue = 0;
    long long totalRevenue = 0;

    for (int j = 0; j < K; j++) {
        int tj = ouf.readInt(1, N, "t_j must be >= 1");

        long long tripWeight = 0;
        long long tripMaxS = 0;
        long long tripSumA = 0;
        long long tripRev = 0;

        for (int k = 0; k < tj; k++) {
            int idx = ouf.readInt(1, N, "household index must be in [1,N]");
            if (used[idx]) {
                quitf(_wa, "Household %d appears more than once in the output", idx);
            }
            used[idx] = true;
            tripWeight += W[idx];
            if (S[idx] > tripMaxS) tripMaxS = S[idx];
            tripSumA += A[idx];
            tripRev += R[idx];
        }

        if (tripWeight > C) {
            quitf(_wa, "Trip %d exceeds weight capacity: %lld > %lld", j+1, tripWeight, C);
        }

        long long fatigue = 2LL * tripMaxS + tripSumA;
        totalFatigue += fatigue;

        if (totalFatigue > B) {
            quitf(_wa, "Total fatigue %lld exceeds budget %lld after trip %d",
                  totalFatigue, B, j+1);
        }

        totalRevenue += tripRev;
    }

    // ---- check for trailing output ----
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra output found after the %d trips", K);
    }

    // ---- compute baseline revenue ----
    // E_i = R_i / (2*S_i + A_i)
    // Sort: decreasing E_i, then smaller S_i, then smaller index
    vector<int> order(N);
    for (int i = 0; i < N; i++) order[i] = i + 1;

    sort(order.begin(), order.end(), [&](int a, int b) {
        // Compare R_a/(2*S_a+A_a) vs R_b/(2*S_b+A_b) via cross-multiply
        // R up to 1e6, (2*S+A) up to ~2e9+1000 => product up to ~2e15, fits in int64
        long long lhs = R[a] * (2LL * S[b] + A[b]);
        long long rhs = R[b] * (2LL * S[a] + A[a]);
        if (lhs != rhs) return lhs > rhs;      // higher efficiency first
        if (S[a] != S[b]) return S[a] < S[b]; // smaller S
        return a < b;                           // smaller index
    });

    long long baseUsed = 0;
    long long baseRevenue = 0;

    for (int i = 0; i < N; i++) {
        int idx = order[i];
        long long fat = 2LL * S[idx] + A[idx];
        if (baseUsed + fat <= B) {
            baseRevenue += R[idx];
            baseUsed += fat;
        }
    }

    // ---- compute score using EXACT stated formula ----
    // Score_test = clamp(0, 2000000, round(1000000 * Value / Base))
    double ratio = 0.0;
    if (baseRevenue <= 0) {
        // Edge case: base revenue is 0; any feasible solution gets full score
        ratio = 1.0;
    } else {
        // Apply round() before clamping, as stated
        long long scoreTest = llround(1000000.0 * (double)totalRevenue / (double)baseRevenue);
        // clamp to [0, 2000000]
        if (scoreTest < 0LL) scoreTest = 0LL;
        if (scoreTest > 2000000LL) scoreTest = 2000000LL;
        // normalize to [0, 1] for quitp
        ratio = (double)scoreTest / 2000000.0;
    }

    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    quitp(ratio,
          "Ratio: %.9f | Value: %lld | Base: %lld | ScoreX1M: %lld",
          ratio,
          totalRevenue,
          baseRevenue,
          (long long)round(ratio * 2000000.0));

    return 0;
}