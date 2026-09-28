#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---------- read problem input ----------
    int n = inf.readInt();
    int m = inf.readInt();
    long long B = inf.readLong();

    vector<long long> a(n);
    for (int i = 0; i < n; i++)
        a[i] = inf.readLong();

    vector<int> U(m), V(m);
    vector<long long> w(m);
    long long W = 0;
    for (int j = 0; j < m; j++) {
        U[j] = inf.readInt() - 1;
        V[j] = inf.readInt() - 1;
        w[j] = inf.readLong();
        W += w[j];
    }

    // ---------- read participant output ----------
    vector<long long> b(n);
    for (int i = 0; i < n; i++) {
        b[i] = ouf.readLong();
        if (b[i] < 1 || b[i] > 1000000000LL)
            quitf(_wa, "b[%d] = %lld is out of range [1, 10^9]", i + 1, b[i]);
    }
    // Ensure no trailing tokens
    if (!ouf.seekEof())
        quitf(_wa, "Extra tokens in participant output after reading %d values", n);

    // ---------- validate retagging cost ----------
    long long cost = 0;
    for (int i = 0; i < n; i++) {
        long long diff = llabs(a[i] - b[i]);
        // guard against overflow before adding
        if (diff > B || cost > B - diff) {
            quitf(_wa,
                  "Total retagging cost exceeds budget B "
                  "(partial cost already exceeds %lld)", B);
        }
        cost += diff;
    }

    // ---------- compute participant ordering ----------
    vector<int> ord(n);
    iota(ord.begin(), ord.end(), 0);
    sort(ord.begin(), ord.end(), [&](int x, int y) {
        if (b[x] != b[y]) return b[x] < b[y];
        return x < y;
    });
    vector<int> pos(n);
    for (int r = 0; r < n; r++) pos[ord[r]] = r;

    long long O = 0;
    for (int j = 0; j < m; j++)
        if (pos[U[j]] < pos[V[j]])
            O += w[j];

    // ---------- compute baseline O_base (b_i = a_i) ----------
    vector<int> base_ord(n);
    iota(base_ord.begin(), base_ord.end(), 0);
    sort(base_ord.begin(), base_ord.end(), [&](int x, int y) {
        if (a[x] != a[y]) return a[x] < a[y];
        return x < y;
    });
    vector<int> base_pos(n);
    for (int r = 0; r < n; r++) base_pos[base_ord[r]] = r;

    long long O_base = 0;
    for (int j = 0; j < m; j++)
        if (base_pos[U[j]] < base_pos[V[j]])
            O_base += w[j];

    // ---------- compute score ratio in [0,1] ----------
    // Statement formula:
    //   if O_base == W  -> score = 1,000,000  (ratio = 1.0)
    //   else            -> ratio = clamp((O - O_base)/(W - O_base), 0, 1)
    // quitp(ratio, ...) with ratio in [0,1] is the testlib convention;
    // the judge multiplies by 10^6 internally.
    double ratio;
    if (W == 0 || O_base == W) {
        // Either no requests at all, or baseline already satisfies everything.
        // In both cases the full score is awarded (ratio = 1.0).
        ratio = 1.0;
    } else {
        ratio = (double)(O - O_base) / (double)(W - O_base);
        if (ratio < 0.0) ratio = 0.0;
        if (ratio > 1.0) ratio = 1.0;
    }

    quitp(ratio,
          "Ratio: %.9f | O=%lld O_base=%lld W=%lld cost=%lld B=%lld",
          ratio, O, O_base, W, cost, B);

    return 0;
}