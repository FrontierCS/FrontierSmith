#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

static const long long INF64 = (long long)2e18;

vector<vector<long long>> computeAPSP(int V, vector<tuple<int,int,long long>>& edges) {
    vector<vector<long long>> d(V+1, vector<long long>(V+1, INF64));
    for (int i = 1; i <= V; i++) d[i][i] = 0;
    for (auto& e : edges) {
        int u = get<0>(e), v = get<1>(e);
        long long w = get<2>(e);
        d[u][v] = min(d[u][v], w);
        d[v][u] = min(d[v][u], w);
    }
    for (int k = 1; k <= V; k++)
        for (int i = 1; i <= V; i++) if (d[i][k] < INF64)
            for (int j = 1; j <= V; j++) if (d[k][j] < INF64)
                d[i][j] = min(d[i][j], d[i][k] + d[k][j]);
    return d;
}

int circDist(int a, int b) {
    int x = abs(a - b);
    return min(x, 100 - x);
}

long long computeBaseline(
    int V, int N, int K,
    vector<tuple<int,int,long long>>& edges,
    vector<int>& startRoom, vector<int>& col, vector<long long>& vol,
    vector<int>& labRoom, vector<int>& targetCol, vector<long long>& pen,
    vector<vector<long long>>& D
) {
    long long B = 0;

    vector<bool> assigned(N+1, false);
    vector<vector<int>> grp(K);

    // Reserve one ingredient per lab: minimize dist, break ties by smaller i
    for (int j = 0; j < K; j++) {
        int bestI = -1;
        long long bestD = INF64;
        for (int i = 1; i <= N; i++) {
            if (assigned[i]) continue;
            long long dd = D[startRoom[i]][labRoom[j]];
            if (bestI == -1 || dd < bestD || (dd == bestD && i < bestI)) {
                bestD = dd;
                bestI = i;
            }
        }
        assigned[bestI] = true;
        grp[j].push_back(bestI);
    }

    // Assign remaining ingredients to nearest lab, tie by smaller j index
    for (int i = 1; i <= N; i++) {
        if (assigned[i]) continue;
        int bestJ = -1;
        long long bestD = INF64;
        for (int j = 0; j < K; j++) {
            long long dd = D[startRoom[i]][labRoom[j]];
            if (bestJ == -1 || dd < bestD || (dd == bestD && j < bestJ)) {
                bestD = dd;
                bestJ = j;
            }
        }
        grp[bestJ].push_back(i);
        assigned[i] = true;
    }

    // Move every ingredient directly to its assigned lab
    for (int j = 0; j < K; j++) {
        for (int i : grp[j]) {
            long long dd = D[startRoom[i]][labRoom[j]];
            B += dd * vol[i] * vol[i];
        }
    }

    // At each lab, sort IDs increasingly and mix in order
    for (int j = 0; j < K; j++) {
        sort(grp[j].begin(), grp[j].end());
        int curColor = col[grp[j][0]];
        long long curVol = vol[grp[j][0]];
        for (int k = 1; k < (int)grp[j].size(); k++) {
            int bc = col[grp[j][k]];
            long long bv = vol[grp[j][k]];
            // smoke
            B += curVol * bv * (long long)curColor * (long long)bc;
            curColor = (curColor + bc) % 100;
            curVol = curVol + bv;
        }
        // lab penalty for this lab
        long long delta = circDist(curColor, targetCol[j]);
        B += pen[j] * delta * curVol;
    }

    return B;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    int T = inf.readInt();

    double totalScore = 0.0;

    for (int tc = 0; tc < T; tc++) {
        int V = inf.readInt();
        int E = inf.readInt();
        int N = inf.readInt();
        int K = inf.readInt();

        vector<tuple<int,int,long long>> edges(E);
        for (int i = 0; i < E; i++) {
            int u = inf.readInt();
            int v = inf.readInt();
            long long d = inf.readLong();
            edges[i] = {u, v, d};
        }

        vector<int> startRoom(N+1), col(N+1);
        vector<long long> vol(N+1);
        for (int i = 1; i <= N; i++) {
            startRoom[i] = inf.readInt();
            col[i] = inf.readInt();
            vol[i] = inf.readLong();
        }

        vector<int> labRoom(K), targetCol(K);
        vector<long long> pen(K);
        for (int j = 0; j < K; j++) {
            labRoom[j] = inf.readInt();
            targetCol[j] = inf.readInt();
            pen[j] = inf.readLong();
        }

        auto D = computeAPSP(V, edges);
        long long B = computeBaseline(V, N, K, edges, startRoom, col, vol, labRoom, targetCol, pen, D);

        // Now read participant output for this test case
        // Read Q
        int Q = ouf.readInt();

        if (Q < 0 || Q > 300000) {
            // Invalid Q - score 0, but we can't reliably parse further
            // We must quit since we can't skip this test case's output
            quitf(_wa, "tc%d: Q=%d is out of range [0,300000]", tc+1, Q);
        }

        // Simulate
        struct Batch {
            int room;
            int color;
            long long volume;
            bool alive;
        };

        // Initial batches: IDs 1..N
        map<int, Batch> batches;
        for (int i = 1; i <= N; i++) {
            batches[i] = {startRoom[i], col[i], vol[i], true};
        }
        int nextId = N + 1;
        int mixCount = 0;

        long long C = 0;
        bool valid = true;
        string invalidReason = "";

        set<int> labRoomSet(labRoom.begin(), labRoom.end());

        for (int q = 0; q < Q; q++) {
            string opType = ouf.readToken();

            if (opType == "MOVE") {
                int x = ouf.readInt();
                int v = ouf.readInt();

                if (!valid) continue; // still consume tokens

                if (batches.find(x) == batches.end() || !batches[x].alive) {
                    valid = false;
                    invalidReason = "tc" + to_string(tc+1) + ": MOVE references non-existent or dead batch " + to_string(x);
                    continue;
                }
                if (v < 1 || v > V) {
                    valid = false;
                    invalidReason = "tc" + to_string(tc+1) + ": MOVE destination room " + to_string(v) + " out of range";
                    continue;
                }
                Batch& b = batches[x];
                int u = b.room;
                long long dd = D[u][v];
                if (dd == INF64) {
                    valid = false;
                    invalidReason = "tc" + to_string(tc+1) + ": MOVE from " + to_string(u) + " to " + to_string(v) + " unreachable";
                    continue;
                }
                C += dd * b.volume * b.volume;
                b.room = v;
            } else if (opType == "MIX") {
                int x = ouf.readInt();
                int y = ouf.readInt();

                if (!valid) {
                    mixCount++;
                    nextId++;
                    continue;
                }

                if (x == y) {
                    valid = false;
                    invalidReason = "tc" + to_string(tc+1) + ": MIX of batch with itself (" + to_string(x) + ")";
                    mixCount++;
                    nextId++;
                    continue;
                }
                if (batches.find(x) == batches.end() || !batches[x].alive) {
                    valid = false;
                    invalidReason = "tc" + to_string(tc+1) + ": MIX references non-existent or dead batch " + to_string(x);
                    mixCount++;
                    nextId++;
                    continue;
                }
                if (batches.find(y) == batches.end() || !batches[y].alive) {
                    valid = false;
                    invalidReason = "tc" + to_string(tc+1) + ": MIX references non-existent or dead batch " + to_string(y);
                    mixCount++;
                    nextId++;
                    continue;
                }
                Batch& bx = batches[x];
                Batch& by = batches[y];
                if (bx.room != by.room) {
                    valid = false;
                    invalidReason = "tc" + to_string(tc+1) + ": MIX batches " + to_string(x) + " and " + to_string(y) + " in different rooms";
                    mixCount++;
                    nextId++;
                    continue;
                }
                // smoke cost
                C += bx.volume * by.volume * (long long)bx.color * (long long)by.color;
                int newColor = (bx.color + by.color) % 100;
                long long newVol = bx.volume + by.volume;
                int newRoom = bx.room;
                bx.alive = false;
                by.alive = false;
                int newId = N + (++mixCount);
                batches[newId] = {newRoom, newColor, newVol, true};
                nextId = N + mixCount + 1;
            } else {
                // Unknown op type - invalid, but we can't reliably skip remaining tokens
                quitf(_wa, "tc%d: unknown operation type '%s' at op %d", tc+1, opType.c_str(), q+1);
            }
        }

        if (valid) {
            // Check final state
            // Count alive batches and verify lab conditions
            map<int, int> roomToBatch; // room -> batch ID
            bool dupRoom = false;
            int aliveCount = 0;
            for (auto& kv : batches) {
                if (!kv.second.alive) continue;
                aliveCount++;
                int r = kv.second.room;
                if (roomToBatch.count(r)) {
                    valid = false;
                    invalidReason = "tc" + to_string(tc+1) + ": multiple alive batches in room " + to_string(r);
                    break;
                }
                roomToBatch[r] = kv.first;
            }

            if (valid && aliveCount != K) {
                valid = false;
                invalidReason = "tc" + to_string(tc+1) + ": expected " + to_string(K) + " alive batches, found " + to_string(aliveCount);
            }

            if (valid) {
                // Check every alive batch is in a lab room
                for (auto& kv : batches) {
                    if (!kv.second.alive) continue;
                    if (!labRoomSet.count(kv.second.room)) {
                        valid = false;
                        invalidReason = "tc" + to_string(tc+1) + ": alive batch " + to_string(kv.first) + " in non-lab room " + to_string(kv.second.room);
                        break;
                    }
                }
            }

            if (valid) {
                // Check every lab has exactly one batch
                for (int j = 0; j < K; j++) {
                    if (!roomToBatch.count(labRoom[j])) {
                        valid = false;
                        invalidReason = "tc" + to_string(tc+1) + ": no batch at lab room " + to_string(labRoom[j]);
                        break;
                    }
                }
            }

            if (valid) {
                // Compute lab penalties
                for (int j = 0; j < K; j++) {
                    Batch& b = batches[roomToBatch[labRoom[j]]];
                    long long delta = circDist(b.color, targetCol[j]);
                    C += pen[j] * delta * b.volume;
                }
            }
        }

        double tcScore;
        if (!valid) {
            tcScore = 0.0;
            // Don't quitf - score 0 and continue
        } else {
            if (B == 0 && C == 0) {
                tcScore = 2000.0;
            } else if (C == 0) {
                tcScore = 2000.0;
            } else {
                double ratio = (double)B / (double)C;
                tcScore = min(2000.0, 1000.0 * ratio);
                if (tcScore < 0.0) tcScore = 0.0;
            }
        }

        totalScore += tcScore;
    }

    // Check no trailing output
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra data in output after all test cases");
    }

    double avgScore = totalScore / (double)T;
    // avgScore in [0, 2000]; convert to [0, 1]
    double scoreRatio = avgScore / 2000.0;
    if (scoreRatio < 0.0) scoreRatio = 0.0;
    if (scoreRatio > 1.0) scoreRatio = 1.0;

    quitp(scoreRatio, "Ratio: %.6f (avg per-case score: %.2f / 2000)", scoreRatio, avgScore);
}