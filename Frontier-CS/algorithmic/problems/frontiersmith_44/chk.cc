#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

typedef long long ll;

ll gcdTwo(ll a, ll b) { return __gcd(a, b); }

ll computeValue(ll a, ll b, ll c, ll d, ll P) {
    ll g2 = gcdTwo(a,b) + gcdTwo(a,c) + gcdTwo(a,d)
           + gcdTwo(b,c) + gcdTwo(b,d) + gcdTwo(c,d);
    ll g4 = __gcd(__gcd(a, b), __gcd(c, d));
    ll val = g2 + 2LL*g4 - P;
    return val > 0LL ? val : 0LL;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---------- read problem input ----------
    int N = inf.readInt();
    ll P  = inf.readLong();
    vector<ll> x(N + 1);
    for (int i = 1; i <= N; i++) x[i] = inf.readLong();

    // ---------- compute baseline B ----------
    vector<pair<ll,int>> sc(N);
    for (int i = 0; i < N; i++) sc[i] = {x[i+1], i+1};
    sort(sc.begin(), sc.end(), [](const pair<ll,int>& a, const pair<ll,int>& b){
        return a.first != b.first ? a.first < b.first : a.second < b.second;
    });

    ll B = 0;
    for (int i = 0; i + 3 < N; i += 4) {
        int ia = sc[i].second, ib = sc[i+1].second,
            ic = sc[i+2].second, id = sc[i+3].second;
        B += computeValue(x[ia], x[ib], x[ic], x[id], P);
    }

    // ---------- read participant output ----------
    int q = ouf.readInt(0, N/4, "q must be in [0, floor(N/4)]");

    vector<bool> used(N + 1, false);
    ll A = 0;

    for (int t = 0; t < q; t++) {
        int idx[4];
        for (int k = 0; k < 4; k++)
            idx[k] = ouf.readInt(1, N, "index out of range [1,N]");

        // distinct indices within quartet
        for (int i = 0; i < 4; i++)
            for (int j = i+1; j < 4; j++)
                if (idx[i] == idx[j])
                    quitf(_wa, "Quartet %d has duplicate index %d", t+1, idx[i]);

        // not already used
        for (int k = 0; k < 4; k++) {
            if (used[idx[k]])
                quitf(_wa, "Index %d appears in multiple quartets", idx[k]);
            used[idx[k]] = true;
        }

        A += computeValue(x[idx[0]], x[idx[1]], x[idx[2]], x[idx[3]], P);
    }

    // NOTE: do NOT call ouf.readEof() — trailing newlines are acceptable.

    // ---------- score ----------
    // problem: score = 100 * clamp(0, 2, A / max(1, B))
    // quitp expects ratio in [0,1] where 1.0 == full marks (100 pts here)
    // so ratio = clamp(0,2, A/max(1,B)) / 2
    ll denom = max(1LL, B);
    double raw   = (double)A / (double)denom;   // in [0, ∞)
    double clamped = max(0.0, min(2.0, raw));   // in [0, 2]
    double ratio   = clamped / 2.0;             // in [0, 1]

    quitp(ratio, "Ratio: %.9f A=%lld B=%lld", ratio, A, B);
}