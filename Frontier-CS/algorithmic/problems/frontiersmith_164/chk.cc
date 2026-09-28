#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- Read problem input (inf) ----
    int n = inf.readInt();
    int M = inf.readInt();

    vector<long long> T(n), W(n);
    for (int i = 0; i < n; i++) T[i] = inf.readLong();
    for (int i = 0; i < n; i++) W[i] = inf.readLong();

    // ---- Helper: compute baseline error E_base ----
    auto computeBaseline = [&]() -> long long {
        long long B = ((long long)n + M - 1) / M;
        long long E_base = 0;
        for (long long bstart = 0; bstart < n; bstart += B) {
            long long bend = min(bstart + B - 1, (long long)n - 1);
            int L = (int)bstart, R = (int)bend;

            long long S = 0;
            for (int i = L; i <= R; i++) S += W[i];
            long long half = (S + 1) / 2; // ceil(S/2)

            vector<pair<long long, long long>> tw;
            tw.reserve(R - L + 1);
            for (int i = L; i <= R; i++) tw.push_back({T[i], W[i]});
            sort(tw.begin(), tw.end());

            long long cum = 0, median_h = 0;
            for (auto& p : tw) {
                cum += p.second;
                if (cum >= half) { median_h = p.first; break; }
            }

            // Baseline: fill pass if median_h > 0, else stays 0
            long long block_h = median_h;
            for (int i = L; i <= R; i++) {
                long long diff = block_h - T[i];
                if (diff < 0) diff = -diff;
                E_base += W[i] * diff;
            }
        }
        return E_base;
    };

    // ---- Compute E_base ----
    long long E_base = computeBaseline();

    // ---- Helper: compute score_ratio from E and E_base ----
    // score = floor(10^6 * min(10, E_base / max(1, E)))
    // normalized ratio = score / 10^7, clamped to [0, 1]
    auto computeScoreRatio = [&](long long E) -> double {
        long long denom = max(1LL, E);
        double ratio = (double)E_base / (double)denom;
        if (ratio > 10.0) ratio = 10.0;
        // score = floor(10^6 * ratio), max is 10^7
        double sr = ratio / 10.0; // normalize to [0, 1]
        if (sr < 0.0) sr = 0.0;
        if (sr > 1.0) sr = 1.0;
        return sr;
    };

    // ---- Handle empty output case (q = 0, all heights remain 0) ----
    if (ouf.seekEof()) {
        long long E = 0;
        for (int i = 0; i < n; i++) {
            E += W[i] * T[i]; // |0 - T[i]|
        }
        double sr = computeScoreRatio(E);
        quitp(sr, "Ratio: %.9f E=%lld E_base=%lld (empty output, q=0)", sr, E, E_base);
    }

    // ---- Read q ----
    int q = ouf.readInt(0, M, "q");

    // ---- Read and validate passes ----
    struct Pass { int t, L, R, h; };
    vector<Pass> passes(q);
    for (int i = 0; i < q; i++) {
        int t = ouf.readInt(1, 2, "pass type");
        int L = ouf.readInt(0, n - 1, "L");
        int R = ouf.readInt(L, n - 1, "R");
        int h = ouf.readInt(0, 100000, "h");
        passes[i] = {t, L, R, h};
    }

    // ---- Check for trailing non-whitespace tokens ----
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra tokens found after all passes were read");
    }

    // ---- Simulate passes ----
    vector<long long> x(n, 0);
    for (auto& p : passes) {
        if (p.t == 1) {
            for (int i = p.L; i <= p.R; i++) {
                if (x[i] < p.h) x[i] = p.h;
            }
        } else {
            for (int i = p.L; i <= p.R; i++) {
                if (x[i] > p.h) x[i] = p.h;
            }
        }
    }

    // ---- Compute participant error E ----
    long long E = 0;
    for (int i = 0; i < n; i++) {
        long long diff = x[i] - T[i];
        if (diff < 0) diff = -diff;
        E += W[i] * diff;
    }

    // ---- Compute and output score ----
    double sr = computeScoreRatio(E);
    quitp(sr, "Ratio: %.9f E=%lld E_base=%lld", sr, E, E_base);

    return 0;
}