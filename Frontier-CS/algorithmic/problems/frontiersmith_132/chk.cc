#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ── read problem input ──────────────────────────────────────────────────
    int n = inf.readInt();
    int S = inf.readInt();
    int T = inf.readInt();

    vector<int> cap(S + 1);
    for (int i = 1; i <= S; i++) cap[i] = inf.readInt();

    struct Car { int serial, dest; long long value; };
    vector<Car> cars(n);
    for (int i = 0; i < n; i++) {
        cars[i].serial = inf.readInt();
        cars[i].dest   = inf.readInt();
        cars[i].value  = (long long)inf.readInt();
    }

    // ── compute U (total value) ─────────────────────────────────────────────
    long long U = 0;
    for (auto& c : cars) U += c.value;

    // ── baseline (no sidings) ───────────────────────────────────────────────
    // Scan front-to-back; ship directly if dest==cur and serial>last.
    // Each shipped car contributes (value - 1): value earned, 1 D-move counted.
    long long B = 0;
    {
        int cur = 1, last = 0;
        for (auto& c : cars) {
            if (c.dest < cur) continue;
            if (c.dest > cur) { cur = c.dest; last = 0; }
            // dest == cur
            if (c.serial > last) {
                B += c.value - 1LL;
                last = c.serial;
            }
        }
    }

    // ── read participant output ─────────────────────────────────────────────
    int q = ouf.readInt();
    if (q < 0 || q > 2000000)
        quitf(_wa, "q = %d is out of allowed range [0, 2000000]", q);

    // ── simulation state ────────────────────────────────────────────────────
    // inbound track: front = index 0
    deque<int> inbound;
    for (int i = 0; i < n; i++) inbound.push_back(i);

    // sidings: index 1..S, each is a stack (back = top)
    vector<vector<int>> sidings(S + 1);

    int current = 1;
    vector<int> lastShipped(T + 2, 0);  // last serial shipped to train t

    long long V = 0;   // total shipped value
    long long M = 0;   // number of H and D operations

    // helpers
    auto srcEmpty = [&](int a) -> bool {
        if (a == 0) return inbound.empty();
        return sidings[a].empty();
    };
    auto srcTop = [&](int a) -> int {
        if (a == 0) return inbound.front();
        return sidings[a].back();
    };
    auto popSrc = [&](int a) {
        if (a == 0) inbound.pop_front();
        else sidings[a].pop_back();
    };

    // ── execute operations ──────────────────────────────────────────────────
    for (int op = 0; op < q; op++) {
        string type = ouf.readToken();

        if (type == "H") {
            int a = ouf.readInt();
            int b = ouf.readInt();
            if (a < 0 || a > S)
                quitf(_wa, "op %d H: source %d out of range [0,%d]", op+1, a, S);
            if (b < 1 || b > S)
                quitf(_wa, "op %d H: siding %d out of range [1,%d]", op+1, b, S);
            if (a == b)
                quitf(_wa, "op %d H: source and destination are both siding %d", op+1, a);
            if (srcEmpty(a))
                quitf(_wa, "op %d H: source %d is empty", op+1, a);
            if ((int)sidings[b].size() >= cap[b])
                quitf(_wa, "op %d H: siding %d is full (cap=%d)", op+1, b, cap[b]);
            int idx = srcTop(a);
            sidings[b].push_back(idx);
            popSrc(a);
            M++;

        } else if (type == "D") {
            int a = ouf.readInt();
            if (a < 0 || a > S)
                quitf(_wa, "op %d D: source %d out of range [0,%d]", op+1, a, S);
            if (srcEmpty(a))
                quitf(_wa, "op %d D: source %d is empty", op+1, a);
            if (current > T)
                quitf(_wa, "op %d D: no current destination train (current=%d > T=%d)",
                      op+1, current, T);
            int idx = srcTop(a);
            if (cars[idx].dest != current)
                quitf(_wa, "op %d D: car dest=%d but current train=%d",
                      op+1, cars[idx].dest, current);
            if (cars[idx].serial <= lastShipped[current])
                quitf(_wa, "op %d D: car serial=%d not > last shipped serial=%d on train %d",
                      op+1, cars[idx].serial, lastShipped[current], current);
            V += cars[idx].value;
            lastShipped[current] = cars[idx].serial;
            popSrc(a);
            M++;

        } else if (type == "X") {
            int a = ouf.readInt();
            if (a < 0 || a > S)
                quitf(_wa, "op %d X: source %d out of range [0,%d]", op+1, a, S);
            if (srcEmpty(a))
                quitf(_wa, "op %d X: source %d is empty", op+1, a);
            popSrc(a);

        } else if (type == "N") {
            if (current > T)
                quitf(_wa, "op %d N: no current destination train to close (current=%d > T=%d)",
                      op+1, current, T);
            current++;

        } else {
            quitf(_wa, "op %d: unknown operation type '%s'", op+1, type.c_str());
        }
    }

    // ── strict end-of-output check ──────────────────────────────────────────
    // Reject any trailing tokens / garbage after the q-th operation.
    if (!ouf.seekEof())
        quitf(_wa, "Extra output found after the %d operations", q);

    // ── post-execution feasibility checks ──────────────────────────────────
    if (!inbound.empty())
        quitf(_wa, "Inbound track is not empty after all operations (%d cars remain)",
              (int)inbound.size());
    for (int i = 1; i <= S; i++)
        if (!sidings[i].empty())
            quitf(_wa, "Siding %d is not empty after all operations (%d cars remain)",
                  i, (int)sidings[i].size());

    // ── objective and scoring ───────────────────────────────────────────────
    // OBJ = V - M
    // score = 1,000,000 * clamp((OBJ - B) / (U - B), 0, 1)
    // quitp expects a ratio in [0, 1]; the 1,000,000 scaling is applied
    // externally by the judge.
    long long OBJ = V - M;

    double ratio;
    if (U == B) {
        // Every feasible submission gets full score
        ratio = 1.0;
    } else {
        ratio = (double)(OBJ - B) / (double)(U - B);
        if (ratio < 0.0) ratio = 0.0;
        if (ratio > 1.0) ratio = 1.0;
    }

    quitp(ratio,
          "OBJ=%lld (V=%lld M=%lld) B=%lld U=%lld | Ratio: %.9f",
          OBJ, V, M, B, U, ratio);

    return 0;
}