#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

// ============================================================
// Constants
// ============================================================
const long long MOD = 998244353;

long long pw(long long a, long long b) {
    a %= MOD; if (a < 0) a += MOD;
    long long res = 1;
    for (; b > 0; b >>= 1, a = a * a % MOD)
        if (b & 1) res = res * a % MOD;
    return res;
}

// ============================================================
// NTT
// ============================================================
void ntt(vector<long long>& a, bool inv_flag) {
    int n = (int)a.size();
    for (int i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) swap(a[i], a[j]);
    }
    for (int len = 2; len <= n; len <<= 1) {
        long long w = inv_flag
            ? pw(3, MOD - 1 - (MOD - 1) / len)
            : pw(3, (MOD - 1) / len);
        for (int i = 0; i < n; i += len) {
            long long wn = 1;
            for (int j = 0; j < len / 2; j++) {
                long long u = a[i + j], v = a[i + j + len / 2] * wn % MOD;
                a[i + j] = (u + v) % MOD;
                a[i + j + len / 2] = (u - v + MOD) % MOD;
                wn = wn * w % MOD;
            }
        }
    }
    if (inv_flag) {
        long long ni = pw(n, MOD - 2);
        for (auto& x : a) x = x * ni % MOD;
    }
}

vector<long long> poly_mul_trunc(const vector<long long>& a,
                                  const vector<long long>& b,
                                  int trunc) {
    int sz = (int)(a.size() + b.size()) - 1;
    int n = 1;
    while (n < sz) n <<= 1;
    vector<long long> fa(a), fb(b);
    fa.resize(n); fb.resize(n);
    ntt(fa, false); ntt(fb, false);
    for (int i = 0; i < n; i++) fa[i] = fa[i] * fb[i] % MOD;
    ntt(fa, true);
    fa.resize(min(sz, trunc));
    return fa;
}

// ============================================================
// Poly power mod x^n, reading big k from string
// ============================================================
vector<long long> poly_pow_mod(vector<long long> base_poly,
                                const string& k_str,
                                int n) {
    // Fast exp on polynomials; k can be astronomically large
    // We need to compute base_poly^k mod x^n
    // If k == 0 return {1}
    // Strategy: binary exponentiation on the string k

    // First check if k == 0
    bool k_is_zero = (k_str == "0");
    if (k_is_zero) {
        vector<long long> res(n, 0);
        res[0] = 1;
        return res;
    }

    // Convert k_str to binary via repeated halving
    // For large k (up to 10^(10^5) digits), we need big-integer halving.
    // We'll store k as a vector of decimal digits and halve it.
    // This is O(digits * log(k)) which could be large, but n <= 1e5 and
    // poly_mul is O(n log n), so we need at most O(n) multiplications.
    // Actually we only need k mod something for the exponent of coefficients,
    // but here we need full binary exp.
    // 
    // Better: since T = A^k mod x^n, and we need T, we do:
    // Take log then exp approach is faster for the checker.
    // But for simplicity and correctness, use binary exponentiation
    // reading bits from k_str (as a big decimal).
    //
    // We'll extract bits of k by dividing the decimal representation by 2.

    // Represent k as decimal string
    vector<int> digits;
    for (char c : k_str) digits.push_back(c - '0');

    // Collect bits (LSB first) via repeated halving
    vector<bool> bits;
    while (!(digits.size() == 1 && digits[0] == 0)) {
        // remainder
        bits.push_back(digits.back() & 1);
        // divide by 2
        int carry = 0;
        for (int i = 0; i < (int)digits.size(); i++) {
            int cur = carry * 10 + digits[i];
            digits[i] = cur / 2;
            carry = cur % 2;
        }
        // remove leading zeros
        while (digits.size() > 1 && digits[0] == 0)
            digits.erase(digits.begin());
    }
    // bits is LSB-first binary representation of k

    base_poly.resize(n);
    vector<long long> result(n, 0);
    result[0] = 1; // identity

    for (bool bit : bits) {
        if (bit) {
            result = poly_mul_trunc(result, base_poly, n);
            result.resize(n);
        }
        base_poly = poly_mul_trunc(base_poly, base_poly, n);
        base_poly.resize(n);
    }
    return result;
}

// ============================================================
// Compute B(x) from participant output
// ============================================================
vector<long long> compute_B(int n, long long s, int z,
                              const vector<pair<long long, int>>& factors) {
    // B(x) = s * x^z * prod(1 + c_j * x^{d_j}) mod x^n
    // Start with [s] shifted by z
    vector<long long> B(n, 0);
    if (z < n) B[z] = s;

    for (auto& [cj, dj] : factors) {
        // multiply B by (1 + cj * x^{dj}) mod x^n
        // new_B[i] = B[i] + cj * B[i - dj]
        vector<long long> newB(B);
        for (int i = dj; i < n; i++) {
            newB[i] = (newB[i] + cj * B[i - dj]) % MOD;
        }
        B = newB;
    }
    return B;
}

