#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

// ---- LCS ----
int lcsLength(const string &a, const string &b) {
    int n = (int)a.size(), m = (int)b.size();
    vector<int> prev(m + 1, 0), cur(m + 1, 0);
    for (int i = 1; i <= n; i++) {
        fill(cur.begin(), cur.end(), 0);
        for (int j = 1; j <= m; j++) {
            if (a[i-1] == b[j-1]) cur[j] = prev[j-1] + 1;
            else cur[j] = max(prev[j], cur[j-1]);
        }
        swap(prev, cur);
    }
    return prev[m];
}

// ---- Min-Cost Max-Flow (SPFA based) ----
// We maximise by negating costs; only augment while path cost < 0.
struct MCMF {
    struct Edge { int to, cap, cost, flow; };
    int n;
    vector<Edge> edges;
    vector<vector<int>> g;
    vector<int> d, p, a;
    vector<bool> inq;

    MCMF(int n_) : n(n_), g(n_), d(n_), p(n_), a(n_), inq(n_) {}

    void addEdge(int from, int to, int cap, int cost) {
        g[from].push_back((int)edges.size());
        edges.push_back({to, cap, cost, 0});
        g[to].push_back((int)edges.size());
        edges.push_back({from, 0, -cost, 0});
    }

    // Returns {flow, cost}. Augments only while path cost < 0 (profit).
    pair<long long,long long> minCostFlow(int s, int t, int maxFlow) {
        long long flow = 0, cost = 0;
        while (flow < maxFlow) {
            fill(d.begin(), d.end(), INT_MAX);
            d[s] = 0;
            fill(inq.begin(), inq.end(), false);
            inq[s] = true;
            a[s] = maxFlow - (int)flow;
            queue<int> q;
            q.push(s);
            while (!q.empty()) {
                int u = q.front(); q.pop();
                inq[u] = false;
                for (int id : g[u]) {
                    Edge &e = edges[id];
                    if (e.cap > e.flow && d[e.to] > d[u] + e.cost) {
                        d[e.to] = d[u] + e.cost;
                        p[e.to] = id;
                        a[e.to] = min(a[u], e.cap - e.flow);
                        if (!inq[e.to]) { inq[e.to] = true; q.push(e.to); }
                    }
                }
            }
            if (d[t] == INT_MAX || d[t] >= 0) break; // no profitable augmenting path
            flow += a[t];
            cost += (long long)a[t] * d[t];
            int u = t;
            while (u != s) {
                edges[p[u]].flow += a[t];
                edges[p[u]^1].flow -= a[t];
                u = edges[p[u]^1].to;
            }
        }
        return {flow, cost};
    }
};

