#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    int t = inf.readInt(1, 50, "t");

    double total_score = 0.0;
    int num_tests = 0;

    for (int tc = 0; tc < t; tc++) {
        // Read problem input for this test case from inf
        int n  = inf.readInt(1, 250, "n");
        int k  = inf.readInt(1, 120, "k");
        long long U = inf.readLong((long long)k, (long long)1000000LL, "U");
        long long S = inf.readLong(1LL, (long long)1e12, "S");

        vector<long long> A(n);
        for (int i = 0; i < n; i++)
            A[i] = inf.readLong(1LL, (long long)1000000LL, "a_i");

        // Compute baseline Cover(B0) with B0 = {1, 2, ..., k}
        set<long long> baseline_cover;
        for (int b = 1; b <= k; b++)
            for (int i = 0; i < n; i++)
                baseline_cover.insert(A[i] + (long long)b);
        long long O0 = (long long)baseline_cover.size();

        // Read participant's answer for this test case
        // Expect "YES" first (case-insensitive)
        string yes_word = ouf.readWord();
        bool feasible = true;

        // Normalise to uppercase for comparison
        string yes_upper = yes_word;
        for (char& c : yes_upper) c = (char)toupper((unsigned char)c);

        if (yes_upper != "YES") {
            // Not "YES": this test case is infeasible, score 0
            // Try to read k numbers to stay in sync with ouf, but if we can't, just give 0
            // We cannot safely re-sync, so record 0 and continue
            total_score += 0.0;
            num_tests++;
            continue;
        }

        // Read k shifts
        vector<long long> B(k);
        set<long long> seen;
        long long sum_b = 0;

        for (int i = 0; i < k; i++) {
            if (ouf.eof()) {
                feasible = false;
                // Fill rest with dummy values
                for (int j = i; j < k; j++) B[j] = 0;
                break;
            }
            B[i] = ouf.readLong(-2000000000LL, 2000000000LL);
            if (B[i] < 1LL || B[i] > U) {
                feasible = false;
            }
            if (seen.count(B[i])) {
                feasible = false;
            }
            seen.insert(B[i]);
            sum_b += B[i];
        }

        if (sum_b > S) {
            feasible = false;
        }

        double per_test_score = 0.0;

        if (feasible && O0 > 0) {
            // Compute Cover(B)
            set<long long> cover;
            for (int j = 0; j < k; j++)
                for (int i = 0; i < n; i++)
                    cover.insert(A[i] + B[j]);
            long long O = (long long)cover.size();

            // score = clamp(100 * O / O0, 0, 1000) per the problem statement
            double raw = 100.0 * (double)O / (double)O0;
            per_test_score = max(0.0, min(1000.0, raw));
        } else if (O0 == 0) {
            // degenerate: no baseline coverage, give full score if feasible
            per_test_score = feasible ? 100.0 : 0.0;
        }
        // else infeasible: per_test_score remains 0.0

        total_score += per_test_score;
        num_tests++;
    }

    // Compute mean score (each test's score is in [0, 1000])
    double mean_score = (num_tests > 0) ? (total_score / (double)num_tests) : 0.0;

    // Convert to ratio in [0, 1] for quitp
    // mean_score is in [0, 1000]; baseline gives mean_score=100 => ratio=0.1
    // We want: ratio = mean_score / 1000.0, clamped to [0, 1]
    double ratio = mean_score / 1000.0;
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    quitp(ratio, "Ratio: %.9f (mean per-test score: %.4f / 1000, over %d test(s))",
          ratio, mean_score, num_tests);

    return 0;
}