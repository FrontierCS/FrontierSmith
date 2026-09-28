#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

typedef long long ll;
typedef pair<ll,int> pli;

static const int DQ[6] = {1, 0, -1, -1,  0,  1};
static const int DR[6] = {0, 1,  1,  0, -1, -1};
static const ll  LINF  = 4e18;

struct CellInfo {
    bool blocked  = false;
    int  terrain  = 1;
    ll   resource = 0;      // 0 means no resource (type != 2)
    bool hasRes   = false;
};

// ---- per-test globals ----
static int gR;
static map<pair<int,int>, int>      gCellId;
static vector<pair<int,int>>        gCells;
static map<pair<int,int>, CellInfo> gCI;

static bool onBoard(int q, int r){
    return abs(q) <= gR && abs(r) <= gR && abs(q+r) <= gR;
}

static void buildCells(){
    gCells.clear(); gCellId.clear();
    for(int q = -gR; q <= gR; q++)
        for(int r = -gR; r <= gR; r++)
            if(onBoard(q, r)){
                int id = (int)gCells.size();
                gCellId[{q, r}] = id;
                gCells.push_back({q, r});
            }
}

// Forward Dijkstra: cost c[d] * terrain(dest)
static vector<ll> dijkstraFwd(int src, const ll c[6]){
    int n = (int)gCells.size();
    vector<ll> dist(n, LINF);
    if(src < 0) return dist;
    {
        auto it = gCI.find(gCells[src]);
        if(it != gCI.end() && it->second.blocked) return dist;
    }
    priority_queue<pli, vector<pli>, greater<pli>> pq;
    dist[src] = 0; pq.push({0, src});
    while(!pq.empty()){
        auto [d, u] = pq.top(); pq.pop();
        if(d > dist[u]) continue;
        auto [q, r] = gCells[u];
        for(int i = 0; i < 6; i++){
            int nq = q + DQ[i], nr = r + DR[i];
            if(!onBoard(nq, nr)) continue;
            auto it = gCI.find({nq, nr});
            if(it != gCI.end() && it->second.blocked) continue;
            int ter = 1;
            if(it != gCI.end()) ter = it->second.terrain;
            int v = gCellId[{nq, nr}];
            ll nd = d + c[i] * (ll)ter;
            if(nd < dist[v]){ dist[v] = nd; pq.push({nd, v}); }
        }
    }
    return dist;
}

// Reverse Dijkstra: dist(v -> src) for all v
// Reverse edge: original edge (u)-dir->(v) with cost c[dir]*terrain(v)
//               becomes (v)->(u) with same cost c[dir]*terrain(v)
// So from node v (q,r) in reverse: for each direction d, neighbour (q-DQ[d], r-DR[d])
// with cost c[d] * terrain(v)  [terrain of the forward destination = v itself]
static vector<ll> dijkstraRev(int src, const ll c[6]){
    int n = (int)gCells.size();
    vector<ll> dist(n, LINF);
    if(src < 0) return dist;
    {
        auto it = gCI.find(gCells[src]);
        if(it != gCI.end() && it->second.blocked) return dist;
    }
    priority_queue<pli, vector<pli>, greater<pli>> pq;
    dist[src] = 0; pq.push({0, src});
    while(!pq.empty()){
        auto [d, v] = pq.top(); pq.pop();
        if(d > dist[v]) continue;
        auto [q, r] = gCells[v];
        // terrain of v
        int terV = 1;
        {
            auto it = gCI.find({q, r});
            if(it != gCI.end()) terV = it->second.terrain;
        }
        for(int i = 0; i < 6; i++){
            // reverse: original neighbour was (q,r) = dest, orig source was (q-DQ[i], r-DR[i])
            int nq = q - DQ[i], nr = r - DR[i];
            if(!onBoard(nq, nr)) continue;
            auto it = gCI.find({nq, nr});
            if(it != gCI.end() && it->second.blocked) continue;
            int u = gCellId[{nq, nr}];
            ll nd = d + c[i] * (ll)terV;
            if(nd < dist[u]){ dist[u] = nd; pq.push({nd, u}); }
        }
    }
    return dist;
}

