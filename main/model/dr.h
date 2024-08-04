#pragma once

#ifndef __cplusplus
#error only c++ support
#endif

#include "my_math.h"

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