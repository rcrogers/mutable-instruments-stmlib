// Thanks Claude!

#ifndef STMLIB_UTILS_PRINT_H_
#define STMLIB_UTILS_PRINT_H_

#include <stdint.h>
#include <stdbool.h>

#include "stmlib/stmlib.h"

namespace stmlib {

/* Configuration constants */
#define SCI_DECIMAL_PLACES 2   /* Number of decimal places to show */
#define SCI_EXP_DIGITS 1       /* Number of exponent digits to show */

/**
 * Converts an int32_t to scientific notation and stores it in the provided buffer
 * 
 * Format will be: 1.23E3 for 1234
 * 
 * @param number The int32_t number to convert
 * @param buffer The character buffer to store the result
 * @param bufferSize The size of the buffer
 * @return Number of characters written (excluding null terminator)
 */
int int32E(int32_t number, char* buffer, uint16_t bufferSize) {
    int32_t absValue;
    int16_t exponent = 0;
    int16_t i = 0;
    bool isNegative = false;
    
    /* Minimum buffer size needed: 
       sign(1) + digit(1) + decimal(1) + decimal_places(SCI_DECIMAL_PLACES) + 
       e(1) + exp_digits(SCI_EXP_DIGITS) + null(1) */
    uint16_t minBufferSize = 1 + 1 + 1 + SCI_DECIMAL_PLACES + 1 + SCI_EXP_DIGITS + 1;
    
    /* Check input parameters */
    if (buffer == NULL || bufferSize < minBufferSize) {
        return 0;  /* Buffer too small or NULL */
    }
    
    /* Handle special case of zero */
    if (number == 0) {
        buffer[0] = '0';
        buffer[1] = '.';
        
        /* Add decimal zeros */
        for (i = 0; i < SCI_DECIMAL_PLACES; i++) {
            buffer[2 + i] = '0';
        }
        
        buffer[2 + SCI_DECIMAL_PLACES] = 'e';
        buffer[3 + SCI_DECIMAL_PLACES] = '0';
        buffer[4 + SCI_DECIMAL_PLACES] = '\0';
        
        return (4 + SCI_DECIMAL_PLACES);
    }
    
    /* Handle negative numbers */
    if (number < 0) {
        isNegative = true;
        /* Avoid overflow with INT32_MIN */
        if (number == INT32_MIN) {
            absValue = INT32_MAX;
            exponent = 9;  /* ~2.1E9 */
        } else {
            absValue = -number;
        }
    } else {
        absValue = number;
    }
    
    /* Count digits and compute exponent */
    {
        int32_t temp = absValue;
        while (temp >= 10) {
            temp /= 10;
            exponent++;
        }
    }
    
    /* Write sign if negative */
    if (isNegative) {
        buffer[i++] = '-';
    }
    
    /* Begin with the most significant digit */
    {
        int32_t divisor = 1;
        int16_t d;
        int16_t digitCount = 0;
        
        /* Calculate divisor for the first digit */
        for (d = 0; d < exponent; d++) {
            divisor *= 10;
        }
        
        /* Write first digit */
        buffer[i++] = '0' + (absValue / divisor);
        
        /* Write decimal point */
        buffer[i++] = '.';
        
        /* Write fractional digits (up to SCI_DECIMAL_PLACES for precision) */
        absValue %= divisor;
        divisor /= 10;
        
        while (divisor > 0 && digitCount < SCI_DECIMAL_PLACES) {
            buffer[i++] = '0' + (absValue / divisor);
            absValue %= divisor;
            divisor /= 10;
            digitCount++;
        }
        
        /* Add trailing zeros for precision if needed */
        while (digitCount < SCI_DECIMAL_PLACES) {
            buffer[i++] = '0';
            digitCount++;
        }
    }
    
    /* Write simplified exponent */
    buffer[i++] = 'e';
    
    /* Handle multi-digit exponents if SCI_EXP_DIGITS > 1 */
    if (SCI_EXP_DIGITS > 1) {
        int16_t expTemp = exponent;
        int16_t expDigits = 1;
        int16_t expDivisor = 1;
        
        /* Count digits in exponent */
        while (expTemp >= 10) {
            expTemp /= 10;
            expDigits++;
        }
        
        /* Add leading zeros if needed */
        for (int16_t j = expDigits; j < SCI_EXP_DIGITS; j++) {
            buffer[i++] = '0';
        }
        
        /* Calculate divisor for most significant exponent digit */
        for (int16_t j = 1; j < expDigits; j++) {
            expDivisor *= 10;
        }
        
        /* Write exponent digits */
        while (expDivisor > 0) {
            buffer[i++] = '0' + (exponent / expDivisor);
            exponent %= expDivisor;
            expDivisor /= 10;
        }
    } else {
        /* Just write the single exponent digit */
        buffer[i++] = '0' + exponent;
    }
    
    /* Null terminate */
    buffer[i] = '\0';
    
    return i;
}

/* Example usage:
 * int32_t number = 1234;
 * char buffer[32];
 * int32ToScientificNotation(number, buffer, sizeof(buffer));
 * Result: "1.23E3"
 */

}

#endif  // STMLIB_UTILS_PRINT_H_