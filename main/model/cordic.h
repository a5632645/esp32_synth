#pragma once

#ifdef __cplusplus
#define MY_CONSTEXPR constexpr
#else
#define MY_CONSTEXPR
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <math.h>
#include <string.h>
#include "table.h"

typedef int16_t MyFpS0_15;
typedef int16_t MyFpS1_14;

/**
 * @brief 
 * @param x -1 ~ 1, -16384 ~ 16384, -pi ~ pi
 * @return -16384 ~ 16384
 */
inline static MY_CONSTEXPR MyFpS1_14 Phase_FpSin(MyFpS1_14 x) {
    uint16_t a = x;
     auto sign = a & 0b1000000000000000;
     return sign | phase_sin_table[a & 16383];
}

/**
 * @brief 
 * @param x -1 ~ 1, -16384 ~ 16384, -pi ~ pi
 * @return -16384 ~ 16384
 */
inline static MY_CONSTEXPR MyFpS1_14 Phase_FpCos(MyFpS1_14 x) {
    return Phase_FpSin(x + 8192);
}

/**
 * @brief 
 * @param x 0 ~ 1, 0 ~ 32768, 0 ~ pi / 2
 * @return 
 */
inline static MY_CONSTEXPR MyFpS0_15 Freq_FpSin(MyFpS0_15 x) {
    uint16_t a = x;
    return freq_sin_table[a & 32767];
}

/**
 * @brief 
 * @param x -1 ~ 1, -32768 ~ 32768, -pi ~ pi
 * @return 
 */
