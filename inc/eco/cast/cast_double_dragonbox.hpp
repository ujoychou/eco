#pragma once
/*******************************************************************************
@ name
    cast_double_dragonbox.hpp

@ function
    Dragonbox: convert IEEE 754 double to its shortest round-trip decimal
    string representation.

@ reference
    Junekey Jeon, "Dragonbox: A New Floating-Point Binary-to-Decimal Conversion
    Algorithm" (2020). https://github.com/jk-jeon/dragonbox

@ note
    The high-level idea (compared to Ryu in cast_double_ryu.hpp):
      * Only ONE 128-bit cache table (no separate pow5 / pow5_inv).
      * The half-ulp interval [xi_real, zi_real] is captured by two corrected
        64-bit floors (zi, xi). No vm/vr/vp triplet, no per-digit trailing-
        zero bookkeeping inside a stripping loop.
      * For binary64 with kappa = 2 the "big divisor" path is a single
        division by 1000 plus one comparison against xi; the small-divisor
        fall-back uses divisor 100 (or 10 for the narrow asymmetric case)
        with explicit clamps into [xi_real, zi_real] under round-half-to-even.
      * Trailing zeros (very common when z_q is a power of ten) are stripped
        once at the end, giving the SHORTEST representation in a single
        pass.

    Output format matches cast_double_ryu.hpp: scientific notation with 'E'
    exponent, e.g. "1.23E2" / "0E0" / "Infinity" / "NaN".

--------------------------------------------------------------------------------
@ [2026-05-08] ujoy created


--------------------------------------------------------------------------------
* copyright(c) 2024 - 2027, ujoy, reserved all right.

*******************************************************************************/
#include <eco/prec.hpp>
#include <eco/cast/cast_double_ryu.hpp>
#include <cstring>


eco_namespace(eco);
eco_namespace(dragonbox_detail);
////////////////////////////////////////////////////////////////////////////////
// Reuse Ryu's IEEE 754 width and arithmetic primitives so the two algorithms
// share a single source of truth.
using ryu_detail::DOUBLE_MANTISSA_BITS;
using ryu_detail::DOUBLE_EXPONENT_BITS;
using ryu_detail::DOUBLE_BIAS;
using ryu_detail::floating_decimal_64;
using ryu_detail::mul_shift_64;
using ryu_detail::big_uint;


////////////////////////////////////////////////////////////////////////////////
// Algorithm parameters for binary64.
//   kappa = 2   : 'big divisor' is 10^kappa = 100.
//                 the analysis 2^e2 * 10^k in [10^kappa, 10^(kappa+1)) (i.e.
//                 deltai in [100, 1000)) makes the big-divisor branch
//                 unconditionally valid for the symmetric case.
//   k range     : derived from valid binary64 e2 = ieee_e - bias - 52.
//                   e2 in [-1074 .. 971]   ->   k = -minus_k in [-290 .. 326].
constexpr int DRAGON_KAPPA      = 2;
constexpr int DRAGON_CACHE_BITS = 128;
constexpr int DRAGON_K_MIN      = -290;
constexpr int DRAGON_K_MAX      =  326;
constexpr int DRAGON_CACHE_LEN  = DRAGON_K_MAX - DRAGON_K_MIN + 1;


////////////////////////////////////////////////////////////////////////////////
// Signed log helpers; valid for |e| <= 1700 which covers binary64 comfortably.
//   floor(e * log10(2))   ~ e * 169464822037455 / 2^49
//   floor(e * log2 (10))  ~ e * 1741647         / 2^19
inline int32_t floor_log10_pow2_signed(int32_t e)
{
    return (int32_t)(((int64_t)e * 169464822037455LL) >> 49);
}
inline int32_t floor_log2_pow10_signed(int32_t e)
{
    return (int32_t)(((int64_t)e * 1741647LL) >> 19);
}


////////////////////////////////////////////////////////////////////////////////
// Dragonbox cache. cache[k] is a 128-bit integer c_k satisfying
//   c_k * 2^(g_k - 127)  ~  10^k    (truncated; under-approximation)
// where g_k = floor(k * log2(10)). The 'truncation' direction is chosen so
// that the computed zi never exceeds the true zi - off-by-one only ever
// shrinks zi by 1, which keeps round-trip safety on every code path.
struct double_dragonbox_cache
{
    __uint128_t cache[DRAGON_CACHE_LEN];

