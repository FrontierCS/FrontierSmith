#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ── Read input ──────────────────────────────────────────────────────────
    int n, Q;
    n = inf.readInt();
    Q = inf.readInt();

    struct Card { long long v, d, h; };

    vector<int> cap(n);
    vector<deque<Card>> stacks(n);
    long long U = 0;

    for (int i = 0; i < n; i++) {
        cap[i] = inf.readInt();
        int s = inf.readInt();
        for (int j = 0; j < s; j++) {
            Card c;
            c.v = inf.readInt();
            c.d = inf.readInt();
            c.h = inf.readInt();
            stacks[i].push_back(c);
            U += c.v;
        }
    }

    // ── Compute baseline B ──────────────────────────────────────────────────
    // Greedy: each turn pick the stack top with maximum immediate gain (>0),
    // tie-break by smaller stack index.
    vector<deque<Card>> bst = stacks;
    long long B = 0;
    for (int t = 1; t <= Q; t++) {
        long long bestG = -1;
        int bestI = -1;
        for (int i = 0; i < n; i++) {
            if (bst[i].empty()) continue;
            Card& c = bst[i].front();
            long long g = max(0LL, c.v - c.d * (long long)(t - 1));
            if (g > bestG || (g == bestG && (bestI == -1 || i < bestI))) {
                bestG = g;
                bestI = i;
            }
        }
        // "If all g_i = 0, stop."
        if (bestI == -1 || bestG == 0) break;
        B += bestG;
        bst[bestI].pop_front();
    }

    // ── Read and validate participant output ────────────────────────────────
    if (ouf.seekEof())
        quitf(_wa, "output is empty; expected number of operations m");

    int m = ouf.readInt();
    if (m < 0 || m > Q)
        quitf(_wa, "m=%d is not in [0, %d]", m, Q);

    vector<deque<Card>> pst = stacks;
    long long P = 0;

    for (int t = 1; t <= m; t++) {
        if (ouf.seekEof())
            quitf(_wa, "Turn %d: expected operation but output ended (declared m=%d)", t, m);

        string op = ouf.readToken();

        if (op == "C") {
            int ci = ouf.readInt();
            if (ci < 1 || ci > n)
                quitf(_wa, "Turn %d: C %d — stack index out of range [1,%d]", t, ci, n);
            int c = ci - 1;
            if (pst[c].empty())
                quitf(_wa, "Turn %d: C %d — stack is empty", t, ci);
            Card card = pst[c].front();
            pst[c].pop_front();
            long long gain = max(0LL, card.v - card.d * (long long)(t - 1));
            P += gain;

        } else if (op == "M") {
            int ai = ouf.readInt();
            int bi = ouf.readInt();
            if (ai < 1 || ai > n)
                quitf(_wa, "Turn %d: M source %d out of range [1,%d]", t, ai, n);
            if (bi < 1 || bi > n)
                quitf(_wa, "Turn %d: M destination %d out of range [1,%d]", t, bi, n);
            if (ai == bi)
                quitf(_wa, "Turn %d: M %d %d — source == destination", t, ai, bi);
            int a = ai - 1, b = bi - 1;
            if (pst[a].empty())
                quitf(_wa, "Turn %d: M %d %d — source stack is empty", t, ai, bi);
            if ((int)pst[b].size() >= cap[b])
                quitf(_wa, "Turn %d: M %d %d — destination stack is full (cap=%d)", t, ai, bi, cap[b]);
            Card card = pst[a].front();
            pst[a].pop_front();
            pst[b].push_front(card);
            P -= card.h;

        } else {
            quitf(_wa, "Turn %d: unknown operation token '%s'", t, op.c_str());
        }
    }

    // ── EOF check: reject any extra non-whitespace tokens ───────────────────
    if (!ouf.seekEof())
        quitf(_wa, "extra output after %d operations", m);

    // ── Scoring ─────────────────────────────────────────────────────────────
    // if P <= B : score = 50 * max(0,P) / max(1,B)
    // if P >  B : score = 50 + 50*(P-B) / max(1, U-B)
    // clamp to [0,100], then ratio = score/100
    double score;
    if (P <= B) {
        long long num = max(0LL, P);
        long long den = max(1LL, B);
        score = 50.0 * (double)num / (double)den;
    } else {
        long long num = P - B;
        long long den = max(1LL, U - B);
        score = 50.0 + 50.0 * (double)num / (double)den;
    }
    if (score < 0.0) score = 0.0;
    if (score > 100.0) score = 100.0;

    double ratio = score / 100.0;
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    quitp(ratio,
          "P=%lld B=%lld U=%lld score=%.4f Ratio: %.6f",
          P, B, U, score, ratio);

    return 0;
}