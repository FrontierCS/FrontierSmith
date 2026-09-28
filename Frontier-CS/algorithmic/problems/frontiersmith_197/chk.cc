#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- Read problem input from inf ----
    int H = inf.readInt();
    int W = inf.readInt();
    int P = inf.readInt();
    int K = inf.readInt();
    int N = inf.readInt();
    long long F = inf.readLong();

    // Target color grid
    vector<vector<int>> T(H, vector<int>(W));
    for (int r = 0; r < H; r++)
        for (int c = 0; c < W; c++)
            T[r][c] = inf.readInt();

    // Importance weights
    vector<vector<long long>> V(H, vector<long long>(W));
    for (int r = 0; r < H; r++)
        for (int c = 0; c < W; c++)
            V[r][c] = (long long)inf.readInt();

    // Offers (1-indexed externally, 0-indexed here)
    struct Offer {
        int color, top, left, bottom, right;
        long long cost;
    };
    vector<Offer> offers(N);
    for (int i = 0; i < N; i++) {
        offers[i].color  = inf.readInt();
        offers[i].top    = inf.readInt();
        offers[i].left   = inf.readInt();
        offers[i].bottom = inf.readInt();
        offers[i].right  = inf.readInt();
        offers[i].cost   = inf.readLong();
    }

    // ---- Compute baseline B: empty plan (all cells blank) ----
    long long B = 0;
    for (int r = 0; r < H; r++)
        for (int c = 0; c < W; c++)
            B += 2LL * V[r][c];

    // ---- Read participant output from ouf ----
    // Handle completely empty / EOF output as M=0
    int M = 0;
    if (!ouf.seekEof()) {
        M = ouf.readInt(0, N, "M must be between 0 and N");
    }

    vector<int> accepted(M);
    set<int> usedIds;

    if (M > 0) {
        for (int i = 0; i < M; i++) {
            if (ouf.seekEof())
                quitf(_wa, "expected %d offer IDs but output ended after %d", M, i);
            int id = ouf.readInt(1, N, "offer ID out of range [1,N]");
            if (usedIds.count(id))
                quitf(_wa, "duplicate offer ID %d", id);
            usedIds.insert(id);
            accepted[i] = id;
        }
    }

    // Strict end-of-file check: reject any trailing tokens
    if (!ouf.seekEof())
        quitf(_wa, "extra output after the last offer ID");

    // ---- Simulate the layered painting ----
    vector<vector<int>> finalColor(H, vector<int>(W, 0));
    for (int i = 0; i < M; i++) {
        const Offer& o = offers[accepted[i] - 1];
        for (int r = o.top - 1; r <= o.bottom - 1; r++)
            for (int c = o.left - 1; c <= o.right - 1; c++)
                finalColor[r][c] = o.color;
    }

    // ---- Compute penalty Y ----
    long long Y = 0;

    // 1. Cell penalty
    for (int r = 0; r < H; r++) {
        for (int c = 0; c < W; c++) {
            int fc = finalColor[r][c];
            if (fc == 0) {
                Y += 2LL * V[r][c];
            } else if (fc != T[r][c]) {
                Y += V[r][c];
            }
            // fc == T[r][c]: penalty 0
        }
    }

    // 2. Offer costs
    for (int i = 0; i < M; i++)
        Y += offers[accepted[i] - 1].cost;

    // 3. Color-overflow penalty
    set<int> visibleColors;
    for (int r = 0; r < H; r++)
        for (int c = 0; c < W; c++)
            if (finalColor[r][c] != 0)
                visibleColors.insert(finalColor[r][c]);

    int D = (int)visibleColors.size();
    long long overflow = (long long)max(0, D - K);
    Y += F * overflow * overflow;

    // ---- Compute score = round(10^6 * B / (B + Y)) exactly ----
    // Use integer arithmetic via llround to avoid floating-point drift.
    long long intScore;
    double ratio;
    if (B == 0 && Y == 0) {
        intScore = 1000000LL;
        ratio = 1.0;
    } else if (B == 0) {
        // No cells at all; any valid answer is perfect
        intScore = 1000000LL;
        ratio = 1.0;
    } else {
        // B > 0, Y >= 0
        // score = round(1e6 * B / (B + Y))
        double denom = (double)(B + Y);
        double exact = (double)B / denom;
        intScore = llround(1e6 * exact);
        if (intScore < 0LL) intScore = 0LL;
        if (intScore > 1000000LL) intScore = 1000000LL;
        ratio = (double)intScore / 1000000.0;
    }

    // The judge reads "Ratio: <value>" from the message and awards
    // round(10^6 * value) points per file, so we pass intScore/1e6.
    quitp(ratio,
          "Ratio: %f Score: %lld Y=%lld B=%lld M=%d D=%d overflow=%lld",
          ratio, intScore, Y, B, M, D, overflow);

    return 0;
}