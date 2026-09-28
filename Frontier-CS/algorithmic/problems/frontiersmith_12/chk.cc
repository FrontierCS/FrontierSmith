#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

// Compute TotalBuzz for a given lineup (0-indexed positions, 1-indexed friend ids)
// p: 1-indexed popularity array
// bad: set of unordered pairs of friends that do NOT know each other
long long computeBuzz(const vector<int>& lineup, const vector<long long>& p,
                      const set<pair<int,int>>& bad) {
    int K = (int)lineup.size();
    // For each position r (0-indexed), find l_min[r] = smallest l such that [l..r] is harmonious
    // l_min[r] = max(l_min[r-1], 1 + max index i < r where (lineup[i], lineup[r]) are in bad)
    long long totalBuzz = 0;

    // prefix_p[i] = sum of p[lineup[0..i-1]] (1-indexed, prefix_p[0]=0)
    // prefix_jp[i] = sum of j*p[lineup[j]] for j=0..i-1 (0-indexed j)
    vector<long long> prefix_p(K + 1, 0), prefix_jp(K + 1, 0);
    for (int i = 0; i < K; i++) {
        prefix_p[i + 1] = prefix_p[i] + p[lineup[i]];
        prefix_jp[i + 1] = prefix_jp[i] + (long long)i * p[lineup[i]];
    }

    int l_min = 0;
    for (int r = 0; r < K; r++) {
        // Update l_min: check who lineup[r] doesn't know among current window
        // We need: new l_min = max(l_min, 1 + last i < r where bad(lineup[i], lineup[r]))
        for (int i = l_min; i < r; i++) {
            int u = lineup[i], v = lineup[r];
            if (u > v) swap(u, v);
            if (bad.count({u, v})) {
                l_min = i + 1;
            }
        }
        // Harmonious blocks ending at r: [l..r] for l in [l_min, r]
        // Contribution = sum_{j=l_min}^{r} p[lineup[j]] * (j - l_min + 1)
        // = sum_{j=l_min}^{r} p[lineup[j]] * (j + 1) - l_min * sum_{j=l_min}^{r} p[lineup[j]]
        // Using 0-indexed: sum_{j=l_min}^{r} p[lineup[j]]*(j - l_min + 1)
        // = (sum_{j=l_min}^{r} j*p[j]) - l_min*(sum_{j=l_min}^{r} p[j]) + (sum_{j=l_min}^{r} p[j])
        // = (prefix_jp[r+1] - prefix_jp[l_min]) - (long long)l_min*(prefix_p[r+1]-prefix_p[l_min]) + (prefix_p[r+1]-prefix_p[l_min])
        long long sumP = prefix_p[r + 1] - prefix_p[l_min];
        long long sumJP = prefix_jp[r + 1] - prefix_jp[l_min];
        totalBuzz += sumJP - (long long)l_min * sumP + sumP;
    }
    return totalBuzz;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    int t = inf.readInt();

    double sumScores = 0.0;

    for (int tc = 0; tc < t; tc++) {
        int n = inf.readInt();
        int m = inf.readInt();
        int K = inf.readInt();

        vector<long long> p(n + 1);
        for (int i = 1; i <= n; i++) p[i] = inf.readLong();

        set<pair<int,int>> bad;
        for (int i = 0; i < m; i++) {
            int x = inf.readInt(), y = inf.readInt();
            if (x > y) swap(x, y);
            bad.insert({x, y});
        }

        // Read participant lineup for this test case
        vector<int> lineup;
        for (int i = 0; i < K; i++) {
            int v = ouf.readInt(1, n, ("lineup person " + to_string(i + 1)).c_str());
            lineup.push_back(v);
        }

        // Check distinctness
        {
            set<int> seen(lineup.begin(), lineup.end());
            if ((int)seen.size() != K) {
                quitf(_wa, "Test case %d: lineup has duplicate friends", tc + 1);
            }
        }

        // Compute participant TotalBuzz
        long long Oc = computeBuzz(lineup, p, bad);

        // Compute baseline lineup:
        // 1. Sort friends by decreasing p, tie-break by smaller index
        // 2. Take first K
        // 3. Sort those K indices in increasing order
        vector<int> order(n);
        iota(order.begin(), order.end(), 1);
        sort(order.begin(), order.end(), [&](int a, int b) {
            if (p[a] != p[b]) return p[a] > p[b];
            return a < b;
        });
        vector<int> baselineSelected(order.begin(), order.begin() + K);
        sort(baselineSelected.begin(), baselineSelected.end());
        long long Bc = computeBuzz(baselineSelected, p, bad);

        // S_c = O_c / (O_c + B_c)
        // B_c > 0 always (p_i >= 1, K >= 1)
        double Sc = (double)Oc / (double)(Oc + Bc);
        sumScores += Sc;
    }

    // Check no extra output
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra output after all test cases");
    }

    double ratio = sumScores / t;
    // Clamp to [0,1]
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    quitp(ratio, "Ratio: %f", ratio);
    return 0;
}