#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- read input ----
    int n = inf.readInt();
    long long G = inf.readLong();

    vector<long long> a(n+1), x(n+1), y(n+1), b(n+1), t(n+1);
    long long S = 0;
    for (int i = 1; i <= n; i++) {
        a[i] = inf.readLong();
        x[i] = inf.readLong();
        y[i] = inf.readLong();
        b[i] = inf.readLong();
        t[i] = inf.readLong();
        S += a[i];
    }

    // ---- compute baseline B using the circular pairing described in the statement ----
    // prefix[i] = sum of a[1..i]
    vector<long long> prefix(n+2, 0);
    for (int i = 1; i <= n; i++) prefix[i] = prefix[i-1] + a[i];

    long long half = S / 2;

    // Collect all breakpoints in [0, half] from both the left half [0,half)
    // and the right half [half, S) shifted back by half.
    vector<long long> bpts;
    bpts.push_back(0);
    bpts.push_back(half);
    for (int i = 0; i <= n; i++) {
        if (prefix[i] > 0 && prefix[i] < half)
            bpts.push_back(prefix[i]);
        long long rp = prefix[i] - half;
        if (rp > 0 && rp < half)
            bpts.push_back(rp);
    }
    sort(bpts.begin(), bpts.end());
    bpts.erase(unique(bpts.begin(), bpts.end()), bpts.end());

    // For a global position pos in [0,S), find 1-indexed reactor label
    auto getLabel = [&](long long pos) -> int {
        // label l such that prefix[l-1] <= pos < prefix[l]
        int lo = 1, hi = n, ans = n;
        while (lo <= hi) {
            int mid = (lo + hi) / 2;
            if (prefix[mid] > pos) { ans = mid; hi = mid - 1; }
            else lo = mid + 1;
        }
        return ans;
    };

    map<pair<int,int>, long long> pairCount;
    for (int idx = 0; idx + 1 < (int)bpts.size(); idx++) {
        long long p = bpts[idx];
        long long len = bpts[idx+1] - p;
        if (len <= 0) continue;
        // left position p, right position p+half
        int lbl_left  = getLabel(p);
        int lbl_right = getLabel(p + half);
        if (lbl_left == lbl_right)
            quitf(_fail, "Baseline pairs reactor with itself (reactor %d)", lbl_left);
        int u = min(lbl_left, lbl_right);
        int v = max(lbl_left, lbl_right);
        pairCount[{u, v}] += len;
    }

    // Compute baseline cost B
    long long B = 0;
    for (auto& [p, cnt] : pairCount) {
        int u = p.first, v = p.second;
        long long cal = b[u] + b[v] + (t[u] != t[v] ? G : 0LL);
        long long dist = abs(x[u] - x[v]) + abs(y[u] - y[v]);
        B += cal + dist * cnt;
    }

    // ---- read participant output ----
    int m = ouf.readInt(0, 3000000, "m");

    vector<long long> used(n+1, 0);
    map<pair<int,int>, long long> links;

    for (int q = 0; q < m; q++) {
        int u = ouf.readInt(1, n, "i");
        int v = ouf.readInt(1, n, "j");
        long long k = ouf.readLong();

        if (u >= v)
            quitf(_wa, "Link %d: require i < j, got i=%d j=%d", q+1, u, v);
        if (k <= 0)
            quitf(_wa, "Link %d: k must be positive, got %lld", q+1, k);
        pair<int,int> key = {u, v};
        if (links.count(key))
            quitf(_wa, "Duplicate link (%d,%d)", u, v);
        links[key] = k;
        used[u] += k;
        used[v] += k;
    }

    // Check for trailing garbage (lenient: skip whitespace before EOF check)
    if (!ouf.seekEof())
        quitf(_wa, "Extra output after last link");

    // Check feasibility: every reactor must be exactly neutralized
    for (int i = 1; i <= n; i++) {
        if (used[i] != a[i])
            quitf(_wa, "Reactor %d: expected total usage %lld, got %lld", i, a[i], used[i]);
    }

    // Compute participant cost C
    long long C = 0;
    for (auto& [p, k] : links) {
        int u = p.first, v = p.second;
        long long cal = b[u] + b[v] + (t[u] != t[v] ? G : 0LL);
        long long dist = abs(x[u] - x[v]) + abs(y[u] - y[v]);
        C += cal + dist * k;
    }

    if (C <= 0) {
        // Should not happen for valid instances, but guard anyway
        quitf(_wa, "Participant cost is non-positive: %lld", C);
    }

    // Score formula from statement: 10^6 * min(2, B/C)
    // Mapped to [0,1] for quitp: ratio = min(1, B / (2*C))
    // This gives continuous partial credit:
    //   C == B  => ratio = 0.5 (matches baseline, scores 1e6)
    //   C < B   => ratio > 0.5 (beats baseline, up to 1.0 = 2e6)
    //   C > B   => ratio < 0.5 (worse than baseline, partial credit)
    double ratio = (double)B / ((double)C * 2.0);
    if (ratio > 1.0) ratio = 1.0;
    if (ratio < 0.0) ratio = 0.0;

    quitp(ratio, "Feasible. C=%lld B=%lld Ratio: %.6f", C, B, ratio);
}