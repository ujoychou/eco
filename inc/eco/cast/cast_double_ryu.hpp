#pragma once
/*******************************************************************************
@ name
    cast_double_ryu.hpp

@ function
    Ryu algorithm: convert IEEE 754 double to its shortest round-trip
    decimal string representation.

@ reference
    Ulf Adams, "Ryu: Fast Float-to-String Conversion", PLDI 2018.
    https://dl.acm.org/doi/10.1145/3192366.3192369

--------------------------------------------------------------------------------
@ [2024-09-04] ujoy created

--------------------------------------------------------------------------------
* copyright(c) 2024 - 2027, ujoy, reserved all right.

*******************************************************************************/
#include <eco/prec.hpp>
#include <cstring>


eco_namespace(eco);
eco_namespace(ryu_detail);
////////////////////////////////////////////////////////////////////////////////
// IEEE 754 double field widths.
constexpr int DOUBLE_MANTISSA_BITS = 52;
constexpr int DOUBLE_EXPONENT_BITS = 11;
constexpr int DOUBLE_BIAS          = 1023;

// Bit-precision of the precomputed power-of-5 / inverse-power-of-5 tables.
constexpr int POW5_BITCOUNT        = 125;
constexpr int POW5_INV_BITCOUNT    = 125;
constexpr int POW5_TBL_SIZE        = 327;   // covers all i = -e2 - q  (e2 < 0)
constexpr int POW5_INV_TBL_SIZE    = 292;   // covers all q for e2 >= 0


////////////////////////////////////////////////////////////////////////////////
// Cheap integer approximations used by Ryu. Constants follow the original
// Ryu C reference implementation; each is a tight UNDER-estimate of log10
// (or tight OVER-estimate for pow5_bits) so that the algorithm's bounds
// are conservative and safe.
//   log10_pow2(e)  = floor(e * log10(2))
//   log10_pow5(e)  = floor(e * log10(5))
//   pow5_bits(e)   = ceil (e * log2 (5))
inline uint32_t log10_pow2(int32_t e)
{
    // 169464822037455 / 2^49 ≈ 0.301029995663...
    return (uint32_t)(((uint64_t)e * 169464822037455ULL) >> 49);
}
inline uint32_t log10_pow5(int32_t e)
{
    // 196742565691928 / 2^48 was a typo. Original Ryu uses the 32-bit form:
    // 732923 / 2^20 ≈ 0.698921203...
    return (uint32_t)(((uint32_t)e * 732923UL) >> 20);
}
inline int32_t pow5_bits(int32_t e)
{
    // 1217359 / 2^19 ≈ 2.32194...  (slightly above log2(5)); ceiling result.
    return (int32_t)(((uint32_t)e * 1217359UL) >> 19) + 1;
}


////////////////////////////////////////////////////////////////////////////////
// 128-bit multiply-shift used by the Ryu core. Computes (m * mul) >> j
// for j in (64, 192]. m has at most 55 bits, mul up to 128 bits.
inline uint64_t mul_shift_64(uint64_t m, __uint128_t mul, int32_t j)
{
    const uint64_t mul_hi = (uint64_t)(mul >> 64);
    const uint64_t mul_lo = (uint64_t)mul;

    // m * mul_lo : 64 + 64 -> 128 bits, occupies bits [0  ..127]
    // m * mul_hi : 64 + 64 -> 128 bits, occupies bits [64 ..191]
    const __uint128_t prod_lo = (__uint128_t)m * mul_lo;
    const __uint128_t prod_hi = (__uint128_t)m * mul_hi;

    // 'mid' carries bits [64..191] of the full product. Note prod_hi < 2^119
    // because m < 2^55, so adding a 64-bit value can never overflow.
    const __uint128_t mid = prod_hi + (uint64_t)(prod_lo >> 64);
    return (uint64_t)(mid >> (j - 64));
}


////////////////////////////////////////////////////////////////////////////////
// Tiny big-unsigned-integer used ONCE at static-init time to construct
// the pow5 / pow5_inv tables. We keep it minimal: only the operations
// Ryu's table generation needs.
struct big_uint
{
    static constexpr int N = 16;        // 16 * 64 = 1024 bits, plenty for 5^325
    uint64_t w[N];
    int      len;

    inline big_uint() { memset(w, 0, sizeof(w)); len = 0; }

    inline void set_zero()              { memset(w, 0, sizeof(w)); len = 0; }
    inline void set_one()               { set_zero(); w[0] = 1; len = 1; }

