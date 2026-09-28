#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

static vector<int> parseInts(const string& s) {
    vector<int> res;
    istringstream iss(s);
    int x;
    while (iss >> x) res.push_back(x);
    return res;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ── Read problem input from inf ──────────────────────────────────────────
    long long T = inf.readLong();
    long long C = inf.readLong();
    int N  = inf.readInt();
    int M  = inf.readInt();
    int K  = inf.readInt();

    vector<long long> toolWeight(K + 1, 0);
    for (int k = 1; k <= K; k++) toolWeight[k] = inf.readLong();

    struct Painting { int room; long long w, t, v; };
    vector<Painting> paintings; // 0-indexed; global id = index+1
    vector<int> roomReqMask(N + 1, 0);
    vector<vector<int>> roomPaintings(N + 1);

    for (int i = 1; i <= N; i++) {
        roomReqMask[i] = inf.readInt();
        int b = inf.readInt();
        for (int j = 0; j < b; j++) {
            long long w = inf.readLong(), t = inf.readLong(), v = inf.readLong();
            paintings.push_back({i, w, t, v});
            roomPaintings[i].push_back((int)paintings.size()); // 1-based global id
        }
    }
    int totalPaintings = (int)paintings.size();

    map<pair<int,int>, long long> corridorTime;
    vector<vector<pair<int,long long>>> adj(N + 1);
    for (int j = 0; j < M; j++) {
        int u = inf.readInt(), v = inf.readInt();
        long long d = inf.readLong();
        corridorTime[{u, v}] = d;
        corridorTime[{v, u}] = d;
        adj[u].push_back({v, d});
        adj[v].push_back({u, d});
    }

    // ── Compute baseline B via Dijkstra from room 1 ─────────────────────────
    vector<long long> dist(N + 1, LLONG_MAX);
    {
        priority_queue<pair<long long,int>, vector<pair<long long,int>>, greater<>> pq;
        dist[1] = 0;
        pq.push({0, 1});
        while (!pq.empty()) {
            auto [dd, u] = pq.top(); pq.pop();
            if (dd > dist[u]) continue;
            for (auto [v, w] : adj[u]) {
                if (dist[u] + w < dist[v]) {
                    dist[v] = dist[u] + w;
                    pq.push({dist[v], v});
                }
            }
        }
    }

    long long B = 0;
    for (int i = 1; i <= N; i++) {
        if (dist[i] == LLONG_MAX) continue;
        // tool weight for this room
        long long tw = 0;
        int req = roomReqMask[i];
        for (int k = 1; k <= K; k++) {
            if (req & (1 << (k-1))) tw += toolWeight[k];
        }
        if (tw > C) continue;
        long long remCap  = C - tw;
        long long remTime = T - 1 - 2LL * dist[i];
        if (remTime < 0) continue;

        // 0/1 knapsack on paintings in room i
        const auto& pids = roomPaintings[i];
        int nb = (int)pids.size();
        // dp[cap][time] = max value — but use 1D over capacity then time
        // Use capacity x time DP; sizes can be large so use map or bounded
        // remCap <= 5000, remTime <= 30000
        // nb <= 30, so simple DP
        int RC = (int)remCap;
        int RT = (int)min(remTime, (long long)30000);
        // dp[c][t] too large (5000*30000). Use painting-time knapsack with small steal times.
        // steal time <= 60 per painting, nb<=30, so total steal time <= 1800. Use that.
        int maxST = 0;
        for (int pid : pids) maxST += (int)paintings[pid-1].t;
        maxST = (int)min((long long)maxST, remTime);

        // dp[w][t] = max value; w up to RC (<=5000), t up to maxST (<=1800)
        vector<vector<long long>> dp(RC+1, vector<long long>(maxST+1, 0));
        for (int pid : pids) {
            const Painting& p = paintings[pid-1];
            int pw = (int)p.w, pt = (int)p.t;
            long long pv = p.v;
            if (pw > RC || pt > maxST) continue;
            for (int ww = RC; ww >= pw; ww--) {
                for (int tt = maxST; tt >= pt; tt--) {
                    dp[ww][tt] = max(dp[ww][tt], dp[ww-pw][tt-pt] + pv);
                }
            }
        }
        for (int ww = 0; ww <= RC; ww++)
            for (int tt = 0; tt <= maxST; tt++)
                B = max(B, dp[ww][tt]);
    }

    // ── Read participant output ──────────────────────────────────────────────
    string line1 = ouf.readLine();
    string line2 = ouf.readLine();
    string line3 = ouf.readLine();
    string line4 = ouf.readLine();

    // ── EOF check: no extra output allowed ──────────────────────────────────
    if (!ouf.seekEof())
        quitf(_wa, "Extra output after the 4 required lines");

    // ── Parse line 1: x L y ─────────────────────────────────────────────────
    {
        vector<int> hdr = parseInts(line1);
        if ((int)hdr.size() != 3)
            quitf(_wa, "Header line must have exactly 3 integers, got %d", (int)hdr.size());
        int x = hdr[0], L = hdr[1], y = hdr[2];

        if (x < 0 || x > K)
            quitf(_wa, "x=%d out of range [0,%d]", x, K);
        if (L < 1)
            quitf(_wa, "L=%d must be >= 1", L);
        if (y < 0)
            quitf(_wa, "y=%d must be >= 0", y);

        // ── Parse line 2: tool ids ───────────────────────────────────────────
        vector<int> toolList = parseInts(line2);
        if ((int)toolList.size() != x)
            quitf(_wa, "Expected %d tool ids, got %d", x, (int)toolList.size());

        int selMask = 0;
        for (int tk : toolList) {
            if (tk < 1 || tk > K)
                quitf(_wa, "Tool id %d out of range [1,%d]", tk, K);
            if (selMask & (1 << (tk-1)))
                quitf(_wa, "Duplicate tool id %d", tk);
            selMask |= (1 << (tk-1));
        }

        long long toolW = 0;
        for (int k = 1; k <= K; k++)
            if (selMask & (1 << (k-1))) toolW += toolWeight[k];

        // ── Parse line 3: walk ───────────────────────────────────────────────
        vector<int> walk = parseInts(line3);
        if ((int)walk.size() != L)
            quitf(_wa, "Expected %d rooms in walk, got %d", L, (int)walk.size());

        if (walk[0] != 1)
            quitf(_wa, "Walk must start at room 1, got %d", walk[0]);
        if (walk[L-1] != 1)
            quitf(_wa, "Walk must end at room 1, got %d", walk[L-1]);

        for (int r : walk)
            if (r < 1 || r > N)
                quitf(_wa, "Room %d out of range [1,%d]", r, N);

        long long travelTime = 0;
        for (int i = 0; i + 1 < L; i++) {
            int u = walk[i], v = walk[i+1];
            auto it = corridorTime.find({u, v});
            if (it == corridorTime.end())
                quitf(_wa, "No corridor between rooms %d and %d", u, v);
            travelTime += it->second;
        }

        vector<bool> roomVisited(N + 1, false);
        for (int r : walk) roomVisited[r] = true;

        // ── Parse line 4: painting ids ───────────────────────────────────────
        vector<int> stolenList = parseInts(line4);
        if ((int)stolenList.size() != y)
            quitf(_wa, "Expected %d painting ids, got %d", y, (int)stolenList.size());

        vector<bool> paintingTaken(totalPaintings + 1, false);
        long long stealTime = 0, paintWeight = 0, totalValue = 0;
        for (int pid : stolenList) {
            if (pid < 1 || pid > totalPaintings)
                quitf(_wa, "Painting id %d out of range [1,%d]", pid, totalPaintings);
            if (paintingTaken[pid])
                quitf(_wa, "Duplicate painting id %d", pid);
            paintingTaken[pid] = true;
            const Painting& p = paintings[pid - 1];
            if (!roomVisited[p.room])
                quitf(_wa, "Painting %d is in room %d which is not visited", pid, p.room);
            int req2 = roomReqMask[p.room];
            if ((selMask & req2) != req2)
                quitf(_wa, "Painting %d: required tools for room %d not all selected", pid, p.room);
            stealTime   += p.t;
            paintWeight += p.w;
            totalValue  += p.v;
        }

        // ── Check capacity ───────────────────────────────────────────────────
        long long usedCap = toolW + paintWeight;
        if (usedCap > C)
            quitf(_wa, "Capacity exceeded: %lld > %lld", usedCap, C);

        // ── Check time ───────────────────────────────────────────────────────
        long long usedTime = travelTime + stealTime;
        if (usedTime > T - 1)
            quitf(_wa, "Time exceeded: %lld > %lld (T-1=%lld)", usedTime, T - 1, T - 1);

        // ── Compute score ────────────────────────────────────────────────────
        // Statement: score = clamp(100*(V+1)/(B+1), 0, 300)
        // Config subtask score = 300, so ratio = score/300 in [0,1].
        // Baseline (V=B) -> raw_score=100 -> ratio=100/300=0.333... -> platform: 0.333*300=100. Correct.
        long long V = totalValue;
        double raw_score = 100.0 * (double)(V + 1) / (double)(B + 1);
        double clamped   = max(0.0, min(300.0, raw_score));
        double ratio     = clamped / 300.0; // [0, 1]; multiply by subtask score 300 to get final

        quitp(ratio, "V=%lld B=%lld raw_score=%.4f clamped=%.4f Ratio: %.6f",
              V, B, raw_score, clamped, ratio);
    }
}