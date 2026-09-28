#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

typedef long long ll;
typedef __int128 lll;

static const ll CAP = (ll)1e18;

ll doSat(lll v) {
    if (v >= (lll)CAP) return CAP;
    if (v < 0LL) return 0LL;
    return (ll)v;
}

// Convert __int128 to string for messages
string fmt128(lll v) {
    if (v == 0) return "0";
    bool neg = (v < 0);
    if (neg) v = -v;
    string s;
    while (v > 0) { s += (char)('0' + (int)(v % 10)); v /= 10; }
    if (neg) s += '-';
    reverse(s.begin(), s.end());
    return s;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- Read input ----
    int n   = inf.readInt();
    ll  a   = inf.readLong();
    ll  B   = inf.readLong();

    vector<ll> p(n + 1), q(n + 1), u(n + 1);
    for (int i = 0; i <= n; i++) p[i] = inf.readLong();
    for (int i = 0; i <= n; i++) q[i] = inf.readLong();
    for (int i = 0; i <= n; i++) u[i] = inf.readLong();

    vector<ll> b(n + 1, 0), w(n + 1, 0);
    for (int t = 1; t <= n; t++) b[t] = inf.readLong();
    for (int t = 1; t <= n; t++) w[t] = inf.readLong();

    // Baseline error
    lll E_base = 0;
    for (int t = 1; t <= n; t++) E_base += (lll)w[t] * (lll)b[t];

    // ---- Read participant output ----
    int s = ouf.readInt(0, n + 1, "s");

    vector<ll> x(n + 1, 0LL);
    set<int>   seen;

    for (int k = 0; k < s; k++) {
        int idx = ouf.readInt(0, n, "chapter index i");
        ll  val = ouf.readLong();

        if (seen.count(idx))
            quitf(_wa, "Duplicate index %d in output", idx);
        seen.insert(idx);

        if (val < 1 || val > u[idx])
            quitf(_wa, "x[%d] = %lld is out of valid range [1, %lld]",
                  idx, val, u[idx]);

        x[idx] = val;
    }

    // Lenient EOF: skip trailing whitespace/newlines before checking end
    if (!ouf.seekEof())
        quitf(_wa, "Extra content after the expected output");

    // ---- Check budget ----
    lll cost = 0;
    for (int i = 0; i <= n; i++) {
        if (x[i] > 0) cost += (lll)p[i];
        cost += (lll)q[i] * (lll)x[i];
    }
    if (cost > (lll)B)
        quitf(_wa, "Budget exceeded: cost=%s > B=%lld",
              fmt128(cost).c_str(), B);

    // ---- Simulate ----
    vector<ll> g(n + 1);
    for (int i = 0; i <= n; i++) g[i] = x[i];

    vector<ll> ans(n + 1, 0LL);
    for (int t = 1; t <= n; t++) {
        for (int i = 1; i <= n; i++) {
            lll nv = (lll)g[i] + (lll)a * (lll)g[i - 1];
            g[i] = doSat(nv);
        }
        ans[t] = g[t];
    }

    // ---- Compute E ----
    lll E = 0;
    for (int t = 1; t <= n; t++) {
        lll diff = (lll)ans[t] - (lll)b[t];
        if (diff < 0) diff = -diff;
        E += (lll)w[t] * diff;
    }

    // ---- Score: ratio = max(0, 1 - E/E_base) continuously in [0,1] ----
    double ratio;
    if (E_base <= 0) {
        // Edge case: baseline is 0 (impossible per constraints, but be safe)
        ratio = (E == 0) ? 1.0 : 0.0;
    } else if (E >= E_base) {
        ratio = 0.0;
    } else {
        // Use 128-bit fraction: ratio = (E_base - E) / E_base
        lll num = E_base - E;
        // Convert carefully to double
        ratio = (double)num / (double)E_base;
        if (ratio < 0.0) ratio = 0.0;
        if (ratio > 1.0) ratio = 1.0;
    }

    quitp(ratio,
          "Ratio: %.9f | E=%s | E_base=%s | score=%.0f/1000000",
          ratio,
          fmt128(E).c_str(),
          fmt128(E_base).c_str(),
          ratio * 1e6);
}