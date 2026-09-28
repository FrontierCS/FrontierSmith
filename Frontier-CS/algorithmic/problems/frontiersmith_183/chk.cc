#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- read problem input ----
    int n = inf.readInt();
    int m = inf.readInt();
    long long B = inf.readLong();

    vector<double> T(m + 1), W(m + 1);
    for (int j = 1; j <= m; j++) {
        T[j] = inf.readDouble();
        W[j] = inf.readDouble();
    }

    vector<long long> sal(n + 1);
    vector<int> cap_limit(n + 1), rec(n + 1), dcnt(n + 1);
    vector<vector<pair<int, double>>> contrib(n + 1);

    for (int i = 1; i <= n; i++) {
        sal[i] = inf.readLong();
        cap_limit[i] = inf.readInt();
        rec[i] = inf.readInt();
        dcnt[i] = inf.readInt();
        contrib[i].resize(dcnt[i]);
        for (int t = 0; t < dcnt[i]; t++) {
            int cj = inf.readInt();
            double aj = inf.readDouble();
            contrib[i][t] = {cj, aj};
        }
    }

    // ---- helper: compute objective from capability totals ----
    auto computeObj = [&](const vector<double>& X) -> double {
        double obj = 0.0;
        for (int j = 1; j <= m; j++) {
            obj += W[j] * (1.0 - exp(-X[j] / T[j]));
        }
        return obj;
    };

    // ---- baseline greedy (exactly as specified) ----
    auto runGreedy = [&]() -> double {
        vector<bool> inS(n + 1, false);
        vector<int> childCount(n + 1, 0);
        long long totalCost = 0;
        vector<double> X(m + 1, 0.0);

        for (;;) {
            double curObj = computeObj(X);
            int best = -1;
            double bestRatio = -1e300, bestDelta = -1e300;
            long long bestSal = LLONG_MAX;

            for (int i = 1; i <= n; i++) {
                if (inS[i]) continue;
                int r = rec[i];
                if (r > 0 && !inS[r]) continue;
                if (r > 0 && childCount[r] >= cap_limit[r]) continue;
                if (totalCost + sal[i] > B) continue;

                // compute delta
                double delta = 0.0;
                for (auto& [cj, aj] : contrib[i]) {
                    delta += W[cj] * (exp(-X[cj] / T[cj]) - exp(-(X[cj] + aj) / T[cj]));
                }

                if (best == -1) {
                    best = i;
                    bestDelta = delta;
                    bestRatio = delta / (double)sal[i];
                    bestSal = sal[i];
                } else {
                    double ratio = delta / (double)sal[i];
                    if (ratio > bestRatio + 1e-15 ||
                        (fabs(ratio - bestRatio) <= 1e-15 && delta > bestDelta + 1e-15) ||
                        (fabs(ratio - bestRatio) <= 1e-15 && fabs(delta - bestDelta) <= 1e-15 && sal[i] < bestSal) ||
                        (fabs(ratio - bestRatio) <= 1e-15 && fabs(delta - bestDelta) <= 1e-15 && sal[i] == bestSal && i < best)) {
                        best = i;
                        bestDelta = delta;
                        bestRatio = ratio;
                        bestSal = sal[i];
                    }
                }
            }

            if (best == -1) break;
            if (bestDelta <= 0.0) break;

            inS[best] = true;
            totalCost += sal[best];
            for (auto& [cj, aj] : contrib[best]) X[cj] += aj;
            if (rec[best] > 0) childCount[rec[best]]++;
        }

        return computeObj(X);
    };

    double G = runGreedy();

    // ---- read R from ans file ----
    // R is the organizer's reference solution objective value, stored in ans.
    // If ans is absent/empty, we fall back to R = G (which gives everyone 100 per spec).
    double R = G; // default: R = G
    {
        // Try to read R from ans; if it fails (e.g. empty), keep R = G
        if (!ans.seekEof()) {
            R = ans.readDouble();
        }
    }

    // ---- read participant output ----
    int q = ouf.readInt();
    if (q < 0 || q > n) {
        quitf(_wa, "invalid q=%d (must be in [0, %d])", q, n);
    }

    vector<int> chosen(q);
    vector<bool> picked(n + 1, false);
    for (int i = 0; i < q; i++) {
        chosen[i] = ouf.readInt();
        if (chosen[i] < 1 || chosen[i] > n) {
            quitf(_wa, "candidate ID %d out of range [1, %d]", chosen[i], n);
        }
        if (picked[chosen[i]]) {
            quitf(_wa, "duplicate candidate id %d", chosen[i]);
        }
        picked[chosen[i]] = true;
    }

    // check EOF
    if (!ouf.seekEof()) {
        quitf(_wa, "extra data after the team selection");
    }

    // ---- validate feasibility ----
    // budget
    long long totalSal = 0;
    for (int i : chosen) totalSal += sal[i];
    if (totalSal > B) {
        quitf(_wa, "budget exceeded: cost=%lld > B=%lld", totalSal, B);
    }

    // referral closure & supervision limit
    vector<int> selectedChildCount(n + 1, 0);
    for (int i : chosen) {
        int r = rec[i];
        if (r > 0) {
            if (!picked[r]) {
                quitf(_wa, "referral closure violated: candidate %d selected but recommender %d not selected", i, r);
            }
            selectedChildCount[r]++;
        }
    }
    for (int i : chosen) {
        if (selectedChildCount[i] > cap_limit[i]) {
            quitf(_wa, "supervision limit violated: candidate %d has %d selected children but cap is %d",
                  i, selectedChildCount[i], cap_limit[i]);
        }
    }

    // ---- compute participant objective ----
    vector<double> X(m + 1, 0.0);
    for (int i : chosen) {
        for (auto& [cj, aj] : contrib[i]) {
            X[cj] += aj;
        }
    }
    double O = computeObj(X);

    // ---- compute score using stated formula ----
    // score = 100 * clamp((O - G) / (R - G), 0, 1)
    // Special case: if R = G (or R <= G + epsilon), every valid submission gets 100.
    double ratio;
    if (R <= G + 1e-9) {
        // R = G case: every valid submission receives 100 points
        ratio = 1.0;
    } else {
        ratio = (O - G) / (R - G);
        if (ratio > 1.0) ratio = 1.0;
        if (ratio < 0.0) ratio = 0.0;
    }

    quitp(ratio,
          "OK q=%d cost=%lld O=%.6f G=%.6f R=%.6f Ratio: %.6f",
          q, totalSal, O, G, R, ratio);

    return 0;
}