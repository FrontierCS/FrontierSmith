#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

typedef long long ll;

struct Point { ll x, y; };

ll cross2d(Point O, Point A, Point B) {
    return (A.x - O.x) * (ll)(B.y - O.y) - (A.y - O.y) * (ll)(B.x - O.x);
}

int sgn(ll v) {
    if (v > 0) return 1;
    if (v < 0) return -1;
    return 0;
}

bool onSeg(Point A, Point B, Point P) {
    if (cross2d(A, B, P) != 0) return false;
    return min(A.x,B.x) <= P.x && P.x <= max(A.x,B.x) &&
           min(A.y,B.y) <= P.y && P.y <= max(A.y,B.y);
}

bool segsIntersect(Point A, Point B, Point C, Point D) {
    ll d1 = cross2d(C, D, A);
    ll d2 = cross2d(C, D, B);
    ll d3 = cross2d(A, B, C);
    ll d4 = cross2d(A, B, D);
    if (sgn(d1)*sgn(d2) < 0 && sgn(d3)*sgn(d4) < 0) return true;
    if (onSeg(C,D,A)) return true;
    if (onSeg(C,D,B)) return true;
    if (onSeg(A,B,C)) return true;
    if (onSeg(A,B,D)) return true;
    return false;
}

// Check if polygon is simple (no self-intersections)
bool isSimplePoly(const vector<Point>& poly) {
    int m = (int)poly.size();
    for (int i = 0; i < m; i++) {
        Point A = poly[i], B = poly[(i+1)%m];
        for (int j = i+2; j < m; j++) {
            // skip adjacent edge pairs that share exactly one endpoint
            if (i == 0 && j == m-1) continue;
            Point C = poly[j], D = poly[(j+1)%m];
            ll d1 = cross2d(C, D, A);
            ll d2 = cross2d(C, D, B);
            ll d3 = cross2d(A, B, C);
            ll d4 = cross2d(A, B, D);
            // proper crossing
            if (sgn(d1)*sgn(d2) < 0 && sgn(d3)*sgn(d4) < 0) return false;
            // endpoint on non-adjacent edge segment means self-intersection
            // edges share no endpoint (non-adjacent), so any collinear endpoint is bad
            // Check B on CD, A on CD, C on AB, D on AB
            bool shareEnd = false;
            // edge i: A=poly[i], B=poly[(i+1)%m]
            // edge j: C=poly[j], D=poly[(j+1)%m]
            // They share a vertex only if adjacent (skipped above)
            if (!shareEnd) {
                if (onSeg(C,D,A)) return false;
                if (onSeg(C,D,B)) return false;
                if (onSeg(A,B,C)) return false;
                if (onSeg(A,B,D)) return false;
            }
        }
    }
    return true;
}

// Point strictly inside polygon using ray casting
bool ptStrictlyInside(const vector<Point>& poly, Point P) {
    int m = (int)poly.size();
    int cnt = 0;
    for (int i = 0; i < m; i++) {
        Point A = poly[i], B = poly[(i+1)%m];
        // on boundary check
        if (onSeg(A, B, P)) return false; // on boundary, not strictly inside
        if ((A.y <= P.y && B.y > P.y) || (B.y <= P.y && A.y > P.y)) {
            double xInt = (double)(B.x - A.x) * (P.y - A.y) / (double)(B.y - A.y) + A.x;
            if (P.x < xInt) cnt++;
        }
    }
    return (cnt % 2) == 1;
}

