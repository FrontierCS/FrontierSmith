#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    int T = inf.readInt();

    long long total_score_sum = 0;
    long long total_max_sum   = 0;

    for (int tc = 0; tc < T; tc++) {
        // --- read problem parameters from input file ---
        int H = inf.readInt();
        int W = inf.readInt();
        int K = inf.readInt();
        int L = inf.readInt();
        long long C0 = inf.readLong();
        long long C1 = inf.readLong();
        long long C2 = inf.readLong();

        vector<vector<int>> a(H, vector<int>(W));
        for (int r = 0; r < H; r++)
            for (int c = 0; c < W; c++)
                a[r][c] = inf.readInt();

        vector<vector<long long>> w(H, vector<long long>(W));
        for (int r = 0; r < H; r++)
            for (int c = 0; c < W; c++)
                w[r][c] = inf.readLong();

        // --- compute baseline B (0 drops) ---
        long long B = 0;
        for (int r = 0; r < H; r++)
            for (int c = 0; c < W; c++)
                B += w[r][c] * (long long)(a[r][c] % K);

        // --- read participant's number of drops ---
        int q = ouf.readInt(0, L,
            ("Test case " + to_string(tc+1) + ": q out of range [0," + to_string(L) + "]").c_str());

        // simulate on a copy of a
        vector<vector<int>> cur(a);
        long long drop_cost = 0;

        for (int i = 0; i < q; i++) {
            int r1 = ouf.readInt();
            int c1 = ouf.readInt();
            int r2 = ouf.readInt();
            int c2 = ouf.readInt();
            int d  = ouf.readInt();

            if (r1 < 1 || r1 > H)
                quitf(_wa, "Test case %d, drop %d: r1=%d out of [1,%d]", tc+1, i+1, r1, H);
            if (r2 < r1 || r2 > H)
                quitf(_wa, "Test case %d, drop %d: r2=%d out of [%d,%d]", tc+1, i+1, r2, r1, H);
            if (c1 < 1 || c1 > W)
                quitf(_wa, "Test case %d, drop %d: c1=%d out of [1,%d]", tc+1, i+1, c1, W);
            if (c2 < c1 || c2 > W)
                quitf(_wa, "Test case %d, drop %d: c2=%d out of [%d,%d]", tc+1, i+1, c2, c1, W);
            if (d < 1 || d >= K)
                quitf(_wa, "Test case %d, drop %d: d=%d out of [1,%d]", tc+1, i+1, d, K-1);

            long long area = (long long)(r2 - r1 + 1) * (c2 - c1 + 1);
            drop_cost += C0 + C1 * (long long)d + C2 * area;

            // apply drop: new_health = ((x - d - 1) mod K) + 1
            for (int r = r1-1; r < r2; r++)
                for (int c = c1-1; c < c2; c++)
                    cur[r][c] = ((cur[r][c] - d - 1) % K + K) % K + 1;
        }

        // --- compute weighted residual sum ---
        long long wres = 0;
        for (int r = 0; r < H; r++)
            for (int c = 0; c < W; c++)
                wres += w[r][c] * (long long)(cur[r][c] % K);

        long long O = drop_cost + wres;

        // --- per-test score (out of 2,000,000) ---
        // score = round(1e6 * clamp(0, 2 - O/B, 2))
        long long per_score_num;
        if (B == 0) {
            per_score_num = (O == 0) ? 2000000LL : 0LL;
        } else {
            double ratio = 2.0 - (double)O / (double)B;
            if (ratio < 0.0) ratio = 0.0;
            if (ratio > 2.0) ratio = 2.0;
            per_score_num = llround(1000000.0 * ratio);
            if (per_score_num < 0)        per_score_num = 0;
            if (per_score_num > 2000000)  per_score_num = 2000000;
        }

        total_score_sum += per_score_num;
        total_max_sum   += 2000000LL;
    }

    // NOTE: do NOT call ouf.readEof() — contestant output typically ends with
    // a trailing newline which testlib's readEof() treats as non-EOF and fails.

    double final_ratio = (total_max_sum > 0)
        ? (double)total_score_sum / (double)total_max_sum
        : 1.0;
    if (final_ratio < 0.0) final_ratio = 0.0;
    if (final_ratio > 1.0) final_ratio = 1.0;

    quitp(final_ratio,
          "Ratio: %f  (total_score=%lld / max=%lld)",
          final_ratio,
          total_score_sum,
          total_max_sum);
}