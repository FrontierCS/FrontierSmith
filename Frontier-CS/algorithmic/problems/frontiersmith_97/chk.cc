#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    int t = inf.readInt();

    long long total_score_sum = 0;
    int total_cases = t;

    for (int tc = 0; tc < t; tc++) {
        // ---- Read problem input ----
        int n = inf.readInt();
        int m = inf.readInt();
        long long A = inf.readLong();
        long long B = inf.readLong();
        long long C = inf.readLong();

        vector<long long> w(n + 1);
        for (int i = 1; i <= n; i++) w[i] = inf.readLong();

        // Build adjacency list (input order) and edge set
        vector<vector<int>> adj(n + 1);
        set<pair<int,int>> edge_set;
        vector<pair<int,int>> edges(m);

        for (int i = 0; i < m; i++) {
            int u = inf.readInt(), v = inf.readInt();
            edges[i] = {u, v};
            adj[u].push_back(v);
            adj[v].push_back(u);
            edge_set.insert({min(u,v), max(u,v)});
        }

        // ---- Compute OBJ_base via DFS greedy ----
        vector<int> par(n + 1, 0);
        vector<int> dep(n + 1, -1);

        // Iterative DFS from vertex 1 in adjacency-list order
        {
            stack<pair<int,int>> stk;
            dep[1] = 0;
            par[1] = 0;
            stk.push({1, 0});
            while (!stk.empty()) {
                auto& [v, idx] = stk.top();
                if (idx < (int)adj[v].size()) {
                    int nb = adj[v][idx++];
                    if (dep[nb] == -1) {
                        dep[nb] = dep[v] + 1;
                        par[nb] = v;
                        stk.push({nb, 0});
                    }
                } else {
                    stk.pop();
                }
            }
        }

        // Sort vertices by decreasing depth, tie-break by smaller index
        vector<int> order;
        for (int i = 1; i <= n; i++) order.push_back(i);
        sort(order.begin(), order.end(), [&](int a, int b) {
            return dep[a] != dep[b] ? dep[a] > dep[b] : a < b;
        });

        vector<bool> base_used(n + 1, false);
        vector<vector<int>> base_paths;
        for (int v : order) {
            if (v == 1) continue;
            if (!base_used[v] && !base_used[par[v]]) {
                base_paths.push_back({v, par[v]});
                base_used[v] = true;
                base_used[par[v]] = true;
            }
        }

        // Compute OBJ_dfs
        long long obj_dfs = 0;
        {
            // sum weights
            for (int v = 1; v <= n; v++)
                if (base_used[v]) obj_dfs += w[v];
            // activation cost
            obj_dfs -= A * (long long)base_paths.size();
            // chord count and cross counts for base
            // For length-2 paths there are no chords
            // cross counts
            vector<int> base_pid(n + 1, -1);
            for (int p = 0; p < (int)base_paths.size(); p++) {
                for (int v : base_paths[p]) base_pid[v] = p;
            }
            map<pair<int,int>, long long> base_cross;
            for (auto& [u, v] : edges) {
                int pu = base_pid[u], pv = base_pid[v];
                if (pu == -1 || pv == -1) continue;
                if (pu != pv) {
                    int a = min(pu, pv), b = max(pu, pv);
                    base_cross[{a, b}]++;
                }
            }
            for (auto& [pr, cnt] : base_cross) {
                if (cnt > 1) obj_dfs -= C * (cnt - 1);
            }
        }
        long long obj_base = max(0LL, obj_dfs);

        // ---- Read participant output for this test case ----
        bool feasible = true;
        int k = ouf.readInt();

        // Feasibility check 1: 0 <= k <= n
        if (k < 0 || k > n) {
            feasible = false;
            // We don't know how many lines to skip; we must abort with WA
            // But per spec we assign 0 and continue if possible.
            // Since k is out of range we can't safely read paths; assign 0.
            // We'll try to read 0 paths below (set k=0 for reading purposes).
            // Actually we can't recover safely here without reading garbage.
            // Assign 0 and skip reading paths by treating k as 0.
            k = 0;
        }

        vector<vector<int>> paths(k);
        vector<int> path_id(n + 1, -1);
        vector<int> path_pos(n + 1, -1);

        for (int p = 0; p < k && feasible; p++) {
            int len = ouf.readInt();
            // Feasibility check 2: len >= 2
            if (len < 2) {
                feasible = false;
                // Read len vertices anyway to advance stream
                for (int j = 0; j < len && j < n; j++) ouf.readInt();
                // Skip remaining paths by treating rest as infeasible
                // We can't safely continue; mark infeasible and break
                // We still need to read the rest of the output for this test case
                // but we don't know how many tokens remain. 
                // Set k to p+1 so we stop reading after this path.
                k = p + 1;
                break;
            }
            paths[p].resize(len);
            for (int j = 0; j < len; j++) {
                int v = ouf.readInt();
                // Feasibility check 3a: vertex in range
                if (v < 1 || v > n) {
                    feasible = false;
                }
                paths[p][j] = v;
            }
            if (!feasible) {
                // Can't safely validate further; stop reading more paths
                k = p + 1;
                break;
            }
            // Feasibility check 3b: all vertices distinct within path
            set<int> seen;
            for (int v : paths[p]) {
                if (!seen.insert(v).second) {
                    feasible = false;
                    break;
                }
            }
            if (!feasible) {
                k = p + 1;
                break;
            }
            // Feasibility check 4: consecutive edges exist
            for (int j = 0; j + 1 < len; j++) {
                int u = paths[p][j], v2 = paths[p][j+1];
                if (!edge_set.count({min(u,v2), max(u,v2)})) {
                    feasible = false;
                    break;
                }
            }
            if (!feasible) {
                k = p + 1;
                break;
            }
            // Feasibility check 5: no vertex in more than one path
            for (int j = 0; j < len; j++) {
                int v = paths[p][j];
                if (path_id[v] != -1) {
                    feasible = false;
                    break;
                }
                path_id[v] = p;
                path_pos[v] = j;
            }
            if (!feasible) {
                k = p + 1;
                break;
            }
        }

        long long case_score = 0;

        if (feasible) {
            // ---- Compute OBJ_you ----
            long long obj_you = 0;

            // sum weights of used vertices
            for (int v = 1; v <= n; v++)
                if (path_id[v] != -1) obj_you += w[v];

            // activation cost
            obj_you -= A * (long long)k;

            // chord count and cross counts
            map<pair<int,int>, long long> cross_count;
            for (auto& [u, v] : edges) {
                int pu = path_id[u], pv = path_id[v];
                if (pu == -1 || pv == -1) continue;
                if (pu == pv) {
                    // chord if not consecutive
                    if (abs(path_pos[u] - path_pos[v]) != 1)
                        obj_you -= B;
                } else {
                    int a = min(pu, pv), b = max(pu, pv);
                    cross_count[{a, b}]++;
                }
            }
            for (auto& [pr, cnt] : cross_count) {
                if (cnt > 1) obj_you -= C * (cnt - 1);
            }

            long long obj_nonneg = max(0LL, obj_you);

            // score = min(10^6, floor(500000 * (obj_nonneg+1) / (obj_base+1)))
            long long num = 500000LL * (obj_nonneg + 1);
            long long den = (obj_base + 1);
            case_score = num / den;
            if (case_score > 1000000LL) case_score = 1000000LL;
        }
        // If infeasible, case_score stays 0.

        total_score_sum += case_score;
    }

    // Check EOF of participant output
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra data after all test cases in participant output");
    }

    // Final score = floor(mean of case scores)
    long long final_score = total_score_sum / total_cases;
    if (final_score > 1000000LL) final_score = 1000000LL;

    double ratio = (double)final_score / 1000000.0;

    quitp(ratio, "Ratio: %.6f (final_score=%lld over %d cases)", ratio, final_score, total_cases);
}