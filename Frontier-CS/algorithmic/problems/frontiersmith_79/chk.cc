#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

static int popcnt64(long long x) {
    return __builtin_popcountll(x);
}

long long computeOBJ(const vector<long long>& A, const vector<long long>& W,
                     const vector<long long>& masks,
                     long long alpha, long long beta) {
    int N = (int)A.size();
    long long obj = 0;
    for (int i = 0; i < N; i++) {
        long long best = A[i];
        for (long long X : masks) {
            best = min(best, A[i] ^ X);
        }
        obj += W[i] * best;
    }
    for (long long X : masks) {
        obj += alpha + beta * popcnt64(X);
    }
    return obj;
}

long long computeBaseline(const vector<long long>& A, const vector<long long>& W,
                          int L, long long alpha, long long beta) {
    int N = (int)A.size();

    // Distinct values of A, sorted for tie-breaking
    vector<long long> candidates;
    {
        set<long long> seen;
        for (long long a : A) {
            if (seen.insert(a).second) candidates.push_back(a);
        }
    }
    sort(candidates.begin(), candidates.end());

    // Current R[i] values
    vector<long long> R(A.begin(), A.end());
    long long curOBJ = 0;
    for (int i = 0; i < N; i++) curOBJ += W[i] * R[i];

    set<long long> chosen;

    for (int step = 0; step < L; step++) {
        long long bestDelta = 0; // only improve if adding X reduces OBJ
        long long bestX = -1;

        for (long long X : candidates) {
            if (chosen.count(X)) continue;

            // Cost of adding X
            long long addCost = alpha + beta * popcnt64(X);
            // Reduction in weighted sum
            long long weightedReduction = 0;
            for (int i = 0; i < N; i++) {
                long long newR = min(R[i], A[i] ^ X);
                weightedReduction += W[i] * (R[i] - newR);
            }
            long long delta = weightedReduction - addCost; // positive = improvement

            if (delta > bestDelta || (delta == bestDelta && bestDelta > 0 && X < bestX)) {
                // tie-break: same delta, pick smaller X
                if (delta > bestDelta || (delta == bestDelta && X < bestX)) {
                    bestDelta = delta;
                    bestX = X;
                }
            }
        }

        if (bestX < 0 || bestDelta <= 0) break;

        chosen.insert(bestX);
        // Update R
        for (int i = 0; i < N; i++) {
            long long newR = A[i] ^ bestX;
            if (newR < R[i]) R[i] = newR;
        }
        curOBJ -= bestDelta;
    }

    // Recompute OBJ from scratch to be sure
    vector<long long> chosenVec(chosen.begin(), chosen.end());
    return computeOBJ(A, W, chosenVec, alpha, beta);
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    const long long MAX_PER_CASE = 2000000LL;

    int T = inf.readInt();

    long long totalScore = 0;
    long long maxScore = 0;

    for (int t = 0; t < T; t++) {
        int N = inf.readInt();
        int L = inf.readInt();
        long long alpha = inf.readLong();
        long long beta  = inf.readLong();

        vector<long long> A(N), W(N);
        for (int i = 0; i < N; i++) {
            A[i] = inf.readLong();
            W[i] = inf.readLong();
        }

        // Compute baseline B
        long long B = computeBaseline(A, W, L, alpha, beta);

        maxScore += MAX_PER_CASE;

        // Read participant's m
        int m = ouf.readInt();

        // If m is negative we cannot recover; hard-fail entire submission
        if (m < 0) {
            quitf(_wa, "Test case %d: m=%d is negative, cannot parse further", t + 1, m);
        }

        bool infeasible = false;

        // m > L is infeasible (but we can still read m masks to stay in sync)
        if (m > L) {
            infeasible = true;
        }

        set<long long> maskSet;
        vector<long long> masks;
        for (int j = 0; j < m; j++) {
            long long X = ouf.readLong();
            if (X < 0 || X >= (1LL << 30)) {
                infeasible = true;
                // still consume; do not insert into maskSet so we can read all m
            } else if (!maskSet.insert(X).second) {
                infeasible = true;
                // duplicate; still consumed
            } else {
                masks.push_back(X);
            }
        }

        if (infeasible) {
            // This test case gets score 0; totalScore unchanged
            continue;
        }

        // Compute OBJ for participant solution
        long long O = computeOBJ(A, W, masks, alpha, beta);

        // Score for this case: floor(2,000,000 * (B+1) / (B+O+2))
        long long num = B + 1;
        long long den = B + O + 2;
        long long caseScore = (long long)(MAX_PER_CASE * ((__int128)num) / den);
        if (caseScore < 0) caseScore = 0;
        if (caseScore > MAX_PER_CASE) caseScore = MAX_PER_CASE;

        totalScore += caseScore;
    }

    // Assert end of participant output (no trailing garbage)
    if (!ouf.seekEof()) {
        quitf(_wa, "Participant output has extra trailing data after all test cases");
    }

    double ratio = (maxScore > 0) ? (double)totalScore / (double)maxScore : 0.0;
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    quitp(ratio, "Ratio: %.10f | Total score: %lld / %lld", ratio, totalScore, maxScore);
}