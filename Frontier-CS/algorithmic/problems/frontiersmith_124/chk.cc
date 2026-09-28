#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

static const int MAXM = (1 << 20);

static int64_t sum_w_arr[MAXM];
static int64_t orig_sum[MAXM];

int64_t computeReward(const vector<int>& masks, const vector<int>& weights, int N) {
    fill(sum_w_arr, sum_w_arr + MAXM, (int64_t)0);
    fill(orig_sum,  orig_sum  + MAXM, (int64_t)0);

    for (int i = 0; i < N; i++) {
        int m = masks[i];
        int64_t w = (int64_t)weights[i];
        sum_w_arr[m] += w;
        orig_sum[m]  += w;
    }

    // SOS DP: sum_w_arr[m] = sum of orig_sum[v] for all v that are subsets of m
    for (int bit = 0; bit < 20; bit++) {
        for (int m = 0; m < MAXM; m++) {
            if (m & (1 << bit)) {
                sum_w_arr[m] += sum_w_arr[m ^ (1 << bit)];
            }
        }
    }

    int64_t R = 0;
    for (int m = 0; m < MAXM; m++) {
        if (orig_sum[m] == 0) continue;
        int64_t sw  = orig_sum[m];   // total weight of devices with exactly mask m
        int64_t sos = sum_w_arr[m];  // total weight of devices with mask that is a subset of m
        int64_t below = sos - sw;    // devices with mask strictly below m

        // pairs (i,j) where X_i < X_j (as subsets) with X_j == m: reward += sw * below
        R += sw * below;

        // pairs (i,j) where X_i == X_j == m
        // number of such pairs = sw*(sw-1)/2 ... but we need sum of W_i*W_j
        // sum_{i<j, mask_i=mask_j=m} W_i*W_j = (sw^2 - sum(W_i^2)) / 2
        // Actually we need sq_w. Let's compute it inline.
    }

    // Redo with squared weights for same-mask pairs
    static int64_t sq_w_arr[MAXM];
    fill(sq_w_arr, sq_w_arr + MAXM, (int64_t)0);
    for (int i = 0; i < N; i++) {
        int64_t w = (int64_t)weights[i];
        sq_w_arr[masks[i]] += w * w;
    }

    for (int m = 0; m < MAXM; m++) {
        if (orig_sum[m] == 0) continue;
        int64_t sw  = orig_sum[m];
        int64_t sqw = sq_w_arr[m];
        // pairs with same mask m: sum W_i*W_j for i<j = (sw*sw - sqw) / 2
        R += (sw * sw - sqw) / 2;
    }

    return R;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    int T = inf.readInt();

    // Per-statement: total_score = floor( sum(score_test) / T )
    // score_test = clamp(0, 5000000, floor(1000000 * (R_submit+1) / (R_base+1)))
    // We report ratio = total_score / 5000000 in [0, 1]

    int64_t total_score_sum = 0;

    for (int t = 0; t < T; t++) {
        int N;
        int64_t B;
        N = inf.readInt();
        B = inf.readLong();

        vector<int64_t> C(20);
        for (int k = 0; k < 20; k++) C[k] = inf.readLong();

        vector<int> A(N), W(N);
        for (int i = 0; i < N; i++) A[i] = inf.readInt();
        for (int i = 0; i < N; i++) W[i] = inf.readInt();

        // Read participant output
        vector<int> X(N);
        for (int i = 0; i < N; i++) {
            X[i] = ouf.readInt();
        }

        // Feasibility check 1: range
        for (int i = 0; i < N; i++) {
            if (X[i] < 0 || X[i] >= (1 << 20)) {
                quitf(_wa, "Test case %d: X[%d]=%d out of range [0,2^20)", t+1, i+1, X[i]);
            }
        }

        // Feasibility check 2: preserve original bits
        for (int i = 0; i < N; i++) {
            if ((A[i] | X[i]) != X[i]) {
                quitf(_wa, "Test case %d: device %d A=%d X=%d, original bits not preserved",
                      t+1, i+1, A[i], X[i]);
            }
        }

        // Feasibility check 3: budget
        int64_t total_cost = 0;
        for (int i = 0; i < N; i++) {
            int new_bits = X[i] & (~A[i]) & ((1 << 20) - 1);
            for (int k = 0; k < 20; k++) {
                if (new_bits & (1 << k)) {
                    total_cost += C[k];
                }
            }
        }
        if (total_cost > B) {
            quitf(_wa, "Test case %d: total upgrade cost %lld exceeds budget %lld",
                  t+1, (long long)total_cost, (long long)B);
        }

        // Compute rewards
        int64_t R_submit = computeReward(X, W, N);
        int64_t R_base   = computeReward(A, W, N);

        // score_test = clamp(0, 5000000, floor(1000000 * (R_submit+1) / (R_base+1)))
        int64_t score_test;
        if (R_base + 1 <= 0) {
            score_test = 5000000LL;
        } else {
            // Use integer arithmetic for floor division
            int64_t num = (int64_t)1000000LL * (R_submit + 1);
            int64_t den = (R_base + 1);
            // floor division for non-negative numerator and positive denominator
            int64_t s = num / den;
            if (s < 0) s = 0;
            if (s > 5000000LL) s = 5000000LL;
            score_test = s;
        }

        total_score_sum += score_test;
    }

    // Strict EOF check: no trailing output allowed (beyond whitespace/newlines)
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra output after all test cases");
    }

    // total_score = floor(total_score_sum / T), in [0, 5000000]
    int64_t total_score = total_score_sum / (int64_t)T;
    if (total_score < 0) total_score = 0;
    if (total_score > 5000000LL) total_score = 5000000LL;

    // Report as ratio in [0, 1]
    double ratio = (double)total_score / 5000000.0;
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    quitp(ratio, "Ratio: %.9f", ratio);

    return 0;
}