    inline int  is_zero() const         { return len == 0; }

    inline void set_pow2(int e)
    {
        set_zero();
        if (e < 0) { return; }
        len      = (e >> 6) + 1;
        w[e>>6]  = (uint64_t)1 << (e & 63);
    }

    // *this *= 5
    inline void mul_5()
    {
        uint64_t carry = 0;
        for (int i = 0; i < len; ++i)
        {
            __uint128_t p = (__uint128_t)w[i] * 5 + carry;
            w[i]  = (uint64_t)p;
            carry = (uint64_t)(p >> 64);
        }
        if (carry) { w[len++] = carry; }
    }

    // bit length: position of highest set bit + 1, or 0 for zero.
    inline int bit_length() const
    {
        if (len == 0) return 0;
        int    bl = (len - 1) * 64;
        uint64_t t = w[len - 1];
        while (t) { ++bl; t >>= 1; }
        return bl;
    }

    // *this <<= s
    inline void shl(int s)
    {
        if (s == 0 || len == 0) return;
        const int word_shift = s >> 6;
        const int bit_shift  = s & 63;
        const int new_len    = len + word_shift + (bit_shift ? 1 : 0);
        if (bit_shift)
        {
            for (int i = new_len - 1; i > word_shift; --i)
            {
                uint64_t hi = (i - word_shift     < len) ? w[i - word_shift]     : 0;
                uint64_t lo = (i - word_shift - 1 < len && i - word_shift - 1 >= 0)
                            ? w[i - word_shift - 1] : 0;
                w[i] = (hi << bit_shift) | (lo >> (64 - bit_shift));
            }
            w[word_shift] = w[0] << bit_shift;
        }
        else
        {
            for (int i = new_len - 1; i >= word_shift; --i)
                w[i] = w[i - word_shift];
        }
        for (int i = 0; i < word_shift; ++i) w[i] = 0;
        len = new_len;
        while (len > 0 && w[len - 1] == 0) --len;
    }

    // compare: -1, 0, +1
    inline int cmp(const big_uint& b) const
    {
        if (len != b.len) return len < b.len ? -1 : 1;
        for (int i = len - 1; i >= 0; --i)
            if (w[i] != b.w[i]) return w[i] < b.w[i] ? -1 : 1;
        return 0;
    }

    // *this -= b   (assumes *this >= b)
#if defined(__GNUC__) && !defined(__clang__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
    inline void sub(const big_uint& b)
    {
        uint64_t borrow = 0;
        for (int i = 0; i < len; ++i)
        {
            uint64_t bw = (i < b.len) ? b.w[i] : 0;
            __uint128_t diff = (__uint128_t)w[i] - bw - borrow;
            w[i]   = (uint64_t)diff;
            borrow = ((diff >> 64) != 0) ? 1 : 0;
        }
        while (len > 0 && w[len - 1] == 0) --len;
    }
#if defined(__GNUC__) && !defined(__clang__)
#  pragma GCC diagnostic pop
#endif

    // a / b -> q, remainder in *a (binary long division, O(bits(a)*len))
    static inline void divmod(big_uint& a, const big_uint& b, big_uint& q)
    {
        q.set_zero();
        if (a.cmp(b) < 0) return;

        const int bits_a = a.bit_length();
        big_uint  r;
        memset(r.w, 0, sizeof(r.w));   // explicit, helps GCC's flow analysis
        r.len = 0;

        for (int i = bits_a - 1; i >= 0; --i)
        {
            r.shl(1);
            const uint64_t bit = (a.w[i >> 6] >> (i & 63)) & 1ULL;
            if (bit)
            {
                if (r.len == 0) r.len = 1;
                r.w[0] |= 1ULL;
            }
            if (r.cmp(b) >= 0)
            {
                r.sub(b);
                const int wi = i >> 6;
                if (wi >= q.len) q.len = wi + 1;
                q.w[wi] |= (uint64_t)1 << (i & 63);
            }
        }
        a = r;
    }