// Compute the minimum-cost path from src to dst using forward Dijkstra from src
// Returns the distance, or LINF if unreachable
static ll pathCost(int src, int dst, const ll c[6]){
    if(src == dst) return 0LL;
    vector<ll> d = dijkstraFwd(src, c);
    return d[dst];
}

int main(int argc, char* argv[]){
    registerTestlibCmd(argc, argv);

    int T = inf.readInt();

    double totalScore = 0.0;
    int    totalCases = 0;

    for(int tc = 0; tc < T; tc++){
        // ---- Read test case input ----
        int R, K;
        ll  B;
        int S;
        R = inf.readInt();
        K = inf.readInt();
        B = inf.readLong();
        S = inf.readInt();

        ll c[6];
        for(int i = 0; i < 6; i++) c[i] = inf.readLong();

        gR = R;
        buildCells();
        gCI.clear();

        // resource cells: store (q,r) -> resource value
        vector<pair<int,int>> resCells;   // all resource cells
        map<pair<int,int>, ll> resValue;  // (q,r)->value

        for(int s = 0; s < S; s++){
            int q = inf.readInt(), r = inf.readInt();
            int type = inf.readInt();
            int w    = inf.readInt();
            ll  p    = inf.readLong();

            CellInfo ci;
            if(type == 0){
                ci.blocked  = true;
                ci.terrain  = 1;
                ci.resource = 0;
                ci.hasRes   = false;
            } else {
                ci.blocked  = false;
                ci.terrain  = w;
                ci.resource = (type == 2) ? p : 0;
                ci.hasRes   = (type == 2);
            }
            gCI[{q, r}] = ci;
            if(type == 2){
                resCells.push_back({q, r});
                resValue[{q, r}] = p;
            }
        }

        // ---- Compute baseline G ----
        // Forward Dijkstra from base (0,0)
        int baseId = gCellId[{0, 0}];
        vector<ll> distFwd = dijkstraFwd(baseId, c);
        // Reverse Dijkstra to base (0,0) = forward Dijkstra in reverse graph from base
        vector<ll> distRev = dijkstraRev(baseId, c);

        // U = sum of all resource values
        ll U = 0;
        for(auto& [pos, val] : resValue) U += val;

        // Baseline: for each resource cell, compute rt = dist(base->v) + dist(v->base)
        struct ResEntry {
            ll   rt;
            ll   val;
            int  q, r;
        };
        vector<ResEntry> baseline;
        for(auto& [pos, val] : resValue){
            int id = gCellId[pos];
            ll dfwd = distFwd[id];
            ll drev = distRev[id];
            if(dfwd >= LINF || drev >= LINF) continue;
            ll rt = dfwd + drev;
            if(rt > B) continue;
            baseline.push_back({rt, val, pos.first, pos.second});
        }
        // Sort: decreasing val/rt (exact: a*d > c*b), then decreasing val, then incr q, incr r
        sort(baseline.begin(), baseline.end(), [](const ResEntry& a, const ResEntry& b){
            // a.val/a.rt vs b.val/b.rt  => a.val * b.rt vs b.val * a.rt
            // use __int128 to avoid overflow
            __int128 lhs = (__int128)a.val * b.rt;
            __int128 rhs = (__int128)b.val * a.rt;
            if(lhs != rhs) return lhs > rhs;
            if(a.val != b.val) return a.val > b.val;
            if(a.q != b.q) return a.q < b.q;
            return a.r < b.r;
        });
        ll G = 0;
        int take = min((int)baseline.size(), K);
        for(int i = 0; i < take; i++) G += baseline[i].val;

        // ---- Read contestant output for this test case ----
        // If infeasible, O=0 (do not quitf _wa)
        ll   O         = 0;
        bool feasible  = true;
        string failMsg = "";

        int m = ouf.readInt(0, K);
        if(m < 0){
            feasible = false;
            failMsg  = "m out of range";
            m        = 0;
        }

        set<pair<int,int>> visited;

        for(int drone = 0; drone < m && feasible; drone++){
            int L = ouf.readInt();
            if(L < 0){
                feasible = false;
                failMsg  = "L < 0";
                break;
            }

            vector<pair<int,int>> stops;
            stops.reserve(L);
            for(int j = 0; j < L; j++){
                int q = ouf.readInt(), r = ouf.readInt();
                stops.push_back({q, r});
            }

            if(!feasible) break;

            // Validate: each stop must be a resource cell on the board
            for(auto& [q, r] : stops){
                if(!onBoard(q, r)){
                    feasible = false;
                    failMsg  = "stop not on board";
                    break;
                }
                auto it = gCI.find({q, r});
                bool isRes = false;
                if(it != gCI.end() && it->second.hasRes) isRes = true;
                if(!isRes){
                    // check resValue map as well (in case gCI doesn't have it => default terrain=1)
                    if(resValue.count({q, r})) isRes = true;
                }
                if(!isRes){
                    feasible = false;
                    failMsg  = "stop is not a resource cell";
                    break;
                }
                // Must not be blocked
                if(it != gCI.end() && it->second.blocked){
                    feasible = false;
                    failMsg  = "stop is blocked";
                    break;
                }
            }
            if(!feasible) break;

            if(L == 0) continue;  // trivial route, cost=0

            // Check route cost
            // base -> stops[0] -> stops[1] -> ... -> stops[L-1] -> base
            // Each leg: minimum-cost path
            ll totalCost = 0;

            // Leg: base -> first stop
            {
                int dst = gCellId[stops[0]];
                ll leg = distFwd[dst];
                if(leg >= LINF){
                    feasible = false;
                    failMsg  = "first stop unreachable from base";
                    break;
                }
                totalCost += leg;
                if(totalCost > B){
                    feasible = false;
                    failMsg  = "budget exceeded on first leg";
                    break;
                }
            }

            // Intermediate legs
            for(int j = 1; j < L && feasible; j++){
                int src = gCellId[stops[j-1]];
                int dst = gCellId[stops[j]];
                // Need Dijkstra from stops[j-1]
                vector<ll> dij = dijkstraFwd(src, c);
                ll leg = dij[dst];
                if(leg >= LINF){
                    feasible = false;
                    failMsg  = "leg unreachable";
                    break;
                }
                totalCost += leg;
                if(totalCost > B){
                    feasible = false;
                    failMsg  = "budget exceeded on intermediate leg";
                    break;
                }
            }
            if(!feasible) break;

            // Last stop -> base
            {
                int lastId = gCellId[stops[L-1]];
                ll back = distRev[lastId];
                if(back >= LINF){
                    feasible = false;
                    failMsg  = "return from last stop unreachable";
                    break;
                }
                totalCost += back;
                if(totalCost > B){
                    feasible = false;
                    failMsg  = "budget exceeded on return leg";
                    break;
                }
            }

            // Collect resources
            for(auto& pos : stops){
                if(visited.insert(pos).second){
                    O += resValue[pos];
                }
            }
        }

        if(!feasible){
            // Per statement: infeasible => objective for this test case = 0
            O = 0;
        }

        // ---- Compute per-test score ----
        double sTest;
        if(U == 0){
            sTest = 10000.0;
        } else if(G >= U){
            sTest = 10000.0 * (double)O / (double)U;
        } else if(O <= G){
            if(G == 0) sTest = 0.0;
            else       sTest = 4000.0 * (double)O / (double)G;
        } else {
            sTest = 4000.0 + 6000.0 * (double)(O - G) / (double)(U - G);
        }
        if(sTest < 0.0)     sTest = 0.0;
        if(sTest > 10000.0) sTest = 10000.0;

        totalScore += sTest;
        totalCases++;
    }

    // Verify no trailing garbage
    if(!ouf.seekEof()){
        quitf(_wa, "Extra data in output after all test cases");
    }

    double ratio = (totalCases > 0) ? (totalScore / totalCases / 10000.0) : 1.0;
    if(ratio < 0.0) ratio = 0.0;
    if(ratio > 1.0) ratio = 1.0;

    quitp(ratio, "Ratio: %.6f (mean score=%.2f/10000 over %d cases)",
          ratio, (totalCases > 0 ? totalScore / totalCases : 10000.0), totalCases);
}