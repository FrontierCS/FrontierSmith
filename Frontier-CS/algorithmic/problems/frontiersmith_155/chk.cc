#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

// Merge-sort based inversion count; modifies arr in place
long long mergeCount(vector<long long>& arr, int l, int r) {
    if (r - l <= 1) return 0LL;
    int m = (l + r) / 2;
    long long cnt = mergeCount(arr, l, m) + mergeCount(arr, m, r);
    vector<long long> tmp;
    tmp.reserve(r - l);
    int i = l, j = m;
    while (i < m && j < r) {
        if (arr[i] <= arr[j]) {
            tmp.push_back(arr[i++]);
        } else {
            cnt += (long long)(m - i);
            tmp.push_back(arr[j++]);
        }
    }
    while (i < m) tmp.push_back(arr[i++]);
    while (j < r) tmp.push_back(arr[j++]);
    for (int p = l; p < r; p++) arr[p] = tmp[p - l];
    return cnt;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- Read problem input from inf ----
    int n = inf.readInt();
    int k = inf.readInt();
    int B = inf.readInt();

    long long lam = inf.readLong();   // lambda
    long long mu  = inf.readLong();
    long long nu  = inf.readLong();

    vector<long long> a(n);
    for (int i = 0; i < n; i++) a[i] = inf.readLong();

    // ---- Baseline cost ----
    // q=1, x_1=0, c_i=1 for all i => b = a unchanged
    // sw=0, comp=popcount(0)=0, q=1
    // C_base = inv(a) + nu*1
    vector<long long> b_base = a;
    long long inv_base = mergeCount(b_base, 0, n);
    long long C_base = inv_base + nu;   // nu * 1 (q=1, comp=0, sw=0)

    // ---- Read participant output from ouf ----
    int q = ouf.readInt();
    if (q < 1 || q > k) {
        quitf(_wa, "q=%d is out of valid range [1,%d]", q, k);
    }

    // 2^B upper bound for masks
    long long mask_max;
    if (B >= 63) {
        mask_max = (long long)9e18;
    } else {
        mask_max = 1LL << B;
    }

    vector<long long> x(q);
    for (int t = 0; t < q; t++) {
        x[t] = ouf.readLong();
        if (x[t] < 0 || x[t] >= mask_max) {
            quitf(_wa, "x[%d]=%lld is out of valid range [0, 2^%d)", t + 1, x[t], B);
        }
    }

    vector<int> c(n);
    for (int i = 0; i < n; i++) {
        c[i] = ouf.readInt();
        if (c[i] < 1 || c[i] > q) {
            quitf(_wa, "c[%d]=%d is out of valid range [1,%d]", i + 1, c[i], q);
        }
    }

    // Assert no trailing content in participant output
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra content found after the required output");
    }

    // ---- Compute decoded array b ----
    vector<long long> b(n);
    for (int i = 0; i < n; i++) b[i] = a[i] ^ x[c[i] - 1];

    // ---- inv(b) ----
    vector<long long> b_copy = b;
    long long inv_b = mergeCount(b_copy, 0, n);

    // ---- sw(c) ----
    long long sw = 0;
    for (int i = 1; i < n; i++) {
        if (c[i] != c[i - 1]) sw++;
    }

    // ---- comp(x) ----
    long long comp = 0;
    for (int t = 0; t < q; t++) comp += (long long)__builtin_popcountll((unsigned long long)x[t]);

    // ---- Total cost of submitted solution ----
    long long C_sub = inv_b
                    + lam * sw
                    + mu  * comp
                    + nu  * (long long)q;

    // ---- Score as stated in the problem ----
    // score = floor(10^6 * min(2, (C_base+1)/(C_sub+1)))
    // Mapped to [0,1]: ratio = score / 2,000,000 = min(2, (C_base+1)/(C_sub+1)) / 2
    double improvement = (double)(C_base + 1) / (double)(C_sub + 1);
    if (improvement > 2.0) improvement = 2.0;
    if (improvement < 0.0) improvement = 0.0;

    // raw_score as stated: floor(10^6 * min(2, (C_base+1)/(C_sub+1)))
    long long raw_score = (long long)(1000000.0 * improvement);  // floor via truncation

    // Ratio in [0,1]: raw_score / 2,000,000  (since max raw_score = 2,000,000)
    double score_ratio = (double)raw_score / 2000000.0;
    if (score_ratio < 0.0) score_ratio = 0.0;
    if (score_ratio > 1.0) score_ratio = 1.0;

    // The Ratio: tag value MUST equal the first argument to quitp
    quitp(score_ratio,
          "Ratio: %.9f | raw_score=%lld C_sub=%lld C_base=%lld "
          "inv_b=%lld sw=%lld comp=%lld q=%d",
          score_ratio, raw_score, C_sub, C_base, inv_b, sw, comp, q);

    return 0;
}