    // Extract a 128-bit window: floor(*this / 2^total) << shift_to_top,
    // where total bits are kept and the result is stored as a __uint128_t.
    // For "round_up", we add 1 if any discarded bits were nonzero.
    inline __uint128_t to_u128_ceil_at_bit(int discard_lo, bool round_up) const
    {
        if (len == 0) return 0;
        if (discard_lo <= 0)
        {
            uint64_t l = w[0];
            uint64_t h = (len > 1) ? w[1] : 0;
            __uint128_t v = ((__uint128_t)h << 64) | l;
            if (-discard_lo > 0) v <<= -discard_lo;
            return v;
        }
        const int wi = discard_lo >> 6;
        const int bi = discard_lo & 63;

        uint64_t lo, hi;
        if (bi == 0)
        {
            lo = (wi     < len) ? w[wi    ] : 0;
            hi = (wi + 1 < len) ? w[wi + 1] : 0;
        }
        else
        {
            uint64_t w0 = (wi     < len) ? w[wi    ] : 0;
            uint64_t w1 = (wi + 1 < len) ? w[wi + 1] : 0;
            uint64_t w2 = (wi + 2 < len) ? w[wi + 2] : 0;
            lo = (w0 >> bi) | (w1 << (64 - bi));
            hi = (w1 >> bi) | (w2 << (64 - bi));
        }

        __uint128_t v = ((__uint128_t)hi << 64) | lo;
        if (round_up)
        {
            bool nz = false;
            for (int i = 0; i < wi && !nz; ++i)
                if (w[i]) nz = true;
            if (!nz && bi > 0 && (w[wi] & (((uint64_t)1 << bi) - 1))) nz = true;
            if (nz) ++v;
        }
        return v;
    }
};


////////////////////////////////////////////////////////////////////////////////
// Singletoned tables (Meyers init-on-first-use).
struct double_pow5_tables
{
    __uint128_t pow5    [POW5_TBL_SIZE];
    __uint128_t pow5_inv[POW5_INV_TBL_SIZE];

    inline double_pow5_tables()
    {
        // pow5[i]: for the e2 < 0 path. Stored to ~125 bits of precision,
        // packed in 128-bit. pow5[i] ≈ 5^i * 2^(POW5_BITCOUNT - bits(5^i)).
        big_uint v;  v.set_one();
        for (int i = 0; i < POW5_TBL_SIZE; ++i)
        {
            const int bits   = v.bit_length();
            const int shift  = bits - POW5_BITCOUNT;        // amount to drop low bits
            pow5[i] = v.to_u128_ceil_at_bit(shift, /*round_up=*/false);
            v.mul_5();
        }

        // pow5_inv[q]: for the e2 >= 0 path. We need (rounded-up) reciprocals.
        //   pow5_inv[q] = ceil(2^(POW5_INV_BITCOUNT - 1 + bits(5^q)) / 5^q)
        // Construct each entry via binary long division 2^N / 5^q.
        big_uint pow_q;  pow_q.set_one();
        for (int q = 0; q < POW5_INV_TBL_SIZE; ++q)
        {
            const int bits = pow_q.bit_length();
            const int N    = POW5_INV_BITCOUNT - 1 + bits;

            big_uint num;  num.set_pow2(N);
            big_uint quo;
            big_uint::divmod(num, pow_q, quo);
            // round-up: if there was any remainder, add 1
            if (!num.is_zero())
            {
                if (quo.len == 0) quo.len = 1;
                __uint128_t one = 1;
                // add 1 to quo
                uint64_t carry = 1;
                for (int i = 0; i < quo.len && carry; ++i)
                {
                    __uint128_t s = (__uint128_t)quo.w[i] + carry;
                    quo.w[i] = (uint64_t)s;
                    carry    = (uint64_t)(s >> 64);
                }
                if (carry) { quo.w[quo.len++] = carry; }
                (void)one;
            }
            // Pack lower 128 bits into __uint128_t.
            uint64_t lo = (quo.len > 0) ? quo.w[0] : 0;
            uint64_t hi = (quo.len > 1) ? quo.w[1] : 0;
            pow5_inv[q] = ((__uint128_t)hi << 64) | lo;

            pow_q.mul_5();
        }
    }
};

inline const double_pow5_tables& tables()
{
    static const double_pow5_tables t;
    return t;
}


////////////////////////////////////////////////////////////////////////////////
// 5-divisibility test: pow5_factor(v) returns max p s.t. 5^p divides v.
// Trick from Hacker's Delight: multiply by mod-inverse of 5 (mod 2^64).
inline uint32_t pow5_factor(uint64_t value)
{
    constexpr uint64_t M_INV_5 = 14757395258967641293ULL; // 5^-1 mod 2^64
    constexpr uint64_t N_DIV_5 = 3689348814741910323ULL;  // (2^64-1) / 5
    uint32_t count = 0;
    for (;;)
    {
        value *= M_INV_5;
        if (value > N_DIV_5) break;
        ++count;
    }
    return count;
}
inline bool multiple_of_pow5(uint64_t v, uint32_t p) { return pow5_factor(v) >= p; }
inline bool multiple_of_pow2(uint64_t v, uint32_t p) { return (v & (((uint64_t)1 << p) - 1)) == 0; }


