#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

// Recursively paint one stamp onto canvas (0-indexed, d>=2 from problem, but
// we recurse down to d=1 internally). side = 2^d.
void fillStamp(vector<vector<int>>& canvas, int r, int c, int d, const string& pat) {
    int half = 1 << (d - 1);
    int qr[4] = {r,        r,        r + half, r + half};
    int qc[4] = {c,        c + half, c,        c + half};
    for (int q = 0; q < 4; q++) {
        if (pat[q] == '1') {
            for (int i = qr[q]; i < qr[q] + half; i++)
                for (int j = qc[q]; j < qc[q] + half; j++)
                    canvas[i][j] = 1;
        } else {
            if (d > 1) {
                fillStamp(canvas, qr[q], qc[q], d - 1, pat);
            }
            // d==1, pat[q]=='0': the 1x1 cell stays white
        }
    }
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- Read problem input ----
    int n = inf.readInt();
    int m = inf.readInt();
    int B = inf.readInt();
    inf.readEoln();

    vector<string> target(n);
    for (int i = 0; i < n; i++) {
        target[i] = inf.readLine();
        if ((int)target[i].size() > m) target[i].resize(m);
    }

    long long black_count = 0;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            if (j < (int)target[i].size() && target[i][j] == '*')
                black_count++;

    long long P_base = 1LL + black_count;

    // ---- Read participant output ----
    int k = ouf.readInt(0, n * m, "k must be a non-negative integer");
    ouf.readEoln();

    // ownership grid for disjointness check
    vector<vector<int>> owner(n, vector<int>(m, -1));
    // canvas, initially all white
    vector<vector<int>> canvas(n, vector<int>(m, 0));

    long long total_cost = 0;

    for (int s = 0; s < k; s++) {
        int r = ouf.readInt(1, n, "r out of range");
        int c = ouf.readInt(1, m, "c out of range");
        int d = ouf.readInt(2, 30, "d must be >= 2");
        string pat = ouf.readToken();
        ouf.readEoln();

        if ((int)pat.size() != 4)
            quitf(_wa, "Stamp %d: pattern must be exactly 4 chars, got '%s'",
                  s + 1, pat.c_str());
        for (int q = 0; q < 4; q++)
            if (pat[q] != '0' && pat[q] != '1')
                quitf(_wa, "Stamp %d: pattern char %d must be '0' or '1', got '%c'",
                      s + 1, q, pat[q]);

        // Side length = 2^d; check it fits in the board
        // d can be at most about 9 for n,m<=500 (2^9=512)
        if (d > 20)
            quitf(_wa, "Stamp %d: d=%d is unreasonably large", s + 1, d);

        long long side = 1LL << d;

        if ((long long)(r - 1) + side > (long long)n)
            quitf(_wa, "Stamp %d: square extends below board (r=%d d=%d side=%lld n=%d)",
                  s + 1, r, d, side, n);
        if ((long long)(c - 1) + side > (long long)m)
            quitf(_wa, "Stamp %d: square extends right of board (c=%d d=%d side=%lld m=%d)",
                  s + 1, c, d, side, m);

        int r0 = r - 1, c0 = c - 1, sz = (int)side;

        // Disjointness check
        for (int i = r0; i < r0 + sz; i++) {
            for (int j = c0; j < c0 + sz; j++) {
                if (owner[i][j] != -1)
                    quitf(_wa, "Stamp %d overlaps stamp %d at cell (%d,%d)",
                          s + 1, owner[i][j] + 1, i + 1, j + 1);
                owner[i][j] = s;
            }
        }

        // Budget check (incremental)
        total_cost += d;
        if (total_cost > (long long)B)
            quitf(_wa, "Total cost %lld exceeds budget %d after stamp %d",
                  total_cost, B, s + 1);

        // Simulate
        fillStamp(canvas, r0, c0, d, pat);
    }

    // Must be end of participant output
    ouf.readEof();

    // ---- Compute Hamming distance ----
    long long H = 0;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++) {
            int tgt = (j < (int)target[i].size() && target[i][j] == '*') ? 1 : 0;
            if (canvas[i][j] != tgt) H++;
        }

    long long P = 1LL + H;

    // ---- Compute score as per statement ----
    // score = round(1,000,000 * (P_base - P) / (P_base - 1)), clamped to [0, 1,000,000]
    // Special case: P_base == 1 (target is completely white)
    //   score = 1,000,000 if P == 1, else 0

    long long int_score;
    if (P_base == 1LL) {
        int_score = (P == 1LL) ? 1000000LL : 0LL;
    } else {
        // P_base - 1 == black_count > 0
        double num = (double)(P_base - P);
        double den = (double)(P_base - 1LL);
        double raw = num / den;
        // clamp to [0, 1] before rounding
        if (raw < 0.0) raw = 0.0;
        if (raw > 1.0) raw = 1.0;
        int_score = (long long)round(1000000.0 * raw);
        if (int_score < 0LL) int_score = 0LL;
        if (int_score > 1000000LL) int_score = 1000000LL;
    }

    // Convert back to ratio in [0,1] for quitp
    double final_ratio = (double)int_score / 1000000.0;

    quitp(final_ratio,
          "Ratio: %.9f | int_score=%lld k=%d H=%lld P=%lld P_base=%lld cost=%lld B=%d",
          final_ratio, int_score, k, H, P, P_base, total_cost, B);

    return 0;
}