    inline double_dragonbox_cache()
    {
        // Pass 1: non-negative k (use 5^k stored in big_uint, shift to 128 bits).
        big_uint pow_5;  pow_5.set_one();
        for (int k = 0; k <= DRAGON_K_MAX; ++k)
        {
            // c_k ~ floor(10^k * 2^(127 - g_k))
            //     = floor(5^k  * 2^(k + 127 - g_k))
            //     = floor(5^k  * 2^(-discard))           where discard = g_k - k - 127
            const int g_k     = floor_log2_pow10_signed(k);
            const int discard = g_k - k - 127;
            cache[k - DRAGON_K_MIN] =
                pow_5.to_u128_ceil_at_bit(discard, /*round_up=*/false);
            if (k < DRAGON_K_MAX) pow_5.mul_5();
        }

        // Pass 2: negative k. c_k = floor(2^(127 - g_k - |k|) / 5^|k|).
        // Using 5^|k| (instead of 10^|k|) keeps the divisor inside the existing
        // 1024-bit big_uint scratch space.
        big_uint pow_5n;  pow_5n.set_one();
        for (int k = -1; k >= DRAGON_K_MIN; --k)
        {
            const int K = -k;
            pow_5n.mul_5();                                 // pow_5n = 5^K
            const int g_k   = floor_log2_pow10_signed(k);   // negative
            const int shift = 127 - g_k - K;                // positive

            big_uint num;  num.set_pow2(shift);
            big_uint quo;
            big_uint::divmod(num, pow_5n, quo);
            // Round-down: do NOT bump quo on non-zero remainder.
            const uint64_t lo = quo.len > 0 ? quo.w[0] : 0;
            const uint64_t hi = quo.len > 1 ? quo.w[1] : 0;
            cache[k - DRAGON_K_MIN] = ((__uint128_t)hi << 64) | lo;
        }
    }
};

inline const double_dragonbox_cache& dragonbox_tables()
{
    static const double_dragonbox_cache t;
    return t;
}

inline __uint128_t dragonbox_cache_get(int32_t k)
{
    return dragonbox_tables().cache[k - DRAGON_K_MIN];
}


////////////////////////////////////////////////////////////////////////////////
// compute_mul:
//   Dragonbox needs floor(u * 10^k * 2^(e2-1)) for the upper bound zi (with
//   u = 2*m + 1) and the lower bound xi (u = 2*m - 1, or u = 4*m - 1 with
//   one more bit of right shift in the asymmetric binade-boundary case).
//
//   With cache c_k ~ 10^k * 2^(127 - g_k) and beta_minus_1 = e2 + g_k - 1,
//   this is a 64x128 multiply-shift identical in form to mul_shift_64:
//      compute_mul(u, c_k, beta_minus_1) == mul_shift_64(u, c_k, 127 - beta_minus_1)
inline uint64_t compute_mul(uint64_t u, __uint128_t c, int32_t beta_minus_1)
{
    return mul_shift_64(u, c, 127 - beta_minus_1);
}


////////////////////////////////////////////////////////////////////////////////
// cache_is_exact:
//   Returns true when cache[k] equals the exact value of 10^k * 2^(127 - g_k)
//   (no rounding loss). For k in [0, 55] the cache is just 5^k (or a left
//   shift thereof) which fits exactly in 128 bits. For k < 0 we use binary
//   long division, and for k >= 56 the right shift drops bits; both lose
//   sub-ULP precision in the cache.
inline bool cache_is_exact(int32_t k)
{
    return k >= 0 && k <= 55;
}


