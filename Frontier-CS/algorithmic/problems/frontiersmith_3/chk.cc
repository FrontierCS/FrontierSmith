#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

typedef long long ll;
typedef __int128 lll;

bool isPerfectPower(ll v, int c, ll B) {
    if (v < 1 || v > B) return false;
    double est = pow((double)v, 1.0 / c);
    ll klo = max(1LL, (ll)(est - 2));
    ll khi = (ll)(est + 2);
    for (ll k = klo; k <= khi; k++) {
        lll pw = 1;
        bool overflow = false;
        for (int i = 0; i < c; i++) {
            pw *= k;
            if (pw > (lll)2e18) { overflow = true; break; }
        }
        if (!overflow && (ll)pw == v) return true;
    }
    return false;
}

ll mygcd(ll a, ll b) { while (b) { ll t = a % b; a = b; b = t; } return a; }

lll transformVal(ll xi, ll bj) {
    ll g = mygcd(xi, bj);
    lll lcmv = (lll)(xi / g) * bj;
    return lcmv / g;
}

lll computeObjective(int N, int K,
                     const vector<ll>& X, const vector<ll>& W,
                     const vector<ll>& bvals,
                     const vector<int>& asgn,
                     const vector<ll>& F) {
    lll total = 0;
    vector<bool> resUsed(K, false);
    for (int i = 0; i < N; i++) {
        int ai = asgn[i];
        if (ai == 0) {
            total += (lll)W[i] * X[i];
        } else {
            lll yval = transformVal(X[i], bvals[ai - 1]);
            total += (lll)W[i] * yval;
            resUsed[ai - 1] = true;
        }
    }
    for (int j = 0; j < K; j++) {
        if (resUsed[j]) total += (lll)F[j];
    }
    return total;
}

