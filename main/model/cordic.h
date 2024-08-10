#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <math.h>
#include "table_helper.h"

typedef struct {
    int16_t x0[8];
    int16_t y0[8];
    int16_t half_coef[8];
} __attribute__((aligned(16))) CoridcData;

typedef struct {
    int16_t phase[8]; // -1 ~ 1, -pi ~ pi, -32768 ~ 32768
    int16_t freq[8];  //  0 ~ 1, 0 ~ pi / 2, 0 ~ 32768
} __attribute__((aligned(16))) CoridcResetStruct;

typedef struct {
    int16_t freq[8]; // 0 ~ 1, 0 ~ pi / 2, 0 ~ 32768
} __attribute__((aligned(16))) CoridcFreqStruct;

typedef struct {
    int16_t s16[8];
} __attribute__((aligned(16))) Vec128Struct;

inline static void Coridc_Reset(CoridcData* ptr, uint32_t num, CoridcResetStruct* reset) {
    for (uint32_t i = 0; i < num; ++i) {
        ptr[i].y0[0] = Phase_FpSin(reset[i].phase[0]);
        ptr[i].y0[1] = Phase_FpSin(reset[i].phase[1]);
        ptr[i].y0[2] = Phase_FpSin(reset[i].phase[2]);
        ptr[i].y0[3] = Phase_FpSin(reset[i].phase[3]);
        ptr[i].y0[4] = Phase_FpSin(reset[i].phase[4]);
        ptr[i].y0[5] = Phase_FpSin(reset[i].phase[5]);
        ptr[i].y0[6] = Phase_FpSin(reset[i].phase[6]);
        ptr[i].y0[7] = Phase_FpSin(reset[i].phase[7]);
        
        ptr[i].x0[0] = Phase_FpCos(reset[i].phase[0] - reset[i].freq[0]);
        ptr[i].x0[1] = Phase_FpCos(reset[i].phase[1] - reset[i].freq[1]);
        ptr[i].x0[2] = Phase_FpCos(reset[i].phase[2] - reset[i].freq[2]);
        ptr[i].x0[3] = Phase_FpCos(reset[i].phase[3] - reset[i].freq[3]);
        ptr[i].x0[4] = Phase_FpCos(reset[i].phase[4] - reset[i].freq[4]);
        ptr[i].x0[5] = Phase_FpCos(reset[i].phase[5] - reset[i].freq[5]);
        ptr[i].x0[6] = Phase_FpCos(reset[i].phase[6] - reset[i].freq[6]);
        ptr[i].x0[7] = Phase_FpCos(reset[i].phase[7] - reset[i].freq[7]);

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

inline static void Coridc_SetFreq(CoridcData* ptr, uint32_t len, CoridcFreqStruct* freq) {
    Vec128Struct pn_cos;
    Vec128Struct cos_phi;
    Vec128Struct sin_phi;
    for (uint32_t i = 0; i < len; ++i) {
        pn_cos.s16[0] = ptr[i].x0[0] > ptr[i].y0[0] ? Sin2Cos(ptr[i].y0[0]) : -Sin2Cos(ptr[i].y0[0]);
        pn_cos.s16[1] = ptr[i].x0[1] > ptr[i].y0[1] ? Sin2Cos(ptr[i].y0[1]) : -Sin2Cos(ptr[i].y0[1]);
        pn_cos.s16[2] = ptr[i].x0[2] > ptr[i].y0[2] ? Sin2Cos(ptr[i].y0[2]) : -Sin2Cos(ptr[i].y0[2]);
        pn_cos.s16[3] = ptr[i].x0[3] > ptr[i].y0[3] ? Sin2Cos(ptr[i].y0[3]) : -Sin2Cos(ptr[i].y0[3]);
        pn_cos.s16[4] = ptr[i].x0[4] > ptr[i].y0[4] ? Sin2Cos(ptr[i].y0[4]) : -Sin2Cos(ptr[i].y0[4]);
        pn_cos.s16[5] = ptr[i].x0[5] > ptr[i].y0[5] ? Sin2Cos(ptr[i].y0[5]) : -Sin2Cos(ptr[i].y0[5]);
        pn_cos.s16[6] = ptr[i].x0[6] > ptr[i].y0[6] ? Sin2Cos(ptr[i].y0[6]) : -Sin2Cos(ptr[i].y0[6]);
        pn_cos.s16[7] = ptr[i].x0[7] > ptr[i].y0[7] ? Sin2Cos(ptr[i].y0[7]) : -Sin2Cos(ptr[i].y0[7]);
        
        cos_phi.s16[0] = Freq_FpSin(freq[i].freq[0]);
        cos_phi.s16[1] = Freq_FpSin(freq[i].freq[1]);
        cos_phi.s16[2] = Freq_FpSin(freq[i].freq[2]);
        cos_phi.s16[3] = Freq_FpSin(freq[i].freq[3]);
        cos_phi.s16[4] = Freq_FpSin(freq[i].freq[4]);
        cos_phi.s16[5] = Freq_FpSin(freq[i].freq[5]);
        cos_phi.s16[6] = Freq_FpSin(freq[i].freq[6]);
        cos_phi.s16[7] = Freq_FpSin(freq[i].freq[7]);

        sin_phi.s16[0] = Freq_FpCos(freq[i].freq[0]);
        sin_phi.s16[1] = Freq_FpCos(freq[i].freq[1]);
        sin_phi.s16[2] = Freq_FpCos(freq[i].freq[2]);
        sin_phi.s16[3] = Freq_FpCos(freq[i].freq[3]);
        sin_phi.s16[4] = Freq_FpCos(freq[i].freq[4]);
        sin_phi.s16[5] = Freq_FpCos(freq[i].freq[5]);
        sin_phi.s16[6] = Freq_FpCos(freq[i].freq[6]);
        sin_phi.s16[7] = Freq_FpCos(freq[i].freq[7]);

        ptr[i].half_coef[0] = Freq_FpSin(freq[i].freq[0]);
        ptr[i].half_coef[1] = Freq_FpSin(freq[i].freq[1]);
        ptr[i].half_coef[2] = Freq_FpSin(freq[i].freq[2]);
        ptr[i].half_coef[3] = Freq_FpSin(freq[i].freq[3]);
        ptr[i].half_coef[4] = Freq_FpSin(freq[i].freq[4]);
        ptr[i].half_coef[5] = Freq_FpSin(freq[i].freq[5]);
        ptr[i].half_coef[6] = Freq_FpSin(freq[i].freq[6]);
        ptr[i].half_coef[7] = Freq_FpSin(freq[i].freq[7]);

        asm volatile
        (
            "movi.n a5, 15\n\r"
            "wsr.sar a5\n\r"
            "ee.vld.128.ip q0, %[p], 16\n\r"     // q0 <= curr.x0
            "ee.vld.128.ip q1, %[p], -16\n\r"    // q1 <= curr.y0, ptr = curr.x0
            "ld.qr q2, %[cosp], 0\n\r"  // q2 <= pn_cos
            "ld.qr q3, %[cosphi], 0\n\r" // q3 <= cos_phi
            "ld.qr q4, %[sinphi], 0\n\r" // q4 <= sin_phi
            "ee.vmul.s16 q5, q1, q3\n\r"
            "ee.vmul.s16 q3, q2, q4\n\r"
            "ee.vadds.s16 q0, q5, q3\n\r"
            "st.qr q0, %[p], 0"
            :
            :[p]"r"(ptr), [cosp]"r"(&pn_cos), [cosphi]"r"(&cos_phi), [sinphi]"r"(&sin_phi)
            :"a5"
        );
        ++freq;
        ++ptr;
    }
}

inline static void Coridc_Tick(CoridcData* ptr, uint32_t len, int16_t* out) 
{
    asm volatile 
    (
        "movi.n a5, 14\n\r" // half_coeff * 2, >>14 turns into >>13
        "wsr.sar a5\n\r"

        ".loop:\n\r"

        "ee.vld.128.ip q0, %[p], 16\n\r"  // q0 <= curr.x0
        "ee.vld.128.ip q1, %[p], 16\n\r"  // q1 <= curr.y0
        "ee.vld.128.ip q2, %[p], -32\n\r" // q2 <= half_coeff, ptr = curr.x0
        "ee.vmul.s16 q3, q2, q1\n\r"
        "ee.vsubs.s16 q4, q0, q3\n\r"     // q4 <= curr.x0 - half_coeff * curr.y0
        "mv.qr q0, q4\n\r"                // q0 <= new_x0
        "ee.vmul.s16.st.incp  q4, %[p], q3, q2, q0\n\r" // ptr = curr.y0
        "ee.vadds.s16 q4, q1, q3\n\r"
        "ee.vst.128.ip q4, %[p], 16\n\r" // ptr = next.x0

        "addi.n %[num], %[num], -1\n\r"
        "bnez %[num], .loop\n\r"
        :
        :[num]"r"(len), [p]"r"(ptr)
        :"a5"
    );
}

#ifdef __cplusplus
}
#endif