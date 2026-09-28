#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- read problem input from inf ----
    int m, n;
    m = inf.readInt();
    n = inf.readInt();

    vector<double> sa(m), sb(m), sc(m), sC(m), sF(m), sL(m), sU(m);
    for (int i = 0; i < m; i++) {
        sa[i] = inf.readDouble();
        sb[i] = inf.readDouble();
        sc[i] = inf.readDouble();
        sC[i] = inf.readDouble();
        sF[i] = inf.readDouble();
        sL[i] = inf.readDouble();
        sU[i] = inf.readDouble();
    }

    vector<double> cx(n), cy(n), cz(n), cQ(n);
    for (int j = 0; j < n; j++) {
        cx[j] = inf.readDouble();
        cy[j] = inf.readDouble();
        cz[j] = inf.readDouble();
        cQ[j] = inf.readDouble();
    }

    // ---- compute baseline cost ----
    double D_fe = 0, D_al = 0, D_sn = 0;
    for (int j = 0; j < n; j++) {
        D_fe += cx[j] * cQ[j];
        D_al += cy[j] * cQ[j];
        D_sn += cz[j] * cQ[j];
    }
    double baseline = 0.0;
    if (D_fe > 1e-12) baseline += sF[0] + sC[0] * D_fe;
    if (D_al > 1e-12) baseline += sF[1] + sC[1] * D_al;
    if (D_sn > 1e-12) baseline += sF[2] + sC[2] * D_sn;

    // ---- read participant output from ouf ----
    // R can be at most n*m (each contract uses all suppliers), but bound generously
    int maxR = n * m + 10;
    int R = ouf.readInt(0, maxR, "R");

    // y[j][i] = total amount assigned from supplier i to contract j
    vector<map<int,double>> assign(n);
    vector<double> sup_total(m, 0.0);

    for (int r = 0; r < R; r++) {
        int j = ouf.readInt(1, n, "j") - 1;
        int ii = ouf.readInt(1, m, "i") - 1;
        double t = ouf.readDouble();
        if (t <= 0.0) {
            quitf(_wa, "Assignment %d has non-positive amount t=%g (contract %d, supplier %d)",
                  r+1, t, j+1, ii+1);
        }
        assign[j][ii] += t;
        sup_total[ii] += t;
    }

    // NOTE: do NOT call ouf.readEof() – trailing whitespace/newlines are acceptable
    // and calling readEof() was causing all solutions to fail.

    // ---- Check constraint: at most 4 suppliers per contract ----
    for (int j = 0; j < n; j++) {
        int cnt = 0;
        for (auto& kv : assign[j]) {
            if (kv.second > 1e-12) cnt++;
        }
        if (cnt > 4) {
            quitf(_wa, "Contract %d uses %d suppliers (max 4)", j+1, cnt);
        }
    }

    // ---- Check contract balance and composition ----
    for (int j = 0; j < n; j++) {
        double eps = 1e-6 * max(1.0, cQ[j]);
        double sumMass = 0, sumFe = 0, sumAl = 0, sumSn = 0;
        for (auto& kv : assign[j]) {
            int ii = kv.first;
            double t = kv.second;
            sumMass += t;
            sumFe   += sa[ii] * t;
            sumAl   += sb[ii] * t;
            sumSn   += sc[ii] * t;
        }
        if (fabs(sumMass - cQ[j]) > eps) {
            quitf(_wa, "Contract %d: total mass %.9g != required %.9g (eps=%.3e)",
                  j+1, sumMass, cQ[j], eps);
        }
        if (fabs(sumFe - cx[j] * cQ[j]) > eps) {
            quitf(_wa, "Contract %d: iron balance %.9g != required %.9g (eps=%.3e)",
                  j+1, sumFe, cx[j]*cQ[j], eps);
        }
        if (fabs(sumAl - cy[j] * cQ[j]) > eps) {
            quitf(_wa, "Contract %d: aluminum balance %.9g != required %.9g (eps=%.3e)",
                  j+1, sumAl, cy[j]*cQ[j], eps);
        }
        if (fabs(sumSn - cz[j] * cQ[j]) > eps) {
            quitf(_wa, "Contract %d: tin balance %.9g != required %.9g (eps=%.3e)",
                  j+1, sumSn, cz[j]*cQ[j], eps);
        }
    }

    // ---- Check supplier bounds ----
    for (int i = 0; i < m; i++) {
        double Xi = sup_total[i];
        double eps = 1e-6 * max(1.0, sU[i]);
        if (Xi < -1e-12) {
            quitf(_wa, "Supplier %d has negative total usage %.9g", i+1, Xi);
        }
        if (Xi > 1e-12) {
            // supplier is used
            if (Xi < sL[i] - eps) {
                quitf(_wa, "Supplier %d: total usage %.9g < minimum %.9g (eps=%.3e)",
                      i+1, Xi, sL[i], eps);
            }
            if (Xi > sU[i] + eps) {
                quitf(_wa, "Supplier %d: total usage %.9g > maximum %.9g (eps=%.3e)",
                      i+1, Xi, sU[i], eps);
            }
        }
    }

    // ---- Compute participant cost ----
    double cost = 0.0;
    for (int i = 0; i < m; i++) {
        if (sup_total[i] > 1e-12) {
            cost += sF[i] + sC[i] * sup_total[i];
        }
    }

    // ---- Compute score ratio ----
    // Problem defines: score = 10^6 * clamp(B/C, 0, 2)
    // For quitp we need ratio in [0,1], so we map:
    //   ratio = clamp(B/C, 0, 2) / 2
    double ratio = 0.0;
    if (cost < 1e-12) {
        // Cost essentially zero => perfect (or baseline also zero)
        ratio = 1.0;
    } else if (baseline < 1e-12) {
        // Baseline is zero; any positive cost => ratio 0 (or give full if cost also ~0)
        // If baseline=0 but cost>0, the formula B/C = 0 => score 0
        ratio = 0.0;
    } else {
        double bc_ratio = baseline / cost;  // B/C
        // clamp to [0, 2] then divide by 2 to get [0,1]
        bc_ratio = max(0.0, min(2.0, bc_ratio));
        ratio = bc_ratio / 2.0;
    }

    quitp(ratio, "Feasible. Cost=%.6f Baseline=%.6f Ratio: %.9f", cost, baseline, ratio);
    return 0;
}