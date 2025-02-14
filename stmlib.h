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

#ifndef STMLIB_STMLIB_H_
#define STMLIB_STMLIB_H_

#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>
#include <algorithm>
#include <limits>

#ifndef NULL
#define NULL 0
#endif

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)

#define CLIP(x) if (x < -32767) x = -32767; if (x > 32767) x = 32767;

#define CONSTRAIN(var, min, max) \
  if (var < (min)) { \
    var = (min); \
  } else if (var > (max)) { \
    var = (max); \
  }

// Always rounds toward -inf, for both positive and negative dividends
#define DIV_FLOOR(dividend, divisor) \
  ( \
    ((dividend) % (divisor)) \
      ? (((dividend) / (divisor)) - (((dividend) < 0) ^ ((divisor) < 0))) \
      : ((dividend) / (divisor)) \
  )

#define JOIN(lhs, rhs)    JOIN_1(lhs, rhs)
#define JOIN_1(lhs, rhs)  JOIN_2(lhs, rhs)
#define JOIN_2(lhs, rhs)  lhs##rhs

#define STATIC_ASSERT(expression, message)\
  struct JOIN(__static_assertion_at_line_, __LINE__)\
  {\
    impl::StaticAssertion<static_cast<bool>((expression))> JOIN(JOIN(JOIN(STATIC_ASSERTION_FAILED_AT_LINE_, __LINE__), _), message);\
  };\
  typedef impl::StaticAssertionTest<sizeof(JOIN(__static_assertion_at_line_, __LINE__))> JOIN(__static_assertion_test_at_line_, __LINE__)

namespace impl {

  template <bool>
  struct StaticAssertion;

  template <>
  struct StaticAssertion<true>
  {
  }; // StaticAssertion<true>

  template<int i>
  struct StaticAssertionTest
  {
  }; // StaticAssertionTest<int>

} // namespace impl


#ifndef TEST
#define IN_RAM __attribute__ ((section (".ramtext")))
#else
#define IN_RAM
#endif  // TEST

#define UNROLL2(x) x; x;
#define UNROLL4(x) x; x; x; x;
#define UNROLL8(x) x; x; x; x; x; x; x; x;

template<bool b>
inline void StaticAssertImplementation() {
	char static_assert_size_mismatch[b] = { 0 };
}
 
namespace stmlib {

typedef union {
  uint16_t value;
  uint8_t bytes[2];
} Word;

typedef union {
  uint32_t value;
  uint16_t words[2];
  uint8_t bytes[4];
} LongWord;


template<uint32_t a, uint32_t b, uint32_t c, uint32_t d>
struct FourCC {
  static const uint32_t value = (((((d << 8) | c) << 8) | b) << 8) | a;
};

// Returns the positive remainder
template<typename Dividend, typename Divisor>
inline Divisor modulo(Dividend dividend, Divisor divisor) {
  return (divisor + (dividend % divisor)) % divisor;
}

// Saturating add for any integer types
template<typename SumType, typename IncType>
inline SumType SaturatingIncrement(SumType num, IncType inc) {
  SumType result = num + inc;
  if (inc > 0) { // Check for overflow
    result = (result < num) ? std::numeric_limits<SumType>::max() : result;
  } else { // Check for underflow
    result = (result > num) ? std::numeric_limits<SumType>::min() : result;
  }
  return result;
}

inline uint8_t modulate_7bit(uint8_t init, int8_t scale, uint8_t mod) {
  int16_t result = init;
  result += (mod * scale) >> 6;
  CONSTRAIN(result, 0, 127);
  return result;
}

inline uint16_t modulate_7_13(uint8_t init, int8_t scale, uint8_t mod) {
  int16_t result = init << 6;
  result += mod * scale;
  CONSTRAIN(result, 0, (1 << 13) - 1);
  return result;
}

inline int32_t slew(int32_t current, int32_t target, uint8_t smoothing = 5) {
  int32_t delta = target - current;
  if (delta) {
    int32_t increment = delta >> smoothing;
    if (!increment) {
      increment = delta > 0 ? 1 : -1;
    }
    current += increment;
  }
  return current;
}

}  // namespace stmlib

#endif   // STMLIB_STMLIB_H_
