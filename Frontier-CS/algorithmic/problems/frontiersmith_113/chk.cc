#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    int t = inf.readInt();

    double total_score = 0.0;

    for (int tc = 0; tc < t; tc++) {
        int n = inf.readInt();
        int s = inf.readInt();
        int A = inf.readInt();
        int B = inf.readInt();
        int C = inf.readInt();

        // Read initial permutation
        vector<int> init_bay(n + 1); // bay -> label
        vector<int> init_pos(n + 1); // label -> bay
        for (int i = 1; i <= n; i++) {
            init_bay[i] = inf.readInt();
            init_pos[init_bay[i]] = i;
        }

        struct SlotC  { int u, p;    long long w; };
        struct OrderC { int u, v;    long long w; };
        struct DistC  { int u, v, d; long long w; };

        vector<SlotC>  slots(A);
        vector<OrderC> orders(B);
        vector<DistC>  dists(C);

        for (int i = 0; i < A; i++) {
            slots[i].u = inf.readInt();
            slots[i].p = inf.readInt();
            slots[i].w = inf.readLong();
        }
        for (int i = 0; i < B; i++) {
            orders[i].u = inf.readInt();
            orders[i].v = inf.readInt();
            orders[i].w = inf.readLong();
        }
        for (int i = 0; i < C; i++) {
            dists[i].u = inf.readInt();
            dists[i].v = inf.readInt();
            dists[i].d = inf.readInt();
            dists[i].w = inf.readLong();
        }

        // Compute U = sum of all contract weights
        long long U = 0;
        for (auto& c : slots)  U += c.w;
        for (auto& c : orders) U += c.w;
        for (auto& c : dists)  U += c.w;

        // Evaluate bonus given label->bay map
        auto evalScore = [&](const vector<int>& lpos) -> long long {
            long long sc = 0;
            for (auto& c : slots)
                if (lpos[c.u] == c.p) sc += c.w;
            for (auto& c : orders)
                if (lpos[c.u] < lpos[c.v]) sc += c.w;
            for (auto& c : dists)
                if (abs(lpos[c.u] - lpos[c.v]) <= c.d) sc += c.w;
            return sc;
        };

        // Compute baseline B0: no-op strategy (day i: swap label i with itself)
        // No-op means no changes, so we evaluate the initial permutation
        long long B0 = evalScore(init_pos);

        // Now read contestant output: 2n integers for this test case
        // We must read all 2n integers regardless of validity
        bool valid = true;
        string err_msg;

        // cur_pos[label] = bay, cur_bay[bay] = label
        vector<int> cur_pos(n + 1);
        vector<int> cur_bay(n + 1);
        for (int i = 1; i <= n; i++) {
            cur_pos[i] = init_pos[i];
            cur_bay[i] = init_bay[i];
        }

        for (int day = 1; day <= n; day++) {
            int xi, yi;
            // Always read, even if already invalid
            if (ouf.eof()) {
                if (valid) {
                    valid = false;
                    err_msg = "Unexpected EOF at day " + to_string(day);
                }
                // Can't read more; break
                break;
            }
            xi = ouf.readInt();
            if (ouf.eof()) {
                if (valid) {
                    valid = false;
                    err_msg = "Unexpected EOF reading y on day " + to_string(day);
                }
                break;
            }
            yi = ouf.readInt();

            if (!valid) continue; // already invalid; read consumed, skip simulation

            int lo = day;
            int hi = min(day + s, n);

            if (xi < lo || xi > hi) {
                valid = false;
                err_msg = "Day " + to_string(day) + ": x=" + to_string(xi)
                          + " out of [" + to_string(lo) + "," + to_string(hi) + "]";
                continue;
            }
            if (yi < lo || yi > hi) {
                valid = false;
                err_msg = "Day " + to_string(day) + ": y=" + to_string(yi)
                          + " out of [" + to_string(lo) + "," + to_string(hi) + "]";
                continue;
            }
            if (xi > yi) {
                valid = false;
                err_msg = "Day " + to_string(day) + ": x=" + to_string(xi)
                          + " > y=" + to_string(yi);
                continue;
            }

            // Simulate swap of labels xi and yi
            if (xi != yi) {
                int bx = cur_pos[xi];
                int by = cur_pos[yi];
                cur_bay[bx] = yi;
                cur_bay[by] = xi;
                cur_pos[xi] = by;
                cur_pos[yi] = bx;
            }
        }

        // Check no trailing garbage in the output stream after all test cases
        // (done after the loop below)

        // Compute F:
        // Per the statement: "malformed or any move is invalid => objective value F = 0"
        // Then apply the normal scoring formula with that F.
        long long F;
        if (!valid) {
            F = 0;
        } else {
            F = evalScore(cur_pos);
        }

        // Scoring formula per statement:
        // if U == B0: S = 1
        // else: S = clamp((F - B0) / (U - B0), 0, 1)
        double S;
        if (U == B0) {
            S = 1.0;
        } else {
            double ratio = (double)(F - B0) / (double)(U - B0);
            S = max(0.0, min(1.0, ratio));
        }

        total_score += S;
    }

    // Check no trailing garbage
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra output after all test cases");
    }

    double avg = (t > 0) ? (total_score / (double)t) : 1.0;
    avg = max(0.0, min(1.0, avg));

    quitp(avg, "Ratio: %.9f (sum_S=%.6f, t=%d)", avg, total_score, t);

    return 0;
}