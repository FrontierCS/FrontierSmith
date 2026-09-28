#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- read the input file ----
    string s = inf.readToken();
    int n = (int)s.size();
    int q = inf.readInt();

    vector<int> ql(q), qr(q);
    vector<long long> qw(q);
    long long T = 0;
    for (int i = 0; i < q; i++) {
        ql[i] = inf.readInt();
        qr[i] = inf.readInt();
        qw[i] = (long long)inf.readLong();
        T += qw[i];
    }

    // ---- read participant output ----
    string t = ouf.readToken();

    // Strict EOF check: reject any extra non-whitespace tokens
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra output found after the answer string");
    }

    // ---- feasibility checks ----
    if ((int)t.size() != n) {
        quitf(_wa, "Length mismatch: expected %d, got %d", n, (int)t.size());
    }
    for (int i = 0; i < n; i++) {
        if (t[i] < 'a' || t[i] > 'z') {
            quitf(_wa, "Non-lowercase character '%c' at position %d", t[i], i + 1);
        }
    }
    {
        int fs[26] = {}, ft[26] = {};
        for (char c : s) fs[(int)(c - 'a')]++;
        for (char c : t) ft[(int)(c - 'a')]++;
        for (int c = 0; c < 26; c++) {
            if (fs[c] != ft[c]) {
                quitf(_wa,
                      "t is not an anagram of s: char '%c' count s=%d t=%d",
                      (char)('a' + c), fs[c], ft[c]);
            }
        }
    }

    // ---- compute D-state hashes (two independent 64-bit hashes) ----
    // D[i] = prefix_count(t,i) - prefix_count(s,i)
    // We use testlib's rnd (seeded from argv) for random weights.
    unsigned long long rw1[26], rw2[26];
    for (int c = 0; c < 26; c++) {
        rw1[c] = (unsigned long long)rnd.next((long long)1, (long long)4e18);
        rw2[c] = (unsigned long long)rnd.next((long long)1, (long long)4e18);
    }

    // Build hash arrays for participant's t
    vector<unsigned long long> hD1(n + 1, 0ULL), hD2(n + 1, 0ULL);
    for (int i = 0; i < n; i++) {
        int ci_t = t[i] - 'a';
        int ci_s = s[i] - 'a';
        hD1[i + 1] = hD1[i] + rw1[ci_t] - rw1[ci_s];
        hD2[i + 1] = hD2[i] + rw2[ci_t] - rw2[ci_s];
    }

    // Map each hash state to sorted list of positions where it occurs
    typedef pair<unsigned long long, unsigned long long> PUU;
    map<PUU, vector<int>> posMap;
    for (int i = 0; i <= n; i++) {
        posMap[{hD1[i], hD2[i]}].push_back(i);
    }

    // ---- compute O: participant's objective ----
    long long O = 0;
    for (int i = 0; i < q; i++) {
        int l = ql[i], r = qr[i];
        if (hD1[r] != hD1[l - 1] || hD2[r] != hD2[l - 1]) continue;
        PUU key = {hD1[l - 1], hD2[l - 1]};
        auto it = posMap.find(key);
        if (it == posMap.end()) {
            O += qw[i];
            continue;
        }
        const vector<int>& positions = it->second;
        // Check no j in [l, r-1] with D[j] == D[l-1]
        auto lb = lower_bound(positions.begin(), positions.end(), l);
        if (lb == positions.end() || *lb > r - 1) {
            O += qw[i];
        }
    }

    // ---- compute B: baseline objective (t = s) ----
    // For t=s, D[i]=0 for all i. Query [l,r] is satisfied iff D[r]==D[l-1]
    // (always true) AND no j in [l, r-1] has D[j]==D[l-1] (always 0).
    // That means no interior position j in [l, r-1] has D[j]=0, which is
    // impossible unless l==r (length-1 queries).
    long long B = 0;
    for (int i = 0; i < q; i++) {
        if (ql[i] == qr[i]) {
            B += qw[i];
        }
    }

    // ---- compute score using the stated discrete formula ----
    // score = floor(1,000,000 * max(0, O - B) / (T - B))  if T != B
    // score = 1,000,000  if T == B and O == T
    // score = 0          if T == B and O != T
    long long score_int;
    if (T == B) {
        score_int = (O >= T) ? 1000000LL : 0LL;
    } else {
        long long num = O - B;
        if (num < 0) num = 0;
        long long denom = T - B; // > 0
        // Use __int128 to avoid overflow (num up to ~2e14, *1e6 = 2e20)
        __int128 big_num = (__int128)1000000 * (__int128)num;
        __int128 big_den = (__int128)denom;
        __int128 big_score = big_num / big_den;
        if (big_score > 1000000) big_score = 1000000;
        if (big_score < 0) big_score = 0;
        score_int = (long long)big_score;
    }

    // Pass discrete score as ratio to quitp
    // quitp multiplies by 1,000,000 internally; we pass score_int/1,000,000
    double score_ratio = (double)score_int / 1000000.0;
    if (score_ratio > 1.0) score_ratio = 1.0;
    if (score_ratio < 0.0) score_ratio = 0.0;

    quitp(score_ratio,
          "Feasible. O=%lld B=%lld T=%lld score_int=%lld Ratio: %.9f",
          O, B, T, score_int, score_ratio);
    return 0;
}