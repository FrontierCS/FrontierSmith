#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

// ── Graph for shortest‑path computation ──────────────────────────────────────
static const long long LINF = 4e18;
static const int MAXV = 3001;

struct SpEdge { int to; long long w; };
static vector<SpEdge> spg[MAXV];
static long long spd[MAXV];

void dijkstra(int src, int V) {
    fill(spd + 1, spd + V + 1, LINF);
    spd[src] = 0;
    priority_queue<pair<long long,int>, vector<pair<long long,int>>, greater<pair<long long,int>>> pq;
    pq.push({0LL, src});
    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (d > spd[u]) continue;
        for (auto& e : spg[u]) {
            if (spd[u] + e.w < spd[e.to]) {
                spd[e.to] = spd[u] + e.w;
                pq.push({spd[e.to], e.to});
            }
        }
    }
}

// ── Dinic max‑flow ───────────────────────────────────────────────────────────
static const int MAXFN = 1700;  // source + C + S + sink ≤ 1602

struct FEdge { int to, rev; long long cap; };
static vector<FEdge> fg[MAXFN];
static int flevel[MAXFN], fiter[MAXFN];
static int fnodes;

void fadd(int u, int v, long long cap) {
    fg[u].push_back({v, (int)fg[v].size(), cap});
    fg[v].push_back({u, (int)fg[u].size()-1, 0LL});
}

bool fbfs(int s, int t) {
    fill(flevel, flevel + fnodes, -1);
    queue<int> q;
    flevel[s] = 0; q.push(s);
    while (!q.empty()) {
        int v = q.front(); q.pop();
        for (auto& e : fg[v])
            if (e.cap > 0 && flevel[e.to] < 0) { flevel[e.to] = flevel[v]+1; q.push(e.to); }
    }
    return flevel[t] >= 0;
}

long long fdfs(int v, int t, long long f) {
    if (v == t) return f;
    for (int& i = fiter[v]; i < (int)fg[v].size(); i++) {
        FEdge& e = fg[v][i];
        if (e.cap > 0 && flevel[v] < flevel[e.to]) {
            long long d = fdfs(e.to, t, min(f, e.cap));
            if (d > 0) { e.cap -= d; fg[e.to][e.rev].cap += d; return d; }
        }
    }
    return 0;
}

long long computeFlow(int s, int t) {
    long long flow = 0;
    while (fbfs(s, t)) {
        fill(fiter, fiter + fnodes, 0);
        long long d;
        while ((d = fdfs(s, t, LINF)) > 0) flow += d;
    }
    return flow;
}

