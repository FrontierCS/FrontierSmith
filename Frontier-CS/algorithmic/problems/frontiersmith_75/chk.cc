#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ------------------------------------------------------------------ //
    // 1. Read the input file (inf)
    // ------------------------------------------------------------------ //
    int N = inf.readInt();
    int S = inf.readInt();
    int B = inf.readInt();

    struct Spell { int u, v, t; long long w; };
    vector<int> par(S + 1, 0);
    vector<vector<Spell>> spells(S + 1);
    vector<vector<int>> children(S + 1);
    long long W = 0;

    for (int i = 1; i <= S; i++) {
        par[i] = inf.readInt();
        int ci = inf.readInt();
        if (i > 1) children[par[i]].push_back(i);
        for (int j = 0; j < ci; j++) {
            Spell sp;
            sp.u = inf.readInt();
            sp.v = inf.readInt();
            sp.t = inf.readInt();
            sp.w = (long long)inf.readInt();
            spells[i].push_back(sp);
            W += sp.w;
        }
    }

    // ------------------------------------------------------------------ //
    // 2. Compute OBJ_base: all hubs = 0, no retunes
    // ------------------------------------------------------------------ //
    long long OBJ_base = 0;
    {
        // all elem = 0; spell satisfied iff (0 + t) % 5 == 0, never for t in {1,2}
        for (int i = 1; i <= S; i++) {
            for (auto& sp : spells[i]) {
                if ((0 + sp.t) % 5 == 0) {
                    OBJ_base += sp.w;
                }
            }
        }
    }

    // ------------------------------------------------------------------ //
    // 3. Read participant output (ouf)
    // ------------------------------------------------------------------ //

    // Line 1: root assignment
    vector<int> root_elem(N + 1);
    for (int u = 1; u <= N; u++) {
        root_elem[u] = ouf.readInt(0, 4,
            ("root element of hub " + to_string(u) + " must be in [0,4]").c_str());
    }

    // Line 2: number of retunes
    int R = ouf.readInt();
    if (R < 0 || R > B) {
        quitf(_wa, "R=%d is out of valid range [0,%d]", R, B);
    }

    // Read retune lines
    // Check (s,u) uniqueness
    map<pair<int,int>, int> retune_map; // (state, hub) -> element
    vector<tuple<int,int,int>> state_retune_list; // all retunes

    // Per-state retunes
    vector<vector<pair<int,int>>> state_retunes(S + 1);

    for (int r = 0; r < R; r++) {
        int s = ouf.readInt();
        int u = ouf.readInt();
        int e = ouf.readInt();

        if (s < 2 || s > S) {
            quitf(_wa, "retune #%d: state s=%d out of range [2,%d]", r + 1, s, S);
        }
        if (u < 1 || u > N) {
            quitf(_wa, "retune #%d: hub u=%d out of range [1,%d]", r + 1, u, N);
        }
        if (e < 0 || e > 4) {
            quitf(_wa, "retune #%d: element e=%d out of range [0,4]", r + 1, e);
        }
        auto key = make_pair(s, u);
        if (retune_map.count(key)) {
            quitf(_wa, "retune #%d: duplicate (s=%d, u=%d)", r + 1, s, u);
        }
        retune_map[key] = e;
        state_retunes[s].push_back({u, e});
    }

    // ------------------------------------------------------------------ //
    // 3b. Strict EOF check — reject trailing garbage
    // ------------------------------------------------------------------ //
    if (!ouf.seekEof()) {
        quitf(_wa, "extra output found after the last retune line");
    }

    // ------------------------------------------------------------------ //
    // 4. Evaluate objective via DFS over the state tree
    // ------------------------------------------------------------------ //
    long long OBJ = 0;
    vector<int> elem(N + 1);
    for (int u = 1; u <= N; u++) elem[u] = root_elem[u];

    // Undo stacks per state
    vector<vector<pair<int,int>>> undo_info(S + 1);

    // Iterative DFS
    stack<pair<int,bool>> stk;
    stk.push({1, false});

    while (!stk.empty()) {
        auto [state, leaving] = stk.top();
        stk.pop();

        if (leaving) {
            // Undo retunes applied when entering this state
            for (auto& [hub, old_e] : undo_info[state]) {
                elem[hub] = old_e;
            }
            undo_info[state].clear();
        } else {
            // Push leaving marker
            stk.push({state, true});

            // Apply retunes (only valid for states 2..S per feasibility)
            for (auto& [hub, new_e] : state_retunes[state]) {
                undo_info[state].push_back({hub, elem[hub]});
                elem[hub] = new_e;
            }

            // Evaluate spells in this state
            for (auto& sp : spells[state]) {
                if ((elem[sp.u] + sp.t) % 5 == elem[sp.v]) {
                    OBJ += sp.w;
                }
            }

            // Push children
            for (int child : children[state]) {
                stk.push({child, false});
            }
        }
    }

    // ------------------------------------------------------------------ //
    // 5. Score according to the problem formula
    // ------------------------------------------------------------------ //
    // score = 1000000 * max(0, min(1, (OBJ - OBJ_base) / (W - OBJ_base)))
    // We report ratio in [0,1]; the judge multiplies by 1000000.
    double ratio;
    if (W == OBJ_base) {
        // Baseline already achieves full weight; any valid solution scores 1.0
        ratio = 1.0;
    } else {
        ratio = (double)(OBJ - OBJ_base) / (double)(W - OBJ_base);
        if (ratio < 0.0) ratio = 0.0;
        if (ratio > 1.0) ratio = 1.0;
    }

    quitp(ratio, "Ratio: %.9f | OBJ=%lld OBJ_base=%lld W=%lld R=%d",
          ratio, OBJ, OBJ_base, W, R);

    return 0;
}