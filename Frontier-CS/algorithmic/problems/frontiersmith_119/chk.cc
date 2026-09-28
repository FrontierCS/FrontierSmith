#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

// Merge-sort based inversion count (works on a copy)
long long countInversions(vector<int> arr) {
    int n = (int)arr.size();
    if (n <= 1) return 0LL;
    vector<int> tmp(n);
    long long inv = 0;
    function<void(int,int)> ms = [&](int l, int r) {
        if (r - l <= 1) return;
        int m = (l + r) / 2;
        ms(l, m);
        ms(m, r);
        int i = l, j = m, k = l;
        while (i < m && j < r) {
            if (arr[i] <= arr[j]) tmp[k++] = arr[i++];
            else { inv += (long long)(m - i); tmp[k++] = arr[j++]; }
        }
        while (i < m) tmp[k++] = arr[i++];
        while (j < r) tmp[k++] = arr[j++];
        for (int x = l; x < r; x++) arr[x] = tmp[x];
    };
    ms(0, n);
    return inv;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    int t = inf.readInt();

    // Accumulate floored per-test scores (each in [0, 2000000])
    long long totalScore = 0LL;
    int numCases = t;

    for (int tc = 0; tc < t; tc++) {
        // --- Read this test case from input ---
        int n = inf.readInt();
        int K = inf.readInt();
        int B = inf.readInt();
        vector<int> a(n);
        for (int i = 0; i < n; i++) a[i] = inf.readInt();

        // --- Simulate baseline ---
        vector<int> base = a;
        int remK = K, remB = B;
        while (remK > 0 && remB >= 2) {
            int p = -1;
            for (int i = 0; i + 1 < n; i++) {
                if (base[i] > base[i+1]) { p = i; break; }
            }
            if (p == -1) break;
            int q = p;
            while (q + 1 < n && base[q] > base[q+1]) q++;
            int len = min(q - p + 1, remB);
            if (len < 2) break;
            sort(base.begin() + p, base.begin() + p + len);
            remB -= len;
            remK--;
        }
        long long I_base = countInversions(base);

        // --- Read participant output for this test case ---
        int ops_count = ouf.readInt();
        if (ops_count < 0 || ops_count > K) {
            quitf(_wa, "Test case %d: q=%d out of range [0,%d]", tc+1, ops_count, K);
        }

        vector<pair<int,int>> segs(ops_count);
        long long totalCost = 0;
        for (int i = 0; i < ops_count; i++) {
            int l = ouf.readInt();
            int r = ouf.readInt();
            if (l < 1 || l > n) {
                quitf(_wa, "Test case %d op %d: l=%d out of [1,%d]", tc+1, i+1, l, n);
            }
            if (r < l || r > n) {
                quitf(_wa, "Test case %d op %d: r=%d invalid (l=%d, n=%d)", tc+1, i+1, r, l, n);
            }
            segs[i] = {l, r};
            totalCost += (long long)(r - l + 1);
        }
        if (totalCost > (long long)B) {
            quitf(_wa, "Test case %d: total cost %lld exceeds budget B=%d", tc+1, totalCost, B);
        }

        // --- Simulate participant's operations ---
        vector<int> sub = a;
        for (auto& seg : segs) {
            int l = seg.first - 1;   // 0-indexed inclusive
            int r = seg.second;      // 0-indexed exclusive
            sort(sub.begin() + l, sub.begin() + r);
        }
        long long I_sub = countInversions(sub);

        // --- Compute per-test score (statement formula) ---
        // score_i = floor(10^6 * min(2, (I_base+1)/(I_sub+1)))
        // This is in [0, 2000000].
        double factor = min(2.0, (double)(I_base + 1) / (double)(I_sub + 1));
        long long caseScore = (long long)(1000000.0 * factor); // floor via truncation
        totalScore += caseScore;
    }

    // --- Strict EOF check: no extra tokens allowed ---
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra output after all test cases");
    }

    // Normalize to [0,1]: max possible total is numCases * 2000000
    double ratio = (numCases > 0)
        ? (double)totalScore / ((double)numCases * 2000000.0)
        : 0.0;
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    // Report: mean per-test score = totalScore/numCases (in [0,2e6] scale)
    double meanScore = (numCases > 0) ? (double)totalScore / (double)numCases : 0.0;
    quitp(ratio, "Ratio: %f (mean per-test score %.2f / 2000000, over %d cases)",
          ratio, meanScore, numCases);
}