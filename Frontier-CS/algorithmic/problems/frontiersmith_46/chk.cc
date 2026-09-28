#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    int T = inf.readInt();

    double total_score = 0.0;
    int num_cases = 0;

    for (int tc = 0; tc < T; tc++) {
        num_cases++;

        // --- read problem instance from inf ---
        int n = inf.readInt();
        int K = inf.readInt();
        int M = inf.readInt();

        vector<long long> a(n + 1);
        for (int i = 1; i <= n; i++) {
            a[i] = (long long)inf.readInt();
        }

        struct BonusCard {
            long long w;
            int p;
            vector<int> positions;
        };

        vector<BonusCard> cards(M);
        for (int j = 0; j < M; j++) {
            cards[j].w = (long long)inf.readInt();
            cards[j].p = inf.readInt();
            int t = inf.readInt();
            cards[j].positions.resize(t);
            for (int k2 = 0; k2 < t; k2++) {
                cards[j].positions[k2] = inf.readInt();
            }
        }

        // --- compute baseline B (q = 0, all x_i = 0) ---
        long long B = 0;
        for (int i = 1; i <= n; i++) B += a[i];
        for (int j = 0; j < M; j++) {
            if (cards[j].p == 0) B += cards[j].w;
        }

        // --- compute upper bound U = sum|a_i| + sum w_j ---
        long long U = 0;
        for (int i = 1; i <= n; i++) U += llabs(a[i]);
        for (int j = 0; j < M; j++) U += cards[j].w;

        // --- read participant output for this test case ---
        // Per the problem statement: malformed/infeasible => V = -10^18 for this case
        // We must NOT abort the whole submission; absorb the error and continue.

        const long long V_INFEASIBLE = (long long)(-1e18);

        // Maximum q we are willing to read even if infeasible, to keep stream aligned.
        // K <= 250, so 600 is a safe upper limit for "still attempt to read q lines".
        const int Q_READ_LIMIT = 600;

        int q = ouf.readInt();

        bool infeasible = false;

        if (q < 0 || q > K) {
            infeasible = true;
        }

        // We can only attempt to read the segment lines if q is in a readable range.
        // If q is wildly out of range we cannot stay in sync with the stream,
        // so we must hard-fail the whole submission.
        if (q < 0 || q > Q_READ_LIMIT) {
            quitf(_wa,
                  "Test case %d: q=%d is out of readable range [0, %d]",
                  tc + 1, q, Q_READ_LIMIT);
        }

        vector<int> diff(n + 2, 0);

        for (int wave = 0; wave < q; wave++) {
            int l = ouf.readInt();
            int r = ouf.readInt();
            if (l < 1 || l > n || r < l || r > n) {
                infeasible = true;
                // Do NOT break; keep reading to stay in stream sync.
            }
            if (!infeasible) {
                diff[l] ^= 1;
                if (r + 1 <= n) diff[r + 1] ^= 1;
            }
        }

        // --- if infeasible, assign V = -10^18 for this case ---
        if (infeasible) {
            // Compute per-test score with V = V_INFEASIBLE
            double case_score;
            if (U == B) {
                // V_INFEASIBLE < U=B, so score = 0
                case_score = 0.0;
            } else {
                double raw = (double)(V_INFEASIBLE - B) / (double)(U - B);
                if (raw < 0.0) raw = 0.0;
                if (raw > 1.0) raw = 1.0;
                case_score = 1000000.0 * raw;
            }
            total_score += case_score;
            continue;
        }

        // --- build parity bits x[1..n] ---
        vector<int> x(n + 1, 0);
        {
            int cur = 0;
            for (int i = 1; i <= n; i++) {
                cur ^= diff[i];
                x[i] = cur;
            }
        }

        // --- compute V ---
        long long V = 0;
        for (int i = 1; i <= n; i++) {
            if (x[i] == 0) V += a[i];
            else            V -= a[i];
        }
        for (int j = 0; j < M; j++) {
            int xorVal = 0;
            for (int pos : cards[j].positions) {
                xorVal ^= x[pos];
            }
            if (xorVal == cards[j].p) V += cards[j].w;
        }

        // --- per-test score ---
        double case_score;
        if (U == B) {
            case_score = (V >= U) ? 1000000.0 : 0.0;
        } else {
            double raw = (double)(V - B) / (double)(U - B);
            if (raw < 0.0) raw = 0.0;
            if (raw > 1.0) raw = 1.0;
            case_score = 1000000.0 * raw;
        }

        total_score += case_score;
    }

    // --- assert no trailing output ---
    if (!ouf.seekEof()) {
        quitf(_wa, "Trailing output detected after all test cases");
    }

    // --- aggregate and report ---
    double avg_score = (num_cases > 0) ? total_score / (double)num_cases : 0.0;
    double ratio = avg_score / 1000000.0;
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    quitp(ratio,
          "Ratio: %.9f (avg per-test score: %.4f / 1000000, cases: %d)",
          ratio, avg_score, num_cases);
}