#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

// ceil(3*n/2) - 2
static long long baseline(int n) {
    return (long long)(3 * n + 1) / 2 - 2;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    int T = inf.readInt();

    double total_score = 0.0;
    int num_instances = 0;

    for (int inst = 0; inst < T; inst++) {
        int n = inf.readInt();
        int k = inf.readInt();

        // Read arrays
        vector<long long> weights(k);
        vector<vector<long long>> arrs(k, vector<long long>(n));
        for (int s = 0; s < k; s++) {
            weights[s] = inf.readLong();
            for (int j = 0; j < n; j++) {
                arrs[s][j] = inf.readLong();
            }
        }

        long long Bn = baseline(n);
        // P(n) = 10 * B(n) + 1
        long long Pn = 10LL * Bn + 1LL;

        // ---- Read participant tree for this instance ----
        int L = -1;
        bool bad = false;
        string bad_reason;

        // Try reading L
        if (ouf.seekEof()) {
            bad = true;
            bad_reason = "unexpected end of output (missing L for instance " + to_string(inst + 1) + ")";
        } else {
            L = ouf.readInt();
            if (L < 1 || L > 100000) {
                bad = true;
                bad_reason = "L=" + to_string(L) + " out of range [1,100000]";
            }
        }

        // Node types: 0=unset, 1=comparison, 2=answer
        struct Node {
            int type; // 1=C, 2=A
            int i, j;       // for C
            int lt, eq, gt; // for C
            int p, q;       // for A
        };

        vector<Node> nodes;
        if (!bad) {
            nodes.resize(L + 1);
            for (int v = 1; v <= L && !bad; v++) {
                if (ouf.seekEof()) {
                    bad = true;
                    bad_reason = "unexpected end of output while reading nodes (instance " + to_string(inst+1) + ")";
                    break;
                }
                string tp = ouf.readWord();
                if (tp == "C") {
                    nodes[v].type = 1;
                    nodes[v].i  = ouf.readInt();
                    nodes[v].j  = ouf.readInt();
                    nodes[v].lt = ouf.readInt();
                    nodes[v].eq = ouf.readInt();
                    nodes[v].gt = ouf.readInt();
                    // Validate
                    if (nodes[v].i < 1 || nodes[v].i > n ||
                        nodes[v].j < 1 || nodes[v].j > n ||
                        nodes[v].i == nodes[v].j) {
                        bad = true;
                        bad_reason = "node " + to_string(v) + ": invalid i/j";
                    } else if (nodes[v].lt < 1 || nodes[v].lt > L ||
                               nodes[v].eq < 1 || nodes[v].eq > L ||
                               nodes[v].gt < 1 || nodes[v].gt > L) {
                        bad = true;
                        bad_reason = "node " + to_string(v) + ": child id out of [1,L]";
                    } else if (nodes[v].lt <= v || nodes[v].eq <= v || nodes[v].gt <= v) {
                        bad = true;
                        bad_reason = "node " + to_string(v) + ": child id not > v (cycle violation)";
                    }
                } else if (tp == "A") {
                    nodes[v].type = 2;
                    nodes[v].p = ouf.readInt();
                    nodes[v].q = ouf.readInt();
                    if (nodes[v].p < 1 || nodes[v].p > n ||
                        nodes[v].q < 1 || nodes[v].q > n) {
                        bad = true;
                        bad_reason = "node " + to_string(v) + ": p/q out of [1,n]";
                    }
                } else {
                    bad = true;
                    bad_reason = "node " + to_string(v) + ": unknown type '" + tp + "'";
                }
            }
        }

        double score_inst;
        if (bad) {
            // Score 0 for this instance
            score_inst = 0.0;
        } else {
            // Simulate on each array
            long long W = 0;
            double weighted_cost = 0.0;

            for (int s = 0; s < k; s++) {
                W += weights[s];
                long long ds = 0;
                long long es = 0;

                // Simulate
                int cur = 1;
                bool reached_answer = false;
                int ans_p = -1, ans_q = -1;

                while (cur >= 1 && cur <= L) {
                    if (nodes[cur].type == 2) {
                        ans_p = nodes[cur].p;
                        ans_q = nodes[cur].q;
                        reached_answer = true;
                        break;
                    } else if (nodes[cur].type == 1) {
                        ds++;
                        long long ai = arrs[s][nodes[cur].i - 1];
                        long long aj = arrs[s][nodes[cur].j - 1];
                        if (ai < aj) cur = nodes[cur].lt;
                        else if (ai == aj) cur = nodes[cur].eq;
                        else cur = nodes[cur].gt;
                    } else {
                        // unset node — bad
                        reached_answer = false;
                        break;
                    }
                }

                if (!reached_answer) {
                    es = 1;
                } else {
                    // Check correctness
                    long long actual_min = *min_element(arrs[s].begin(), arrs[s].end());
                    long long actual_max = *max_element(arrs[s].begin(), arrs[s].end());
                    if (arrs[s][ans_p - 1] != actual_min || arrs[s][ans_q - 1] != actual_max) {
                        es = 1;
                    }
                }

                double cost_s = (double)ds + (double)Pn * (double)es;
                weighted_cost += (double)weights[s] * cost_s;
            }

            double C_val;
            if (W == 0) {
                C_val = 0.0;
            } else {
                C_val = weighted_cost / (double)W;
            }

            // score_instance = clamp(1_000_000 * B(n) / C, 0, 2_000_000)
            if (Bn == 0) {
                // n=1 edge case: B(1)=0
                // If C=0 (all correct, 0 comparisons), perfect; else 0
                if (C_val <= 0.0) {
                    score_inst = 2000000.0;
                } else {
                    score_inst = 0.0;
                }
            } else if (C_val <= 0.0) {
                // Shouldn't normally happen if Bn>0
                score_inst = 2000000.0;
            } else {
                double raw = 1000000.0 * (double)Bn / C_val;
                if (raw < 0.0) raw = 0.0;
                if (raw > 2000000.0) raw = 2000000.0;
                score_inst = raw;
            }
        }

        total_score += score_inst;
        num_instances++;
    }

    // Check end of output
    if (!ouf.seekEof()) {
        // There is trailing garbage — penalize? 
        // We'll just warn but still give the score
        // Actually per playbook: assert end of stream
        // but we don't want to zero out a good solution for minor trailing whitespace
        // let's do a strict check
        quitf(_wa, "Trailing content in output after all instances processed. Ratio: %.6f", 0.0);
    }

    double final_ratio;
    if (num_instances == 0) {
        final_ratio = 0.0;
    } else {
        double mean_score = total_score / (double)num_instances;
        // mean_score in [0, 2_000_000], normalize to [0,1]
        final_ratio = mean_score / 2000000.0;
    }

    if (final_ratio < 0.0) final_ratio = 0.0;
    if (final_ratio > 1.0) final_ratio = 1.0;

    quitp(final_ratio, "Ratio: %.6f (mean score: %.2f / 2000000)", final_ratio, total_score / max(1, num_instances));
    return 0;
}