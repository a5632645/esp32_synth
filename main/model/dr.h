#pragma once

#ifndef __cplusplus
#error only c++ support
#endif

#include "my_math.h"
#include "my_fp.h"

extern "C" {
typedef struct {
    MyFpInt128T sin0_;
    MyFpInt128T sin1_;
    MyFpInt128T cos0_;
    MyFpInt128T cos1_;
    MyFpInt128T coeff_;
    MyFpInt128T gain_;
} ParalleDr;

extern void ParalleDr_Reset(ParalleDr* dr, MyFpInt128T* freq, MyFpInt128T* phase, uint32_t num);
inline static void ParalleDr_ResetF(ParalleDr* dr, float* freq, float* phase, uint32_t num) {
    MyFpFloatBundleT fb = {};
    MyFpFloatBundleT fb1 = {};
    MyFpFloatBundleT fb2 = {};
    MyFpFloatBundleT fb3 = {};
    MyFpFloatBundleT fb4 = {};
    for (uint32_t i = 0; i < num; ++i) {
        for (int j = 0; j < 8; ++j) {
            fb.f32[j] = MySin(*phase);
            fb1.f32[j] = MySin(*phase + *freq);
            fb2.f32[j] = MyCos(*phase);
            fb3.f32[j] = MyCos(*phase + *freq);
            fb4.f32[j] = 2.0f * MyCos(*freq);
            ++phase;
            ++freq;
        }
        MyFp_FromFloatBundle(&fb, &dr[i].sin0_);
        MyFp_FromFloatBundle(&fb1, &dr[i].sin1_);
        MyFp_FromFloatBundle(&fb2, &dr[i].cos0_);
        MyFp_FromFloatBundle(&fb3, &dr[i].cos1_);
        MyFp_FromFloatBundle(&fb4, &dr[i].coeff_);
    }
}
/*
* there is no vector arithmetic sign16 left shift instruction
* so we do it by hand
*/
extern void __ParalleDr_SetFreq(ParalleDr* dr, MyFpInt128T* fsin, MyFpInt128T* fcos, uint32_t num);
inline static void ParalleDr_SetFreq(ParalleDr* dr, MyFpInt128T* fsin, MyFpInt128T* fcos, uint32_t num) {
    __ParalleDr_SetFreq(dr, fsin, fcos, num);
    for (uint32_t i = 0; i < num; ++i) {
        dr[i].coeff_.s16[0] = fcos[i].s16[0] << 1;
        dr[i].coeff_.s16[1] = fcos[i].s16[1] << 1;
        dr[i].coeff_.s16[2] = fcos[i].s16[2] << 1;
        dr[i].coeff_.s16[3] = fcos[i].s16[3] << 1;
        dr[i].coeff_.s16[4] = fcos[i].s16[4] << 1;
        dr[i].coeff_.s16[5] = fcos[i].s16[5] << 1;
        dr[i].coeff_.s16[6] = fcos[i].s16[6] << 1;
        dr[i].coeff_.s16[7] = fcos[i].s16[7] << 1;
    }
}
inline static void ParalleDr_SetFreqF(ParalleDr* dr, float* freq, uint32_t num) {
    MyFpFloatBundleT fb = {};
    MyFpFloatBundleT fb1 = {};
    MyFpInt128T fcos = {};
    MyFpInt128T fsin = {};
    MyFpInt128T tmp0 = {};
    MyFpInt128T tmp1 = {};
    MyFpS1_15 two = MYFP_FROM_FLOAT(2.0f);
    for (uint32_t i = 0; i < num; ++i) {
        for (int j = 0; j < 8; ++j) {
            fb.f32[j] = MySin(*freq);
            fb1.f32[j] = MyCos(*freq);
            ++freq;
        }
        MyFp_FromFloatBundle(&fb, &fcos);
        MyFp_FromFloatBundle(&fb1, &fsin);

        MyFp_Mul(&dr[i].sin0_, &fcos, &tmp0);
        MyFp_Mul(&dr[i].cos0_, &fsin, &tmp1);
        MyFp_AddSat(&tmp0, &tmp1, &dr[i].sin1_);

        MyFp_Mul(&dr[i].cos0_, &fcos, &tmp0);
        MyFp_Mul(&dr[i].sin0_, &fsin, &tmp1);
        MyFp_AddSat(&tmp0, &tmp1, &dr[i].cos1_);

        MyFp_MulBC(&fcos, &two, &dr[i].coeff_);
    }
}
extern void ParalleDr_Tick(ParalleDr* dr, int16_t* sample_out, uint32_t num, uint32_t sample_len);
}

template <typename T>
struct DR {
    T sin0_;
    T sin1_;
    T cos0_;
    T cos1_;
    T coeff_;

    /**
     * @brief set the initial value
     * @param freq 0 ~ pi
     * @param phase 0 ~ 2pi
     */
    inline void Reset(T freq, T phase) noexcept {
        sin0_ = MySin(phase);
        sin1_ = MySin(phase + freq);
        cos0_ = MyCos(phase);
        cos1_ = MyCos(phase + freq);
        coeff_ = T(2) * MyCos(freq);
    }

    /**
     * @brief set the frequency
     * @param freq 0 ~ pi
     */
    inline void SetFreq(T freq) noexcept {
        T fcos = MyCos(freq);
        T fsin = MySin(freq);
        sin1_ = sin0_ * fcos + cos0_ * fsin;
        cos1_ = cos0_ * fcos - sin0_ * fsin;
        coeff_ = T(2) * fcos;
    }

    inline void Tick() noexcept {
        T e = coeff_ * sin1_ - sin0_;
        sin0_ = sin1_;
        sin1_ = e;
        e = coeff_ * cos1_ - cos0_;
        cos0_ = cos1_;
        cos1_ = e;
    }

    inline T Sin() const noexcept { return sin0_; }
    inline T Cos() const noexcept { return cos0_; } 
};