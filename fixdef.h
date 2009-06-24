/* Definitions used in fixed point math library assembly.
   Author:  Sean D'Epagnier
*/

#ifndef	_FIXDEF_H
#define	_FIXDEF_H

#define	rB0	r18
#define	rB1	r19
#define	rB2	r20
#define	rB3	r21

#define	rA0	r22
#define	rA1	r23
#define	rA2	r24
#define	rA3	r25

#define __scratch_reg__ r0
#define __zero_reg__ r1

/* defines for cordic routines */
#define CORDICC    0x9b75  /* circular cordic gain constant */
#define CORDICC_X2 0x136ea /* circular cordic gain constant times 2 */
#define CORDICC_X4 0x26dd4 /* circular cordic gain constant times 4 */

#define CORDICH    0x1352e /* hyperbolic cordic gain constant */
#define CORDICH_X4 0x4d4a7 /* hyperbolic cordic gain constant times 4 */

#define PIK        0x3243f /* Pi */
#define PI2K       0x1921f /* Pi/2 */
#define PIMK       0xc90fdaa /* Pi shifted to the left the max */

#define LOG2K      0xb172  /* log(2) */

#define rXh0 r6
#define rXh1 r7
#define rXh2 r8
#define rXh3 r9
#define rYh0 r10
#define rYh1 r11
#define rYh2 r12
#define rYh3 r13
#define rY0  r14
#define rY1  r15
#define rY2  r16
#define rY3  r17
#define rX0  r18
#define rX1  r19
#define rX2  r20
#define rX3  r21

#define rcnt r26
#define rk   r27   /* used for hyperbolic repeat count */

#define rAt0  r2    /* used for asin only */
#define rAt1  r3
#define rAt2  r4
#define rAt3  r5

#if     defined(__AVR_ENHANCED__) && __AVR_ENHANCED__
#define	rmul1L  r16		/* multiplier Low */
#define	rmul1H  r17
#define	rmul1HL r18
#define	rmul1HH r19		/* multiplier High */

#define	rmul2L  r20		/* multiplicand Low */
#define	rmul2H  r21
#define	rmul2HL r22
#define	rmul2HH r23		/* multiplicand High */

#define rmulrL	 r24     	/* result Low */
#define rmulrH   r25
#define rmulrHL	 r26
#define rmulrHH  r27	        /* result High */
#else
#define	rmul1L  r18		/* multiplier Low */
#define	rmul1H  r19
#define	rmul1HL r20
#define	rmul1HH r21		/* multiplier High */

#define	rmul2L  r24		/* multiplicand Low */
#define	rmul2H  r25
#define	rmul2HL r26
#define	rmul2HH r27		/* multiplicand High */

#define rmulrL	 r14     	/* result Low */
#define rmulrH   r15
#define rmulrHL	 r16
#define rmulrHH  r17	        /* result High */
#endif

/* Put functions at this section.	*/
#ifdef	FUNCTION
# error	"The FUNCTION macro must be defined after FUNC_SEGNAME"
#endif
#define FUNC_SEGNAME	.text.fixlib

/* Put constant tables at low addresses in program memory, so they are
   reachable for "lpm" without using RAMPZ on >64K devices.  */
#define PGM_SECTION	.section  .progmem.gcc_fixlib, "a", @progbits

#endif	/* !_FIXDEF_H */
