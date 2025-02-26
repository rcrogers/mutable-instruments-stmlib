// Copyright 2012 Emilie Gillet.
//
// Author: Emilie Gillet (emilie.o.gillet@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// 
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------
//
// DSP utility routines.

#ifndef STMLIB_UTILS_DSP_DSP_H_
#define STMLIB_UTILS_DSP_DSP_H_

#include "stmlib/stmlib.h"

#include <cmath>
#include <math.h>

namespace stmlib {

#define MAKE_INTEGRAL_FRACTIONAL(x) \
  int32_t x ## _integral = static_cast<int32_t>(x); \
  float x ## _fractional = x - static_cast<float>(x ## _integral);

inline float Interpolate(const float* table, float index, float size) {
  index *= size;
  MAKE_INTEGRAL_FRACTIONAL(index)
  float a = table[index_integral];
  float b = table[index_integral + 1];
  return a + (b - a) * index_fractional;
}


inline float InterpolateHermite(const float* table, float index, float size) {
  index *= size;
  MAKE_INTEGRAL_FRACTIONAL(index)
  const float xm1 = table[index_integral - 1];
  const float x0 = table[index_integral + 0];
  const float x1 = table[index_integral + 1];
  const float x2 = table[index_integral + 2];
  const float c = (x1 - xm1) * 0.5f;
  const float v = x0 - x1;
  const float w = c + v;
  const float a = w + v + (x2 - x0) * 0.5f;
  const float b_neg = w + a;
  const float f = index_fractional;
  return (((a * f) - b_neg) * f + c) * f + x0;
}

inline float InterpolateWrap(const float* table, float index, float size) {
  index -= static_cast<float>(static_cast<int32_t>(index));
  index *= size;
  MAKE_INTEGRAL_FRACTIONAL(index)
  float a = table[index_integral];
  float b = table[index_integral + 1];
  return a + (b - a) * index_fractional;
}

inline float SmoothStep(float value) {
  return value * value * (3.0f - 2.0f * value);
}

#define ONE_POLE(out, in, coefficient) out += (coefficient) * ((in) - out);
#define SLOPE(out, in, positive, negative) { \
  float error = (in) - out; \
  out += (error > 0 ? positive : negative) * error; \
}
#define SLEW(out, in, delta) { \
  float error = (in) - out; \
  float d = (delta); \
  if (error > d) { \
    error = d; \
  } else if (error < -d) { \
    error = -d; \
  } \
  out += error; \
}

inline float Crossfade(float a, float b, float fade) {
  return a + (b - a) * fade;
}

inline float SoftLimit(float x) {
  return x * (27.0f + x * x) / (27.0f + 9.0f * x * x);
}

inline float SoftClip(float x) {
  if (x < -3.0f) {
    return -1.0f;
  } else if (x > 3.0f) {
    return 1.0f;
  } else {
    return SoftLimit(x);
  }
}

#ifdef TEST
  inline int32_t Clip16(int32_t x) {
    if (x < -32768) {
      return -32768;
    } else if (x > 32767) {
      return 32767;
    } else {
      return x;
    }
  }
  inline uint16_t ClipU16(int32_t x) {
    if (x < 0) {
      return 0;
    } else if (x > 65535) {
      return 65535;
    } else {
      return x;
    }
  }
#else
  inline int32_t Clip16(int32_t x) {
    int32_t result;
    __asm ("ssat %0, %1, %2" : "=r" (result) :  "I" (16), "r" (x) );
    return result;
  }
  inline uint32_t ClipU16(int32_t x) {
    uint32_t result;
    __asm ("usat %0, %1, %2" : "=r" (result) :  "I" (16), "r" (x) );
    return result;
  }
#endif
  
#ifdef TEST
  inline float Sqrt(float x) {
    return sqrtf(x);
  }
#else
  inline float Sqrt(float x) {
    float result;
    __asm ("vsqrt.f32 %0, %1" : "=w" (result) : "w" (x) );
    return result;
  }
#endif

inline int16_t SoftConvert(float x) {
  return Clip16(static_cast<int32_t>(SoftLimit(x * 0.5f) * 32768.0f));
}

#define Q15_SHIFT 15

