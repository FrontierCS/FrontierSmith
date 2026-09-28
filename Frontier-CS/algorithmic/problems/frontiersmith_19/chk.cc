#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // --- Read problem input from inf ---
    long long n = inf.readLong();
    long long q = inf.readLong();
    long long K = inf.readLong();
    long long B = inf.readLong();
    long long F = inf.readLong();
    long long G = inf.readLong();
    long long P = inf.readLong();

    vector<long long> mv(q);
    long long M = 0;
    for (int i = 0; i < (int)q; i++) {
        mv[i] = inf.readLong();
        M = max(M, mv[i]);
    }

    // BaseCost = n * q * P  (install 0 chests, every block pays penalty)
    // Use double to avoid overflow in intermediate; but check carefully
    // n<=3000, q<=3000, P<=1e9 => max baseCost = 9e15, fits in long long
    long long baseCost = n * q * P;

    // --- Read participant output from ouf ---
    long long T = ouf.readLong();

    if (T < 0 || T > K) {
        quitf(_wa, "T=%lld is out of range [0, %lld]", T, K);
    }

    // For each color c (1..M), store sorted list of chest positions containing c
    vector<vector<long long>> colorPositions(M + 1);

    long long totalColors = 0;

    for (long long t = 0; t < T; t++) {
        long long x = ouf.readLong();
        if (x < 1 || x > n) {
            quitf(_wa, "Chest %lld has invalid position %lld (must be 1..%lld)", t + 1, x, n);
        }

        long long s = ouf.readLong();
        if (s < 0 || s > B) {
            quitf(_wa, "Chest %lld has s=%lld colors, exceeds capacity B=%lld", t + 1, s, B);
        }

        set<long long> seenColors;
        for (long long ci = 0; ci < s; ci++) {
            long long c = ouf.readLong();
            if (c < 1 || c > M) {
                quitf(_wa, "Chest %lld has color %lld out of range [1, %lld]", t + 1, c, M);
            }
            if (seenColors.count(c)) {
                quitf(_wa, "Chest %lld has duplicate color %lld", t + 1, c);
            }
            seenColors.insert(c);
            colorPositions[c].push_back(x);
        }
        totalColors += s;
    }

    // Sort positions for each color (binary search later)
    for (long long c = 1; c <= M; c++) {
        sort(colorPositions[c].begin(), colorPositions[c].end());
    }

    // Helper: min distance from position j to any chest containing color c, or P if none
    auto serviceCost = [&](long long c, long long j) -> long long {
        const auto& positions = colorPositions[c];
        if (positions.empty()) {
            return P;
        }
        auto it = lower_bound(positions.begin(), positions.end(), j);
        long long best = LLONG_MAX / 2;
        if (it != positions.end()) {
            best = min(best, *it - j);
        }
        if (it != positions.begin()) {
            --it;
            best = min(best, j - *it);
        }
        return best;
    };

    // Compute total service cost over all rounds and positions
    // n*q <= 6e6, each call is O(log K), so total ~6e6 * log(300) ~ 50M ops — acceptable
    long long serviceTotalCost = 0;

    for (long long i = 0; i < q; i++) {
        long long mi = mv[i];
        for (long long j = 1; j <= n; j++) {
            long long color = ((j - 1) % mi) + 1;
            serviceTotalCost += serviceCost(color, j);
        }
    }

    // Total cost = F*T + G*totalColors + serviceTotalCost
    // Overflow check: F*T <= 1e9*300=3e11, G*totalColors <= 1e9*300*30=9e12,
    // serviceTotalCost <= n*q*P = 9e15; sum fits in long long
    long long fixedCost = F * T + G * totalColors;
    long long cost = fixedCost + serviceTotalCost;

    // Compute score ratio = min(10, baseCost/cost) / 10, giving a value in [0,1]
    double ratio;
    if (baseCost == 0 && cost == 0) {
        // Both zero: perfect solution
        ratio = 1.0;
    } else if (baseCost == 0) {
        // Baseline is zero: any positive cost is worse than baseline
        ratio = 0.0;
    } else if (cost == 0) {
        // Our cost is 0 but baseline isn't: best possible, cap ratio at 1
        ratio = 1.0;
    } else {
        // ratio = min(10, baseCost/cost) / 10
        double r = (double)baseCost / (double)cost;
        if (r > 10.0) r = 10.0;
        ratio = r / 10.0;
    }

    // Clamp to [0, 1]
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    quitp(ratio,
          "Ratio: %.9f | BaseCost: %lld | Cost: %lld (fixed=%lld service=%lld) | T=%lld colors=%lld",
          ratio,
          baseCost,
          cost,
          fixedCost,
          serviceTotalCost,
          T,
          totalColors);

    return 0;
}