// Check two polygons are fully disjoint (no intersection, no containment, no touching)
bool polygonsDisjoint(const vector<Point>& P, const vector<Point>& Q) {
    int pm = (int)P.size(), qm = (int)Q.size();
    // Check all edge pairs
    for (int i = 0; i < pm; i++) {
        for (int j = 0; j < qm; j++) {
            if (segsIntersect(P[i], P[(i+1)%pm], Q[j], Q[(j+1)%qm])) return false;
        }
    }
    // Check if one polygon is inside the other
    if (ptStrictlyInside(P, Q[0])) return false;
    if (ptStrictlyInside(Q, P[0])) return false;
    return true;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // Read input
    int n = inf.readInt();
    ll C = inf.readLong();

    vector<Point> pts(n);
    vector<ll> w(n);
    for (int i = 0; i < n; i++) {
        pts[i].x = inf.readLong();
        pts[i].y = inf.readLong();
        w[i] = inf.readLong();
    }

    // Compute upper bound U = sum of max(w_i, 0)
    ll U = 0;
    for (int i = 0; i < n; i++) {
        if (w[i] > 0) U += w[i];
    }

    // Read participant output
    int k = ouf.readInt(0, n, "k (number of fences)");

    vector<vector<int>> fences(k);
    vector<vector<Point>> polys(k);

    vector<bool> usedMarker(n, false);

    for (int fi = 0; fi < k; fi++) {
        int m = ouf.readInt(3, n, "fence size");
        fences[fi].resize(m);
        polys[fi].resize(m);
        for (int j = 0; j < m; j++) {
            int v = ouf.readInt(1, n, "vertex index") - 1;
            if (usedMarker[v]) {
                quitf(_wa, "Marker %d used more than once across all fences", v+1);
            }
            usedMarker[v] = true;
            fences[fi][j] = v;
            polys[fi][j] = pts[v];
        }
        // Check all indices in this fence are distinct
        set<int> fenceSet(fences[fi].begin(), fences[fi].end());
        if ((int)fenceSet.size() != m) {
            quitf(_wa, "Fence %d has duplicate vertex indices", fi+1);
        }
    }

    // Strict EOF check — no trailing garbage allowed
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra output found after the last fence");
    }

    // Check each fence is a simple polygon
    for (int fi = 0; fi < k; fi++) {
        if (!isSimplePoly(polys[fi])) {
            quitf(_wa, "Fence %d is not a simple polygon", fi+1);
        }
    }

    // Check pairs of fences are disjoint
    for (int i = 0; i < k; i++) {
        for (int j = i+1; j < k; j++) {
            if (!polygonsDisjoint(polys[i], polys[j])) {
                quitf(_wa, "Fences %d and %d intersect, touch, or overlap", i+1, j+1);
            }
        }
    }

    // Compute protected set
    vector<bool> isVertex(n, false);
    for (int fi = 0; fi < k; fi++)
        for (int idx : fences[fi])
            isVertex[idx] = true;

    vector<bool> protectedMark(n, false);
    for (int i = 0; i < n; i++) {
        if (isVertex[i]) {
            protectedMark[i] = true;
            continue;
        }
        for (int fi = 0; fi < k; fi++) {
            if (ptStrictlyInside(polys[fi], pts[i])) {
                protectedMark[i] = true;
                break;
            }
        }
    }

    // Compute OBJ
    ll sumProtected = 0;
    for (int i = 0; i < n; i++) {
        if (protectedMark[i]) sumProtected += w[i];
    }

    ll sumEdgeCost = 0;
    for (int fi = 0; fi < k; fi++) {
        int m = (int)fences[fi].size();
        for (int j = 0; j < m; j++) {
            int a = fences[fi][j];
            int b = fences[fi][(j+1)%m];
            ll dx = pts[a].x - pts[b].x;
            ll dy = pts[a].y - pts[b].y;
            sumEdgeCost += C * (dx*dx + dy*dy);
        }
    }

    ll OBJ = sumProtected - sumEdgeCost;

    // Scoring per problem statement:
    // B = 0 (baseline = no fences)
    // If U == 0: score = 1,000,000 if OBJ == 0, else 0
    // Else: score = 1,000,000 * clamp(OBJ / U, 0, 1)
    // quitp takes a value in [0, 1]; Frontier-CS judge multiplies by 1,000,000.
    double score_ratio;
    if (U == 0) {
        score_ratio = (OBJ == 0) ? 1.0 : 0.0;
    } else {
        double ratio = (double)OBJ / (double)U;
        score_ratio = max(0.0, min(1.0, ratio));
    }

    quitp(score_ratio, "OBJ=%lld U=%lld Ratio: %f", OBJ, U, score_ratio);
    return 0;
}