// Compute optimal assignment value given v[i][j], N fragments, K probes, capacity C.
long long computeObjValue(const vector<vector<long long>> &v, int N, int K, int C) {
    // Source = 0, fragments = 1..N, probes = N+1..N+K, sink = N+K+1
    int S = 0, T = N + K + 1;
    MCMF mcmf(T + 1);
    for (int i = 1; i <= N; i++)
        mcmf.addEdge(S, i, 1, 0);
    for (int j = 1; j <= K; j++)
        mcmf.addEdge(N + j, T, C, 0);
    for (int i = 1; i <= N; i++)
        for (int j = 1; j <= K; j++) {
            long long val = v[i-1][j-1];
            if (val > 0)
                mcmf.addEdge(i, N + j, 1, -(int)min(val, (long long)INT_MAX));
        }
    auto [fl, cost] = mcmf.minCostFlow(S, T, N);
    return -cost;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    int numCases = inf.readInt();
    long long totalScore = 0;

    const string NUCS = "ACGT";

    for (int tc = 1; tc <= numCases; tc++) {
        int N = inf.readInt();
        int K = inf.readInt();
        int M = inf.readInt();
        int C = inf.readInt();

        vector<int> w(N);
        vector<string> S(N);
        for (int i = 0; i < N; i++) {
            w[i] = inf.readInt();
            S[i] = inf.readToken();
        }

        // Read K probes from participant output; on format error, assign objPart=0
        // but do NOT abort globally.
        vector<string> probes(K);
        bool caseValid = true;
        string invalidReason = "";

        for (int j = 0; j < K; j++) {
            if (ouf.seekEof()) {
                caseValid = false;
                invalidReason = "Test case " + to_string(tc) + ": not enough probes (only " + to_string(j) + " provided, need " + to_string(K) + ")";
                // Fill remaining with empty to avoid reading further
                for (int jj = j; jj < K; jj++) probes[jj] = "";
                break;
            }
            string probe = ouf.readToken();
            // Check length
            if ((int)probe.size() != M) {
                caseValid = false;
                invalidReason = "Test case " + to_string(tc) + ", probe " + to_string(j+1) +
                    ": length " + to_string(probe.size()) + " != " + to_string(M);
                probes[j] = probe;
                // Read remaining probes to keep stream in sync
                for (int jj = j+1; jj < K; jj++) {
                    if (!ouf.seekEof()) probes[jj] = ouf.readToken();
                    else probes[jj] = "";
                }
                break;
            }
            // Check alphabet
            bool alphabetOk = true;
            for (char c : probe) {
                if (c != 'A' && c != 'C' && c != 'G' && c != 'T') {
                    alphabetOk = false;
                    break;
                }
            }
            if (!alphabetOk) {
                caseValid = false;
                invalidReason = "Test case " + to_string(tc) + ", probe " + to_string(j+1) +
                    ": invalid character in \"" + probe + "\"";
                probes[j] = probe;
                for (int jj = j+1; jj < K; jj++) {
                    if (!ouf.seekEof()) probes[jj] = ouf.readToken();
                    else probes[jj] = "";
                }
                break;
            }
            probes[j] = probe;
        }

        long long objPart = 0;
        if (caseValid) {
            // Compute participant objective
            vector<vector<long long>> vPart(N, vector<long long>(K, 0));
            for (int i = 0; i < N; i++)
                for (int j = 0; j < K; j++) {
                    int lcs = lcsLength(S[i], probes[j]);
                    vPart[i][j] = (long long)w[i] * lcs * lcs;
                }
            objPart = computeObjValue(vPart, N, K, C);
        }
        // else objPart stays 0

        // Compute baseline probes
        // Step 1: F_c for each nucleotide
        long long F[4] = {0, 0, 0, 0};
        for (int i = 0; i < N; i++) {
            for (char c : S[i]) {
                if (c == 'A') F[0] += w[i];
                else if (c == 'C') F[1] += w[i];
                else if (c == 'G') F[2] += w[i];
                else if (c == 'T') F[3] += w[i];
            }
        }
        // Step 2: sort descending by F, ties alphabetically
        vector<int> order = {0, 1, 2, 3};
        sort(order.begin(), order.end(), [&](int a, int b) {
            if (F[a] != F[b]) return F[a] > F[b];
            return a < b;
        });
        // Step 3: build K baseline probes
        vector<string> baseProbes(K);
        for (int j = 0; j < K; j++) {
            char nucChar = NUCS[order[j % 4]];
            baseProbes[j] = string(M, nucChar);
        }
        // Compute baseline objective
        vector<vector<long long>> vBase(N, vector<long long>(K, 0));
        for (int i = 0; i < N; i++)
            for (int j = 0; j < K; j++) {
                int lcs = lcsLength(S[i], baseProbes[j]);
                vBase[i][j] = (long long)w[i] * lcs * lcs;
            }
        long long objBase = computeObjValue(vBase, N, K, C);

        // Per-test score
        long long perScore = 0;
        if (objBase == 0 && objPart > 0) {
            perScore = 1000000LL;
        } else if (objBase == 0 && objPart == 0) {
            perScore = 0LL;
        } else {
            long long denom = objPart + objBase;
            if (denom > 0)
                perScore = (long long)((__int128)1000000LL * objPart / denom);
        }

        totalScore += perScore;
    }

    // Check no extra output
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra output after all test cases");
    }

    long long finalScore = 0;
    if (numCases > 0)
        finalScore = totalScore / numCases;
    if (finalScore < 0) finalScore = 0;
    if (finalScore > 1000000) finalScore = 1000000;

    double ratio = (double)finalScore / 1000000.0;

    quitp(ratio, "Ratio: %f (final score %lld / 1000000, averaged over %d test cases)",
          ratio, finalScore, numCases);

    return 0;
}