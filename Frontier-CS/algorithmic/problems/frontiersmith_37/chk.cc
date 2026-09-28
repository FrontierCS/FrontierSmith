#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

typedef long long ll;

static ll greedyCount(const vector<int>& descDenoms, int x) {
    ll cnt = 0;
    int rem = x;
    for (int d : descDenoms) {
        if (d <= rem) {
            cnt += rem / d;
            rem %= d;
        }
        if (rem == 0) break;
    }
    return cnt;
}

static ll computeCost(
    const vector<int>& selIdx0,   // 0-based
    const vector<int>& vals,
    const vector<ll>& costs,
    ll A, ll B,
    const vector<int>& xs,
    const vector<ll>& ws,
    int maxX)
{
    ll cost = 0;
    vector<int> ascVals;
    for (int i : selIdx0) {
        cost += costs[i];
        ascVals.push_back(vals[i]);
    }
    sort(ascVals.begin(), ascVals.end());
    vector<int> descVals(ascVals.rbegin(), ascVals.rend());

    // unbounded knapsack DP for optimal coin count
    const ll INF = (ll)2e18;
    vector<ll> dp(maxX + 1, INF);
    dp[0] = 0;
    for (int v : ascVals) {
        for (int i = v; i <= maxX; i++) {
            if (dp[i - v] != INF && dp[i - v] + 1 < dp[i])
                dp[i] = dp[i - v] + 1;
        }
    }

    int Q = (int)xs.size();
    for (int j = 0; j < Q; j++) {
        int x = xs[j];
        ll G = greedyCount(descVals, x);
        ll O = (dp[x] == INF) ? G : dp[x];
        cost += ws[j] * (A * G + B * (G - O));
    }
    return cost;
}

