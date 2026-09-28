#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    int T = inf.readInt();

    // Accumulate sum of score_t (each in [0, 2000])
    double total_score = 0.0;

    for (int t = 0; t < T; t++) {
        int N = inf.readInt();
        int K = inf.readInt();
        int M = inf.readInt();

        // Read baseline set from input
        vector<int> bvec(K);
        for (int i = 0; i < K; i++) {
            bvec[i] = inf.readInt();
        }

        // Read incompatible pairs from input
        set<pair<int,int>> incompatible;
        for (int i = 0; i < M; i++) {
            int u = inf.readInt();
            int v = inf.readInt();
            if (u > v) swap(u, v);
            incompatible.insert({u, v});
        }

        // Compute baseline F value (B_t)
        // Sum of K is at most 15000 total, so O(K^2) per test case is fine
        long long B = 0;
        for (int i = 0; i < K; i++)
            for (int j = i + 1; j < K; j++)
                B += (long long)(bvec[i] ^ bvec[j]);

        // Problem guarantees K >= 2 and baseline is feasible with distinct elements
        // so B_t > 0; guard defensively anyway
        if (B <= 0) {
            // Degenerate: read K tokens and award max score
            for (int i = 0; i < K; i++) ouf.readInt();
            total_score += 2000.0;
            continue;
        }

        // Read participant output: exactly K integers
        vector<int> svec(K);
        for (int i = 0; i < K; i++) {
            svec[i] = ouf.readInt(1, N,
                ("Test case " + to_string(t + 1) +
                 ": value out of range [1," + to_string(N) + "]").c_str());
        }

        // Check distinctness
        set<int> sset(svec.begin(), svec.end());
        bool feasible = ((int)sset.size() == K);
        string fail_reason;
        if (!feasible) {
            fail_reason = "Test case " + to_string(t + 1) + ": duplicate keys in output";
        }

        // Check incompatible pairs
        if (feasible) {
            for (auto& p : incompatible) {
                if (sset.count(p.first) && sset.count(p.second)) {
                    feasible = false;
                    fail_reason = "Test case " + to_string(t + 1)
                        + ": incompatible pair ("
                        + to_string(p.first) + "," + to_string(p.second)
                        + ") both in output";
                    break;
                }
            }
        }

        if (!feasible) {
            // Infeasible => score_t = 0
            total_score += 0.0;
            continue;
        }

        // Compute A_t = F(S)
        long long A = 0;
        for (int i = 0; i < K; i++)
            for (int j = i + 1; j < K; j++)
                A += (long long)(svec[i] ^ svec[j]);

        // score_t = min(2000, 1000 * A_t / B_t)  -- exactly as stated
        double score_t = min(2000.0, 1000.0 * (double)A / (double)B);
        if (score_t < 0.0) score_t = 0.0;
        total_score += score_t;
    }

    // Assert end of participant output stream
    if (!ouf.seekEof()) {
        quitf(_wa, "Trailing data in output after all test cases");
    }

    // final score = arithmetic mean of score_t  (in [0, 2000])
    double mean_score = total_score / (double)T;
    if (mean_score < 0.0) mean_score = 0.0;
    if (mean_score > 2000.0) mean_score = 2000.0;

    // Normalize to [0, 1] for quitp (Frontier-CS expects a ratio in [0,1])
    double final_ratio = mean_score / 2000.0;
    if (final_ratio < 0.0) final_ratio = 0.0;
    if (final_ratio > 1.0) final_ratio = 1.0;

    // "Ratio: <score_ratio>" tag is required by the judge parser
    quitp(final_ratio,
          "Ratio: %.9f (mean_score=%.4f, total_score=%.4f over %d cases)",
          final_ratio, mean_score, total_score, T);

    return 0;
}