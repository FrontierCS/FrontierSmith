#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // --- Read problem input from inf ---
    int n = inf.readInt();
    int q = inf.readInt();

    vector<long long> a(n + 1), c(n + 1);
    for (int i = 1; i <= n; i++) a[i] = inf.readLong();
    for (int i = 1; i <= n; i++) c[i] = inf.readLong();

    vector<int> ql(q), qr(q);
    vector<long long> qw(q);
    for (int j = 0; j < q; j++) {
        ql[j] = inf.readInt();
        qr[j] = inf.readInt();
        qw[j] = inf.readLong();
    }

    // --- Read participant output from ouf ---
    vector<long long> x(n + 1);
    for (int i = 1; i <= n; i++) {
        x[i] = ouf.readLong();
        if (x[i] < 1LL || x[i] > 1000000000LL) {
            quitf(_wa, "Position %d: value %lld is out of range [1, 10^9]", i, x[i]);
        }
    }
    // Ensure no trailing tokens
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra tokens found after the %d required integers", n);
    }

    // --- Compute baseline B = F(a), i.e. the original tape unchanged ---
    // Rewrite cost for baseline is 0 (x == a everywhere).
    // Only palindromicity contributes.
    long long B = 0;
    for (int j = 0; j < q; j++) {
        long long pairs = (long long)(qr[j] - ql[j]) / 2;
        for (long long t = 0; t < pairs; t++) {
            if (a[ql[j] + t] != a[qr[j] - t]) {
                B += qw[j];
            }
        }
    }

    // --- Compute participant objective Y = F(x) ---
    long long Y = 0;

    // Rewrite cost
    for (int i = 1; i <= n; i++) {
        if (x[i] != a[i]) {
            Y += c[i];
        }
    }

    // Palindromicity penalties
    for (int j = 0; j < q; j++) {
        long long pairs = (long long)(qr[j] - ql[j]) / 2;
        for (long long t = 0; t < pairs; t++) {
            if (x[ql[j] + t] != x[qr[j] - t]) {
                Y += qw[j];
            }
        }
    }

    // --- Compute score per statement formula ---
    // score = floor(10^6 * max(0, min(1, (B-Y)/B)) + 0.5)
    // Special case: B == 0
    long long intScore;
    if (B == 0) {
        if (Y == 0) {
            // Both zero: perfect score
            intScore = 1000000LL;
        } else {
            // Baseline already optimal, participant made it worse
            intScore = 0LL;
        }
    } else {
        // General case
        double ratio = (double)(B - Y) / (double)B;
        if (ratio < 0.0) ratio = 0.0;
        if (ratio > 1.0) ratio = 1.0;
        // floor(...+ 0.5) is standard rounding to nearest integer
        intScore = (long long)(1e6 * ratio + 0.5);
        if (intScore < 0LL) intScore = 0LL;
        if (intScore > 1000000LL) intScore = 1000000LL;
    }

    // Convert integer score back to ratio in [0,1] for quitp
    double scoreRatio = (double)intScore / 1000000.0;

    quitp(scoreRatio,
          "Ratio: %.6f | B=%lld, Y=%lld, intScore=%lld",
          scoreRatio, B, Y, intScore);

    return 0;
}