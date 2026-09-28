#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

// Omega function: count prime factors with multiplicity
int bigOmega(long long x) {
    int cnt = 0;
    for (long long p = 2; p * p <= x; p++) {
        while (x % p == 0) {
            cnt++;
            x /= p;
        }
    }
    if (x > 1) cnt++;
    return cnt;
}

long long myGcd(long long a, long long b) {
    while (b) { a %= b; swap(a, b); }
    return a;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    int T = inf.readInt(1, 5, "T");

    // We accumulate per-test numerators and denominators separately
    // to compute the mean ratio correctly.
    // Each test contributes one ratio in [0,1].
    // Mean = sum(ratio_i) / T
    double totalRatio = 0.0;

    for (int tc = 0; tc < T; tc++) {
        long long N = inf.readLong(1, 100000LL, "N");
        long long M = inf.readLong(0, 200000LL, "M");
        long long K = inf.readLong(1, 1000000000LL, "K");
        long long S = inf.readLong(0, 300000LL, "S");

        int n = (int)N;
        int m = (int)M;

        vector<long long> A(n);
        for (int i = 0; i < n; i++) {
            A[i] = inf.readLong(1, 1000000000LL, "A_i");
        }

        vector<int> eu(m), ev(m);
        vector<long long> ew(m);
        long long UPPER = 0;
        for (int i = 0; i < m; i++) {
            eu[i] = inf.readInt(1, n, "u") - 1;
            ev[i] = inf.readInt(1, n, "v") - 1;
            ew[i] = inf.readLong(1, 1000000LL, "w");
            UPPER += ew[i];
        }

        // BASE: R_i = 1 for all i, so B_i = A_i
        long long BASE = 0;
        for (int i = 0; i < m; i++) {
            if (myGcd(A[eu[i]], A[ev[i]]) == 1LL) {
                BASE += ew[i];
            }
        }

        // Read participant's R values
        vector<long long> R(n);
        bool feasible = true;
        string failReason;

        for (int i = 0; i < n; i++) {
            if (ouf.seekEof()) {
                quitf(_wa, "Test case %d: unexpected end of output while reading R[%d]",
                      tc + 1, i + 1);
            }
            R[i] = ouf.readLong(-2000000000LL, 2000000000LL, "R_i");
        }

        // Validate feasibility
        for (int i = 0; i < n && feasible; i++) {
            if (R[i] < 1LL || R[i] > K) {
                quitf(_wa,
                      "Test case %d: R[%d] = %lld is out of valid range [1, %lld]",
                      tc + 1, i + 1, R[i], K);
            }
            if (A[i] % R[i] != 0LL) {
                quitf(_wa,
                      "Test case %d: R[%d] = %lld does not divide A[%d] = %lld",
                      tc + 1, i + 1, R[i], i + 1, A[i]);
            }
        }

        long long totalCost = 0;
        for (int i = 0; i < n; i++) {
            totalCost += (long long)bigOmega(R[i]);
        }
        if (totalCost > S) {
            quitf(_wa,
                  "Test case %d: total Omega cost %lld exceeds budget S = %lld",
                  tc + 1, totalCost, S);
        }

        // Compute B_i = A_i / R_i
        vector<long long> B(n);
        for (int i = 0; i < n; i++) {
            B[i] = A[i] / R[i];
        }

        // Compute objective V
        long long Vval = 0;
        for (int i = 0; i < m; i++) {
            if (myGcd(B[eu[i]], B[ev[i]]) == 1LL) {
                Vval += ew[i];
            }
        }

        // Per-test ratio in [0,1]
        double ratio;
        if (UPPER == BASE) {
            // Baseline already achieves maximum; any feasible answer is perfect
            ratio = 1.0;
        } else {
            // Continuous scoring as per problem statement
            ratio = (double)(Vval - BASE) / (double)(UPPER - BASE);
            if (ratio < 0.0) ratio = 0.0;
            if (ratio > 1.0) ratio = 1.0;
        }

        totalRatio += ratio;
    }

    // Check no trailing garbage
    if (!ouf.seekEof()) {
        quitf(_wa, "Participant output has extra tokens after all test cases");
    }

    double finalRatio = totalRatio / (double)T;
    if (finalRatio < 0.0) finalRatio = 0.0;
    if (finalRatio > 1.0) finalRatio = 1.0;

    // The judge parses "Ratio: <value>" from this message
    quitp(finalRatio, "Ratio: %.9f (mean over %d test case(s))", finalRatio, T);

    return 0;
}