////////////////////////////////////////////////////////////////////////////////
// Decimal representation: value = mantissa * 10^exponent.
struct floating_decimal_64
{
    uint64_t mantissa;
    int32_t  exponent;
};


////////////////////////////////////////////////////////////////////////////////
// d2d - the heart of Ryu. Convert decoded IEEE bits to (mantissa, exponent).
inline floating_decimal_64 d2d(uint64_t ieee_m, uint32_t ieee_e)
{
    int32_t  e2;
    uint64_t m2;
    if (ieee_e == 0)
    {   // sub-normal
        e2 = 1 - DOUBLE_BIAS - DOUBLE_MANTISSA_BITS - 2;
        m2 = ieee_m;
    }
    else
    {   // normal: implicit leading 1
        e2 = (int32_t)ieee_e - DOUBLE_BIAS - DOUBLE_MANTISSA_BITS - 2;
        m2 = ((uint64_t)1 << DOUBLE_MANTISSA_BITS) | ieee_m;
    }
    const bool even          = (m2 & 1) == 0;
    const bool accept_bounds = even;

    // Step 2: legal interval [mm, mp] around mv = 4*m2.
    //   mp = mv + 2,   mm = mv - 1 - mm_shift
    // mm_shift is 1 unless we are exactly on the boundary 2^52 ('the smallest
    // normalized'), in which case the lower neighbour is half a ULP closer.
    const uint64_t mv       = 4 * m2;
    const uint32_t mm_shift = (ieee_m != 0 || ieee_e <= 1) ? 1 : 0;
    const uint64_t mp       = mv + 2;
    const uint64_t mm       = mv - 1 - mm_shift;

    // Step 3: convert {mm, mv, mp} * 2^e2 into decimals using the lookup tables.
    int32_t  e10;
    uint64_t vr, vp, vm;
    bool     vm_trailing_zeros = false;
    bool     vr_trailing_zeros = false;

    if (e2 >= 0)
    {
        // Subtract 1 from log10 estimate to be safe (q can never be too large).
        const uint32_t q = log10_pow2(e2) - (e2 > 3 ? 1 : 0);
        e10 = (int32_t)q;
        const int32_t k = POW5_INV_BITCOUNT + pow5_bits((int32_t)q) - 1;
        const int32_t j = -e2 + (int32_t)q + k;

        const __uint128_t mul = tables().pow5_inv[q];
        vr = mul_shift_64(mv, mul, j);
        vp = mul_shift_64(mp, mul, j);
        vm = mul_shift_64(mm, mul, j);

        // Trailing-zero analysis: only matters for small q (less than 21 since
        // m2 has 53 bits and 5^q must not exceed 2^53).
        if (q <= 21)
        {
            // mv % 5 == 0  <=>  mv is a multiple of 5  <=>  the rounded vr
            // had no trailing-error bits at all.
            if (mv % 5 == 0)
            {
                vr_trailing_zeros = multiple_of_pow5(mv, q - 1);
            }
            else if (accept_bounds)
            {
                vm_trailing_zeros = multiple_of_pow5(mm, q);
            }
            else
            {
                vp -= multiple_of_pow5(mp, q);
            }
        }
    }
    else
    {
        const uint32_t q = log10_pow5(-e2) - (-e2 > 1 ? 1 : 0);
        e10 = (int32_t)q + e2;
        const int32_t i = -e2 - (int32_t)q;
        const int32_t k = pow5_bits(i) - POW5_BITCOUNT;
        const int32_t j = (int32_t)q - k;

        const __uint128_t mul = tables().pow5[i];
        vr = mul_shift_64(mv, mul, j);
        vp = mul_shift_64(mp, mul, j);
        vm = mul_shift_64(mm, mul, j);

        if (q <= 1)
        {
            // {vr,vp,vm} have at least q trailing 0 bits (since multiplying
            // by 4 gives 2 zero bits and the rest follow from the shift).
            vr_trailing_zeros = true;
            if (accept_bounds)
                vm_trailing_zeros = (mm_shift == 1);
            else
                --vp;
        }
        else if (q < 63)
        {
            vr_trailing_zeros = multiple_of_pow2(mv, q);
        }
    }

    // Step 4: find the shortest representation in (vm, vp).
    int32_t  removed       = 0;
    uint8_t  last_digit    = 0;
    uint64_t output;

    if (vm_trailing_zeros || vr_trailing_zeros)
    {
        // ~0.7% of cases: need to track trailing zeros.
        for (;;)
        {
            const uint64_t vp_d10 = vp / 10;
            const uint64_t vm_d10 = vm / 10;
            if (vp_d10 <= vm_d10) break;
            const uint64_t vm_m10 = vm - 10 * vm_d10;
            const uint64_t vr_d10 = vr / 10;
            const uint64_t vr_m10 = vr - 10 * vr_d10;
            vm_trailing_zeros &= (vm_m10 == 0);
            vr_trailing_zeros &= (last_digit == 0);
            last_digit = (uint8_t)vr_m10;
            vr = vr_d10;  vp = vp_d10;  vm = vm_d10;
            ++removed;
        }
        if (vm_trailing_zeros)
        {
            for (;;)
            {
                const uint64_t vm_d10 = vm / 10;
                const uint64_t vm_m10 = vm - 10 * vm_d10;
                if (vm_m10 != 0) break;
                const uint64_t vp_d10 = vp / 10;
                const uint64_t vr_d10 = vr / 10;
                const uint64_t vr_m10 = vr - 10 * vr_d10;
                vr_trailing_zeros &= (last_digit == 0);
                last_digit = (uint8_t)vr_m10;
                vr = vr_d10;  vp = vp_d10;  vm = vm_d10;
                ++removed;
            }
        }
        if (vr_trailing_zeros && last_digit == 5 && (vr & 1) == 0)
        {
            // round-half-to-even
            last_digit = 4;
        }
        output = vr +
            ((vr == vm && (!accept_bounds || !vm_trailing_zeros)) || last_digit >= 5);
    }
    else
    {
        // ~99.3% common case: avoid trailing-zero bookkeeping for speed.
        bool round_up = false;
        // First strip in groups of 100 if possible (fewer iterations).
        const uint64_t vp_d100 = vp / 100;
        const uint64_t vm_d100 = vm / 100;
        if (vp_d100 > vm_d100)
        {
            const uint64_t vr_d100 = vr / 100;
            const uint64_t vr_m100 = vr - 100 * vr_d100;
            round_up = (vr_m100 >= 50);
            vr = vr_d100;  vp = vp_d100;  vm = vm_d100;
            removed += 2;
        }
        for (;;)
        {
            const uint64_t vp_d10 = vp / 10;
            const uint64_t vm_d10 = vm / 10;
            if (vp_d10 <= vm_d10) break;
            const uint64_t vr_d10 = vr / 10;
            const uint64_t vr_m10 = vr - 10 * vr_d10;
            round_up = (vr_m10 >= 5);
            vr = vr_d10;  vp = vp_d10;  vm = vm_d10;
            ++removed;
        }
        output = vr + (vr == vm || round_up);
    }

    floating_decimal_64 fd;
    fd.exponent = e10 + removed;
    fd.mantissa = output;
    return fd;
}


