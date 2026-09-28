#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- Read input ----
    int N = inf.readInt();

    vector<long long> B(N + 1, 0LL);
    for (int l = 3; l <= N; l++) {
        B[l] = inf.readLong();
    }

    vector<vector<long long>> W(N + 1, vector<long long>(N + 1, 0LL));
    for (int i = 1; i <= N; i++)
        for (int j = 1; j <= N; j++)
            W[i][j] = inf.readLong();

    // ---- Read participant output ----
    vector<int> pi(N + 1);
    for (int i = 1; i <= N; i++) {
        if (ouf.seekEof()) {
            quitf(_wa, "expected %d integers but stream ended at position %d", N, i - 1);
        }
        pi[i] = ouf.readInt();
    }

    // Strict EOF check: reject any trailing non-whitespace tokens
    if (!ouf.seekEof()) {
        quitf(_wa, "extra output after the %d integers", N);
    }

    // ---- Feasibility checks ----
    for (int i = 1; i <= N; i++) {
        if (pi[i] < 1 || pi[i] > N) {
            quitf(_wa, "pi[%d] = %d is out of range [1, %d]", i, pi[i], N);
        }
    }
    for (int i = 1; i <= N; i++) {
        if (pi[i] == i) {
            quitf(_wa, "pi[%d] = %d is a self-hold (not allowed)", i, i);
        }
    }
    vector<int> cnt(N + 1, 0);
    for (int i = 1; i <= N; i++) cnt[pi[i]]++;
    for (int j = 1; j <= N; j++) {
        if (cnt[j] != 1) {
            quitf(_wa, "value %d appears %d time(s) in the permutation (must be exactly 1)", j, cnt[j]);
        }
    }

    // Find cycles and check length >= 3
    vector<bool> visited(N + 1, false);
    vector<int> cycle_lengths;
    for (int start = 1; start <= N; start++) {
        if (!visited[start]) {
            int cur = start;
            int len = 0;
            while (!visited[cur]) {
                visited[cur] = true;
                cur = pi[cur];
                len++;
            }
            if (len < 3) {
                quitf(_wa, "cycle starting at %d has length %d (minimum is 3)", start, len);
            }
            cycle_lengths.push_back(len);
        }
    }

    // ---- Compute objective ----
    long long obj = 0;
    for (int i = 1; i <= N; i++) obj += W[i][pi[i]];
    for (int len : cycle_lengths) obj += B[len];

    // ---- Compute baseline ----
    vector<int> base_pi(N + 1, 0);
    auto setup_cycle = [&](int l, int r) {
        for (int i = l; i < r; i++) base_pi[i] = i + 1;
        base_pi[r] = l;
    };

    if (N % 3 == 0) {
        for (int i = 1; i <= N; i += 3) setup_cycle(i, i + 2);
    } else if (N % 3 == 1) {
        for (int i = 1; i <= N - 4; i += 3) setup_cycle(i, i + 2);
        setup_cycle(N - 3, N);
    } else {
        // N % 3 == 2
        for (int i = 1; i <= N - 5; i += 3) setup_cycle(i, i + 2);
        setup_cycle(N - 4, N);
    }

    long long base_edges = 0;
    for (int i = 1; i <= N; i++) base_edges += W[i][base_pi[i]];

    long long base_bonus = 0;
    {
        vector<bool> vis2(N + 1, false);
        for (int start = 1; start <= N; start++) {
            if (!vis2[start]) {
                int cur = start;
                int len = 0;
                while (!vis2[cur]) {
                    vis2[cur] = true;
                    cur = base_pi[cur];
                    len++;
                }
                base_bonus += B[len];
            }
        }
    }

    long long base_val = base_edges + base_bonus;

    // ---- Compute upper bound ----
    // U_edge: for each row pick the max off-diagonal entry
    long long u_edge = 0;
    for (int i = 1; i <= N; i++) {
        long long best = -1;
        for (int j = 1; j <= N; j++) {
            if (j != i) best = max(best, W[i][j]);
        }
        u_edge += best;
    }

    // U_bonus via DP over partitions of N into parts >= 3
    const long long NEG_INF = (long long)(-4e18);
    vector<long long> dp(N + 1, NEG_INF);
    dp[0] = 0LL;
    for (int x = 1; x <= N; x++) {
        for (int l = 3; l <= x; l++) {
            if (dp[x - l] != NEG_INF) {
                dp[x] = max(dp[x], dp[x - l] + B[l]);
            }
        }
    }
    long long u_bonus = dp[N];
    long long ub = u_edge + u_bonus;

    // ---- Compute score as per statement ----
    // Score = floor(1,000,000 * clamp(0.5 + 0.5*(Obj-Base)/(UB-Base), 0, 1))
    // Special case: if UB == Base, any feasible solution gets full score
    long long integer_score;
    if (ub == base_val) {
        integer_score = 1000000LL;
    } else {
        double denom = (double)(ub - base_val);
        double numer = (double)(obj - base_val);
        double raw = 0.5 + 0.5 * numer / denom;
        if (raw < 0.0) raw = 0.0;
        if (raw > 1.0) raw = 1.0;
        // Apply floor as specified in the statement
        integer_score = (long long)(1000000.0 * raw);
    }

    // Return ratio in [0,1] for quitp; Ratio tag must match the ratio we report
    double score_ratio = (double)integer_score / 1000000.0;

    quitp(score_ratio,
        "Obj=%lld Base=%lld UB=%lld IntScore=%lld Ratio: %.9f",
        obj, base_val, ub, integer_score, score_ratio);

    return 0;
}