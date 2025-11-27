/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2025 Amlogic, Inc. All rights reserved.
 */

#ifndef __BASE_C_H__
#define __BASE_C_H__

/* not about ic, not about other struct */

#define C_BIT0		0x00000001
#define C_BIT1		0x00000002
#define C_BIT2		0x00000004
#define C_BIT3		0x00000008
#define C_BIT4		0x00000010
#define C_BIT5		0x00000020
#define C_BIT6		0x00000040
#define C_BIT7		0x00000080
#define C_BIT8		0x00000100
#define C_BIT9		0x00000200
#define C_BIT10	0x00000400
#define C_BIT11	0x00000800
#define C_BIT12	0x00001000
#define C_BIT13	0x00002000
#define C_BIT14	0x00004000
#define C_BIT15	0x00008000
#define C_BIT16	0x00010000
#define C_BIT17	0x00020000
#define C_BIT18	0x00040000
#define C_BIT19	0x00080000
#define C_BIT20	0x00100000
#define C_BIT21	0x00200000
#define C_BIT22	0x00400000
#define C_BIT23	0x00800000
#define C_BIT24	0x01000000
#define C_BIT25	0x02000000
#define C_BIT26	0x04000000
#define C_BIT27	0x08000000
#define C_BIT28	0x10000000
#define C_BIT29	0x20000000
#define C_BIT30	0x40000000
#define C_BIT31	0x80000000

/*****************************************/
#define D_COM_M(m, a, b)	(((a) & (m)) == ((b) & (m)))
#define D_COM_MV(a, m, v)	(((a) & (m)) == (v))
#define D_COM_ME(a, m)	(((a) & (m)) == (m))

/*****************************************/
static inline unsigned int get_reg_bits(unsigned int val, unsigned int bstart,
					unsigned int bw)
{
	return ((val & (((1L << bw) - 1) << bstart)) >> (bstart));
}

/*****************************************/

unsigned int code_count(void *p);
bool code_check(void *p, unsigned int code);

//tmp bool dpss_timer_cnt(unsigned long *ptimer, unsigned int hs_nub);

#endif	/*__BASE_C_H__*/
