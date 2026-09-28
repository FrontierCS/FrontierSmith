#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

typedef long long ll;
typedef pair<ll,int> pli;

const ll INF = 4e18;
const int MAXN = 505;

int n, m, q;
vector<pair<int,ll>> adj[MAXN];
ll dist_mat[MAXN][MAXN];
ll cap[MAXN];
map<pair<int,int>,ll> edgeLen;

void dijkstra(int src){
    for(int i=1;i<=n;i++) dist_mat[src][i]=INF;
    dist_mat[src][src]=0;
    priority_queue<pli,vector<pli>,greater<pli>> pq;
    pq.push({0,src});
    while(!pq.empty()){
        auto [d,u]=pq.top(); pq.pop();
        if(d>dist_mat[src][u]) continue;
        for(auto [v,w]:adj[u]){
            if(dist_mat[src][u]+w<dist_mat[src][v]){
                dist_mat[src][v]=dist_mat[src][u]+w;
                pq.push({dist_mat[src][v],v});
            }
        }
    }
}

// Lexicographically smallest shortest path from s to t
vector<int> canonicalPath(int s, int t){
    ll D=dist_mat[s][t];
    vector<int> path;
    path.push_back(s);
    set<int> visited;
    visited.insert(s);
    int cur=s;
    while(cur!=t){
        int best=-1;
        for(auto [v,w]:adj[cur]){
            if(visited.count(v)) continue;
            if(dist_mat[s][cur]+w+dist_mat[v][t]==D){
                if(best==-1||v<best) best=v;
            }
        }
        if(best==-1) break;
        path.push_back(best);
        visited.insert(best);
        cur=best;
    }
    return path;
}

struct Req {
    int s,t;
    ll d,p,delta;
};

