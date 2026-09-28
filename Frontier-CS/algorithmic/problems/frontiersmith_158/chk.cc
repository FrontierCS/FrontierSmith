#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

// Fenwick tree storing (count, sum) of urgency values — indexed 1..n
struct BIT {
    int sz;
    vector<long long> cnt, sm;
    BIT() : sz(0) {}
    BIT(int n) : sz(n), cnt(n+1, 0LL), sm(n+1, 0LL) {}
    void update(int i, long long v) {
        for (int x = i; x <= sz; x += x & -x) { cnt[x]++; sm[x] += v; }
    }
    // returns {count, sum} of values in [1..i]
    pair<long long,long long> query(int i) {
        long long c = 0, s = 0;
        for (int x = i; x > 0; x -= x & -x) { c += cnt[x]; s += sm[x]; }
        return {c, s};
    }
};

// Compute total cost for a given assignment (1-indexed arrays)
long long computeCost(int n, int k, long long R,
                      const vector<int>& x,   // x[1..n], CPU ids in [1..k]
                      const vector<int>& a,   // a[1..n], urgency permutation
                      const vector<tuple<int,int,long long>>& pairs)
{
    long long cost = 0;

    // 1. Cold-start cost: count runs
    long long runs = 1;
    for (int i = 2; i <= n; i++)
        if (x[i] != x[i-1]) runs++;
    cost += R * runs;

    // 2. Transfer cost
    for (auto& [u, v, w] : pairs)
        if (x[u] != x[v]) cost += w;

    // 3. Urgency regret cost using BIT per CPU
    // For each CPU c, maintain (count, sum) of urgency values seen so far.
    // For each new task j on CPU c: regret += a[j]*count(a[i]<a[j]) - sum(a[i] for a[i]<a[j])
    vector<BIT> bits(k+1, BIT(n));
    for (int i = 1; i <= n; i++) {
        int c = x[i];
        int val = a[i];
        if (val > 1) {
            auto [cnt, sm] = bits[c].query(val - 1);
            cost += (long long)val * cnt - sm;
        }
        bits[c].update(val, (long long)val);
    }

    return cost;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // Read problem input (1-indexed)
    int n = inf.readInt();
    int k = inf.readInt();
    int m = inf.readInt();
    long long R = inf.readLong();

    vector<int> a(n+1);
    for (int i = 1; i <= n; i++)
        a[i] = inf.readInt();

    vector<tuple<int,int,long long>> pairs(m);
    for (int i = 0; i < m; i++) {
        int u = inf.readInt();
        int v = inf.readInt();
        long long w = inf.readLong();
        pairs[i] = {u, v, w};
    }

    // Read participant output — exactly n integers, then strict EOF check
    vector<int> x(n+1);
    for (int i = 1; i <= n; i++) {
        x[i] = ouf.readInt();
        if (x[i] < 1 || x[i] > k)
            quitf(_wa, "task %d assigned to CPU %d which is out of range [1,%d]", i, x[i], k);
    }

    // Strict end-of-output check: reject trailing non-whitespace tokens
    if (!ouf.seekEof())
        quitf(_wa, "extra output found after the %d required integers", n);

    // Compute participant's cost
    long long C_you = computeCost(n, k, R, x, a, pairs);

    // Compute baseline assignment:
    // q = floor(n/k), r = n mod k
    // first r blocks have size q+1, remaining k-r blocks have size q
    int q = n / k, r_mod = n % k;
    vector<int> xbase(n+1);
    {
        int pos = 1;
        for (int cpu = 1; cpu <= k; cpu++) {
            int blk = (cpu <= r_mod) ? (q + 1) : q;
            for (int j = 0; j < blk; j++)
                xbase[pos++] = cpu;
        }
    }
    long long C_base = computeCost(n, k, R, xbase, a, pairs);

    // Per-test score formula (from statement):
    //   score = round(1,000,000 * clamp(C_base / C_you, 0, 2))
    // where clamp(t, 0, 2) = min(2, max(0, t)), and C_you=0 => clamp=2.
    //
    // quitp(ratio, ...) where ratio in [0,1] and full score = 2,000,000 points.
    // So: ratio = clamp(C_base / C_you, 0, 2) / 2
    //
    // Baseline (C_you == C_base) => clamp=1 => ratio=0.5 => 1,000,000 pts.  Correct.
    // Perfect   (C_you == 0)     => clamp=2 => ratio=1.0 => 2,000,000 pts.  Correct.

    double ratio;
    if (C_you == 0LL) {
        // Perfect solution: clamp value = 2, ratio = 1.0
        ratio = 1.0;
    } else {
        double t = (double)C_base / (double)C_you;
        // clamp t to [0, 2], then normalize to [0, 1]
        if (t > 2.0) t = 2.0;
        if (t < 0.0) t = 0.0;
        ratio = t / 2.0;
    }

    // Safety clamp to [0, 1]
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    // Required tag: "Ratio: <value>" where value is the ratio in [0,1] passed to quitp
    quitp(ratio,
          "Ratio: %.9f | C_you=%lld C_base=%lld",
          ratio, C_you, C_base);

    return 0;
}