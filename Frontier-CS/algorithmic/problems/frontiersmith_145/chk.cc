#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

typedef long long ll;
typedef __int128 lll;

static lll absVal128(lll x) { return x < 0 ? -x : x; }

// Compute bundle cost: S + (W - M) + P*|W - L|
lll bundleCost(lll S, lll P, lll L, lll W, map<int,ll>& colorW) {
    lll M = 0;
    for (auto& kv : colorW) if ((lll)kv.second > M) M = (lll)kv.second;
    lll diff = absVal128(W - L);
    return S + (W - M) + P * diff;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // --- Read input ---
    int n    = inf.readInt();
    ll  S    = inf.readLong();
    ll  P    = inf.readLong();
    ll  L    = inf.readLong();

    vector<vector<int>> adj(n + 1);
    for (int i = 0; i < n - 1; i++) {
        int u = inf.readInt(), v = inf.readInt();
        adj[u].push_back(v);
        adj[v].push_back(u);
    }
    vector<ll> w(n + 1);
    for (int i = 1; i <= n; i++) w[i] = inf.readLong();
    vector<int> c(n + 1);
    for (int i = 1; i <= n; i++) c[i] = inf.readInt();

    // --- Read participant output ---
    int m = ouf.readInt(1, n, "m out of range");

    vector<int> b(n + 1);
    for (int i = 1; i <= n; i++) {
        b[i] = ouf.readInt(1, m, "bundle label out of range");
    }

    // --- EOF check: reject trailing tokens ---
    if (!ouf.seekEof()) {
        quitf(_wa, "extra output after the %d bundle labels", n);
    }

    // --- Validate: every label 1..m used ---
    vector<bool> used(m + 1, false);
    for (int i = 1; i <= n; i++) used[b[i]] = true;
    for (int t = 1; t <= m; t++) {
        if (!used[t]) quitf(_wa, "label %d not used by any vertex", t);
    }

    // --- Validate: each label induces a connected subgraph ---
    {
        vector<vector<int>> bundleVerts(m + 1);
        for (int i = 1; i <= n; i++) bundleVerts[b[i]].push_back(i);

        for (int t = 1; t <= m; t++) {
            // BFS inside the bundle using only edges where both endpoints share label t
            vector<int>& verts = bundleVerts[t];
            if (verts.empty()) continue;
            // Build a set for O(1) membership
            // Use visited array indexed by vertex
            // We only visit vertices in this bundle
            unordered_set<int> inBundle(verts.begin(), verts.end());
            unordered_set<int> visited;
            queue<int> q;
            q.push(verts[0]);
            visited.insert(verts[0]);
            while (!q.empty()) {
                int v = q.front(); q.pop();
                for (int u : adj[v]) {
                    if (inBundle.count(u) && !visited.count(u)) {
                        visited.insert(u);
                        q.push(u);
                    }
                }
            }
            if ((int)visited.size() != (int)verts.size()) {
                quitf(_wa, "bundle %d is not connected (%d reachable out of %d vertices)",
                      t, (int)visited.size(), (int)verts.size());
            }
        }
    }

    // --- Compute participant cost ---
    lll participantCost = 0;
    {
        vector<ll> bundleW(m + 1, 0);
        vector<map<int,ll>> bundleColor(m + 1);
        for (int i = 1; i <= n; i++) {
            bundleW[b[i]] += w[i];
            bundleColor[b[i]][c[i]] += w[i];
        }
        for (int t = 1; t <= m; t++) {
            participantCost += bundleCost((lll)S, (lll)P, (lll)L, (lll)bundleW[t], bundleColor[t]);
        }
    }

    // --- Compute baseline cost ---
    // Root tree at vertex 1, process in postorder.
    // For each vertex v, accumulate weight. If v != 1 and accW[v] > L, cut and finalize.
    lll baselineCost = 0;
    {
        // BFS to get parent and order
        vector<int> parent(n + 1, 0);
        vector<int> order;
        order.reserve(n);
        vector<bool> visited2(n + 1, false);
        queue<int> q;
        q.push(1); visited2[1] = true; parent[1] = 0;
        while (!q.empty()) {
            int v = q.front(); q.pop();
            order.push_back(v);
            for (int u : adj[v]) {
                if (!visited2[u]) {
                    visited2[u] = true;
                    parent[u] = v;
                    q.push(u);
                }
            }
        }

        // Process in reverse BFS order (postorder approximation)
        vector<lll> accW(n + 1, 0);
        vector<map<int,ll>*> accColor(n + 1, nullptr);
        for (int i = 1; i <= n; i++) accColor[i] = new map<int,ll>();

        auto finalizeBundle = [&](int v) {
            baselineCost += bundleCost((lll)S, (lll)P, (lll)L, accW[v], *accColor[v]);
            accW[v] = 0;
            accColor[v]->clear();
        };

        auto mergeInto = [&](int par2, int child) {
            accW[par2] += accW[child];
            if (accColor[child]->size() > accColor[par2]->size()) {
                swap(accColor[par2], accColor[child]);
            }
            for (auto& kv : *accColor[child]) {
                (*accColor[par2])[kv.first] += kv.second;
            }
            accColor[child]->clear();
            accW[child] = 0;
        };

        for (int i = (int)order.size() - 1; i >= 0; i--) {
            int v = order[i];
            // Initialize with just v
            accW[v] = w[v];
            (*accColor[v])[c[v]] = w[v];

            // Merge unfinalized children
            for (int u : adj[v]) {
                if (parent[u] == v) {
                    // u is a child of v
                    if (accW[u] > 0) {
                        mergeInto(v, u);
                    }
                    // if accW[u] == 0, it was already finalized
                }
            }

            // If v != 1 and accumulated weight > L, cut and finalize
            if (v != 1 && accW[v] > (lll)L) {
                finalizeBundle(v);
            }
        }

        // The remaining part containing vertex 1
        if (accW[1] > 0) {
            finalizeBundle(1);
        }

        for (int i = 1; i <= n; i++) {
            delete accColor[i];
            accColor[i] = nullptr;
        }
    }

    // --- Compute score ratio ---
    // Score = 100 * B / (B + O), ratio = B / (B + O) in [0,1]
    double ratio;
    double B = (double)baselineCost;
    double O = (double)participantCost;

    if (B + O <= 0.0) {
        ratio = 1.0;
    } else {
        ratio = B / (B + O);
        if (ratio < 0.0) ratio = 0.0;
        if (ratio > 1.0) ratio = 1.0;
    }

    quitp(ratio, "Ratio: %.9f | participant=%.0f baseline=%.0f bundles=%d",
          ratio, O, B, m);

    return 0;
}