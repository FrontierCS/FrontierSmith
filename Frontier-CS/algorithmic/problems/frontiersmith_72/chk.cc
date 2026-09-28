#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- Read problem input from inf ----
    int N = inf.readInt();
    int M = inf.readInt();
    string S = inf.readToken();

    if ((int)S.size() != N)
        quitf(_fail, "Judge input error: S length %d != N=%d", (int)S.size(), N);

    vector<long long> C(N);
    for (int j = 0; j < N; j++)
        C[j] = inf.readLong();

    vector<long long> A(M);
    vector<string> P(M);
    for (int i = 0; i < M; i++) {
        int L = inf.readInt();
        A[i] = inf.readLong();
        P[i] = inf.readToken();
        if ((int)P[i].size() != L)
            quitf(_fail, "Judge input error: P[%d] length %d != L=%d", i, (int)P[i].size(), L);
    }

    // ---- Read participant output ----
    if (ouf.seekEof())
        quitf(_wa, "Participant output is empty");

    string U = ouf.readToken();

    // Enforce no extra tokens after the single output string
    if (!ouf.seekEof())
        quitf(_wa, "Extra data after the output string");

    // ---- Validate U ----
    if ((int)U.size() != N)
        quitf(_wa, "Output length %d, expected %d", (int)U.size(), N);

    for (int j = 0; j < N; j++) {
        if (U[j] != '0' && U[j] != '1')
            quitf(_wa, "Invalid character '%c' at position %d (1-indexed)", U[j], j + 1);
    }

    // ---- Compute FlipCost(U) ----
    long long flipCost = 0;
    for (int j = 0; j < N; j++) {
        if (U[j] != S[j])
            flipCost += C[j];
    }

    // ---- Compute Reward(U) ----
    long long rewardU = 0;
    for (int i = 0; i < M; i++) {
        if (U.find(P[i]) != string::npos)
            rewardU += A[i];
    }

    long long objU = rewardU - flipCost;

    // ---- Compute baseline B = Obj(S) (no flips, so FlipCost=0) ----
    long long rewardS = 0;
    for (int i = 0; i < M; i++) {
        if (S.find(P[i]) != string::npos)
            rewardS += A[i];
    }
    long long B = rewardS; // FlipCost for S is 0

    // ---- Compute UPPER = sum of all A_i ----
    long long UPPER = 0;
    for (int i = 0; i < M; i++)
        UPPER += A[i];

    // ---- Compute score ----
    // Score = floor(10^6 * max(0, Obj(U)-B) / max(1, UPPER-B)), clamped to [0, 10^6]
    // The judge reads "Ratio: <value>" from the message; value must be in [0,1].
    // ratio = max(0, Obj(U)-B) / max(1, UPPER-B)  -- this is exactly Score/10^6.
    long long numerator   = max(0LL, objU - B);
    long long denominator = max(1LL, UPPER - B);

    double ratio = (double)numerator / (double)denominator;
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    // scoreInt matches floor(10^6 * ratio) from the statement
    long long scoreInt = (long long)(1000000.0 * ratio);
    if (scoreInt > 1000000LL) scoreInt = 1000000LL;
    if (scoreInt < 0LL) scoreInt = 0LL;

    // quitp first argument is the ratio in [0,1]; the judge also reads "Ratio: <value>" tag.
    quitp(ratio,
          "Obj(U)=%lld B=%lld UPPER=%lld FlipCost=%lld Reward=%lld | "
          "Ratio: %.9f | Score: %lld / 1000000",
          objU, B, UPPER, flipCost, rewardU,
          ratio, scoreInt);

    return 0;
}