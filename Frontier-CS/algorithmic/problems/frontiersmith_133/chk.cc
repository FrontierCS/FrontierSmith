#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

static const int MAXN = 200005;
static const int LOG  = 18;

int  par_lca[MAXN][LOG];
long long depth_w[MAXN];
int  dep_h[MAXN];
int  n_nodes;
vector<pair<int,long long>> adj_g[MAXN];
bool vis_g[MAXN];

void bfs_lca(int root) {
    queue<int> q;
    q.push(root);
    vis_g[root] = true;
    dep_h[root] = 0;
    depth_w[root] = 0LL;
    par_lca[root][0] = root;
    while (!q.empty()) {
        int u = q.front(); q.pop();
        for (int i = 1; i < LOG; i++)
            par_lca[u][i] = par_lca[par_lca[u][i-1]][i-1];
        for (auto& e : adj_g[u]) {
            int v = e.first; long long w = e.second;
            if (!vis_g[v]) {
                vis_g[v] = true;
                dep_h[v] = dep_h[u] + 1;
                depth_w[v] = depth_w[u] + w;
                par_lca[v][0] = u;
                q.push(v);
            }
        }
    }
}

int lca_query(int u, int v) {
    if (dep_h[u] < dep_h[v]) swap(u, v);
    int diff = dep_h[u] - dep_h[v];
    for (int i = 0; i < LOG; i++)
        if ((diff >> i) & 1) u = par_lca[u][i];
    if (u == v) return u;
    for (int i = LOG-1; i >= 0; i--)
        if (par_lca[u][i] != par_lca[v][i]) {
            u = par_lca[u][i];
            v = par_lca[v][i];
        }
    return par_lca[u][0];
}

long long tree_dist(int u, int v) {
    int l = lca_query(u, v);
    return depth_w[u] + depth_w[v] - 2LL * depth_w[l];
}

bool on_path(int v, int a, int b) {
    return tree_dist(a, b) == tree_dist(a, v) + tree_dist(v, b);
}

