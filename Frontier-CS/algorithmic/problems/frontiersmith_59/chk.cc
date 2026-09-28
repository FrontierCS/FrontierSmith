#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

static const int MAXN  = 100005;
static const int MAXLOG = 17;

int  n, q;
long long a[MAXN], c_cost[MAXN], b[MAXN];
long long prefA[MAXN], prefB[MAXN];

vector<int> adj[MAXN];
int  par[MAXN][MAXLOG];
int  dep[MAXN];

void buildTree(int root) {
    // iterative BFS to set par[][0] and dep[]
    queue<int> bfsQ;
    bfsQ.push(root);
    dep[root] = 0;
    par[root][0] = 0;
    vector<bool> vis(n + 1, false);
    vis[root] = true;
    while (!bfsQ.empty()) {
        int u = bfsQ.front(); bfsQ.pop();
        for (int v : adj[u]) {
            if (!vis[v]) {
                vis[v] = true;
                par[v][0] = u;
                dep[v] = dep[u] + 1;
                bfsQ.push(v);
            }
        }
    }
    // binary lifting
    for (int j = 1; j < MAXLOG; j++)
        for (int v = 1; v <= n; v++)
            par[v][j] = par[par[v][j-1]][j-1];
}

int lcaQuery(int u, int v) {
    if (dep[u] < dep[v]) swap(u, v);
    int diff = dep[u] - dep[v];
    for (int j = 0; j < MAXLOG; j++)
        if ((diff >> j) & 1) u = par[u][j];
    if (u == v) return u;
    for (int j = MAXLOG-1; j >= 0; j--)
        if (par[u][j] != par[v][j]) { u = par[u][j]; v = par[v][j]; }
    return par[u][0];
}

void computePrefXor(int root, long long* keys, long long* pref) {
    // iterative BFS-based prefix XOR from root
    queue<int> bfsQ;
    bfsQ.push(root);
    pref[root] = keys[root];
    vector<bool> vis(n + 1, false);
    vis[root] = true;
    while (!bfsQ.empty()) {
        int u = bfsQ.front(); bfsQ.pop();
        for (int v : adj[u]) {
            if (!vis[v]) {
                vis[v] = true;
                pref[v] = pref[u] ^ keys[v];
                bfsQ.push(v);
            }
        }
    }
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    n = inf.readInt();
    q = inf.readInt();

    for (int i = 1; i <= n; i++) a[i]      = inf.readLong();
    for (int i = 1; i <= n; i++) c_cost[i] = inf.readLong();

    for (int i = 0; i < n - 1; i++) {
        int x = inf.readInt(), y = inf.readInt();
        adj[x].push_back(y);
        adj[y].push_back(x);
    }

    vector<int>       qu(q), qv(q);
    vector<long long> qt(q), qw(q);
    for (int i = 0; i < q; i++) {
        qu[i] = inf.readInt();
        qv[i] = inf.readInt();
        qt[i] = inf.readLong();
        qw[i] = inf.readLong();
    }

    // Build LCA structures and prefix XOR with original keys
    buildTree(1);
    computePrefXor(1, a, prefA);

    // Compute baseline loss (b = a, no reprogramming)
    long long L_base = 0;
    for (int i = 0; i < q; i++) {
        int l = lcaQuery(qu[i], qv[i]);
        long long xv = prefA[qu[i]] ^ prefA[qv[i]] ^ a[l];
        if (xv != qt[i]) L_base += qw[i];
    }

    // Read and validate participant output
    for (int i = 1; i <= n; i++) {
        b[i] = ouf.readLong();
        if (b[i] < 0 || b[i] >= (1LL << 30)) {
            quitf(_wa, "b[%d] = %lld is out of range [0, 2^30)", i, b[i]);
        }
    }
    // Ensure no trailing tokens in participant output
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra content found after the %d keys", n);
    }

    // Prefix XOR with participant keys
    computePrefXor(1, b, prefB);

    // Edit cost
    long long editCost = 0;
    for (int i = 1; i <= n; i++) {
        if (b[i] != a[i]) editCost += c_cost[i];
    }

    // Unsatisfied penalty
    long long penalty = 0;
    for (int i = 0; i < q; i++) {
        int l = lcaQuery(qu[i], qv[i]);
        long long xv = prefB[qu[i]] ^ prefB[qv[i]] ^ b[l];
        if (xv != qt[i]) penalty += qw[i];
    }

    long long L_you = editCost + penalty;

    // score_int = max(0, floor(1,000,000 * (L_base - L_you) / (L_base + 1)))
    // This matches the problem statement exactly.
    long long score_int = 0LL;
    {
        long long num = L_base - L_you;
        long long den = L_base + 1LL;
        if (num > 0 && den > 0) {
            // Use __int128 to avoid overflow in 1000000 * num
            score_int = (long long)((__int128)1000000LL * (long long)num / (long long)den);
        }
    }
    if (score_int < 0)       score_int = 0;
    if (score_int > 1000000) score_int = 1000000;

    // quitp expects a value in [0, 1]; score_int is in [0, 1000000].
    // Normalize: ratio = score_int / 1000000.0
    // The judge parse "Ratio: <value>" from the message.
    double ratio = (double)score_int / 1000000.0;

    quitp(ratio,
          "L_base=%lld L_you=%lld EditCost=%lld Penalty=%lld "
          "Score=%lld Ratio: %.6f",
          L_base, L_you, editCost, penalty, score_int, ratio);

    return 0;
}