////////////////////////////////////////////////////////////////////////////////
// product_is_integer:
//   Returns true iff u * 10^k * 2^(e2 - 1) is exactly integer-valued.
//
//   When cache_is_exact(k) is true, compute_mul returns the exact floor and
//   nothing needs correcting. Otherwise the cache truncation bias means
//   compute_mul can underestimate by 1 ULP precisely when the underlying
//   value is integer; this predicate tells us when to correct.
//
//     k >  0 :  10^k = 5^k * 2^k.  product is integer iff e2 + k >= 1 OR
//               u carries the missing power of two (impossible for the odd
//               (2m +/- 1); the even (2m) and (2m - 2) carry their own).
//     k <= 0 :  product is integer iff e2 - 1 >= |k| AND 5^|k| divides u.
inline bool product_is_integer(uint64_t u, int32_t e2, int32_t k)
{
    if (k >= 0)
    {
        if (e2 + k >= 1) return true;
        const uint32_t p = (uint32_t)(1 - e2 - k);
        // u fits in 54 bits, so any p >= 54 trivially fails (and p >= 64
        // would invoke UB in the bit-mask form of multiple_of_pow2).
        if (p >= 64) return u == 0;
        return ryu_detail::multiple_of_pow2(u, p);
    }
    if (e2 - 1 < -k) return false;
    return ryu_detail::multiple_of_pow5(u, (uint32_t)(-k));
}


////////////////////////////////////////////////////////////////////////////////
// compute_mul_corrected:
//   compute_mul + the cache-truncation correction described above. For exact
//   caches it is a plain pass-through.
inline uint64_t compute_mul_corrected(uint64_t u, __uint128_t c,
    int32_t beta_minus_1, int32_t e2, int32_t k)
{
    uint64_t v = compute_mul(u, c, beta_minus_1);
    if (!cache_is_exact(k) && product_is_integer(u, e2, k))
    {
        ++v;
    }
    return v;
}


////////////////////////////////////////////////////////////////////////////////
// xi corrected / xi_is_integer:
//   In the symmetric case (the overwhelming majority of inputs) the lower
//   endpoint is xi_real = (2m-1) * 10^k * 2^(e2-1).
//   At a binade boundary with m = 2^p AND e2 > the smallest-normal limit the
//   left neighbour has half the ULP, making the asymmetric lower endpoint
//   xi_real = (4m-1) * 10^k * 2^(e2-2). Computing it via compute_mul with
//   beta_minus_1 - 1 (i.e. one extra right shift) and adjusting the
//   product_is_integer probe by the same one-step downshift.
inline uint64_t compute_xi_corrected(uint64_t two_fc, __uint128_t c,
    int32_t beta_minus_1, int32_t e2, int32_t k, bool asymmetric)
{
    if (!asymmetric)
        return compute_mul_corrected(two_fc - 1, c, beta_minus_1, e2, k);

    const uint64_t u = (two_fc << 1) - 1ULL;             // 4m - 1
    uint64_t v = compute_mul(u, c, beta_minus_1 - 1);
    if (!cache_is_exact(k) && product_is_integer(u, e2 - 1, k))
        ++v;
    return v;
}

inline bool xi_is_integer_check(uint64_t two_fc, int32_t e2, int32_t k,
    bool asymmetric)
{
    if (!asymmetric)
        return product_is_integer(two_fc - 1, e2, k);
    return product_is_integer((two_fc << 1) - 1ULL, e2 - 1, k);
}


