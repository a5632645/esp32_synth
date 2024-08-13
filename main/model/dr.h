#pragma once

#include "table_helper.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    MyFpS0_15 half_coeff[8];
    MyFpS1_14 sin1[8];
    MyFpS1_14 sin0[8];
    MyFpS1_14 cos1[8];
    MyFpS1_14 cos0[8];
} __attribute__((aligned(16))) DrData;

inline static void MY_CONSTEXPR Dr_Reset(DrData* ptr, uint32_t num, MyFpS0_15* freq, MyFpS1_14* phase) {
    for (uint32_t i = 0; i < num; i++) {
        auto offset = i * 8;
        ptr[i].half_coeff[0] = Freq_FpCos(freq[0 + offset]);
        ptr[i].half_coeff[1] = Freq_FpCos(freq[1 + offset]);
        ptr[i].half_coeff[2] = Freq_FpCos(freq[2 + offset]);
        ptr[i].half_coeff[3] = Freq_FpCos(freq[3 + offset]);
        ptr[i].half_coeff[4] = Freq_FpCos(freq[4 + offset]);
        ptr[i].half_coeff[5] = Freq_FpCos(freq[5 + offset]);
        ptr[i].half_coeff[6] = Freq_FpCos(freq[6 + offset]);
        ptr[i].half_coeff[7] = Freq_FpCos(freq[7 + offset]);

        ptr[i].sin0[0] = Phase_FpSin(phase[0 + offset]);
        ptr[i].sin0[1] = Phase_FpSin(phase[1 + offset]);
        ptr[i].sin0[2] = Phase_FpSin(phase[2 + offset]);
        ptr[i].sin0[3] = Phase_FpSin(phase[3 + offset]);
        ptr[i].sin0[4] = Phase_FpSin(phase[4 + offset]);
        ptr[i].sin0[5] = Phase_FpSin(phase[5 + offset]);
        ptr[i].sin0[6] = Phase_FpSin(phase[6 + offset]);
        ptr[i].sin0[7] = Phase_FpSin(phase[7 + offset]);

        ptr[i].sin1[0] = Phase_FpSin(phase[0 + offset] + freq[0 + offset]);
        ptr[i].sin1[1] = Phase_FpSin(phase[1 + offset] + freq[1 + offset]);
        ptr[i].sin1[2] = Phase_FpSin(phase[2 + offset] + freq[2 + offset]);
        ptr[i].sin1[3] = Phase_FpSin(phase[3 + offset] + freq[3 + offset]);
        ptr[i].sin1[4] = Phase_FpSin(phase[4 + offset] + freq[4 + offset]);
        ptr[i].sin1[5] = Phase_FpSin(phase[5 + offset] + freq[5 + offset]);
        ptr[i].sin1[6] = Phase_FpSin(phase[6 + offset] + freq[6 + offset]);
        ptr[i].sin1[7] = Phase_FpSin(phase[7 + offset] + freq[7 + offset]);
        
        ptr[i].cos0[0] = Phase_FpCos(phase[0 + offset]);
        ptr[i].cos0[1] = Phase_FpCos(phase[1 + offset]);
        ptr[i].cos0[2] = Phase_FpCos(phase[2 + offset]);
        ptr[i].cos0[3] = Phase_FpCos(phase[3 + offset]);
        ptr[i].cos0[4] = Phase_FpCos(phase[4 + offset]);
        ptr[i].cos0[5] = Phase_FpCos(phase[5 + offset]);
        ptr[i].cos0[6] = Phase_FpCos(phase[6 + offset]);
        ptr[i].cos0[7] = Phase_FpCos(phase[7 + offset]);

        ptr[i].cos1[0] = Phase_FpCos(phase[0 + offset] + freq[0 + offset]);
        ptr[i].cos1[1] = Phase_FpCos(phase[1 + offset] + freq[1 + offset]);
        ptr[i].cos1[2] = Phase_FpCos(phase[2 + offset] + freq[2 + offset]);
        ptr[i].cos1[3] = Phase_FpCos(phase[3 + offset] + freq[3 + offset]);
        ptr[i].cos1[4] = Phase_FpCos(phase[4 + offset] + freq[4 + offset]);
        ptr[i].cos1[5] = Phase_FpCos(phase[5 + offset] + freq[5 + offset]);
        ptr[i].cos1[6] = Phase_FpCos(phase[6 + offset] + freq[6 + offset]);
        ptr[i].cos1[7] = Phase_FpCos(phase[7 + offset] + freq[7 + offset]);
    }
}

