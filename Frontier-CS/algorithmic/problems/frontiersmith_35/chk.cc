#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ── Read problem input ──────────────────────────────────────────────────
    int M = inf.readInt();
    int P = inf.readInt();

    vector<int> C(M + 1, 0);
    long long N = 0;
    for (int i = 1; i <= M; i++) {
        C[i] = inf.readInt();
        N += C[i];
    }

    vector<int>         L(P);
    vector<long long>   W(P);
    vector<vector<int>> motifs(P);

    for (int k = 0; k < P; k++) {
        L[k] = inf.readInt();
        W[k] = inf.readLong();
        motifs[k].resize(L[k]);
        for (int j = 0; j < L[k]; j++) {
            motifs[k][j] = inf.readInt();
        }
    }

    // ── Read participant output ─────────────────────────────────────────────
    vector<int> S((int)N);
    for (int i = 0; i < (int)N; i++) {
        S[i] = ouf.readInt(1, M,
                           ("s[" + to_string(i + 1) + "]").c_str());
    }
    // Reject trailing garbage
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra tokens found after the %lld required integers", N);
    }

    // ── Feasibility: exact multiset check ──────────────────────────────────
    vector<int> cnt(M + 1, 0);
    for (int v : S) cnt[v]++;
    for (int i = 1; i <= M; i++) {
        if (cnt[i] != C[i]) {
            quitf(_wa,
                  "Rune type %d appears %d time(s) but expected %d",
                  i, cnt[i], C[i]);
        }
    }

    // ── Helper: compute Revenue for a sequence ──────────────────────────────
    auto computeRevenue = [&](const vector<int>& seq) -> long long {
        int n = (int)seq.size();
        long long rev = 0;
        for (int k = 0; k < P; k++) {
            int lk = L[k];
            long long occ = 0;
            for (int x = 0; x + lk <= n; x++) {
                bool match = true;
                for (int j = 0; j < lk; j++) {
                    if (seq[x + j] != motifs[k][j]) { match = false; break; }
                }
                if (match) occ++;
            }
            rev += W[k] * occ;
        }
        return rev;
    };

    // ── Participant revenue ─────────────────────────────────────────────────
    long long R = computeRevenue(S);

    // ── Baseline: 1^C1, 2^C2, …, M^CM ─────────────────────────────────────
    vector<int> baseline;
    baseline.reserve((int)N);
    for (int i = 1; i <= M; i++)
        for (int j = 0; j < C[i]; j++)
            baseline.push_back(i);

    long long B = computeRevenue(baseline);

    // ── Score formula: floor(2,000,000 * R / (R + B + 1)), clamped [0,2e6] ─
    // Per the problem statement. We then express as ratio = int_score/2000000
    // so that quitp(ratio,...) correctly conveys the floored integer score.
    long long denom_ll = R + B + 1;
    long long int_score = (long long)(2000000LL * R / denom_ll);
    if (int_score < 0LL)        int_score = 0LL;
    if (int_score > 2000000LL)  int_score = 2000000LL;

    // The ratio we pass to quitp must equal int_score/2,000,000 so the
    // judge's internal multiplication reproduces the statement's formula.
    double score_ratio = (double)int_score / 2000000.0;
    if (score_ratio < 0.0) score_ratio = 0.0;
    if (score_ratio > 1.0) score_ratio = 1.0;

    quitp(score_ratio,
          "Ratio: %.10f | R=%lld B=%lld IntScore=%lld",
          score_ratio, R, B, int_score);

    return 0;
}