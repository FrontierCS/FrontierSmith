#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    const string ALPHA = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    const int ALEN = (int)ALPHA.size(); // 62

    // Build alphabet order map
    map<char, int> alphaOrder;
    for (int i = 0; i < ALEN; i++) alphaOrder[ALPHA[i]] = i;

    set<char> validChars(ALPHA.begin(), ALPHA.end());

    // ---- Read input ----
    int n = inf.readInt();
    int m = inf.readInt();
    int s = inf.readInt();

    map<string, long long> dict;
    for (int i = 0; i < n; i++) {
        string word = inf.readToken();
        long long weight = inf.readLong();
        dict[word] = weight;
    }

    vector<vector<int>> beams(s);
    for (int i = 0; i < s; i++) {
        int len = inf.readInt();
        beams[i].resize(len);
        for (int j = 0; j < len; j++) {
            beams[i][j] = inf.readInt() - 1; // convert to 0-indexed
        }
    }

    // ---- Read participant output ----
    // Read line / token
    string ans = ouf.readToken();

    // Check for trailing content
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra content after the answer string");
    }

    // Validate length
    if ((int)ans.size() != m) {
        quitf(_wa, "Expected string of length %d, got length %d", m, (int)ans.size());
    }
    // Validate characters
    for (char c : ans) {
        if (!validChars.count(c)) {
            quitf(_wa, "Invalid character '%c' in output", c);
        }
    }

    // ---- Compute bestWeight[L] and U ----
    map<int, long long> bestWeight;
    for (auto& kv : dict) {
        int L = (int)kv.first.size();
        long long w = kv.second;
        auto it = bestWeight.find(L);
        if (it == bestWeight.end() || it->second < w)
            bestWeight[L] = w;
    }

    long long U = 0;
    for (auto& beam : beams) {
        int L = (int)beam.size();
        auto it = bestWeight.find(L);
        if (it != bestWeight.end()) U += it->second;
    }

    // ---- Compute Obj (participant) ----
    long long Obj = 0;
    for (auto& beam : beams) {
        int L = (int)beam.size();
        string F(L, ' ');
        for (int j = 0; j < L; j++) F[j] = ans[beam[j]];
        string R = F;
        reverse(R.begin(), R.end());

        long long contrib = 0;
        auto it = dict.find(F);
        if (it != dict.end()) contrib = max(contrib, it->second);
        it = dict.find(R);
        if (it != dict.end()) contrib = max(contrib, it->second);
        Obj += contrib;
    }

    // ---- Compute baseline B ----
    // Step 1: find rep[L] for each length L
    // rep[L] = word with max weight; ties broken by lex order in ALPHA
    map<int, string> rep;
    for (auto& kv : dict) {
        int L = (int)kv.first.size();
        const string& word = kv.first;
        long long w = kv.second;
        auto it = rep.find(L);
        if (it == rep.end()) {
            rep[L] = word;
        } else {
            long long curW = dict[it->second];
            if (w > curW) {
                rep[L] = word;
            } else if (w == curW) {
                // lex smallest in ALPHA order
                const string& cur = it->second;
                bool newSmaller = false;
                for (int p = 0; p < (int)word.size(); p++) {
                    int ao = alphaOrder.count(word[p]) ? alphaOrder[word[p]] : -1;
                    int ac = alphaOrder.count(cur[p]) ? alphaOrder[cur[p]] : -1;
                    if (ao < ac) { newSmaller = true; break; }
                    if (ao > ac) { break; }
                }
                if (newSmaller) rep[L] = word;
            }
        }
    }

    // Step 2: build counter[v][c]
    // Using vector<array<long long, 62>>
    vector<array<long long, 62>> counter(m);
    for (int v = 0; v < m; v++) counter[v].fill(0LL);

    for (auto& beam : beams) {
        int L = (int)beam.size();
        auto it = rep.find(L);
        if (it == rep.end()) continue;
        const string& r = it->second;
        long long w = dict[r];
        for (int p = 0; p < L; p++) {
            int v = beam[p];
            int fwdChar = alphaOrder[r[p]];
            int revChar = alphaOrder[r[L - 1 - p]];
            counter[v][fwdChar] += w;
            counter[v][revChar] += w;
        }
    }

    // Step 3: assign character to each cube
    string baseline(m, '0');
    for (int v = 0; v < m; v++) {
        long long maxVal = 0;
        int bestIdx = -1;
        for (int c = 0; c < ALEN; c++) {
            if (counter[v][c] > maxVal) {
                maxVal = counter[v][c];
                bestIdx = c;
            }
        }
        baseline[v] = (bestIdx >= 0) ? ALPHA[bestIdx] : '0';
    }

    // Step 4: compute B
    long long B = 0;
    for (auto& beam : beams) {
        int L = (int)beam.size();
        string F(L, ' ');
        for (int j = 0; j < L; j++) F[j] = baseline[beam[j]];
        string R = F;
        reverse(R.begin(), R.end());

        long long contrib = 0;
        auto it = dict.find(F);
        if (it != dict.end()) contrib = max(contrib, it->second);
        it = dict.find(R);
        if (it != dict.end()) contrib = max(contrib, it->second);
        B += contrib;
    }

    // ---- Compute integer score in [0, 1000000] exactly as stated ----
    // If U = B:
    //   score = 1000000 if Obj == U, else 0
    // If U > B:
    //   score = floor(1000000 * max(0, Obj - B) / (U - B)), clamped to [0, 1000000]
    long long score_int = 0;
    if (U == B) {
        score_int = (Obj >= U) ? 1000000LL : 0LL;
    } else {
        // U > B (B <= U by construction since B is achievable and U is an upper bound)
        long long num = Obj - B;
        if (num <= 0) {
            score_int = 0;
        } else {
            long long denom = U - B;
            // floor(1000000 * num / denom) using __int128 to avoid overflow
            __int128 numerator = (__int128)1000000LL * (__int128)num;
            __int128 denominator = (__int128)denom;
            long long floored = (long long)(numerator / denominator);
            if (floored > 1000000LL) floored = 1000000LL;
            score_int = floored;
        }
    }

    // ratio in [0, 1] for quitp
    double ratio = (double)score_int / 1000000.0;

    quitp(ratio,
          "Obj=%lld B=%lld U=%lld Score=%lld Ratio: %.9f",
          Obj, B, U, score_int, ratio);

    return 0;
}