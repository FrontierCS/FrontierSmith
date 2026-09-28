#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ── Read input ──────────────────────────────────────────────────────────
    int n = inf.readInt();
    int m = inf.readInt();

    vector<vector<int>> a(n, vector<int>(m));
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            a[i][j] = inf.readInt();

    long long nm = (long long)n * m;

    // ── Read participant output ─────────────────────────────────────────────
    int k = ouf.readInt(0, (int)nm, "k");

    vector<vector<bool>> covered(n, vector<bool>(m, false));
    long long A = 0;

    for (int t = 0; t < k; t++) {
        int i1 = ouf.readInt(1, n, "i1");
        int j1 = ouf.readInt(1, m, "j1");
        int i2 = ouf.readInt(1, n, "i2");
        int j2 = ouf.readInt(1, m, "j2");

        if (i1 > i2)
            quitf(_wa, "Gallery %d: i1=%d > i2=%d", t + 1, i1, i2);
        if (j1 > j2)
            quitf(_wa, "Gallery %d: j1=%d > j2=%d", t + 1, j1, j2);

        // Check for overlap
        for (int i = i1 - 1; i < i2; i++) {
            for (int j = j1 - 1; j < j2; j++) {
                if (covered[i][j])
                    quitf(_wa,
                          "Gallery %d overlaps with a previously declared gallery at cell (%d,%d)",
                          t + 1, i + 1, j + 1);
            }
        }

        // Check label uniqueness inside the rectangle
        set<int> labels;
        for (int i = i1 - 1; i < i2; i++) {
            for (int j = j1 - 1; j < j2; j++) {
                if (!labels.insert(a[i][j]).second)
                    quitf(_wa,
                          "Gallery %d has duplicate label %d (not a clean gallery)",
                          t + 1, a[i][j]);
            }
        }

        // Mark covered
        for (int i = i1 - 1; i < i2; i++)
            for (int j = j1 - 1; j < j2; j++)
                covered[i][j] = true;

        long long area = (long long)(i2 - i1 + 1) * (long long)(j2 - j1 + 1);
        A += area * area;
    }

    // ── Trailing-data check ─────────────────────────────────────────────────
    // After all k rectangles have been read, no extra tokens are allowed.
    if (!ouf.seekEof())
        quitf(_wa, "Extra output after the last gallery");

    // ── Scoring ─────────────────────────────────────────────────────────────
    // B  = n*m   (baseline: every cell is its own 1×1 gallery, score=0)
    // U  = (n*m)^2  (upper bound, score=100)
    // score = clamp( (ln(A+1) - ln(B+1)) / (ln(U+1) - ln(B+1)), 0, 1 )

    long long B = nm;
    long long U = nm * nm;   // fits in long long for n,m ≤ 400 (max ~25.6e9)

    double score;
    if (nm == 1LL) {
        // Special case: only one cell, optimum is exactly 1
        score = (A == 1LL) ? 1.0 : 0.0;
    } else {
        double dA = (double)A;
        double dB = (double)B;
        double dU = (double)U;

        double num = log(dA + 1.0) - log(dB + 1.0);
        double den = log(dU + 1.0) - log(dB + 1.0);

        score = num / den;
        if (score < 0.0) score = 0.0;
        if (score > 1.0) score = 1.0;
    }

    quitp(score,
          "A=%lld B=%lld U=%lld Ratio: %.9f",
          A, B, U, score);

    return 0;
}