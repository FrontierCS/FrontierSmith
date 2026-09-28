#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    int T = inf.readInt();

    long long total_S = 0;
    int total_tests = 0;

    for (int t = 0; t < T; t++) {
        int N = inf.readInt();
        int K = inf.readInt();

        struct Item {
            long long V;
            int M;
            vector<pair<long long, int>> factors; // (prime, exponent)
        };
        vector<Item> items(N);
        for (int i = 0; i < N; i++) {
            items[i].V = inf.readLong();
            items[i].M = inf.readInt();
            items[i].factors.resize(items[i].M);
            for (int j = 0; j < items[i].M; j++) {
                items[i].factors[j].first = inf.readLong();
                items[i].factors[j].second = inf.readInt();
            }
        }

        // Read participant output: N integers
        vector<int> assignment(N);
        for (int i = 0; i < N; i++) {
            assignment[i] = ouf.readInt();
        }

        // ---- Feasibility Check ----
        bool feasible = true;
        string fail_reason;

        for (int i = 0; i < N && feasible; i++) {
            if (assignment[i] < 0 || assignment[i] > K) {
                feasible = false;
                fail_reason = "assignment out of range for item " + to_string(i + 1)
                              + ": got " + to_string(assignment[i]);
            }
        }

        if (feasible) {
            // Any item assigned to a crate (!=0) must have all exponents < 2
            for (int i = 0; i < N && feasible; i++) {
                if (assignment[i] != 0) {
                    for (auto& f : items[i].factors) {
                        if (f.second >= 2) {
                            feasible = false;
                            fail_reason = "item " + to_string(i + 1)
                                          + " has exponent >= 2 but is assigned to crate "
                                          + to_string(assignment[i]);
                            break;
                        }
                    }
                }
            }
        }

        if (feasible) {
            // For each crate, no prime may appear more than once across all assigned items
            // crate index 1..K
            vector<map<long long, int>> crate_prime_owner(K + 1); // prime -> first item index using it
            for (int i = 0; i < N && feasible; i++) {
                int c = assignment[i];
                if (c == 0) continue;
                for (auto& f : items[i].factors) {
                    long long prime = f.first;
                    auto it = crate_prime_owner[c].find(prime);
                    if (it != crate_prime_owner[c].end()) {
                        feasible = false;
                        fail_reason = "crate " + to_string(c)
                                      + ": prime " + to_string(prime)
                                      + " appears in both item " + to_string(it->second + 1)
                                      + " and item " + to_string(i + 1);
                        break;
                    }
                    crate_prime_owner[c][prime] = i;
                }
            }
        }

        // ---- Compute participant objective O ----
        long long O = 0;
        if (feasible) {
            for (int i = 0; i < N; i++) {
                if (assignment[i] != 0) {
                    O += items[i].V;
                }
            }
        }

        // ---- Compute baseline B ----
        // Step 1: filter items with no exponent >= 2
        vector<int> eligible;
        for (int i = 0; i < N; i++) {
            bool bad = false;
            for (auto& f : items[i].factors) {
                if (f.second >= 2) { bad = true; break; }
            }
            if (!bad) eligible.push_back(i);
        }

        // Step 2: compute density D_i = V_i / M_i (as double)
        // Step 3: sort by (-D_i, -V_i, +index)
        sort(eligible.begin(), eligible.end(), [&](int a, int b) {
            double Da = (double)items[a].V / (double)items[a].M;
            double Db = (double)items[b].V / (double)items[b].M;
            if (Da != Db) return Da > Db;
            if (items[a].V != items[b].V) return items[a].V > items[b].V;
            return a < b;
        });

        // Step 4: greedy placement into lowest-indexed available crate
        vector<set<long long>> crate_primes(K + 1);
        long long B = 0;

        for (int idx : eligible) {
            for (int c = 1; c <= K; c++) {
                bool ok = true;
                for (auto& f : items[idx].factors) {
                    if (crate_primes[c].count(f.first)) {
                        ok = false;
                        break;
                    }
                }
                if (ok) {
                    for (auto& f : items[idx].factors) {
                        crate_primes[c].insert(f.first);
                    }
                    B += items[idx].V;
                    break;
                }
            }
        }

        // ---- Compute per-test score S ----
        // S = min(2000000, floor(1000000 * (O+1) / (B+1)))
        long long S;
        if (!feasible) {
            S = 0;
        } else {
            // Use double; O and B are at most ~4e12, ratio O/B <= 2 before capping
            double ratio = (double)(O + 1) / (double)(B + 1);
            long long raw = (long long)floor(1000000.0 * ratio);
            S = min(2000000LL, raw);
        }

        total_S += S;
        total_tests++;
    }

    // Verify no extra output
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra output found after all test cases");
    }

    // Final score = floor(arithmetic mean of per-test S values)
    // Then normalize to [0,1] for quitp
    long long mean_S_floor = (total_tests > 0) ? (total_S / total_tests) : 0LL;

    double score_ratio = (total_tests > 0)
        ? (double)mean_S_floor / 2000000.0
        : 0.0;

    if (score_ratio < 0.0) score_ratio = 0.0;
    if (score_ratio > 1.0) score_ratio = 1.0;

    quitp(score_ratio,
          "Ratio: %.6f (mean raw score floored: %lld / 2000000, sum_S=%lld, T=%d)",
          score_ratio,
          mean_S_floor,
          total_S,
          total_tests);

    return 0;
}