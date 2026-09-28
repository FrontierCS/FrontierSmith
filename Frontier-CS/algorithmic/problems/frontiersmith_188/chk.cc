#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

static int H, W;

// Apply value `val` to a non-wrapping sub-rectangle [r1..r2] x [c1..c2]
// in a (H+1)x(W+1) 2-D difference array stored flat.
static void apply2D(vector<long long>& diff,
                    int r1, int c1, int r2, int c2, long long val) {
    int stride = W + 1;
    diff[r1 * stride + c1]             += val;
    diff[r1 * stride + (c2 + 1)]       -= val;
    diff[(r2 + 1) * stride + c1]       -= val;
    diff[(r2 + 1) * stride + (c2 + 1)] += val;
}

// Add `val` to every cell in toroidal rectangle (top,bot,left,right)
static void addRect(vector<long long>& diff,
                    int top, int bot, int left, int right, long long val) {
    bool rw = (top > bot), cw = (left > right);
    if (!rw && !cw) {
        apply2D(diff, top, left, bot, right, val);
    } else if (!rw && cw) {
        apply2D(diff, top, left,   bot, W - 1, val);
        apply2D(diff, top, 0,      bot, right,  val);
    } else if (rw && !cw) {
        apply2D(diff, top,   left, H - 1, right, val);
        apply2D(diff, 0,     left, bot,   right,  val);
    } else {
        apply2D(diff, top,   left,   H - 1, W - 1, val);
        apply2D(diff, top,   0,      H - 1, right,  val);
        apply2D(diff, 0,     left,   bot,   W - 1, val);
        apply2D(diff, 0,     0,      bot,   right,  val);
    }
}

// Build f[] from a[] plus the difference array
static vector<long long> buildF(const vector<long long>& a,
                                vector<long long>& diff) {
    // prefix-sum the (H+1)*(W+1) diff array
    int stride = W + 1;
    // row-wise prefix sum
    for (int r = 0; r <= H; r++)
        for (int c = 1; c <= W; c++)
            diff[r * stride + c] += diff[r * stride + c - 1];
    // column-wise prefix sum
    for (int r = 1; r <= H; r++)
        for (int c = 0; c <= W; c++)
            diff[r * stride + c] += diff[(r - 1) * stride + c];

    vector<long long> f(H * W);
    for (int r = 0; r < H; r++)
        for (int c = 0; c < W; c++)
            f[r * W + c] = a[r * W + c] + diff[r * stride + c];
    return f;
}

// Minimum of f over a toroidal rectangle
static long long rectMin(const vector<long long>& f,
                         int top, int bot, int left, int right) {
    long long mn = LLONG_MAX;
    int row = top;
    for (;;) {
        int col = left;
        for (;;) {
            mn = min(mn, f[row * W + col]);
            if (col == right) break;
            col = (col + 1) % W;
        }
        if (row == bot) break;
        row = (row + 1) % H;
    }
    return mn;
}

struct Plan {
    int top, bot, left, right;
    long long v, c;
};
struct Contract {
    int top, bot, left, right;
    long long x, w;
};

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ── Read the input ──────────────────────────────────────────────────────
    H = inf.readInt();
    W = inf.readInt();

    vector<long long> a(H * W);
    for (int i = 0; i < H * W; i++)
        a[i] = inf.readLong();

    int P = inf.readInt();
    int Q = inf.readInt();

    long long B = inf.readLong();
    int K      = inf.readInt();

    vector<Plan> plans(P);
    for (int j = 0; j < P; j++) {
        plans[j].top   = inf.readInt();
        plans[j].bot   = inf.readInt();
        plans[j].left  = inf.readInt();
        plans[j].right = inf.readInt();
        plans[j].v     = inf.readLong();
        plans[j].c     = inf.readLong();
    }

    vector<Contract> contracts(Q);
    for (int k = 0; k < Q; k++) {
        contracts[k].top   = inf.readInt();
        contracts[k].bot   = inf.readInt();
        contracts[k].left  = inf.readInt();
        contracts[k].right = inf.readInt();
        contracts[k].x     = inf.readLong();
        contracts[k].w     = inf.readLong();
    }

    // ── Compute BASE (empty selection) ──────────────────────────────────────
    vector<long long> diffBase((H + 1) * (W + 1), 0LL);
    vector<long long> fBase = buildF(a, diffBase);

    long long BASE = 0;
    for (int k = 0; k < Q; k++) {
        long long mn = rectMin(fBase,
            contracts[k].top, contracts[k].bot,
            contracts[k].left, contracts[k].right);
        if (mn >= contracts[k].x)
            BASE += contracts[k].w;
    }

    // ── Compute MAXPOSSIBLE ─────────────────────────────────────────────────
    long long MAXPOSSIBLE = 0;
    for (int k = 0; k < Q; k++)
        MAXPOSSIBLE += contracts[k].w;

    // ── Read participant output ─────────────────────────────────────────────
    int s = ouf.readInt(0, P,
        "Number of selected plans must be in [0, P]");

    if (s > K)
        quitf(_wa, "s=%d exceeds K=%d", s, K);

    vector<int> selected(s);
    set<int> selectedSet;
    long long totalCost = 0;

    for (int i = 0; i < s; i++) {
        if (P == 0)
            quitf(_wa, "s=%d but P=0, no valid plan index exists", s);
        selected[i] = ouf.readInt(0, P - 1,
            "Plan index out of range [0, P-1]");
        if (!selectedSet.insert(selected[i]).second)
            quitf(_wa, "Duplicate plan index %d", selected[i]);
        totalCost += plans[selected[i]].c;
    }

    // ── Strict EOF check: no extra non-whitespace tokens allowed ────────────
    if (!ouf.seekEof())
        quitf(_wa, "Extra output found after the %d plan indices", s);

    // ── Feasibility checks ──────────────────────────────────────────────────
    if (totalCost > B)
        quitf(_wa, "Total cost %lld exceeds budget B=%lld", totalCost, B);

    // ── Compute f[][] after applying selected plans ─────────────────────────
    vector<long long> diff((H + 1) * (W + 1), 0LL);
    for (int idx : selectedSet)
        addRect(diff,
                plans[idx].top, plans[idx].bot,
                plans[idx].left, plans[idx].right,
                plans[idx].v);

    vector<long long> f = buildF(a, diff);

    // ── Compute VALUE(S) ────────────────────────────────────────────────────
    long long VALUE = 0;
    for (int k = 0; k < Q; k++) {
        long long mn = rectMin(f,
            contracts[k].top, contracts[k].bot,
            contracts[k].left, contracts[k].right);
        if (mn >= contracts[k].x)
            VALUE += contracts[k].w;
    }

    // ── Compute normalized score in [0, 1] ──────────────────────────────────
    // Statement: if MAXPOSSIBLE == BASE then norm = 1
    //            else norm = clamp((VALUE - BASE) / (MAXPOSSIBLE - BASE), 0, 1)
    // The judge then computes score = 1,000,000 * norm.
    // quitp(ratio, ...) passes ratio in [0,1] to the judge which scales it.
    double norm;
    if (MAXPOSSIBLE == BASE) {
        norm = 1.0;
    } else {
        norm = (double)(VALUE - BASE) / (double)(MAXPOSSIBLE - BASE);
        if (norm < 0.0) norm = 0.0;
        if (norm > 1.0) norm = 1.0;
    }

    quitp(norm,
          "VALUE=%lld BASE=%lld MAXPOSSIBLE=%lld Ratio: %.9f",
          VALUE, BASE, MAXPOSSIBLE, norm);
}