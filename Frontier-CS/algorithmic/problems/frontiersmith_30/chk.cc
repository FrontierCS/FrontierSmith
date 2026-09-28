#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- read input ----
    int N, K;
    N = inf.readInt();
    K = inf.readInt();

    vector<long long> A(N + 1);
    for (int i = 1; i <= N; i++) A[i] = inf.readLong();

    int Q = inf.readInt();
    vector<int> ql(Q), qr(Q), qw(Q);
    for (int i = 0; i < Q; i++) {
        ql[i] = inf.readInt();
        qr[i] = inf.readInt();
        qw[i] = inf.readInt();
    }

    // W[i][j] = combined weight for pair (i,j): sum of w for queries with l<=i and r>=j
    vector<vector<long long>> W(N + 2, vector<long long>(N + 2, 0LL));
    for (int i = 0; i < Q; i++) {
        W[ql[i]][qr[i]] += (long long)qw[i];
    }
    // prefix sum over l dimension
    for (int r = 1; r <= N; r++)
        for (int l = 2; l <= N; l++)
            W[l][r] += W[l - 1][r];
    // suffix sum over r dimension
    for (int l = 1; l <= N; l++)
        for (int r = N - 1; r >= 1; r--)
            W[l][r] += W[l][r + 1];

    // Compute total weighted bubble energy for a given permutation P (1-indexed).
    // Uses __int128 to avoid overflow.
    auto computeCost = [&](const vector<int>& P) -> __int128 {
        __int128 cost = 0;
        for (int i = 1; i <= N; i++) {
            for (int j = i + 1; j <= N; j++) {
                long long bi = A[P[i]], bj = A[P[j]];
                if (bi > bj) {
                    cost += (__int128)bi * bj * W[i][j];
                }
            }
        }
        return cost;
    };

    // Baseline: identity permutation P[i] = i
    vector<int> identity(N + 1);
    for (int i = 1; i <= N; i++) identity[i] = i;
    __int128 C_base = computeCost(identity);

    // ---- read participant output ----
    vector<int> P(N + 1);
    vector<bool> seen(N + 1, false);
    for (int i = 1; i <= N; i++) {
        P[i] = ouf.readInt(1, N, "P[i] must be in [1,N]");
        if (seen[P[i]]) {
            quitf(_wa, "Permutation has duplicate value %d at position %d", P[i], i);
        }
        seen[P[i]] = true;
    }

    // ---- strict EOF check: reject any trailing non-whitespace tokens ----
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra output found after the permutation");
    }

    // ---- verify inversion count <= K ----
    long long inv = 0;
    for (int i = 1; i <= N; i++)
        for (int j = i + 1; j <= N; j++)
            if (P[i] > P[j]) inv++;

    if (inv > (long long)K) {
        quitf(_wa, "Too many inversions: %lld > K=%d", inv, K);
    }

    // ---- compute participant's cost ----
    __int128 C_out = computeCost(P);

    // ---- scoring ----
    // Problem formula: Score = floor(10^6 * min(10, (C_base+1)/(C_out+1)))
    // Maximum possible score is 10^7 (when C_out=0), baseline gives 10^6.
    // Map to quitp ratio in [0, 1] by dividing by 10:
    //   ratio = min(10, (C_base+1)/(C_out+1)) / 10
    // Baseline gives ratio = 1.0/10 = 0.1 (10% of max points = 10^6 of 10^7).
    // Perfect solution gives ratio = 1.0.
    // Worse-than-baseline gives ratio < 0.1.
    double c_base_d = (double)C_base;
    double c_out_d  = (double)C_out;
    double raw_ratio;
    if (c_out_d + 1.0 <= 0.0) {
        raw_ratio = 10.0;
    } else {
        raw_ratio = (c_base_d + 1.0) / (c_out_d + 1.0);
    }
    if (raw_ratio > 10.0) raw_ratio = 10.0;
    if (raw_ratio < 0.0)  raw_ratio = 0.0;
    // Normalize to [0, 1] for quitp
    double ratio = raw_ratio / 10.0;
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    // Safe conversion of __int128 to long long for display
    const long long LLMAX = (long long)9000000000000000000LL;
    long long c_base_ll = (C_base > (__int128)LLMAX) ? LLMAX : (long long)C_base;
    long long c_out_ll  = (C_out  > (__int128)LLMAX) ? LLMAX : (long long)C_out;

    // The "Ratio: <value>" tag is parsed by the judge.
    quitp(ratio, "C_out=%lld C_base=%lld inv=%lld K=%d Ratio: %.9f",
          c_out_ll, c_base_ll, inv, K, ratio);

    return 0;
}