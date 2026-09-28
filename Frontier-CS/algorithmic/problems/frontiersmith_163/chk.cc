#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

typedef unsigned long long ull;
typedef __uint128_t u128;

// Circular error on 2^64 ring
ull circular_error(ull y, ull t) {
    ull delta = y - t; // mod 2^64 naturally
    ull neg_delta = (ull)(0ULL - delta);
    return delta < neg_delta ? delta : neg_delta;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // Read problem input
    int n = inf.readInt();
    int m = inf.readInt();

    vector<ull> b(n+1);
    vector<long long> u(n+1);
    for (int i = 1; i <= n; i++) {
        b[i] = inf.readUnsignedLong();
        u[i] = inf.readLong();
    }

    vector<ull> t(m+1);
    vector<long long> w(m+1);
    vector<int> l(m+1);
    for (int j = 1; j <= m; j++) {
        t[j] = inf.readUnsignedLong();
        w[j] = inf.readLong();
        l[j] = inf.readInt();
    }

    // ---- Compute baseline penalty ----
    vector<long long> rem(n+1);
    for (int i = 1; i <= n; i++) rem[i] = u[i];

    u128 baseline_penalty = 0;
    for (int j = 1; j <= m; j++) {
        // Option: empty set
        ull best_err = circular_error(0ULL, t[j]);
        int best_k = 0;
        int best_idx = -1; // -1 means empty

        if (l[j] >= 1) {
            for (int i = 1; i <= n; i++) {
                if (rem[i] <= 0) continue;
                ull y = b[i];
                ull err = circular_error(y, t[j]);
                // tie-break: smaller error, fewer beacons (1 < 0? no, we compare 1 vs 0), lex smaller index
                bool better = false;
                if (err < best_err) better = true;
                else if (err == best_err) {
                    // fewer beacons: singleton(1) vs empty(0) -> empty wins tie on fewer
                    if (1 < best_k) better = true;
                    else if (1 == best_k) {
                        // lex smaller index
                        if (best_idx == -1 || i < best_idx) better = true;
                    }
                }
                if (better) {
                    best_err = err;
                    best_k = 1;
                    best_idx = i;
                }
            }
        }

        baseline_penalty += (u128)w[j] * best_err;
        if (best_idx != -1) {
            rem[best_idx]--;
        }
    }

    // ---- Read and validate participant output ----
    vector<long long> usage(n+1, 0);
    u128 participant_penalty = 0;

    for (int j = 1; j <= m; j++) {
        int k = ouf.readInt(0, n, ("window " + to_string(j) + ": k out of range").c_str());

        if (k > l[j]) {
            quitf(_wa, "Window %d: k=%d exceeds limit l=%d", j, k, l[j]);
        }

        vector<int> chosen(k);
        for (int r = 0; r < k; r++) {
            chosen[r] = ouf.readInt(1, n, ("window " + to_string(j) + ": beacon index out of range").c_str());
        }

        // Check strictly increasing
        for (int r = 1; r < k; r++) {
            if (chosen[r] <= chosen[r-1]) {
                quitf(_wa, "Window %d: beacon indices not strictly increasing (%d then %d)",
                      j, chosen[r-1], chosen[r]);
            }
        }

        // Accumulate usage
        for (int r = 0; r < k; r++) {
            usage[chosen[r]]++;
        }

        // Compute y_j
        ull y = 0ULL;
        for (int r = 0; r < k; r++) {
            y += b[chosen[r]]; // wraps mod 2^64
        }

        ull err = circular_error(y, t[j]);
        participant_penalty += (u128)w[j] * err;
    }

    // Check end of output
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra data in output after %d windows", m);
    }

    // Check beacon usage constraints
    for (int i = 1; i <= n; i++) {
        if (usage[i] > u[i]) {
            quitf(_wa, "Beacon %d used %lld times but limit is %lld",
                  i, usage[i], u[i]);
        }
    }

    // ---- Compute score ----
    // score = min(200, 100 * (B+1) / (P+1))
    // ratio = score / 200 = min(1.0, 0.5 * (B+1) / (P+1))

    // Use long double for ratio computation
    long double B = (long double)baseline_penalty;
    long double P = (long double)participant_penalty;

    long double ratio = (B + 1.0L) / (2.0L * (P + 1.0L));
    if (ratio > 1.0L) ratio = 1.0L;
    if (ratio < 0.0L) ratio = 0.0L;

    // Format penalty strings for display (baseline and participant)
    // Since u128 can be large, convert to string manually
    // For display purposes use long double approximation
    long double score_val = ratio * 200.0L;

    quitp((double)ratio,
          "Ratio: %.9f | Score: %.4f/200 | Participant penalty: %.0Lf | Baseline penalty: %.0Lf",
          (double)ratio, (double)score_val, P, B);

    return 0;
}