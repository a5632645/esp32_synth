.text
// void ParalleDr_Reset(ParalleDr* dr, MyFpInt128T* freq, MyFpInt128T* phase, uint32_t num);
.align 4
.global ParalleDr_Reset
.type ParalleDr_Reset,@function
ParalleDr_Reset:
    entry a1, 32
    retw.n

//                               a2             a3               a4                a5
// void __ParalleDr_SetFreq(ParalleDr* dr, MyFpInt128T* fsin, MyFpInt128T* fcos, uint32_t num);
.align 4
.global __ParalleDr_SetFreq
.type __ParalleDr_SetFreq, @function
__ParalleDr_SetFreq:
            entry a1, 32

            movi.n a6, 16
            wsr.sar a6

.set_freq_loop:
            ee.vld.128.ip q0, a2, 32  // q0 <= sin0_, exec.a2.offset = curr.dr.cos0_
            ee.vld.128.ip q1, a2, -16 // q1 <= cos0_, exec.a2.offset = curr.dr.sin1_
            ee.vld.128.ip q2, a3, 16  // q2 <= fsin
            ee.vld.128.ip q3, a4, 16  // q3 <= fcos

            // sin1_ = sin0_ * fcos + cos0_ * fsin;
            // TODO: use ee.cmul to reduce instruction......
            ee.vmul.s16 q4, q0, q3
            ee.vmul.s16 q5, q1, q2
            ee.vadds.s16.st.incp q4, a2, q4, q4, q5 // sin1_new => curr.dr.sin1_, exec.a2.offset = curr.dr.cos0_
            addi a2, a2, 16 // exec.a2.offset = curr.dr.cos1_

            // cos1_ = cos0_ * fcos - sin0_ * fsin;
            ee.vmul.s16 q4, q1, q3
            ee.vmul.s16 q5, q0, q2
            ee.vsubs.s16.st.incp q4, a2, q4, q4, q5 // cos1_new => curr.dr.cos1_, exec.a2.offset = curr.dr.coeff_

            // coeff_ = T(2) * fcos; fcos << 1...
            // no instruction for vector arithmetic left shift
            addi a2, a2, 32 // exec.a2.offset = next.dr.sin0_

            bbsi a5, 0, .set_freq_loop // check dr_num != 0?

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
            movi.n a6, 10 // accx 14.26 to 24.16

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
            ee.vld.128.ip q5, a2, -80 // q5 <= gain_, a2 = curr.dr.sin0

            ee.vmul.s16.st.incp  q1, a2, q6, q1, q4 // q6 = coeff_ * sin1             , sin1_old => a2(offset = sin0)
            ee.vsubs.s16.st.incp q6, a2, q6, q6, q0 // q6(sin1) = coeff_ * sin1 - sin0, sin1_new => a2(offset = sin1)
            mv.qr q0, q1 // sin0 = sin1
            mv.qr q1, q6 // sin1 = coeff_ * sin1 - sin0

            ee.vmul.s16.st.incp  q3, a2, q6, q3, q4 // q6 = coeff_ * cos1             , cos1_old => a2(offset = cos0)
            ee.vsubs.s16.st.incp q6, a2, q6, q6, q2 // q6(cos1) = coeff_ * cos1 - cos0, cos1_new => a2(offset = cos1)
            addi a2, a2, 32 // after exec, a2 = next.dr.sin0
            # mv.qr q2, q3 // cos1 = cos0
            # mv.qr q3, q6 // cos1 = coeff_ * cos1 - cos0

            # // write back dr values
            # addi a2, a2, -72
            # ee.vst.128.ip q0, a5, 16
            # ee.vst.128.ip q1, a5, 16
            # ee.vst.128.ip q2, a5, 16
            # ee.vst.128.ip q3, a5, 48

            // accx <= reduce_sum(sin0_ * gain) + accx
            ee.vmulas.s16.accx q0, q5

            addi a4, a4, -1
            bbsi a4, 0, _dr_reduce_loop // check dr_num != 0?.

            // accx = float * 1 << 26; 14.26 fixed point => 24.16 fixed point
            ee.srs.accx a7, a6, 0
            // write low 16 to sample_out and inc a3
            l16si a6, a3, 0
            add a7, a7, a6
            s16i a7, a3, 0
            addi a3, a3, 2

            bbsi a5, 0, _sample_end_loop // check sample_len != 1?
            retw.n
