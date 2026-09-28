#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

// BIT for counting inversions
struct BIT {
    int sz;
    vector<long long> tree;
    BIT(int n): sz(n), tree(n+1, 0) {}
    void update(int i, long long v) { for(; i<=sz; i+=i&(-i)) tree[i]+=v; }
    long long query(int i) { long long s=0; for(; i>0; i-=i&(-i)) s+=tree[i]; return s; }
    long long query(int l, int r) { return l>r?0:query(r)-(l>1?query(l-1):0); }
};

long long countInversions(const vector<int>& a, int V) {
    int n = (int)a.size();
    BIT bit(V);
    long long inv = 0;
    for (int i = n-1; i >= 0; i--) {
        if (a[i] > 1) inv += bit.query(1, a[i]-1);
        bit.update(a[i], 1);
    }
    return inv;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- read input ----
    int n = inf.readInt();
    int m = inf.readInt();
    int K = inf.readInt();
    int V = inf.readInt();

    vector<int> a0(n);
    for (int i = 0; i < n; i++) a0[i] = inf.readInt();

    struct Event { char type; int l, r, x, w; };
    vector<Event> events(m);
    for (int i = 0; i < m; i++) {
        string s = inf.readToken();
        if (s == "U") {
            events[i].type = 'U';
            events[i].l = inf.readInt() - 1;
            events[i].r = inf.readInt() - 1;
            events[i].x = inf.readInt();
        } else {
            events[i].type = 'Q';
            events[i].w = inf.readInt();
        }
    }

    // ---- compute baseline penalty ----
    long long baseline = 0;
    {
        vector<int> a = a0;
        for (int t = 0; t < m; t++) {
            if (events[t].type == 'U') {
                int l = events[t].l, r = events[t].r, x = events[t].x;
                for (int i = l; i <= r; i++) a[i] = x + (i - l);
            } else {
                long long inv = countInversions(a, V);
                baseline += (long long)events[t].w * inv;
            }
        }
    }

    // ---- read participant output ----
    int k = ouf.readInt();
    if (k < 0 || k > K)
        quitf(_wa, "k=%d is out of range [0,%d]", k, K);

    struct Repair { int t, l, r, x; };
    vector<Repair> repairs(k);
    for (int i = 0; i < k; i++) {
        int t = ouf.readInt();
        int l = ouf.readInt();
        int r = ouf.readInt();
        int x = ouf.readInt();
        if (t < 1 || t > m)
            quitf(_wa, "repair %d: t=%d not in [1,%m]", i+1, t, m);
        if (l < 1 || r < l || r > n)
            quitf(_wa, "repair %d: l=%d r=%d invalid for n=%d", i+1, l, r, n);
        if (x < 1 || x + (r-l) > V)
            quitf(_wa, "repair %d: x=%d with r-l=%d violates [1,V]=%d bound", i+1, x, r-l, V);
        repairs[i] = {t, l-1, r-1, x};
    }

    // check end of output
    if (!ouf.seekEof())
        quitf(_wa, "extra tokens in output after %d repairs", k);

    // ---- simulate participant plan ----
    long long participant = 0;
    {
        vector<int> a = a0;

        // group repairs by time (1-indexed)
        vector<vector<int>> repairAt(m+1);
        for (int i = 0; i < k; i++) repairAt[repairs[i].t].push_back(i);

        for (int t = 1; t <= m; t++) {
            // apply repairs before event t
            for (int ri : repairAt[t]) {
                int l = repairs[ri].l, r = repairs[ri].r, x = repairs[ri].x;
                for (int i = l; i <= r; i++) a[i] = x + (i - l);
            }
            // apply mandatory event t (0-indexed internally)
            Event& ev = events[t-1];
            if (ev.type == 'U') {
                int l = ev.l, r = ev.r, x = ev.x;
                for (int i = l; i <= r; i++) a[i] = x + (i - l);
            } else {
                long long inv = countInversions(a, V);
                participant += (long long)ev.w * inv;
            }
        }
    }

    // ---- compute score ----
    double score;
    if (baseline == 0) {
        score = (participant == 0) ? 1.0 : 0.0;
    } else {
        double improvement = (double)(baseline - participant) / (double)baseline;
        score = max(0.0, min(1.0, improvement));
    }

    quitp(score, "Ratio: %.9f | baseline=%lld participant=%lld", score, baseline, participant);
}