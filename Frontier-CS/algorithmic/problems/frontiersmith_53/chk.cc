#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- read problem input from inf ----
    int N = inf.readInt();
    int T = inf.readInt();
    int R = inf.readInt();
    long long F = (long long)inf.readInt();

    vector<int> Us(R), Vs(R), Ws(R), Cs(R);
    for (int i = 0; i < R; i++) {
        Us[i] = inf.readInt();
        Vs[i] = inf.readInt();
        Ws[i] = inf.readInt();
        Cs[i] = inf.readInt();
    }

    // ---- trivial upper bound U = sum(w*c) ----
    long long U_val = 0;
    for (int i = 0; i < R; i++)
        U_val += (long long)Ws[i] * Cs[i];

    // ---- baseline B: identity permutation ----
    // Identity keeps the row [1..N] every morning; fatigue = 0.
    // For pair (u,v), adj = T if |u-v|==1, else 0.
    long long B = 0;
    for (int i = 0; i < R; i++) {
        if (abs(Us[i] - Vs[i]) == 1)
            B += (long long)Ws[i] * min(T, Cs[i]);
    }

    // ---- read participant permutation from ouf ----
    vector<int> A(N + 1);
    for (int i = 1; i <= N; i++) {
        A[i] = ouf.readInt();
        if (A[i] < 1 || A[i] > N) {
            quitf(_wa, "A[%d] = %d is out of range [1, %d]", i, A[i], N);
        }
    }

    // Ensure no extra tokens after the N values
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra tokens found after the permutation");
    }

    // Validate it is a permutation (no duplicates)
    vector<bool> seen(N + 1, false);
    for (int i = 1; i <= N; i++) {
        if (seen[A[i]])
            quitf(_wa, "Duplicate value %d in permutation", A[i]);
        seen[A[i]] = true;
    }

    // ---- simulate T mornings ----
    // S[i] = cow currently at position i (1-indexed)
    vector<int> S(N + 1), S_new(N + 1);
    for (int i = 1; i <= N; i++) S[i] = i;

    // pos[cow] = current position of that cow
    vector<int> pos(N + 1);
    for (int i = 1; i <= N; i++) pos[i] = i;

    vector<int> adj_count(R, 0);

    for (int t = 0; t < T; t++) {
        // apply permutation: S'[A[i]] = S[i]
        for (int i = 1; i <= N; i++)
            S_new[A[i]] = S[i];
        swap(S, S_new);

        // rebuild pos
        for (int i = 1; i <= N; i++)
            pos[S[i]] = i;

        // count adjacencies for each rewarded pair
        for (int r = 0; r < R; r++) {
            int pu = pos[Us[r]];
            int pv = pos[Vs[r]];
            if (abs(pu - pv) == 1)
                adj_count[r]++;
        }
    }

    // ---- compute reward ----
    long long reward = 0;
    for (int r = 0; r < R; r++)
        reward += (long long)Ws[r] * min(adj_count[r], Cs[r]);

    // ---- compute fatigue ----
    long long fat_sum = 0;
    for (int i = 1; i <= N; i++)
        fat_sum += (long long)abs(A[i] - i);
    long long fatigue = (long long)T * F * fat_sum;

    long long V = reward - fatigue;

    // ---- scoring ----
    // Per-test score formula (statement):
    //   if U == B:
    //     score = 1,000,000 if V == B, else 0
    //   else:
    //     score = 1,000,000 * clamp((V - B) / (U - B), 0, 1)
    //
    // testlib's quitp(p, ...) accepts p in [0,1] and the judge reads
    // the "Ratio: <p>" tag from the message string.
    // We pass ratio = clamp((V-B)/(U-B), 0, 1) directly to quitp,
    // which is equivalent to the statement's formula divided by 1,000,000.
    // The "Ratio:" tag in the message MUST equal the first argument of quitp.

    if (U_val == B) {
        if (V == B) {
            double ratio = 1.0;
            quitp(ratio, "Ratio: 1.000000 | V=%lld B=%lld U=%lld (trivial case: U==B, V==B)", V, B, U_val);
        } else {
            double ratio = 0.0;
            quitp(ratio, "Ratio: 0.000000 | V=%lld B=%lld U=%lld (trivial case: U==B, V!=B)", V, B, U_val);
        }
    } else {
        double num = (double)(V - B);
        double den = (double)(U_val - B);
        double ratio = num / den;
        if (ratio < 0.0) ratio = 0.0;
        if (ratio > 1.0) ratio = 1.0;
        quitp(ratio, "Ratio: %f | V=%lld B=%lld U=%lld fatigue=%lld reward=%lld",
              ratio, V, B, U_val, fatigue, reward);
    }

    return 0;
}