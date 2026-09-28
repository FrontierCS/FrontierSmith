#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ── Read problem input ──────────────────────────────────────────────────
    int N = inf.readInt();
    int M = inf.readInt();
    int L = inf.readInt();
    long long B = inf.readLong();

    vector<int> p(N + 1), q(N + 1);
    for (int i = 1; i <= N; i++) p[i] = inf.readInt();
    for (int i = 1; i <= N; i++) q[i] = inf.readInt();

    vector<long long> w(N + 1);
    long long U = 0;
    for (int b = 1; b <= N; b++) {
        w[b] = inf.readLong();
        U += w[b];
    }

    vector<int> eu(M + 1), ev(M + 1);
    vector<long long> ec(M + 1);
    for (int e = 1; e <= M; e++) {
        eu[e] = inf.readInt();
        ev[e] = inf.readInt();
        ec[e] = inf.readLong();
    }

    // target[b] = server index i such that q[i] == b
    vector<int> target(N + 1);
    for (int i = 1; i <= N; i++) target[q[i]] = i;

    // ── Baseline: objective of the empty (K=0) sequence ────────────────────
    long long Base = 0;
    for (int b = 1; b <= N; b++) {
        if (p[target[b]] == b) Base += w[b];
    }

    // ── Handle completely empty output (treat as K=0) ───────────────────────
    if (ouf.seekEof()) {
        // Empty output treated as K=0
        long long A = Base;
        long long scoreInt;
        if (U == Base) {
            scoreInt = 1000000LL;
        } else {
            long long num = max(0LL, A - Base);
            scoreInt = (long long)floor(1000000.0 * (double)num / (double)(U - Base));
            if (scoreInt < 0) scoreInt = 0;
            if (scoreInt > 1000000) scoreInt = 1000000;
        }
        double ratio = scoreInt / 1000000.0;
        quitp(ratio,
              "Empty output treated as K=0. A=%lld Base=%lld U=%lld IntScore=%lld Ratio: %.6f",
              A, Base, U, scoreInt, ratio);
    }

    // ── Read participant K ──────────────────────────────────────────────────
    int K = ouf.readInt();
    if (K < 0 || K > L) {
        quitf(_wa, "K=%d is out of range [0, %d]", K, L);
    }

    // Simulate swaps on a working copy of the placement
    vector<int> f(N + 1);
    for (int i = 1; i <= N; i++) f[i] = p[i];

    long long totalCost = 0;
    for (int t = 0; t < K; t++) {
        if (ouf.seekEof()) {
            quitf(_wa, "Expected %d edge indices but found only %d before EOF", K, t);
        }
        int e = ouf.readInt();
        if (e < 1 || e > M) {
            quitf(_wa, "Swap %d: edge index %d is out of range [1, %d]", t + 1, e, M);
        }
        totalCost += ec[e];
        if (totalCost > B) {
            quitf(_wa,
                  "Total bandwidth cost %lld exceeds B=%lld after swap %d (edge %d)",
                  totalCost, B, t + 1, e);
        }
        swap(f[eu[e]], f[ev[e]]);
    }

    // ── Strict EOF check: reject trailing non-whitespace tokens ────────────
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra tokens found after the %d swap indices", K);
    }

    // ── Compute objective A ─────────────────────────────────────────────────
    long long A = 0;
    for (int b = 1; b <= N; b++) {
        if (f[target[b]] == b) A += w[b];
    }

    // ── Scoring per problem statement ───────────────────────────────────────
    // score = 1,000,000                                   if U == Base
    // score = floor(1,000,000 * max(0, A-Base)/(U-Base))  otherwise
    long long scoreInt;
    if (U == Base) {
        scoreInt = 1000000LL;
    } else {
        long long num = max(0LL, A - Base);
        scoreInt = (long long)floor(1000000.0 * (double)num / (double)(U - Base));
        if (scoreInt < 0) scoreInt = 0;
        if (scoreInt > 1000000) scoreInt = 1000000;
    }

    // The ratio reported MUST equal scoreInt/1000000 so the integer-million
    // rule from the statement is applied exactly.
    double ratio = scoreInt / 1000000.0;

    quitp(ratio,
          "A=%lld Base=%lld U=%lld IntScore=%lld Ratio: %.6f",
          A, Base, U, scoreInt, ratio);

    return 0;
}