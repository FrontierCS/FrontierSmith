#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

struct SimResult {
    bool feasible;
    long long V;
    long long B;
};

SimResult simulate(int n, int m,
                   const set<pair<int,int>>& anomalies,
                   const map<pair<int,int>, long long>& caches,
                   const map<pair<int,int>, long long>& smps,
                   long long s,
                   const string& route) {
    if ((long long)route.size() > 300000LL) return {false, 0, 0};

    long long battery = s;
    int r = 1, c = 1;
    long long V = 0;

    set<pair<int,int>> vis_cache;
    set<pair<int,int>> vis_smp;

    // Apply initial cell effects at (1,1)
    {
        auto pos = make_pair(r, c);
        if (anomalies.count(pos)) battery = battery / 2;
        if (caches.count(pos)) {
            vis_cache.insert(pos);
            battery += caches.at(pos);
        }
        if (smps.count(pos)) {
            vis_smp.insert(pos);
            V += smps.at(pos);
        }
    }

    for (char ch : route) {
        int nr = r, nc = c;
        if      (ch == 'U') nr--;
        else if (ch == 'D') nr++;
        else if (ch == 'L') nc--;
        else if (ch == 'R') nc++;
        else return {false, 0, 0};

        if (nr < 1 || nr > n || nc < 1 || nc > m) return {false, 0, 0};

        battery--;
        if (battery < 0) return {false, 0, 0};

        r = nr; c = nc;
        auto pos = make_pair(r, c);

        if (anomalies.count(pos)) battery = battery / 2;

        if (caches.count(pos) && !vis_cache.count(pos)) {
            vis_cache.insert(pos);
            battery += caches.at(pos);
        }

        if (smps.count(pos) && !vis_smp.count(pos)) {
            vis_smp.insert(pos);
            V += smps.at(pos);
        }
    }

    if (r != n || c != m) return {false, 0, 0};
    return {true, V, battery};
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    int n    = inf.readInt();
    int m    = inf.readInt();
    int a    = inf.readInt();
    int ccnt = inf.readInt();
    int p    = inf.readInt();
    long long s = inf.readLong();

    set<pair<int,int>> anomalies;
    for (int i = 0; i < a; i++) {
        int ri = inf.readInt();
        int ci = inf.readInt();
        anomalies.insert({ri, ci});
    }

    map<pair<int,int>, long long> caches;
    long long total_g = 0;
    for (int i = 0; i < ccnt; i++) {
        int ri = inf.readInt();
        int ci = inf.readInt();
        long long g = inf.readLong();
        caches[{ri, ci}] = g;
        total_g += g;
    }

    map<pair<int,int>, long long> smps;
    for (int i = 0; i < p; i++) {
        int ri = inf.readInt();
        int ci = inf.readInt();
        long long v = inf.readLong();
        smps[{ri, ci}] = v;
    }

    long long H = 1LL + s + total_g;

    // Compute baseline: right (m-1) times then down (n-1) times
    string baseline = string(m - 1, 'R') + string(n - 1, 'D');
    SimResult base_res = simulate(n, m, anomalies, caches, smps, s, baseline);
    // Problem guarantees baseline is always feasible
    long long BASE = base_res.V * H + base_res.B;

    // Read participant route (may be empty for n=m=1)
    string route = "";
    if (!ouf.seekEof()) {
        route = ouf.readToken();
    }
    // Ensure no extra tokens
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra output found after the route string. Ratio: 0.0");
    }

    SimResult res = simulate(n, m, anomalies, caches, smps, s, route);

    if (!res.feasible) {
        quitf(_wa, "Infeasible route (bad move, out of bounds, battery exhausted, or wrong endpoint). Ratio: 0.0");
    }

    long long OBJ = res.V * H + res.B;

    // score = floor(500000 * min(2, OBJ / BASE))
    // ratio for quitp = score / 1000000 = min(2, OBJ/BASE) / 2  (clamped to [0,1])
    double ratio;
    if (BASE <= 0) {
        // Should not happen per problem guarantee, but handle gracefully
        ratio = (OBJ >= 0) ? 1.0 : 0.0;
    } else {
        double raw = (double)OBJ / (double)BASE; // in [0, 2+]
        if (raw > 2.0) raw = 2.0;
        ratio = raw / 2.0; // map [0,2] -> [0,1]
    }
    // Clamp to [0,1]
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    long long score500k;
    if (BASE <= 0) {
        score500k = 500000;
    } else {
        double raw = (double)OBJ / (double)BASE;
        if (raw > 2.0) raw = 2.0;
        score500k = (long long)(500000.0 * raw);
    }

    quitp(ratio,
          "OBJ=%lld BASE=%lld H=%lld V=%lld B=%lld score=%lld Ratio: %.9f",
          OBJ, BASE, H, res.V, res.B, score500k, ratio);
}