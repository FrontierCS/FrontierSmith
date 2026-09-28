#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

typedef long long ll;
typedef pair<ll,ll> pll;

struct Card { ll x, y; };

ll mdist(ll x1, ll y1, ll x2, ll y2) {
    return abs(x1 - x2) + abs(y1 - y2);
}

// Returns {total_distance, total_turns}
pair<ll,ll> simulate(int N, ll H, ll sx, ll sy,
                     const vector<Card>& cards,
                     const vector<int>& pic) {
    int M = 2 * N;
    // state: 0=unseen, 1=seen-unmatched, 2=removed
    vector<int> state(M, 0);

    ll cur_x = sx, cur_y = sy;
    ll total_dist = 0, total_turns = 0;
    int removed = 0;

    while (removed < M) {
        total_turns++;

        // Build seen-unmatched map: picture -> list of card indices
        // (at most 2 per picture)
        vector<vector<int>> seen_by_pic(N);
        for (int i = 0; i < M; i++)
            if (state[i] == 1)
                seen_by_pic[pic[i]].push_back(i);

        // Collect known pairs (both seen-unmatched)
        // For each such pair, enumerate both orderings (u,v)
        // pick best by (cost, u, v) lex
        ll best_cost = LLONG_MAX;
        int best_u = INT_MAX, best_v = INT_MAX;
        bool has_pair = false;

        for (int p = 0; p < N; p++) {
            if ((int)seen_by_pic[p].size() == 2) {
                has_pair = true;
                int a = seen_by_pic[p][0], b = seen_by_pic[p][1];
                // try order (a, b)
                ll cost_ab = mdist(cur_x, cur_y, cards[a].x, cards[a].y)
                           + mdist(cards[a].x, cards[a].y, cards[b].x, cards[b].y);
                // try order (b, a)
                ll cost_ba = mdist(cur_x, cur_y, cards[b].x, cards[b].y)
                           + mdist(cards[b].x, cards[b].y, cards[a].x, cards[a].y);

                // compare (cost_ab, a, b) vs best
                if (cost_ab < best_cost ||
                    (cost_ab == best_cost && a < best_u) ||
                    (cost_ab == best_cost && a == best_u && b < best_v)) {
                    best_cost = cost_ab; best_u = a; best_v = b;
                }
                // compare (cost_ba, b, a) vs best
                if (cost_ba < best_cost ||
                    (cost_ba == best_cost && b < best_u) ||
                    (cost_ba == best_cost && b == best_u && a < best_v)) {
                    best_cost = cost_ba; best_u = b; best_v = a;
                }
            }
        }

        if (has_pair) {
            total_dist += best_cost;
            cur_x = cards[best_v].x;
            cur_y = cards[best_v].y;
            state[best_u] = 2;
            state[best_v] = 2;
            removed += 2;
        } else {
            // Find nearest unseen u from cur, ties by smaller index
            ll best_d = LLONG_MAX;
            int best_uu = -1;
            for (int i = 0; i < M; i++) {
                if (state[i] == 0) {
                    ll d = mdist(cur_x, cur_y, cards[i].x, cards[i].y);
                    if (d < best_d || (d == best_d && i < best_uu)) {
                        best_d = d; best_uu = i;
                    }
                }
            }
            total_dist += best_d;
            cur_x = cards[best_uu].x;
            cur_y = cards[best_uu].y;
            state[best_uu] = 1; // flip it

            // Check if mate is seen-unmatched
            int mate = -1;
            for (int i = 0; i < M; i++) {
                if (i != best_uu && pic[i] == pic[best_uu] && state[i] == 1) {
                    mate = i; break;
                }
            }

            if (mate != -1) {
                ll d2 = mdist(cur_x, cur_y, cards[mate].x, cards[mate].y);
                total_dist += d2;
                cur_x = cards[mate].x;
                cur_y = cards[mate].y;
                state[best_uu] = 2;
                state[mate] = 2;
                removed += 2;
            } else {
                // Find nearest unseen v from best_uu, ties by smaller index
                ll best_d2 = LLONG_MAX;
                int best_vv = -1;
                for (int i = 0; i < M; i++) {
                    if (state[i] == 0) { // unseen (best_uu is now state=1)
                        ll d = mdist(cards[best_uu].x, cards[best_uu].y,
                                     cards[i].x, cards[i].y);
                        if (d < best_d2 || (d == best_d2 && i < best_vv)) {
                            best_d2 = d; best_vv = i;
                        }
                    }
                }
                if (best_vv == -1) {
                    // No unseen card left; all remaining must be seen-unmatched
                    // but known_pairs was empty – this shouldn't normally happen
                    // with a valid assignment, but break gracefully
                    break;
                }
                total_dist += best_d2;
                cur_x = cards[best_vv].x;
                cur_y = cards[best_vv].y;

                if (pic[best_uu] == pic[best_vv]) {
                    state[best_uu] = 2;
                    state[best_vv] = 2;
                    removed += 2;
                } else {
                    // best_uu already state=1
                    state[best_vv] = 1;
                }
            }
        }
    }
    return {total_dist, total_turns};
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    int N = inf.readInt();
    ll H = inf.readLong();
    ll sx = inf.readLong(), sy = inf.readLong();

    vector<Card> cards(2 * N);
    for (int i = 0; i < 2 * N; i++) {
        cards[i].x = inf.readLong();
        cards[i].y = inf.readLong();
    }

    // Read participant output
    vector<int> pic(2 * N);
    for (int i = 0; i < 2 * N; i++) {
        pic[i] = ouf.readInt() - 1; // convert to 0-indexed
    }
    // Ensure no trailing tokens
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra tokens in output after the required %d integers", 2 * N);
    }

    // Validate feasibility
    for (int i = 0; i < 2 * N; i++) {
        if (pic[i] < 0 || pic[i] >= N) {
            quitf(_wa, "Picture label %d at position %d is out of range [1,%d]",
                  pic[i] + 1, i + 1, N);
        }
    }
    vector<int> cnt(N, 0);
    for (int i = 0; i < 2 * N; i++) cnt[pic[i]]++;
    for (int p = 0; p < N; p++) {
        if (cnt[p] != 2) {
            quitf(_wa, "Picture label %d appears %d time(s) (expected exactly 2)",
                  p + 1, cnt[p]);
        }
    }

    // Simulate participant assignment
    auto [d_you, t_you] = simulate(N, H, sx, sy, cards, pic);
    ll V_you = d_you + H * t_you;

    // Baseline assignment: positions 1&2 -> pic 1, 3&4 -> pic 2, ...
    vector<int> base_pic(2 * N);
    for (int i = 0; i < 2 * N; i++) base_pic[i] = i / 2;
    auto [d_base, t_base] = simulate(N, H, sx, sy, cards, base_pic);
    ll V_base = d_base + H * t_base;

    // score = 1,000,000 * clamp(V_you / V_base, 0, 3)
    // score_ratio for quitp is in [0,1], so divide by 3 (max multiplier)
    double ratio;
    if (V_base == 0) {
        // If baseline is 0, any non-negative V_you gives max score
        ratio = (V_you >= 0) ? 3.0 : 0.0;
    } else {
        ratio = (double)V_you / (double)V_base;
    }

    double clamped = max(0.0, min(3.0, ratio));
    double score_ratio = clamped / 3.0; // in [0,1]

    quitp(score_ratio,
          "V_you=%lld (dist=%lld turns=%lld), V_base=%lld (dist=%lld turns=%lld), "
          "raw_ratio=%.6f, Ratio: %.9f",
          V_you, d_you, t_you,
          V_base, d_base, t_base,
          ratio, score_ratio);
}