// Macro to unpack, multiply two Q15 values, and store the result
#define Q15_MULT_PAIR(a_pair, b_pair, result_ptr) do {              \
  int16_t a0 = (int16_t)((a_pair) & 0xFFFF);                        \
  int16_t a1 = (int16_t)(((a_pair) >> 16) & 0xFFFF);                \
  int16_t b0 = (int16_t)((b_pair) & 0xFFFF);                        \
  int16_t b1 = (int16_t)(((b_pair) >> 16) & 0xFFFF);                \
  int32_t result0 = ((int32_t)a0 * b0) >> Q15_SHIFT;                \
  int32_t result1 = ((int32_t)a1 * b1) >> Q15_SHIFT;                \
  *(int32_t*)(result_ptr) = (result1 << 16) | (result0 & 0xFFFF);   \
  result_ptr += 2;                                                  \
} while (0)

// Seems slower
// *result_ptr++ = (int16_t)(result0 & 0xFFFF);                      
// *result_ptr++ = (int16_t)(result1 & 0xFFFF);                      

// Multiply two arrays of Q15 values
template<int Length>
inline void q15_mult(int16_t* a, int16_t* b, int16_t* result) {
  STATIC_ASSERT(Length % 4 == 0, not_multiple_of_4);
  int count = Length / 4; // Pre-divide the length by 4
  while (count--) {
    // Load two pairs of Q15 values from arrays 'a' and 'b'
    int32_t a_pair1 = *(int32_t*)a;         // Load a[0] and a[1]
    int32_t a_pair2 = *(int32_t*)(a + 2);   // Load a[2] and a[3]
    int32_t b_pair1 = *(int32_t*)b;         // Load b[0] and b[1]
    int32_t b_pair2 = *(int32_t*)(b + 2);   // Load b[2] and b[3]

    // Perform the multiplication and store the results using the macro
    Q15_MULT_PAIR(a_pair1, b_pair1, result);
    Q15_MULT_PAIR(a_pair2, b_pair2, result);

    // Increment the input pointers as well
    a += 4;
    b += 4;
  }
}

template<int Length>
inline void add_q15(int16_t* a, int16_t* b, int16_t* result) {
  for (int i = 0; i < Length; i++) {
    int32_t sum = (int32_t)a[i] + b[i];
    result[i] = Clip16(sum);
  }
}

/**
 * @brief Multiply two Q15 vectors
 * @param[in]  pSrcA   pointer to first input vector
 * @param[in]  pSrcB   pointer to second input vector
 * @param[out] pDst    pointer to output vector
 * @param[in]  blockSize number of samples in each vector
 */
// void ersatz_mult_q15(
//     const int16_t * pSrcA,
//     const int16_t * pSrcB,
//     int16_t * pDst,
//     uint32_t blockSize)
// {
//     uint32_t blkCnt;                               /* Loop counter */
//     int32_t result;                                /* Temporary result storage */

//     /* Loop unrolling: Compute 4 outputs at a time */
//     blkCnt = blockSize >> 2U;

//     while (blkCnt > 0U)
//     {
//         /* C = A * B */
//         /* Multiply inputs and store result in temporary variable */
//         result = ((int32_t)(*pSrcA++) * (*pSrcB++)) >> 15;
//         /* Saturate result to 16-bit */
//         if (result > 32767) result = 32767;
//         if (result < -32768) result = -32768;
//         *pDst++ = (int16_t)result;

//         result = ((int32_t)(*pSrcA++) * (*pSrcB++)) >> 15;
//         if (result > 32767) result = 32767;
//         if (result < -32768) result = -32768;
//         *pDst++ = (int16_t)result;

//         result = ((int32_t)(*pSrcA++) * (*pSrcB++)) >> 15;
//         if (result > 32767) result = 32767;
//         if (result < -32768) result = -32768;
//         *pDst++ = (int16_t)result;

//         result = ((int32_t)(*pSrcA++) * (*pSrcB++)) >> 15;
//         if (result > 32767) result = 32767;
//         if (result < -32768) result = -32768;
//         *pDst++ = (int16_t)result;

//         /* Decrement loop counter */
//         blkCnt--;
//     }

//     /* Loop unrolling: Compute remaining outputs */
//     blkCnt = blockSize % 0x4U;

//     while (blkCnt > 0U)
//     {
//         /* C = A * B */
//         /* Multiply inputs and store result in temporary variable */
//         result = ((int32_t)(*pSrcA++) * (*pSrcB++)) >> 15;
//         /* Saturate result to 16-bit */
//         if (result > 32767) result = 32767;
//         if (result < -32768) result = -32768;
//         *pDst++ = (int16_t)result;

//         /* Decrement loop counter */
//         blkCnt--;
//     }
// }

}  // namespace stmlib

#endif  // STMLIB_UTILS_DSP_DSP_H_