#pragma once

#include "table.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 
 * @param x -1 ~ 1, -32768 ~ 32768, -pi ~ pi
 * @return -32768 ~ 32768
 */
inline static int16_t Phase_FpSin(int16_t x) {
    uint16_t a = x;
     auto sign = a & 0b1000000000000000;
     return sign | phase_sin_table[a & 32767];
}

/**
 * @brief 
 * @param x -1 ~ 1, -32768 ~ 32768, -pi ~ pi
 * @return -32768 ~ 32768
 */
inline static int16_t Phase_FpCos(int16_t x) {
    return Phase_FpSin(x + 16384);
}

/**
 * @brief 
 * @param x 0 ~ 1, 0 ~ 32768, 0 ~ pi / 2
 * @return 
 */
inline static int16_t Freq_FpSin(int16_t x) {
    uint16_t a = x;
    return freq_sin_table[a & 32767];
}

/**
 * @brief 
 * @param x -1 ~ 1, -32768 ~ 32768, -pi ~ pi
 * @return 
 */
inline static int16_t Freq_FpCos(int16_t x) {
    return Freq_FpSin(x + 16384);
}

/**
 * @brief 
 * @param x -1 ~ 1, -32768 ~ 32768 
 * @return 
 */
inline static int16_t Sin2Cos(int16_t x) {
    if (x == -32768)
        return 0;

    uint16_t a = x;
    return sin2cos_table[a & 32767];
}

#ifdef __cplusplus
}
#endif