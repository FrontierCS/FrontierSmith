#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

typedef long long ll;
typedef unsigned long long ull;
typedef __int128 lll;

ll digitSum(ll x) {
    if (x <= 0) return 0;
    ll s = 0;
    while (x > 0) {
        s += x % 10;
        x /= 10;
    }
    return s;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    int T = inf.readInt();

    // We accumulate per-test scores (each in [0, 5e6]), then take floor of mean / 5e6
    double totalScore = 0.0;
    int numCases = 0;

    for (int t = 0; t < T; t++) {
        int H = inf.readInt();
        int W = inf.readInt();
        int K = inf.readInt();
        ll C = inf.readLong();
        ll F = inf.readLong();

        vector<vector<ll>> grid(H, vector<ll>(W));
        for (int r = 0; r < H; r++)
            for (int c = 0; c < W; c++)
                grid[r][c] = inf.readLong();

        // --- Read participant output ---
        int M = ouf.readInt();
        if (M < 0 || M > K)
            quitf(_wa, "Test %d: M=%d out of range [0,%d]", t+1, M, K);

        struct Pulse { int r1, c1, r2, c2; };
        vector<Pulse> pulses(M);
        for (int i = 0; i < M; i++) {
            int r1 = ouf.readInt();
            int c1 = ouf.readInt();
            int r2 = ouf.readInt();
            int c2 = ouf.readInt();
            if (r1 < 1 || r2 > H || r1 > r2)
                quitf(_wa, "Test %d, pulse %d: invalid row range [%d,%d] for H=%d", t+1, i+1, r1, r2, H);
            if (c1 < 1 || c2 > W || c1 > c2)
                quitf(_wa, "Test %d, pulse %d: invalid col range [%d,%d] for W=%d", t+1, i+1, c1, c2, W);
            pulses[i] = {r1, c1, r2, c2};
        }

        // Check total cost does not exceed budget
        ll totalCost = 0;
        for (int i = 0; i < M; i++) {
            ll area = (ll)(pulses[i].r2 - pulses[i].r1 + 1) * (ll)(pulses[i].c2 - pulses[i].c1 + 1);
            ll pulseCost = F + area;
            // overflow-safe: if totalCost > C already, we know it's over
            if (totalCost > C || C - totalCost < pulseCost)
                quitf(_wa, "Test %d: total cost exceeds budget C=%lld at pulse %d", t+1, C, i+1);
            totalCost += pulseCost;
        }

        // --- Simulate participant's solution ---
        vector<vector<ll>> pgrid = grid;
        for (int i = 0; i < M; i++) {
            int r1 = pulses[i].r1 - 1;
            int c1 = pulses[i].c1 - 1;
            int r2 = pulses[i].r2 - 1;
            int c2 = pulses[i].c2 - 1;
            for (int r = r1; r <= r2; r++)
                for (int c = c1; c <= c2; c++)
                    pgrid[r][c] = digitSum(pgrid[r][c]);
        }

        ll S = 0;
        for (int r = 0; r < H; r++)
            for (int c = 0; c < W; c++)
                S += pgrid[r][c];

        // --- Simulate deterministic baseline ---
        vector<vector<ll>> bgrid = grid;
        ll remBudget = C;
        int remPulses = K;

        while (remPulses > 0 && remBudget >= F + 1) {
            int bestR = -1, bestC = -1;
            ll bestGain = 0;
            for (int r = 0; r < H; r++) {
                for (int c = 0; c < W; c++) {
                    ll gain = bgrid[r][c] - digitSum(bgrid[r][c]);
                    if (gain > bestGain ||
                        (gain == bestGain && bestR >= 0 && (r < bestR || (r == bestR && c < bestC)))) {
                        bestGain = gain;
                        bestR = r;
                        bestC = c;
                    }
                }
            }
            if (bestGain == 0 || bestR < 0) break;
            bgrid[bestR][bestC] = digitSum(bgrid[bestR][bestC]);
            remBudget -= F + 1;
            remPulses--;
        }

        ll B = 0;
        for (int r = 0; r < H; r++)
            for (int c = 0; c < W; c++)
                B += bgrid[r][c];

        // --- Compute per-test score per the statement ---
        // score = 10^6 * clamp(B/S, 0, 5)
        // S >= 1 always (all values positive, sd(x)>=1 for x>=1)
        double ratio;
        if (S <= 0) {
            ratio = 5.0;
        } else {
            ratio = (double)B / (double)S;
        }
        if (ratio < 0.0) ratio = 0.0;
        if (ratio > 5.0) ratio = 5.0;

        // per-test score in [0, 5000000]
        double perTestScore = 1e6 * ratio;
        totalScore += perTestScore;
        numCases++;
    }

    // Assert end of participant output
    if (!ouf.seekEof())
        quitf(_wa, "Unexpected trailing data in participant output");

    // Overall score = floor(arithmetic mean of per-test scores)
    // Each per-test score in [0, 5e6], so overall in [0, 5e6]
    double meanScore = (numCases > 0) ? (totalScore / (double)numCases) : 0.0;
    double flooredMean = floor(meanScore);

    // Normalize to [0, 1] for quitp: divide by max possible (5e6)
    double scoreRatio = flooredMean / 5e6;
    if (scoreRatio < 0.0) scoreRatio = 0.0;
    if (scoreRatio > 1.0) scoreRatio = 1.0;

    quitp(scoreRatio,
          "Ratio: %.9f (floor(mean per-test score)=%.0f out of 5000000, over %d cases)",
          scoreRatio, flooredMean, numCases);

    return 0;
}