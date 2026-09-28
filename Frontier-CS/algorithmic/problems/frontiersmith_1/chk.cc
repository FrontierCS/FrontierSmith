#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

typedef long long ll;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    int t = inf.readInt();

    double sum_ratio = 0.0;

    for (int tc = 0; tc < t; tc++) {
        // --- read problem parameters from inf ---
        int n = inf.readInt();
        int q = inf.readInt();
        int k = inf.readInt();
        ll  m = inf.readLong();

        vector<int> p(q);
        vector<char> ctype(q);
        vector<ll> b(q), w(q), d(q);

        for (int j = 0; j < q; j++) {
            p[j] = inf.readInt();
            string cs = inf.readToken();
            ctype[j] = cs[0];
            b[j] = inf.readLong();
            w[j] = inf.readLong();
            d[j] = inf.readLong();
        }

        // --- reward helper ---
        auto reward_val = [&](int j, ll pred) -> ll {
            ll diff = pred - b[j];
            if (diff < 0) diff = -diff;
            if (diff > d[j]) return 0LL;
            return w[j] * (d[j] + 1 - diff);
        };

        // --- compute baseline B ---
        // helper: given prefix sums pr[1..n] and suffix sums su[1..n], compute best obj
        auto compute_obj = [&](vector<ll>& pr, vector<ll>& su) -> ll {
            ll obj = 0;
            for (int j = 0; j < q; j++) {
                if (ctype[j] == 'P') {
                    obj += reward_val(j, pr[p[j]]);
                } else if (ctype[j] == 'S') {
                    obj += reward_val(j, su[p[j]]);
                } else {
                    ll rP = reward_val(j, pr[p[j]]);
                    ll rS = reward_val(j, su[p[j]]);
                    obj += max(rP, rS);
                }
            }
            return obj;
        };

        // Zero array
        ll B = 0;
        {
            vector<ll> pr(n + 1, 0), su(n + 1, 0);
            B = max(B, compute_obj(pr, su));
        }

        // Single-spike arrays: a[x] = +m or -m, rest zero
        for (int x = 1; x <= n; x++) {
            for (int sign = -1; sign <= 1; sign += 2) {
                ll v = (ll)sign * m;
                // pref[i] = 0 if i < x, else v
                // suff[i] = 0 if i > x, else v
                ll obj = 0;
                for (int j = 0; j < q; j++) {
                    ll predP = (p[j] >= x) ? v : 0LL;
                    ll predS = (p[j] <= x) ? v : 0LL;
                    if (ctype[j] == 'P') {
                        obj += reward_val(j, predP);
                    } else if (ctype[j] == 'S') {
                        obj += reward_val(j, predS);
                    } else {
                        ll rP = reward_val(j, predP);
                        ll rS = reward_val(j, predS);
                        obj += max(rP, rS);
                    }
                }
                B = max(B, obj);
            }
        }

        // --- read participant output ---
        // We'll compute OBJ; on any feasibility violation, OBJ=0 for this test case
        ll OBJ = 0;
        bool feasible = true;
        string feasibility_reason = "";

        vector<ll> a(n + 1, 0);
        for (int i = 1; i <= n; i++) {
            if (ouf.seekEof()) {
                feasible = false;
                feasibility_reason = "premature EOF reading array";
                break;
            }
            a[i] = ouf.readLong();
        }

        string o_str = "";
        if (feasible) {
            if (ouf.seekEof()) {
                feasible = false;
                feasibility_reason = "premature EOF reading directions";
            } else {
                o_str = ouf.readToken();
            }
        }

        if (feasible) {
            // Check |a_i| <= m
            for (int i = 1; i <= n && feasible; i++) {
                if (a[i] < -m || a[i] > m) {
                    feasible = false;
                    feasibility_reason = "a[i] out of range";
                }
            }
        }

        if (feasible) {
            // Check non-zero count
            int nz = 0;
            for (int i = 1; i <= n; i++) if (a[i] != 0) nz++;
            if (nz > k) {
                feasible = false;
                feasibility_reason = "too many non-zero cells";
            }
        }

        if (feasible) {
            // Check direction string length
            if ((int)o_str.size() != q) {
                feasible = false;
                feasibility_reason = "direction string wrong length";
            }
        }

        if (feasible) {
            // Check direction characters and fixed constraints
            for (int j = 0; j < q && feasible; j++) {
                char oc = o_str[j];
                if (oc != 'P' && oc != 'S') {
                    feasible = false;
                    feasibility_reason = "invalid direction char";
                } else if (ctype[j] == 'P' && oc != 'P') {
                    feasible = false;
                    feasibility_reason = "fixed P reading assigned S";
                } else if (ctype[j] == 'S' && oc != 'S') {
                    feasible = false;
                    feasibility_reason = "fixed S reading assigned P";
                }
            }
        }

        if (feasible) {
            // Compute prefix and suffix sums
            vector<ll> pref(n + 2, 0), suff(n + 2, 0);
            for (int i = 1; i <= n; i++) pref[i] = pref[i-1] + a[i];
            for (int i = n; i >= 1; i--) suff[i] = suff[i+1] + a[i];

            for (int j = 0; j < q; j++) {
                ll pred;
                if (o_str[j] == 'P') pred = pref[p[j]];
                else pred = suff[p[j]];
                OBJ += reward_val(j, pred);
            }
        } else {
            // Infeasible: OBJ = 0
            OBJ = 0;
        }

        // Compute per-test-case ratio: floor(1e6 * min(5, (OBJ+1)/(B+1))) / 1e6
        // Then as a fraction of 1: divide by 5 since cap is 5x
        double raw = (double)(OBJ + 1) / (double)(B + 1);
        if (raw > 5.0) raw = 5.0;
        if (raw < 0.0) raw = 0.0;
        // Per problem: score = floor(1_000_000 * min(5, raw)) / 1_000_000
        // Normalized to [0,1]: raw/5
        double ratio = raw / 5.0;
        if (ratio < 0.0) ratio = 0.0;
        if (ratio > 1.0) ratio = 1.0;
        sum_ratio += ratio;
    }

    double final_ratio = sum_ratio / (double)t;
    if (final_ratio < 0.0) final_ratio = 0.0;
    if (final_ratio > 1.0) final_ratio = 1.0;

    quitp(final_ratio, "Ratio: %.9f", final_ratio);

    return 0;
}