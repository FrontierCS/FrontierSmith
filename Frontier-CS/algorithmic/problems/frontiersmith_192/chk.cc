#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    int t = inf.readInt();

    long long total_score = 0;
    int num_cases = 0;

    for (int tc = 0; tc < t; tc++) {
        int n, k, x, m;
        n = inf.readInt();
        k = inf.readInt();
        x = inf.readInt();
        m = inf.readInt();

        vector<long long> a(n + 1);
        for (int i = 1; i <= n; i++) a[i] = inf.readLong();

        vector<int> eu(m), ev(m), ec(m);
        vector<long long> ew(m);
        for (int i = 0; i < m; i++) {
            eu[i] = inf.readInt();
            ev[i] = inf.readInt();
            ec[i] = inf.readInt();
            ew[i] = inf.readLong();
        }

        // Compute baseline B: everyone is '+', c=0 links are same-polarity => satisfied
        long long sum_a = 0;
        for (int i = 1; i <= n; i++) sum_a += a[i];

        long long sum_w = 0;
        for (int i = 0; i < m; i++) sum_w += ew[i];

        long long U_val = sum_a + sum_w;

        long long B = sum_a;
        for (int i = 0; i < m; i++) {
            if (ec[i] == 0) B += ew[i]; // same polarity satisfied by all-'+'
        }

        // Read participant answer
        string s = ouf.readToken();

        bool feasible = true;
        long long V = 0;

        if ((int)s.size() != n) {
            feasible = false;
        } else {
            for (char c : s) {
                if (c != '+' && c != '-' && c != '0') {
                    feasible = false;
                    break;
                }
            }
            if (feasible) {
                int zeros = 0, minuses = 0;
                for (char c : s) {
                    if (c == '0') zeros++;
                    if (c == '-') minuses++;
                }
                if (zeros > k || minuses > x) feasible = false;
            }
        }

        if (feasible) {
            // Base power contributions
            for (int i = 1; i <= n; i++) {
                if (s[i - 1] != '0') V += a[i];
            }
            // Link contributions
            for (int i = 0; i < m; i++) {
                int u = eu[i], v = ev[i];
                if (s[u - 1] == '0' || s[v - 1] == '0') continue;
                bool same = (s[u - 1] == s[v - 1]);
                if (ec[i] == 0 && same)  V += ew[i];
                if (ec[i] == 1 && !same) V += ew[i];
            }
        } else {
            V = 0;
        }

        // Compute per-test score in [0, 1000000] using integer arithmetic
        long long per_score;
        if (U_val == B) {
            // score = floor(1,000,000 * V / U)
            if (U_val == 0) {
                per_score = 1000000LL;
            } else {
                per_score = 1000000LL * V / U_val;
            }
        } else {
            if (V <= B) {
                if (B == 0) {
                    per_score = (V == 0) ? 500000LL : 0LL;
                } else {
                    per_score = 500000LL * V / B;
                }
            } else {
                // 500000 + floor(500000 * (V - B) / (U_val - B))
                per_score = 500000LL + 500000LL * (V - B) / (U_val - B);
            }
        }

        per_score = max(0LL, min(1000000LL, per_score));
        total_score += per_score;
        num_cases++;
    }

    // Ensure no trailing garbage in participant output
    if (!ouf.seekEof()) {
        quitf(_wa, "Trailing content in participant output after all test cases.");
    }

    long long avg_score = (num_cases > 0) ? (total_score / num_cases) : 0LL;
    double ratio = (double)avg_score / 1000000.0;

    quitp(ratio, "Ratio: %.6f (avg per-test score: %lld / 1000000 over %d cases)",
          ratio, avg_score, num_cases);

    return 0;
}