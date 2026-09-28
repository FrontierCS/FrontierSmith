#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    int T = inf.readInt();

    double sum_score = 0.0;
    int valid_cases = 0;

    for (int tc = 1; tc <= T; tc++) {
        long long n = inf.readLong();
        long long m = inf.readLong();
        long long Cd = inf.readLong();
        long long Ca = inf.readLong();

        set<pair<int,int>> G_edges;
        for (long long i = 0; i < m; i++) {
            int u = inf.readInt();
            int v = inf.readInt();
            if (u > v) swap(u, v);
            G_edges.insert({u, v});
        }

        long long total_pairs = n * (n - 1) / 2;
        long long B = min(Cd * m, Ca * (total_pairs - m));

        // Try to read k; if EOF or bad format, treat as infeasible (score 0)
        int k = -1;
        if (ouf.eof()) {
            // No output at all for this test case -> infeasible
            sum_score += 0.0;
            valid_cases++;
            continue;
        }
        k = ouf.readInt();

        if (k != (int)(n - 1)) {
            // infeasible: skip remaining output we can't reliably parse
            // score 0 for this test case
            sum_score += 0.0;
            valid_cases++;
            // We cannot safely skip the remaining output, so just quit WA
            quitf(_wa, "Test case %d: k=%d but expected n-1=%lld", tc, k, n - 1);
        }

        // Each module maps to its list of original vertices
        map<int, vector<int>> modules;
        for (int i = 1; i <= (int)n; i++) {
            modules[i] = {i};
        }
        set<int> active;
        for (int i = 1; i <= (int)n; i++) active.insert(i);

        set<pair<int,int>> H_edges;
        bool feasible = true;
        string fail_reason;

        for (int i = 0; i < k; i++) {
            int x = ouf.readInt();
            int y = ouf.readInt();
            int t = ouf.readInt();

            if (!feasible) {
                // keep reading but don't process
                continue;
            }

            if (active.find(x) == active.end()) {
                feasible = false;
                fail_reason = format("Test case %d, merge %d: module %d is not active", tc, i+1, x);
                continue;
            }
            if (active.find(y) == active.end()) {
                feasible = false;
                fail_reason = format("Test case %d, merge %d: module %d is not active", tc, i+1, y);
                continue;
            }
            if (x == y) {
                feasible = false;
                fail_reason = format("Test case %d, merge %d: x == y == %d", tc, i+1, x);
                continue;
            }
            if (t != 0 && t != 1) {
                feasible = false;
                fail_reason = format("Test case %d, merge %d: invalid t=%d", tc, i+1, t);
                continue;
            }

            if (t == 1) {
                const vector<int>& vx = modules[x];
                const vector<int>& vy = modules[y];
                for (int a : vx) {
                    for (int b : vy) {
                        int ea = a, eb = b;
                        if (ea > eb) swap(ea, eb);
                        H_edges.insert({ea, eb});
                    }
                }
            }

            // Create new module with ID n + i + 1 (i is 0-indexed)
            int new_id = (int)n + i + 1;
            vector<int> combined;
            combined.reserve(modules[x].size() + modules[y].size());
            for (int v : modules[x]) combined.push_back(v);
            for (int v : modules[y]) combined.push_back(v);
            modules[new_id] = move(combined);

            active.erase(x);
            active.erase(y);
            modules.erase(x);
            modules.erase(y);
            active.insert(new_id);
        }

        if (!feasible) {
            quitf(_wa, "%s", fail_reason.c_str());
        }

        if ((int)active.size() != 1) {
            quitf(_wa, "Test case %d: %zu active modules remain after all merges (expected 1)",
                  tc, (size_t)active.size());
        }

        // Compute cost
        long long deleted_count = 0;
        long long added_count   = 0;

        for (auto& e : G_edges) {
            if (H_edges.find(e) == H_edges.end()) deleted_count++;
        }
        for (auto& e : H_edges) {
            if (G_edges.find(e) == G_edges.end()) added_count++;
        }

        long long C = Cd * deleted_count + Ca * added_count;

        double S;
        if (B == 0) {
            S = (C == 0) ? 100.0 : 0.0;
        } else if (C == 0) {
            S = 200.0;
        } else {
            double raw = 100.0 * (double)B / (double)C;
            S = max(0.0, min(200.0, raw));
        }

        sum_score += S;
        valid_cases++;
    }

    double avg_score = (T > 0) ? (sum_score / (double)T) : 0.0;
    // ratio in [0,1]: full score is 200
    double ratio = max(0.0, min(1.0, avg_score / 200.0));

    quitp(ratio, "Ratio: %.6f (avg score per test: %.4f / 200)", ratio, avg_score);

    return 0;
}