#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

typedef long long ll;
typedef unsigned long long ull;
typedef __int128 lll;

static lll myabs(lll x) { return x >= 0 ? x : -x; }

static double lll_to_double(lll x) {
    if (x == 0) return 0.0;
    bool neg = (x < 0);
    if (neg) x = -x;
    // convert via repeated division
    char buf[60];
    int idx = 0;
    lll tmp = x;
    while (tmp > 0) {
        buf[idx++] = (int)(tmp % 10) + '0';
        tmp /= 10;
    }
    double r = 0.0, base = 1.0;
    for (int i = 0; i < idx; i++) {
        r += (double)(buf[i] - '0') * base;
        base *= 10.0;
    }
    return neg ? -r : r;
}

static ull parseChunk(const string &s, int l, int r) {
    // l, r are 0-based inclusive; chunk length <= 18
    ull val = 0;
    for (int i = l; i <= r; i++) {
        val = val * 10 + (ull)(s[i] - '0');
    }
    return val;
}

// Compute baseline objective per problem statement:
// Partition into chunks of length exactly D (last may be shorter).
// Greedily assign each chunk to minimize weighted deviation increase, tie-break smallest j.
static lll computeBaseline(int n, int m, int D,
                            const string &s,
                            const vector<ll> &T,
                            const vector<ll> &W,
                            const vector<ll> &C) {
    vector<lll> S(m, 0);
    lll obj = 0;
    bool first = true;
    int pos = 0;

    while (pos < n) {
        int end = min(pos + D - 1, n - 1); // 0-based inclusive end
        ull v = parseChunk(s, pos, end);

        // Cut cost: cut before position pos (0-based), i.e. between pos-1 and pos (0-based)
        // In 1-based terms: cut after position pos (since pos is 0-based start),
        // which is C[pos-1] in 0-based C array.
        if (!first) {
            // cut is after 1-based position `pos` (which equals 0-based index pos)
            obj += (lll)C[pos - 1];
        }
        first = false;

        // Find best blackboard
        int best_j = 0;
        lll best_delta = (lll)9e36;
        // initialize properly
        {
            lll wj = (lll)W[0];
            lll sj = S[0];
            lll tj = (lll)T[0];
            lll lv = (lll)v;
            best_delta = wj * myabs(sj + lv - tj) - wj * myabs(sj - tj);
            best_j = 0;
        }
        for (int j = 1; j < m; j++) {
            lll wj = (lll)W[j];
            lll sj = S[j];
            lll tj = (lll)T[j];
            lll lv = (lll)v;
            lll delta = wj * myabs(sj + lv - tj) - wj * myabs(sj - tj);
            if (delta < best_delta) {
                best_delta = delta;
                best_j = j;
            }
        }
        S[best_j] += (lll)v;
        pos = end + 1;
    }

    for (int j = 0; j < m; j++) {
        lll diff = myabs(S[j] - (lll)T[j]);
        obj += (lll)W[j] * diff;
    }
    return obj;
}

int main(int argc, char *argv[]) {
    registerTestlibCmd(argc, argv);

    // --- Read problem input from inf ---
    int n = inf.readInt(1, 100000, "n");
    int m = inf.readInt(1, 50, "m");
    int D = inf.readInt(1, 18, "D");
    inf.readEoln();

    string s = inf.readToken();
    inf.readEoln();

    vector<ll> T(m), W(m);
    for (int j = 0; j < m; j++) T[j] = inf.readLong();
    inf.readEoln();
    for (int j = 0; j < m; j++) W[j] = inf.readLong();
    inf.readEoln();

    vector<ll> C(n - 1, 0);
    for (int i = 0; i < n - 1; i++) {
        C[i] = inf.readLong();
        if (i < n - 2) {
            // more to read
        }
    }
    // Don't strictly require eof on inf

    // Compute baseline
    lll Base = computeBaseline(n, m, D, s, T, W, C);

    // --- Read participant output from ouf ---
    int q = ouf.readInt(1, n, "q");
    ouf.readEoln();

    vector<int> r(q), a(q);
    for (int i = 0; i < q; i++) {
        r[i] = ouf.readInt(1, n, "r");
        if (i < q - 1) {
            // space between
        }
    }
    ouf.readEoln();
    for (int i = 0; i < q; i++) {
        a[i] = ouf.readInt(1, m, "a");
        if (i < q - 1) {
            // space between
        }
    }
    ouf.readEoln();
    // Consume any trailing whitespace / EOF
    ouf.readEof();

    // --- Validate participant output ---
    // 1) r must be strictly increasing, r[q-1] == n
    for (int i = 0; i < q; i++) {
        if (i > 0 && r[i] <= r[i - 1]) {
            quitf(_wa, "r[%d]=%d is not strictly greater than r[%d]=%d", i + 1, r[i], i, r[i - 1]);
        }
    }
    if (r[q - 1] != n) {
        quitf(_wa, "Last r[%d]=%d must equal n=%d", q, r[q - 1], n);
    }
    // 2) Each chunk length in [1, D]
    {
        int prev = 0;
        for (int i = 0; i < q; i++) {
            int len = r[i] - prev;
            if (len < 1 || len > D) {
                quitf(_wa, "Chunk %d has length %d, must be in [1, %d]", i + 1, len, D);
            }
            prev = r[i];
        }
    }

    // --- Compute participant's objective ---
    lll Obj = 0;
    vector<lll> S(m, 0);
    {
        int prev = 0; // 0-based
        for (int i = 0; i < q; i++) {
            int l0 = prev;          // 0-based start
            int r0 = r[i] - 1;     // 0-based end
            ull v = parseChunk(s, l0, r0);
            int j = a[i] - 1;
            S[j] += (lll)v;

            // Cut cost: if not last chunk, cut after r[i] (1-based position)
            // => C[r[i]-1] in 0-based C array
            if (i < q - 1) {
                Obj += (lll)C[r[i] - 1];
            }
            prev = r[i];
        }
    }
    for (int j = 0; j < m; j++) {
        lll diff = myabs(S[j] - (lll)T[j]);
        Obj += (lll)W[j] * diff;
    }

    // --- Compute score ratio ---
    double score_ratio;
    if (Base == (lll)0) {
        // baseline is 0
        if (Obj == (lll)0) {
            score_ratio = 1.0;
        } else {
            score_ratio = 0.0;
        }
    } else {
        if (Obj < (lll)0) {
            quitf(_wa, "Negative objective computed, internal error");
        }
        double dBase = lll_to_double(Base);
        double dObj  = lll_to_double(Obj);
        // score = 1,000,000 * Base / (Base + Obj)  normalized to [0,1]
        score_ratio = dBase / (dBase + dObj);
        if (score_ratio < 0.0) score_ratio = 0.0;
        if (score_ratio > 1.0) score_ratio = 1.0;
    }

    double dBase2 = lll_to_double(Base);
    double dObj2  = lll_to_double(Obj);

    quitp(score_ratio, "Ratio: %.9f | Obj=%.0f Base=%.0f", score_ratio, dObj2, dBase2);

    return 0;
}