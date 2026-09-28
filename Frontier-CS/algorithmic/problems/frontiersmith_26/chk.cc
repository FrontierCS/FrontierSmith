#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    int T = inf.readInt();

    double total_weighted_score = 0.0;
    double total_weight = 0.0;

    for (int t = 0; t < T; t++) {
        long long N = inf.readLong();
        long long M = inf.readLong();
        long long B = inf.readLong();
        long long C = inf.readLong();

        vector<long long> p(N), v(N);
        for (int i = 0; i < N; i++) {
            p[i] = inf.readLong();
            v[i] = inf.readLong();
        }

        vector<long long> emask(M), cost(M);
        for (int j = 0; j < M; j++) {
            emask[j] = inf.readLong();
            cost[j] = inf.readLong();
        }

        // R_all = sum of all node values
        long long R_all = 0;
        for (int i = 0; i < N; i++) R_all += v[i];

        // ---- Read and validate participant output ----
        long long q_raw = ouf.readLong();
        if (q_raw < 0 || q_raw > M) {
            // Treat as 0 for this test case
            total_weight += (double)R_all;
            // still need to compute baseline below; mark as infeasible
            // We'll set R_you = 0 and fall through after skipping index reads
            // Actually we need to compute R_base still. Use a flag.
            // Simplest: quitf for malformed, but problem says treat as 0.
            // We'll add to weight with score=0 contribution.
            // Compute baseline first (we haven't consumed ouf indices yet since q_raw is bad)
            // For the baseline computation we need the full logic — do it inline.
            // Build password->value map
            map<long long, long long> bmap;
            {
                long long init_gain = 0;
                for (int i = 0; i < N; i++) {
                    if (p[i] == 0) init_gain += v[i];
                    else bmap[p[i]] += v[i];
                }
                long long R_base_local = init_gain;
                vector<bool> base_used(M, false);
                long long rem_budget = C;
                long long base_cumXOR = 0;
                while (true) {
                    int best_j = -1;
                    long long best_gain = 0, best_cost = 0;
                    for (int j = 0; j < M; j++) {
                        if (base_used[j]) continue;
                        if (cost[j] > rem_budget) continue;
                        long long nx = base_cumXOR ^ emask[j];
                        long long gain = 0;
                        auto it2 = bmap.find(nx);
                        if (it2 != bmap.end()) gain = it2->second;
                        if (gain > 0) {
                            if (best_j == -1) {
                                best_j = j; best_gain = gain; best_cost = cost[j];
                            } else {
                                double r1 = (double)gain / (double)cost[j];
                                double r0 = (double)best_gain / (double)best_cost;
                                if (r1 > r0 + 1e-12) {
                                    best_j = j; best_gain = gain; best_cost = cost[j];
                                } else if (fabs(r1 - r0) < 1e-12) {
                                    if (gain > best_gain) {
                                        best_j = j; best_gain = gain; best_cost = cost[j];
                                    } else if (gain == best_gain) {
                                        if (cost[j] < best_cost) {
                                            best_j = j; best_gain = gain; best_cost = cost[j];
                                        } else if (cost[j] == best_cost && j < best_j) {
                                            best_j = j; best_gain = gain; best_cost = cost[j];
                                        }
                                    }
                                }
                            }
                        }
                    }
                    if (best_j == -1) break;
                    base_used[best_j] = true;
                    rem_budget -= cost[best_j];
                    base_cumXOR ^= emask[best_j];
                    auto it2 = bmap.find(base_cumXOR);
                    if (it2 != bmap.end()) {
                        R_base_local += it2->second;
                        bmap.erase(it2);
                    }
                }
                double score_local = 0.0;
                if (R_all == R_base_local) {
                    score_local = 1.0;
                } else {
                    // R_you = 0 (infeasible), R_base = R_base_local
                    double num = (double)(0 - R_base_local);
                    double den = (double)(R_all - R_base_local);
                    score_local = num / den;
                    if (score_local < 0.0) score_local = 0.0;
                    if (score_local > 1.0) score_local = 1.0;
                }
                total_weighted_score += (double)R_all * score_local;
                // total_weight already added above
            }
            continue;
        }
        int q = (int)q_raw;

        vector<int> chosen(q);
        set<int> seen;
        long long used_cost = 0;
        bool feasible = true;

        for (int i = 0; i < q; i++) {
            long long idx_raw = ouf.readLong();
            if (idx_raw < 1 || idx_raw > M) {
                feasible = false;
                // drain remaining
                // we can't easily drain; just mark infeasible and break
                break;
            }
            int idx = (int)idx_raw;
            if (seen.count(idx)) {
                feasible = false;
                break;
            }
            seen.insert(idx);
            chosen[i] = idx;
            used_cost += cost[idx - 1];
        }

        if (used_cost > C) feasible = false;

        // ---- Build password->value map ----
        // We need this for both simulation and baseline
        // Initial gain (nodes already at 0)
        long long init_gain = 0;
        map<long long, long long> pmap;
        for (int i = 0; i < N; i++) {
            if (p[i] == 0) init_gain += v[i];
            else pmap[p[i]] += v[i];
        }

        // ---- Simulate participant's solution ----
        long long R_you = 0;
        if (feasible) {
            R_you = init_gain;
            // Track which cumulative XOR values have already been "claimed"
            // A node with password pw is cracked when cumXOR == pw
            // We use a set of already-claimed cumXOR values to avoid double-counting
            set<long long> claimed_xors;
            claimed_xors.insert(0); // init: cumXOR=0 cracks nodes with p=0
            long long cumXOR = 0;
            map<long long, long long> sim_pmap = pmap;
            for (int i = 0; i < q; i++) {
                int j = chosen[i]; // 1-based
                cumXOR ^= emask[j - 1];
                if (!claimed_xors.count(cumXOR)) {
                    claimed_xors.insert(cumXOR);
                    auto it2 = sim_pmap.find(cumXOR);
                    if (it2 != sim_pmap.end()) {
                        R_you += it2->second;
                        sim_pmap.erase(it2);
                    }
                }
            }
        }

        // ---- Compute baseline ----
        long long R_base = init_gain;
        {
            map<long long, long long> bmap = pmap;
            vector<bool> base_used(M, false);
            long long rem_budget = C;
            long long base_cumXOR = 0;
            while (true) {
                int best_j = -1;
                long long best_gain = 0, best_cost_val = 0;
                for (int j = 0; j < M; j++) {
                    if (base_used[j]) continue;
                    if (cost[j] > rem_budget) continue;
                    long long nx = base_cumXOR ^ emask[j];
                    long long gain = 0;
                    auto it2 = bmap.find(nx);
                    if (it2 != bmap.end()) gain = it2->second;
                    if (gain > 0) {
                        if (best_j == -1) {
                            best_j = j; best_gain = gain; best_cost_val = cost[j];
                        } else {
                            // Compare gain/cost ratios
                            // gain/cost[j] vs best_gain/best_cost_val
                            // Use cross-multiplication to avoid floating point issues
                            // gain * best_cost_val vs best_gain * cost[j]
                            // But these can be huge (gain up to ~2e14, cost up to 1e12)
                            // product up to ~2e26, need __int128
                            __int128 lhs = (__int128)gain * best_cost_val;
                            __int128 rhs = (__int128)best_gain * cost[j];
                            if (lhs > rhs) {
                                best_j = j; best_gain = gain; best_cost_val = cost[j];
                            } else if (lhs == rhs) {
                                // tie: larger immediate_gain
                                if (gain > best_gain) {
                                    best_j = j; best_gain = gain; best_cost_val = cost[j];
                                } else if (gain == best_gain) {
                                    // tie: smaller cost
                                    if (cost[j] < best_cost_val) {
                                        best_j = j; best_gain = gain; best_cost_val = cost[j];
                                    } else if (cost[j] == best_cost_val) {
                                        // tie: smaller index (j is 0-based)
                                        if (j < best_j) {
                                            best_j = j; best_gain = gain; best_cost_val = cost[j];
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                if (best_j == -1) break;
                base_used[best_j] = true;
                rem_budget -= cost[best_j];
                base_cumXOR ^= emask[best_j];
                auto it2 = bmap.find(base_cumXOR);
                if (it2 != bmap.end()) {
                    R_base += it2->second;
                    bmap.erase(it2);
                }
            }
        }

        // ---- Compute normalized score for this test case ----
        // S = clamp((R_you - R_base) / (R_all - R_base), 0, 1)  if R_all != R_base
        // S = 1  if R_all == R_base
        double score;
        if (!feasible) {
            // Infeasible => R_you treated as 0
            if (R_all == R_base) {
                score = 1.0;
            } else {
                double num = (double)(0 - R_base);
                double den = (double)(R_all - R_base);
                score = num / den;
                if (score < 0.0) score = 0.0;
                if (score > 1.0) score = 1.0;
            }
        } else if (R_all == R_base) {
            score = 1.0;
        } else {
            double num = (double)(R_you - R_base);
            double den = (double)(R_all - R_base);
            score = num / den;
            if (score < 0.0) score = 0.0;
            if (score > 1.0) score = 1.0;
        }

        total_weighted_score += (double)R_all * score;
        total_weight += (double)R_all;
    }

    // Assert end of participant output
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra content found in participant output after all test cases");
    }

    // final_ratio = sum(R_all * S) / sum(R_all), in [0,1]
    // This corresponds to the statement's formula:
    //   1,000,000 * sum(R_all * S) / sum(R_all)
    // divided by 1,000,000 to fit in [0,1] for quitp and Ratio tag.
    double final_ratio;
    if (total_weight <= 0.0) {
        final_ratio = 1.0;
    } else {
        final_ratio = total_weighted_score / total_weight;
        if (final_ratio < 0.0) final_ratio = 0.0;
        if (final_ratio > 1.0) final_ratio = 1.0;
    }

    // The statement's displayed score = 1,000,000 * final_ratio
    // We report final_ratio in [0,1] as required by the Ratio: tag.
    quitp(final_ratio, "Score: %.2f / 1000000 | Ratio: %.9f", final_ratio * 1000000.0, final_ratio);
}