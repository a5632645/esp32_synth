#pragma once

#include "table.h"

#ifdef __cplusplus
    #include <cstdint>
    #define MY_CONSTEXPR constexpr
#else // __cplusplus
    #include <stdint.h>
    #define MY_CONSTEXPR
#endif // !__cplusplus

#ifdef __cplusplus
extern "C" {
#endif

typedef int16_t MyFpS0_15;
typedef int16_t MyFpS1_14;
typedef int32_t MyFpS7_24;

inline static MY_CONSTEXPR MyFpS1_14 MyFpS1_14_FromFloat(float x) {
    return (int16_t)(x * (1 << 14));
}
inline static MY_CONSTEXPR float MyFpS1_14_ToFloat(MyFpS1_14 x) {
    return x * (1.0f / (1 << 14));
}

inline static MY_CONSTEXPR MyFpS0_15 MyFpS0_15_FromFloat(float x) {
    return (int16_t)(x * (1 << 15));
}
inline static MY_CONSTEXPR float MyFpS0_15_ToFloat(MyFpS0_15 x) {
    return x * (1.0f / (1 << 15));
}

typedef struct {
    int16_t s16[8];
} __attribute__((aligned(16))) Vec128Struct;

/**
 * @brief 
 * @param x -1 ~ 1, -16384 ~ 16384, -pi ~ pi
 * @return -16384 ~ 16384
 */
inline static MY_CONSTEXPR MyFpS1_14 Phase_FpSin(uint16_t x) {
     auto sign = x & 0b1000000000000000u;
     return sign | phase_sin_table[x & 16383u];
}

/**
 * @brief 
 * @param x -1 ~ 1, -16384 ~ 16384, -pi ~ pi
 * @return -16384 ~ 16384
 */
inline static MY_CONSTEXPR MyFpS1_14 Phase_FpCos(uint16_t x) {
    return Phase_FpSin(x + 8191u);
}

/**
 * @brief 
 * @param x 0 ~ 1, 0 ~ 32768, 0 ~ pi / 2
 * @return 
 */
inline static MY_CONSTEXPR MyFpS0_15 Freq_FpSin(uint16_t x) {
    auto sign = x & 0b1000000000000000u;
    return sign | freq_sin_table[x & 32767u];
}

/**
 * @brief 
 * @param x -1 ~ 1, -32768 ~ 32768, -pi ~ pi
 * @return 
 */
inline static MY_CONSTEXPR MyFpS0_15 Freq_FpCos(uint16_t x) {
    return Freq_FpSin(x + 16383u);
}

/**
 * @brief 
 * @param x -1 ~ 1, -16384 ~ 16384 
 * @return 
 */
inline static MY_CONSTEXPR MyFpS1_14 Sin2Cos(MyFpS1_14 x) {
    uint16_t a = x;
    a &= 0b0111111111111111; // remove sign
    if (x > 16383)
        return 0;

    return sin2cos_table[a];
}

#ifdef __cplusplus
}
#endif