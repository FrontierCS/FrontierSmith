#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    int t = inf.readInt();

    double total_score = 0.0;
    int num_cases = t;

    for (int tc = 0; tc < t; tc++) {
        int m = inf.readInt();
        int n = inf.readInt();

        vector<long long> h(n - 1), v(n);
        for (int i = 0; i < n - 1; i++) h[i] = inf.readLong();
        for (int i = 0; i < n;     i++) v[i] = inf.readLong();

        vector<string> s(m);
        for (int i = 0; i < m; i++) s[i] = inf.readToken();

        // Compute objective for an arbitrary row arrangement
        auto compute_obj = [&](const vector<string>& rows) -> long long {
            long long obj = 0;
            for (int r = 0; r < m; r++)
                for (int c = 0; c < n - 1; c++)
                    if (rows[r][c] != rows[r][c + 1]) obj += h[c];
            for (int r = 0; r < m - 1; r++)
                for (int c = 0; c < n; c++)
                    if (rows[r][c] != rows[r + 1][c]) obj += v[c];
            return obj;
        };

        // Baseline: original order, no reversals
        long long base_obj = compute_obj(s);

        // Trivial upper bound
        long long sum_h = 0, sum_v = 0;
        for (int c = 0; c < n - 1; c++) sum_h += h[c];
        for (int c = 0; c < n;     c++) sum_v += v[c];
        long long upper = (long long)m * sum_h + (long long)(m - 1) * sum_v;

        // ---- Read participant output ----
        // Read permutation (m integers)
        vector<int> perm(m);
        bool feasible = true;

        for (int i = 0; i < m; i++) {
            if (ouf.seekEof()) { feasible = false; break; }
            perm[i] = ouf.readInt();
        }

        // Validate permutation
        if (feasible) {
            vector<bool> used(m + 1, false);
            for (int i = 0; i < m; i++) {
                if (perm[i] < 1 || perm[i] > m || used[perm[i]]) {
                    feasible = false;
                    break;
                }
                used[perm[i]] = true;
            }
        }

        // Read reversal pairs for each original strip i=1..m
        vector<pair<int,int>> rev(m, {0, 0});
        if (feasible) {
            for (int i = 0; i < m; i++) {
                if (ouf.seekEof()) { feasible = false; break; }
                int l = ouf.readInt();
                if (ouf.seekEof()) { feasible = false; break; }
                int r = ouf.readInt();
                if (l == 0 && r == 0) {
                    rev[i] = {0, 0};
                } else if (l >= 1 && r <= n && l < r) {
                    rev[i] = {l, r};
                } else {
                    feasible = false;
                }
            }
        }

        if (!feasible) {
            total_score += 0.0;
            continue;
        }

        // Apply reversals to each original strip
        vector<string> modified(m);
        for (int i = 0; i < m; i++) {
            modified[i] = s[i];
            if (rev[i].first != 0) {
                int l = rev[i].first - 1;   // 0-indexed
                int r = rev[i].second - 1;
                reverse(modified[i].begin() + l, modified[i].begin() + r + 1);
            }
        }

        // Build final matrix in permutation order
        vector<string> rows(m);
        for (int i = 0; i < m; i++) {
            rows[i] = modified[perm[i] - 1];
        }

        long long obj = compute_obj(rows);

        // Per-test-case score using the formula from the problem statement:
        // if Upper == Base → score 100
        // else score = 100 * clamp((Obj - Base) / (Upper - Base), 0, 1)
        double case_score;
        if (upper == base_obj) {
            case_score = 100.0;
        } else {
            double ratio = (double)(obj - base_obj) / (double)(upper - base_obj);
            ratio = max(0.0, min(1.0, ratio));
            case_score = 100.0 * ratio;
        }

        total_score += case_score;
    }

    // After consuming all expected output for all test cases,
    // check that there are no extra non-whitespace tokens remaining.
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra output found after all test cases");
    }

    double avg_score = (num_cases > 0) ? (total_score / num_cases) : 0.0;
    double score_ratio = avg_score / 100.0;
    score_ratio = max(0.0, min(1.0, score_ratio));

    quitp(score_ratio, "Ratio: %.6f (avg score=%.4f over %d cases)",
          score_ratio, avg_score, num_cases);

    return 0;
}