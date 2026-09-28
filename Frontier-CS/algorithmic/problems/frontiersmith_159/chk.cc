#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

static long long computeCost(
    const vector<int>& kept,
    const vector<int>& bought_ids,
    const vector<tuple<int,int,int,int>>& shortcuts_all,
    const vector<long long>& w,
    long long L,
    int N)
{
    int K = (int)kept.size();
    if (K == 0) return (long long)4e18;

    map<int,int> pos;
    for (int i = 0; i < K; i++) pos[kept[i]] = i;

    const long long INF = (long long)4e18;
    vector<vector<pair<int,long long>>> adj(K);
    for (int i = 0; i + 1 < K; i++)
        adj[i].push_back({i+1, 1LL});
    for (int id : bought_ids) {
        auto [u, v, c, t] = shortcuts_all[id-1];
        auto itu = pos.find(u), itv = pos.find(v);
        if (itu == pos.end() || itv == pos.end()) continue;
        adj[itu->second].push_back({itv->second, (long long)t});
    }

    vector<long long> dist(K, INF);
    dist[0] = 0;
    priority_queue<pair<long long,int>, vector<pair<long long,int>>, greater<>> pq;
    pq.push({0LL, 0});
    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (d > dist[u]) continue;
        for (auto& [nv, wt] : adj[u]) {
            if (dist[u] + wt < dist[nv]) {
                dist[nv] = dist[u] + wt;
                pq.push({dist[nv], nv});
            }
        }
    }

    long long total = 0;
    for (int i = 1; i <= N; i++) {
        int lb = (int)(lower_bound(kept.begin(), kept.end(), i) - kept.begin());
        int rep_idx = -1;
        int best_diff = INT_MAX;
        for (int cand = lb - 1; cand <= lb; cand++) {
            if (cand < 0 || cand >= K) continue;
            int diff = abs(kept[cand] - i);
            if (diff < best_diff ||
                (diff == best_diff && rep_idx >= 0 && kept[cand] < kept[rep_idx])) {
                best_diff = diff;
                rep_idx = cand;
            }
        }
        if (rep_idx < 0) rep_idx = 0;
        long long dcost = (dist[rep_idx] >= INF/2) ? (long long)4e18 : dist[rep_idx];
        long long penalty = L * (long long)abs(kept[rep_idx] - i);
        total += w[i] * (dcost + penalty);
    }
    return total;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // Read input
    int N = inf.readInt();
    int M = inf.readInt();
    int B = inf.readInt();
    long long L = inf.readLong();

    vector<long long> w(N+1);
    for (int i = 1; i <= N; i++) w[i] = inf.readLong();

    vector<tuple<int,int,int,int>> shortcuts(M);
    for (int i = 0; i < M; i++) {
        int u = inf.readInt();
        int v = inf.readInt();
        int c = inf.readInt();
        int t = inf.readInt();
        shortcuts[i] = {u, v, c, t};
    }

    // Read participant output
    // Line 1: K P
    int K = ouf.readInt(1, N, "K must be in [1,N]");
    int P = ouf.readInt(0, M, "P must be in [0,M]");
    ouf.readEoln();

    // Line 2: K kept records
    vector<int> kept(K);
    for (int i = 0; i < K; i++) {
        kept[i] = ouf.readInt(1, N, "kept record out of range");
        if (i > 0) {
            if (kept[i] <= kept[i-1])
                quitf(_wa, "Kept records not strictly increasing: kept[%d]=%d <= kept[%d]=%d",
                      i, kept[i], i-1, kept[i-1]);
        }
    }
    ouf.readEoln();

    // Feasibility check 1: record 1 must be kept
    if (kept[0] != 1)
        quitf(_wa, "Record 1 must be kept, but first kept record is %d", kept[0]);

    // Line 3: P bought shortcut IDs (possibly empty line)
    vector<int> bought;
    if (P > 0) {
        set<int> bought_set;
        for (int i = 0; i < P; i++) {
            int id = ouf.readInt(1, M, "shortcut ID out of range [1,M]");
            if (bought_set.count(id))
                quitf(_wa, "Duplicate shortcut ID %d", id);
            bought_set.insert(id);
            bought.push_back(id);
        }
    }
    // Read end of line 3 (could be empty line when P=0)
    // Use readEoln or skip to next line
    // When P=0 the solution may output an empty line or nothing; be lenient
    // readEoln is lenient about leading whitespace
    if (ouf.seekEof()) {
        // acceptable: no newline after empty shortcuts
    } else {
        ouf.readEoln();
    }

    // Check EOF (allow trailing whitespace/newlines)
    // Don't be too strict since problem says "if P=0, output an empty line"
    // but solutions may omit it. We've consumed everything we need.
    // Skip remaining whitespace and assert EOF.
    while (!ouf.seekEof()) {
        char c = ouf.readChar();
        if (c != ' ' && c != '\n' && c != '\r' && c != '\t')
            quitf(_wa, "Unexpected trailing non-whitespace character after output");
    }

    // Feasibility check 4: shortcut endpoints must be kept
    {
        set<int> kset(kept.begin(), kept.end());
        for (int id : bought) {
            auto [u, v, c, t] = shortcuts[id-1];
            if (!kset.count(u))
                quitf(_wa, "Shortcut %d: endpoint u=%d is not kept", id, u);
            if (!kset.count(v))
                quitf(_wa, "Shortcut %d: endpoint v=%d is not kept", id, v);
        }
    }

    // Feasibility check 5: memory budget
    {
        long long mem = (long long)K;
        for (int id : bought) {
            auto [u, v, c, t] = shortcuts[id-1];
            mem += c;
        }
        if (mem > (long long)B)
            quitf(_wa, "Memory used %lld exceeds budget %d", mem, B);
    }

    // Compute participant's objective
    long long C_sub = computeCost(kept, bought, shortcuts, w, L, N);

    // Compute baseline
    long long C_base;
    {
        int K0 = min(B, N);
        vector<int> base_kept;
        if (K0 == 1) {
            base_kept.push_back(1);
        } else {
            for (int tt = 0; tt < K0; tt++) {
                int rec = 1 + (int)((long long)tt * (N - 1) / (K0 - 1));
                base_kept.push_back(rec);
            }
            sort(base_kept.begin(), base_kept.end());
            base_kept.erase(unique(base_kept.begin(), base_kept.end()), base_kept.end());
        }
        vector<int> no_shortcuts;
        C_base = computeCost(base_kept, no_shortcuts, shortcuts, w, L, N);
    }

    // Compute score ratio in [0, 1]
    // Problem score = 1,000,000 * clamp(C_base / C_sub, 0, 2)
    // We map to [0,1]: ratio = clamp(C_base/C_sub, 0, 2) / 2
    double ratio;
    if (C_sub <= 0) {
        // Shouldn't happen for valid inputs, treat as perfect
        ratio = 1.0;
    } else {
        double r = (double)C_base / (double)C_sub;
        if (r < 0.0) r = 0.0;
        if (r > 2.0) r = 2.0;
        ratio = r / 2.0;
    }
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    // The judge parses "Ratio: <value>" from this message
    quitp(ratio,
          "Ratio: %.9f | C_sub=%lld C_base=%lld K=%d P=%d",
          ratio, C_sub, C_base, K, P);

    return 0;
}