#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

// Simulate one full lap on a 1-indexed grid of size r x c.
static int one_lap(const vector<vector<int>>& g, int r, int c, int s) {
    int cur = s;
    for (int col = 1; col <= c; col++) {
        int nc = col % c + 1;
        int ru = (cur - 2 + r) % r + 1;
        int rd = cur % r + 1;
        int es = g[cur][nc];
        int eu = g[ru][nc];
        int ed = g[rd][nc];
        // tie-break: straight > up-right > down-right
        int best = es;
        int next = cur;  // straight
        if (eu > best) { best = eu; next = ru; }
        if (ed > best) { /* best = ed; */ next = rd; }
        cur = next;
    }
    return cur;
}

static vector<int> compute_perm(const vector<vector<int>>& g, int r, int c) {
    vector<int> perm(r + 1);
    for (int s = 1; s <= r; s++)
        perm[s] = one_lap(g, r, c, s);
    return perm;
}

static vector<vector<int>> build_lift(const vector<int>& perm, int r) {
    const int LOG = 31;
    vector<vector<int>> lift(LOG, vector<int>(r + 1, 0));
    for (int s = 1; s <= r; s++) lift[0][s] = perm[s];
    for (int k = 1; k < LOG; k++)
        for (int s = 1; s <= r; s++)
            lift[k][s] = lift[k-1][lift[k-1][s]];
    return lift;
}

static int apply_lift(const vector<vector<int>>& lift, long long laps, int s) {
    for (int k = 0; k < (int)lift.size(); k++)
        if ((laps >> k) & 1LL)
            s = lift[k][s];
    return s;
}

static int circ_dist(int f, int t, int r) {
    int d1 = (f - t + r) % r;
    int d2 = (t - f + r) % r;
    return min(d1, d2);
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ── Read problem input ──────────────────────────────────────────────────
    int r = inf.readInt();
    int c = inf.readInt();
    int B = inf.readInt();

    vector<vector<int>> grid(r + 1, vector<int>(c + 1));
    for (int i = 1; i <= r; i++)
        for (int j = 1; j <= c; j++)
            grid[i][j] = inf.readInt();

    int n = inf.readInt();
    vector<int> sv(n), tv(n), lv_arr(n), wv(n);
    for (int i = 0; i < n; i++) {
        sv[i] = inf.readInt();
        tv[i] = inf.readInt();
        lv_arr[i] = inf.readInt();
        wv[i] = inf.readInt();
    }

    // ── Read participant output ─────────────────────────────────────────────
    // If anything is malformed, treat as infeasible (V_edit = 0)
    bool infeasible = false;
    string infeasible_reason = "";

    int q = 0;
    vector<tuple<int,int,int>> edits;

    // Try to parse q
    if (ouf.seekEof()) {
        // Empty output
        infeasible = true;
        infeasible_reason = "empty output";
    } else {
        // Read q
        q = ouf.readInt(-1000000000, 1000000000, "q");
        if (q < 0 || q > B) {
            infeasible = true;
            infeasible_reason = "q out of range: " + to_string(q);
        } else {
            // Read q edit lines
            set<pair<int,int>> seen;
            for (int i = 0; i < q && !infeasible; i++) {
                if (ouf.seekEof()) {
                    infeasible = true;
                    infeasible_reason = "unexpected EOF reading edit " + to_string(i);
                    break;
                }
                int a = ouf.readInt(-1000000000, 1000000000, "a");
                int b = ouf.readInt(-1000000000, 1000000000, "b");
                int e = ouf.readInt(-1000000000, 1000000000, "e");
                if (a < 1 || a > r) {
                    infeasible = true;
                    infeasible_reason = "row out of range: " + to_string(a);
                    break;
                }
                if (b < 1 || b > c) {
                    infeasible = true;
                    infeasible_reason = "col out of range: " + to_string(b);
                    break;
                }
                if (e < 1 || e > 1000000000) {
                    infeasible = true;
                    infeasible_reason = "elevation out of range: " + to_string(e);
                    break;
                }
                if (!seen.insert({a, b}).second) {
                    infeasible = true;
                    infeasible_reason = "duplicate cell (" + to_string(a) + "," + to_string(b) + ")";
                    break;
                }
                edits.push_back({a, b, e});
            }
        }
    }
    // Drain remaining output (don't enforce EOF strictly; trailing whitespace/newlines are ok)
    // We just ignore extra content silently

    // ── Apply edits ─────────────────────────────────────────────────────────
    vector<vector<int>> edited = grid;
    if (!infeasible) {
        for (auto& [a, b, e] : edits)
            edited[a][b] = e;
    }

    // ── Compute permutations and binary lifts ────────────────────────────────
    vector<int> base_perm = compute_perm(grid, r, c);
    vector<vector<int>> lift_base = build_lift(base_perm, r);

    vector<vector<int>> lift_edit;
    if (!infeasible) {
        vector<int> edit_perm = compute_perm(edited, r, c);
        lift_edit = build_lift(edit_perm, r);
    }

    int H = r / 2;

    // ── Compute V_base, V_edit, U ────────────────────────────────────────────
    long long V_base = 0, V_edit = 0, U = 0;
    for (int i = 0; i < n; i++) {
        long long hp1 = (long long)(H + 1);
        U += (long long)wv[i] * hp1;

        long long laps = (long long)lv_arr[i];
        int fb = apply_lift(lift_base, laps, sv[i]);
        V_base += (long long)wv[i] * (hp1 - circ_dist(fb, tv[i], r));

        if (!infeasible) {
            int fe = apply_lift(lift_edit, laps, sv[i]);
            V_edit += (long long)wv[i] * (hp1 - circ_dist(fe, tv[i], r));
        }
        // If infeasible, V_edit stays 0
    }

    // ── Score computation ────────────────────────────────────────────────────
    // score = 10^6 * max(0, V - V_base) / (U - V_base), clamped to [0, 1]
    if (U == V_base) {
        // Baseline already optimal; any feasible output (including no-edit) is perfect
        quitp(1.0,
              "Ratio: 1.000000000 (baseline already optimal; V_base=%lld U=%lld)",
              V_base, U);
    }

    long long V_used = infeasible ? 0LL : V_edit;

    double num = (double)(V_used - V_base);
    double den = (double)(U - V_base);
    double ratio = num / den;
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    if (infeasible) {
        // V_used=0, so ratio = (0 - V_base)/(U - V_base) which may be negative -> clamped to 0
        quitp(ratio,
              "Ratio: %.9f (infeasible: %s; V=0 V_base=%lld U=%lld)",
              ratio, infeasible_reason.c_str(), V_base, U);
    }

    quitp(ratio,
          "Ratio: %.9f (V=%lld V_base=%lld U=%lld q=%d)",
          ratio, V_edit, V_base, U, q);

    return 0;
}