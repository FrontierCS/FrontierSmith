#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

// DSU for Kruskal
struct DSU {
    vector<int> p, rnk;
    DSU(int n): p(n), rnk(n,0) { iota(p.begin(),p.end(),0); }
    int find(int x){ return p[x]==x?x:p[x]=find(p[x]); }
    bool unite(int a, int b){
        a=find(a); b=find(b);
        if(a==b) return false;
        if(rnk[a]<rnk[b]) swap(a,b);
        p[b]=a;
        if(rnk[a]==rnk[b]) rnk[a]++;
        return true;
    }
};

// Find path from src to dst in a rooted tree (stored as adjacency list with parent info)
// Returns sequence of nodes: src, ..., dst
vector<int> treePath(int src, int dst, vector<int>& par, int root){
    // get ancestors of src and dst up to root
    auto ancestors = [&](int v) -> vector<int> {
        vector<int> anc;
        while(v != -1){ anc.push_back(v); v = par[v]; }
        return anc;
    };
    vector<int> as = ancestors(src), ad = ancestors(dst);
    // find LCA
    set<int> setS(as.begin(), as.end());
    int lca = -1;
    for(int x : ad) if(setS.count(x)){ lca = x; break; }
    // path: src -> lca -> dst
    vector<int> path;
    for(int v = src; v != lca; v = par[v]) path.push_back(v);
    path.push_back(lca);
    vector<int> tail;
    for(int v = dst; v != lca; v = par[v]) tail.push_back(v);
    reverse(tail.begin(), tail.end());
    for(int x : tail) path.push_back(x);
    return path;
}

// Compute baseline B for one test case
long long computeBaseline(int n, int m,
                           vector<int>& perm,       // 0-indexed: perm[i] = crate at station i
                           vector<tuple<int,int,long long>>& edges) // 0-indexed u,v,w
{
    // 1. Kruskal MST, sort by (weight, original_index)
    vector<int> idx(m);
    iota(idx.begin(), idx.end(), 0);
    sort(idx.begin(), idx.end(), [&](int a, int b){
        auto [ua,va,wa] = edges[a];
        auto [ub,vb,wb] = edges[b];
        if(wa != wb) return wa < wb;
        return a < b;
    });
    DSU dsu(n);
    // adj list for MST: adj[u] = list of (v, weight)
    vector<vector<pair<int,long long>>> adj(n);
    for(int ei : idx){
        auto [u,v,w] = edges[ei];
        if(dsu.unite(u,v)){
            adj[u].push_back({v,w});
            adj[v].push_back({u,w});
        }
    }
    // 2. Root tree at vertex 0 (station 1 in 0-indexed)
    vector<int> par(n,-1);
    vector<long long> parW(n,0);
    vector<vector<int>> children(n);
    // BFS/DFS to set parents
    {
        vector<bool> vis(n,false);
        stack<int> st;
        st.push(0);
        vis[0]=true;
        // DFS to set parent
        // Use iterative DFS
        while(!st.empty()){
            int u = st.top(); st.pop();
            // sort children by vertex number (ascending)
            // we need to process adj[u] but we set children later
            for(auto [v,w]: adj[u]){
                if(!vis[v]){
                    vis[v]=true;
                    par[v]=u;
                    parW[v]=w;
                    st.push(v);
                }
            }
        }
    }
    for(int v=0;v<n;v++) if(par[v]!=-1) children[par[v]].push_back(v);
    for(int v=0;v<n;v++) sort(children[v].begin(),children[v].end());

    // 3. DFS from 0 (vertex 1), children in increasing order, get postorder
    vector<int> postorder;
    {
        // iterative DFS with postorder
        stack<pair<int,int>> st; // (node, child_index)
        st.push({0,0});
        while(!st.empty()){
            auto& [u,ci] = st.top();
            if(ci < (int)children[u].size()){
                int c = children[u][ci++];
                st.push({c,0});
            } else {
                postorder.push_back(u);
                st.pop();
            }
        }
    }

    // 4. Process postorder excluding root (which is last = 0)
    // Track: pos[crate] = current station (0-indexed)
    //        at[station] = current crate (0-indexed)
    vector<int> pos(n), at(n);
    for(int i=0;i<n;i++){
        at[i] = perm[i]; // perm[i] is 0-indexed crate at station i
        pos[perm[i]] = i;
    }

    long long B = 0;
    for(int v : postorder){
        if(v == 0) continue; // skip root
        // find current location of crate v
        int cur = pos[v];
        if(cur == v) continue; // already home
        // find path from cur to v in the tree
        vector<int> path = treePath(cur, v, par, 0);
        // swap along path
        for(int i=0;i+1<(int)path.size();i++){
            int a = path[i], b = path[i+1];
            // find edge weight between a and b in MST
            long long w = 0;
            for(auto [nb,ww]: adj[a]) if(nb==b){ w=ww; break; }
            B += w;
            // swap crates at a and b
            int ca = at[a], cb = at[b];
            at[a]=cb; at[b]=ca;
            pos[ca]=b; pos[cb]=a;
        }
    }
    return B;
}

