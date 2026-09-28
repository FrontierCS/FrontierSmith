#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- Read problem input ----
    int K = inf.readInt();
    string s = inf.readWord();
    int n = (int)s.size();

    vector<int> d(n + 1);
    for (int i = 1; i <= n; i++) d[i] = s[i - 1] - '0';

    // E0 = (sum d_i)^2
    long long sum_d = 0;
    for (int i = 1; i <= n; i++) sum_d += d[i];
    long long E0 = sum_d * sum_d;

    // ---- Read participant output ----
    // m: number of rectangles
    if (ouf.seekEof()) {
        // Empty output — treat as m=0
        double score_ratio = (E0 == 0LL) ? 1.0 : 0.0;
        quitp(score_ratio, "E=%lld E0=%lld Ratio: %.9f", E0, E0, score_ratio);
    }

    int m = ouf.readInt(0, K, "m");

    // 2-D difference array for C (1-indexed, size (n+2)x(n+2))
    // |w| <= 1e6, K <= 60 => |C[i][j]| <= 6e7, fits in long long easily
    int sz = n + 2;
    // Use long long to avoid overflow when accumulating weights
    vector<long long> diff((long long)sz * sz, 0LL);

    auto idx = [&](int r, int c) -> long long {
        return (long long)r * sz + c;
    };

    for (int r = 0; r < m; r++) {
        int x = ouf.readInt(1, n, "x");
        int y = ouf.readInt(x, n, "y");
        int z = ouf.readInt(1, n, "z");
        int t = ouf.readInt(z, n, "t");
        int w = ouf.readInt(-1000000, 1000000, "w");

        diff[idx(x,     z    )] += w;
        diff[idx(x,     t + 1)] -= w;
        diff[idx(y + 1, z    )] -= w;
        diff[idx(y + 1, t + 1)] += w;
    }

    // Do NOT call ouf.readEof() — trailing whitespace/newlines are acceptable
    // in partial-scoring problems and would cause false WA for valid solutions.

    // ---- Prefix-sum the difference array to recover C ----
    // Row-wise prefix sum (for each row i, sum over j)
    for (int i = 1; i <= n; i++)
        for (int j = 1; j <= n; j++)
            diff[idx(i, j)] += diff[idx(i, j - 1)];
    // Column-wise prefix sum (for each col j, sum over i)
    for (int j = 1; j <= n; j++)
        for (int i = 1; i <= n; i++)
            diff[idx(i, j)] += diff[idx(i - 1, j)];

    // ---- Compute E = sum |B[i][j] - C[i][j]| ----
    long long E = 0;
    for (int i = 1; i <= n; i++) {
        for (int j = 1; j <= n; j++) {
            long long bij = (long long)d[i] * d[j];
            long long cij = diff[idx(i, j)];
            E += (bij >= cij) ? (bij - cij) : (cij - bij);
        }
    }

    // ---- Compute score ratio per problem statement ----
    // If E0 > 0: ratio = max(0, 1 - E/E0)  (continuous partial credit)
    // If E0 = 0: ratio = 1 if E==0, else 0
    double score_ratio;
    if (E0 == 0LL) {
        score_ratio = (E == 0LL) ? 1.0 : 0.0;
    } else {
        score_ratio = max(0.0, 1.0 - (double)E / (double)E0);
    }

    // Required tag: "Ratio: <score_ratio>" must appear in the message.
    quitp(score_ratio,
          "E=%lld E0=%lld Ratio: %.9f",
          E, E0, score_ratio);

    return 0;
}