////////////////////////////////////////////////////////////////////////////////
// d2d: the Dragonbox core. Convert decoded IEEE bits (ieee_m, ieee_e) to a
// {mantissa, exponent} decimal pair representing the shortest round-trip
// decimal of the input double.
inline floating_decimal_64 d2d(uint64_t ieee_m, uint32_t ieee_e)
{
    int32_t  e2;
    uint64_t m2;
    if (ieee_e == 0)
    {
        e2 = 1 - DOUBLE_BIAS - DOUBLE_MANTISSA_BITS;
        m2 = ieee_m;
    }
    else
    {
        e2 = (int32_t)ieee_e - DOUBLE_BIAS - DOUBLE_MANTISSA_BITS;
        m2 = ((uint64_t)1 << DOUBLE_MANTISSA_BITS) | ieee_m;
    }

    const bool is_even     = (m2 & 1) == 0;
    const bool round_left  = is_even;
    const bool round_right = is_even;
    // Asymmetric half-ulp ONLY at the boundary between binades, i.e. when the
    // mantissa is exactly the implicit-bit value AND we are not the smallest
    // normal (which would have e2 already at its subnormal limit).
    const bool asymmetric  = (ieee_m == 0 && ieee_e > 1);

    // Step 1: pick decimal exponent k and binary correction beta-1.
    const int32_t minus_k      = floor_log10_pow2_signed(e2) - DRAGON_KAPPA;
    const int32_t k            = -minus_k;
    const int32_t beta_minus_1 = e2 + floor_log2_pow10_signed(k) - 1;

    // Step 2: cache lookup + corrected 64x128 multiply-shift to obtain zi
    // and xi (=floor of the true real-valued upper / lower endpoints of the
    // half-ulp interval). With a truncated cache, compute_mul under-estimates
    // by one ULP precisely when the underlying real value is an exact
    // integer; compute_mul_corrected applies that correction.
    const __uint128_t c     = dragonbox_cache_get(k);
    const uint64_t two_fc   = m2 << 1;
    const uint64_t two_fc_p = two_fc | 1ULL;                       // 2m + 1

    const uint64_t zi = compute_mul_corrected(two_fc_p, c, beta_minus_1, e2, k);
    const uint64_t xi = compute_xi_corrected (two_fc,  c, beta_minus_1, e2, k, asymmetric);

    // Real-value-is-integer predicates. Note these are properties of the TRUE
    // unrounded products and therefore independent of cache exactness; the
    // cache flag matters only inside compute_*_corrected.
    const bool xi_is_integer = xi_is_integer_check(two_fc, e2, k, asymmetric);
    const bool zi_is_integer = product_is_integer(two_fc_p, e2, k);

    // Step 3: big-divisor decision via DIRECT interval check.
    //   The 'big divisor' is 10^(kappa+1) = 1000; the 'small divisor' is
    //   10^kappa = 100. Removing kappa+1 decimal digits in one division
    //   yields the canonical mantissa for ~99% of symmetric inputs.
    //   The candidate cand = z_q * 1000 is in [xi_real, zi_real] with the
    //   appropriate open/closed endpoints iff:
    //     cand >  xi  (always inside, since xi_real < xi+1)
    //     cand == xi  (lower endpoint, valid only when round_left and xi is
    //                  an exact integer)
    //   And on the upper side, cand <= zi by construction; cand == zi is
    //   valid only when round_right OR zi is non-integer.
    //
    //   Going via cand vs xi (instead of the classical big_r vs deltai
    //   comparison) avoids an off-by-one trap: when cache truncation makes
    //   the computed zi - xi exceed deltai by 1, big_r == deltai still
    //   leaves cand strictly above xi_real and the big-divisor candidate is
    //   actually valid. The direct comparison handles this uniformly.
    constexpr uint64_t big_divisor   = 1000;   // 10^(kappa+1)
    constexpr uint64_t small_divisor =  100;   // 10^kappa

    const uint64_t z_q  = zi / big_divisor;
    const uint64_t cand = z_q * big_divisor;

    bool cand_valid;
    if      (cand >  xi) cand_valid = true;
    else if (cand == xi) cand_valid = round_left && xi_is_integer;
    else                 cand_valid = false;
    if (cand_valid && cand == zi && zi_is_integer && !round_right)
        cand_valid = false;

    floating_decimal_64 fd;
    if (cand_valid)
    {
        // Common path (~99% of symmetric inputs): take z_q at big-divisor
        // scale and exit. Trailing zeros (very common when z_q lands on a
        // power of ten) are stripped below.
        fd.mantissa = z_q;
        fd.exponent = minus_k + DRAGON_KAPPA + 1;
    }
    else
    {
        // Small-divisor case: emit one extra decimal digit. We round the
        // true midpoint yi to the nearest multiple of small_divisor (round-
        // half-to-even), then CLAMP into the actual half-ulp interval.
        //
        // For binary64 with kappa=2, the symmetric half-ulp interval width
        // (10^k * 2^e2) lies in [100, 1000) - ALWAYS containing a multiple
        // of 100, so divisor 100 succeeds. The asymmetric case at a binade
        // boundary has width 3/4 of that, i.e. [75, 750), and the lower
        // edge of this range may contain NO multiple of 100. When that
        // happens (s_q_low > s_q_high), we drop one more digit and try
        // divisor 10 instead, which always succeeds for binary64.
        const uint64_t yi = compute_mul_corrected(two_fc, c, beta_minus_1, e2, k);
        const bool yi_is_integer = product_is_integer(two_fc, e2, k);

        // Precompute s_q_low / s_q_high at divisor 100 to determine if a
        // 16-digit answer exists.
        uint64_t s_q_low_100 = xi / small_divisor;
        if ((xi - s_q_low_100 * small_divisor) != 0) ++s_q_low_100;
        if (s_q_low_100 * small_divisor == xi && (!xi_is_integer || !round_left))
            ++s_q_low_100;

        uint64_t s_q_high_100 = zi / small_divisor;
        if (s_q_high_100 * small_divisor == zi && zi_is_integer && !round_right)
            --s_q_high_100;

        const bool fits_at_100 = (s_q_low_100 <= s_q_high_100);

        const uint64_t divisor = fits_at_100 ? small_divisor : 10ULL;
        const uint32_t half    = fits_at_100 ? 50u           : 5u;

        uint64_t       s_q = yi / divisor;
        const uint32_t s_r = (uint32_t)(yi - s_q * divisor);
        const bool round_up =
               s_r > half
            || (s_r == half && (!yi_is_integer || (s_q & 1)));
        if (round_up) ++s_q;

        if (fits_at_100)
        {
            if (s_q < s_q_low_100)  s_q = s_q_low_100;
            if (s_q > s_q_high_100) s_q = s_q_high_100;
            fd.mantissa = s_q;
            fd.exponent = minus_k + DRAGON_KAPPA;
        }
        else
        {
            // Fallback divisor 10 (one extra decimal digit). Compute clamps
            // at the new scale: s_q_low_10 = ceil(xi_real / 10),
            // s_q_high_10 = floor(zi_real / 10).
            uint64_t s_q_low_10 = xi / divisor;
            if ((xi - s_q_low_10 * divisor) != 0) ++s_q_low_10;
            if (s_q_low_10 * divisor == xi && (!xi_is_integer || !round_left))
                ++s_q_low_10;

            uint64_t s_q_high_10 = zi / divisor;
            if (s_q_high_10 * divisor == zi && zi_is_integer && !round_right)
                --s_q_high_10;

            if (s_q < s_q_low_10)  s_q = s_q_low_10;
            if (s_q > s_q_high_10) s_q = s_q_high_10;

            fd.mantissa = s_q;
            fd.exponent = minus_k + DRAGON_KAPPA - 1;
        }
    }

    // Step 4: trailing-zero stripping for the shortest representation.
    while (fd.mantissa >= 10 && fd.mantissa % 10 == 0)
    {
        fd.mantissa /= 10;
        ++fd.exponent;
    }
    return fd;
}


