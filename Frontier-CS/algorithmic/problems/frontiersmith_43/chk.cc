#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

// Largest d such that suffix of A of length d equals prefix of B of length d.
static int computeOverlap(const string &A, const string &B) {
    if (A.empty() || B.empty()) return 0;
    string combined = B + "#" + A;
    int n = (int)combined.size();
    vector<int> fail(n, 0);
    for (int i = 1; i < n; i++) {
        int j = fail[i - 1];
        while (j > 0 && combined[i] != combined[j]) j = fail[j - 1];
        if (combined[i] == combined[j]) j++;
        fail[i] = j;
    }
    return fail[n - 1];
}

int main(int argc, char *argv[]) {
    registerTestlibCmd(argc, argv);

    // ---------- read input ----------
    int N = inf.readInt();
    int P = inf.readInt();
    inf.readEoln();
    string S = inf.readToken();

    if ((int)S.size() != N)
        quitf(_fail, "Input string length mismatch: expected %d got %d", N, (int)S.size());

    long long R[26][26];
    memset(R, 0, sizeof(R));
    for (int i = 0; i < P; i++) {
        string c1s = inf.readToken();
        string c2s = inf.readToken();
        long long w  = inf.readLong();
        if (c1s.size() != 1 || c2s.size() != 1)
            quitf(_fail, "Invalid reward pair in input");
        R[c1s[0] - 'a'][c2s[0] - 'a'] = w;
    }

    // ---------- read participant output ----------
    int k = ouf.readInt(1, N, "k must be between 1 and N");

    vector<int> L(k), Rv(k);
    for (int i = 0; i < k; i++) {
        L[i]  = ouf.readInt(1, N, ("l_" + to_string(i+1) + " out of range").c_str());
        Rv[i] = ouf.readInt(1, N, ("r_" + to_string(i+1) + " out of range").c_str());
    }

    // Strict EOF check: reject any trailing tokens
    if (!ouf.seekEof())
        quitf(_wa, "Extra output after the last fragment");

    // ---------- validate feasibility ----------
    // Check each interval and coverage
    vector<bool> covered(N + 1, false);
    for (int i = 0; i < k; i++) {
        int l = L[i], r = Rv[i];
        if (l > r)
            quitf(_wa, "Fragment %d: l=%d > r=%d", i + 1, l, r);
        if (l < 1 || r > N)
            quitf(_wa, "Fragment %d: [%d,%d] out of [1,%d]", i + 1, l, r, N);
        int len = r - l + 1;
        if (len % 2 != 0)
            quitf(_wa, "Fragment %d: length %d is odd (must be even)", i + 1, len);
        for (int pos = l; pos <= r; pos++) {
            if (covered[pos])
                quitf(_wa, "Fragment %d: position %d already covered", i + 1, pos);
            covered[pos] = true;
        }
    }
    for (int pos = 1; pos <= N; pos++) {
        if (!covered[pos])
            quitf(_wa, "Position %d is not covered by any fragment", pos);
    }

    // ---------- build processed fragments ----------
    // For fragment [l,r] (1-indexed), length 2m:
    //   processed = reverse(S[l..l+m-1]) + reverse(S[l+m..r])
    vector<string> frags(k);
    for (int i = 0; i < k; i++) {
        int l = L[i] - 1;  // 0-indexed
        int r = Rv[i] - 1; // 0-indexed
        int len = r - l + 1;
        int m = len / 2;
        string f(len, 0);
        for (int j = 0; j < m; j++) f[j] = S[l + m - 1 - j];
        for (int j = 0; j < m; j++) f[m + j] = S[r - j];
        frags[i] = f;
    }

    // ---------- compute participant's V ----------
    // Build full banner
    string banner = "";
    for (int i = 0; i < k; i++) banner += frags[i];

    long long flow_score = 0;
    for (int i = 0; i + 1 < N; i++) {
        flow_score += R[banner[i] - 'a'][banner[i + 1] - 'a'];
    }

    long long seam_score = 0;
    for (int i = 0; i + 1 < k; i++) {
        long long ov = (long long)computeOverlap(frags[i], frags[i + 1]);
        seam_score += ov * ov;
    }

    long long V = flow_score + seam_score;

    // ---------- compute baseline B ----------
    // Baseline: consecutive fragments of length 2: [1,2],[3,4],...,[N-1,N]
    // Processing length-2 fragment [l,l+1]: reverse(S[l]) + reverse(S[l+1]) = S[l]S[l+1] (unchanged)
    // So baseline banner = S

    long long B_flow = 0;
    for (int i = 0; i + 1 < N; i++) {
        B_flow += R[S[i] - 'a'][S[i + 1] - 'a'];
    }

    int num_base = N / 2;
    long long B_seam = 0;
    for (int i = 0; i + 1 < num_base; i++) {
        string A(1, S[2 * i]);
        A += S[2 * i + 1];
        string Bstr(1, S[2 * (i + 1)]);
        Bstr += S[2 * (i + 1) + 1];
        long long ov = (long long)computeOverlap(A, Bstr);
        B_seam += ov * ov;
    }

    long long B_beauty = B_flow + B_seam;

    // ---------- scoring ----------
    // Per-file score = 100 * min(10, (V+1)/(B+1))
    // ratio for judge  = min(1.0, (V+1) / (10.0*(B+1)))
    double ratio;
    if (B_beauty == 0 && V == 0) {
        ratio = 0.1;
    } else {
        double numerator   = (double)(V + 1);
        double denominator = 10.0 * (double)(B_beauty + 1);
        ratio = numerator / denominator;
        if (ratio > 1.0) ratio = 1.0;
        if (ratio < 0.0) ratio = 0.0;
    }

    double display_score = 100.0 * min(10.0, (double)(V + 1) / (double)(B_beauty + 1));

    quitp(ratio,
          "OK: V=%lld (flow=%lld seam=%lld), B=%lld (flow=%lld seam=%lld), "
          "display_score=%.4f, Ratio: %.6f",
          V, flow_score, seam_score,
          B_beauty, B_flow, B_seam,
          display_score,
          ratio);
}