#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // --- Read problem input from inf ---
    int n = inf.readInt();
    long long M = inf.readLong();

    vector<long long> x(n + 1), y(n + 1), w(n + 1);
    for (int i = 1; i <= n; i++) {
        x[i] = inf.readLong();
        y[i] = inf.readLong();
        w[i] = inf.readLong();
    }

    vector<int> p(n + 1);
    for (int i = 1; i <= n; i++) {
        p[i] = inf.readInt();
    }

    // --- Read participant output from ouf ---
    vector<int> q(n + 1);
    for (int i = 1; i <= n; i++) {
        // Use readInt() with no format args (valid testlib API)
        q[i] = ouf.readInt();
        if (q[i] < 1 || q[i] > n) {
            quitf(_wa, "q[%d] = %d is out of range [1, %d]", i, q[i], n);
        }
    }

    // Ensure no trailing tokens
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra tokens in output after reading %d values", n);
    }

    // --- Validate permutation ---
    {
        vector<bool> seen(n + 1, false);
        for (int i = 1; i <= n; i++) {
            if (seen[q[i]]) {
                quitf(_wa, "q is not a permutation: value %d appears more than once", q[i]);
            }
            seen[q[i]] = true;
        }
    }

    // --- Validate single cycle of length n ---
    {
        vector<bool> visited(n + 1, false);
        int cur = 1;
        int cycleLen = 0;
        while (!visited[cur]) {
            visited[cur] = true;
            cur = q[cur];
            cycleLen++;
        }
        if (cycleLen != n || cur != 1) {
            quitf(_wa, "q does not form a single cycle of length n (detected cycle length = %d, need %d)", cycleLen, n);
        }
    }

    // --- Helper: Manhattan distance ---
    auto mdist = [&](int a, int b) -> long long {
        return llabs(x[a] - x[b]) + llabs(y[a] - y[b]);
    };

    // --- Compute T(q) and mismatch penalty for participant's q ---
    long long T_q = 0;
    long long wpen_q = 0;
    for (int i = 1; i <= n; i++) {
        T_q += mdist(i, q[i]);
        if (q[q[i]] != p[i]) {
            wpen_q += w[i];
        }
    }
    long long P_q = M * wpen_q;
    long long Y = T_q + P_q;

    // --- Compute baseline q_base = [2, 3, ..., n, 1] ---
    long long T_base = 0;
    long long wpen_base = 0;
    for (int i = 1; i <= n; i++) {
        int qi  = (i % n) + 1;       // q_base[i]
        int qqi = (qi % n) + 1;      // q_base[q_base[i]]
        T_base += mdist(i, qi);
        if (qqi != p[i]) {
            wpen_base += w[i];
        }
    }
    long long B = T_base + M * wpen_base;

    // --- Compute score in [0, 1000] per problem statement ---
    double score;
    if (B == 0 && Y == 0) {
        score = 1000.0;
    } else if (B == 0) {
        // Y > 0
        score = 0.0;
    } else if (Y == 0) {
        // B > 0, Y == 0: ratio is infinite -> capped at 1000
        score = 1000.0;
    } else {
        score = min(1000.0, 100.0 * (double)B / (double)Y);
    }

    // Ratio in [0, 1] for quitp
    double ratio = score / 1000.0;
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    quitp(ratio,
          "Ratio: %.9f | score=%.4f/1000 | Y=%lld (T=%lld P=%lld) | B=%lld",
          ratio, score, Y, T_q, P_q, B);

    return 0;
}