// ============================================================
// Circular distance
// ============================================================
long long dist_P(long long u, long long v) {
    long long d = (u - v % MOD + MOD) % MOD;
    return min(d, MOD - d);
}

// ============================================================
// Compute objective for a given B vs T
// ============================================================
long long compute_obj(const vector<long long>& B,
                       const vector<long long>& T,
                       int n, long long C, int m) {
    long long E = 0;
    for (int i = 0; i < n; i++) {
        E += (long long)(n - i) * dist_P(B[i], T[i]);
    }
    return E + C * (long long)m;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // --------------------------------------------------------
    // Read problem input
    // --------------------------------------------------------
    int n = inf.readInt();
    int L = inf.readInt();
    long long C = inf.readLong();
    string k_str = inf.readToken(); // big decimal k
    vector<long long> A(n);
    for (int i = 0; i < n; i++) A[i] = inf.readLong();

    // --------------------------------------------------------
    // Compute T(x) = A(x)^k mod x^n
    // --------------------------------------------------------
    vector<long long> T = poly_pow_mod(A, k_str, n);
    T.resize(n, 0);

    // --------------------------------------------------------
    // Compute baseline
    // --------------------------------------------------------
    long long Base;
    {
        bool all_zero = true;
        for (int i = 0; i < n; i++) if (T[i] != 0) { all_zero = false; break; }
        if (all_zero) {
            Base = 0; // baseline B0=0, T=0, E=0, m=0
        } else {
            int v = 0;
            while (v < n && T[v] == 0) v++;
            // Baseline: m=0, z=v, s=T[v]
            // B0[i] = T[v] if i==v, else 0
            long long E0 = 0;
            for (int i = 0; i < n; i++) {
                long long bi = (i == v) ? T[v] : 0LL;
                E0 += (long long)(n - i) * dist_P(bi, T[i]);
            }
            Base = E0; // m=0, no C*m term
        }
    }

    // --------------------------------------------------------
    // Read participant output -- be lenient about trailing whitespace
    // --------------------------------------------------------

    // Read m, z, s
    int m;
    {
        // Use readInt with bounds; WA on violation
        if (ouf.eof())
            quitf(_wa, "output is empty");
        m = ouf.readInt();
        if (m < 0 || m > L)
            quitf(_wa, "m=%d out of range [0,%d]", m, L);
    }
    int z;
    {
        z = ouf.readInt();
        if (z < 0 || z >= n)
            quitf(_wa, "z=%d out of range [0,%d]", z, n - 1);
    }
    long long s;
    {
        s = ouf.readLong();
        if (s < 0 || s >= MOD)
            quitf(_wa, "s=%lld out of range [0,P)", s);
    }

    vector<pair<long long, int>> factors(m);
    for (int j = 0; j < m; j++) {
        long long cj = ouf.readLong();
        if (cj < 0 || cj >= MOD)
            quitf(_wa, "c_%d=%lld out of range [0,P)", j, cj);
        int dj = ouf.readInt();
        if (dj < 1 || dj >= n)
            quitf(_wa, "d_%d=%d out of range [1,%d]", j, dj, n - 1);
        factors[j] = {cj, dj};
    }

    // Lenient EOF check: skip whitespace, then verify nothing else remains
    while (!ouf.eof()) {
        char c = ouf.readChar();
        if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
            quitf(_wa, "extra non-whitespace output after last factor");
        }
    }

    // --------------------------------------------------------
    // Compute B and objective
    // --------------------------------------------------------
    vector<long long> B = compute_B(n, s, z, factors);
    long long Obj = compute_obj(B, T, n, C, m);

    // --------------------------------------------------------
    // Score = floor(1e6 * min(3, (Base+1)/(Obj+1)))
    // ratio for quitp = score / 3000000, clamped to [0,1]
    // --------------------------------------------------------
    double ratio_inner = (double)(Base + 1) / (double)(Obj + 1);
    double clamped = min(3.0, ratio_inner);
    long long score_int = (long long)(1e6 * clamped); // floor
    // quitp expects a ratio in [0, 1]
    double qp_ratio = (double)score_int / 3000000.0;
    if (qp_ratio > 1.0) qp_ratio = 1.0;
    if (qp_ratio < 0.0) qp_ratio = 0.0;

    quitp(qp_ratio,
          "Ratio: %.9f | score=%lld | Obj=%lld (E=%lld + C*m=%lld*%d) | Base=%lld",
          qp_ratio, score_int, Obj,
          Obj - C * (long long)m, C, m, Base);

    return 0;
}