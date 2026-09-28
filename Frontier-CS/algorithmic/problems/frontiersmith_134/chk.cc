#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

typedef long long ll;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- read input ----
    int n, m;
    ll lambda;
    n = inf.readInt();
    m = inf.readInt();
    lambda = inf.readLong();

    vector<ll> tx(n+1), ty(n+1), tf(n+1);
    for(int i = 1; i <= n; i++){
        tx[i] = inf.readLong();
        ty[i] = inf.readLong();
        tf[i] = inf.readLong();
    }

    vector<ll> stx(m+1), sty(m+1), stc(m+1), stq(m+1);
    for(int j = 1; j <= m; j++){
        stx[j] = inf.readLong();
        sty[j] = inf.readLong();
        stc[j] = inf.readLong();
        stq[j] = inf.readLong();
    }

    int numRounds = n - 1;
    vector<ll> B(numRounds+1);
    for(int r = 1; r <= numRounds; r++) B[r] = inf.readLong();

    vector<vector<ll>> Riv(n+1, vector<ll>(n+1, 0));
    for(int i = 1; i <= n; i++)
        for(int j = 1; j <= n; j++)
            Riv[i][j] = inf.readLong();

    // availability[r][j] = true if stadium j available in round r (1-indexed)
    vector<vector<bool>> avail(numRounds+1, vector<bool>(m+1, false));
    for(int r = 1; r <= numRounds; r++){
        string s = inf.readToken();
        if((int)s.size() != m)
            quitf(_fail, "availability string length mismatch for round %d", r);
        for(int j = 1; j <= m; j++)
            avail[r][j] = (s[j-1] == '1');
    }

    // ---- Manhattan distance helpers ----
    auto mdistSS = [&](int a, int b) -> ll {
        return abs(stx[a]-stx[b]) + abs(sty[a]-sty[b]);
    };
    auto mdistTS = [&](int t, int s) -> ll {
        return abs(tx[t]-stx[s]) + abs(ty[t]-sty[s]);
    };

    // ---- demand helper ----
    auto demand = [&](int i, int j) -> ll {
        return tf[i] + tf[j] + Riv[i][j];
    };

    // ---- compute objective from schedule ----
    // sched entries: (r, i, j, s) all 1-indexed
    auto computeObj = [&](vector<tuple<int,int,int,int>>& sched) -> ll {
        ll totalRev = 0, totalTravel = 0;

        // teamStadium[team][round] = stadium_id (0 = not set)
        vector<vector<int>> teamStad(n+1, vector<int>(numRounds+1, 0));
        for(auto& [r,i,j,s] : sched){
            teamStad[i][r] = s;
            teamStad[j][r] = s;
            ll D = demand(i, j);
            ll att = min(stc[s], D);
            totalRev += B[r] * stq[s] * att;
        }

        // travel per team
        for(int t = 1; t <= n; t++){
            int prev = 0; // 0 = at home
            for(int r = 1; r <= numRounds; r++){
                int s = teamStad[t][r];
                if(s == 0) continue; // shouldn't happen for feasible
                if(prev == 0){
                    totalTravel += mdistTS(t, s);
                } else {
                    totalTravel += mdistSS(prev, s);
                }
                prev = s;
            }
            if(prev != 0){
                totalTravel += mdistTS(t, prev);
            }
        }
        return totalRev - lambda * totalTravel;
    };

    // ---- compute baseline schedule ----
    // Circle method pairings
    // Returns vector of (r, i, j, s) 1-indexed
    auto computeBaseline = [&]() -> ll {
        // circle method: fix team 1, rotate 2..n
        vector<int> circle(n);
        for(int i = 0; i < n; i++) circle[i] = i+1; // 1..n

        vector<tuple<int,int,int,int>> baseSched;

        for(int r = 1; r <= numRounds; r++){
            // pair first with last, second with second-last, ...
            vector<pair<int,int>> matches;
            for(int k = 0; k < n/2; k++){
                int a = circle[k];
                int b = circle[n-1-k];
                if(a > b) swap(a, b);
                matches.push_back({a, b});
            }

            // sort matches: decreasing D(i,j), then increasing i, then increasing j
            sort(matches.begin(), matches.end(), [&](const pair<int,int>& x, const pair<int,int>& y){
                ll dx = demand(x.first, x.second);
                ll dy = demand(y.first, y.second);
                if(dx != dy) return dx > dy;
                if(x.first != y.first) return x.first < y.first;
                return x.second < y.second;
            });

            // collect available stadiums for this round
            vector<int> stadList;
            for(int j = 1; j <= m; j++)
                if(avail[r][j]) stadList.push_back(j);

            // sort stadiums: decreasing q*c, then decreasing q, then decreasing c, then increasing s
            sort(stadList.begin(), stadList.end(), [&](int a, int b){
                ll qa = stq[a], qb = stq[b];
                ll ca = stc[a], cb = stc[b];
                ll pa = qa*ca, pb = qb*cb;
                if(pa != pb) return pa > pb;
                if(qa != qb) return qa > qb;
                if(ca != cb) return ca > cb;
                return a < b;
            });

            // assign k-th match to k-th stadium
            for(int k = 0; k < n/2; k++){
                baseSched.push_back({r, matches[k].first, matches[k].second, stadList[k]});
            }

            // rotate: keep circle[0] fixed, rotate circle[1..n-1] one step to the right
            // "one step to the right" means the last element goes to position 1
            int last = circle[n-1];
            for(int i = n-1; i > 1; i--) circle[i] = circle[i-1];
            circle[1] = last;
        }

        return computeObj(baseSched);
    };

    ll objBase = computeBaseline();

    // ---- compute LB ----
    // LB = -lambda * n^2 * Delta
    // Delta = max Manhattan distance between any two locations (teams + stadiums)
    // Collect all locations
    vector<pair<ll,ll>> locs;
    for(int i = 1; i <= n; i++) locs.push_back({tx[i], ty[i]});
    for(int j = 1; j <= m; j++) locs.push_back({stx[j], sty[j]});
    ll Delta = 0;
    for(int a = 0; a < (int)locs.size(); a++)
        for(int b = a+1; b < (int)locs.size(); b++){
            ll d = abs(locs[a].first - locs[b].first) + abs(locs[a].second - locs[b].second);
            Delta = max(Delta, d);
        }
    ll LB = -lambda * (ll)n * (ll)n * Delta;

    // ---- read and validate participant output ----
    int totalMatches = n*(n-1)/2;
    vector<tuple<int,int,int,int>> partSched;

    // track coverage
    vector<vector<bool>> pairSeen(n+1, vector<bool>(n+1, false));
    // teamInRound[round][team] = true if team already plays in this round
    vector<vector<bool>> teamInRound(numRounds+1, vector<bool>(n+1, false));
    // stadInRound[round][stad] = true if stadium already used in this round
    vector<vector<bool>> stadInRound(numRounds+1, vector<bool>(m+1, false));

    bool feasible = true;
    string failReason;

    for(int idx = 0; idx < totalMatches && feasible; idx++){
        if(ouf.eof()){
            feasible = false;
            failReason = "not enough lines in output";
            break;
        }

        int r = ouf.readInt();
        int i = ouf.readInt();
        int j = ouf.readInt();
        int s = ouf.readInt();

        // range checks
        if(r < 1 || r > numRounds){
            feasible = false;
            failReason = format("round %d out of range [1,%d]", r, numRounds);
            break;
        }
        if(i < 1 || i > n){
            feasible = false;
            failReason = format("team i=%d out of range [1,%d]", i, n);
            break;
        }
        if(j < 1 || j > n){
            feasible = false;
            failReason = format("team j=%d out of range [1,%d]", j, n);
            break;
        }
        if(s < 1 || s > m){
            feasible = false;
            failReason = format("stadium %d out of range [1,%d]", s, m);
            break;
        }
        if(i >= j){
            feasible = false;
            failReason = format("require i < j, got i=%d j=%d", i, j);
            break;
        }
        // pair duplicate
        if(pairSeen[i][j]){
            feasible = false;
            failReason = format("pair (%d,%d) scheduled more than once", i, j);
            break;
        }
        // team conflict in round
        if(teamInRound[r][i]){
            feasible = false;
            failReason = format("team %d plays more than once in round %d", i, r);
            break;
        }
        if(teamInRound[r][j]){
            feasible = false;
            failReason = format("team %d plays more than once in round %d", j, r);
            break;
        }
        // stadium conflict in round
        if(stadInRound[r][s]){
            feasible = false;
            failReason = format("stadium %d used twice in round %d", s, r);
            break;
        }
        // availability
        if(!avail[r][s]){
            feasible = false;
            failReason = format("stadium %d not available in round %d", s, r);
            break;
        }

        pairSeen[i][j] = true;
        teamInRound[r][i] = true;
        teamInRound[r][j] = true;
        stadInRound[r][s] = true;
        partSched.push_back({r, i, j, s});
    }

    if(feasible){
        // check all pairs covered
        for(int i = 1; i <= n && feasible; i++)
            for(int j = i+1; j <= n && feasible; j++)
                if(!pairSeen[i][j]){
                    feasible = false;
                    failReason = format("pair (%d,%d) never scheduled", i, j);
                }
    }

    if(feasible){
        // check no trailing garbage
        ouf.skipBlanks();
        if(!ouf.eof()){
            feasible = false;
            failReason = "extra content after expected output";
        }
    }

    if(!feasible){
        // Score 0: return Ratio 0.0
        quitf(_wa, "Infeasible: %s. Ratio: 0.0", failReason.c_str());
    }

    // ---- compute participant objective ----
    ll objYou = computeObj(partSched);

    // ---- score ----
    // Statement: Score_test = 100 * clamp(0, 2, (OBJ_you - LB) / (OBJ_base - LB))
    // quitp expects a value in [0, 1]; subtask is worth 200 points.
    // So: scoreRatio = clamp(0, 1, rawRatio / 2)
    //   where rawRatio = (OBJ_you - LB) / (OBJ_base - LB)
    // Then judge score = scoreRatio * 200, which gives 100 at baseline and up to 200.
    double denom = (double)(objBase - LB);
    double num   = (double)(objYou  - LB);
    double rawRatio;
    if(denom <= 0.0){
        // degenerate: all feasible schedules are equally good
        rawRatio = 1.0;
    } else {
        rawRatio = num / denom;
    }
    // clamp rawRatio to [0, 2], then map to [0, 1] for quitp
    double scoreRatio = max(0.0, min(1.0, rawRatio / 2.0));

    // The "Ratio:" tag must contain the value passed to quitp (in [0,1])
    quitp(scoreRatio,
          "Feasible. OBJ_you=%lld OBJ_base=%lld LB=%lld raw_ratio=%.6f Ratio: %.6f",
          (long long)objYou, (long long)objBase, (long long)LB, rawRatio, scoreRatio);

    return 0;
}