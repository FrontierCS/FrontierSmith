#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    int T = inf.readInt();

    double total_ratio = 0.0;
    int total_cases = 0;

    for (int tc = 0; tc < T; tc++) {
        int n, m, K;
        long long B;
        n = inf.readInt();
        m = inf.readInt();
        K = inf.readInt();
        B = inf.readLong();

        vector<long long> gx(n), gy(n), gt(n), gp(n);
        for (int i = 0; i < n; i++) {
            gx[i] = inf.readLong();
            gy[i] = inf.readLong();
            gt[i] = inf.readLong();
            gp[i] = inf.readLong();
        }

        vector<long long> hu(m), hv(m), hs(m), hr(m), hc(m);
        for (int j = 0; j < m; j++) {
            hu[j] = inf.readLong();
            hv[j] = inf.readLong();
            hs[j] = inf.readLong();
            hr[j] = inf.readLong();
            hc[j] = inf.readLong();
        }

        // Compute baseline F_base
        // For each hub j with c_j <= B, assign all groups sorted by arrival time (ties: smaller group index)
        double F_base = 1e300;
        int best_hub_idx = -1;

        for (int j = 0; j < m; j++) {
            if (hc[j] > B) continue;

            // compute arrivals for all groups
            vector<pair<long long,int>> order(n);
            for (int i = 0; i < n; i++) {
                long long dist = abs(gx[i] - hu[j]) + abs(gy[i] - hv[j]);
                long long arr = gt[i] + dist;
                order[i] = {arr, i};
            }
            sort(order.begin(), order.end(), [](const pair<long long,int>&a, const pair<long long,int>&b){
                if (a.first != b.first) return a.first < b.first;
                return a.second < b.second;
            });

            double cur = (double)hs[j];
            double obj = 0.0;
            for (int k = 0; k < n; k++) {
                int gi = order[k].second;
                double arr = (double)(gt[gi] + abs(gx[gi] - hu[j]) + abs(gy[gi] - hv[j]));
                double start = max(cur, arr);
                double finish = start + (double)gp[gi] / (double)hr[j];
                cur = finish;
                obj = max(obj, finish);
            }

            if (obj < F_base || (obj == F_base && j < best_hub_idx)) {
                F_base = obj;
                best_hub_idx = j;
            }
        }

        // Now read participant output
        int q = ouf.readInt(1, m, "number of opened hubs q");

        if (q > K) {
            quitf(_wa, "Test case %d: q=%d exceeds K=%d", tc+1, q, K);
        }

        vector<int> hub_indices(q);
        vector<vector<int>> hub_groups(q);
        set<int> used_hubs;
        vector<bool> group_used(n, false);
        long long total_cost = 0;

        for (int qi = 0; qi < q; qi++) {
            int j = ouf.readInt(1, m, "hub index");
            if (used_hubs.count(j)) {
                quitf(_wa, "Test case %d: hub %d appears more than once", tc+1, j);
            }
            used_hubs.insert(j);
            hub_indices[qi] = j - 1; // 0-based
            total_cost += hc[j-1];

            int cnt = ouf.readInt(1, n, "cnt");
            hub_groups[qi].resize(cnt);
            for (int k = 0; k < cnt; k++) {
                int g = ouf.readInt(1, n, "group index");
                if (group_used[g-1]) {
                    quitf(_wa, "Test case %d: group %d appears more than once", tc+1, g);
                }
                group_used[g-1] = true;
                hub_groups[qi][k] = g - 1; // 0-based
            }
        }

        // Check all groups assigned
        for (int i = 0; i < n; i++) {
            if (!group_used[i]) {
                quitf(_wa, "Test case %d: group %d not assigned to any hub", tc+1, i+1);
            }
        }

        // Check cost
        if (total_cost > B) {
            quitf(_wa, "Test case %d: total cost %lld exceeds budget %lld", tc+1, total_cost, B);
        }

        // Check EOF for this test case... we'll check overall EOF after loop

        // Compute participant's objective F
        double F_part = 0.0;
        for (int qi = 0; qi < q; qi++) {
            int j = hub_indices[qi]; // 0-based hub
            double cur = (double)hs[j];
            for (int k = 0; k < (int)hub_groups[qi].size(); k++) {
                int gi = hub_groups[qi][k];
                double arr = (double)(gt[gi] + abs(gx[gi] - hu[j]) + abs(gy[gi] - hv[j]));
                double start = max(cur, arr);
                double finish = start + (double)gp[gi] / (double)hr[j];
                cur = finish;
                F_part = max(F_part, finish);
            }
        }

        // Compute ratio for this test case
        // score_case = 100 * min(2, F_base / F_part)
        // ratio = score_case / 200 so that max ratio=1 when F_part <= F_base/2
        double ratio;
        if (F_part <= 0.0) {
            ratio = 1.0;
        } else {
            double improvement = F_base / F_part;
            double score_case = 100.0 * min(2.0, improvement);
            ratio = score_case / 200.0;
        }
        ratio = max(0.0, min(1.0, ratio));

        total_ratio += ratio;
        total_cases++;
    }

    // Check EOF
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra content in output after all test cases");
    }

    // Average ratio across test cases
    double avg_ratio = (total_cases > 0) ? (total_ratio / total_cases) : 0.0;
    avg_ratio = max(0.0, min(1.0, avg_ratio));

    quitp(avg_ratio, "Ratio: %.6f (avg over %d test cases)", avg_ratio, total_cases);
    return 0;
}