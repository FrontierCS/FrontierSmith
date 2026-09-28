#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ── Read test input ──────────────────────────────────────────────────────
    int m = inf.readInt();
    int V = inf.readInt();
    int K = inf.readInt();
    int B = inf.readInt();

    vector<long long> w(m);
    vector<vector<int>> logs(m);

    for (int j = 0; j < m; j++) {
        w[j]   = (long long)inf.readInt();
        int nj = inf.readInt();
        logs[j].resize(nj);
        for (int i = 0; i < nj; i++) logs[j][i] = inf.readInt();
    }

    // ── Compute OBJ_base (greedy singleton library) ──────────────────────────
    // Build val_logs[v] = list of log indices that contain value v
    vector<vector<int>> val_logs(V + 1);
    for (int j = 0; j < m; j++) {
        vector<bool> seen(V + 1, false);
        for (int x : logs[j]) {
            if (x >= 1 && x <= V && !seen[x]) {
                seen[x] = true;
                val_logs[x].push_back(j);
            }
        }
    }

    vector<bool> covered(m, false);
    long long obj_base = 0;
    int baseline_iters = min(K, B);

    for (int iter = 0; iter < baseline_iters; iter++) {
        long long best_wt = 0;
        int best_v = -1;
        for (int v = 1; v <= V; v++) {
            long long wt = 0;
            for (int j : val_logs[v])
                if (!covered[j]) wt += w[j];
            if (wt > best_wt) {
                best_wt = wt;
                best_v  = v;
            }
        }
        if (best_wt == 0) break;
        for (int j : val_logs[best_v]) {
            if (!covered[j]) {
                covered[j] = true;
                obj_base  += w[j]; // singleton gives L_j = 1
            }
        }
    }

    // Problem guarantees OBJ_base > 0
    if (obj_base <= 0) {
        quitf(_fail, "Internal error: OBJ_base=%lld <= 0", obj_base);
    }

    // ── Read participant output ──────────────────────────────────────────────
    // Handle completely empty output
    if (ouf.seekEof()) {
        // 0 signatures → OBJ_sub = 0
        double score = 0.0;
        double ratio = 0.0;
        quitp(ratio,
              "OBJ_sub=0 OBJ_base=%lld Score=%.4f Ratio: %.9f",
              obj_base, score, ratio);
    }

    int q = ouf.readInt();
    if (q < 0 || q > K)
        quitf(_wa, "q=%d is out of range [0, K=%d]", q, K);

    vector<vector<int>> sigs(q);
    long long total_len = 0;

    for (int i = 0; i < q; i++) {
        int t = ouf.readInt();
        if (t < 1)
            quitf(_wa, "Signature %d has length %d < 1", i + 1, t);
        sigs[i].resize(t);
        for (int k = 0; k < t; k++) {
            int x = ouf.readInt();
            if (x < 1 || x > V)
                quitf(_wa, "Signature %d value %d out of range [1, V=%d]", i + 1, x, V);
            if (k > 0 && x <= sigs[i][k - 1])
                quitf(_wa,
                      "Signature %d is not strictly increasing at position %d: %d <= %d",
                      i + 1, k + 1, x, sigs[i][k - 1]);
            sigs[i][k] = x;
        }
        total_len += t;
        if (total_len > (long long)B)
            quitf(_wa,
                  "Total signature length %lld exceeds B=%d after reading signature %d",
                  total_len, B, i + 1);
    }

    // ── EOF check: reject any trailing tokens ────────────────────────────────
    if (!ouf.seekEof())
        quitf(_wa, "Extra output after the %d signature(s)", q);

    // ── Compute OBJ_sub ──────────────────────────────────────────────────────
    long long obj_sub = 0;
    for (int j = 0; j < m; j++) {
        long long best_len = 0;
        for (int i = 0; i < q; i++) {
            const vector<int>& sig = sigs[i];
            int si = 0, t = (int)sig.size();
            for (int v : logs[j]) {
                if (si < t && v == sig[si]) ++si;
            }
            if (si == t && (long long)t > best_len)
                best_len = t;
        }
        obj_sub += w[j] * best_len;
    }

    // ── Score ─────────────────────────────────────────────────────────────────
    // Score_test = min(2000, 1000 * OBJ_sub / OBJ_base)
    // ratio for quitp  = Score_test / 2000, clamped to [0, 1]
    double raw   = (double)obj_sub / (double)obj_base;
    double score = min(2000.0, 1000.0 * raw);
    double ratio = score / 2000.0;
    ratio        = max(0.0, min(1.0, ratio));

    quitp(ratio,
          "OBJ_sub=%lld OBJ_base=%lld Score=%.4f Ratio: %.9f",
          obj_sub, obj_base, score, ratio);

    return 0;
}