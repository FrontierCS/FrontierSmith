#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

static bool isLuckyVal(long long v) {
    if (v <= 0) return false;
    while (v > 0) {
        int d = (int)(v % 10);
        if (d != 4 && d != 7) return false;
        v /= 10;
    }
    return true;
}

static long long countLuckyRange(const vector<long long>& arr, int l, int r) {
    long long cnt = 0;
    for (int i = l; i <= r; i++) cnt += (isLuckyVal(arr[i]) ? 1 : 0);
    return cnt;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- Read input ----
    int n  = inf.readInt();
    int M  = inf.readInt();
    int T  = inf.readInt();
    long long B = inf.readLong();

    vector<long long> a0(n + 1);
    for (int i = 1; i <= n; i++) a0[i] = inf.readLong();

    vector<int> Lq(T + 1), Rq(T + 1);
    vector<long long> Wq(T + 1);
    for (int t = 1; t <= T; t++) {
        Lq[t] = inf.readInt();
        Rq[t] = inf.readInt();
        Wq[t] = inf.readLong();
    }

    vector<int> cl(M + 1), cr(M + 1), cS(M + 1), cE(M + 1);
    vector<long long> cD(M + 1), cC(M + 1);
    for (int i = 1; i <= M; i++) {
        cl[i] = inf.readInt();
        cr[i] = inf.readInt();
        cD[i] = inf.readLong();
        cC[i] = inf.readLong();
        cS[i] = inf.readInt();
        cE[i] = inf.readInt();
    }

    // ---- Read participant output (exactly T integers) ----
    vector<int> x(T + 1, 0);
    for (int t = 1; t <= T; t++) {
        x[t] = ouf.readInt(-10000000, 10000000,
                           ("x_" + to_string(t)).c_str());
    }
    // Enforce no extra tokens after the T integers
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra output after the %d required integers", T);
    }

    // ---- Feasibility checks ----
    for (int t = 1; t <= T; t++) {
        if (x[t] < 0 || x[t] > M)
            quitf(_wa, "Round %d: card index %d out of range [0,%d]", t, x[t], M);
    }

    // Each nonzero card used at most once
    vector<bool> usedCard(M + 1, false);
    for (int t = 1; t <= T; t++) {
        if (x[t] == 0) continue;
        int i = x[t];
        if (usedCard[i])
            quitf(_wa, "Round %d: card %d used more than once", t, i);
        usedCard[i] = true;
    }

    // Card used within its valid time window
    for (int t = 1; t <= T; t++) {
        if (x[t] == 0) continue;
        int i = x[t];
        if (t < cS[i] || t > cE[i])
            quitf(_wa, "Round %d: card %d used outside its window [%d,%d]",
                  t, i, cS[i], cE[i]);
    }

    // Budget constraint
    long long totalCost = 0;
    for (int t = 1; t <= T; t++) {
        if (x[t] == 0) continue;
        totalCost += cC[x[t]];
    }
    if (totalCost > B)
        quitf(_wa, "Total cost %lld exceeds budget %lld", totalCost, B);

    // ---- Compute participant objective ----
    vector<long long> arr(n + 1);
    for (int i = 1; i <= n; i++) arr[i] = a0[i];

    long long obj_you = 0;
    for (int t = 1; t <= T; t++) {
        if (x[t] > 0) {
            int i = x[t];
            for (int j = cl[i]; j <= cr[i]; j++) arr[j] += cD[i];
        }
        obj_you += Wq[t] * countLuckyRange(arr, Lq[t], Rq[t]);
    }

    // ---- Compute baseline objective ----
    vector<long long> arrB(n + 1);
    for (int i = 1; i <= n; i++) arrB[i] = a0[i];

    vector<bool> cardUsed(M + 1, false);
    long long remBudget = B;
    long long obj_base = 0;

    for (int t = 1; t <= T; t++) {
        long long curCnt = countLuckyRange(arrB, Lq[t], Rq[t]);

        int    bestCard  = -1;
        long long bestGain  = 0;
        double bestRatio = -1e18;

        for (int i = 1; i <= M; i++) {
            if (cardUsed[i]) continue;
            if (t < cS[i] || t > cE[i]) continue;
            if (cC[i] > remBudget) continue;

            // Compute gain
            long long newCnt = curCnt;
            int ol = max(cl[i], Lq[t]);
            int orr = min(cr[i], Rq[t]);
            if (ol <= orr) {
                for (int j = ol; j <= orr; j++) {
                    bool wasLucky = isLuckyVal(arrB[j]);
                    bool nowLucky = isLuckyVal(arrB[j] + cD[i]);
                    if (!wasLucky && nowLucky) newCnt++;
                    else if (wasLucky && !nowLucky) newCnt--;
                }
            }

            long long gain = Wq[t] * (newCnt - curCnt);

            if (gain > 0) {
                double ratio = (double)gain / (double)cC[i];
                bool better = false;
                if (ratio > bestRatio + 1e-12) {
                    better = true;
                } else if (ratio >= bestRatio - 1e-12) {
                    if (gain > bestGain) better = true;
                    else if (gain == bestGain && (bestCard == -1 || i < bestCard)) better = true;
                }
                if (better) {
                    bestCard  = i;
                    bestGain  = gain;
                    bestRatio = ratio;
                }
            }
        }

        if (bestCard != -1) {
            for (int j = cl[bestCard]; j <= cr[bestCard]; j++) arrB[j] += cD[bestCard];
            cardUsed[bestCard] = true;
            remBudget -= cC[bestCard];
        }

        obj_base += Wq[t] * countLuckyRange(arrB, Lq[t], Rq[t]);
    }

    // ---- Compute score ----
    // Score_file = 10^6 * clamp((OBJ_you+1)/(OBJ_base+1), 0, 2)
    // For quitp we need score_ratio in [0, 1].
    // We map: score_ratio = clamp((OBJ_you+1)/(OBJ_base+1), 0, 2) / 2
    // so that equal performance (raw=1.0) -> score_ratio=0.5,
    // and 2x performance (raw=2.0) -> score_ratio=1.0.
    // This matches the statement: full relative score = 10^6 (raw=1 -> 0.5 in [0,1])
    // and max = 2*10^6 (raw=2 -> 1.0 in [0,1]).
    double raw = (double)(obj_you + 1) / (double)(obj_base + 1);
    if (raw < 0.0) raw = 0.0;
    if (raw > 2.0) raw = 2.0;
    // score_ratio in [0,1]: divide clamp result by 2
    double score_ratio = raw / 2.0;

    quitp(score_ratio,
          "OBJ_you=%lld OBJ_base=%lld raw=%.6f Ratio: %.9f",
          obj_you, obj_base, raw, score_ratio);

    return 0;
}