inline static void MY_CONSTEXPR Dr_SetFreq(DrData* ptr, uint32_t num, MyFpS0_15* freq) {
    Vec128Struct sinw = {};
    Vec128Struct cosw = {};
    for (uint32_t i = 0; i < num; ++i) {
        auto offset = i * 8;

        sinw.s16[0] = Freq_FpSin(freq[0 + offset]);
        sinw.s16[1] = Freq_FpSin(freq[1 + offset]);
        sinw.s16[2] = Freq_FpSin(freq[2 + offset]);
        sinw.s16[3] = Freq_FpSin(freq[3 + offset]);
        sinw.s16[4] = Freq_FpSin(freq[4 + offset]);
        sinw.s16[5] = Freq_FpSin(freq[5 + offset]);
        sinw.s16[6] = Freq_FpSin(freq[6 + offset]);
        sinw.s16[7] = Freq_FpSin(freq[7 + offset]);

        cosw.s16[0] = Freq_FpCos(freq[0 + offset]);
        cosw.s16[1] = Freq_FpCos(freq[1 + offset]);
        cosw.s16[2] = Freq_FpCos(freq[2 + offset]);
        cosw.s16[3] = Freq_FpCos(freq[3 + offset]);
        cosw.s16[4] = Freq_FpCos(freq[4 + offset]);
        cosw.s16[5] = Freq_FpCos(freq[5 + offset]);
        cosw.s16[6] = Freq_FpCos(freq[6 + offset]);
        cosw.s16[7] = Freq_FpCos(freq[7 + offset]);
        
        ptr[i].half_coeff[0] = Freq_FpCos(freq[0 + offset]);
        ptr[i].half_coeff[1] = Freq_FpCos(freq[1 + offset]);
        ptr[i].half_coeff[2] = Freq_FpCos(freq[2 + offset]);
        ptr[i].half_coeff[3] = Freq_FpCos(freq[3 + offset]);
        ptr[i].half_coeff[4] = Freq_FpCos(freq[4 + offset]);
        ptr[i].half_coeff[5] = Freq_FpCos(freq[5 + offset]);
        ptr[i].half_coeff[6] = Freq_FpCos(freq[6 + offset]);
        ptr[i].half_coeff[7] = Freq_FpCos(freq[7 + offset]);

        asm volatile(
            "movi a5, 15\n\r"
            "wsr.sar a5\n\r"
            "ld.qr               q0,   %[c], 0         \n\r"
            "ee.vst.128.ip       q0,   %[p], 16        \n\r" // q0 <= sinw
            "ee.vld.128.ip       q1,   %[p], 32        \n\r" // q1 <= sin1
            "ee.vmul.s16.ld.incp q2,   %[p], q3, q0, q1\n\r" // q2 <= cos1
            "addi                %[p], %[p], -48       \n\r"
            "ld.qr               q4,   %[s], 0         \n\r" // q4 <= cosw
            "ee.vmul.s16         q5,   q2,   q4        \n\r"
            "ee.vadds.s16        q6,   q3,   q5        \n\r"
            "ee.vmul.s16.ld.incp q6,   %[p], q5, q2, q4\n\r"
            "addi                %[p], %[p], 16        \n\r"
            "ee.vmul.s16         q3,   q1,   q0        \n\r"
            "ee.vsubs.s16        q4,   q5,   q3        \n\r"
            "st.qr               q4,   %[p], 0         \n\r"
            :
            :[p]"r"(ptr), [s]"r"(&sinw), [c]"r"(&cosw)
            :"a5"
        );

        ++ptr;
        freq += 8;
    }
}

inline static void Dr_Tick(DrData* ptr, uint32_t num, MyFpS0_15* gain, int32_t* out) {
    if (num == 0) {
        *out = 0;
        return;
    }

    asm volatile (
        "ee.zero.accx\n\r"
        "movi a6, 14\n\r"   
        "wsr.sar a6\n\r"

        ".dr_loop:\n\r"
        "ee.vld.128.ip            q0, %[p], 16         \n\r" // q0 <= half_coeff
        "ee.vld.128.ip            q1, %[p], 16         \n\r" // q1 <= sin1
        "ee.vmul.s16.ld.incp      q2, %[p], q3, q0, q1 \n\r" // q2 <= sin0, q3 <= c * s1, exec.ptr = curr.cos1
        "ee.vsubs.s16.ld.incp     q4, %[p], q5, q3, q2 \n\r" // q4 <= cos1, q5 <= new_sin1, q1 <= new_sin0, exec.ptr = curr.cos0
        "ee.vld.128.ip            q3, %[g], 16         \n\r" // q3 <= gain
        "ee.vmulas.s16.accx.ld.ip q6, %[p], -48, q3, q2\n\r" // q6 <= cos0, exec.ptr = curr.sin1
        "ee.vmul.s16.st.incp      q5, %[p], q3, q4, q0 \n\r" // exec.ptr = curr.sin0
        "ee.vsubs.s16.st.incp     q1, %[p], q5, q3, q6 \n\r" // q5 <= new_cos1, exec.ptr = curr.cos1
        "ee.vst.128.ip            q5, %[p], 16         \n\r"
        "ee.vst.128.ip            q4, %[p], 16         \n\r" // exec.ptr = next.half_coeff

        "addi %[num], %[num], -1\n\r"
        "bnez %[num], .dr_loop\n\r"
        
        "movi a7, 5\n\r"
        "ee.srs.accx a6, a7, 0\n\r"
        "s32i a6, %[out], 0\n\r"
        :
        :[p]"r"(ptr), [num]"r"(num), [g]"r"(gain), [out]"r"(out)
        :"a6", "a7"
    );
}

#ifdef __cplusplus
}
#endif