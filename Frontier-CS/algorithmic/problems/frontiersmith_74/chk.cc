#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ── Read problem input (inf) ────────────────────────────────────────────
    long long n = inf.readLong();
    int       m = inf.readInt();
    int       p = inf.readInt();
    long long K = inf.readLong();

    vector<long long> rs(m+1), rt(m+1), rc(m+1), rf(m+1);
    for (int i = 1; i <= m; i++) {
        rs[i] = inf.readLong();
        rt[i] = inf.readLong();
        rc[i] = inf.readLong();
        rf[i] = inf.readLong();
    }

    vector<long long> ga(p+1), gb(p+1), gq(p+1), gr(p+1);
    for (int j = 1; j <= p; j++) {
        ga[j] = inf.readLong();
        gb[j] = inf.readLong();
        gq[j] = inf.readLong();
        gr[j] = inf.readLong();
    }

    // ── Compute baseline B ──────────────────────────────────────────────────
    // Index routes by endpoint
    unordered_map<long long, vector<int>> routes_by_t;
    routes_by_t.reserve(m * 2 + 4);
    for (int i = 1; i <= m; i++)
        routes_by_t[rt[i]].push_back(i);

    // For each group j: preferred direct route = min f_i (tie: min i)
    // where s_i <= a_j, t_i == b_j, c_i >= q_j
    vector<int> pref(p+1, -1);
    for (int j = 1; j <= p; j++) {
        auto it = routes_by_t.find(gb[j]);
        if (it == routes_by_t.end()) continue;
        for (int i : it->second) {
            if (rs[i] <= ga[j] && rc[i] >= gq[j]) {
                if (pref[j] == -1
                    || rf[i] < rf[pref[j]]
                    || (rf[i] == rf[pref[j]] && i < pref[j]))
                    pref[j] = i;
            }
        }
    }

    // Collect non-ignored groups and sort for baseline
    vector<int> base_grps;
    base_grps.reserve(p);
    for (int j = 1; j <= p; j++)
        if (pref[j] != -1) base_grps.push_back(j);

    // Sort: decreasing r_j / f_pref, then decreasing r_j, then increasing j
    sort(base_grps.begin(), base_grps.end(), [&](int a, int b) {
        // r_a / f_pa  vs  r_b / f_pb  <==>  r_a * f_pb  vs  r_b * f_pa
        // use __int128 to avoid overflow
        __int128 lhs = (__int128)gr[a] * rf[pref[b]];
        __int128 rhs = (__int128)gr[b] * rf[pref[a]];
        if (lhs != rhs) return lhs > rhs;
        if (gr[a] != gr[b]) return gr[a] > gr[b];
        return a < b;
    });

    // Simulate baseline
    long long R = K;
    vector<long long> rem_cap(m+1);
    for (int i = 1; i <= m; i++) rem_cap[i] = rc[i];
    vector<bool> active(m+1, false);

    long long B = 0;
    for (int j : base_grps) {
        int i = pref[j];
        if (rem_cap[i] < gq[j]) continue;
        if (!active[i] && rf[i] > R) continue;
        // serve group j
        if (!active[i]) {
            active[i] = true;
            R -= rf[i];
        }
        rem_cap[i] -= gq[j];
        B += gr[j];
    }

    // ── Read participant output (ouf) ───────────────────────────────────────
    // z: number of served groups
    int z = ouf.readInt(-1, p, "z (number of served groups)");

    vector<bool>           served(p+1, false);
    vector<vector<int>>    groutes(p+1);
    vector<long long>      route_load(m+1, 0);
    set<int>               used_routes;

    long long O = 0;

    for (int t = 0; t < z; t++) {
        int g = ouf.readInt(1, p, "group ID");
        if (served[g])
            quitf(_wa, "Group %d appears more than once in output", g);

        int k = ouf.readInt(1, 20, "k (number of routes for group)");

        vector<int> rseq(k);
        for (int l = 0; l < k; l++) {
            rseq[l] = ouf.readInt(1, m, "route ID");
        }

        served[g] = true;
        groutes[g] = rseq;

        // Validate travel path
        long long cur = ga[g];
        for (int l = 0; l < k; l++) {
            int ri = rseq[l];
            if (rs[ri] > cur)
                quitf(_wa,
                    "Group %d step %d: route %d starts at %lld, "
                    "cannot board from current stop %lld",
                    g, l+1, ri, rs[ri], cur);
            if (cur >= rt[ri])
                quitf(_wa,
                    "Group %d step %d: current stop %lld >= route %d end %lld",
                    g, l+1, cur, ri, rt[ri]);
            cur = rt[ri];
        }
        if (cur != gb[g])
            quitf(_wa,
                "Group %d: path ends at stop %lld but destination is %lld",
                g, cur, gb[g]);

        // Accumulate loads
        for (int l = 0; l < k; l++) {
            int ri = rseq[l];
            route_load[ri] += gq[g];
            used_routes.insert(ri);
        }

        O += gr[g];
    }

    // ── Feasibility checks ──────────────────────────────────────────────────

    // Capacity constraints
    for (int i = 1; i <= m; i++) {
        if (route_load[i] > rc[i])
            quitf(_wa,
                "Route %d capacity exceeded: load %lld > capacity %lld",
                i, route_load[i], rc[i]);
    }

    // Budget constraint
    long long total_cost = 0;
    for (int i : used_routes) {
        total_cost += rf[i];
    }
    if (total_cost > K)
        quitf(_wa,
            "Budget exceeded: total route cost %lld > K = %lld",
            total_cost, K);

    // ── Score: continuous ratio O / (O + B), clamped to [0, 1] ────────────
    double ratio;
    if (O == 0 && B == 0) {
        ratio = 0.0;
    } else {
        ratio = (double)O / (double)(O + B);
    }
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    quitp(ratio,
        "Participant O=%lld, Baseline B=%lld, Ratio: %.9f",
        O, B, ratio);

    return 0;
}