// ── Helper: compute objective given a set of active tube indices ─────────────
// Returns max-flow value M for the given tube subset.
// All problem data passed as parameters.
long long evaluate(
    int V, int C, int S, long long L,
    // existing edges stored separately (already in spg[] baseline)
    vector<pair<int,int>>& colV, vector<long long>& colA,
    vector<pair<int,int>>& sugV, vector<long long>& sugH,
    // tube endpoints/lengths for activated tubes
    vector<tuple<int,int,long long>>& activeTubes,
    // base edges (u,v,d) — needed to rebuild graph
    vector<tuple<int,int,long long>>& baseEdges
) {
    // Rebuild graph
    for (int i = 1; i <= V; i++) spg[i].clear();
    for (auto& [u, v, d] : baseEdges) {
        spg[u].push_back({v, d});
        spg[v].push_back({u, d});
    }
    for (auto& [a, b, t] : activeTubes) {
        spg[a].push_back({b, t});
        spg[b].push_back({a, t});
    }

    // Collect unique vertices to run Dijkstra from
    // We need dist from each colony vertex to each sugar vertex.
    // Run Dijkstra from each unique colony vertex and store distances.
    // C,S ≤ 800 each.

    // dist_col[i][j] = shortest dist from colony i's vertex to sugar j's vertex
    vector<vector<long long>> distCS(C, vector<long long>(S));

    // group colonies by vertex to avoid redundant Dijkstras
    map<int, vector<int>> colByVtx;
    for (int i = 0; i < C; i++) colByVtx[colV[i].first].push_back(i);

    for (auto& [vtx, ids] : colByVtx) {
        dijkstra(vtx, V);
        for (int i : ids)
            for (int j = 0; j < S; j++)
                distCS[i][j] = spd[sugV[j].first];
    }

    // Build flow network
    // node 0 = source, 1..C = colonies, C+1..C+S = sugar, C+S+1 = sink
    int SRC = 0, SNK = C + S + 1;
    fnodes = C + S + 2;
    for (int i = 0; i < fnodes; i++) fg[i].clear();

    for (int i = 0; i < C; i++) fadd(SRC, i+1, colA[i]);
    for (int j = 0; j < S; j++) fadd(C+1+j, SNK, sugH[j]);
    for (int i = 0; i < C; i++)
        for (int j = 0; j < S; j++)
            if (distCS[i][j] <= L)
                fadd(i+1, C+1+j, LINF);

    return computeFlow(SRC, SNK);
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ── Read problem input ────────────────────────────────────────────────────
    int V = inf.readInt();
    int E = inf.readInt();
    int P = inf.readInt();
    int C = inf.readInt();
    int S = inf.readInt();
    long long B = inf.readLong();
    long long L = inf.readLong();

    vector<tuple<int,int,long long>> baseEdges(E);
    for (int i = 0; i < E; i++) {
        int u = inf.readInt(), v = inf.readInt();
        long long d = inf.readLong();
        baseEdges[i] = {u, v, d};
    }

    // tubes: a, b, t, c
    vector<int> ta(P), tb(P);
    vector<long long> tt(P), tc(P);
    for (int i = 0; i < P; i++) {
        ta[i] = inf.readInt();
        tb[i] = inf.readInt();
        tt[i] = inf.readLong();
        tc[i] = inf.readLong();
    }

    vector<pair<int,int>> colV(C);
    vector<long long> colA(C);
    for (int i = 0; i < C; i++) {
        colV[i].first = inf.readInt();
        colA[i] = inf.readLong();
        colV[i].second = 0;
    }

    vector<pair<int,int>> sugV(S);
    vector<long long> sugH(S);
    for (int i = 0; i < S; i++) {
        sugV[i].first = inf.readInt();
        sugH[i] = inf.readLong();
        sugV[i].second = 0;
    }

    // ── Compute U ─────────────────────────────────────────────────────────────
    long long sumA = 0, sumH = 0;
    for (int i = 0; i < C; i++) sumA += colA[i];
    for (int j = 0; j < S; j++) sumH += sugH[j];
    long long U = min(sumA, sumH);

    // ── Compute M0 (no tubes) ─────────────────────────────────────────────────
    vector<tuple<int,int,long long>> noTubes;
    long long M0 = evaluate(V, C, S, L, colV, colA, sugV, sugH, noTubes, baseEdges);

    // ── Read participant output ───────────────────────────────────────────────
    int R = ouf.readInt(0, P, "R must be in [0,P]");

    vector<int> chosenIds(R);
    for (int k = 0; k < R; k++) {
        chosenIds[k] = ouf.readInt(1, P, "tube id must be in [1,P]");
    }
    ouf.readEof();

    // Check for duplicate IDs
    {
        vector<int> sorted_ids = chosenIds;
        sort(sorted_ids.begin(), sorted_ids.end());
        for (int k = 1; k < R; k++)
            if (sorted_ids[k] == sorted_ids[k-1])
                quitf(_wa, "Duplicate tube ID: %d", sorted_ids[k]);
    }

    // Check total cost
    long long totalCost = 0;
    for (int id : chosenIds) {
        totalCost += tc[id-1];
        if (totalCost > B) // early overflow guard
            quitf(_wa, "Total activation cost %lld exceeds budget %lld", totalCost, B);
    }

    // ── Compute M ─────────────────────────────────────────────────────────────
    vector<tuple<int,int,long long>> activeTubes;
    for (int id : chosenIds)
        activeTubes.push_back({ta[id-1], tb[id-1], tt[id-1]});

    long long M = evaluate(V, C, S, L, colV, colA, sugV, sugH, activeTubes, baseEdges);

    // ── Sanity: M >= M0 ───────────────────────────────────────────────────────
    if (M < M0)
        quitf(_wa, "M=%lld < M0=%lld (internal error, activating tubes cannot reduce flow)", M, M0);

    // ── Compute score ratio ───────────────────────────────────────────────────
    double ratio;
    if (U == M0) {
        // Already optimal with no tubes; any feasible answer is full score
        ratio = 1.0;
    } else {
        ratio = (double)(M - M0) / (double)(U - M0);
        if (ratio < 0.0) ratio = 0.0;
        if (ratio > 1.0) ratio = 1.0;
    }

    // Emit required tag "Ratio: <ratio>" so the judge can parse it
    quitp(ratio,
        "M=%lld M0=%lld U=%lld R=%d cost=%lld budget=%lld | Ratio: %.9f",
        M, M0, U, R, totalCost, B, ratio);
}