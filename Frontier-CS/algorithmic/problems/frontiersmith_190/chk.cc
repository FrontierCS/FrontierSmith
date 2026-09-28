#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

static int omegaOf(int y) {
    int cnt = 0;
    int tmp = y;
    for (int d = 2; (long long)d * d <= tmp; d++) {
        if (tmp % d == 0) {
            cnt++;
            while (tmp % d == 0) tmp /= d;
        }
    }
    if (tmp > 1) cnt++;
    return cnt;
}

static int gcd2(int a, int b) {
    while (b) { int t = a % b; a = b; b = t; }
    return a;
}

// MCMF for max-profit assignment
struct MCMF {
    struct Edge { int to, cap, cost, flow; };
    int N;
    vector<Edge> edges;
    vector<vector<int>> g;
    vector<int> dist, par, parEdge;
    vector<bool> inq;

    MCMF(int N_) : N(N_), g(N_), dist(N_), par(N_), parEdge(N_), inq(N_) {}

    void addEdge(int u, int v, int cap, int cost) {
        g[u].push_back((int)edges.size());
        edges.push_back({v, cap, cost, 0});
        g[v].push_back((int)edges.size());
        edges.push_back({u, 0, -cost, 0});
    }

    bool spfa(int s, int t) {
        fill(dist.begin(), dist.end(), INT_MAX);
        fill(inq.begin(), inq.end(), false);
        dist[s] = 0; inq[s] = true;
        queue<int> q;
        q.push(s);
        while (!q.empty()) {
            int u = q.front(); q.pop();
            inq[u] = false;
            for (int id : g[u]) {
                auto& e = edges[id];
                if (e.cap > e.flow && dist[u] != INT_MAX && dist[u] + e.cost < dist[e.to]) {
                    dist[e.to] = dist[u] + e.cost;
                    par[e.to] = u;
                    parEdge[e.to] = id;
                    if (!inq[e.to]) { inq[e.to] = true; q.push(e.to); }
                }
            }
        }
        return dist[t] != INT_MAX;
    }

    long long maxProfit(int s, int t) {
        long long profit = 0;
        while (spfa(s, t)) {
            if (dist[t] >= 0) break;
            // find bottleneck
            int flow = INT_MAX;
            int v = t;
            while (v != s) {
                flow = min(flow, edges[parEdge[v]].cap - edges[parEdge[v]].flow);
                v = par[v];
            }
            v = t;
            while (v != s) {
                edges[parEdge[v]].flow += flow;
                edges[parEdge[v] ^ 1].flow -= flow;
                v = par[v];
            }
            profit += -(long long)dist[t] * flow;
        }
        return profit;
    }
};

static long long computeMaxProfit(
    const vector<int>& rx, const vector<int>& rs,
    const vector<int>& rp, const vector<int>& rw,
    const vector<int>& freqs)
{
    int n = (int)rx.size();
    int m = (int)freqs.size();
    if (m == 0) return 0LL;

    // nodes: 0=source, 1=sink, 2..2+n-1=receivers, 2+n..2+n+m-1=frequencies
    int S = 0, T = 1;
    MCMF mcmf(2 + n + m);

    // source -> each receiver with capacity 1, cost -w (profit)
    for (int i = 0; i < n; i++) {
        mcmf.addEdge(S, 2 + i, 1, -rw[i]);
    }
    // each frequency -> sink with capacity cap(y)
    for (int j = 0; j < m; j++) {
        int y = freqs[j];
        int cap = 1 + omegaOf(y);
        mcmf.addEdge(2 + n + j, T, cap, 0);
    }
    // receiver -> frequency if compatible
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int y = freqs[j];
            if (y > rx[i] && y <= rx[i] + rs[i] && gcd2(y, rp[i]) == 1) {
                mcmf.addEdge(2 + i, 2 + n + j, 1, 0);
            }
        }
    }

    return mcmf.maxProfit(S, T);
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // Read input
    int n = inf.readInt();
    int U = inf.readInt();
    int B = inf.readInt();

    vector<int> rx(n), rs(n), rp(n), rw(n);
    long long W = 0;
    for (int i = 0; i < n; i++) {
        rx[i] = inf.readInt();
        rs[i] = inf.readInt();
        rp[i] = inf.readInt();
        rw[i] = inf.readInt();
        W += rw[i];
    }

    // Compute baseline
    // For each receiver, find f_i = smallest y in (x_i, x_i+s_i] with gcd(y,p_i)=1
    map<int, long long> bucket;
    for (int i = 0; i < n; i++) {
        for (int y = rx[i] + 1; y <= rx[i] + rs[i]; y++) {
            if (gcd2(y, rp[i]) == 1) {
                bucket[y] += rw[i];
                break;
            }
        }
    }

    vector<pair<long long, int>> bvec;
    bvec.reserve(bucket.size());
    for (auto& kv : bucket)
        bvec.push_back({kv.second, kv.first});
    sort(bvec.begin(), bvec.end(), [](const pair<long long,int>& a, const pair<long long,int>& b) {
        if (a.first != b.first) return a.first > b.first;
        return a.second < b.second;
    });

    vector<int> baseFreqs;
    for (int i = 0; i < (int)bvec.size() && (int)baseFreqs.size() < B; i++)
        baseFreqs.push_back(bvec[i].second);

    long long V_base = computeMaxProfit(rx, rs, rp, rw, baseFreqs);

    // Read participant output
    int m = ouf.readInt(0, B, "m must be in [0, B]");

    vector<int> freqs;
    freqs.reserve(m);
    set<int> seen;
    for (int j = 0; j < m; j++) {
        int y = ouf.readInt(1, U, "frequency must be in [1, U]");
        if (seen.count(y)) {
            quitf(_wa, "duplicate frequency %d", y);
        }
        seen.insert(y);
        freqs.push_back(y);
    }

    // EOF check: reject extra output (trailing whitespace/newlines are OK, but extra tokens are not)
    if (!ouf.seekEof()) {
        quitf(_wa, "extra output after the %d frequencies", m);
    }

    long long V = computeMaxProfit(rx, rs, rp, rw, freqs);

    // Score formula: 100 * max(0, V - V_base) / max(1, W - V_base), clamped to [0,100]
    long long num = (V > V_base) ? (V - V_base) : 0LL;
    long long den = (W > V_base) ? (W - V_base) : 1LL;
    if (den <= 0) den = 1;

    double ratio = (double)num / (double)den;
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    quitp(ratio, "V=%lld V_base=%lld W=%lld Ratio: %.6f", V, V_base, W, ratio);
}