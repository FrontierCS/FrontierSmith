#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    int T = inf.readInt();

    double total_score_sum = 0.0;
    int total_instances = T;

    for (int tc = 0; tc < T; tc++) {
        int n = inf.readInt();
        int k = inf.readInt();

        vector<long long> C(k+1);
        for (int j = 1; j <= k; j++) C[j] = inf.readLong();

        vector<long long> d(n+1), t(n+1), v(n+1);
        for (int i = 1; i <= n; i++) {
            d[i] = inf.readLong();
            t[i] = inf.readLong();
            v[i] = inf.readLong();
        }

        // Read participant output: k lines
        // Each line: s_j p_1 ... p_{s_j}
        vector<vector<int>> tracks(k+1);
        bool feasible = true;
        string fail_reason = "";

        vector<bool> used(n+1, false);

        for (int j = 1; j <= k; j++) {
            int sj = ouf.readInt();
            if (sj < 0 || sj > n) {
                feasible = false;
                fail_reason = "s_j out of range for track " + to_string(j);
                // consume rest somehow - but we can't easily, just quit
                break;
            }
            tracks[j].resize(sj);
            for (int x = 0; x < sj; x++) {
                int p = ouf.readInt();
                if (p < 1 || p > n) {
                    feasible = false;
                    fail_reason = "proposal index out of range on track " + to_string(j);
                    break;
                }
                tracks[j][x] = p;
            }
            if (!feasible) break;
        }

        if (!feasible) {
            // Try to compute baseline B still
            // Run baseline
            vector<long long> rem(k+1);
            vector<long long> last_d(k+1, -1);
            for (int j = 1; j <= k; j++) rem[j] = C[j];
            long long B = 0;
            for (int i = 1; i <= n; i++) {
                // find feasible tracks
                int best_j = -1;
                long long best_rem_minus_t = LLONG_MAX;
                long long best_last = LLONG_MIN;
                for (int j = 1; j <= k; j++) {
                    if (rem[j] >= t[i] && last_d[j] < d[i]) {
                        long long rmt = rem[j] - t[i];
                        if (best_j == -1 ||
                            rmt < best_rem_minus_t ||
                            (rmt == best_rem_minus_t && last_d[j] > best_last) ||
                            (rmt == best_rem_minus_t && last_d[j] == best_last && j < best_j)) {
                            best_j = j;
                            best_rem_minus_t = rmt;
                            best_last = last_d[j];
                        }
                    }
                }
                if (best_j != -1) {
                    B += v[i];
                    rem[best_j] -= t[i];
                    last_d[best_j] = d[i];
                }
            }
            // score for this instance is 0 (infeasible)
            double score_inst = 0.0;
            total_score_sum += score_inst;
            continue;
        }

        // Validate all constraints
        if (feasible) {
            for (int j = 1; j <= k; j++) {
                int sj = (int)tracks[j].size();
                long long dur_sum = 0;
                for (int x = 0; x < sj; x++) {
                    int p = tracks[j][x];
                    // check used
                    if (used[p]) {
                        feasible = false;
                        fail_reason = "proposal " + to_string(p) + " used more than once";
                        break;
                    }
                    used[p] = true;
                    // check strictly increasing index
                    if (x > 0 && tracks[j][x] <= tracks[j][x-1]) {
                        feasible = false;
                        fail_reason = "indices not strictly increasing on track " + to_string(j);
                        break;
                    }
                    // check strictly increasing difficulty
                    if (x > 0 && d[tracks[j][x]] <= d[tracks[j][x-1]]) {
                        feasible = false;
                        fail_reason = "difficulties not strictly increasing on track " + to_string(j);
                        break;
                    }
                    dur_sum += t[p];
                }
                if (!feasible) break;
                if (dur_sum > C[j]) {
                    feasible = false;
                    fail_reason = "duration exceeds budget on track " + to_string(j);
                    break;
                }
            }
        }

        // Compute Y
        long long Y = 0;
        if (feasible) {
            for (int j = 1; j <= k; j++) {
                for (int p : tracks[j]) {
                    Y += v[p];
                }
            }
        }

        // Run baseline to compute B
        vector<long long> rem(k+1);
        vector<long long> last_d(k+1, -1);
        for (int j = 1; j <= k; j++) rem[j] = C[j];
        long long B = 0;
        for (int i = 1; i <= n; i++) {
            int best_j = -1;
            long long best_rem_minus_t = LLONG_MAX;
            long long best_last = LLONG_MIN;
            for (int j = 1; j <= k; j++) {
                if (rem[j] >= t[i] && last_d[j] < d[i]) {
                    long long rmt = rem[j] - t[i];
                    if (best_j == -1 ||
                        rmt < best_rem_minus_t ||
                        (rmt == best_rem_minus_t && last_d[j] > best_last) ||
                        (rmt == best_rem_minus_t && last_d[j] == best_last && j < best_j)) {
                        best_j = j;
                        best_rem_minus_t = rmt;
                        best_last = last_d[j];
                    }
                }
            }
            if (best_j != -1) {
                B += v[i];
                rem[best_j] -= t[i];
                last_d[best_j] = d[i];
            }
        }

        double dB = (double)max(1LL, B);
        double dY = feasible ? (double)Y : 0.0;
        double ratio_inner = dY / dB;
        if (ratio_inner > 3.0) ratio_inner = 3.0;
        double score_inst = 100.0 * ratio_inner; // in [0, 300]
        total_score_sum += score_inst;
    }

    // Check EOF
    if (!ouf.seekEof()) {
        quitf(_wa, "Trailing data in output after all instances processed");
    }

    // Final ratio: mean score / 300
    double mean_score = total_score_sum / (double)total_instances; // in [0, 300]
    double final_ratio = mean_score / 300.0; // in [0, 1]
    if (final_ratio < 0.0) final_ratio = 0.0;
    if (final_ratio > 1.0) final_ratio = 1.0;

    quitp(final_ratio, "Ratio: %.6f (mean score over instances: %.2f / 300)", final_ratio, mean_score);

    return 0;
}