int main(int argc, char* argv[]){
    registerTestlibCmd(argc,argv);

    n=inf.readInt();
    m=inf.readInt();
    q=inf.readInt();

    for(int i=0;i<m;i++){
        int u=inf.readInt();
        int v=inf.readInt();
        ll w=inf.readLong();
        adj[u].push_back({v,w});
        adj[v].push_back({u,w});
        edgeLen[{u,v}]=w;
        edgeLen[{v,u}]=w;
    }

    for(int i=1;i<=n;i++) cap[i]=inf.readLong();

    vector<Req> reqs(q);
    for(int i=0;i<q;i++){
        reqs[i].s=inf.readInt();
        reqs[i].t=inf.readInt();
        reqs[i].d=inf.readLong();
        reqs[i].p=inf.readLong();
        reqs[i].delta=inf.readLong();
    }

    // Run Dijkstra from every node
    for(int i=1;i<=n;i++) dijkstra(i);

    // Parse participant output: q lines
    vector<bool> accepted(q, false);
    vector<vector<int>> paths(q);

    for(int i=0;i<q;i++){
        // Read the first token on this line: either 0 or path length k
        int first = ouf.readInt();
        if(first == 0){
            // rejected
            accepted[i] = false;
        } else {
            int k = first;
            if(k < 2 || k > n){
                quitf(_wa, "Request %d: invalid path length %d", i+1, k);
            }
            vector<int> path(k);
            for(int j=0;j<k;j++){
                path[j] = ouf.readInt();
                if(path[j] < 1 || path[j] > n){
                    quitf(_wa, "Request %d: invalid router index %d", i+1, path[j]);
                }
            }
            accepted[i] = true;
            paths[i] = path;
        }
    }

    // --- EOF check: no extra output allowed ---
    if(!ouf.seekEof()){
        quitf(_wa, "Extra output after %d lines", q);
    }

    // Validate each accepted path
    vector<ll> load(n+1, 0);
    for(int i=0;i<q;i++){
        if(!accepted[i]) continue;
        auto& path = paths[i];
        int k = (int)path.size();

        // Check start and end
        if(path[0] != reqs[i].s){
            quitf(_wa, "Request %d: path starts at %d, expected %d", i+1, path[0], reqs[i].s);
        }
        if(path[k-1] != reqs[i].t){
            quitf(_wa, "Request %d: path ends at %d, expected %d", i+1, path[k-1], reqs[i].t);
        }

        // Check simplicity (no repeated router)
        set<int> seen;
        for(int j=0;j<k;j++){
            if(seen.count(path[j])){
                quitf(_wa, "Request %d: repeated router %d in path", i+1, path[j]);
            }
            seen.insert(path[j]);
        }

        // Check consecutive links exist and compute path length
        ll pathLen = 0;
        for(int j=0;j+1<k;j++){
            int u=path[j], v=path[j+1];
            auto it = edgeLen.find({u,v});
            if(it == edgeLen.end()){
                quitf(_wa, "Request %d: no link between %d and %d", i+1, u, v);
            }
            pathLen += it->second;
        }

        // Check latency constraint
        ll Di = dist_mat[reqs[i].s][reqs[i].t];
        if(Di == INF){
            quitf(_wa, "Request %d: no path exists from %d to %d", i+1, reqs[i].s, reqs[i].t);
        }
        if(pathLen > Di + reqs[i].delta){
            quitf(_wa, "Request %d: path length %lld exceeds D+delta=%lld+%lld=%lld",
                  i+1, pathLen, Di, reqs[i].delta, Di+reqs[i].delta);
        }

        // Accumulate internal router loads
        for(int j=1;j+1<k;j++){
            int v=path[j];
            load[v] += reqs[i].d;
        }
    }

    // Check capacity constraints
    for(int v=1;v<=n;v++){
        if(load[v] > cap[v]){
            quitf(_wa, "Router %d capacity exceeded: load=%lld cap=%lld", v, load[v], cap[v]);
        }
    }

    // Compute R
    ll R=0;
    for(int i=0;i<q;i++) if(accepted[i]) R+=reqs[i].p;

    // Compute U
    ll U=0;
    for(int i=0;i<q;i++) U+=reqs[i].p;

    if(U==0){
        quitp(1.0,"Ratio: 1.000000 (U=0, full score) R=%lld B=0 U=0",R);
    }

    // ---------- Compute baseline B ----------
    // 1. Canonical paths for each request
    vector<vector<int>> cpaths(q);
    for(int i=0;i<q;i++){
        cpaths[i]=canonicalPath(reqs[i].s,reqs[i].t);
    }

    // 2. Sort order: decreasing p/d, then increasing D, then increasing index
    vector<int> order(q);
    iota(order.begin(),order.end(),0);
    sort(order.begin(),order.end(),[&](int a,int b){
        __int128 lhs=(__int128)reqs[a].p*reqs[b].d;
        __int128 rhs=(__int128)reqs[b].p*reqs[a].d;
        if(lhs!=rhs) return lhs>rhs;
        ll Da=dist_mat[reqs[a].s][reqs[a].t];
        ll Db=dist_mat[reqs[b].s][reqs[b].t];
        if(Da!=Db) return Da<Db;
        return a<b;
    });

    vector<ll> residual(n+1);
    for(int v=1;v<=n;v++) residual[v]=cap[v];

    ll B=0;
    for(int idx:order){
        auto& path=cpaths[idx];
        int k=(int)path.size();
        bool ok=true;
        for(int j=1;j+1<k;j++){
            if(residual[path[j]]<reqs[idx].d){ ok=false; break; }
        }
        if(ok){
            B+=reqs[idx].p;
            for(int j=1;j+1<k;j++) residual[path[j]]-=reqs[idx].d;
        }
    }

    // ---------- Score ----------
    double score_ratio;
    if(B==0||B==U){
        score_ratio=(double)R/(double)U;
    } else {
        double c1=(double)R/(double)B;
        if(c1<0.0) c1=0.0; if(c1>1.0) c1=1.0;
        double c2=(double)(R-B)/(double)(U-B);
        if(c2<0.0) c2=0.0; if(c2>1.0) c2=1.0;
        score_ratio=(50.0*c1+50.0*c2)/100.0;
    }
    if(score_ratio<0.0) score_ratio=0.0;
    if(score_ratio>1.0) score_ratio=1.0;

    quitp(score_ratio,"Ratio: %f | R=%lld B=%lld U=%lld",score_ratio,R,B,U);
}