int main(int argc, char* argv[]){
    registerTestlibCmd(argc, argv);

    int t = inf.readInt();

    double totalScore = 0.0;
    double maxScore = 0.0;

    for(int tc=0; tc<t; tc++){
        int n = inf.readInt();
        int m = inf.readInt();

        vector<int> perm(n);
        for(int i=0;i<n;i++){
            perm[i] = inf.readInt()-1; // 0-indexed crate
        }

        // edges: 0-indexed
        vector<tuple<int,int,long long>> edges(m);
        for(int j=0;j<m;j++){
            int u = inf.readInt()-1;
            int v = inf.readInt()-1;
            long long w = inf.readLong();
            edges[j] = {u,v,w};
        }

        // Read participant output for this test case
        int k = ouf.readInt(0, n*(n-1)/2,
            ("Test "+to_string(tc+1)+": k out of range").c_str());

        vector<int> ops(k);
        for(int i=0;i<k;i++){
            ops[i] = ouf.readInt(1, m,
                ("Test "+to_string(tc+1)+": edge index out of range").c_str()) - 1;
        }

        // Simulate participant's swaps
        vector<int> at(n), pos(n);
        for(int i=0;i<n;i++){
            at[i] = perm[i];
            pos[perm[i]] = i;
        }
        long long C = 0;
        for(int i=0;i<k;i++){
            int ei = ops[i];
            auto [u,v,w] = edges[ei];
            C += w;
            int cu = at[u], cv = at[v];
            at[u]=cv; at[v]=cu;
            pos[cu]=v; pos[cv]=u;
        }

        // Check feasibility: station i must contain crate i
        bool feasible = true;
        for(int i=0;i<n;i++){
            if(at[i]!=i){ feasible=false; break; }
        }

        // Compute baseline B
        long long B = computeBaseline(n, m, perm, edges);

        maxScore += 2000000.0;

        if(!feasible){
            // score 0 for this test
            // do not quit, continue
        } else {
            double score_tc;
            if(B == 0){
                score_tc = (C == 0) ? 1000000.0 : 0.0;
            } else {
                double ratio = (double)B / (double)C;
                double capped = min(2.0, ratio);
                score_tc = floor(1000000.0 * capped);
            }
            totalScore += score_tc;
        }
    }

    // Check end of participant output
    if(!ouf.seekEof()){
        quitf(_wa, "Participant output has extra tokens after all test cases.");
    }

    double scoreRatio = (maxScore > 0.0) ? (totalScore / maxScore) : 1.0;
    if(scoreRatio < 0.0) scoreRatio = 0.0;
    if(scoreRatio > 1.0) scoreRatio = 1.0;

    quitp(scoreRatio, "Ratio: %.6f (totalScore=%.0f, maxPossible=%.0f)",
          scoreRatio, totalScore, maxScore);

    return 0;
}