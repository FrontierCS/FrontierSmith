#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // Read problem input
    int n = inf.readInt();
    int m = inf.readInt();

    vector<int> eu(m), ev(m), et(m);
    vector<long long> ew(m);
    long long sumW = 0;

    for (int i = 0; i < m; i++) {
        eu[i] = inf.readInt();
        ev[i] = inf.readInt();
        et[i] = inf.readInt();
        ew[i] = inf.readInt();
        sumW += ew[i];
    }

    // Read participant output: perm[i] = booth at position (i+1)
    vector<int> perm(n);
    for (int i = 0; i < n; i++) {
        perm[i] = ouf.readInt(1, n, "booth value");
    }
    // Ensure no trailing content
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra data found after the permutation");
    }

    // Validate permutation
    vector<bool> seen(n + 1, false);
    for (int i = 0; i < n; i++) {
        if (seen[perm[i]]) {
            quitf(_wa, "Booth %d appears more than once in the output", perm[i]);
        }
        seen[perm[i]] = true;
    }
    for (int b = 1; b <= n; b++) {
        if (!seen[b]) {
            quitf(_wa, "Booth %d is missing from the output", b);
        }
    }

    // Build pos[booth] = 1-indexed position
    vector<int> pos(n + 1);
    for (int i = 0; i < n; i++) {
        pos[perm[i]] = i + 1;
    }

    // Compute participant's objective S
    long long S = 0;
    for (int i = 0; i < m; i++) {
        long long d = abs(pos[eu[i]] - pos[ev[i]]);
        if (et[i] == 0) {
            S += ew[i] * ((long long)(n - 1) - d);
        } else {
            S += ew[i] * d;
        }
    }

    // Compute baseline B: layout 1 2 3 ... n, so pos[x] = x
    long long B = 0;
    for (int i = 0; i < m; i++) {
        long long d = (long long)abs(eu[i] - ev[i]);
        if (et[i] == 0) {
            B += ew[i] * ((long long)(n - 1) - d);
        } else {
            B += ew[i] * d;
        }
    }

    // Trivial upper bound U = (n-1) * sum(w_j)
    long long U = (long long)(n - 1) * sumW;

    double score_ratio;
    if (U == B) {
        // Special case: baseline already achieves upper bound
        score_ratio = 1.0;
    } else {
        score_ratio = (double)(S - B) / (double)(U - B);
        if (score_ratio < 0.0) score_ratio = 0.0;
        if (score_ratio > 1.0) score_ratio = 1.0;
    }

    quitp(score_ratio,
          "S=%lld B=%lld U=%lld Ratio: %.9f",
          S, B, U, score_ratio);

    return 0;
}