static ll computeBaseline(
    int N, int K,
    const vector<int>& vals,
    const vector<ll>& costs,
    ll A, ll B,
    const vector<int>& xs,
    const vector<ll>& ws,
    int maxX)
{
    int Q = (int)xs.size();

    vector<bool> inS(N, false);
    inS[0] = true;
    vector<int> S = {0};

    auto buildDesc = [&]() {
        vector<int> dv;
        for (int i : S) dv.push_back(vals[i]);
        sort(dv.begin(), dv.end(), greater<int>());
        return dv;
    };

    auto buildDp = [&]() {
        const ll INF = (ll)2e18;
        vector<ll> dp(maxX + 1, INF);
        dp[0] = 0;
        vector<int> asc;
        for (int i : S) asc.push_back(vals[i]);
        sort(asc.begin(), asc.end());
        for (int v : asc)
            for (int i = v; i <= maxX; i++)
                if (dp[i - v] != INF && dp[i - v] + 1 < dp[i])
                    dp[i] = dp[i - v] + 1;
        return dp;
    };

    while ((int)S.size() < K) {
        vector<int> descS = buildDesc();

        // precompute greedy counts for current S
        vector<ll> gCur(Q);
        for (int j = 0; j < Q; j++)
            gCur[j] = greedyCount(descS, xs[j]);

        // find best candidate
        ll bestBenNum = 0, bestBenDen = 1; // benefit/denominator comparison
        // efficiency = benefit / (1 + c_d)
        // compare a/b vs c/d by cross-mult
        int bestD = -1;
        ll bestCD = -1;

        for (int d = 0; d < N; d++) {
            if (inS[d]) continue;

            // build new desc list with d inserted
            vector<int> newDesc = descS;
            {
                int vd = vals[d];
                auto it = lower_bound(newDesc.begin(), newDesc.end(), vd, greater<int>());
                newDesc.insert(it, vd);
            }

            ll benefit = 0;
            for (int j = 0; j < Q; j++) {
                ll gNew = greedyCount(newDesc, xs[j]);
                ll diff = gCur[j] - gNew;
                if (diff > 0) benefit += ws[j] * diff;
            }

            if (benefit == 0 && bestBenNum == 0) {
                // still might be "best" if nothing chosen yet
                // but we'll stop if best benefit = 0
            }

            // compare benefit/(1+c_d) vs bestBenNum/bestBenDen
            // i.e. benefit * bestBenDen vs bestBenNum * (1+c_d)
            ll denom = 1 + costs[d];
            // benefit/denom > bestBenNum/bestBenDen ?
            // iff benefit * bestBenDen > bestBenNum * denom
            bool better = false;
            if (benefit * bestBenDen > bestBenNum * denom) {
                better = true;
            } else if (benefit * bestBenDen == bestBenNum * denom) {
                // tie-break: smaller c_d
                if (bestD == -1 || costs[d] < bestCD) better = true;
                else if (costs[d] == bestCD && d < bestD) better = true;
            }

            if (better) {
                bestBenNum = benefit;
                bestBenDen = denom;
                bestD = d;
                bestCD = costs[d];
            }
        }

        if (bestD == -1 || bestBenNum == 0) break;

        inS[bestD] = true;
        S.push_back(bestD);
    }

    return computeCost(S, vals, costs, A, B, xs, ws, maxX);
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // --- Read input ---
    int N = inf.readInt();
    int K = inf.readInt();
    ll A = inf.readLong();
    ll B = inf.readLong();

    vector<int> vals(N);
    for (int i = 0; i < N; i++) vals[i] = inf.readInt();

    vector<ll> costs(N);
    for (int i = 0; i < N; i++) costs[i] = inf.readLong();

    int Q = inf.readInt();
    vector<int> xs(Q);
    vector<ll> ws(Q);
    int maxX = 0;
    for (int j = 0; j < Q; j++) {
        xs[j] = inf.readInt();
        ws[j] = inf.readLong();
        if (xs[j] > maxX) maxX = xs[j];
    }

    // --- Read participant output ---
    // Line 1: m
    int m = ouf.readInt(1, K, "m must be between 1 and K");

    // Line 2: m indices
    vector<int> idxs(m);
    for (int i = 0; i < m; i++) {
        idxs[i] = ouf.readInt(1, N, "each index must be in [1,N]");
    }

    // Check for trailing non-whitespace content
    // (testlib readEof skips whitespace before checking)
    // We skip the strict readEof to avoid false positives with trailing newlines,
    // but we do consume and check for unexpected extra tokens.
    // In testlib, readEof() skips blank chars. Use it.
    // If there is extra real content after the m indices, flag it.
    // We'll just ignore trailing whitespace (testlib readEof handles this).
    // Only fail if there's actual non-whitespace extra content.
    // The safest approach: don't call readEof at all, just proceed.

    // Validate distinct
    {
        vector<int> tmp = idxs;
        sort(tmp.begin(), tmp.end());
        for (int i = 1; i < m; i++) {
            if (tmp[i] == tmp[i-1])
                quitf(_wa, "duplicate index %d in output", tmp[i]);
        }
    }

    // Validate index 1 is present
    {
        bool has1 = false;
        for (int i = 0; i < m; i++) if (idxs[i] == 1) has1 = true;
        if (!has1) quitf(_wa, "index 1 (denomination 1) must be selected");
    }

    // Convert to 0-based
    vector<int> selIdx0(m);
    for (int i = 0; i < m; i++) selIdx0[i] = idxs[i] - 1;

    // --- Compute costs ---
    ll C_you = computeCost(selIdx0, vals, costs, A, B, xs, ws, maxX);
    ll C_base = computeBaseline(N, K, vals, costs, A, B, xs, ws, maxX);

    // score = 100 * min(2, C_base / C_you)
    // ratio for quitp in [0,1]:  ratio = score/200 = min(1, C_base / (2 * C_you))
    double ratio = 0.0;
    if (C_you <= 0) {
        ratio = 1.0;
    } else {
        double r = (double)C_base / (2.0 * (double)C_you);
        ratio = min(1.0, max(0.0, r));
    }

    quitp(ratio, "C_you=%lld C_base=%lld Ratio: %.9f", C_you, C_base, ratio);
    return 0;
}