// Run the deterministic baseline described in the problem statement
lll runBaseline(int N, int K, ll B,
                const vector<ll>& X, const vector<ll>& W,
                const vector<int>& Cv, const vector<ll>& Uv, const vector<ll>& Fv) {
    vector<bool> used(N, false);
    vector<ll> bchosen(K, 1);
    vector<int> res(N, 0);

    for (int j = 0; j < K; j++) {
        int c = Cv[j];
        // Build list of perfect c-th powers in [1, B]
        vector<ll> Pj;
        for (ll base = 1; ; base++) {
            lll pw = 1;
            bool overflow = false;
            for (int e = 0; e < c; e++) {
                pw *= base;
                if (pw > (lll)B) { overflow = true; break; }
            }
            if (overflow) break;
            Pj.push_back((ll)pw);
        }

        // Step 2-3: find (i*, b*) with max gain among unassigned crystals
        ll bestGain = 0;
        int bestI = -1;
        ll bestB = 1;
        for (int i = 0; i < N; i++) {
            if (used[i]) continue;
            for (ll b : Pj) {
                lll yval = transformVal(X[i], b);
                ll gain = (ll)((lll)W[i] * X[i] - (lll)W[i] * yval);
                if (gain > bestGain ||
                    (gain == bestGain && bestI != -1 && (i < bestI || (i == bestI && b < bestB)))) {
                    bestGain = gain;
                    bestI = i;
                    bestB = b;
                }
            }
        }

        // Step 4: if max gain <= 0
        if (bestGain <= 0) {
            bchosen[j] = 1;
            continue;
        }

        // Step 5: b_j = b*
        ll bj = bestB;
        bchosen[j] = bj;

        // Step 6-7: collect unassigned crystals with positive gain for b_j
        vector<pair<ll, int>> positiveGains;
        for (int i = 0; i < N; i++) {
            if (used[i]) continue;
            lll yval = transformVal(X[i], bj);
            ll gain = (ll)((lll)W[i] * X[i] - (lll)W[i] * yval);
            if (gain > 0) {
                positiveGains.push_back({gain, i});
            }
        }
        // Sort by decreasing gain, tie-break smaller i
        sort(positiveGains.begin(), positiveGains.end(), [](const pair<ll,int>& a, const pair<ll,int>& b) {
            return a.first > b.first || (a.first == b.first && a.second < b.second);
        });

        // Step 8: take min(u_j, m)
        ll take = min((ll)positiveGains.size(), Uv[j]);
        ll S = 0;
        for (ll k = 0; k < take; k++) S += positiveGains[k].first;

        if (S > Fv[j]) {
            for (ll k = 0; k < take; k++) {
                int ci = positiveGains[k].second;
                used[ci] = true;
                res[ci] = j + 1;
            }
        } else {
            bchosen[j] = 1;
        }
    }

    return computeObjective(N, K, X, W, bchosen, res, Fv);
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // Read input
    int N = inf.readInt();
    int K = inf.readInt();
    ll B = inf.readLong();

    vector<ll> X(N), W(N);
    for (int i = 0; i < N; i++) {
        X[i] = inf.readLong();
        W[i] = inf.readLong();
    }
    vector<int> Cv(K);
    vector<ll> Uv(K), Fv(K);
    for (int j = 0; j < K; j++) {
        Cv[j] = inf.readInt();
        Uv[j] = inf.readLong();
        Fv[j] = inf.readLong();
    }

    // Read participant output: Line 1 - K tuning values
    vector<ll> bvals(K);
    for (int j = 0; j < K; j++) {
        bvals[j] = ouf.readLong();
        if (bvals[j] < 1 || bvals[j] > B) {
            quitf(_wa, "b[%d] = %lld is out of range [1, %lld]", j + 1, bvals[j], B);
        }
        if (!isPerfectPower(bvals[j], Cv[j], B)) {
            quitf(_wa, "b[%d] = %lld is not a perfect %d-th power", j + 1, bvals[j], Cv[j]);
        }
    }

    // Line 2: N assignments
    vector<int> asgn(N);
    for (int i = 0; i < N; i++) {
        asgn[i] = ouf.readInt();
        if (asgn[i] < 0 || asgn[i] > K) {
            quitf(_wa, "a[%d] = %d is out of range [0, %d]", i + 1, asgn[i], K);
        }
    }

    // Check for extra content (skip whitespace first)
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra content found after expected output");
    }

    // Check capacity constraints
    vector<int> resCount(K, 0);
    for (int i = 0; i < N; i++) {
        if (asgn[i] > 0) {
            int j = asgn[i] - 1;
            resCount[j]++;
            if (resCount[j] > (int)Uv[j]) {
                quitf(_wa, "Resonator %d exceeds capacity %lld (assigned %d crystals)", j + 1, Uv[j], resCount[j]);
            }
        }
    }

    // Compute C_out
    lll Cout = computeObjective(N, K, X, W, bvals, asgn, Fv);

    // Compute C_base
    lll Cbase = runBaseline(N, K, B, X, W, Cv, Uv, Fv);

    double dCout = (double)Cout;
    double dCbase = (double)Cbase;

    double ratio;
    if (Cbase <= 0) {
        // Degenerate case: baseline is zero or negative; give full score
        ratio = 1.0;
    } else if (Cout <= 0) {
        // Shouldn't happen (W_i >= 1, X_i >= 1, final value >= 1)
        ratio = 1.0;
    } else {
        // ratio = min(10, C_base / C_out) / 10, clamped to [0, 1]
        double r = dCbase / dCout;
        ratio = min(10.0, r) / 10.0;
        if (ratio < 0.0) ratio = 0.0;
        if (ratio > 1.0) ratio = 1.0;
    }

    quitp(ratio, "Feasible. C_out=%.0f, C_base=%.0f, C_base/C_out=%.6f, Ratio: %.6f",
          dCout, dCbase, (Cout > 0 ? dCbase / dCout : 999.0), ratio);

    return 0;
}