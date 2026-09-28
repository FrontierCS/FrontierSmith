#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

// Compute transition cost between two padded strings of length L
// using the cost array (0-indexed positions 0..L-1)
static long long transitionCost(const string &A, const string &B,
                                const vector<long long> &costArr) {
    long long c = 0;
    int Llen = (int)A.size();
    for (int j = 0; j < Llen; j++) {
        if (A[j] != B[j]) c += costArr[j];
    }
    return c;
}

// Pad a binary string s to length L with leading zeros
static string padLeft(const string &s, int L) {
    if ((int)s.size() >= L) return s;
    return string(L - (int)s.size(), '0') + s;
}

// Compare two binary strings by numeric value:
// shorter string is numerically smaller; equal length: lex order
static bool numericLess(const string &a, const string &b) {
    if (a.size() != b.size()) return a.size() < b.size();
    return a < b;
}

int main(int argc, char *argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- Read problem input ----
    int N = inf.readInt();
    int L = inf.readInt();

    vector<long long> costArr(L);
    for (int j = 0; j < L; j++) {
        costArr[j] = inf.readLong();
    }

    vector<string> ids(N + 1); // 1-indexed
    for (int i = 1; i <= N; i++) {
        ids[i] = inf.readToken();
    }

    // Precompute padded versions (1-indexed)
    vector<string> padded(N + 1);
    for (int i = 1; i <= N; i++) {
        padded[i] = padLeft(ids[i], L);
    }

    // ---- Read participant output ----
    vector<int> perm(N);
    for (int i = 0; i < N; i++) {
        perm[i] = ouf.readInt(1, N, "permutation element");
    }

    // Assert end of stream
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra data after the permutation");
    }

    // Validate permutation: check for duplicates
    vector<bool> seen(N + 1, false);
    for (int i = 0; i < N; i++) {
        int v = perm[i];
        if (seen[v]) {
            quitf(_wa, "Duplicate index %d in permutation", v);
        }
        seen[v] = true;
    }

    // ---- Compute participant's total energy E ----
    string zeros(L, '0');
    long long E = 0;

    E += transitionCost(zeros, padded[perm[0]], costArr);
    for (int i = 0; i + 1 < N; i++) {
        E += transitionCost(padded[perm[i]], padded[perm[i + 1]], costArr);
    }
    E += transitionCost(padded[perm[N - 1]], zeros, costArr);

    // ---- Compute baseline energy B ----
    // Baseline: sort IDs by numeric value (shorter length = smaller number; ties by lex)
    vector<int> baseOrder(N);
    iota(baseOrder.begin(), baseOrder.end(), 1); // 1..N
    sort(baseOrder.begin(), baseOrder.end(), [&](int a, int b) {
        return numericLess(ids[a], ids[b]);
    });

    long long B = 0;
    B += transitionCost(zeros, padded[baseOrder[0]], costArr);
    for (int i = 0; i + 1 < N; i++) {
        B += transitionCost(padded[baseOrder[i]], padded[baseOrder[i + 1]], costArr);
    }
    B += transitionCost(padded[baseOrder[N - 1]], zeros, costArr);

    // ---- Compute score ratio ----
    //
    // Per the problem statement:
    //   score = 1,000,000 * clamp(B / E, 0, 2)
    //
    // The config sets per-test max to 200 (× 10000 platform multiplier = 2,000,000).
    // quitp(ratio) awards ratio * max_score.
    // So ratio = clamp(B/E, 0, 2) / 2 maps:
    //   B/E = 1  (baseline)  -> ratio = 0.5 -> 0.5 * 2,000,000 = 1,000,000  ✓
    //   B/E = 2  (2x better) -> ratio = 1.0 -> 1.0 * 2,000,000 = 2,000,000  ✓
    //   B/E > 2  (capped)    -> ratio = 1.0 -> 2,000,000                      ✓
    //   B/E < 1  (worse)     -> ratio < 0.5 -> < 1,000,000                    ✓
    //
    double be_ratio;
    if (E == 0) {
        // If E = 0 all costs must be zero; treat as perfect (B = 0 too).
        be_ratio = 2.0;
    } else {
        be_ratio = (double)B / (double)E;
    }

    // clamp to [0, 2]
    if (be_ratio < 0.0) be_ratio = 0.0;
    if (be_ratio > 2.0) be_ratio = 2.0;

    // Normalize to [0, 1] for quitp (config score:200, so full range = 2,000,000)
    double ratio = be_ratio / 2.0;

    // Safety clamp to [0, 1]
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    quitp(ratio,
          "Ratio: %.9f | E=%lld B=%lld B/E=%.6f",
          ratio,
          (long long)E,
          (long long)B,
          (E > 0 ? (double)B / (double)E : 0.0));

    return 0;
}