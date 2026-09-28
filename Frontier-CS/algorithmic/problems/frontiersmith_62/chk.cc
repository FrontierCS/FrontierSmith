#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

// O(n log n) LIS length
long long computeLIS(const vector<int>& a) {
    vector<int> tails;
    for (int x : a) {
        auto it = lower_bound(tails.begin(), tails.end(), x);
        if (it == tails.end()) tails.push_back(x);
        else *it = x;
    }
    return (long long)tails.size();
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // --- Read problem input ---
    int N = inf.readInt();
    int M = inf.readInt();
    long long C = (long long)inf.readInt();

    vector<int> T(M), W(M);
    vector<vector<int>> B(M);
    long long UB = 0;

    for (int i = 0; i < M; i++) {
        T[i] = inf.readInt();
        W[i] = inf.readInt();
        B[i].resize(T[i]);
        for (int j = 0; j < T[i]; j++) {
            B[i][j] = inf.readInt();
        }
        UB += (long long)W[i];
    }

    // --- Read participant output ---
    vector<int> P(N);
    for (int j = 0; j < N; j++) {
        P[j] = ouf.readInt(1, N, "permutation element");
    }
    // Assert no trailing tokens
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra tokens found after the permutation");
    }

    // Validate it is a permutation
    vector<int> cnt(N + 1, 0);
    for (int j = 0; j < N; j++) {
        cnt[P[j]]++;
        if (cnt[P[j]] > 1) {
            quitf(_wa, "Duplicate value %d in permutation", P[j]);
        }
    }

    // Position array: pos[v] = 0-based index of exhibit v in P
    vector<int> posArr(N + 1);
    for (int j = 0; j < N; j++) {
        posArr[P[j]] = j;
    }

    // --- Compute RouteScore for participant's permutation ---
    long long routeScore = 0;
    for (int i = 0; i < M; i++) {
        bool sat = true;
        for (int j = 1; j < T[i]; j++) {
            if (posArr[B[i][j]] <= posArr[B[i][j - 1]]) {
                sat = false;
                break;
            }
        }
        if (sat) routeScore += (long long)W[i];
    }

    // --- Compute LIS for participant's permutation ---
    long long lis = computeLIS(P);
    long long objective = routeScore - C * lis * lis;

    // --- Compute BASE (reverse permutation: N, N-1, ..., 1) ---
    // pos[v] = N - v (0-based), so strictly increasing pos requires v strictly decreasing.
    long long baseRouteScore = 0;
    for (int i = 0; i < M; i++) {
        bool sat = true;
        for (int j = 1; j < T[i]; j++) {
            if (B[i][j] >= B[i][j - 1]) {
                sat = false;
                break;
            }
        }
        if (sat) baseRouteScore += (long long)W[i];
    }
    // LIS of (N, N-1, ..., 1) is 1
    long long baseLIS = 1LL;
    long long BASE = baseRouteScore - C * baseLIS * baseLIS;

    // --- Score formula ---
    // Score = floor(10^9 * max(0, Objective - BASE) / max(1, UB - BASE))
    long long num = objective - BASE;
    if (num < 0) num = 0;
    long long den = UB - BASE;
    if (den < 1) den = 1;

    // Compute floor(10^9 * num / den) using __int128 to avoid overflow
    // num can be up to ~10^13, 10^9 * num up to ~10^22, which overflows long long.
    long long score_int = 0;
    if (num > 0) {
        __int128 big_num = (__int128)num * 1000000000LL;
        __int128 big_den = (__int128)den;
        __int128 big_score = big_num / big_den;  // floor division (both positive)
        if (big_score > (__int128)1000000000LL) {
            big_score = (__int128)1000000000LL;
        }
        score_int = (long long)big_score;
    }

    // Convert back to [0,1] ratio for quitp
    double score_ratio = (double)score_int / 1000000000.0;
    // Clamp to [0, 1] just in case
    if (score_ratio < 0.0) score_ratio = 0.0;
    if (score_ratio > 1.0) score_ratio = 1.0;

    quitp(score_ratio,
          "Objective=%lld, RouteScore=%lld, LIS=%lld, BASE=%lld, UB=%lld, "
          "ScoreInt=%lld, Ratio: %.9f",
          objective, routeScore, lis, BASE, UB, score_int, score_ratio);

    return 0;
}