inline static MY_CONSTEXPR MyFpS0_15 Freq_FpCos(MyFpS0_15 x) {
    return Freq_FpSin(x + 16384);
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

typedef struct {
    MyFpS1_14 x0[8];
    MyFpS1_14 old_y0[8];
    MyFpS1_14 y0[8];
    MyFpS0_15 half_coef[8];
} __attribute__((aligned(16))) CoridcData;

typedef struct {
    MyFpS1_14 phase[8]; // -1 ~ 1, -pi ~ pi, -16384 ~ 16384
    MyFpS0_15 freq[8];  //  0 ~ 1, 0 ~ pi / 2, 0 ~ 32768
} __attribute__((aligned(16))) CoridcParamStruct;

typedef struct {
    int16_t s16[8];
} __attribute__((aligned(16))) Vec128Struct;

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

inline static MY_CONSTEXPR void Coridc_Reset(CoridcData* ptr, uint32_t num, CoridcParamStruct* reset) {
    for (uint32_t i = 0; i < num; ++i) {
        ptr[i].y0[0] = Phase_FpSin(reset[i].phase[0]);
        ptr[i].y0[1] = Phase_FpSin(reset[i].phase[1]);
        ptr[i].y0[2] = Phase_FpSin(reset[i].phase[2]);
        ptr[i].y0[3] = Phase_FpSin(reset[i].phase[3]);
        ptr[i].y0[4] = Phase_FpSin(reset[i].phase[4]);
        ptr[i].y0[5] = Phase_FpSin(reset[i].phase[5]);
        ptr[i].y0[6] = Phase_FpSin(reset[i].phase[6]);
        ptr[i].y0[7] = Phase_FpSin(reset[i].phase[7]);
        
        ptr[i].x0[0] = Phase_FpCos(reset[i].phase[0] - (reset[i].freq[0] >> 1));
        ptr[i].x0[1] = Phase_FpCos(reset[i].phase[1] - (reset[i].freq[1] >> 1));
        ptr[i].x0[2] = Phase_FpCos(reset[i].phase[2] - (reset[i].freq[2] >> 1));
        ptr[i].x0[3] = Phase_FpCos(reset[i].phase[3] - (reset[i].freq[3] >> 1));
        ptr[i].x0[4] = Phase_FpCos(reset[i].phase[4] - (reset[i].freq[4] >> 1));
        ptr[i].x0[5] = Phase_FpCos(reset[i].phase[5] - (reset[i].freq[5] >> 1));
        ptr[i].x0[6] = Phase_FpCos(reset[i].phase[6] - (reset[i].freq[6] >> 1));
        ptr[i].x0[7] = Phase_FpCos(reset[i].phase[7] - (reset[i].freq[7] >> 1));

        ptr[i].half_coef[0] = Freq_FpSin(reset[i].freq[0]);
        ptr[i].half_coef[1] = Freq_FpSin(reset[i].freq[1]);
        ptr[i].half_coef[2] = Freq_FpSin(reset[i].freq[2]);
        ptr[i].half_coef[3] = Freq_FpSin(reset[i].freq[3]);
        ptr[i].half_coef[4] = Freq_FpSin(reset[i].freq[4]);
        ptr[i].half_coef[5] = Freq_FpSin(reset[i].freq[5]);
        ptr[i].half_coef[6] = Freq_FpSin(reset[i].freq[6]);
        ptr[i].half_coef[7] = Freq_FpSin(reset[i].freq[7]);
    }
}

inline static void Coridc_SetFreq(CoridcData* ptr, uint32_t len, CoridcParamStruct* freq) {
    Vec128Struct cos_p;
    Vec128Struct sin_w;
    Vec128Struct cos_w;
    for (uint32_t i = 0; i < len; ++i) {
        cos_p.s16[0] = ptr[i].y0[0] > ptr[i].old_y0[0] ? Sin2Cos(ptr[i].y0[0]) : -Sin2Cos(ptr[i].y0[0]);
        cos_p.s16[1] = ptr[i].y0[1] > ptr[i].old_y0[1] ? Sin2Cos(ptr[i].y0[1]) : -Sin2Cos(ptr[i].y0[1]);
        cos_p.s16[2] = ptr[i].y0[2] > ptr[i].old_y0[2] ? Sin2Cos(ptr[i].y0[2]) : -Sin2Cos(ptr[i].y0[2]);
        cos_p.s16[3] = ptr[i].y0[3] > ptr[i].old_y0[3] ? Sin2Cos(ptr[i].y0[3]) : -Sin2Cos(ptr[i].y0[3]);
        cos_p.s16[4] = ptr[i].y0[4] > ptr[i].old_y0[4] ? Sin2Cos(ptr[i].y0[4]) : -Sin2Cos(ptr[i].y0[4]);
        cos_p.s16[5] = ptr[i].y0[5] > ptr[i].old_y0[5] ? Sin2Cos(ptr[i].y0[5]) : -Sin2Cos(ptr[i].y0[5]);
        cos_p.s16[6] = ptr[i].y0[6] > ptr[i].old_y0[6] ? Sin2Cos(ptr[i].y0[6]) : -Sin2Cos(ptr[i].y0[6]);
        cos_p.s16[7] = ptr[i].y0[7] > ptr[i].old_y0[7] ? Sin2Cos(ptr[i].y0[7]) : -Sin2Cos(ptr[i].y0[7]);
        
        sin_w.s16[0] = Freq_FpSin(freq[i].freq[0]);
        sin_w.s16[1] = Freq_FpSin(freq[i].freq[1]);
        sin_w.s16[2] = Freq_FpSin(freq[i].freq[2]);
        sin_w.s16[3] = Freq_FpSin(freq[i].freq[3]);
        sin_w.s16[4] = Freq_FpSin(freq[i].freq[4]);
        sin_w.s16[5] = Freq_FpSin(freq[i].freq[5]);
        sin_w.s16[6] = Freq_FpSin(freq[i].freq[6]);
        sin_w.s16[7] = Freq_FpSin(freq[i].freq[7]);
        memcpy(ptr[i].half_coef, sin_w.s16, 8 * sizeof(uint16_t));

        cos_w.s16[0] = Freq_FpCos(freq[i].freq[0]);
        cos_w.s16[1] = Freq_FpCos(freq[i].freq[1]);
        cos_w.s16[2] = Freq_FpCos(freq[i].freq[2]);
        cos_w.s16[3] = Freq_FpCos(freq[i].freq[3]);
        cos_w.s16[4] = Freq_FpCos(freq[i].freq[4]);
        cos_w.s16[5] = Freq_FpCos(freq[i].freq[5]);
        cos_w.s16[6] = Freq_FpCos(freq[i].freq[6]);
        cos_w.s16[7] = Freq_FpCos(freq[i].freq[7]);

        asm volatile (
            "movi.n a5, 15\n\r"
            "wsr.sar a5\n\r"
            "addi %[p], %[p], 32\n\r"            // ptr = curr.y0
            "ee.vld.128.ip q1, %[p], -32\n\r"    // q1 <= curr.y0, ptr = curr.x0
            "ld.qr q2, %[cosp], 0\n\r"  // q2 <= cos_p
            "ld.qr q3, %[cosw], 0\n\r"  // q3 <= cosw
            "ld.qr q4, %[sinw], 0\n\r"  // q4 <= sinw
            "ee.vmul.s16 q0, q2, q3\n\r"
            "ee.vmul.s16 q3, q1, q4\n\r"
            "ee.vadds.s16 q2, q0, q3\n\r"
            "st.qr q2, %[p], 0"
            :
            :[p]"r"(ptr), [cosp]"r"(&cos_p), [cosw]"r"(&cos_w), [sinw]"r"(&sin_w)
            :"a5"
        );
        ++freq;
        ++ptr;
    }
}

typedef int32_t MyFpS7_24;
inline static void Coridc_Tick(CoridcData* ptr, MyFpS0_15* gain, uint32_t len, MyFpS7_24* out) {
    asm volatile (
        "movi.n a5, 14\n\r" // half_coeff * 2, >>15 turns into >>14
        "wsr.sar a5\n\r"
        "ee.zero.accx\n\r"

        ".dr_tick_loop:\n\r"
        "ee.vld.128.ip q0, %[p], 32\n\r"  // q0 <= curr.x0
        "ee.vld.128.ip q1, %[p], 16\n\r"  // q1 <= curr.y0
        "ee.vld.128.ip q2, %[p], -48\n\r"  // q2 <= half_coeff

        "ee.vmul.s16.ld.incp q4, %[g], q3, q2, q1\n\r"
        "ee.vmulas.s16.accx q4, q1\n\r"                 // accx += gain * y0
        "ee.vsubs.s16 q4, q0, q3\n\r"                   // q4 <= curr.x0 - half_coeff * curr.y0
        "ee.vmul.s16.st.incp q4, %[p], q3, q2, q4\n\r"  // save new_x0, ptr = curr.old_y0
        "ee.vadds.s16.st.incp q1, %[p], q4, q1, q3\n\r" // save old_y0, q4 <= new_y0
        "ee.vst.128.ip q4, %[p], 32\n\r"                // save new_y0, ptr = next.x0

        "addi %[num], %[num], -1\n\r"
        "bnez %[num], .dr_tick_loop\n\r"

        // accx contains float << 29
        "movi.n a5, 5\n\r"
        "ee.srs.accx a6, a5, 0\n\r" // sum <= a6
        "s32i.n a6, %[o], 0\n\r"
        :
        :[num]"r"(len), [p]"r"(ptr), [g]"r"(gain), [o]"r"(out)
        :"a5", "a6"
    );
}

#ifdef __cplusplus
}
#endif