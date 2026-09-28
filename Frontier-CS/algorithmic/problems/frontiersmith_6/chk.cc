#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- Read problem input from inf ----
    long long n = inf.readLong();
    long long C = inf.readLong();
    long long m = inf.readLong();
    long long K = inf.readLong();
    long long B = inf.readLong();
    long long F = inf.readLong();

    vector<long long> p(C + 1);
    for (int c = 1; c <= (int)C; c++) p[c] = inf.readLong();

    vector<int> a(n + 1);
    for (int i = 1; i <= (int)n; i++) a[i] = inf.readInt();

    vector<int> lq(m), rq(m);
    vector<long long> wq(m);
    for (int j = 0; j < (int)m; j++) {
        lq[j] = inf.readInt();
        rq[j] = inf.readInt();
        wq[j] = inf.readLong();
    }

    // ---- Compute upper bound: sum(w_j) * P_max ----
    long long pmax = *max_element(p.begin() + 1, p.end());
    long long upper = 0;
    for (int j = 0; j < (int)m; j++) upper += wq[j] * pmax;

    // ---- Helper: compute revenue for array x ----
    auto compute_revenue = [&](const vector<int>& x) -> long long {
        // build sorted positions per product for binary search
        vector<vector<int>> pos(C + 1);
        for (int i = 1; i <= (int)n; i++) pos[x[i]].push_back(i);

        long long total = 0;
        for (int j = 0; j < (int)m; j++) {
            int L = lq[j], R = rq[j];
            int len = R - L + 1;

            // Boyer-Moore majority vote to find candidate
            int candidate = 0, cnt = 0;
            for (int i = L; i <= R; i++) {
                if (cnt == 0) { candidate = x[i]; cnt = 1; }
                else if (x[i] == candidate) cnt++;
                else cnt--;
            }

            if (candidate < 1 || candidate > (int)C) continue;

            // Verify via binary search on sorted positions
            auto& v = pos[candidate];
            int c_cnt = (int)(upper_bound(v.begin(), v.end(), R)
                             - lower_bound(v.begin(), v.end(), L));

            if (2 * c_cnt > len) {
                total += wq[j] * p[candidate];
            }
        }
        return total;
    };

    // ---- Compute baseline revenue (no campaigns) ----
    long long base_revenue = compute_revenue(a);

    // ---- Read participant output from ouf ----
    int s = ouf.readInt();
    if (s < 0 || s > (int)K)
        quitf(_wa, "s=%d is out of range [0, %lld]", s, K);

    struct Campaign { int L, R, c; };
    vector<Campaign> campaigns(s);
    vector<pair<int,int>> intervals;
    long long total_cost = 0;

    for (int t = 0; t < s; t++) {
        int L = ouf.readInt();
        int R = ouf.readInt();
        int c = ouf.readInt();

        if (L < 1 || L > (int)n)
            quitf(_wa, "Campaign %d: L=%d out of range [1, %lld]", t+1, L, n);
        if (R < 1 || R > (int)n)
            quitf(_wa, "Campaign %d: R=%d out of range [1, %lld]", t+1, R, n);
        if (L > R)
            quitf(_wa, "Campaign %d: L=%d > R=%d", t+1, L, R);
        if (c < 1 || c > (int)C)
            quitf(_wa, "Campaign %d: c=%d out of range [1, %lld]", t+1, c, C);

        long long cost = F + (long long)(R - L + 1);
        total_cost += cost;
        if (total_cost > B)
            quitf(_wa, "Total cost %lld exceeds budget B=%lld after campaign %d",
                  total_cost, B, t+1);

        intervals.push_back({L, R});
        campaigns[t] = {L, R, c};
    }

    // Check pairwise disjoint
    {
        vector<pair<int,int>> sorted_ivs = intervals;
        sort(sorted_ivs.begin(), sorted_ivs.end());
        for (int i = 1; i < (int)sorted_ivs.size(); i++) {
            if (sorted_ivs[i].first <= sorted_ivs[i-1].second) {
                quitf(_wa,
                    "Campaigns have overlapping intervals: [%d,%d] and [%d,%d]",
                    sorted_ivs[i-1].first, sorted_ivs[i-1].second,
                    sorted_ivs[i].first,   sorted_ivs[i].second);
            }
        }
    }

    // ---- Strict EOF check: reject any trailing non-whitespace tokens ----
    if (!ouf.seekEof())
        quitf(_wa, "Extra output found after all campaigns were read");

    // ---- Apply campaigns to get final array ----
    vector<int> x(a.begin(), a.end());
    for (auto& camp : campaigns) {
        for (int i = camp.L; i <= camp.R; i++) x[i] = camp.c;
    }

    // ---- Compute participant revenue ----
    long long your_revenue = compute_revenue(x);

    // ---- Compute continuous score ----
    double score;
    if (upper == base_revenue) {
        // Already at upper bound; any feasible solution scores 1
        score = 1.0;
    } else {
        score = (double)(your_revenue - base_revenue)
              / (double)(upper - base_revenue);
        if (score < 0.0) score = 0.0;
        if (score > 1.0) score = 1.0;
    }

    quitp(score,
          "Your=%lld Base=%lld Upper=%lld Ratio: %.6f",
          your_revenue, base_revenue, upper, score);

    return 0;
}