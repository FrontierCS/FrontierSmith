#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---------- read problem input ----------
    int n = inf.readInt();
    int q = inf.readInt();

    struct Observer {
        int k;
        long long w;
        vector<int> s;
        string t;
    };

    vector<Observer> obs(q);
    for (int j = 0; j < q; j++) {
        obs[j].k = inf.readInt();
        obs[j].w = (long long)inf.readInt();
        obs[j].s.resize(obs[j].k);
        for (int i = 0; i < obs[j].k; i++) {
            obs[j].s[i] = inf.readInt();
        }
        obs[j].t = inf.readToken();
    }

    // ---------- read participant permutation ----------
    vector<int> perm(n);
    for (int i = 0; i < n; i++) {
        perm[i] = ouf.readInt(1, n, "permutation element out of range [1,n]");
    }
    // enforce end-of-stream
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra tokens after the permutation");
    }

    // check all values distinct
    vector<bool> seen(n + 1, false);
    for (int i = 0; i < n; i++) {
        if (seen[perm[i]]) {
            quitf(_wa, "Duplicate value %d in permutation", perm[i]);
        }
        seen[perm[i]] = true;
    }

    // ---------- helper: compute objective for a given pos[] array ----------
    // pos[v] = 0-based index of value v in the permutation
    auto computeObj = [&](const vector<int>& pos_arr) -> long long {
        long long obj = 0;
        for (int j = 0; j < q; j++) {
            // sort the watched set by position
            vector<int> sv = obs[j].s;
            sort(sv.begin(), sv.end(), [&](int a, int b) {
                return pos_arr[a] < pos_arr[b];
            });

            int mn = sv[0], mx = sv[0];
            for (int i = 1; i < obs[j].k; i++) {
                char produced;
                if (sv[i] < mn) {
                    produced = '<';
                    mn = sv[i];
                } else if (sv[i] > mx) {
                    produced = '>';
                    mx = sv[i];
                } else {
                    produced = '?';
                }
                if (produced == obs[j].t[i - 1]) {
                    obj += obs[j].w;
                }
            }
        }
        return obj;
    };

    // ---------- participant score X ----------
    vector<int> pos(n + 1);
    for (int i = 0; i < n; i++) pos[perm[i]] = i;
    long long X = computeObj(pos);

    // ---------- baseline B = Obj(identity permutation 1,2,...,n) ----------
    vector<int> id_pos(n + 1);
    for (int i = 1; i <= n; i++) id_pos[i] = i - 1;
    long long B = computeObj(id_pos);

    // ---------- trivial upper bound U ----------
    long long U = 0;
    for (int j = 0; j < q; j++) {
        U += obs[j].w * (long long)(obs[j].k - 1);
    }

    // ---------- normalised score ----------
    double norm_score;
    if (U == B) {
        // baseline already achieves the upper bound
        norm_score = 1.0;
    } else {
        norm_score = (double)(X - B) / (double)(U - B);
        if (norm_score < 0.0) norm_score = 0.0;
        if (norm_score > 1.0) norm_score = 1.0;
    }

    quitp(norm_score,
          "X=%lld B=%lld U=%lld Ratio: %.9f",
          X, B, U, norm_score);

    return 0;
}