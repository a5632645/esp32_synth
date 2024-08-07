.text

// void ParalleDr_Reset(ParalleDr* dr, MyFpInt128T* freq, MyFpInt128T* phase, uint32_t num);
.align 4
.global ParalleDr_Reset
.type ParalleDr_Reset,@function
ParalleDr_Reset:
    entry a1, 32
    retw.n

// void ParalleDr_SetFreq(ParalleDr* dr, MyFpInt128T* freq, uint32_t num);
.align 4
.global ParalleDr_SetFreq
.type ParalleDr_SetFreq,@function
ParalleDr_SetFreq:
    entry a1, 32
    retw.n

//                            a2             a3                a4                 a5
// void ParalleDr_Tick(ParalleDr* dr, int16_t* sample_out, uint32_t num, uint32_t sample_len);
.align 4
.global ParalleDr_Tick
.type ParalleDr_Tick,@function
ParalleDr_Tick:
            entry a1, 64

            s32i.n a4, a1, 4 // save dr_num to sp + 4
            s32i.n a2, a1, 8 // save dr_ptr to sp + 8

            movi.n a6, 13 // set up SAR for s2.13 fixed point
            wsr.sar a6

            beqi a5, 1, _dr_tick_once

//  for (int i = 0; i < sample_len; i++)
_sample_end_loop:
            l32i.n a4, a1, 4 // a4 = dr_num
            l32i.n a2, a1, 8 // a2 = dr_ptr

//          for (int j = 0; j < dr_num; j++)
            ee.zero.accx // accx = 0

_dr_reduce_loop:
            ee.vld.128.ip q0, a2, 16 // q0 <= sin0_
            ee.vld.128.ip q1, a2, 16 // q1 <= sin1_
            ee.vld.128.ip q2, a2, 16 // q2 <= cos0_
            ee.vld.128.ip q3, a2, 16 // q3 <= cos1_
            ee.vld.128.ip q4, a2, 16 // q4 <= coeff_
            ee.vld.128.ip q5, a2, 16 // q5 <= gain_

            ee.vmul.s16  q6, q1, q4 // coeff_ * sin1
            ee.vsubs.s16 q6, q6, q0 // coeff_ * sin1 - sin0
            mv.qr q0, q1 // sin1 = sin0
            mv.qr q1, q6 // sin1 = coeff_ * sin1 - sin0

            ee.vmul.s16  q6, q3, q4 // coeff_ * cos1
            ee.vsubs.s16 q6, q6, q2 // coeff_ * cos1 - cos0
            mv.qr q2, q3 // cos1 = cos0
            mv.qr q3, q6 // cos1 = coeff_ * cos1 - cos0

            // accx <= sin0_ * gain + accx
            ee.vmulas.s16.accx q0, q5

            addi a4, a4, -1
            bbsi a4, 0, _dr_reduce_loop // check dr_num != 0?
            // accx = float * 1 << 26; 14.26 fixed point => 24.16 fixed point
            movi.n a6, 10 // accx 14.26 to 24.16
            ee.srs.accx a7, a6, 0
            // write low 16 to sample_out and inc a3
            ; l16si a6, a3, 0
            ; add a7, a7, a6
            ; s16i a7, a3, 0
            ; addi a3, a3, 2
            s16i a7, a3, 0
            addi a3, a3, 2

            bbsi a5, 1, _sample_end_loop // check sample_len != 1?

_dr_tick_once:
//          last dr process, need to write back dr values to dr memory
//          for (int j = 0; j < dr_num; j++)
            ee.zero.accx // accx = 0
            l32i.n a4, a1, 4 // a4 = dr_num
            l32i.n a2, a1, 8 // a2 = dr_ptr
            mov.n  a5, a2

_dr_tick_once_loop:
            ee.vld.128.ip q0, a2, 16 // q0 <= sin0_
            ee.vld.128.ip q1, a2, 16 // q1 <= sin1_
            ee.vld.128.ip q2, a2, 16 // q2 <= cos0_
            ee.vld.128.ip q3, a2, 16 // q3 <= cos1_
            ee.vld.128.ip q4, a2, 16 // q4 <= coeff_
            ee.vld.128.ip q5, a2, 16 // q5 <= gain_

            ee.vmul.s16  q6, q1, q4 // coeff_ * sin1
            ee.vsubs.s16 q6, q6, q0 // coeff_ * sin1 - sin0
            mv.qr q0, q1 // sin1 = sin0
            mv.qr q1, q6 // sin1 = coeff_ * sin1 - sin0

            ee.vmul.s16  q6, q3, q4 // coeff_ * cos1
            ee.vsubs.s16 q6, q6, q2 // coeff_ * cos1 - cos0
            mv.qr q2, q3 // cos1 = cos0
            mv.qr q3, q6 // cos1 = coeff_ * cos1 - cos0

            // write back dr values
            ee.vst.128.ip q0, a5, 16
            ee.vst.128.ip q1, a5, 16
            ee.vst.128.ip q2, a5, 16
            ee.vst.128.ip q3, a5, 48

            // accx <= sin0_ * gain + accx
            ee.vmulas.s16.accx q0, q5

            addi a4, a4, -1
            bbsi a4, 0, _dr_tick_once_loop // check dr_num != 0?

            // accx = float * 1 << 26; 14.26 fixed point => 24.16 fixed point
            movi.n a6, 10
            ee.srs.accx a7, a6, 0
            // write low 16 to sample_out
            ; l16si a6, a3, 0
            ; add a7, a7, a6
            ; s16i a7, a3, 2
            s16i a7, a3, 0
            addi a3, a3, 2

            retw.n
