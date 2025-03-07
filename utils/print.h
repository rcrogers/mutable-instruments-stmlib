// Thanks Claude!

#ifndef STMLIB_UTILS_PRINT_H_
#define STMLIB_UTILS_PRINT_H_

#include <stdint.h>
#include <stdbool.h>

#include "stmlib/stmlib.h"

namespace stmlib {

/**
 * Converts an int32_t to scientific notation and stores it in the provided buffer
 * 
 * Format will be: 1.234e+05 for 123400
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
    
    /* Check input parameters */
    if (buffer == NULL || bufferSize < 10) {
        return 0;  /* Buffer too small or NULL */
    }
    
    /* Handle special case of zero */
    if (number == 0) {
        if (bufferSize >= 10) {
            buffer[0] = '0';
            buffer[1] = '.';
            buffer[2] = '0';
            buffer[3] = 'e';
            buffer[4] = '+';
            buffer[5] = '0';
            buffer[6] = '0';
            buffer[7] = '\0';
            return 7;
        } else {
            return 0;
        }
    }
    
    /* Handle negative numbers */
    if (number < 0) {
        isNegative = true;
        /* Avoid overflow with INT32_MIN */
        if (number == INT32_MIN) {
            absValue = INT32_MAX;
            exponent = 10;  /* ~2.147e+09 */
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
        
        /* Write fractional digits (up to 2 for precision) */
        absValue %= divisor;
        divisor /= 10;
        
        while (divisor > 0 && digitCount < 2) {
            buffer[i++] = '0' + (absValue / divisor);
            absValue %= divisor;
            divisor /= 10;
            digitCount++;
        }
        
        /* Add trailing zeros for precision if needed */
        while (digitCount < 2) {
            buffer[i++] = '0';
            digitCount++;
        }
    }
    
    /* Write exponent */
    buffer[i++] = 'e';
    
    // if (exponent >= 0) {
    //     buffer[i++] = '+';
    // } else {
    //     buffer[i++] = '-';
    //     exponent = -exponent;
    // }
    
    /* Write at least 2 digits for exponent */
    if (exponent < 10) {
        buffer[i++] = '0';
    }
    
    /* Convert exponent to characters */
    if (exponent >= 100) {
        buffer[i++] = '0' + (exponent / 100);
        exponent %= 100;
    }
    
    if (exponent >= 10 || i > 0) {
        buffer[i++] = '0' + (exponent / 10);
        exponent %= 10;
    }
    
    buffer[i++] = '0' + exponent;
    
    /* Null terminate */
    buffer[i] = '\0';
    
    return i;
}

/* Example usage:
 * int32_t number = 123456;
 * char buffer[32];
 * int32ToScientificNotation(number, buffer, sizeof(buffer));
 */

}

#endif  // STMLIB_UTILS_PRINT_H_