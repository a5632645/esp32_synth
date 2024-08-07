#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int16_t MyFp2_13;
typedef __attribute((aligned(16))) struct {
    MyFp2_13 s16[8];
} MyFpInt128T;

typedef struct {
    float f32[8];
} MyFpFloatBundleT;

#define MYFP_FROM_FLOAT(FLOAT_VAL) ((MyFp2_13)((FLOAT_VAL) * (1 << 13)))
#define MYFP_TO_FLOAT(MYFP_VAL) ((float)(MYFP_VAL) / (float)(1 << 13))

#define MYFP_FROM_FLOAT_BC(FLOAT_VAL) {MYFP_FROM_FLOAT(FLOAT_VAL), MYFP_FROM_FLOAT(FLOAT_VAL), MYFP_FROM_FLOAT(FLOAT_VAL), MYFP_FROM_FLOAT(FLOAT_VAL), MYFP_FROM_FLOAT(FLOAT_VAL), MYFP_FROM_FLOAT(FLOAT_VAL), MYFP_FROM_FLOAT(FLOAT_VAL), MYFP_FROM_FLOAT(FLOAT_VAL)}
#define MYFP_FROM_FLOAT_ARRAY(VAL0, VAL1, VAL2, VAL3, VAL4, VAL5, VAL6, VAL7) {MYFP_FROM_FLOAT(VAL0), MYFP_FROM_FLOAT(VAL1), MYFP_FROM_FLOAT(VAL2), MYFP_FROM_FLOAT(VAL3), MYFP_FROM_FLOAT(VAL4), MYFP_FROM_FLOAT(VAL5), MYFP_FROM_FLOAT(VAL6), MYFP_FROM_FLOAT(VAL7)}

inline static void MyFp_FromFloatPtr(float* a0, MyFpInt128T* result) {
    result->s16[0] = MYFP_FROM_FLOAT(a0[0]);
    result->s16[1] = MYFP_FROM_FLOAT(a0[1]);
    result->s16[2] = MYFP_FROM_FLOAT(a0[2]);
    result->s16[3] = MYFP_FROM_FLOAT(a0[3]);
    result->s16[4] = MYFP_FROM_FLOAT(a0[4]);
    result->s16[5] = MYFP_FROM_FLOAT(a0[5]);
    result->s16[6] = MYFP_FROM_FLOAT(a0[6]);
    result->s16[7] = MYFP_FROM_FLOAT(a0[7]);
}
inline static void MyFp_FromFloat(MyFpFloatBundleT* a0, MyFpInt128T* result) {
    MyFp_FromFloatPtr(a0->f32, result);
}
inline static void MyFp_ToFloat(MyFpInt128T* a0, MyFpFloatBundleT* result) {
    result->f32[0] = MYFP_TO_FLOAT(a0->s16[0]);
    result->f32[1] = MYFP_TO_FLOAT(a0->s16[1]);
    result->f32[2] = MYFP_TO_FLOAT(a0->s16[2]);
    result->f32[3] = MYFP_TO_FLOAT(a0->s16[3]);
    result->f32[4] = MYFP_TO_FLOAT(a0->s16[4]);
    result->f32[5] = MYFP_TO_FLOAT(a0->s16[5]);
    result->f32[6] = MYFP_TO_FLOAT(a0->s16[6]);
    result->f32[7] = MYFP_TO_FLOAT(a0->s16[7]);
}

void MyFp_AddSat(MyFpInt128T* a0, MyFpInt128T* a1, MyFpInt128T* result);
void MyFp_SubSat(MyFpInt128T* a0, MyFpInt128T* a1, MyFpInt128T* result);
void MyFp_Mul(MyFpInt128T* a0, MyFpInt128T* a1, MyFpInt128T* result);

void MyFp_AddSatBC(MyFpInt128T* a2, MyFp2_13* a3, MyFpInt128T* result);
void MyFp_SubSatBC(MyFpInt128T* a2, MyFp2_13* a3, MyFpInt128T* result);
void MyFp_MulBC(MyFpInt128T* a2, MyFp2_13* a3, MyFpInt128T* result);

#ifdef __cplusplus
}
#endif