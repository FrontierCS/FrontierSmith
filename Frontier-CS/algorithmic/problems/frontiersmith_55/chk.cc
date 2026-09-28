#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

// ── Dinic max-flow (undirected) for 2-edge-disjoint path check ──────────────
struct FlowEdge { int to, rev, cap; };

struct Dinic {
    int N;
    vector<vector<FlowEdge>> g;
    vector<int> level, it;
    Dinic() : N(0) {}
    Dinic(int n) : N(n), g(n), level(n), it(n) {}

    void addEdge(int u, int v, int cap) {
        int su = (int)g[u].size(), sv = (int)g[v].size();
        g[u].push_back({v, sv, cap});
        g[v].push_back({u, su, 0});
    }

    void addUndirected(int u, int v) {
        addEdge(u, v, 1);
        addEdge(v, u, 1);
    }

    bool bfs(int s, int t) {
        fill(level.begin(), level.end(), -1);
        queue<int> q;
        level[s] = 0; q.push(s);
        while (!q.empty()) {
            int v = q.front(); q.pop();
            for (auto& e : g[v])
                if (e.cap > 0 && level[e.to] < 0) {
                    level[e.to] = level[v] + 1;
                    q.push(e.to);
                }
        }
        return level[t] >= 0;
    }

    int dfs(int v, int t, int f) {
        if (v == t) return f;
        for (int& i = it[v]; i < (int)g[v].size(); ++i) {
            FlowEdge& e = g[v][i];
            if (e.cap > 0 && level[v] < level[e.to]) {
                int d = dfs(e.to, t, min(f, e.cap));
                if (d > 0) { e.cap -= d; g[e.to][e.rev].cap += d; return d; }
            }
        }
        return 0;
    }

    int maxFlow(int s, int t, int need) {
        int flow = 0;
        while (flow < need && bfs(s, t)) {
            fill(it.begin(), it.end(), 0);
            int d;
            while (flow < need && (d = dfs(s, t, need - flow)) > 0)
                flow += d;
        }
        return flow;
    }
};

struct Event {
    char type;
    int a, b;
    long long w;
};

long long simulate(
    int n, int m, int c,
    const vector<int>& eu, const vector<int>& ev_arr,
    const vector<int>& cau, const vector<int>& cav,
    const vector<bool>& fort,
    const vector<bool>& built,
    const vector<Event>& events)
{
    // Build initial adjacency: all existing edges + built candidate edges
    // We track which existing edges are currently present
    vector<bool> present(m, true);

    long long score = 0;

    for (auto& ev : events) {
        if (ev.type == 'Z') {
            int i = ev.a; // 0-indexed edge
            if (!fort[i]) {
                present[i] = false;
            }
        } else {
            // P event: check 2-edge-disjoint paths
            int x = ev.a, y = ev.b;
            // Build flow network
            Dinic din(n);
            for (int i = 0; i < m; ++i) {
                if (present[i]) {
                    din.addUndirected(eu[i], ev_arr[i]);
                }
            }
            for (int j = 0; j < c; ++j) {
                if (built[j]) {
                    din.addUndirected(cau[j], cav[j]);
                }
            }
            int fl = din.maxFlow(x, y, 2);
            if (fl >= 2) {
                score += ev.w;
            }
        }
    }
    return score;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ── Read input ─────────────────────────────────────────────────────────────
    int n = inf.readInt();
    int m = inf.readInt();
    int c = inf.readInt();
    int q = inf.readInt();
    long long K = inf.readLong();

    vector<int> eu(m), ev_arr(m);
    vector<long long> hf(m);
    for (int i = 0; i < m; ++i) {
        eu[i] = inf.readInt() - 1;
        ev_arr[i] = inf.readInt() - 1;
        hf[i] = inf.readLong();
    }

    vector<int> cau(c), cav(c);
    vector<long long> cb(c);
    for (int j = 0; j < c; ++j) {
        cau[j] = inf.readInt() - 1;
        cav[j] = inf.readInt() - 1;
        cb[j] = inf.readLong();
    }

    vector<Event> events(q);
    long long U = 0;
    for (int i = 0; i < q; ++i) {
        string tok = inf.readToken();
        events[i].type = tok[0];
        if (tok[0] == 'Z') {
            events[i].a = inf.readInt() - 1; // 0-indexed
            events[i].b = 0;
            events[i].w = 0;
        } else {
            events[i].a = inf.readInt() - 1;
            events[i].b = inf.readInt() - 1;
            events[i].w = inf.readLong();
            U += events[i].w;
        }
    }

    // ── Read participant output ────────────────────────────────────────────────
    string S = ouf.readToken();
    string T = ouf.readToken();

    // ── EOF check: reject any trailing tokens ──────────────────────────────────
    if (!ouf.seekEof())
        quitf(_wa, "Extra output after the two required lines");

    // ── Validate lengths ───────────────────────────────────────────────────────
    if ((int)S.size() != m)
        quitf(_wa, "S has length %d, expected %d", (int)S.size(), m);
    if ((int)T.size() != c)
        quitf(_wa, "T has length %d, expected %d", (int)T.size(), c);

    for (int i = 0; i < m; ++i)
        if (S[i] != '0' && S[i] != '1')
            quitf(_wa, "S[%d] is '%c', expected '0' or '1'", i, S[i]);
    for (int j = 0; j < c; ++j)
        if (T[j] != '0' && T[j] != '1')
            quitf(_wa, "T[%d] is '%c', expected '0' or '1'", j, T[j]);

    // ── Check feasibility ──────────────────────────────────────────────────────
    long long HF = 0, CB = 0;
    for (int i = 0; i < m; ++i) if (S[i] == '1') HF += hf[i];
    for (int j = 0; j < c; ++j) if (T[j] == '1') CB += cb[j];

    if (HF + CB > K)
        quitf(_wa, "Infeasible: HF+CB=%lld > K=%lld", HF + CB, K);

    // ── Compute baseline B (no fortification, no build) ────────────────────────
    vector<bool> fort_none(m, false);
    vector<bool> built_none(c, false);
    long long B = simulate(n, m, c, eu, ev_arr, cau, cav, fort_none, built_none, events);

    // ── Compute participant objective O ────────────────────────────────────────
    vector<bool> fort_p(m), built_p(c);
    for (int i = 0; i < m; ++i) fort_p[i]  = (S[i] == '1');
    for (int j = 0; j < c; ++j) built_p[j] = (T[j] == '1');
    long long O = simulate(n, m, c, eu, ev_arr, cau, cav, fort_p, built_p, events);

    // ── Score ──────────────────────────────────────────────────────────────────
    // score = floor(1,000,000 * max(0, O-B) / (U-B))   when U != B
    // score = 1,000,000                                  when U == B
    long long int_score;
    if (U == B) {
        int_score = 1000000LL;
    } else {
        long long num = max(0LL, O - B);
        long long den = U - B;
        // den > 0 because U >= B always (baseline can only score <= U)
        int_score = (long long)((__int128)1000000LL * num / den);
        if (int_score > 1000000LL) int_score = 1000000LL;
        if (int_score < 0LL)       int_score = 0LL;
    }

    double ratio = (double)int_score / 1000000.0;
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    quitp(ratio,
          "O=%lld B=%lld U=%lld HF+CB=%lld int_score=%lld Ratio: %.9f",
          O, B, U, HF + CB, int_score, ratio);
}