struct Scenario {
    int a, b, t;
    long long w;
};

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // Read input
    int n = inf.readInt();
    int m = inf.readInt();
    int k = inf.readInt();
    int C = inf.readInt();
    n_nodes = n;

    for (int i = 0; i < n-1; i++) {
        int u = inf.readInt();
        int v = inf.readInt();
        long long d = (long long)inf.readInt();
        adj_g[u].push_back({v, d});
        adj_g[v].push_back({u, d});
    }

    bfs_lca(1);

    vector<Scenario> scens(m);
    long long U = 0;
    for (int i = 0; i < m; i++) {
        scens[i].a = inf.readInt();
        scens[i].b = inf.readInt();
        scens[i].t = inf.readInt();
        scens[i].w = (long long)inf.readInt();
        U += scens[i].w;
    }

    // Compute baseline
    // gain[x][c] = sum of w_i where t_i == c and x on path a_i..b_i
    // We use 1-indexed nodes, channels 1..C
    // For large n,m we compute per node per channel
    // gain_arr[x][c]
    vector<vector<long long>> gain_arr(n+1, vector<long long>(C+1, 0LL));
    for (int i = 0; i < m; i++) {
        int a = scens[i].a, b = scens[i].b, t = scens[i].t;
        long long w = scens[i].w;
        // For baseline computation we need gain per node
        // This is O(n*m) which is too slow for n=200000, m=300000
        // We must compute it efficiently
        // Actually the checker just needs to compute the baseline beacons
        // which requires gain(x,c) for all x,c
        // This is O(n*m) = 6*10^10 which is too slow
        // 
        // Instead: for each scenario i, the nodes on path a->b are those v
        // where dist(a,b) = dist(a,v)+dist(v,b)
        // We can't enumerate all nodes on path for large trees
        //
        // Alternative: use difference arrays on the tree (HLD or euler tour)
        // For each scenario i and channel t_i, add w_i to all nodes on path
        // This is standard "path update" on tree using LCA + difference array
        (void)a; (void)b; (void)t; (void)w;
    }

    // Path sum via difference array technique:
    // For each channel c, for each scenario i with t_i=c:
    //   diff[a_i] += w_i, diff[b_i] += w_i, diff[lca(a_i,b_i)] -= 2*w_i
    // Then subtree sum gives gain for each node
    // (standard trick: gain[v] = sum over ancestors contribution)
    // Actually the standard trick: 
    //   val[v] += w for path a->b:
    //   diff[a] += w, diff[b] += w, diff[lca] -= 2w
    //   then do DFS summing children: gain[v] = diff[v] + sum(gain[children])
    //   and gain[root] should be halved? No, let me think again.
    //
    // Standard: for path update (add w to all nodes on path a..b):
    //   diff[a]+=w, diff[b]+=w, diff[lca(a,b)]-=w, diff[par[lca(a,b)]]-=w
    // then gain[v] = sum of diff in subtree(v)
    // That gives the sum over edges. For nodes it's slightly different.
    //
    // For NODE values: a node v is on path(a,b) iff lca(a,v)==v or lca(b,v)==v or lca(a,b)==v
    // Standard node-path update:
    //   diff[a]+=w, diff[b]+=w, diff[lca(a,b)]-=w
    //   if lca(a,b) has parent p: diff[p]-=w (to cancel the lca being double-counted)
    //   then subtree sums give gain[v]
    // Wait, let me use the correct formulation:
    // diff[a]+=w, diff[b]+=w, diff[lca]−=w, diff[par[lca]]−=w
    // subtree_sum[v] = gain[v]
    // This counts each node on the path exactly once.

    // Build BFS order for subtree sums (reverse BFS = leaves first)
    vector<int> bfs_order;
    {
        queue<int> q2;
        q2.push(1);
        vector<bool> vs(n+1, false);
        vs[1] = true;
        while (!q2.empty()) {
            int u = q2.front(); q2.pop();
            bfs_order.push_back(u);
            for (auto& e : adj_g[u]) {
                if (!vs[e.first]) {
                    vs[e.first] = true;
                    q2.push(e.first);
                }
            }
        }
    }

    // Compute gain_arr for all channels
    for (int c = 1; c <= C; c++) {
        vector<long long> diff(n+1, 0LL);
        for (int i = 0; i < m; i++) {
            if (scens[i].t != c) continue;
            int a = scens[i].a, b = scens[i].b;
            long long w = scens[i].w;
            int lc = lca_query(a, b);
            diff[a] += w;
            diff[b] += w;
            diff[lc] -= w;
            int plc = par_lca[lc][0];
            if (plc != lc) diff[plc] -= w;
        }
        // Compute subtree sums in reverse BFS order
        for (int i = (int)bfs_order.size()-1; i >= 0; i--) {
            int v = bfs_order[i];
            for (auto& e : adj_g[v]) {
                int ch = e.first;
                if (dep_h[ch] == dep_h[v]+1) {
                    diff[v] += diff[ch];
                }
            }
            gain_arr[v][c] = diff[v];
        }
    }

    // Baseline computation
    auto computeBaseline = [&]() -> vector<pair<int,int>> {
        // For each node x: best(x) = smallest c maximizing gain(x,c)
        // score_x = gain(x, best(x))
        vector<pair<long long,pair<int,int>>> nodes; // (-score, node_index, channel)
        for (int x = 1; x <= n; x++) {
            long long best_gain = -1;
            int best_c = 1;
            for (int c = 1; c <= C; c++) {
                if (gain_arr[x][c] > best_gain || 
                    (gain_arr[x][c] == best_gain && c < best_c)) {
                    best_gain = gain_arr[x][c];
                    best_c = c;
                }
            }
            nodes.push_back({best_gain, {x, best_c}});
        }
        // Sort: larger score first, then smaller node index first
        sort(nodes.begin(), nodes.end(), [](const pair<long long,pair<int,int>>& a,
                                            const pair<long long,pair<int,int>>& b){
            if (a.first != b.first) return a.first > b.first;
            return a.second.first < b.second.first;
        });
        vector<pair<int,int>> beacons;
        for (int i = 0; i < k; i++) {
            beacons.push_back(nodes[i].second);
        }
        return beacons;
    };

    auto computeObj = [&](const vector<pair<int,int>>& beacons) -> long long {
        vector<vector<int>> by_chan(C+1);
        for (auto& p : beacons) by_chan[p.second].push_back(p.first);

        long long R = 0;
        for (int i = 0; i < m; i++) {
            int t = scens[i].t;
            for (int x : by_chan[t]) {
                if (on_path(x, scens[i].a, scens[i].b)) {
                    R += scens[i].w;
                    break;
                }
            }
        }

        long long S = 0;
        for (int c = 1; c <= C; c++) {
            int sz = (int)by_chan[c].size();
            for (int i = 0; i < sz; i++)
                for (int j = i+1; j < sz; j++)
                    S += tree_dist(by_chan[c][i], by_chan[c][j]);
        }
        return R - S;
    };

    vector<pair<int,int>> baseline_beacons = computeBaseline();
    long long Base = computeObj(baseline_beacons);

    // Read participant output
    vector<pair<int,int>> part_beacons(k);
    set<int> used_stations;
    for (int i = 0; i < k; i++) {
        if (ouf.seekEof()) {
            quitf(_wa, "Expected %d beacon lines, got only %d", k, i);
        }
        int x = ouf.readInt(1, n, "station index");
        int c = ouf.readInt(1, C, "channel");
        if (used_stations.count(x)) {
            quitf(_wa, "Station %d used more than once (beacon line %d)", x, i+1);
        }
        used_stations.insert(x);
        part_beacons[i] = {x, c};
    }
    // Check no trailing non-whitespace
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra output found after %d beacons", k);
    }

    long long Obj = computeObj(part_beacons);

    // Compute score ratio exactly per statement:
    // If U == Base:
    //   score = 100 if Obj >= U, else 0
    //   (this covers U==Base==0: ratio=1 iff Obj>=0, else 0)
    // Else:
    //   ratio = clamp((Obj - Base) / (U - Base), 0, 1)
    double ratio;
    if (U == Base) {
        // Full score iff Obj >= U, else exactly 0 (no fractional credit)
        ratio = (Obj >= U) ? 1.0 : 0.0;
    } else {
        double r = (double)(Obj - Base) / (double)(U - Base);
        ratio = max(0.0, min(1.0, r));
    }

    quitp(ratio, "Obj: %lld, Base: %lld, U: %lld, Ratio: %.6f", Obj, Base, U, ratio);
}