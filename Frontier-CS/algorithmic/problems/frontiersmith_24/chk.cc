#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // --- Read input ---
    int N = inf.readInt();
    int K = inf.readInt();
    long long B = inf.readLong();

    vector<vector<long long>> V(N, vector<long long>(N));
    vector<vector<long long>> C(N, vector<long long>(N));
    vector<vector<int>> U(N, vector<int>(N));

    for (int r = 0; r < N; r++)
        for (int c = 0; c < N; c++)
            V[r][c] = inf.readLong();

    for (int r = 0; r < N; r++)
        for (int c = 0; c < N; c++)
            C[r][c] = inf.readLong();

    for (int r = 0; r < N; r++)
        for (int c = 0; c < N; c++)
            U[r][c] = inf.readInt();

    // --- Compute baseline: all K drills in column 1 (0-indexed col 0) ---
    long long col1_reward = 0;
    for (int r = 0; r < N; r++)
        col1_reward += V[r][0];
    long long R_base = (long long)K * col1_reward;

    if (R_base <= 0) {
        // Per problem guarantees this should not happen, but guard defensively
        quitf(_fail, "Baseline reward is non-positive: %lld", R_base);
    }

    // --- Read participant output ---
    // K lines, each with N integers (1-indexed lanes)
    vector<vector<int>> paths(K, vector<int>(N));
    for (int i = 0; i < K; i++) {
        for (int r = 0; r < N; r++) {
            paths[i][r] = ouf.readInt();
        }
    }
    // NOTE: Do NOT call ouf.readEof() here.
    // Contestant programs typically emit a trailing newline; calling readEof()
    // would reject every valid submission with "Expected EOF".

    // --- Validate feasibility ---

    // Check 1: lane bounds
    for (int i = 0; i < K; i++) {
        for (int r = 0; r < N; r++) {
            if (paths[i][r] < 1 || paths[i][r] > N) {
                quitf(_wa, "Drill %d at level %d: lane %d out of range [1,%d]",
                      i+1, r+1, paths[i][r], N);
            }
        }
    }

    // Check 2: consecutive lane difference <= 1
    for (int i = 0; i < K; i++) {
        for (int r = 0; r < N-1; r++) {
            int diff = abs(paths[i][r+1] - paths[i][r]);
            if (diff > 1) {
                quitf(_wa, "Drill %d: move from level %d (lane %d) to level %d (lane %d) invalid (diff=%d)",
                      i+1, r+1, paths[i][r], r+2, paths[i][r+1], diff);
            }
        }
    }

    // Count drills per cell
    vector<vector<int>> drill_count(N, vector<int>(N, 0));
    for (int i = 0; i < K; i++) {
        for (int r = 0; r < N; r++) {
            int c = paths[i][r] - 1; // 0-indexed
            drill_count[r][c]++;
        }
    }

    // Check 3: capacity constraints
    for (int r = 0; r < N; r++) {
        for (int c = 0; c < N; c++) {
            if (drill_count[r][c] > U[r][c]) {
                quitf(_wa, "Cell (%d,%d) used by %d drills but capacity is %d",
                      r+1, c+1, drill_count[r][c], U[r][c]);
            }
        }
    }

    // Check 4: budget constraint
    long long total_setup_cost = 0;
    for (int r = 0; r < N; r++) {
        for (int c = 0; c < N; c++) {
            if (drill_count[r][c] > 0) {
                total_setup_cost += C[r][c];
            }
        }
    }
    if (total_setup_cost > B) {
        quitf(_wa, "Total setup cost %lld exceeds budget %lld", total_setup_cost, B);
    }

    // --- Compute reward ---
    long long R_sub = 0;
    for (int i = 0; i < K; i++) {
        for (int r = 0; r < N; r++) {
            int c = paths[i][r] - 1; // 0-indexed
            R_sub += V[r][c];
        }
    }

    // --- Compute score ratio ---
    // Score_test = 10^6 * clamp(R_sub / R_base, 0, 10)
    // We pass score_ratio = clamp(R_sub/R_base, 0, 10) / 10  into [0,1] for quitp.
    double raw_ratio = (double)R_sub / (double)R_base;
    if (raw_ratio < 0.0) raw_ratio = 0.0;
    if (raw_ratio > 10.0) raw_ratio = 10.0;
    double score_ratio = raw_ratio / 10.0; // normalise to [0,1]

    quitp(score_ratio,
          "Feasible. R_sub=%lld R_base=%lld setup_cost=%lld budget=%lld "
          "raw_ratio=%.6f Ratio: %.9f",
          R_sub, R_base, total_setup_cost, B, raw_ratio, score_ratio);

    return 0;
}