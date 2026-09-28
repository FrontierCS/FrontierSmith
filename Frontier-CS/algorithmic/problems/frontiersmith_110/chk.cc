#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

// BIT-based inversion count: count pairs (i<j) with arr[i] > arr[j]
// arr values must be in [1..n]
long long countInversions(const vector<int>& arr) {
    int n = (int)arr.size();
    vector<int> bit(n + 2, 0);
    long long inv = 0;
    for (int i = n - 1; i >= 0; i--) {
        int x = arr[i] - 1; // 0-indexed
        for (int j = x; j > 0; j -= j & (-j))
            inv += bit[j];
        for (int j = arr[i]; j <= n; j += j & (-j))
            bit[j]++;
    }
    return inv;
}

long long countIncreasingPairs(const vector<int>& arr) {
    int n = (int)arr.size();
    long long total = (long long)n * (n - 1) / 2;
    return total - countInversions(arr);
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // Read problem input
    int N = inf.readInt();
    int K = inf.readInt();

    vector<int> A(N);
    for (int i = 0; i < N; i++)
        A[i] = inf.readInt();

    // Read participant output: N lane assignments in [1..K]
    vector<int> C(N);
    for (int i = 0; i < N; i++) {
        C[i] = ouf.readInt(1, K,
            ("C[" + to_string(i + 1) + "]").c_str());
    }

    // Reject any extra tokens after the expected N values
    if (!ouf.seekEof())
        quitf(_wa, "Extra output found after the %d lane assignments", N);

    // Validate: every lane 1..K must be used at least once
    vector<bool> used(K + 1, false);
    for (int i = 0; i < N; i++)
        used[C[i]] = true;
    for (int k = 1; k <= K; k++) {
        if (!used[k])
            quitf(_wa, "Lane %d received no packages (every lane must get at least one)", k);
    }

    // Build B: drain lanes 1..K in order, preserving arrival order within each lane
    vector<vector<int>> lanes(K + 1);
    for (int i = 0; i < N; i++)
        lanes[C[i]].push_back(A[i]);

    vector<int> B;
    B.reserve(N);
    for (int k = 1; k <= K; k++)
        for (int v : lanes[k])
            B.push_back(v);

    // Compute G(B), G0 = G(A), U
    long long GB = countIncreasingPairs(B);
    long long G0 = countIncreasingPairs(A);
    long long U  = (long long)N * (N - 1) / 2;

    // U - G0 > 0 is guaranteed by the problem (A is not already sorted)
    long long denom = U - G0;
    if (denom <= 0)
        quitf(_fail, "Internal error: U-G0 <= 0 (U=%lld, G0=%lld)", U, G0);

    // Compute integer score = floor(10^6 * max(0, G(B)-G0) / (U-G0))
    // matching exactly the statement formula
    long long score_int = 0;
    if (GB > G0) {
        score_int = (long long)((long double)(GB - G0) * 1000000LL / (long double)denom);
        if (score_int < 0)      score_int = 0;
        if (score_int > 1000000) score_int = 1000000;
    }

    // Pass score_int/1000000.0 as the ratio so the judge awards the correct
    // integer-scaled score (floor(10^6 * ratio) from the statement).
    double score_ratio = (double)score_int / 1000000.0;

    quitp(score_ratio,
        "G(B)=%lld, G0=%lld, U=%lld, score=%lld/1000000. Ratio: %.9f",
        GB, G0, U, score_int, score_ratio);

    return 0;
}