#include <cstdint>
#include <cassert>
#include <cmath>
#include <stddef.h>

// Assertion macro
#define RJ_ASSERT(condition) assert(condition)

// Boolean types
typedef bool		tB;

// Character types
typedef char		tC8;

// Unsigned integer types
typedef uint8_t		tU8;
typedef uint16_t	tU16;
typedef uint32_t	tU32;
typedef uint64_t	tU64;

// Signed integer types
typedef int8_t		tS8;
typedef int16_t		tS16;
typedef int32_t		tS32;
typedef int64_t		tS64;

// Floating point types
typedef float		tF32;
typedef double		tF64;

// Size types
typedef size_t		tSize;
typedef ptrdiff_t	tPtrDiff;

//******************************************************************************
// Maximum number of 32 bit blocks needed in high precision arithmetic
// to print out 64 bit IEEE floating point values.
//******************************************************************************
const tU32 c_BigInt_MaxBlocks = 35;

//******************************************************************************
// This structure stores a high precision unsigned integer. It uses a buffer
// of 32 bit integer blocks along with a length. The lowest bits of the integer
// are stored at the start of the buffer and the length is set to the minimum
// value that contains the integer. Thus, there are never any zero blocks at the
// end of the buffer.
//******************************************************************************
struct tBigInt
{
    // Copy integer
    tBigInt & operator=(const tBigInt &rhs)
    {
        tU32 length = rhs.m_length;
        tU32 * pLhsCur = m_blocks;
        for (const tU32 *pRhsCur = rhs.m_blocks, *pRhsEnd = pRhsCur + length;
             pRhsCur != pRhsEnd;
             ++pLhsCur, ++pRhsCur)
        {
            *pLhsCur = *pRhsCur;
        }
        m_length = length;
        return *this;
    }

    // Data accessors
    tU32 GetLength() const        { return m_length; }
    tU32 GetBlock(tU32 idx) const { return m_blocks[idx]; }

    // Zero helper functions
    void	SetZero()		{ m_length = 0; }
    tB		IsZero() const	{ return m_length == 0; }

    // Basic type accessors
    void SetU64(tU64 val)
    {
        if (val > 0xFFFFFFFF)
        {
            m_blocks[0] = val & 0xFFFFFFFF;
            m_blocks[1] = (val >> 32) & 0xFFFFFFFF;
            m_length    = 2;
        }
        else if (val != 0)
        {
            m_blocks[0] = val & 0xFFFFFFFF;
            m_length    = 1;
        }
        else
        {
            m_length = 0;
        }
    }

    void SetU32(tU32 val)
    {
        if (val != 0)
        {
            m_blocks[0] = val;
            m_length    = (val != 0);
        }
        else
        {
            m_length = 0;
        }
    }

    tU32 GetU32() const { return (m_length == 0) ? 0 : m_blocks[0]; }

    // Member data
    tU32 m_length;
    tU32 m_blocks[c_BigInt_MaxBlocks];
};

