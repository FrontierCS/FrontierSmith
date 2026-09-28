#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- Read problem input from inf ----
    int m = inf.readInt();
    int n = inf.readInt();
    long long K_cap = inf.readLong();
    int C_cap = inf.readInt();

    vector<long long> X(m + 1), Y(m + 1);
    for (int j = 1; j <= m; j++) {
        X[j] = inf.readLong();
        Y[j] = inf.readLong();
    }

    vector<int> S(n + 1), W(n + 1), U(n + 1);
    for (int i = 1; i <= n; i++) {
        S[i] = inf.readInt();
        W[i] = inf.readInt();
        U[i] = inf.readInt();
    }

    auto mdist = [&](int a, int b) -> long long {
        return llabs(X[a] - X[b]) + llabs(Y[a] - Y[b]);
    };

    // ---- Parse participant output from ouf ----
    if (ouf.seekEof()) quitf(_wa, "output is empty");

    int q = ouf.readInt();
    if (q < 0 || q > 300000)
        quitf(_wa, "q=%d out of range [0,300000]", q);

    long long cur_time = 0;
    int cur_dock = 1;
    long long onboard_weight = 0;
    int onboard_count = 0;

    // person_state: 0=waiting at dock, 1=on ferry, 2=evacuated
    vector<int> pstate(n + 1, 0);
    vector<long long> evac_time(n + 1, 0);

    for (int step = 0; step < q; step++) {
        string cmd = ouf.readToken();

        if (cmd == "MOVE") {
            int v = ouf.readInt();
            if (v < 1 || v > m)
                quitf(_wa, "MOVE: dock %d out of range [1,%d]", v, m);
            if (v == cur_dock)
                quitf(_wa, "MOVE: destination %d equals current dock %d", v, cur_dock);
            cur_time += mdist(cur_dock, v);
            cur_dock = v;

        } else if (cmd == "BOARD") {
            int t = ouf.readInt();
            if (t < 0)
                quitf(_wa, "BOARD: count %d is negative", t);
            if (t == 0) continue;

            vector<int> ids(t);
            for (int k = 0; k < t; k++) {
                ids[k] = ouf.readInt();
                if (ids[k] < 1 || ids[k] > n)
                    quitf(_wa, "BOARD: person id %d out of range [1,%d]", ids[k], n);
            }

            // Check uniqueness within this BOARD action
            {
                vector<int> tmp = ids;
                sort(tmp.begin(), tmp.end());
                for (int k = 1; k < (int)tmp.size(); k++)
                    if (tmp[k] == tmp[k-1])
                        quitf(_wa, "BOARD: duplicate person id %d in same action", tmp[k]);
            }

            // Validate each person
            long long new_weight = onboard_weight;
            int new_count = onboard_count;
            for (int id : ids) {
                if (pstate[id] != 0)
                    quitf(_wa, "BOARD: person %d is not waiting (state=%d)", id, pstate[id]);
                if (S[id] != cur_dock)
                    quitf(_wa, "BOARD: person %d is at dock %d, ferry is at dock %d", id, S[id], cur_dock);
                new_weight += W[id];
                new_count++;
            }
            if (new_count > C_cap)
                quitf(_wa, "BOARD: would exceed capacity (%d > %d)", new_count, C_cap);
            if (new_weight > K_cap)
                quitf(_wa, "BOARD: would exceed weight limit (%lld > %lld)", new_weight, K_cap);

            for (int id : ids) pstate[id] = 1;
            onboard_weight = new_weight;
            onboard_count = new_count;

        } else if (cmd == "UNLOAD") {
            if (cur_dock != 1)
                quitf(_wa, "UNLOAD: ferry is at dock %d, not dock 1", cur_dock);

            int t = ouf.readInt();
            if (t < 0)
                quitf(_wa, "UNLOAD: count %d is negative", t);
            if (t == 0) continue;

            vector<int> ids(t);
            for (int k = 0; k < t; k++) {
                ids[k] = ouf.readInt();
                if (ids[k] < 1 || ids[k] > n)
                    quitf(_wa, "UNLOAD: person id %d out of range [1,%d]", ids[k], n);
            }

            // Check uniqueness within this UNLOAD action
            {
                vector<int> tmp = ids;
                sort(tmp.begin(), tmp.end());
                for (int k = 1; k < (int)tmp.size(); k++)
                    if (tmp[k] == tmp[k-1])
                        quitf(_wa, "UNLOAD: duplicate person id %d in same action", tmp[k]);
            }

            for (int id : ids) {
                if (pstate[id] != 1)
                    quitf(_wa, "UNLOAD: person %d is not on ferry (state=%d)", id, pstate[id]);
                pstate[id] = 2;
                evac_time[id] = cur_time;
                onboard_weight -= W[id];
                onboard_count--;
            }

        } else {
            quitf(_wa, "Unknown action: %s", cmd.c_str());
        }
    }

    // After reading all q actions, check EOF
    if (!ouf.seekEof())
        quitf(_wa, "Extra output after %d actions", q);

    // Check all people evacuated
    for (int i = 1; i <= n; i++) {
        if (pstate[i] != 2)
            quitf(_wa, "Person %d was not evacuated (state=%d)", i, pstate[i]);
    }

    // Check ferry is empty
    if (onboard_count != 0)
        quitf(_wa, "Ferry is not empty at end (%d people remain)", onboard_count);

    // Compute participant objective
    long long participant_F = 0;
    for (int i = 1; i <= n; i++) {
        participant_F += (long long)U[i] * evac_time[i];
    }

    // ---- Compute baseline objective ----
    // Baseline: greedy one-dock round trips using ratio = sum_u / (2*dist(1,j))
    // Ties broken by smaller dock index
    // Sort each dock's waiting list by (larger u first, smaller index first)

    vector<vector<int>> dock_waiting(m + 1);
    for (int i = 1; i <= n; i++) dock_waiting[S[i]].push_back(i);
    for (int j = 2; j <= m; j++) {
        sort(dock_waiting[j].begin(), dock_waiting[j].end(), [&](int a, int b) {
            if (U[a] != U[b]) return U[a] > U[b];
            return a < b;
        });
    }

    long long baseline_F = 0;
    long long bl_time = 0;
    vector<bool> bl_evacuated(n + 1, false);
    int remaining = n;

    while (remaining > 0) {
        // For each non-empty dock, compute prefix and ratio
        int best_dock = -1;
        // We'll use cross-multiplication for exact comparison
        // ratio_j = value_j / cost_j => compare a/b vs c/d as a*d vs c*b
        long long best_val = -1, best_cost = -1;

        for (int j = 2; j <= m; j++) {
            if (dock_waiting[j].empty()) continue;

            long long trip_dist = mdist(1, j);
            long long cost_j = 2 * trip_dist;
            if (cost_j == 0) cost_j = 1; // degenerate: dock at same location as shelter

            // Build prefix P_j
            long long total_weight = 0;
            int count = 0;
            long long value_j = 0;
            for (int idx : dock_waiting[j]) {
                if (count + 1 > C_cap) break;
                if (total_weight + W[idx] > K_cap) break;
                count++;
                total_weight += W[idx];
                value_j += U[idx];
            }
            if (count == 0) continue;

            // Compare value_j / cost_j vs best_val / best_cost
            // value_j / cost_j > best_val / best_cost
            // <=> value_j * best_cost > best_val * cost_j
            bool is_better = false;
            if (best_dock == -1) {
                is_better = true;
            } else {
                long long lhs = value_j * best_cost;
                long long rhs = best_val * cost_j;
                if (lhs > rhs) is_better = true;
                else if (lhs == rhs && j < best_dock) is_better = true;
            }

            if (is_better) {
                best_dock = j;
                best_val = value_j;
                best_cost = cost_j;
            }
        }

        if (best_dock == -1) break; // no valid dock (shouldn't happen)

        // Execute trip to best_dock
        long long trip_dist = mdist(1, best_dock);
        bl_time += trip_dist; // go to dock

        // Board people in prefix
        long long total_weight = 0;
        int count = 0;
        vector<int> boarded;
        for (int idx : dock_waiting[best_dock]) {
            if (count + 1 > C_cap) break;
            if (total_weight + W[idx] > K_cap) break;
            boarded.push_back(idx);
            count++;
            total_weight += W[idx];
        }

        bl_time += trip_dist; // return to dock 1

        for (int idx : boarded) {
            baseline_F += (long long)U[idx] * bl_time;
            bl_evacuated[idx] = true;
        }
        remaining -= (int)boarded.size();

        // Remove boarded from dock_waiting[best_dock]
        vector<int> new_waiting;
        for (int idx : dock_waiting[best_dock]) {
            bool found = false;
            for (int b : boarded) if (b == idx) { found = true; break; }
            if (!found) new_waiting.push_back(idx);
        }
        dock_waiting[best_dock] = new_waiting;
    }

    // ---- Score = min(200, 100 * baseline_F / participant_F) ----
    // ratio passed to quitp in [0,1]: ratio = score / 200
    // score = min(200, 100 * B / Y)
    // => ratio = min(1.0, 0.5 * B / Y)

    double score_ratio;
    if (participant_F <= 0) {
        // All urgencies 0 or n=0, both are 0 => perfect
        score_ratio = 1.0;
    } else if (baseline_F <= 0) {
        score_ratio = 1.0;
    } else {
        double raw = 0.5 * (double)baseline_F / (double)participant_F;
        score_ratio = raw < 0.0 ? 0.0 : (raw > 1.0 ? 1.0 : raw);
    }

    quitp(score_ratio, "Ratio: %.9f (participant_F=%lld, baseline_F=%lld)",
          score_ratio, participant_F, baseline_F);

    return 0;
}