////////////////////////////////////////////////////////////////////////////////
// Two-digit lookup table for fast decimal printing (e.g. 47 -> "47").
inline const char* digit_pairs()
{
    static const char DIGIT_PAIRS[201] =
        "00010203040506070809"
        "10111213141516171819"
        "20212223242526272829"
        "30313233343536373839"
        "40414243444546474849"
        "50515253545556575859"
        "60616263646566676869"
        "70717273747576777879"
        "80818283848586878889"
        "90919293949596979899";
    return DIGIT_PAIRS;
}


////////////////////////////////////////////////////////////////////////////////
// Decimal digit count of a 64-bit unsigned integer.
inline uint32_t decimal_length(uint64_t v)
{
    static const uint64_t pow10[] = {
        1ULL, 10ULL, 100ULL, 1000ULL, 10000ULL, 100000ULL,
        1000000ULL, 10000000ULL, 100000000ULL, 1000000000ULL,
        10000000000ULL, 100000000000ULL, 1000000000000ULL,
        10000000000000ULL, 100000000000000ULL, 1000000000000000ULL,
        10000000000000000ULL, 100000000000000000ULL,
        1000000000000000000ULL, 10000000000000000000ULL };
    for (uint32_t i = 1; i < 20; ++i)
        if (v < pow10[i]) return i;
    return 20;
}