//******************************************************************************
// This function will divide two large numbers under the assumption that the
// result is within the range [0,10) and the input numbers have been shifted
// to satisfy:
// - The highest block of the divisor is greater than or equal to 8 such that
//   there is enough precision to make an accurate first guess at the quotient.
// - The highest block of the divisor is less than the maximum value on an
//   unsigned 32-bit integer such that we can safely increment without overflow.
// - The dividend does not contain more blocks than the divisor such that we
//   can estimate the quotient by dividing the equivalently placed high blocks.
//
// quotient  = floor(dividend / divisor)
// remainder = dividend - quotient*divisor
//
// pDividend is updated to be the remainder and the quotient is returned.
//******************************************************************************
static tU32 BigInt_DivideWithRemainder_MaxQuotient9(tBigInt * pDividend, const tBigInt & divisor)
{
    // Check that the divisor has been correctly shifted into range and that it is not
    // smaller than the dividend in length.
    RJ_ASSERT(	!divisor.IsZero() &&
                divisor.m_blocks[divisor.m_length-1] >= 8 &&
                divisor.m_blocks[divisor.m_length-1] < 0xFFFFFFFF &&
                pDividend->m_length <= divisor.m_length );

    // If the dividend is smaller than the divisor, the quotient is zero and the divisor is already
    // the remainder.
    tU32 length = divisor.m_length;
    if (pDividend->m_length < divisor.m_length)
        return 0;

    const tU32 * pFinalDivisorBlock	 = divisor.m_blocks + length - 1;			
    tU32 *       pFinalDividendBlock = pDividend->m_blocks + length - 1;

    // Compute an estimated quotient based on the high block value. This will either match the actual quotient or
    // undershoot by one.
    tU32  quotient = *pFinalDividendBlock / (*pFinalDivisorBlock + 1);
    RJ_ASSERT(quotient <= 9);

    // Divide out the estimated quotient
    if (quotient != 0)
    {
        // dividend = dividend - divisor*quotient
        const tU32 *pDivisorCur	= divisor.m_blocks;
        tU32 *pDividendCur		= pDividend->m_blocks;

        tU64 borrow = 0;
        tU64 carry = 0;
        do
        {
            tU64 product = (tU64)*pDivisorCur * (tU64)quotient + carry;
            carry = product >> 32;

            tU64 difference = (tU64)*pDividendCur - (product & 0xFFFFFFFF) - borrow;
            borrow = (difference >> 32) & 1;
            
            *pDividendCur = difference & 0xFFFFFFFF;

            ++pDivisorCur;
            ++pDividendCur;
        } while(pDivisorCur <= pFinalDivisorBlock);

        // remove all leading zero blocks from dividend
        while (length > 0 && pDividend->m_blocks[length - 1] == 0)
            --length;
        
        pDividend->m_length = length;
    }

    // If the dividend is still larger than the divisor, we overshot our estimate quotient. To correct,
    // we increment the quotient and subtract one more divisor from the dividend.
    if ( BigInt_Compare(*pDividend, divisor) >= 0 )
    {
        ++quotient;

        // dividend = dividend - divisor
        const tU32 *pDivisorCur	= divisor.m_blocks;
        tU32 *pDividendCur		= pDividend->m_blocks;

        tU64 borrow = 0;
        do 
        {
            tU64 difference = (tU64)*pDividendCur - (tU64)*pDivisorCur - borrow;
            borrow = (difference >> 32) & 1;

            *pDividendCur = difference & 0xFFFFFFFF;

            ++pDivisorCur;
            ++pDividendCur;
        } while(pDivisorCur <= pFinalDivisorBlock);

        // remove all leading zero blocks from dividend
        while (length > 0 && pDividend->m_blocks[length - 1] == 0)
            --length;
        
        pDividend->m_length = length;
    }

    return quotient;
}


