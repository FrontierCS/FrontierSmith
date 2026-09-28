#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

// Compute shelf congestion cost for one shelf (1-indexed positions internally)
static long long shelfCongestion(const vector<int>& shelf, int d_val, long long w_val) {
    map<int, vector<int>> pos_map;
    for (int j = 0; j < (int)shelf.size(); j++) {
        pos_map[shelf[j]].push_back(j + 1);
    }
    long long cost = 0;
    for (auto& kv : pos_map) {
        const vector<int>& positions = kv.second;
        for (int k = 0; k + 1 < (int)positions.size(); k++) {
            int gap = positions[k + 1] - positions[k];
            long long pen = (long long)d_val - gap;
            if (pen > 0) cost += pen;
        }
    }
    return w_val * cost;
}

// Compute split penalty
static long long splitPenalty(const vector<vector<int>>& shelves, int m, int q,
                               const vector<long long>& h) {
    vector<int> shelf_used(q + 1, 0);
    for (int i = 0; i < m; i++) {
        vector<bool> seen(q + 1, false);
        for (int c : shelves[i]) {
            if (!seen[c]) {
                seen[c] = true;
                shelf_used[c]++;
            }
        }
    }
    long long cost = 0;
    for (int c = 1; c <= q; c++) {
        if (shelf_used[c] > 1) cost += h[c] * (long long)(shelf_used[c] - 1);
    }
    return cost;
}

// Run baseline algorithm and return its cost
static long long runBaseline(int n, int m, int q,
                              const vector<int>& a,
                              const vector<long long>& h,
                              const vector<int>& s,
                              const vector<int>& dv,
                              const vector<long long>& wt) {
    vector<int> rem_cnt(q + 1, 0);
    for (int x : a) rem_cnt[x]++;

    vector<int> base_shelf_used(q + 1, 0);
    vector<vector<int>> base_shelves(m);

    for (int i = 0; i < m; i++) {
        base_shelves[i].resize(s[i]);
        // window of last d_i-1 colors placed on this shelf
        // track counts of colors in window
        vector<int> win_cnt(q + 1, 0);
        deque<int> window;

        for (int j = 0; j < s[i]; j++) {
            // Maintain window of last dv[i]-1 positions
            while ((int)window.size() >= dv[i] - 1 && !window.empty()) {
                win_cnt[window.front()]--;
                window.pop_front();
            }

            // Find best color: prefer not in window, largest rem_cnt, smallest id
            int chosen = -1;
            // First pass: colors not in window
            for (int c = 1; c <= q; c++) {
                if (rem_cnt[c] <= 0) continue;
                if (win_cnt[c] > 0) continue; // in window
                if (chosen == -1 ||
                    rem_cnt[c] > rem_cnt[chosen] ||
                    (rem_cnt[c] == rem_cnt[chosen] && c < chosen)) {
                    chosen = c;
                }
            }
            // Second pass if none found
            if (chosen == -1) {
                for (int c = 1; c <= q; c++) {
                    if (rem_cnt[c] <= 0) continue;
                    if (chosen == -1 ||
                        rem_cnt[c] > rem_cnt[chosen] ||
                        (rem_cnt[c] == rem_cnt[chosen] && c < chosen)) {
                        chosen = c;
                    }
                }
            }

            base_shelves[i][j] = chosen;
            rem_cnt[chosen]--;
            window.push_back(chosen);
            win_cnt[chosen]++;
        }
    }

    long long B = 0;
    for (int i = 0; i < m; i++) {
        B += shelfCongestion(base_shelves[i], dv[i], wt[i]);
    }
    B += splitPenalty(base_shelves, m, q, h);
    return B;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    int t = inf.readInt();

    double score_sum = 0.0;
    int total_cases = 0;

    for (int tc = 0; tc < t; tc++) {
        // Read input
        int n = inf.readInt();
        int m = inf.readInt();
        int q = inf.readInt();

        vector<int> a(n);
        for (int i = 0; i < n; i++) a[i] = inf.readInt();

        vector<long long> h(q + 1);
        for (int c = 1; c <= q; c++) h[c] = inf.readLong();

        vector<int> s(m), dv(m);
        vector<long long> wt(m);
        for (int i = 0; i < m; i++) s[i] = inf.readInt();
        for (int i = 0; i < m; i++) dv[i] = inf.readInt();
        for (int i = 0; i < m; i++) wt[i] = inf.readLong();

        // Compute expected color counts
        vector<int> expected_cnt(q + 1, 0);
        for (int x : a) expected_cnt[x]++;

        // Read participant output: m lines, each with s[i] integers
        vector<vector<int>> shelves(m);
        for (int i = 0; i < m; i++) {
            shelves[i].resize(s[i]);
            for (int j = 0; j < s[i]; j++) {
                shelves[i][j] = ouf.readInt(1, q,
                    ("Test case " + to_string(tc + 1) +
                     " shelf " + to_string(i + 1) +
                     " position " + to_string(j + 1) +
                     ": color out of range [1," + to_string(q) + "]").c_str());
            }
        }

        // Check color multiset matches
        vector<int> user_cnt(q + 1, 0);
        for (int i = 0; i < m; i++)
            for (int j = 0; j < s[i]; j++)
                user_cnt[shelves[i][j]]++;

        for (int c = 1; c <= q; c++) {
            if (expected_cnt[c] != user_cnt[c]) {
                quitf(_wa,
                      "Test case %d: color %d count mismatch: expected %d got %d",
                      tc + 1, c, expected_cnt[c], user_cnt[c]);
            }
        }

        // Compute user cost U
        long long U = 0;
        for (int i = 0; i < m; i++) {
            U += shelfCongestion(shelves[i], dv[i], wt[i]);
        }
        U += splitPenalty(shelves, m, q, h);

        // Compute baseline cost B
        long long B = runBaseline(n, m, q, a, h, s, dv, wt);

        // Compute per-test-case score in [0, 200] per problem statement
        // arithmetic mean => just sum them and divide by t
        double case_score;
        if (B == 0 && U == 0) {
            case_score = 200.0;
        } else if (B == 0 && U > 0) {
            case_score = 0.0;
        } else {
            // Score = 100 * 2B / (B + U)
            case_score = 100.0 * 2.0 * (double)B / ((double)B + (double)U);
        }

        score_sum += case_score;
        total_cases++;
    }

    // Check no extra output
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra output found after all test cases");
    }

    // File score = arithmetic mean of per-test-case scores, each in [0,200]
    // Normalize to [0,1]
    double ratio = 0.0;
    if (total_cases > 0) {
        ratio = (score_sum / (double)total_cases) / 200.0;
    }
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    quitp(ratio, "Ratio: %.9f (score_sum=%.4f cases=%d)",
          ratio, score_sum, total_cases);

    return 0;
}