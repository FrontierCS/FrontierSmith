#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ── read problem input ──────────────────────────────────────────────────
    int n = inf.readInt();
    int m = inf.readInt();

    vector<long long> a(n + 1);
    for (int i = 1; i <= n; i++)
        a[i] = inf.readLong();

    struct Req { int l, r, p, t; long long w; };
    vector<Req> reqs(m);
    for (int i = 0; i < m; i++) {
        reqs[i].l = inf.readInt();
        reqs[i].r = inf.readInt();
        reqs[i].p = inf.readInt();
        reqs[i].t = inf.readInt();
        reqs[i].w = inf.readLong();
    }

    int len = 2 * n - 1;   // length of the final sequence b

    // ── build baseline sequence (xi = ai) and its prefix-XOR ────────────────
    vector<long long> b_base(len + 1, 0LL);
    for (int i = 1; i <= n; i++) {
        b_base[2 * i - 1] = a[i];
        if (i < n) b_base[2 * i] = a[i];   // xi_base = a[i]
    }

    vector<long long> px_base(len + 1, 0LL);
    for (int i = 1; i <= len; i++)
        px_base[i] = px_base[i - 1] ^ b_base[i];

    long long W = 0, V_base = 0;
    for (int i = 0; i < m; i++) {
        W += reqs[i].w;
        long long xv = px_base[reqs[i].r] ^ px_base[reqs[i].l - 1];
        int bit = (int)((xv >> reqs[i].p) & 1LL);
        if (bit == reqs[i].t) V_base += reqs[i].w;
    }

    // ── read participant output ─────────────────────────────────────────────
    vector<long long> x(n + 1, 0LL);   // x[1..n-1]
    for (int i = 1; i < n; i++)
        x[i] = ouf.readLong();

    // require end of file (no trailing garbage)
    if (!ouf.seekEof())
        quitf(_wa, "Extra tokens found after the %d relay values", n - 1);

    // ── feasibility check ───────────────────────────────────────────────────
    for (int i = 1; i < n; i++) {
        if (x[i] < a[i] || x[i] > a[i + 1])
            quitf(_wa,
                  "x[%d] = %lld is out of range [%lld, %lld]",
                  i, x[i], a[i], a[i + 1]);
    }

    // ── build participant sequence and its prefix-XOR ────────────────────────
    vector<long long> b(len + 1, 0LL);
    for (int i = 1; i <= n; i++) {
        b[2 * i - 1] = a[i];
        if (i < n) b[2 * i] = x[i];
    }

    vector<long long> px(len + 1, 0LL);
    for (int i = 1; i <= len; i++)
        px[i] = px[i - 1] ^ b[i];

    // ── evaluate objective V ─────────────────────────────────────────────────
    long long V = 0;
    for (int i = 0; i < m; i++) {
        long long xv = px[reqs[i].r] ^ px[reqs[i].l - 1];
        int bit = (int)((xv >> reqs[i].p) & 1LL);
        if (bit == reqs[i].t) V += reqs[i].w;
    }

    // ── scoring formula from the problem statement ──────────────────────────
    double ratio;
    if (W == V_base) {
        // degenerate case: baseline already achieves maximum
        ratio = (V == W) ? 1.0 : 0.0;
    } else {
        double z = (double)(V - V_base) / (double)(W - V_base);
        if (z < 0.0) z = 0.0;
        if (z > 1.0) z = 1.0;
        ratio = z;
    }

    quitp(ratio,
          "Ratio: %.10f | V=%lld V_base=%lld W=%lld",
          ratio, V, V_base, W);

    return 0;
}