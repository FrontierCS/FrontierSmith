#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    int t = inf.readInt();

    double total_score = 0.0;
    int total_cases = 0;

    for (int tc = 0; tc < t; tc++) {
        int n = inf.readInt();
        int m = inf.readInt();

        vector<long long> a(n + 1), b(n + 1);
        for (int i = 1; i <= n; i++) a[i] = inf.readLong();
        for (int i = 1; i <= n; i++) b[i] = inf.readLong();

        vector<int> eu(m), ev(m), ew(m);
        for (int i = 0; i < m; i++) {
            eu[i] = inf.readInt();
            ev[i] = inf.readInt();
            ew[i] = inf.readInt();
        }

        total_cases++;

        // ---- Read participant permutation ----
        // We attempt to read n integers; any format/range error => score 0 for this case.
        vector<int> p(n + 1, 0);
        bool feasible = true;
        string err_msg;

        for (int i = 1; i <= n; i++) {
            if (ouf.seekEof()) {
                feasible = false;
                err_msg = format("Test case %d: output ended prematurely at p_%d", tc + 1, i);
                break;
            }
            int v = ouf.readInt();
            if (v < 1 || v > n) {
                feasible = false;
                err_msg = format("Test case %d: p_%d = %d out of range [1, %d]", tc + 1, i, v, n);
                break;
            }
            p[i] = v;
        }

        if (feasible) {
            // Check permutation (no duplicates)
            vector<bool> used(n + 1, false);
            for (int i = 1; i <= n; i++) {
                if (used[p[i]]) {
                    feasible = false;
                    err_msg = format("Test case %d: module %d assigned more than once", tc + 1, p[i]);
                    break;
                }
                used[p[i]] = true;
            }
        }

        if (!feasible) {
            // Skip rest of this test case's output tokens (we already read what we could)
            // Score for this case is 0; continue to next test case.
            total_score += 0.0;
            continue;
        }

        // ---- Compute participant cost C ----
        long long C = 0;
        for (int i = 1; i <= n; i++) {
            C += llabs(a[i] - b[p[i]]);
        }
        for (int i = 0; i < m; i++) {
            C += (long long)ew[i] * llabs(b[p[eu[i]]] - b[p[ev[i]]]);
        }

        // ---- Compute baseline assignment ----
        // Sort panel indices by (a_i, i) nondecreasing
        vector<int> panel_order(n);
        iota(panel_order.begin(), panel_order.end(), 1);
        sort(panel_order.begin(), panel_order.end(), [&](int x, int y) {
            if (a[x] != a[y]) return a[x] < a[y];
            return x < y;
        });

        // Sort module indices by (b_j, j) nondecreasing
        vector<int> module_order(n);
        iota(module_order.begin(), module_order.end(), 1);
        sort(module_order.begin(), module_order.end(), [&](int x, int y) {
            if (b[x] != b[y]) return b[x] < b[y];
            return x < y;
        });

        // k-th module in sorted module list -> k-th panel in sorted panel list
        vector<int> base_p(n + 1);
        for (int k = 0; k < n; k++) {
            base_p[panel_order[k]] = module_order[k];
        }

        // ---- Compute baseline cost B ----
        long long B = 0;
        for (int i = 1; i <= n; i++) {
            B += llabs(a[i] - b[base_p[i]]);
        }
        for (int i = 0; i < m; i++) {
            B += (long long)ew[i] * llabs(b[base_p[eu[i]]] - b[base_p[ev[i]]]);
        }

        // ---- Per-test score: min(1.0, 0.5 * (B+1) / (C+1)) ----
        // This gives 0.5 when C == B, approaches 1.0 when C << B,
        // and is less than 0.5 when C > B (continuous, no saturation to {0,1}).
        double ratio = 0.5 * (double)(B + 1) / (double)(C + 1);
        if (ratio > 1.0) ratio = 1.0;
        if (ratio < 0.0) ratio = 0.0;

        total_score += ratio;
    }

    // ---- Ensure no trailing garbage in participant output ----
    if (!ouf.seekEof()) {
        // There is extra output; penalize by not adjusting scores — they've
        // already been accumulated. We still call quitp so the judge gets
        // the partial score, but we note the extra output.
        double final_ratio = (total_cases > 0) ? total_score / (double)total_cases : 0.0;
        if (final_ratio < 0.0) final_ratio = 0.0;
        if (final_ratio > 1.0) final_ratio = 1.0;
        quitp(final_ratio, "Ratio: %.9f (mean over %d test cases; trailing garbage in output)", final_ratio, total_cases);
    }

    double final_ratio = (total_cases > 0) ? total_score / (double)total_cases : 0.0;
    if (final_ratio < 0.0) final_ratio = 0.0;
    if (final_ratio > 1.0) final_ratio = 1.0;

    quitp(final_ratio, "Ratio: %.9f (mean over %d test cases)", final_ratio, total_cases);
}