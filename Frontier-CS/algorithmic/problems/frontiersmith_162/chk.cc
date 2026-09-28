#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- Read problem input from inf ----
    int N = inf.readInt();
    int C = inf.readInt();
    int B = inf.readInt();
    int M = inf.readInt();

    struct Contract {
        long long p;
        int q;
        int px[3], pc[3];
    };

    vector<Contract> contracts(M);
    long long U = 0;

    for (int j = 0; j < M; j++) {
        contracts[j].p = inf.readLong();
        contracts[j].q = inf.readInt();
        for (int t = 0; t < contracts[j].q; t++) {
            contracts[j].px[t] = inf.readInt();
            contracts[j].pc[t] = inf.readInt();
        }
        U += contracts[j].p;
    }

    // Edge case: zero total profit -> any output scores 1
    if (U <= 0) {
        // consume whatever is in ouf
        ouf.readEof(); // lenient: just accept
        quitp(1.0, "Ratio: 1.000000000, V=0, U=0 (no profitable contracts)");
    }

    // ---- Read and STRICTLY validate participant output ----
    // Read T
    if (ouf.seekEof()) {
        // Empty output: 0 strokes, score based on V=0
        long long score_int = 0;
        double ratio = 0.0;
        quitp(ratio, "Ratio: %.9f, V=0, U=%lld (empty output, T=0)", ratio, U);
    }

    int T = ouf.readInt();

    // T must be in [0, B]
    if (T < 0 || T > B) {
        // Invalid output -> score 0
        quitp(0.0, "Ratio: 0.000000000, V=0, U=%lld (invalid T=%d, must be in [0,%d])", U, T, B);
    }

    vector<int> sl(T), sr(T), sc(T);

    for (int i = 0; i < T; i++) {
        int l = ouf.readInt();
        int r = ouf.readInt();
        int c = ouf.readInt();

        // Strict validation
        if (l < 1 || l > N) {
            quitp(0.0, "Ratio: 0.000000000, V=0, U=%lld (stroke %d: l=%d out of [1,%d])", U, i+1, l, N);
        }
        if (r < l || r > N) {
            quitp(0.0, "Ratio: 0.000000000, V=0, U=%lld (stroke %d: r=%d invalid, l=%d, N=%d)", U, i+1, r, l, N);
        }
        if (c < 0 || c > C) {
            quitp(0.0, "Ratio: 0.000000000, V=0, U=%lld (stroke %d: c=%d out of [0,%d])", U, i+1, c, C);
        }

        sl[i] = l;
        sr[i] = r;
        sc[i] = c;
    }

    // EOF check: no trailing tokens allowed (beyond whitespace/newlines)
    if (!ouf.seekEof()) {
        quitp(0.0, "Ratio: 0.000000000, V=0, U=%lld (extra output after %d strokes)", U, T);
    }

    // ---- Simulate to find final color of every referenced panel ----
    // Build set of needed positions
    vector<bool> needed(N + 1, false);
    for (int j = 0; j < M; j++)
        for (int t = 0; t < contracts[j].q; t++)
            needed[contracts[j].px[t]] = true;

    vector<int> final_color(N + 1, 0); // 0 = white (initial)
    vector<bool> settled(N + 1, false);

    int remaining = 0;
    for (int x = 1; x <= N; x++) if (needed[x]) remaining++;

    // Iterate strokes in reverse to find last stroke covering each needed panel
    for (int i = T - 1; i >= 0 && remaining > 0; i--) {
        int l = sl[i], r = sr[i], c = sc[i];
        for (int x = l; x <= r && remaining > 0; x++) {
            if (needed[x] && !settled[x]) {
                final_color[x] = c;
                settled[x] = true;
                remaining--;
            }
        }
    }
    // Panels never painted stay 0 (white).

    // ---- Evaluate contracts ----
    long long V = 0;
    for (int j = 0; j < M; j++) {
        bool ok = true;
        for (int t = 0; t < contracts[j].q; t++) {
            if (final_color[contracts[j].px[t]] != contracts[j].pc[t]) {
                ok = false;
                break;
            }
        }
        if (ok) V += contracts[j].p;
    }

    // ---- Compute exact score: floor(1e9 * V / U), clamped to [0, 1e9] ----
    // Use __int128 to avoid overflow (V up to ~3e14, 1e9*V up to ~3e23)
    __int128 numerator = (__int128)1000000000LL * (long long)V;
    long long score_int = (long long)(numerator / (long long)U);
    if (score_int < 0LL) score_int = 0LL;
    if (score_int > 1000000000LL) score_int = 1000000000LL;

    // ratio in [0,1] for quitp
    double ratio = (double)score_int / 1000000000.0;
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    quitp(ratio,
        "Ratio: %.9f, score=%lld, V=%lld, U=%lld, T=%d",
        ratio, score_int, V, U, T);

    return 0;
}