////////////////////////////////////////////////////////////////////////////////
// Format (mantissa, exponent) into a scientific-notation string.
// Layout: [-]d.dddE[-]nn  (exponent uses scientific decimal).
// Returns the number of characters written (no NUL terminator).
inline int to_chars(floating_decimal_64 v, bool sign, char* result)
{
    int index = 0;
    if (sign) result[index++] = '-';

    uint64_t  output = v.mantissa;
    const uint32_t olen = decimal_length(output);

    // Write digits least-significant-first into a temporary, then copy
    // to the output reversed in the order needed for "d.ddddd".
    char buf[24];
    uint32_t i = 0;
    const char* dp = digit_pairs();
    while (output >= 10000)
    {
        const uint32_t c   = (uint32_t)(output % 10000);
        output            /= 10000;
        const uint32_t c0  = (c % 100) << 1;
        const uint32_t c1  = (c / 100) << 1;
        buf[i    ] = dp[c0 + 1];
        buf[i + 1] = dp[c0    ];
        buf[i + 2] = dp[c1 + 1];
        buf[i + 3] = dp[c1    ];
        i += 4;
    }
    if (output >= 100)
    {
        const uint32_t c = (uint32_t)((output % 100) << 1);
        output /= 100;
        buf[i    ] = dp[c + 1];
        buf[i + 1] = dp[c    ];
        i += 2;
    }
    if (output >= 10)
    {
        const uint32_t c = (uint32_t)(output << 1);
        buf[i    ] = dp[c + 1];
        buf[i + 1] = dp[c    ];
        i += 2;
    }
    else
    {
        buf[i++] = (char)('0' + (uint32_t)output);
    }

    // buf now holds the digits least-significant-first (so reversed).
    // Emit "d.ddddd"
    result[index] = buf[i - 1];
    if (olen > 1)
    {
        result[index + 1] = '.';
        for (uint32_t k = 1; k < olen; ++k)
            result[index + 1 + k] = buf[i - 1 - k];
        index += olen + 1;
    }
    else
    {
        ++index;
    }

    // Emit exponent: 'E' [+/-] digits
    result[index++] = 'E';
    int32_t exp = v.exponent + (int32_t)olen - 1;
    if (exp < 0) { result[index++] = '-'; exp = -exp; }
    if (exp >= 100)
    {
        result[index    ] = (char)('0' + (exp / 100));
        result[index + 1] = (char)('0' + (exp / 10) % 10);
        result[index + 2] = (char)('0' + (exp % 10));
        index += 3;
    }
    else if (exp >= 10)
    {
        result[index    ] = (char)('0' + (exp / 10));
        result[index + 1] = (char)('0' + (exp % 10));
        index += 2;
    }
    else
    {
        result[index++] = (char)('0' + exp);
    }
    return index;
}


////////////////////////////////////////////////////////////////////////////////
// Special string for NaN / +/-Inf / +/-0.
inline int copy_special(bool sign, bool exponent_all_one, bool mantissa_zero, char* result)
{
    if (exponent_all_one && !mantissa_zero)
    {
        memcpy(result, "NaN", 3);
        return 3;
    }
    int idx = 0;
    if (sign) result[idx++] = '-';
    if (exponent_all_one)
    {
        memcpy(result + idx, "Infinity", 8);
        return idx + 8;
    }
    // +/- 0
    memcpy(result + idx, "0E0", 3);
    return idx + 3;
}


////////////////////////////////////////////////////////////////////////////////
// Top-level: convert a double to its shortest scientific representation.
// 'result' must have at least 25 bytes; returns number of bytes written.
inline int d2s_buffered_n(double d, char* result)
{
    uint64_t bits;
    memcpy(&bits, &d, sizeof(bits));

    const bool     ieee_sign = (bits >> (DOUBLE_MANTISSA_BITS + DOUBLE_EXPONENT_BITS)) & 1;
    const uint64_t ieee_m    = bits & (((uint64_t)1 << DOUBLE_MANTISSA_BITS) - 1);
    const uint32_t ieee_e    = (uint32_t)((bits >> DOUBLE_MANTISSA_BITS)
                                          & (((uint32_t)1 << DOUBLE_EXPONENT_BITS) - 1));

    // Special cases.
    const bool exp_all_one = (ieee_e == ((1u << DOUBLE_EXPONENT_BITS) - 1u));
    if (exp_all_one || (ieee_e == 0 && ieee_m == 0))
        return copy_special(ieee_sign, exp_all_one, ieee_m == 0, result);

    floating_decimal_64 fd = d2d(ieee_m, ieee_e);
    return to_chars(fd, ieee_sign, result);
}


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(ryu_detail);
eco_namespace_end(eco);
