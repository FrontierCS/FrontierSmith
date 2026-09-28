#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

static int N, M, Q, B;
static vector<pair<int,char>> adj[100005];
static int par[100005], dep[100005];
static char parEdge[100005];

void bfsTree(int root) {
    queue<int> bq;
    bq.push(root);
    par[root] = -1;
    dep[root] = 0;
    vector<bool> vis(N + 1, false);
    vis[root] = true;
    while (!bq.empty()) {
        int u = bq.front(); bq.pop();
        for (auto &e : adj[u]) {
            int v = e.first; char c = e.second;
            if (!vis[v]) {
                vis[v] = true;
                par[v] = u;
                parEdge[v] = c;
                dep[v] = dep[u] + 1;
                bq.push(v);
            }
        }
    }
}

void getPath(int u, int v, vector<int> &pathV, string &pathS) {
    vector<int> pu, pv;
    int a = u, b = v;
    while (a != b) {
        if (dep[a] >= dep[b]) { pu.push_back(a); a = par[a]; }
        else                  { pv.push_back(b); b = par[b]; }
    }
    pu.push_back(a); // LCA
    reverse(pv.begin(), pv.end());
    pathV = pu;
    for (int x : pv) pathV.push_back(x);

    pathS.clear();
    for (int i = 0; i + 1 < (int)pathV.size(); i++) {
        int cur = pathV[i], nxt = pathV[i+1];
        if (par[cur] == nxt) pathS += parEdge[cur];
        else                 pathS += parEdge[nxt];
    }
}

vector<int> kmpSearch(const string &txt, const string &pat) {
    vector<int> res;
    if (pat.empty() || pat.size() > txt.size()) return res;
    string s = pat + "#" + txt;
    int sz = (int)s.size();
    vector<int> fail(sz, 0);
    for (int i = 1; i < sz; i++) {
        int j = fail[i-1];
        while (j && s[i] != s[j]) j = fail[j-1];
        if (s[i] == s[j]) j++;
        fail[i] = j;
    }
    int plen = (int)pat.size();
    for (int i = plen + 1; i < sz; i++) {
        if (fail[i] == plen) {
            res.push_back(i - 2*plen); // 0-indexed start in txt
        }
    }
    return res;
}

struct Group {
    int u, v, k;
    long long w;
};

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // Read problem input
    N = inf.readInt();
    M = inf.readInt();
    Q = inf.readInt();
    B = inf.readInt();

    for (int i = 0; i < N - 1; i++) {
        int a = inf.readInt();
        int b = inf.readInt();
        string cs = inf.readToken();
        char c = cs[0];
        adj[a].push_back({b, c});
        adj[b].push_back({a, c});
    }

    bfsTree(1);

    vector<string> words(M + 1);
    for (int i = 1; i <= M; i++) {
        words[i] = inf.readToken();
    }

    vector<Group> groups(Q);
    for (int j = 0; j < Q; j++) {
        groups[j].u = inf.readInt();
        groups[j].v = inf.readInt();
        groups[j].k = inf.readInt();
        groups[j].w = inf.readLong();
    }

    // Read participant output
    int r = ouf.readInt();
    if (r < 0 || r > B)
        quitf(_wa, "r=%d is out of range [0,%d]", r, B);

    // wards[k] = set of vertices with a ward for word k
    vector<set<int>> wards(M + 1);
    set<pair<int,int>> wardSet;

    for (int i = 0; i < r; i++) {
        int x = ouf.readInt();
        int k = ouf.readInt();
        if (x < 1 || x > N)
            quitf(_wa, "Ward vertex x=%d out of range [1,%d]", x, N);
        if (k < 1 || k > M)
            quitf(_wa, "Ward word k=%d out of range [1,%d]", k, M);
        if (wardSet.count({x, k}))
            quitf(_wa, "Duplicate ward (%d,%d)", x, k);
        wardSet.insert({x, k});
        wards[k].insert(x);
    }

    // Check for extra output after all wards
    if (!ouf.seekEof())
        quitf(_wa, "Extra output found after %d wards", r);

    // Compute R0 and R
    long long R0 = 0, R = 0;

    for (int j = 0; j < Q; j++) {
        int u = groups[j].u, v = groups[j].v, k = groups[j].k;
        long long w = groups[j].w;
        const string &pat = words[k];

        vector<int> pathV;
        string pathS;
        getPath(u, v, pathV, pathS);

        vector<int> occs = kmpSearch(pathS, pat);
        long long occ_j = (long long)occs.size();
        R0 += occ_j * w;

        if (wards[k].empty()) {
            R += occ_j * w;
            continue;
        }

        long long blocked = 0;
        int L = (int)pat.size();
        for (int p : occs) {
            // occurrence of length L starts at edge-index p (0-based),
            // spans vertices at path indices p .. p+L (inclusive)
            bool disrupted = false;
            for (int vi = p; vi <= p + L && vi < (int)pathV.size(); vi++) {
                if (wards[k].count(pathV[vi])) { disrupted = true; break; }
            }
            if (disrupted) blocked++;
        }
        R += (occ_j - blocked) * w;
    }

    // Score: floor(1_000_000 * (R0 - R) / max(1, R0))
    long long denom = (R0 > 0LL) ? R0 : 1LL;
    long long gained = R0 - R;
    if (gained < 0LL) gained = 0LL;
    if (gained > denom) gained = denom;

    // Compute integer score as per statement: floor(1_000_000 * gained / denom)
    long long scoreInt = (long long)((double)gained / (double)denom * 1000000.0);
    // Clamp
    if (scoreInt < 0LL) scoreInt = 0LL;
    if (scoreInt > 1000000LL) scoreInt = 1000000LL;

    // ratio for quitp must be in [0,1] and match scoreInt/1000000
    double ratio = (double)scoreInt / 1000000.0;

    quitp(ratio,
          "Ratio: %.9f | R0=%lld R=%lld gained=%lld score=%lld",
          ratio, R0, R, gained, scoreInt);
}