//******************************************************************************
// This is an implementation the Dragon4 algorithm to convert a binary number
// in floating point format to a decimal number in string format. The function
// returns the number of digits written to the output buffer and the output is
// not NUL terminated.
//
// The floating point input value is (mantissa * 2^exponent).
//
// See the following papers for more information on the algorithm:
//  "How to Print Floating-Point Numbers Accurately"
//    Steele and White
//    http://kurtstephens.com/files/p372-steele.pdf
//  "Printing Floating-Point Numbers Quickly and Accurately"
//    Burger and Dybvig
//    http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.72.4656&rep=rep1&type=pdf
//******************************************************************************
tU32 Dragon4 (
    const tU64			mantissa,			// value significand
    const tS32			exponent,			// value exponent in base 2
    const tU32			mantissaHighBitIdx,	// index of the highest set mantissa bit
    const tB			hasUnequalMargins,	// is the high margin twice as large as the low margin
    const tCutoffMode	cutoffMode,			// how to determine output length
    tU32				cutoffNumber,		// parameter to the selected cutoffMode
    tC8 *				pOutBuffer,			// buffer to output into
    tU32				bufferSize,			// maximum characters that can be printed to pOutBuffer
    tS32 *				pOutExponent		// the base 10 exponent of the first digit
)
{
    tC8 * pCurDigit = pOutBuffer;

    RJ_ASSERT( bufferSize > 0 );

    // if the mantissa is zero, the value is zero regardless of the exponent
    if (mantissa == 0)
    {
        *pCurDigit = '0';
        *pOutExponent = 0;
        return 1;
    }

    // compute the initial state in integral form such that 
    //  value     = scaledValue / scale
    //  marginLow = scaledMarginLow / scale
    tBigInt scale;				// positive scale applied to value and margin such that they can be
                                //  represented as whole numbers
    tBigInt scaledValue;		// scale * mantissa
    tBigInt scaledMarginLow;	// scale * 0.5 * (distance between this floating-point number and its
                                //  immediate lower value)

    // For normalized IEEE floating point values, each time the exponent is incremented the margin also
    // doubles. That creates a subset of transition numbers where the high margin is twice the size of
    // the low margin.
    tBigInt * pScaledMarginHigh;
    tBigInt optionalMarginHigh;

    if ( hasUnequalMargins )
    {
        // if we have no fractional component
        if (exponent > 0)
        {
            // 1) Expand the input value by multiplying out the mantissa and exponent. This represents
            //    the input value in its whole number representation.
            // 2) Apply an additional scale of 2 such that later comparisons against the margin values
            //    are simplified.
            // 3) Set the margin value to the lowest mantissa bit's scale.
            
            // scaledValue      = 2 * 2 * mantissa*2^exponent
            scaledValue.SetU64( 4 * mantissa );
            BigInt_ShiftLeft( &scaledValue, exponent );
            
            // scale            = 2 * 2 * 1
            scale.SetU32( 4 );
            
            // scaledMarginLow  = 2 * 2^(exponent-1)
            BigInt_Pow2( &scaledMarginLow, exponent );

            // scaledMarginHigh = 2 * 2 * 2^(exponent-1)
            BigInt_Pow2( &optionalMarginHigh, exponent + 1 );
        }
        // else we have a fractional exponent
        else
        {
            // In order to track the mantissa data as an integer, we store it as is with a large scale
            
            // scaledValue      = 2 * 2 * mantissa
            scaledValue.SetU64( 4 * mantissa );	
            
            // scale            = 2 * 2 * 2^(-exponent)
            BigInt_Pow2(&scale, -exponent + 2 );
            
            // scaledMarginLow  = 2 * 2^(-1)
            scaledMarginLow.SetU32( 1 );
            
            // scaledMarginHigh = 2 * 2 * 2^(-1)
            optionalMarginHigh.SetU32( 2 );
        }

        // the high and low margins are different
        pScaledMarginHigh = &optionalMarginHigh;
    }
    else
    {
        // if we have no fractional component
        if (exponent > 0)
        {
            // 1) Expand the input value by multiplying out the mantissa and exponent. This represents
            //    the input value in its whole number representation.
            // 2) Apply an additional scale of 2 such that later comparisons against the margin values
            //    are simplified.
            // 3) Set the margin value to the lowest mantissa bit's scale.
            
            // scaledValue     = 2 * mantissa*2^exponent
            scaledValue.SetU64( 2 * mantissa );
            BigInt_ShiftLeft( &scaledValue, exponent );
            
            // scale           = 2 * 1
            scale.SetU32( 2 );

            // scaledMarginLow = 2 * 2^(exponent-1)
            BigInt_Pow2( &scaledMarginLow, exponent );
        }
        // else we have a fractional exponent
        else
        {
            // In order to track the mantissa data as an integer, we store it as is with a large scale

            // scaledValue     = 2 * mantissa
            scaledValue.SetU64( 2 * mantissa );
            
            // scale           = 2 * 2^(-exponent)
            BigInt_Pow2(&scale, -exponent + 1 );

            // scaledMarginLow = 2 * 2^(-1)
            scaledMarginLow.SetU32( 1 );
        }
    
        // the high and low margins are equal
        pScaledMarginHigh = &scaledMarginLow;
    }

    // Compute an estimate for digitExponent that will be correct or undershoot by one.
    // This optimization is based on the paper "Printing Floating-Point Numbers Quickly and Accurately"
    // by Burger and Dybvig http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.72.4656&rep=rep1&type=pdf
    // We perform an additional subtraction of 0.69 to increase the frequency of a failed estimate
    // because that lets us take a faster branch in the code. 0.69 is chosen because 0.69 + log10(2) is
    // less than one by a reasonable epsilon that will account for any floating point error.
    //
    // We want to set digitExponent to floor(log10(v)) + 1
    //  v = mantissa*2^exponent
    //  log2(v) = log2(mantissa) + exponent;
    //  log10(v) = log2(v) * log10(2)
    //  floor(log2(v)) = mantissaHighBitIdx + exponent;
    //  log10(v) - log10(2) < (mantissaHighBitIdx + exponent) * log10(2) <= log10(v)
    //  log10(v) < (mantissaHighBitIdx + exponent) * log10(2) + log10(2) <= log10(v) + log10(2)
    //  floor( log10(v) ) < ceil( (mantissaHighBitIdx + exponent) * log10(2) ) <= floor( log10(v) ) + 1
    const tF64 log10_2 = 0.30102999566398119521373889472449;
    tS32 digitExponent = (tS32)(ceil(tF64((tS32)mantissaHighBitIdx + exponent) * log10_2 - 0.69));

    // if the digit exponent is smaller than the smallest desired digit for fractional cutoff,
    // pull the digit back into legal range at which point we will round to the appropriate value.
    // Note that while our value for digitExponent is still an estimate, this is safe because it
    // only increases the number. This will either correct digitExponent to an accurate value or it
    // will clamp it above the accurate value.
    if (cutoffMode == CutoffMode_FractionLength && digitExponent <= -(tS32)cutoffNumber)
    {
        digitExponent = -(tS32)cutoffNumber + 1;
    }

    // Divide value by 10^digitExponent. 
    if (digitExponent > 0)
    {
        // The exponent is positive creating a division so we multiply up the scale.
        tBigInt temp;
        BigInt_MultiplyPow10( &temp, scale, digitExponent );
        scale = temp;
    }
    else if (digitExponent < 0)
    {
        // The exponent is negative creating a multiplication so we multiply up the scaledValue,
        // scaledMarginLow and scaledMarginHigh.
        tBigInt pow10;
        BigInt_Pow10( &pow10, -digitExponent);

        tBigInt temp;
        BigInt_Multiply( &temp, scaledValue, pow10);
        scaledValue = temp;

        BigInt_Multiply( &temp, scaledMarginLow, pow10);
        scaledMarginLow = temp;
        
        if (pScaledMarginHigh != &scaledMarginLow)
            BigInt_Multiply2( pScaledMarginHigh, scaledMarginLow );
    }

    // If (value >= 1), our estimate for digitExponent was too low
    if( BigInt_Compare(scaledValue,scale) >= 0 )
    {
        // The exponent estimate was incorrect.
        // Increment the exponent and don't perform the premultiply needed
        // for the first loop iteration.
        digitExponent = digitExponent + 1;
    }
    else
    {
        // The exponent estimate was correct.
        // Multiply larger by the output base to prepare for the first loop iteration.
        BigInt_Multiply10( &scaledValue );
        BigInt_Multiply10( &scaledMarginLow );
        if (pScaledMarginHigh != &scaledMarginLow)
            BigInt_Multiply2( pScaledMarginHigh, scaledMarginLow );
    }
    
    // Compute the cutoff exponent (the exponent of the final digit to print).
    // Default to the maximum size of the output buffer.
    tS32 cutoffExponent = digitExponent - bufferSize;
    switch(cutoffMode)
    {
    // print digits until we pass the accuracy margin limits or buffer size
    case CutoffMode_Unique:
        break;

    // print cutoffNumber of digits or until we reach the buffer size
    case CutoffMode_TotalLength:
        {
            tS32 desiredCutoffExponent = digitExponent - (tS32)cutoffNumber;
            if (desiredCutoffExponent > cutoffExponent)
                cutoffExponent = desiredCutoffExponent;
        }
        break;

    // print cutoffNumber digits past the decimal point or until we reach the buffer size
    case CutoffMode_FractionLength:
        {
            tS32 desiredCutoffExponent = -(tS32)cutoffNumber;
            if (desiredCutoffExponent > cutoffExponent)
                cutoffExponent = desiredCutoffExponent;
        }
        break;
    }

    // Output the exponent of the first digit we will print
    *pOutExponent = digitExponent-1;

    // In preparation for calling BigInt_DivideWithRemainder_MaxQuotient9(), 
    // we need to scale up our values such that the highest block of the denominator
    // is greater than or equal to 8. We also need to guarantee that the numerator
    // can never have a length greater than the denominator after each loop iteration.
    // This requires the highest block of the denominator to be less than or equal to
    // 429496729 which is the highest number that can be multiplied by 10 without
    // overflowing to a new block.
    RJ_ASSERT( scale.GetLength() > 0 );
    tU32 hiBlock = scale.GetBlock( scale.GetLength() - 1 );
    if (hiBlock < 8 || hiBlock > 429496729)
    {
        // Perform a bit shift on all values to get the highest block of the denominator into
        // the range [8,429496729]. We are more likely to make accurate quotient estimations
        // in BigInt_DivideWithRemainder_MaxQuotient9() with higher denominator values so
        // we shift the denominator to place the highest bit at index 27 of the highest block.
        // This is safe because (2^28 - 1) = 268435455 which is less than 429496729. This means
        // that all values with a highest bit at index 27 are within range.			
        tU32 hiBlockLog2 = LogBase2(hiBlock);
        RJ_ASSERT(hiBlockLog2 < 3 || hiBlockLog2 > 27);
        tU32 shift = (32 + 27 - hiBlockLog2) % 32;

        BigInt_ShiftLeft( &scale, shift );
        BigInt_ShiftLeft( &scaledValue, shift);
        BigInt_ShiftLeft( &scaledMarginLow, shift);
        if (pScaledMarginHigh != &scaledMarginLow)
            BigInt_Multiply2( pScaledMarginHigh, scaledMarginLow );
    }

    // These values are used to inspect why the print loop terminated so we can properly
    // round the final digit.
    tB		low;			// did the value get within marginLow distance from zero
    tB		high;			// did the value get within marginHigh distance from one
    tU32	outputDigit;	// current digit being output
    
    if (cutoffMode == CutoffMode_Unique)
    {
        // For the unique cutoff mode, we will try to print until we have reached a level of
        // precision that uniquely distinguishes this value from its neighbors. If we run
        // out of space in the output buffer, we terminate early.
        for (;;)
        {
            digitExponent = digitExponent-1;

            // divide out the scale to extract the digit
            outputDigit = BigInt_DivideWithRemainder_MaxQuotient9(&scaledValue, scale);
            RJ_ASSERT( outputDigit < 10 );

            // update the high end of the value
            tBigInt scaledValueHigh;
            BigInt_Add( &scaledValueHigh, scaledValue, *pScaledMarginHigh );

            // stop looping if we are far enough away from our neighboring values
            // or if we have reached the cutoff digit
            low = BigInt_Compare(scaledValue, scaledMarginLow) < 0;
            high = BigInt_Compare(scaledValueHigh, scale) > 0;
            if (low | high | (digitExponent == cutoffExponent))
                break;
            
            // store the output digit
            *pCurDigit = (tC8)('0' + outputDigit);
            ++pCurDigit;

            // multiply larger by the output base
            BigInt_Multiply10( &scaledValue );
            BigInt_Multiply10( &scaledMarginLow );
            if (pScaledMarginHigh != &scaledMarginLow)
                BigInt_Multiply2( pScaledMarginHigh, scaledMarginLow );					
        }
    }
    else
    {
        // For length based cutoff modes, we will try to print until we
        // have exhausted all precision (i.e. all remaining digits are zeros) or
        // until we reach the desired cutoff digit.
        low = false;
        high = false;

        for (;;)
        {
            digitExponent = digitExponent-1;

            // divide out the scale to extract the digit
            outputDigit = BigInt_DivideWithRemainder_MaxQuotient9(&scaledValue, scale);
            RJ_ASSERT( outputDigit < 10 );

            if ( scaledValue.IsZero() | (digitExponent == cutoffExponent) )
                break;

            // store the output digit
            *pCurDigit = (tC8)('0' + outputDigit);
            ++pCurDigit;

            // multiply larger by the output base
            BigInt_Multiply10(&scaledValue);
        }
    }

    // round off the final digit
    // default to rounding down if value got too close to 0
	tB roundDown = low;
    
    // if it is legal to round up and down
    if (low == high)
    {
        // round to the closest digit by comparing value with 0.5. To do this we need to convert
        // the inequality to large integer values.
        //  compare( value, 0.5 )
        //  compare( scale * value, scale * 0.5 )
        //  compare( 2 * scale * value, scale )
        BigInt_Multiply2(&scaledValue);
        tS32 compare = BigInt_Compare(scaledValue, scale);
        roundDown = compare < 0;
        
        // if we are directly in the middle, round towards the even digit (i.e. IEEE rouding rules)
        if (compare == 0)
            roundDown = (outputDigit & 1) == 0;				
    }

    // print the rounded digit
    if (roundDown)
    {
        *pCurDigit = (tC8)('0' + outputDigit);
        ++pCurDigit;
    }
    else
    {
        // handle rounding up
        if (outputDigit == 9)
        {
            // find the first non-nine prior digit
            for (;;)
            {
                // if we are at the first digit
                if (pCurDigit == pOutBuffer)
                {
                    // output 1 at the next highest exponent
                    *pCurDigit = '1';
                    ++pCurDigit;
                    *pOutExponent += 1;
                    break;
                }

                --pCurDigit;
                if (*pCurDigit != '9')
                {
                    // increment the digit
                    *pCurDigit += 1;
                    ++pCurDigit;
                    break;
                }
            }
        }
        else
        {
            // values in the range [0,8] can perform a simple round up
            *pCurDigit = (tC8)('0' + outputDigit + 1);
            ++pCurDigit;
        }
    }

    // return the number of digits output
    tU32 outputLen = (tU32)(pCurDigit - pOutBuffer);
    RJ_ASSERT(outputLen <= bufferSize);
    return outputLen;
}