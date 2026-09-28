#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

typedef long long ll;

ll calcT(ll k, ll h) {
    return k * (2*h - k + 1) / 2;
}

ll calcR(ll k, ll h, ll A, ll B, ll C, ll D) {
    return D + A*k + B*h - C*calcT(k, h);
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ── read input ──────────────────────────────────────────────────────────
    int N = inf.readInt();
    int M = inf.readInt();

    vector<ll> S(N+1), F(N+1);
    for (int i = 1; i <= N; i++) {
        S[i] = inf.readLong();
        F[i] = inf.readLong();
    }

    vector<ll> Lc(M+1), Uc(M+1), Hc(M+1);
    vector<ll> Ac(M+1), Bc(M+1), Cc(M+1), Dc(M+1);
    for (int j = 1; j <= M; j++) {
        Lc[j] = inf.readLong();
        Uc[j] = inf.readLong();
        Hc[j] = inf.readLong();
        Ac[j] = inf.readLong();
        Bc[j] = inf.readLong();
        Cc[j] = inf.readLong();
        Dc[j] = inf.readLong();
    }

    // ── read participant output ─────────────────────────────────────────────
    int Q = ouf.readInt(0, M, "Q");

    vector<int> cid(Q), bid(Q);
    vector<ll>  ck(Q),  ch(Q);

    for (int q = 0; q < Q; q++) {
        cid[q] = ouf.readInt(1, M, "contract_id");
        bid[q] = ouf.readInt(1, N, "block_id");
        ck[q]  = ouf.readLong();
        ch[q]  = ouf.readLong();
    }

    // ── strict EOF check: no trailing tokens allowed ────────────────────────
    if (!ouf.seekEof())
        quitf(_wa, "extra output after the %d described assignments", Q);

    // ── feasibility checks ──────────────────────────────────────────────────
    // 1. distinct contract ids
    vector<bool> contract_used(M+1, false);
    for (int q = 0; q < Q; q++) {
        int j = cid[q];
        if (contract_used[j])
            quitf(_wa, "contract_id %d appears more than once", j);
        contract_used[j] = true;
    }

    // 2. (k,h) within contract bounds
    for (int q = 0; q < Q; q++) {
        int j = cid[q];
        ll k = ck[q], h = ch[q];
        if (k < Lc[j] || k > Uc[j])
            quitf(_wa, "contract %d: k=%lld not in [%lld,%lld]", j, k, Lc[j], Uc[j]);
        if (h < k || h > Hc[j])
            quitf(_wa, "contract %d: h=%lld not in [k=%lld, H=%lld]", j, h, k, Hc[j]);
    }

    // 3. block capacity
    vector<ll> used_stones(N+1, 0);
    for (int q = 0; q < Q; q++) {
        int i = bid[q];
        ll t = calcT(ck[q], ch[q]);
        used_stones[i] += t;
        if (used_stones[i] > S[i])
            quitf(_wa, "block %d overloaded: used %lld > capacity %lld", i, used_stones[i], S[i]);
    }

    // ── compute participant profit O ─────────────────────────────────────────
    vector<bool> blkUsed(N+1, false);
    for (int q = 0; q < Q; q++) blkUsed[bid[q]] = true;

    ll O = 0;
    for (int q = 0; q < Q; q++) {
        int j = cid[q];
        O += calcR(ck[q], ch[q], Ac[j], Bc[j], Cc[j], Dc[j]);
    }
    for (int i = 1; i <= N; i++) {
        if (blkUsed[i]) O -= F[i];
    }

    // ── compute upper bound U ────────────────────────────────────────────────
    ll U_val = 0;
    for (int j = 1; j <= M; j++) {
        ll best = 0;
        for (ll k = Lc[j]; k <= Uc[j]; k++) {
            for (ll h = k; h <= Hc[j]; h++) {
                ll r = calcR(k, h, Ac[j], Bc[j], Cc[j], Dc[j]);
                if (r > best) best = r;
            }
        }
        U_val += best;
    }

    // ── compute baseline B ───────────────────────────────────────────────────
    struct ContractBest {
        int j;
        ll R_val, T_val;
        ll k_val, h_val;
    };

    vector<ContractBest> candidates;
    for (int j = 1; j <= M; j++) {
        ll best_r = -1, best_t = 0, best_k = 0, best_h = 0;
        // density = R/T, maximize; tiebreak: larger R, smaller T, smaller k, smaller h
        // We compare using cross-multiplication to avoid floating point
        bool found = false;
        for (ll k = Lc[j]; k <= Uc[j]; k++) {
            for (ll h = k; h <= Hc[j]; h++) {
                ll r = calcR(k, h, Ac[j], Bc[j], Cc[j], Dc[j]);
                ll t = calcT(k, h);
                if (t <= 0) continue;
                if (!found || r <= 0) {
                    // first candidate or
                    if (!found && r > 0) {
                        best_r = r; best_t = t; best_k = k; best_h = h;
                        found = true;
                    } else if (!found) {
                        // track best even if non-positive, for tiebreak
                        if (r > best_r) { best_r = r; best_t = t; best_k = k; best_h = h; }
                    }
                    continue;
                }
                // compare density r/t vs best_r/best_t (cross multiply)
                // r*best_t vs best_r*t
                // use __int128 to avoid overflow
                __int128 lhs = (__int128)r * best_t;
                __int128 rhs = (__int128)best_r * t;
                bool better = false;
                if (lhs > rhs) better = true;
                else if (lhs == rhs) {
                    if (r > best_r) better = true;
                    else if (r == best_r) {
                        if (t < best_t) better = true;
                        else if (t == best_t) {
                            if (k < best_k) better = true;
                            else if (k == best_k && h < best_h) better = true;
                        }
                    }
                }
                if (better) { best_r = r; best_t = t; best_k = k; best_h = h; found = true; }
            }
        }
        if (!found) {
            // no valid (k,h) found — shouldn't happen given constraints
            // but handle gracefully
            continue;
        }
        if (best_r <= 0) continue; // discard if best reward is not positive

        candidates.push_back({j, best_r, best_t, best_k, best_h});
    }

    // Sort: desc density, desc R, asc contract id
    sort(candidates.begin(), candidates.end(), [&](const ContractBest& a, const ContractBest& b) {
        // compare a.R/a.T vs b.R/b.T
        __int128 lhs = (__int128)a.R_val * b.T_val;
        __int128 rhs = (__int128)b.R_val * a.T_val;
        if (lhs != rhs) return lhs > rhs;
        if (a.R_val != b.R_val) return a.R_val > b.R_val;
        return a.j < b.j;
    });

    vector<ll> rem(N+1);
    for (int i = 1; i <= N; i++) rem[i] = S[i];
    vector<bool> blkUsedB(N+1, false);
    ll B_val = 0;

    for (auto& cb : candidates) {
        ll t = cb.T_val;
        ll r = cb.R_val;

        int bestBlk = -1;
        ll bestInc  = 0;
        ll bestCap  = -1;

        for (int i = 1; i <= N; i++) {
            if (rem[i] < t) continue;
            ll inc = blkUsedB[i] ? r : (r - F[i]);
            if (inc > bestInc) {
                bestInc = inc;
                bestBlk = i;
                bestCap = rem[i];
            } else if (inc == bestInc && bestBlk != -1) {
                if (rem[i] > bestCap) {
                    bestBlk = i;
                    bestCap = rem[i];
                } else if (rem[i] == bestCap && i < bestBlk) {
                    bestBlk = i;
                }
            }
        }

        if (bestBlk != -1) {
            if (!blkUsedB[bestBlk]) {
                B_val -= F[bestBlk];
                blkUsedB[bestBlk] = true;
            }
            B_val += r;
            rem[bestBlk] -= t;
        }
    }

    // ── compute score ────────────────────────────────────────────────────────
    double score;
    if (U_val == B_val) {
        score = (O >= U_val) ? 1.0 : 0.0;
    } else {
        double ratio = (double)(O - B_val) / (double)(U_val - B_val);
        score = min(1.0, max(0.0, ratio));
    }

    quitp(score, "Profit=%lld Baseline=%lld UpperBound=%lld Ratio: %.6f",
          O, B_val, U_val, score);
}