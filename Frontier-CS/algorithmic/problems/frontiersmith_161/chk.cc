#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

typedef long long ll;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- Read problem input from inf ----
    int n = inf.readInt();
    ll B  = inf.readLong();

    vector<ll> X(n + 1), Y(n + 1), H(n + 1), Q(n + 1);
    X[0] = inf.readLong();
    Y[0] = inf.readLong();
    Q[0] = inf.readLong();
    H[0] = 0;

    for (int i = 1; i <= n; i++) {
        X[i] = inf.readLong();
        Y[i] = inf.readLong();
        H[i] = inf.readLong();
        Q[i] = inf.readLong();
    }

    auto mdist = [&](int a, int b) -> ll {
        return abs(X[a] - X[b]) + abs(Y[a] - Y[b]);
    };

    // ---- Compute baseline cost ----
    ll C_base = 0;
    {
        vector<bool> cut(n + 1, false);
        vector<bool> availCamp(n + 1, false);
        availCamp[0] = true;
        int remaining = n;

        while (remaining > 0) {
            // Step 1: choose camp with min q, break ties by smaller id
            int c = -1;
            for (int i = 0; i <= n; i++) {
                if (availCamp[i]) {
                    if (c == -1 || Q[i] < Q[c] || (Q[i] == Q[c] && i < c))
                        c = i;
                }
            }

            // Step 2: start day
            ll rem = B;
            int cur = c;
            vector<bool> cutToday(n + 1, false);
            vector<int> todayTrees;

            // Step 3: greedy nearest uncut with height <= rem
            while (true) {
                int best = -1;
                ll bestDist = -1;
                for (int i = 1; i <= n; i++) {
                    if (!cut[i] && !cutToday[i] && H[i] <= rem) {
                        ll d = mdist(cur, i);
                        if (best == -1 || d < bestDist || (d == bestDist && i < best)) {
                            best = i;
                            bestDist = d;
                        }
                    }
                }
                if (best == -1) break;
                cutToday[best] = true;
                todayTrees.push_back(best);
                rem -= H[best];
                cur = best;
            }

            if (todayTrees.empty()) {
                // Should not happen given constraints (h_i <= B), but guard anyway
                break;
            }

            // Travel cost for baseline day
            ll travel = 0;
            int prev = c;
            for (int v : todayTrees) {
                travel += mdist(prev, v);
                prev = v;
            }
            travel += mdist(prev, c);

            // Electricity cost
            ll totalH = 0;
            for (int v : todayTrees) totalH += H[v];
            ll elec = Q[c] * totalH;

            C_base += travel + elec;

            // Mark trees as cut/available
            for (int v : todayTrees) {
                cut[v] = true;
                availCamp[v] = true;
                remaining--;
            }
        }
    }

    // ---- Read and validate participant output ----
    int D = ouf.readInt(1, n, "D must be between 1 and n");

    vector<int> treeAppearCount(n + 1, 0);
    vector<bool> cut(n + 1, false);
    vector<bool> availCamp(n + 1, false);
    availCamp[0] = true;

    ll C_you = 0;

    for (int day = 1; day <= D; day++) {
        // Read camp
        int c = ouf.readInt(0, n, "camp c must be in [0,n]");

        if (!availCamp[c]) {
            quitf(_wa, "Day %d: camp %d is not available (not yet cut or invalid)", day, c);
        }

        // Read k
        int k = ouf.readInt(1, n, "k must be >= 1");

        // Read trees
        vector<int> visited;
        ll remCap = B;

        for (int j = 0; j < k; j++) {
            int v = ouf.readInt(1, n, "tree id must be in [1,n]");
            treeAppearCount[v]++;

            if (treeAppearCount[v] > 1) {
                quitf(_wa, "Day %d: tree %d appears more than once in total output", day, v);
            }

            if (cut[v]) {
                quitf(_wa, "Day %d: tree %d was already cut before this day", day, v);
            }

            // Check for duplicate within same day
            for (int prev : visited) {
                if (prev == v) {
                    quitf(_wa, "Day %d: tree %d appears twice on the same day", day, v);
                }
            }

            visited.push_back(v);
            remCap -= H[v];
            if (remCap < 0) {
                quitf(_wa, "Day %d: total height exceeds B=%lld", day, B);
            }
        }

        // Travel cost
        ll travel = 0;
        int prev = c;
        for (int v : visited) {
            travel += mdist(prev, v);
            prev = v;
        }
        travel += mdist(prev, c);

        // Electricity cost
        ll totalH = 0;
        for (int v : visited) totalH += H[v];
        ll elec = Q[c] * totalH;

        C_you += travel + elec;

        // Mark trees as cut and available for next days
        for (int v : visited) {
            cut[v] = true;
            availCamp[v] = true;
        }
    }

    // Check all trees appear exactly once
    for (int i = 1; i <= n; i++) {
        if (treeAppearCount[i] != 1) {
            quitf(_wa,
                  "Tree %d appears %d times (expected exactly 1)",
                  i, treeAppearCount[i]);
        }
    }

    // Strict EOF check: reject trailing garbage tokens
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra tokens found after the end of the output");
    }

    // ---- Compute score ratio in [0, 1] ----
    // Problem: Score_test = clamp(0, 200, 100 * C_base / C_you)
    // We map this to [0, 1] by dividing by 200:
    //   ratio = clamp(0, 1, 0.5 * C_base / C_you)
    // Matching baseline (C_you == C_base): ratio = 0.5 -> 50% of 200 = 100 pts
    // Beating 2x (C_you = 0.5 * C_base): ratio = 1.0 -> 100% of 200 = 200 pts
    // Worse (C_you > C_base): ratio < 0.5 -> < 100 pts
    // config subtasks score: 200 so each test awards up to 200/n_cases points.

    double ratio;
    if (C_you <= 0) {
        // Zero cost only if all q=0 and all trees at same location as camp
        ratio = 1.0;
    } else {
        // ratio = clamp(0, 1, C_base / (2 * C_you))
        ratio = (double)C_base / (2.0 * (double)C_you);
        if (ratio > 1.0) ratio = 1.0;
        if (ratio < 0.0) ratio = 0.0;
    }

    quitp(ratio,
          "OK: D=%d TotalCost=%lld BaselineCost=%lld Ratio: %.6f",
          D, (long long)C_you, (long long)C_base, ratio);

    return 0;
}