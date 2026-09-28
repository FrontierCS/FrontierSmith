#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- Read input ----
    int N = inf.readInt();
    int S = inf.readInt();

    vector<long long> C(S);
    for (int j = 0; j < S; j++) C[j] = inf.readLong();

    long long L[3], R[3], M[3][3];
    for (int x = 0; x < 3; x++) L[x] = inf.readLong();
    for (int y = 0; y < 3; y++) R[y] = inf.readLong();
    for (int x = 0; x < 3; x++)
        for (int y = 0; y < 3; y++)
            M[x][y] = inf.readLong();

    vector<int> a(N+1), b(N+1);
    vector<long long> len(N+1), val(N+1);
    for (int i = 1; i <= N; i++) {
        a[i] = inf.readInt();
        b[i] = inf.readInt();
        len[i] = inf.readLong();
        val[i] = inf.readLong();
    }

    // ---- Read participant output ----
    // We do NOT call ouf.readEof() at the end; trailing whitespace/newlines
    // are perfectly normal and must not be penalised.
    vector<vector<pair<int,int>>> shelves(S);
    for (int j = 0; j < S; j++) {
        int k = ouf.readInt(0, N, "k must be in [0,N]");
        shelves[j].resize(k);
        for (int t = 0; t < k; t++) {
            int id = ouf.readInt(1, N, "plaque id must be in [1,N]");
            int f  = ouf.readInt(0, 1,  "flip must be 0 or 1");
            shelves[j][t] = {id, f};
        }
    }
    // NOTE: intentionally omitting ouf.readEof() so trailing newlines are fine.

    // ---- Feasibility checks ----
    vector<bool> used(N+1, false);
    for (int j = 0; j < S; j++) {
        long long total_len = 0;
        for (auto& [id, f] : shelves[j]) {
            if (used[id])
                quitf(_wa, "Plaque %d used more than once", id);
            used[id] = true;
            total_len += len[id];
        }
        if (total_len > C[j])
            quitf(_wa, "Shelf %d exceeds capacity: total length %lld > %lld",
                  j+1, total_len, C[j]);
    }

    // ---- Compute participant objective V ----
    long long V = 0;
    for (int j = 0; j < S; j++) {
        int k = (int)shelves[j].size();
        if (k == 0) continue;

        // Build actual left/right arrays for this shelf
        vector<int> lv(k), rv(k);
        for (int t = 0; t < k; t++) {
            int id = shelves[j][t].first;
            int f  = shelves[j][t].second;
            lv[t] = (f == 0) ? a[id] : b[id];
            rv[t] = (f == 0) ? b[id] : a[id];
        }

        long long contrib = 0;
        for (int t = 0; t < k; t++) contrib += val[shelves[j][t].first];
        contrib += L[lv[0]];
        contrib += R[rv[k-1]];
        for (int t = 0; t + 1 < k; t++) contrib += M[rv[t]][lv[t+1]];
        V += contrib;
    }

    // ---- Compute baseline objective B ----
    // Sort plaques by decreasing val/len ratio, then decreasing val, then increasing index
    vector<int> order(N);
    iota(order.begin(), order.end(), 1);   // 1-based ids
    sort(order.begin(), order.end(), [&](int i, int j) {
        // Compare val[i]/len[i] vs val[j]/len[j]  ->  val[i]*len[j] vs val[j]*len[i]
        long long lhs = val[i] * len[j];
        long long rhs = val[j] * len[i];
        if (lhs != rhs) return lhs > rhs;
        if (val[i] != val[j]) return val[i] > val[j];
        return i < j;
    });

    vector<long long> rem(S);
    for (int j = 0; j < S; j++) rem[j] = C[j];

    // bfl[j] = first-left on shelf j (-1 if empty), brl[j] = last-right
    vector<int> bfl(S, -1), brl(S, -1);
    vector<long long> bsv(S, 0), bsm(S, 0);

    for (int id : order) {
        int target = -1;
        for (int j = 0; j < S; j++) {
            if (rem[j] >= len[id]) { target = j; break; }
        }
        if (target == -1) continue;

        int fl;
        if (bfl[target] == -1) {
            // Shelf empty: maximise L[left] + R[right]
            long long s0 = L[a[id]] + R[b[id]];
            long long s1 = L[b[id]] + R[a[id]];
            fl = (s1 > s0) ? 1 : 0;
        } else {
            int x = brl[target];
            long long s0 = M[x][a[id]] + R[b[id]];
            long long s1 = M[x][b[id]] + R[a[id]];
            fl = (s1 > s0) ? 1 : 0;
        }

        int li = (fl == 0) ? a[id] : b[id];
        int ri = (fl == 0) ? b[id] : a[id];

        rem[target] -= len[id];
        bsv[target] += val[id];
        if (bfl[target] == -1) {
            bfl[target] = li;
        } else {
            bsm[target] += M[brl[target]][li];
        }
        brl[target] = ri;
    }

    long long B = 0;
    for (int j = 0; j < S; j++) {
        if (bfl[j] == -1) continue;
        B += bsv[j] + L[bfl[j]] + R[brl[j]] + bsm[j];
    }

    // ---- Compute score ratio ----
    // score = min(200, 100 * V / B)
    // judge ratio in [0,1] = score / 200 = min(1, V / (2*B))
    // So baseline (V==B) -> ratio 0.5 -> 100 pts; 2x baseline -> ratio 1.0 -> 200 pts.
    if (B <= 0) {
        // Shouldn't happen on official data; treat gracefully
        double ratio = (V > 0) ? 1.0 : 0.5;
        quitp(ratio, "Ratio: %.9f | V=%lld B=%lld (B<=0, fallback)", ratio, V, B);
    }

    double raw_score = 100.0 * (double)V / (double)B;   // 0..200+
    double capped    = (raw_score < 200.0) ? raw_score : 200.0;
    double ratio     = capped / 200.0;                   // 0..1
    if (ratio < 0.0) ratio = 0.0;

    quitp(ratio, "Ratio: %.9f | V=%lld B=%lld score=%.4f/200",
          ratio, V, B, capped);
}