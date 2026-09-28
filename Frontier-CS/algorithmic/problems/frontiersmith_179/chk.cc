#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

static int computeLcpLen(const string &a, const string &b) {
    int len = (int)min(a.size(), b.size());
    for (int i = 0; i < len; i++)
        if (a[i] != b[i]) return i;
    return len;
}

static long long frontCostOfPage(const vector<int> &page, const vector<string> &S) {
    if (page.empty()) return 0LL;
    long long cost = (long long)S[page[0]].size();
    for (int r = 1; r < (int)page.size(); r++) {
        int lcp = computeLcpLen(S[page[r-1]], S[page[r]]);
        cost += (long long)S[page[r]].size() - lcp;
    }
    return cost;
}

int main(int argc, char *argv[]) {
    registerTestlibCmd(argc, argv);

    // --- Read problem input ---
    int n = inf.readInt();
    int m = inf.readInt();
    int K = inf.readInt();
    long long B = inf.readLong();
    long long G = inf.readLong();
    long long F = inf.readLong();

    // strings are 1-indexed; S[0] unused
    vector<string> S(n + 1);
    for (int i = 1; i <= n; i++) {
        S[i] = inf.readToken();
    }

    struct Request { int u, v; long long w; };
    vector<Request> reqs(m);
    for (int h = 0; h < m; h++) {
        reqs[h].u = inf.readInt();
        reqs[h].v = inf.readInt();
        reqs[h].w = inf.readLong();
    }

    // --- Read and validate participant output ---
    vector<int> page_of(n + 1, -1);
    vector<int> pos_of(n + 1, -1);
    vector<vector<int>> pages(K);

    for (int j = 0; j < K; j++) {
        if (ouf.eof()) {
            quitf(_wa, "Page %d: unexpected end of output (expected %d pages)", j + 1, K);
        }
        int t = ouf.readInt();
        if (t < 0 || t > n) {
            quitf(_wa, "Page %d: invalid count %d", j + 1, t);
        }
        long long rawSum = 0;
        for (int r = 0; r < t; r++) {
            int idx = ouf.readInt();
            if (idx < 1 || idx > n) {
                quitf(_wa, "Page %d position %d: index %d out of range [1,%d]", j + 1, r + 1, idx, n);
            }
            if (page_of[idx] != -1) {
                quitf(_wa, "String %d appears on both page %d and page %d", idx, page_of[idx] + 1, j + 1);
            }
            rawSum += (long long)S[idx].size();
            if (rawSum > B) {
                quitf(_wa, "Page %d: raw-capacity exceeded (sum=%lld > B=%lld)", j + 1, rawSum, B);
            }
            page_of[idx] = j;
            pos_of[idx] = r + 1; // 1-based position
            pages[j].push_back(idx);
        }
    }

    // Check every string appears exactly once
    for (int i = 1; i <= n; i++) {
        if (page_of[i] == -1) {
            quitf(_wa, "String %d never placed on any page", i);
        }
    }

    // --- Compute participant's cost ---
    long long subFrontTotal = 0;
    for (int j = 0; j < K; j++) {
        subFrontTotal += frontCostOfPage(pages[j], S);
    }

    long long subMoveCost = 0;
    for (int h = 0; h < m; h++) {
        int u = reqs[h].u, v = reqs[h].v;
        long long w = reqs[h].w;
        long long x = pos_of[u], y = pos_of[v];
        if (page_of[u] == page_of[v]) {
            subMoveCost += w * (1LL + llabs(x - y));
        } else {
            subMoveCost += w * (G + x + y);
        }
    }

    long long C_sub = F * subFrontTotal + subMoveCost;

    // --- Compute baseline cost ---
    // Sort indices 1..n lexicographically by string value
    vector<int> sortedIdx(n);
    for (int i = 0; i < n; i++) sortedIdx[i] = i + 1;
    sort(sortedIdx.begin(), sortedIdx.end(), [&](int a, int b) {
        return S[a] < S[b];
    });

    vector<vector<int>> basePages(K);
    int curPage = 0;
    long long curRaw = 0;
    for (int i = 0; i < n; i++) {
        int idx = sortedIdx[i];
        long long len = (long long)S[idx].size();
        if (curRaw + len > B) {
            curPage++;
            if (curPage >= K) curPage = K - 1;
            curRaw = 0;
        }
        basePages[curPage].push_back(idx);
        curRaw += len;
    }

    vector<int> base_page_of(n + 1, -1);
    vector<int> base_pos_of(n + 1, -1);
    for (int j = 0; j < K; j++) {
        for (int r = 0; r < (int)basePages[j].size(); r++) {
            int idx = basePages[j][r];
            base_page_of[idx] = j;
            base_pos_of[idx] = r + 1;
        }
    }

    long long baseFrontTotal = 0;
    for (int j = 0; j < K; j++) {
        baseFrontTotal += frontCostOfPage(basePages[j], S);
    }

    long long baseMoveCost = 0;
    for (int h = 0; h < m; h++) {
        int u = reqs[h].u, v = reqs[h].v;
        long long w = reqs[h].w;
        long long x = base_pos_of[u], y = base_pos_of[v];
        if (base_page_of[u] == base_page_of[v]) {
            baseMoveCost += w * (1LL + llabs(x - y));
        } else {
            baseMoveCost += w * (G + x + y);
        }
    }

    long long C_base = F * baseFrontTotal + baseMoveCost;

    // --- Compute score ratio in [0, 1] ---
    // Score = floor(10^6 * min(2, C_base / C_sub))
    // We map this to ratio = min(1, min(2, C_base/C_sub) / 2) in [0,1].
    // Special case: if C_sub == 0, give maximum score.
    double ratio;
    if (C_sub <= 0) {
        // Both costs are 0 (trivial problem); give full score.
        ratio = 1.0;
    } else {
        double r = (double)C_base / (double)C_sub;
        // clamp r to [0, 2], then divide by 2 to get ratio in [0, 1]
        if (r > 2.0) r = 2.0;
        if (r < 0.0) r = 0.0;
        ratio = r / 2.0;
    }
    if (ratio > 1.0) ratio = 1.0;
    if (ratio < 0.0) ratio = 0.0;

    quitp(ratio,
        "Ratio: %.9f | C_sub=%lld C_base=%lld F=%lld front_sub=%lld move_sub=%lld front_base=%lld move_base=%lld",
        ratio,
        C_sub, C_base,
        F, subFrontTotal, subMoveCost,
        baseFrontTotal, baseMoveCost);

    return 0;
}