////////////////////////////////////////////////////////////////////////////////
// Top-level entry: convert a double to its shortest scientific representation.
// 'result' must have at least 25 bytes; returns number of bytes written
// (no trailing NUL). Output format matches cast_double_ryu.hpp.
inline int d2s_buffered_n(double d, char* result)
{
    uint64_t bits;
    memcpy(&bits, &d, sizeof(bits));

    const bool     ieee_sign = (bits >> (DOUBLE_MANTISSA_BITS + DOUBLE_EXPONENT_BITS)) & 1;
    const uint64_t ieee_m    = bits & (((uint64_t)1 << DOUBLE_MANTISSA_BITS) - 1);
    const uint32_t ieee_e    = (uint32_t)((bits >> DOUBLE_MANTISSA_BITS)
                                          & (((uint32_t)1 << DOUBLE_EXPONENT_BITS) - 1));

    const bool exp_all_one = (ieee_e == ((1u << DOUBLE_EXPONENT_BITS) - 1u));
    if (exp_all_one || (ieee_e == 0 && ieee_m == 0))
        return ryu_detail::copy_special(ieee_sign, exp_all_one, ieee_m == 0, result);

    const floating_decimal_64 fd = d2d(ieee_m, ieee_e);
    return ryu_detail::to_chars(fd, ieee_sign, result);
}


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(dragonbox_detail);
eco_namespace_end(eco);
