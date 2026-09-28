#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ── Read problem input ────────────────────────────────────────────────────
    int R = inf.readInt(1, 90, "R");
    int C = inf.readInt(1, 90, "C");

    vector<vector<long long>> A(R, vector<long long>(C));
    for (int i = 0; i < R; i++)
        for (int j = 0; j < C; j++)
            A[i][j] = inf.readLong(1LL, 1000000000LL, "A[i][j]");

    // ── Read participant row permutation ──────────────────────────────────────
    vector<int> p(R), q(C);
    for (int i = 0; i < R; i++) {
        p[i] = ouf.readInt(1, R, "p[i]") - 1;
    }
    {
        vector<bool> seen(R, false);
        for (int i = 0; i < R; i++) {
            if (seen[p[i]])
                quitf(_wa, "Row permutation has duplicate value %d", p[i] + 1);
            seen[p[i]] = true;
        }
    }

    // ── Read participant column permutation ───────────────────────────────────
    for (int j = 0; j < C; j++) {
        q[j] = ouf.readInt(1, C, "q[j]") - 1;
    }
    {
        vector<bool> seen(C, false);
        for (int j = 0; j < C; j++) {
            if (seen[q[j]])
                quitf(_wa, "Column permutation has duplicate value %d", q[j] + 1);
            seen[q[j]] = true;
        }
    }

    // ── Build reordered matrix A' ─────────────────────────────────────────────
    vector<vector<long long>> Ap(R, vector<long long>(C));
    for (int i = 0; i < R; i++)
        for (int j = 0; j < C; j++)
            Ap[i][j] = A[p[i]][q[j]];

    // ── Read participant matrix B ─────────────────────────────────────────────
    const long long BLIMIT = 1000000000000LL;
    vector<vector<long long>> B(R, vector<long long>(C));
    for (int i = 0; i < R; i++)
        for (int j = 0; j < C; j++)
            B[i][j] = ouf.readLong(-BLIMIT, BLIMIT, "B[i][j]");

    // ── Reject trailing garbage ───────────────────────────────────────────────
    if (!ouf.seekEof())
        quitf(_wa, "Extra output found after the B matrix");

    // ── Check strict row-monotonicity ─────────────────────────────────────────
    for (int i = 0; i < R; i++) {
        for (int j = 0; j + 1 < C; j++) {
            if (B[i][j] >= B[i][j + 1])
                quitf(_wa,
                      "B[%d][%d]=%lld >= B[%d][%d]=%lld (not strictly increasing in row)",
                      i + 1, j + 1, (long long)B[i][j],
                      i + 1, j + 2, (long long)B[i][j + 1]);
        }
    }

    // ── Check strict column-monotonicity ─────────────────────────────────────
    for (int j = 0; j < C; j++) {
        for (int i = 0; i + 1 < R; i++) {
            if (B[i][j] >= B[i + 1][j])
                quitf(_wa,
                      "B[%d][%d]=%lld >= B[%d][%d]=%lld (not strictly increasing in column)",
                      i + 1, j + 1, (long long)B[i][j],
                      i + 2, j + 1, (long long)B[i + 1][j]);
        }
    }

    // ── Compute participant total cost ────────────────────────────────────────
    long long part_cost = 0;
    for (int i = 0; i < R; i++)
        for (int j = 0; j < C; j++)
            part_cost += llabs(Ap[i][j] - B[i][j]);

    // ── Compute baseline cost (original order, greedy scan) ───────────────────
    // G[i][j] = max(A[i][j], G[i-1][j]+1, G[i][j-1]+1)
    vector<vector<long long>> G(R, vector<long long>(C));
    for (int i = 0; i < R; i++) {
        for (int j = 0; j < C; j++) {
            G[i][j] = A[i][j];
            if (i > 0) G[i][j] = max(G[i][j], G[i - 1][j] + 1LL);
            if (j > 0) G[i][j] = max(G[i][j], G[i][j - 1] + 1LL);
        }
    }

    long long base_cost = 0;
    for (int i = 0; i < R; i++)
        for (int j = 0; j < C; j++)
            base_cost += llabs(A[i][j] - G[i][j]);

    // ── Score formula from problem statement ──────────────────────────────────
    // score = 10^6 * clamp( (C_base+1)/(C_part+1), 0, 5 )
    //
    // The platform multiplies quitp's value by 10^6 to get the displayed score.
    // Hence quitp must receive a value in [0, 1] representing score / 10^6.
    //
    // Statement: max achievable score = 5 * 10^6  (clamp upper bound = 5).
    // So:  quitp_value = clamp((base_cost+1)/(part_cost+1), 0, 5) / 5
    //
    //   Matching baseline  => raw_ratio = 1.0 => clamped = 1.0 => quitp = 0.2
    //     => displayed score = 0.2 * 5 * 10^6 / 1 ... 
    //
    // Actually the platform uses quitp value in [0,1] and maps it to [0, 10^6].
    // To make "matching baseline => 10^6" we must pass 1.0 at baseline.
    // We therefore clamp raw_ratio into [0, 5] and then divide by 5 to get [0,1].
    // But that gives 0.2 at baseline (raw_ratio=1).
    //
    // The correct mapping consistent with the statement
    // "matching baseline = 1,000,000" requires quitp=1.0 at baseline.
    // We use: score_ratio = min(1.0, raw_ratio)
    // This means:
    //   part_cost == base_cost => ratio=1.0 => quitp=1.0 => displayed 1,000,000  ✓
    //   part_cost > base_cost  => ratio<1   => quitp<1   => displayed <1,000,000 ✓
    //   part_cost < base_cost  => ratio>1   => quitp=1.0 => displayed 1,000,000  (capped)
    //
    // To preserve the full 5x-bonus range specified in the statement we divide
    // by 5 and rely on the platform's 5×10^6 max-score configuration:
    //
    //   score_ratio = clamp((base+1)/(part+1), 0, 5) / 5   in [0, 1]
    //   platform_score = score_ratio * 5 * 10^6
    //   => baseline => 0.2 * 5e6 = 1e6  ✓   5x better => 1.0 * 5e6 = 5e6  ✓
    //
    // We use this formula and report the Ratio tag as the raw [0,5]-clamped value
    // divided by 5, which is what quitp receives.

    double raw_ratio = (double)(base_cost + 1) / (double)(part_cost + 1);
    double clamped   = min(5.0, max(0.0, raw_ratio));   // in [0, 5]
    double score_ratio = clamped / 5.0;                  // in [0, 1] for quitp

    // The "Ratio:" tag must equal the quitp value (required by judge regex).
    quitp(score_ratio,
          "Ratio: %.9f | base_cost=%lld part_cost=%lld",
          score_ratio,
          (long long)base_cost,
          (long long)part_cost);

    return 0;
}