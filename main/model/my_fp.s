.text
.align 4

// void MyFp_Mul(MyFp1_15* x: a2, MyFp1_15* y: a3, MyFp1_15* result: a4)
.global MyFp_Mul
.type MyFp_Mul,@function
MyFp_Mul:
    entry a1, 32

    ld.qr q0, a2, 0
    ld.qr q1, a3, 0
    movi.n a2, 13
    wsr.sar a2
    ee.vmul.s16 q2, q0, q1
    st.qr q2, a4, 0

    retw.n

// void MyFp_AddSat(MyFpInt128T* a0, MyFpInt128T* a1, MyFpInt128T* result);
.align 4
.global MyFp_AddSat
.type MyFp_AddSat,@function
MyFp_AddSat:
    entry a1, 32

    ld.qr q0, a2, 0
    ld.qr q1, a3, 0
    ee.vadds.s16 q2, q0, q1
    st.qr q2, a4, 0

    retw.n

// void MyFp_SubSat(MyFpInt128T* a0, MyFpInt128T* a1, MyFpInt128T* result);
.align 4
.global MyFp_SubSat
.type MyFp_SubSat,@function
MyFp_SubSat:
    entry a1, 32

    ld.qr q0, a2, 0
    ld.qr q1, a3, 0
    ee.vsubs.s16 q2, q0, q1
    st.qr q2, a4, 0

    retw.n

// ==================================================================
// broadcast
// ==================================================================

// void MyFp_AddSatBC(MyFpInt128T* a2, MyFp1_15* a3, MyFpInt128T* result);
.align 4
.global MyFp_AddSatBC
.type MyFp_AddSatBC,@function
MyFp_AddSatBC:
    entry a1, 32

    ld.qr q0, a2, 0
    ee.vldbc.16 q1, a3
    ee.vadds.s16 q2, q0, q1
    st.qr q2, a4, 0

    retw.n

// void MyFp_SubSatBC(MyFpInt128T* a2, MyFp1_15* a3, MyFpInt128T* result);
.align 4
.global MyFp_SubSatBC
.type MyFp_SubSatBC,@function
MyFp_SubSatBC:
    entry a1, 32

    ld.qr q0, a2, 0
    ee.vldbc.16 q1, a3
    ee.vsubs.s16 q2, q0, q1
    st.qr q2, a4, 0

    retw.n

// void MyFp_MulBC(MyFpInt128T* a2, MyFp1_15* a3, MyFpInt128T* result);
.align 4
.global MyFp_MulBC
.type MyFp_MulBC,@function
MyFp_MulBC:
    entry a1, 32

    ld.qr q0, a2, 0
    movi.n a2, 13
    wsr.sar a2
    ee.vldbc.16 q1, a3
    ee.vmul.s16 q2, q0, q1
    st.qr q2, a4, 0

    retw.n
