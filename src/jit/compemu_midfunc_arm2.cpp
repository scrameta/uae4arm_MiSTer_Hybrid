/*
 * compiler/compemu_midfunc_arm.cpp - Native MIDFUNCS for ARM (JIT v2)
 *
 * Copyright (c) 2014 Jens Heitmann of ARAnyM dev team (see AUTHORS)
 *
 * Inspired by Christian Bauer's Basilisk II
 *
 *  Original 68040 JIT compiler for UAE, copyright 2000-2002 Bernd Meyer
 *
 *  Adaptation for Basilisk II and improvements, copyright 2000-2002
 *    Gwenole Beauchesne
 *
 *  Basilisk II (C) 1997-2002 Christian Bauer
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  Note:
 *  	File is included by compemu_support.cpp
 *
 */

const uae_u32 ARM_CCR_MAP[] = { 0, ARM_C_FLAG, // 1 C
	ARM_V_FLAG, // 2 V
	ARM_C_FLAG | ARM_V_FLAG, // 3 VC
	ARM_Z_FLAG, // 4 Z
	ARM_Z_FLAG | ARM_C_FLAG, // 5 ZC
	ARM_Z_FLAG | ARM_V_FLAG, // 6 ZV
	ARM_Z_FLAG | ARM_C_FLAG | ARM_V_FLAG, // 7 ZVC
	ARM_N_FLAG, // 8 N
	ARM_N_FLAG | ARM_C_FLAG, // 9 NC
	ARM_N_FLAG | ARM_V_FLAG, // 10 NV
	ARM_N_FLAG | ARM_C_FLAG | ARM_V_FLAG, // 11 NVC
	ARM_N_FLAG | ARM_Z_FLAG, // 12 NZ
	ARM_N_FLAG | ARM_Z_FLAG | ARM_C_FLAG, // 13 NZC
	ARM_N_FLAG | ARM_Z_FLAG | ARM_V_FLAG, // 14 NZV
	ARM_N_FLAG | ARM_Z_FLAG | ARM_C_FLAG | ARM_V_FLAG, // 15 NZVC
};


MIDFUNC(0,restore_inverted_carry,(void))
{
  RR4 r = readreg(FLAGX, 4);
  MRS_CPSR(REG_WORK1);
#if defined(ARMV6_ASSEMBLY)
	BFI_rrii(REG_WORK1, r, ARM_C_FLAG, ARM_C_FLAG);
#else
  TEQ_ri(r, 1);
  CC_BIC_rri(NATIVE_CC_EQ, REG_WORK1, REG_WORK1, ARM_C_FLAG);
  CC_ORR_rri(NATIVE_CC_NE, REG_WORK1, REG_WORK1, ARM_C_FLAG);
#endif
  MSR_CPSRf_r(REG_WORK1);
  unlock2(r);
}
MENDFUNC(0,restore_inverted_carry,(void))

/*
 * ADD
 * Operand Syntax: 	<ea>, Dn
 * 					Dn, <ea>
 *
 * Operand Size: 8,16,32
 *
 * X Set the same as the carry bit.
 * N Set if the result is negative. Cleared otherwise.
 * Z Set if the result is zero. Cleared otherwise.
 * V Set if an overflow is generated. Cleared otherwise.
 * C Set if a carry is generated. Cleared otherwise.
 *
 */
MIDFUNC(3,jnf_ADD_imm,(W4 d, RR4 s, IMM v))
{
	if (isconst(s)) {
		set_const(d, live.state[s].val + v);
		return;
	}

	s = readreg(s, 4);
	d = writereg(d, 4);

	compemu_raw_mov_l_ri(REG_WORK1, v);
	ADD_rrr(d, s, REG_WORK1);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jnf_ADD_imm,(W4 d, RR4 s, IMM v))

MIDFUNC(3,jnf_ADD,(W4 d, RR4 s, RR4 v))
{
	if (isconst(v)) {
		COMPCALL(jnf_ADD_imm)(d, s, live.state[v].val);
		return;
	}

	v = readreg(v, 4);
	s = readreg(s, 4);
	d = writereg(d, 4);

	ADD_rrr(d, s, v);

	unlock2(d);
	unlock2(s);
	unlock2(v);
}
MENDFUNC(3,jnf_ADD,(W4 d, RR4 s, RR4 v))

MIDFUNC(3,jff_ADD_b_imm,(W4 d, RR1 s, IMM v))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	MOV_ri8RORi(REG_WORK2, v & 0xff, 8);
  ADDS_rrrLSLi(d, REG_WORK2, s, 24);
	ASR_rri(d, d, 24);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jff_ADD_b_imm,(W4 d, RR1 s, IMM v))

MIDFUNC(3,jff_ADD_b,(W4 d, RR1 s, RR1 v))
{
	if (isconst(v)) {
		COMPCALL(jff_ADD_b_imm)(d, s, live.state[v].val);
		return;
	}

	v = readreg(v, 4);
	s = readreg(s, 4);
	d = writereg(d, 4);

	LSL_rri(REG_WORK2, v, 24);
  ADDS_rrrLSLi(d, REG_WORK2, s, 24);
	ASR_rri(d, d, 24);

	unlock2(d);
	unlock2(s);
	unlock2(v);
}
MENDFUNC(3,jff_ADD_b,(W4 d, RR1 s, RR1 v))

MIDFUNC(3,jff_ADD_w_imm,(W4 d, RR2 s, IMM v))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

  uae_s32 offs = data_long_offs(v << 16);
	LDR_rRI(REG_WORK1, RPC_INDEX, offs);
  ADDS_rrrLSLi(d, REG_WORK1, s, 16);
	ASR_rri(d, d, 16);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jff_ADD_w_imm,(W4 d, RR2 s, IMM v))

MIDFUNC(3,jff_ADD_w,(W4 d, RR2 s, RR2 v))
{
	if (isconst(v)) {
		COMPCALL(jff_ADD_w_imm)(d,s,live.state[v].val);
		return;
	}
	v = readreg(v,  4);
	s = readreg(s,4);
	d = writereg(d, 4);

	LSL_rri(REG_WORK1, s, 16);
  ADDS_rrrLSLi(d, REG_WORK1, v, 16);
	ASR_rri(d, d, 16);

	unlock2(d);
	unlock2(s);
	unlock2(v);
}
MENDFUNC(3,jff_ADD_w,(W4 d, RR2 s, RR2 v))

MIDFUNC(3,jff_ADD_l_imm,(W4 d, RR4 s, IMM v))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	compemu_raw_mov_l_ri(REG_WORK2, v);
	ADDS_rrr(d, s, REG_WORK2);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jff_ADD_l_imm,(W4 d, RR4 s, IMM v))

MIDFUNC(3,jff_ADD_l,(W4 d, RR4 s, RR4 v))
{
	if (isconst(v)) {
		COMPCALL(jff_ADD_l_imm)(d, s, live.state[v].val);
		return;
	}

	v = readreg(v, 4);
	s = readreg(s, 4);
	d = writereg(d, 4);

	ADDS_rrr(d, s, v);

	unlock2(d);
	unlock2(s);
	unlock2(v);
}
MENDFUNC(3,jff_ADD_l,(W4 d, RR4 s, RR4 v))

/*
 * ADDA
 * Operand Syntax: 	<ea>, An
 *
 * Operand Size: 16,32
 *
 * Flags: Not affected.
 *
 */
MIDFUNC(2,jnf_ADDA_b,(W4 d, RR1 s))
{
	s = readreg(s, 4);
	d = rmw(d, 4, 4);

	SIGNED8_REG_2_REG(REG_WORK1, s);
	ADD_rrr(d, d, REG_WORK1);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jnf_ADDA_b,(W4 d, RR1 s))

MIDFUNC(2,jnf_ADDA_w,(W4 d, RR2 s))
{
	s = readreg(s, 4);
	d = rmw(d, 4, 4);

	SIGNED16_REG_2_REG(REG_WORK1, s);
	ADD_rrr(d, d, REG_WORK1);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jnf_ADDA_w,(W4 d, RR2 s))

MIDFUNC(2,jnf_ADDA_l,(W4 d, RR4 s))
{
	s = readreg(s, 4);
	d = rmw(d, 4, 4);

	ADD_rrr(d, d, s);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jnf_ADDA_l,(W4 d, RR4 s))

/*
 * ADDX
 * Operand Syntax: 	Dy, Dx
 * 					-(Ay), -(Ax)
 *
 * Operand Size: 8,16,32
 *
 * X Set the same as the carry bit.
 * N Set if the result is negative. Cleared otherwise.
 * Z Cleared if the result is nonzero; unchanged otherwise.
 * V Set if an overflow is generated. Cleared otherwise.
 * C Set if a carry is generated. Cleared otherwise.
 *
 * Attention: Z is cleared only if the result is nonzero. Unchanged otherwise
 *
 */
MIDFUNC(3,jnf_ADDX,(W4 d, RR4 s, RR4 v))
{
	s = readreg(s, 4);
	v = readreg(v, 4);
	d = writereg(d, 4);

	ADC_rrr(d, s, v);

	unlock2(d);
	unlock2(s);
	unlock2(v);
}
MENDFUNC(3,jnf_ADDX,(W4 d, RR4 s, RR4 v))

MIDFUNC(3,jff_ADDX_b,(W4 d, RR1 s, RR1 v))
{
	s = readreg(s, 4);
	v = readreg(v, 4);
	d = writereg(d, 4);

	CC_MVN_ri(NATIVE_CC_EQ, REG_WORK2, 0);
	CC_MVN_ri(NATIVE_CC_NE, REG_WORK2, ARM_Z_FLAG);

	MVN_ri(REG_WORK1, 0);
	BFI_rrii(REG_WORK1, s, 24, 31);
  ADCS_rrrLSLi(d, REG_WORK1, v, 24);
	ASR_rri(d, d, 24);

	MRS_CPSR(REG_WORK1);
	AND_rrr(REG_WORK1, REG_WORK1, REG_WORK2);
	MSR_CPSR_r(REG_WORK1);

	unlock2(d);
	unlock2(s);
	unlock2(v);
}
MENDFUNC(3,jff_ADDX_b,(W4 d, RR1 s, RR1 v))

MIDFUNC(3,jff_ADDX_w,(W4 d, RR2 s, RR2 v))
{
	s = readreg(s, 4);
	v = readreg(v, 4);
	d = writereg(d, 4);

	CC_MVN_ri(NATIVE_CC_EQ, REG_WORK2, 0);
	CC_MVN_ri(NATIVE_CC_NE, REG_WORK2, ARM_Z_FLAG);

	MVN_ri(REG_WORK1, 0);
	BFI_rrii(REG_WORK1, s, 16, 31);
  ADCS_rrrLSLi(d, REG_WORK1, v, 16);
	ASR_rri(d, d, 16);

	MRS_CPSR(REG_WORK1);
	AND_rrr(REG_WORK1, REG_WORK1, REG_WORK2);
	MSR_CPSR_r(REG_WORK1);

	unlock2(d);
	unlock2(s);
	unlock2(v);
}
MENDFUNC(3,jff_ADDX_w,(W4 d, RR2 s, RR2 v))

MIDFUNC(3,jff_ADDX_l,(W4 d, RR4 s, RR4 v))
{
	s = readreg(s, 4);
	v = readreg(v, 4);
	d = writereg(d, 4);

	CC_MVN_ri(NATIVE_CC_EQ, REG_WORK2, 0);
	CC_MVN_ri(NATIVE_CC_NE, REG_WORK2, ARM_Z_FLAG);

	ADCS_rrr(d, s, v);

	MRS_CPSR(REG_WORK1);
	AND_rrr(REG_WORK1, REG_WORK1, REG_WORK2);
	MSR_CPSR_r(REG_WORK1);

	unlock2(d);
	unlock2(s);
	unlock2(v);
}
MENDFUNC(3,jff_ADDX_l,(W4 d, RR4 s, RR4 v))

/*
 * ANDI
 * Operand Syntax: 	#<data>, CCR
 *
 * Operand Size: 8
 *
 * X Cleared if bit 4 of immediate operand is zero. Unchanged otherwise.
 * N Cleared if bit 3 of immediate operand is zero. Unchanged otherwise.
 * Z Cleared if bit 2 of immediate operand is zero. Unchanged otherwise.
 * V Cleared if bit 1 of immediate operand is zero. Unchanged otherwise.
 * C Cleared if bit 0 of immediate operand is zero. Unchanged otherwise.
 *
 */
MIDFUNC(1,jff_ANDSR,(IMM s, IMM x))
{
	MRS_CPSR(REG_WORK1);
	AND_rri(REG_WORK1, REG_WORK1, s);
	MSR_CPSRf_r(REG_WORK1);

	if (!x) {
		compemu_raw_mov_l_ri(REG_WORK1, (uintptr)live.state[FLAGX].mem);
		MOV_ri(REG_WORK2, 0);
		STRB_rR(REG_WORK2, REG_WORK1);
	}
}
MENDFUNC(1,jff_ANDSR,(IMM s))

/*
 * AND
 * Operand Syntax: 	<ea>, Dn
 * 					Dn, <ea>
 *
 * Operand Size: 8,16,32
 *
 * X Not affected.
 * N Set if the most significant bit of the result is set. Cleared otherwise.
 * Z Set if the result is zero. Cleared otherwise.
 * V Always cleared.
 * C Always cleared.
 *
 */
MIDFUNC(3,jnf_AND,(W4 d, RR4 s, RR4 v))
{
	if (isconst(s) && isconst(v)) {
		set_const(d, live.state[s].val & live.state[v].val);
		return;
	}

	v = readreg(v, 4);
	s = readreg(s, 4);
	d = writereg(d, 4);

	AND_rrr(d, s, v);

	unlock2(v);
	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jnf_AND,(RW4 d, RR4 s, RR4 v))

MIDFUNC(3,jff_AND_b,(W4 d, RR1 s, RR1 v))
{
	v = readreg(v, 4);
	s = readreg(s, 4);
	d = writereg(d, 4);

	SIGNED8_REG_2_REG(REG_WORK1, s);
	SIGNED8_REG_2_REG(REG_WORK2, v);
	MSR_CPSRf_i(0);
	ANDS_rrr(d, REG_WORK1, REG_WORK2);

	unlock2(v);
	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jff_AND_b,(RW4 d, RR1 s, RR1 v))

MIDFUNC(3,jff_AND_w,(W4 d, RR2 s, RR2 v))
{
	v = readreg(v, 4);
	s = readreg(s, 4);
	d = writereg(d, 4);

	SIGNED16_REG_2_REG(REG_WORK1, s);
	SIGNED16_REG_2_REG(REG_WORK2, v);
	MSR_CPSRf_i(0);
	ANDS_rrr(d, REG_WORK1, REG_WORK2);

	unlock2(v);
	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jff_AND_w,(RW4 d, RR2 s, RR2 v))

MIDFUNC(3,jff_AND_l,(W4 d, RR4 s, RR4 v))
{
	v = readreg(v, 4);
	s = readreg(s, 4);
	d = writereg(d, 4);

	MSR_CPSRf_i(0);
	ANDS_rrr(d, s, v);

	unlock2(v);
	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jff_AND_l,(RW4 d, RR4 s, RR4 v))

/*
 * ASL
 * Operand Syntax: 	Dx, Dy
 * 					#<data>, Dy
 *					<ea>
 *
 * Operand Size: 8,16,32
 *
 * X Set according to the last bit shifted out of the operand. Unaffected for a shift count of zero.
 * N Set if the most significant bit of the result is set. Cleared otherwise.
 * Z Set if the result is zero. Cleared otherwise.
 * V Set if the most significant bit is changed at any time during the shift operation. Cleared otherwise.
 * C Set according to the last bit shifted out of the operand. Cleared for a shift count of zero.
 *
 */
// ToDo: X unaffected for a shift count of zero. Currently duplicate_carry() is used by caller in all cases.
MIDFUNC(3,jff_ASL_b_imm,(W4 d, RR4 s, IMM i))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	LSL_rri(d, s, 24);
	if (i) {
		// Calculate V Flag
		MOV_ri8RORi(REG_WORK2, 0x80, 8);
		ASR_rri(REG_WORK2, REG_WORK2, i);
		ANDS_rrr(REG_WORK1, d, REG_WORK2);
		CC_TEQ_rr(NATIVE_CC_NE, REG_WORK1, REG_WORK2);
		CC_MOV_ri(NATIVE_CC_EQ, REG_WORK1, 0);
		CC_MOV_ri(NATIVE_CC_NE, REG_WORK1, ARM_V_FLAG);

		MSR_CPSRf_r(REG_WORK1); // store V flag

		LSLS_rri(d, d, i);
	} else {
		MSR_CPSRf_i(0);
		TST_rr(d, d);
	}
	ASR_rri(d, d, 24);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jff_ASL_b_imm,(W4 d, RR4 s, IMM i))

MIDFUNC(3,jff_ASL_w_imm,(W4 d, RR4 s, IMM i))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	LSL_rri(d, s, 16);
	if (i) {
		// Calculate V Flag
		MOV_ri8RORi(REG_WORK2, 0x80, 8);
		ASR_rri(REG_WORK2, REG_WORK2, i);
		ANDS_rrr(REG_WORK1, d, REG_WORK2);
		CC_TEQ_rr(NATIVE_CC_NE, REG_WORK1, REG_WORK2);
		CC_MOV_ri(NATIVE_CC_EQ, REG_WORK1, 0);
		CC_MOV_ri(NATIVE_CC_NE, REG_WORK1, ARM_V_FLAG);

		MSR_CPSRf_r(REG_WORK1); // store V flag

		LSLS_rri(d, d, i);
	} else {
		MSR_CPSRf_i(0);
		TST_rr(d, d);
	}
	ASR_rri(d, d, 16);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jff_ASL_w_imm,(W4 d, RR4 s, IMM i))

MIDFUNC(3,jff_ASL_l_imm,(W4 d, RR4 s, IMM i))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	if (i) {
		// Calculate V Flag
		MOV_ri8RORi(REG_WORK2, 0x80, 8);
		ASR_rri(REG_WORK2, REG_WORK2, i);
		ANDS_rrr(REG_WORK1, s, REG_WORK2);
		CC_TEQ_rr(NATIVE_CC_NE, REG_WORK1, REG_WORK2);
		CC_MOV_ri(NATIVE_CC_EQ, REG_WORK1, 0);
		CC_MOV_ri(NATIVE_CC_NE, REG_WORK1, ARM_V_FLAG);

		MSR_CPSRf_r(REG_WORK1); // store V flag

		LSLS_rri(d, s, i);
	} else {
		MSR_CPSRf_i(0);
		MOVS_rr(d, s);
	}

	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jff_ASL_l_imm,(W4 d, RR4 s, IMM i))

MIDFUNC(3,jff_ASL_b_reg,(W4 d, RR4 s, RR4 i))
{
	i = readreg(i, 4);
	s = readreg(s, 4);
	d = writereg(d, 4);


	LSL_rri(d, s, 24);
	// Calculate V Flag
	MOV_ri8RORi(REG_WORK2, 0x80, 8);
	ASR_rrr(REG_WORK2, REG_WORK2, i);
	ANDS_rrr(REG_WORK1, d, REG_WORK2);
	CC_TEQ_rr(NATIVE_CC_NE, REG_WORK1, REG_WORK2);
	CC_MOV_ri(NATIVE_CC_EQ, REG_WORK1, 0);
	CC_MOV_ri(NATIVE_CC_NE, REG_WORK1, ARM_V_FLAG);

	MSR_CPSRf_r(REG_WORK1); // store V flag

	AND_rri(REG_WORK2, i, 63);
	LSLS_rrr(d, d, REG_WORK2);
	ASR_rri(d, d, 24);

	unlock2(d);
	unlock2(s);
	unlock2(i);
}
MENDFUNC(3,jff_ASL_b_reg,(W4 d, RR4 s, RR4 i))

MIDFUNC(3,jff_ASL_w_reg,(W4 d, RR4 s, RR4 i))
{
	i = readreg(i, 4);
	s = readreg(s, 4);
	d = writereg(d, 4);


	LSL_rri(d, s, 16);
	// Calculate V Flag
	MOV_ri8RORi(REG_WORK2, 0x80, 8);
	ASR_rrr(REG_WORK2, REG_WORK2, i);
	ANDS_rrr(REG_WORK1, d, REG_WORK2);
	CC_TEQ_rr(NATIVE_CC_NE, REG_WORK1, REG_WORK2);
	CC_MOV_ri(NATIVE_CC_EQ, REG_WORK1, 0);
	CC_MOV_ri(NATIVE_CC_NE, REG_WORK1, ARM_V_FLAG);

	MSR_CPSRf_r(REG_WORK1); // store V flag

	AND_rri(REG_WORK2, i, 63);
	LSLS_rrr(d, d, REG_WORK2);
	ASR_rri(d, d, 16);

	unlock2(d);
	unlock2(s);
	unlock2(i);
}
MENDFUNC(3,jff_ASL_w_reg,(W4 d, RR4 s, RR4 i))

MIDFUNC(3,jff_ASL_l_reg,(W4 d, RR4 s, RR4 i))
{
	i = readreg(i, 4);
	s = readreg(s, 4);
	d = writereg(d, 4);

	// Calculate V Flag
	MOV_ri8RORi(REG_WORK2, 0x80, 8);
	ASR_rrr(REG_WORK2, REG_WORK2, i);
	ANDS_rrr(REG_WORK1, s, REG_WORK2);
	CC_TEQ_rr(NATIVE_CC_NE, REG_WORK1, REG_WORK2);
	CC_MOV_ri(NATIVE_CC_EQ, REG_WORK1, 0);
	CC_MOV_ri(NATIVE_CC_NE, REG_WORK1, ARM_V_FLAG);

	MSR_CPSRf_r(REG_WORK1); // store V flag

	AND_rri(REG_WORK2, i, 63);
	LSLS_rrr(d, s, REG_WORK2);

	unlock2(d);
	unlock2(s);
	unlock2(i);
}
MENDFUNC(3,jff_ASL_l_reg,(W4 d, RR4 s, RR4 i))

/*
 * ASLW
 * Operand Syntax: 	<ea>
 *
 * Operand Size: 16
 *
 * X Set according to the last bit shifted out of the operand.
 * N Set if the most significant bit of the result is set. Cleared otherwise.
 * Z Set if the result is zero. Cleared otherwise.
 * V Set if the most significant bit is changed at any time during the shift operation. Cleared otherwise.
 * C Set according to the last bit shifted out of the operand.
 *
 */
MIDFUNC(2,jnf_ASLW,(W4 d, RR4 s))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	LSL_rri(d, s, 1);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jnf_ASLW,(W4 d, RR4 s))

MIDFUNC(2,jff_ASLW,(W4 d, RR4 s))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	MSR_CPSRf_i(0);
	LSLS_rri(d, s, 17);

  // Calculate V flag
	MRS_CPSR(REG_WORK1);
	CC_ORR_rri(NATIVE_CC_MI, REG_WORK1, REG_WORK1, ARM_V_FLAG);
	CC_EOR_rri(NATIVE_CC_CS, REG_WORK1, REG_WORK1, ARM_V_FLAG);
	MSR_CPSRf_r(REG_WORK1);
	ASR_rri(d, d, 16);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jff_ASLW,(W4 d, RR4 s))

/*
 * ASR
 * Operand Syntax: 	Dx, Dy
 * 					#<data>, Dy
 *					<ea>
 *
 * Operand Size: 8,16,32
 *
 * X Set according to the last bit shifted out of the operand. Unaffected for a shift count of zero.
 * N Set if the most significant bit of the result is set. Cleared otherwise.
 * Z Set if the result is zero. Cleared otherwise.
 * V Set if the most significant bit is changed at any time during the shift operation. Cleared otherwise. Shift right -> always 0
 * C Set according to the last bit shifted out of the operand. Cleared for a shift count of zero.
 *
 */
// ToDo: X unaffected for a shift count of zero. Currently duplicate_carry() is used by caller in all cases.
MIDFUNC(3,jnf_ASR_b_imm,(W4 d, RR4 s, IMM i))
{
	if (!i) return;

	s = readreg(s, 4);
	d = writereg(d, 4);

	SIGNED8_REG_2_REG(d, s);
	ASR_rri(d, d, i);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jnf_ASR_b_imm,(W4 d, RR4 s, IMM i))

MIDFUNC(3,jnf_ASR_w_imm,(W4 d, RR4 s, IMM i))
{
	if (!i) return;

	s = readreg(s, 4);
	d = writereg(d, 4);

	SIGNED16_REG_2_REG(d, s);
	ASR_rri(d, d, i);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jnf_ASR_w_imm,(W4 d, RR4 s, IMM i))

MIDFUNC(3,jnf_ASR_l_imm,(W4 d, RR4 s, IMM i))
{
	if (!i) return;

	s = readreg(s, 4);
	d = writereg(d, 4);

	ASR_rri(d, s, i);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jnf_ASR_l_imm,(W4 d, RR4 s, IMM i))

MIDFUNC(3,jff_ASR_b_imm,(W4 d, RR4 s, IMM i))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	SIGNED8_REG_2_REG(d, s);
	MSR_CPSRf_i(0);
	if (i) {
		ASRS_rri(d, d, i);
	} else {
		TST_rr(d, d);
	}

	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jff_ASR_b_imm,(W4 d, RR4 s, IMM i))

MIDFUNC(3,jff_ASR_w_imm,(W4 d, RR4 s, IMM i))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	SIGNED16_REG_2_REG(d, s);
	MSR_CPSRf_i(0);
	if (i) {
		ASRS_rri(d, d, i);
	} else {
		TST_rr(d, d);
	}

	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jff_ASR_w_imm,(W4 d, RR4 s, IMM i))

MIDFUNC(3,jff_ASR_l_imm,(W4 d, RR4 s, IMM i))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	MSR_CPSRf_i(0);
	if (i) {
		ASRS_rri(d, s, i);
	} else {
		MOVS_rr(d, s);
	}

	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jff_ASR_l_imm,(W4 d, RR4 s, IMM i))

MIDFUNC(3,jnf_ASR_b_reg,(W4 d, RR4 s, RR4 i)) 
{
	i = readreg(i, 4);
	s = readreg(s, 4);
	d = writereg(d, 4);

	SIGNED8_REG_2_REG(d, s);
	AND_rri(REG_WORK1, i, 63);
	ASR_rrr(d, d, REG_WORK1);

	unlock2(d);
	unlock2(s);
	unlock2(i);
}
MENDFUNC(3,jnf_ASR_b_reg,(W4 d, RR4 s, RR4 i))

MIDFUNC(3,jnf_ASR_w_reg,(W4 d, RR4 s, RR4 i))
{
	i = readreg(i, 4);
	s = readreg(s, 4);
	d = writereg(d, 4);

	SIGNED16_REG_2_REG(d, s);
	AND_rri(REG_WORK1, i, 63);
	ASR_rrr(d, d, REG_WORK1);

	unlock2(d);
	unlock2(s);
	unlock2(i);
}
MENDFUNC(3,jnf_ASR_w_reg,(W4 d, RR4 s, RR4 i))

MIDFUNC(3,jnf_ASR_l_reg,(W4 d, RR4 s, RR4 i))
{
	i = readreg(i, 4);
	s = readreg(s, 4);
	d = writereg(d, 4);

	AND_rri(REG_WORK1, i, 63);
	ASR_rrr(d, s, REG_WORK1);

	unlock2(d);
	unlock2(s);
	unlock2(i);
}
MENDFUNC(3,jnf_ASR_l_reg,(W4 d, RR4 s, RR4 i))

MIDFUNC(3,jff_ASR_b_reg,(W4 d, RR4 s, RR4 i))
{
	i = readreg(i, 4);
	s = readreg(s, 4);
	d = writereg(d, 4);

	SIGNED8_REG_2_REG(d, s);
	MSR_CPSRf_i(0);
	AND_rri(REG_WORK1, i, 63);
	ASRS_rrr(d, d, REG_WORK1);

	unlock2(d);
	unlock2(s);
	unlock2(i);
}
MENDFUNC(3,jff_ASR_b_reg,(W4 d, RR4 s, RR4 i))

MIDFUNC(3,jff_ASR_w_reg,(W4 d, RR4 s, RR4 i)) 
{
	i = readreg(i, 4);
	s = readreg(s, 4);
	d = writereg(d, 4);

	SIGNED16_REG_2_REG(d, s);
	MSR_CPSRf_i(0);
	AND_rri(REG_WORK1, i, 63);
	ASRS_rrr(d, d, REG_WORK1);

	unlock2(d);
	unlock2(s);
	unlock2(i);
}
MENDFUNC(3,jff_ASR_w_reg,(W4 d, RR4 s, RR4 i))

MIDFUNC(3,jff_ASR_l_reg,(W4 d, RR4 s, RR4 i))
{
	i = readreg(i, 4);
	s = readreg(s, 4);
	d = writereg(d, 4);

	MSR_CPSRf_i(0);
	AND_rri(REG_WORK1, i, 63);
	ASRS_rrr(d, s, REG_WORK1);

	unlock2(d);
	unlock2(s);
	unlock2(i);
}
MENDFUNC(3,jff_ASR_l_reg,(W4 d, RR4 s, RR4 i))

/*
 * ASRW
 * Operand Syntax: 	<ea>
 *
 * Operand Size: 16
 *
 * X Set according to the last bit shifted out of the operand.
 * N Set if the most significant bit of the result is set. Cleared otherwise.
 * Z Set if the result is zero. Cleared otherwise.
 * V Set if the most significant bit is changed at any time during the shift operation. Cleared otherwise. Shift right -> always 0
 * C Set according to the last bit shifted out of the operand.
 *
 */
MIDFUNC(2,jnf_ASRW,(W4 d, RR4 s))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	SIGNED16_REG_2_REG(d, s);
	ASR_rri(d, d, 1);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jnf_ASRW,(W4 d, RR4 s))

MIDFUNC(2,jff_ASRW,(W4 d, RR4 s))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	SIGNED16_REG_2_REG(d, s);
	MSR_CPSRf_i(0);
	ASR_rri(d, d, 1);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jff_ASRW,(W4 d, RR4 s))

/*
 * BCHG
 * Operand Syntax: 	Dn,<ea>
 *					#<data>,<ea>
 *
 *  Operand Size: 8,32
 *
 * X Not affected.
 * N Not affected.
 * Z Set if the bit tested was zero. Cleared otherwise.
 * V Not affected.
 * C Not affected.
 *
 */
MIDFUNC(2,jnf_BCHG_b_imm,(RW4 d, IMM s))
{
	d = rmw(d, 4, 4);
	EOR_rri(d, d, (1 << s));
	unlock2(d);
}
MENDFUNC(2,jnf_BCHG_b_imm,(RW4 d, IMM s))

MIDFUNC(2,jnf_BCHG_l_imm,(RW4 d, IMM s))
{
	d = rmw(d, 4, 4);
	EOR_rri(d, d, (1 << s));
	unlock2(d);
}
MENDFUNC(2,jnf_BCHG_l_imm,(RW4 d, IMM s))

MIDFUNC(2,jnf_BCHG_b,(RW4 d, RR4 s))
{
	if (isconst(s)) {
		COMPCALL(jnf_BCHG_b_imm)(d, live.state[s].val & 7);
		return;
	}
	s = readreg(s, 4);
	d = rmw(d, 4, 4);

	AND_rri(REG_WORK1, s, 7);
	MOV_ri(REG_WORK2, 1);

	EOR_rrrLSLr(d, d, REG_WORK2, REG_WORK1);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jnf_BCHG_b,(RW4 d, RR4 s))

MIDFUNC(2,jnf_BCHG_l,(RW4 d, RR4 s))
{
	if (isconst(s)) {
		COMPCALL(jnf_BCHG_l_imm)(d, live.state[s].val & 31);
		return;
	}

	s = readreg(s, 4);
	d = rmw(d, 4, 4);

	AND_rri(REG_WORK1, s, 31);
	MOV_ri(REG_WORK2, 1);

	EOR_rrrLSLr(d, d, REG_WORK2, REG_WORK1);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jnf_BCHG_l,(RW4 d, RR4 s))

MIDFUNC(2,jff_BCHG_b_imm,(RW4 d, IMM s))
{
	d = rmw(d, 4, 4);

	uae_u32 v = (1 << s);
	MRS_CPSR(REG_WORK1);
	TST_ri(d, v);
	CC_BIC_rri(NATIVE_CC_NE, REG_WORK1, REG_WORK1, ARM_Z_FLAG);
	CC_ORR_rri(NATIVE_CC_EQ, REG_WORK1, REG_WORK1, ARM_Z_FLAG);
	MSR_CPSR_r(REG_WORK1);
	EOR_rri(d, d, v);

	unlock2(d);
}
MENDFUNC(2,jff_BCHG_b_imm,(RW4 d, IMM s))

MIDFUNC(2,jff_BCHG_l_imm,(RW4 d, IMM s))
{
	d = rmw(d, 4, 4);

	uae_u32 v = (1 << s);
	MRS_CPSR(REG_WORK1);
	TST_ri(d, v);
	CC_BIC_rri(NATIVE_CC_NE, REG_WORK1, REG_WORK1, ARM_Z_FLAG);
	CC_ORR_rri(NATIVE_CC_EQ, REG_WORK1, REG_WORK1, ARM_Z_FLAG);
	MSR_CPSR_r(REG_WORK1);
	EOR_rri(d, d, v);

	unlock2(d);
}
MENDFUNC(2,jff_BCHG_l_imm,(RW4 d, IMM s))

MIDFUNC(2,jff_BCHG_b,(RW4 d, RR4 s))
{
	if (isconst(s)) {
		COMPCALL(jff_BCHG_b_imm)(d, live.state[s].val & 7);
		return;
	}
	s = readreg(s, 4);
	d = rmw(d, 4, 4);

	AND_rri(REG_WORK1, s, 7);
	MOV_ri(REG_WORK2, 1);
	LSL_rrr(REG_WORK2, REG_WORK2, REG_WORK1);

	MRS_CPSR(REG_WORK1);
	TST_rr(d, REG_WORK2);
	CC_BIC_rri(NATIVE_CC_NE, REG_WORK1, REG_WORK1, ARM_Z_FLAG);
	CC_ORR_rri(NATIVE_CC_EQ, REG_WORK1, REG_WORK1, ARM_Z_FLAG);
	MSR_CPSR_r(REG_WORK1);
	EOR_rrr(d, d, REG_WORK2);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jff_BCHG_b,(RW4 d, RR4 s))

MIDFUNC(2,jff_BCHG_l,(RW4 d, RR4 s))
{
	if (isconst(s)) {
		COMPCALL(jff_BCHG_l_imm)(d, live.state[s].val & 31);
		return;
	}

	s = readreg(s, 4);
	d = rmw(d, 4, 4);

	AND_rri(REG_WORK1, s, 31);
	MOV_ri(REG_WORK2, 1);
	LSL_rrr(REG_WORK2, REG_WORK2, REG_WORK1);

	MRS_CPSR(REG_WORK1);
	TST_rr(d, REG_WORK2);
	CC_BIC_rri(NATIVE_CC_NE, REG_WORK1, REG_WORK1, ARM_Z_FLAG);
	CC_ORR_rri(NATIVE_CC_EQ, REG_WORK1, REG_WORK1, ARM_Z_FLAG);
	MSR_CPSR_r(REG_WORK1);
	EOR_rrr(d, d, REG_WORK2);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jff_BCHG_l,(RW4 d, RR4 s))

/*
 * BCLR
 * Operand Syntax: 	Dn,<ea>
 *					#<data>,<ea>
 *
 * Operand Size: 8,32
 *
 * X Not affected.
 * N Not affected.
 * Z Set if the bit tested was zero. Cleared otherwise.
 * V Not affected.
 * C Not affected.
 *
 */
MIDFUNC(2,jnf_BCLR_b_imm,(RW4 d, IMM s))
{
	d = rmw(d, 4, 4);
	BIC_rri(d, d, (1 << s));
	unlock2(d);
}
MENDFUNC(2,jnf_BCLR_b_imm,(RW4 d, IMM s))

MIDFUNC(2,jnf_BCLR_l_imm,(RW4 d, IMM s))
{
	d = rmw(d, 4, 4);
	BIC_rri(d, d, (1 << s));
	unlock2(d);
}
MENDFUNC(2,jnf_BCLR_l_imm,(RW4 d, IMM s))

MIDFUNC(2,jnf_BCLR_b,(RW4 d, RR4 s))
{
	if (isconst(s)) {
		COMPCALL(jnf_BCLR_b_imm)(d, live.state[s].val & 7);
		return;
	}
	s = readreg(s, 4);
	d = rmw(d, 4, 4);

	AND_rri(REG_WORK1, s, 7);
	MOV_ri(REG_WORK2, 1);

	BIC_rrrLSLr(d, d, REG_WORK2, REG_WORK1);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jnf_BCLR_b,(RW4 d, RR4 s))

MIDFUNC(2,jnf_BCLR_l,(RW4 d, RR4 s))
{
	if (isconst(s)) {
		COMPCALL(jnf_BCLR_l_imm)(d, live.state[s].val & 31);
		return;
	}

	s = readreg(s, 4);
	d = rmw(d, 4, 4);

	AND_rri(REG_WORK1, s, 31);
	MOV_ri(REG_WORK2, 1);

	BIC_rrrLSLr(d, d, REG_WORK2, REG_WORK1);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jnf_BCLR_l,(RW4 d, RR4 s))

MIDFUNC(2,jff_BCLR_b_imm,(RW4 d, IMM s))
{
	d = rmw(d, 4, 4);

	uae_u32 v = (1 << s);
	MRS_CPSR(REG_WORK1);
	TST_ri(d, v);
	CC_BIC_rri(NATIVE_CC_NE, REG_WORK1, REG_WORK1, ARM_Z_FLAG);
	CC_ORR_rri(NATIVE_CC_EQ, REG_WORK1, REG_WORK1, ARM_Z_FLAG);
	MSR_CPSR_r(REG_WORK1);
	BIC_rri(d, d, v);

	unlock2(d);
}
MENDFUNC(2,jff_BCLR_b_imm,(RW4 d, IMM s))

MIDFUNC(2,jff_BCLR_l_imm,(RW4 d, IMM s))
{
	d = rmw(d, 4, 4);

	uae_u32 v = (1 << s);
	MRS_CPSR(REG_WORK1);
	TST_ri(d, v);
	CC_BIC_rri(NATIVE_CC_NE, REG_WORK1, REG_WORK1, ARM_Z_FLAG);
	CC_ORR_rri(NATIVE_CC_EQ, REG_WORK1, REG_WORK1, ARM_Z_FLAG);
	MSR_CPSR_r(REG_WORK1);
	BIC_rri(d, d, v);

	unlock2(d);
}
MENDFUNC(2,jff_BCLR_l_imm,(RW4 d, IMM s))

MIDFUNC(2,jff_BCLR_b,(RW4 d, RR4 s))
{
	if (isconst(s)) {
		COMPCALL(jff_BCLR_b_imm)(d, live.state[s].val & 7);
		return;
	}
	s = readreg(s, 4);
	d = rmw(d, 4, 4);

	AND_rri(REG_WORK1, s, 7);
	MOV_ri(REG_WORK2, 1);
	LSL_rrr(REG_WORK2, REG_WORK2, REG_WORK1);

	MRS_CPSR(REG_WORK1);
	TST_rr(d,REG_WORK2);
	CC_BIC_rri(NATIVE_CC_NE, REG_WORK1, REG_WORK1, ARM_Z_FLAG);
	CC_ORR_rri(NATIVE_CC_EQ, REG_WORK1, REG_WORK1, ARM_Z_FLAG);
	MSR_CPSR_r(REG_WORK1);
	BIC_rrr(d, d, REG_WORK2);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jff_BCLR_b,(RW4 d, RR4 s))

MIDFUNC(2,jff_BCLR_l,(RW4 d, RR4 s))
{
	if (isconst(s)) {
		COMPCALL(jff_BCLR_l_imm)(d, live.state[s].val & 31);
		return;
	}

	s = readreg(s, 4);
	d = rmw(d, 4, 4);

	AND_rri(REG_WORK1, s, 31);
	MOV_ri(REG_WORK2, 1);
	LSL_rrr(REG_WORK2, REG_WORK2, REG_WORK1);

	MRS_CPSR(REG_WORK1);
	TST_rr(d, REG_WORK2);
	CC_BIC_rri(NATIVE_CC_NE, REG_WORK1, REG_WORK1, ARM_Z_FLAG);
	CC_ORR_rri(NATIVE_CC_EQ, REG_WORK1, REG_WORK1, ARM_Z_FLAG);
	MSR_CPSR_r(REG_WORK1);
	BIC_rrr(d, d ,REG_WORK2);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jff_BCLR_l,(RW4 d, RR4 s))

/*
 * BSET
 * Operand Syntax: 	Dn,<ea>
 *					#<data>,<ea>
 *
 *  Operand Size: 8,32
 *
 * X Not affected.
 * N Not affected.
 * Z Set if the bit tested was zero. Cleared otherwise.
 * V Not affected.
 * C Not affected.
 *
 */
MIDFUNC(2,jnf_BSET_b_imm,(RW4 d, IMM s))
{
	d = rmw(d, 4, 4);
	ORR_rri(d, d, (1 << s));
	unlock2(d);
}
MENDFUNC(2,jnf_BSET_b_imm,(RW4 d, IMM s))

MIDFUNC(2,jnf_BSET_l_imm,(RW4 d, IMM s))
{
	d = rmw(d, 4, 4);
	ORR_rri(d, d, (1 << s));
	unlock2(d);
}
MENDFUNC(2,jnf_BSET_l_imm,(RW4 d, IMM s))

MIDFUNC(2,jnf_BSET_b,(RW4 d, RR4 s))
{
	if (isconst(s)) {
		COMPCALL(jnf_BSET_b_imm)(d, live.state[s].val & 7);
		return;
	}
	s = readreg(s, 4);
	d = rmw(d, 4 ,4);

	AND_rri(REG_WORK1, s, 7);
	MOV_ri(REG_WORK2, 1);

	ORR_rrrLSLr(d, d, REG_WORK2, REG_WORK1);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jnf_BSET_b,(RW4 d, RR4 s))

MIDFUNC(2,jnf_BSET_l,(RW4 d, RR4 s))
{
	if (isconst(s)) {
		COMPCALL(jnf_BSET_l_imm)(d, live.state[s].val & 31);
		return;
	}

	s = readreg(s, 4);
	d = rmw(d, 4, 4);

	AND_rri(REG_WORK1, s, 31);
	MOV_ri(REG_WORK2, 1);

	ORR_rrrLSLr(d, d, REG_WORK2, REG_WORK1);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jnf_BSET_l,(RW4 d, RR4 s))

MIDFUNC(2,jff_BSET_b_imm,(RW4 d, IMM s))
{
	d = rmw(d, 4, 4);

	uae_u32 v = (1 << s);
	MRS_CPSR(REG_WORK1);
	TST_ri(d, v);
	CC_BIC_rri(NATIVE_CC_NE, REG_WORK1, REG_WORK1, ARM_Z_FLAG);
	CC_ORR_rri(NATIVE_CC_EQ, REG_WORK1, REG_WORK1, ARM_Z_FLAG);
	MSR_CPSR_r(REG_WORK1);
	ORR_rri(d, d, v);

	unlock2(d);
}
MENDFUNC(2,jff_BSET_b_imm,(RW4 d, IMM s))

MIDFUNC(2,jff_BSET_l_imm,(RW4 d, IMM s))
{
	d = rmw(d, 4, 4);

	uae_u32 v = (1 << s);
	MRS_CPSR(REG_WORK1);
	TST_ri(d, v);
	CC_BIC_rri(NATIVE_CC_NE, REG_WORK1, REG_WORK1, ARM_Z_FLAG);
	CC_ORR_rri(NATIVE_CC_EQ, REG_WORK1, REG_WORK1, ARM_Z_FLAG);
	MSR_CPSR_r(REG_WORK1);
	ORR_rri(d, d, v);

	unlock2(d);
}
MENDFUNC(2,jff_BSET_l_imm,(RW4 d, IMM s))

MIDFUNC(2,jff_BSET_b,(RW4 d, RR4 s))
{
	if (isconst(s)) {
		COMPCALL(jff_BSET_b_imm)(d, live.state[s].val & 7);
		return;
	}
	s = readreg(s, 4);
	d = rmw(d, 4, 4);

	AND_rri(REG_WORK1, s, 7);
	MOV_ri(REG_WORK2, 1);
	LSL_rrr(REG_WORK2, REG_WORK2, REG_WORK1);

	MRS_CPSR(REG_WORK1);
	TST_rr(d, REG_WORK2);
	CC_BIC_rri(NATIVE_CC_NE, REG_WORK1, REG_WORK1, ARM_Z_FLAG);
	CC_ORR_rri(NATIVE_CC_EQ, REG_WORK1, REG_WORK1, ARM_Z_FLAG);
	MSR_CPSR_r(REG_WORK1);
	ORR_rrr(d, d, REG_WORK2);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jff_BSET_b,(RW4 d, RR4 s))

MIDFUNC(2,jff_BSET_l,(RW4 d, RR4 s))
{
	if (isconst(s)) {
		COMPCALL(jff_BSET_l_imm)(d, live.state[s].val & 31);
		return;
	}

	s = readreg(s, 4);
	d = rmw(d, 4, 4);

	AND_rri(REG_WORK1, s, 31);
	MOV_ri(REG_WORK2, 1);
	LSL_rrr(REG_WORK2, REG_WORK2, REG_WORK1);

	MRS_CPSR(REG_WORK1);
	TST_rr(d, REG_WORK2);
	CC_BIC_rri(NATIVE_CC_NE, REG_WORK1, REG_WORK1, ARM_Z_FLAG);
	CC_ORR_rri(NATIVE_CC_EQ, REG_WORK1, REG_WORK1, ARM_Z_FLAG);
	MSR_CPSR_r(REG_WORK1);
	ORR_rrr(d, d, REG_WORK2);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jff_BSET_l,(RW4 d, RR4 s))

/*
 * BTST
 * Operand Syntax: 	Dn,<ea>
 *					#<data>,<ea>
 *
 *  Operand Size: 8,32
 *
 * X Not affected
 * N Not affected
 * Z Set if the bit tested is zero. Cleared otherwise
 * V Not affected
 * C Not affected
 *
 */
MIDFUNC(2,jff_BTST_b_imm,(RR4 d, IMM s))
{
	d = readreg(d, 4);

	MRS_CPSR(REG_WORK1);
	TST_ri(d, (1 << s));
	CC_BIC_rri(NATIVE_CC_NE, REG_WORK1, REG_WORK1, ARM_Z_FLAG);
	CC_ORR_rri(NATIVE_CC_EQ, REG_WORK1, REG_WORK1, ARM_Z_FLAG);
	MSR_CPSR_r(REG_WORK1);

	unlock2(d);
}
MENDFUNC(2,jff_BTST_b_imm,(RR4 d, IMM s))

MIDFUNC(2,jff_BTST_l_imm,(RR4 d, IMM s))
{
	d = readreg(d, 4);

	MRS_CPSR(REG_WORK1);
	TST_ri(d, (1 << s));
	CC_BIC_rri(NATIVE_CC_NE, REG_WORK1, REG_WORK1, ARM_Z_FLAG);
	CC_ORR_rri(NATIVE_CC_EQ, REG_WORK1, REG_WORK1, ARM_Z_FLAG);
	MSR_CPSR_r(REG_WORK1);

	unlock2(d);
}
MENDFUNC(2,jff_BTST_l_imm,(RR4 d, IMM s))

MIDFUNC(2,jff_BTST_b,(RR4 d, RR4 s))
{
	if (isconst(s)) {
		COMPCALL(jff_BTST_b_imm)(d, live.state[s].val & 7);
		return;
	}
	s = readreg(s, 4);
	d = readreg(d, 4);

	AND_rri(REG_WORK1, s, 7);
	MOV_ri(REG_WORK2, 1);
	LSL_rrr(REG_WORK2, REG_WORK2, REG_WORK1);

	MRS_CPSR(REG_WORK1);
	TST_rr(d, REG_WORK2);
	CC_BIC_rri(NATIVE_CC_NE, REG_WORK1, REG_WORK1, ARM_Z_FLAG);
	CC_ORR_rri(NATIVE_CC_EQ, REG_WORK1, REG_WORK1, ARM_Z_FLAG);
	MSR_CPSR_r(REG_WORK1);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jff_BTST_b,(RR4 d, RR4 s))

MIDFUNC(2,jff_BTST_l,(RR4 d, RR4 s))
{
	if (isconst(s)) {
		COMPCALL(jff_BTST_l_imm)(d, live.state[s].val & 31);
		return;
	}

	s = readreg(s ,4);
	d = readreg(d, 4);

	AND_rri(REG_WORK1, s, 31);
	MOV_ri(REG_WORK2, 1);
	LSL_rrr(REG_WORK2, REG_WORK2, REG_WORK1);

	MRS_CPSR(REG_WORK1);
	TST_rr(d, REG_WORK2);
	CC_BIC_rri(NATIVE_CC_NE, REG_WORK1, REG_WORK1, ARM_Z_FLAG);
	CC_ORR_rri(NATIVE_CC_EQ, REG_WORK1, REG_WORK1, ARM_Z_FLAG);
	MSR_CPSR_r(REG_WORK1);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jff_BTST_l,(RR4 d, RR4 s))

/*
 * CLR
 * Operand Syntax: <ea>
 *
 * Operand Size: 8,16,32
 *
 * X Not affected.
 * N Always cleared.
 * Z Always set.
 * V Always cleared.
 * C Always cleared.
 *
 */
MIDFUNC(1,jnf_CLR,(W4 d))
{
	d = writereg(d, 4);
	MOV_ri(d, 0);
	unlock2(d);
}
MENDFUNC(1,jnf_CLR,(W4 d))

MIDFUNC(1,jff_CLR,(W4 d))
{
	d = writereg(d, 4);
	MOV_ri(d, 0);
	MSR_CPSR_i(ARM_Z_FLAG);
	unlock2(d);
}
MENDFUNC(1,jff_CLR,(W4 d))

/*
 * CMP
 * Operand Syntax: <ea>, Dn
 *
 * Operand Size: 8,16,32
 *
 * X Not affected.
 * N Set if the result is negative. Cleared otherwise.
 * Z Set if the result is zero. Cleared otherwise.
 * V Set if an overflow occurs. Cleared otherwise.
 * C Set if a borrow occurs. Cleared otherwise.
 *
 */
MIDFUNC(2,jff_CMP_b,(RR1 d, RR1 s))
{
	d = readreg(d, 4);
	s = readreg(s, 4);

	LSL_rri(REG_WORK1, d, 24);
	CMP_rrLSLi(REG_WORK1, s, 24);

	MRS_CPSR(REG_WORK1);
	EOR_rri(REG_WORK1, REG_WORK1, ARM_C_FLAG);
	MSR_CPSR_r(REG_WORK1);

	unlock2(s);
	unlock2(d);
}
MENDFUNC(2,jff_CMP_b,(RR1 d, RR1 s))

MIDFUNC(2,jff_CMP_w,(RR2 d, RR2 s))
{
	d = readreg(d, 4);
	s = readreg(s, 4);

	LSL_rri(REG_WORK1, d, 16);
	CMP_rrLSLi(REG_WORK1, s, 16);

	MRS_CPSR(REG_WORK1);
	EOR_rri(REG_WORK1, REG_WORK1, ARM_C_FLAG);
	MSR_CPSR_r(REG_WORK1);

	unlock2(s);
	unlock2(d);
}
MENDFUNC(2,jff_CMP_w,(RR2 d, RR2 s))

MIDFUNC(2,jff_CMP_l,(RR4 d, RR4 s))
{
	d = readreg(d, 4);
	s = readreg(s, 4);

	CMP_rr(d, s);

	MRS_CPSR(REG_WORK1);
	EOR_rri(REG_WORK1, REG_WORK1, ARM_C_FLAG);
	MSR_CPSR_r(REG_WORK1);

	unlock2(s);
	unlock2(d);
}
MENDFUNC(2,jff_CMP_l,(RR4 d, RR4 s))

/*
 * CMPA
 * Operand Syntax: 	<ea>, An
 *
 * Operand Size: 16,32
 *
 * X Not affected.
 * N Set if the result is negative. Cleared otherwise.
 * Z Set if the result is zero. Cleared otherwise.
 * V Set if an overflow occurs. Cleared otherwise.
 * C Set if a borrow occurs. Cleared otherwise.
 *
 */
MIDFUNC(2,jff_CMPA_b,(RR1 d, RR1 s))
{
	d = readreg(d, 4);
	s = readreg(s, 4);

	LSL_rri(REG_WORK1, d, 24);
	CMP_rrLSLi(REG_WORK1, s, 24);

	MRS_CPSR(REG_WORK1);
	EOR_rri(REG_WORK1, REG_WORK1, ARM_C_FLAG);
	MSR_CPSR_r(REG_WORK1);

	unlock2(s);
	unlock2(d);
}
MENDFUNC(2,jff_CMPA_b,(RR1 d, RR1 s))

MIDFUNC(2,jff_CMPA_w,(RR2 d, RR2 s))
{
	d = readreg(d, 4);
	s = readreg(s, 4);

	LSL_rri(REG_WORK1, d, 16);
	CMP_rrLSLi(REG_WORK1, s, 16);

	MRS_CPSR(REG_WORK1);
	EOR_rri(REG_WORK1, REG_WORK1, ARM_C_FLAG);
	MSR_CPSR_r(REG_WORK1);

	unlock2(s);
	unlock2(d);
}
MENDFUNC(2,jff_CMPA_w,(RR2 d, RR2 s))

MIDFUNC(2,jff_CMPA_l,(RR4 d, RR4 s))
{
	d = readreg(d, 4);
	s = readreg(s, 4);

	CMP_rr(d, s);

	MRS_CPSR(REG_WORK1);
	EOR_rri(REG_WORK1, REG_WORK1, ARM_C_FLAG);
	MSR_CPSR_r(REG_WORK1);

	unlock2(s);
	unlock2(d);
}
MENDFUNC(2,jff_CMPA_l,(RR4 d, RR4 s))

/*
 * EOR
 * Operand Syntax: 	Dn, <ea>
 *
 * Operand Size: 8,16,32
 *
 * X Not affected.
 * N Set if the most significant bit of the result is set. Cleared otherwise.
 * Z Set if the result is zero. Cleared otherwise.
 * V Always cleared.
 * C Always cleared.
 *
 */
MIDFUNC(3,jnf_EOR,(W4 d, RR4 s, RR4 v))
{
	if (isconst(s) && isconst(v)) {
		set_const(d, live.state[s].val ^ live.state[v].val);
		return;
	}

	v = readreg(v, 4);
	s = readreg(s, 4);
	d = writereg(d, 4);

	EOR_rrr(d, s, v);

	unlock2(v);
	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jnf_EOR,(RW4 d, RR4 s, RR4 v))

MIDFUNC(3,jff_EOR_b,(W4 d, RR1 s, RR1 v))
{
	v = readreg(v, 4);
	s = readreg(s, 4);
	d = writereg(d, 4);

	SIGNED8_REG_2_REG(REG_WORK1, s);
	SIGNED8_REG_2_REG(REG_WORK2, v);
	MSR_CPSRf_i(0);
	EORS_rrr(d, REG_WORK1, REG_WORK2);

	unlock2(v);
	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jff_EOR_b,(RW4 d, RR1 s, RR1 v))

MIDFUNC(3,jff_EOR_w,(W4 d, RR2 s, RR2 v))
{
	v = readreg(v, 4);
	s = readreg(s, 4);
	d = writereg(d, 4);

	SIGNED16_REG_2_REG(REG_WORK1, s);
	SIGNED16_REG_2_REG(REG_WORK2, v);
	MSR_CPSRf_i(0);
	EORS_rrr(d, REG_WORK1, REG_WORK2);

	unlock2(v);
	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jff_EOR_w,(RW4 d, RR2 s, RR2 v))

MIDFUNC(3,jff_EOR_l,(W4 d, RR4 s, RR4 v))
{
	v = readreg(v, 4);
	s = readreg(s, 4);
	d = writereg(d, 4);

	MSR_CPSRf_i(0);
	EORS_rrr(d, s, v);

	unlock2(v);
	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jff_EOR_l,(RW4 d, RR4 s, RR4 v))

/*
 * EORI
 * Operand Syntax: 	#<data>, CCR
 *
 * Operand Size: 8
 *
 * X — Changed if bit 4 of immediate operand is one; unchanged otherwise.
 * N — Changed if bit 3 of immediate operand is one; unchanged otherwise.
 * Z — Changed if bit 2 of immediate operand is one; unchanged otherwise.
 * V — Changed if bit 1 of immediate operand is one; unchanged otherwise.
 * C — Changed if bit 0 of immediate operand is one; unchanged otherwise.
 *
 */
MIDFUNC(1,jff_EORSR,(IMM s, IMM x))
{
	MRS_CPSR(REG_WORK1);
	EOR_rri(REG_WORK1, REG_WORK1, s);
	MSR_CPSRf_r(REG_WORK1);

	if (x) {
		compemu_raw_mov_l_ri(REG_WORK1, (uintptr)live.state[FLAGX].mem);
		LDRB_rR(REG_WORK2, REG_WORK1);
		EOR_rri(REG_WORK2, REG_WORK2, 1);
		STRB_rR(REG_WORK2, REG_WORK1);
	}
}
MENDFUNC(1,jff_EORSR,(IMM s))

/*
 * EXT
 * Operand Syntax: <ea>
 *
 * Operand Size: 16,32
 *
 * X Not affected.
 * N Set if the result is negative. Cleared otherwise.
 * Z Set if the result is zero. Cleared otherwise.
 * V Always cleared.
 * C Always cleared.
 *
 */
MIDFUNC(2,jnf_EXT_b,(W4 d, RR4 s))
{
	if (isconst(s)) {
		set_const(d, (uae_s32)(uae_s8)live.state[s].val);
		return;
	}

	s = readreg(s, 4);
	d = writereg(d, 4);

	SIGNED8_REG_2_REG(d, s);

	unlock2(s);
	unlock2(d);
}
MENDFUNC(2,jnf_EXT_b,(W4 d, RR4 s))

MIDFUNC(2,jnf_EXT_w,(W4 d, RR4 s))
{
	if (isconst(s)) {
		set_const(d, (uae_s32)(uae_s8)live.state[s].val);
		return;
	}

	s = readreg(s, 4);
	d = writereg(d, 4);

	SIGNED8_REG_2_REG(d, s);

	unlock2(s);
	unlock2(d);
}
MENDFUNC(2,jnf_EXT_w,(W4 d, RR4 s))

MIDFUNC(2,jnf_EXT_l,(W4 d, RR4 s))
{
	if (isconst(s)) {
		set_const(d, (uae_s32)(uae_s16)live.state[s].val);
		return;
	}

	s = readreg(s, 4);
	d = writereg(d, 4);

	SIGNED16_REG_2_REG(d, s);

	unlock2(s);
	unlock2(d);
}
MENDFUNC(2,jnf_EXT_l,(W4 d, RR4 s))

MIDFUNC(2,jff_EXT_b,(W4 d, RR4 s))
{
	if (isconst(s)) {
		d = writereg(d, 4);
		SIGNED8_IMM_2_REG(d, (uae_u8)live.state[s].val);
	} else {
		s = readreg(s, 4);
		d = writereg(d, 4);
		SIGNED8_REG_2_REG(d, s);
		unlock2(s);
	}

	MSR_CPSRf_i(0);
	TST_rr(d, d);

	unlock2(d);
}
MENDFUNC(2,jff_EXT_b,(W4 d, RR4 s))

MIDFUNC(2,jff_EXT_w,(W4 d, RR4 s))
{
	if (isconst(s)) {
		d = writereg(d, 4);
		SIGNED8_IMM_2_REG(d, (uae_u8)live.state[s].val);
	} else {
		s = readreg(s, 4);
		d = writereg(d, 4);
		SIGNED8_REG_2_REG(d, s);
		unlock2(s);
	}

	MSR_CPSRf_i(0);
	TST_rr(d, d);

	unlock2(d);
}
MENDFUNC(2,jff_EXT_w,(W4 d, RR4 s))

MIDFUNC(2,jff_EXT_l,(W4 d, RR4 s))
{
	if (isconst(s)) {
		d = writereg(d, 4);
		SIGNED16_IMM_2_REG(d, (uae_u16)live.state[s].val);
	} else {
		s = readreg(s, 4);
		d = writereg(d, 4);
		SIGNED16_REG_2_REG(d, s);
		unlock2(s);
	}
	MSR_CPSRf_i(0);
	TST_rr(d, d);

	unlock2(d);
}
MENDFUNC(2,jff_EXT_l,(W4 d, RR4 s))

/*
 * LSL
 * Operand Syntax: 	Dx, Dy
 * 					#<data>, Dy
 *					<ea>
 *
 * Operand Size: 8,16,32
 *
 * X Set according to the last bit shifted out of the operand. Unaffected for a shift count of zero.
 * N Set if the result is negative. Cleared otherwise.
 * Z Set if the result is zero. Cleared otherwise.
 * V Always cleared.
 * C Set according to the last bit shifted out of the operand. Cleared for a shift count of zero.
 *
 */
// ToDo: X unaffected for a shift count of zero. Currently duplicate_carry() is used by caller in all cases.
MIDFUNC(3,jnf_LSL_imm,(W4 d, RR4 s, IMM i))
{
	if (!i) return;

	s = readreg(s, 4);
	d = writereg(d, 4);

	LSL_rri(d, s, i);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jnf_LSL_imm,(W4 d, RR4 s, IMM i))

MIDFUNC(3,jnf_LSL_reg,(W4 d, RR4 s, RR4 i))
{
	i = readreg(i, 4);
	s = readreg(s, 4);
	d = writereg(d, 4);

	AND_rri(REG_WORK1, i, 63);
	LSL_rrr(d, s, REG_WORK1);

	unlock2(d);
	unlock2(s);
	unlock2(i);
}
MENDFUNC(3,jnf_LSL_reg,(W4 d, RR4 s, RR4 i))

MIDFUNC(3,jff_LSL_b_imm,(W4 d, RR4 s, IMM i))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	MSR_CPSRf_i(0);

	LSL_rri(d, s, 24);
	if (i) {
		LSLS_rri(d, d, i);
	} else {
		TST_rr(d, d);
	}
	LSR_rri(d, d, 24);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jff_LSL_b_imm,(W4 d, RR4 s, IMM i))

MIDFUNC(3,jff_LSL_w_imm,(W4 d, RR4 s, IMM i))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	MSR_CPSRf_i(0);

	LSL_rri(d, s, 16);
	if (i) {
		LSLS_rri(d, d, i);
	} else {
		TST_rr(d, d);
	}
	LSR_rri(d, d, 16);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jff_LSL_w_imm,(W4 d, RR4 s, IMM i))

MIDFUNC(3,jff_LSL_l_imm,(W4 d, RR4 s, IMM i))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	MSR_CPSRf_i(0);
	if (i) {
		LSLS_rri(d, s, i);
	} else {
		MOVS_rr(d, s);
	}

	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jff_LSL_l_imm,(W4 d, RR4 s, IMM i))

MIDFUNC(3,jff_LSL_b_reg,(W4 d, RR4 s, RR4 i))
{
	i = readreg(i, 4);
	s = readreg(s, 4);
	d = writereg(d, 4);

	MSR_CPSRf_i(0);
	LSL_rri(d, s, 24);
	AND_rri(REG_WORK1, i, 63);
	LSLS_rrr(d, d, REG_WORK1);
	LSR_rri(d, d, 24);

	unlock2(d);
	unlock2(s);
	unlock2(i);
}
MENDFUNC(3,jff_LSL_b_reg,(W4 d, RR4 s, RR4 i))

MIDFUNC(3,jff_LSL_w_reg,(W4 d, RR4 s, RR4 i))
{
	i = readreg(i, 4);
	s = readreg(s, 4);
	d = writereg(d, 4);

	MSR_CPSRf_i(0);
	LSL_rri(d, s, 16);
	AND_rri(REG_WORK1, i, 63);
	LSLS_rrr(d, d, REG_WORK1);
	LSR_rri(d, d, 16);

	unlock2(d);
	unlock2(s);
	unlock2(i);
}
MENDFUNC(3,jff_LSL_w_reg,(W4 d, RR4 s, RR4 i))

MIDFUNC(3,jff_LSL_l_reg,(W4 d, RR4 s, RR4 i))
{
	i = readreg(i, 4);
	s = readreg(s, 4);
	d = writereg(d, 4);

	MSR_CPSRf_i(0);
	AND_rri(REG_WORK1, i, 63);
	LSLS_rrr(d, s, REG_WORK1);

	unlock2(d);
	unlock2(s);
	unlock2(i);
}
MENDFUNC(3,jff_LSL_l_reg,(W4 d, RR4 s, RR4 i))

/*
 * LSLW
 * Operand Syntax: 	<ea>
 *
 * Operand Size: 16
 *
 * X Set according to the last bit shifted out of the operand. Unaffected for a shift count of zero.
 * N Set if the result is negative. Cleared otherwise.
 * Z Set if the result is zero. Cleared otherwise.
 * V Always cleared.
 * C Set according to the last bit shifted out of the operand. Cleared for a shift count of zero.
 *
 */
MIDFUNC(2,jnf_LSLW,(W4 d, RR4 s))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	LSL_rri(d, s, 1);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jnf_LSLW,(W4 d, RR4 s))

MIDFUNC(2,jff_LSLW,(W4 d, RR4 s))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	MSR_CPSRf_i(0);
	LSLS_rri(d, s, 17);
	LSR_rri(d, d, 16);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jff_LSLW,(W4 d, RR4 s))

/*
 * LSR
 * Operand Syntax: 	Dx, Dy
 * 					#<data>, Dy
 *					<ea>
 *
 * Operand Size: 8,16,32
 *
 * X Set according to the last bit shifted out of the operand. Unaffected for a shift count of zero.
 * N Set if the result is negative. Cleared otherwise.
 * Z Set if the result is zero. Cleared otherwise.
 * V Always cleared.
 * C Set according to the last bit shifted out of the operand. Cleared for a shift count of zero.
 *
 */
// ToDo: X unaffected for a shift count of zero. Currently duplicate_carry() is used by caller in all cases.
MIDFUNC(3,jnf_LSR_b_imm,(W4 d, RR4 s, IMM i))
{
	int isrmw;

	if (!i)
	  return;

	isrmw = (s == d);
	if (!isrmw) {
		s = readreg(s, 4);
		d = writereg(d, 4);
	}
	else {
		s = d = rmw(s, 4, 4);
	}

	UNSIGNED8_REG_2_REG(d, s);
	LSR_rri(d, d, i);

	if (!isrmw) {
		unlock2(d);
		unlock2(s);
	}
	else {
		unlock2(s);
	}
}
MENDFUNC(3,jnf_LSR_b_imm,(W4 d, RR4 s, IMM i))

MIDFUNC(3,jnf_LSR_w_imm,(W4 d, RR4 s, IMM i))
{
	int isrmw;

	if (!i)
	  return;

	isrmw = (s == d);
	if (!isrmw) {
		s = readreg(s, 4);
		d = writereg(d, 4);
	}
	else {
		s = d = rmw(s, 4, 4);
	}

	UNSIGNED16_REG_2_REG(d, s);
	LSR_rri(d, d, i);

	if (!isrmw) {
		unlock2(d);
		unlock2(s);
	}
	else {
		unlock2(s);
	}
}
MENDFUNC(3,jnf_LSR_w_imm,(W4 d, RR4 s, IMM i))

MIDFUNC(3,jnf_LSR_l_imm,(W4 d, RR4 s, IMM i))
{
	int isrmw;

	if (!i)
	  return;

	isrmw = (s == d);
	if (!isrmw) {
		s = readreg(s, 4);
		d = writereg(d, 4);
	}
	else {
		s = d = rmw(s, 4, 4);
	}

	LSR_rri(d, s, i);

	if (!isrmw) {
		unlock2(d);
		unlock2(s);
	}
	else {
		unlock2(s);
	}
}
MENDFUNC(3,jnf_LSR_l_imm,(W4 d, RR4 s, IMM i))

MIDFUNC(3,jff_LSR_b_imm,(W4 d, RR4 s, IMM i))
{
	int isrmw;

	isrmw = (s == d);
	if (!isrmw) {
		s = readreg(s, 4);
		d = writereg(d, 4);
	}
	else {
		s = d = rmw(s, 4, 4);
	}

	MSR_CPSRf_i(0);
	if (i) {
  	UNSIGNED8_REG_2_REG(d, s);
		LSRS_rri(d, d, i);
	} else {
	  SIGNED8_REG_2_REG(d, s);
		TST_rr(d, d);
	}
	if (!isrmw) {
		unlock2(d);
		unlock2(s);
	}
	else {
		unlock2(s);
	}
}
MENDFUNC(3,jff_LSR_b_imm,(W4 d, RR4 s, IMM i))

MIDFUNC(3,jff_LSR_w_imm,(W4 d, RR4 s, IMM i))
{
	int isrmw;

	isrmw = (s == d);
	if (!isrmw) {
		s = readreg(s, 4);
		d = writereg(d, 4);
	}
	else {
		s = d = rmw(s, 4, 4);
	}

	MSR_CPSRf_i(0);
	if (i) {
  	UNSIGNED16_REG_2_REG(d, s);
		LSRS_rri(d, d, i);
	} else {
	  SIGNED16_REG_2_REG(d, s);
		TST_rr(d, d);
	}
	if (!isrmw) {
		unlock2(d);
		unlock2(s);
	}
	else {
		unlock2(s);
	}
}
MENDFUNC(3,jff_LSR_w_imm,(W4 d, RR4 s, IMM i))

MIDFUNC(3,jff_LSR_l_imm,(W4 d, RR4 s, IMM i))
{
	int isrmw;

	isrmw = (s == d);
	if (!isrmw) {
		s = readreg(s, 4);
		d = writereg(d, 4);
	}
	else {
		s = d = rmw(s, 4, 4);
	}

	MSR_CPSRf_i(0);
	if (i) {
		LSRS_rri(d, s, i);
	} else {
		MOVS_rr(d, s);
	}
	if (!isrmw) {
		unlock2(d);
		unlock2(s);
	}
	else {
		unlock2(s);
	}
}
MENDFUNC(3,jff_LSR_l_imm,(W4 d, RR4 s, IMM i))

MIDFUNC(3,jnf_LSR_b_reg,(W4 d, RR4 s, RR4 i))
{
	int isrmw;

	i = readreg(i, 4);
	isrmw = (s == d);
	if (!isrmw) {
		s = readreg(s, 4);
		d = writereg(d, 4);
	}
	else {
		s = d = rmw(s, 4, 4);
	}

	UNSIGNED8_REG_2_REG(d, s);
	AND_rri(REG_WORK1, i, 63);
	LSR_rrr(d, d, REG_WORK1);

	if (!isrmw) {
		unlock2(d);
		unlock2(s);
	}
	else {
		unlock2(s);
	}
	unlock2(i);
}
MENDFUNC(3,jnf_LSR_b_reg,(W4 d, RR4 s, RR4 i))

MIDFUNC(3,jnf_LSR_w_reg,(W4 d, RR4 s, RR4 i))
{
	int isrmw;

	i = readreg(i, 4);
	isrmw = (s == d);
	if (!isrmw) {
		s = readreg(s, 4);
		d = writereg(d, 4);
	}
	else {
		s = d = rmw(s, 4, 4);
	}

	UNSIGNED16_REG_2_REG(d, s);
	AND_rri(REG_WORK1, i, 63);
	LSR_rrr(d, d, REG_WORK1);

	if (!isrmw) {
		unlock2(d);
		unlock2(s);
	}
	else {
		unlock2(s);
	}
	unlock2(i);
}
MENDFUNC(3,jnf_LSR_w_reg,(W4 d, RR4 s, RR4 i))

MIDFUNC(3,jnf_LSR_l_reg,(W4 d, RR4 s, RR4 i))
{
	int isrmw;

	i = readreg(i, 4);
	isrmw = (s == d);
	if (!isrmw) {
		s = readreg(s, 4);
		d = writereg(d, 4);
	}
	else {
		s = d = rmw(s, 4, 4);
	}

	AND_rri(REG_WORK1, i, 63);
	LSR_rrr(d, s, REG_WORK1);

	if (!isrmw) {
		unlock2(d);
		unlock2(s);
	}
	else {
		unlock2(s);
	}
	unlock2(i);
}
MENDFUNC(3,jnf_LSR_l_reg,(W4 d, RR4 s, RR4 i))

MIDFUNC(3,jff_LSR_b_reg,(W4 d, RR4 s, RR4 i))
{
	int isrmw;

	i = readreg(i, 4);
	isrmw = (s == d);
	if (!isrmw) {
		s = readreg(s, 4);
		d = writereg(d, 4);
	}
	else {
		s = d = rmw(s, 4, 4);
	}

	SIGNED8_REG_2_REG(d, s);              // Make sure, sign is in MSB if shift count is 0 (to get correct N flag)
	ANDS_rri(REG_WORK1, i, 63);
  CC_AND_rri(NATIVE_CC_NE, d, d, 0xff); // Shift count is not 0 -> unsigned required
	MSR_CPSRf_i(0);
	LSRS_rrr(d, d, REG_WORK1);

	if (!isrmw) {
		unlock2(d);
		unlock2(s);
	}
	else {
		unlock2(s);
	}
	unlock2(i);
}
MENDFUNC(3,jff_LSR_b_reg,(W4 d, RR4 s, RR4 i))

MIDFUNC(3,jff_LSR_w_reg,(W4 d, RR4 s, RR4 i))
{
	int isrmw;

	i = readreg(i, 4);
	isrmw = (s == d);
	if (!isrmw) {
		s = readreg(s, 4);
		d = writereg(d, 4);
	}
	else {
		s = d = rmw(s, 4, 4);
	}

	SIGNED16_REG_2_REG(d, s);       // Make sure, sign is in MSB if shift count is 0 (to get correct N flag)
	ANDS_rri(REG_WORK1, i, 63);
#if defined(ARMV6_ASSEMBLY)
	CC_UXTH_rr(NATIVE_CC_NE, d, d); // Shift count is not 0 -> unsigned required
#else
	CC_LSL_rri(NATIVE_CC_NE, d, d, 16);
	CC_LSR_rri(NATIVE_CC_NE, d, d, 16);
#endif
  MSR_CPSRf_i(0); 
	LSRS_rrr(d, d, REG_WORK1);

	if (!isrmw) {
		unlock2(d);
		unlock2(s);
	}
	else {
		unlock2(s);
	}
	unlock2(i);
}
MENDFUNC(3,jff_LSR_w_reg,(W4 d, RR4 s, RR4 i))

MIDFUNC(3,jff_LSR_l_reg,(W4 d, RR4 s, RR4 i))
{
	int isrmw;

	i = readreg(i, 4);
	isrmw = (s == d);
	if (!isrmw) {
		s = readreg(s, 4);
		d = writereg(d, 4);
	}
	else {
		s = d = rmw(s, 4, 4);
	}

  MSR_CPSRf_i(0); 
	AND_rri(REG_WORK1, i, 63);
	LSRS_rrr(d, s, REG_WORK1);

	if (!isrmw) {
		unlock2(d);
		unlock2(s);
	}
	else {
		unlock2(s);
	}
	unlock2(i);
}
MENDFUNC(3,jff_LSR_l_reg,(W4 d, RR4 s, RR4 i))

/*
 * LSRW
 * Operand Syntax: 	<ea>
 *
 * Operand Size: 16
 *
 * X Set according to the last bit shifted out of the operand. Unaffected for a shift count of zero.
 * N Set if the result is negative. Cleared otherwise.
 * Z Set if the result is zero. Cleared otherwise.
 * V Always cleared.
 * C Set according to the last bit shifted out of the operand. Cleared for a shift count of zero.
 *
 */
MIDFUNC(2,jnf_LSRW,(W4 d, RR4 s))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	UNSIGNED16_REG_2_REG(d, s);
	LSR_rri(d, d, 1);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jnf_LSRW,(W4 d, RR4 s))

MIDFUNC(2,jff_LSRW,(W4 d, RR4 s))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	UNSIGNED16_REG_2_REG(d, s);
	MSR_CPSRf_i(0);
	LSR_rri(d, d, 1);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jff_LSRW,(W4 d, RR4 s))

/*
 * MOVE
 * Operand Syntax: <ea>, <ea>
 *
 * Operand Size: 8,16,32
 *
 * X Not affected.
 * N Set if the result is negative. Cleared otherwise.
 * Z Set if the result is zero. Cleared otherwise.
 * V Always cleared.
 * C Always cleared.
 *
 */
MIDFUNC(2,jnf_MOVE,(W4 d, RR4 s))
{
	if (isconst(s)) {
		set_const(d, live.state[s].val);
		return;
	}
	s = readreg(s, 4);
	d = writereg(d, 4);

	MOV_rr(d, s);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jnf_MOVE,(W4 d, RR4 s))

MIDFUNC(2,jff_MOVE_b_imm,(W4 d, IMM s))
{
	d = writereg(d, 4);

	SIGNED8_IMM_2_REG(d, (uae_u8)s);
	MSR_CPSRf_i(0);
	TST_rr(d, d);
  
	unlock2(d);
}
MENDFUNC(2,jff_MOVE_b_imm,(W4 d, IMM s))

MIDFUNC(2,jff_MOVE_w_imm,(W4 d, IMM s))
{
	d = writereg(d, 4);

	SIGNED16_IMM_2_REG(d, (uae_u16)s);
	MSR_CPSRf_i(0);
	TST_rr(d, d);

	unlock2(d);
}
MENDFUNC(2,jff_MOVE_w_imm,(W4 d, IMM s))

MIDFUNC(2,jff_MOVE_l_imm,(W4 d, IMM s))
{
	d = writereg(d, 4);

	compemu_raw_mov_l_ri(d, s);
	MSR_CPSRf_i(0);
	TST_rr(d, d);

	unlock2(d);
}
MENDFUNC(2,jff_MOVE_l_imm,(W4 d, IMM s))

MIDFUNC(2,jff_MOVE_b,(W4 d, RR1 s))
{
	if (isconst(s)) {
		COMPCALL(jff_MOVE_b_imm)(d, live.state[s].val);
		return;
	}

	s = readreg(s, 4);
	d = writereg(d, 4);

	SIGNED8_REG_2_REG(d, s);
	MSR_CPSRf_i(0);
	TST_rr(d, d);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jff_MOVE_b,(W4 d, RR1 s))

MIDFUNC(2,jff_MOVE_w,(W4 d, RR2 s))
{
	if (isconst(s)) {
		COMPCALL(jff_MOVE_w_imm)(d, live.state[s].val);
		return;
	}

	s = readreg(s, 4);
	d = writereg(d, 4);

	SIGNED16_REG_2_REG(d, s);
	MSR_CPSRf_i(0);
	TST_rr(d, d);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jff_MOVE_w,(W4 d, RR2 s))

MIDFUNC(2,jff_MOVE_l,(W4 d, RR4 s))
{
	if (isconst(s)) {
		COMPCALL(jff_MOVE_l_imm)(d, live.state[s].val);
		return;
	}

	s = readreg(s, 4);
	d = writereg(d, 4);

	MSR_CPSRf_i(0);
	MOVS_rr(d, s);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jff_MOVE_l,(W4 d, RR4 s))

/*
 * MOVE16
 *
 * Flags: Not affected.
 *
 */
MIDFUNC(2,jnf_MOVE16,(RR4 d, RR4 s))
{
	s = readreg(s, 4);
	d = readreg(d, 4);

	BIC_rri(s, s, 0x0000000F);
	BIC_rri(d, d, 0x0000000F);

	compemu_raw_mov_l_ri(REG_WORK1, (IMM)NATMEM_OFFSETX);
	ADD_rrr(s, s, REG_WORK1);
	ADD_rrr(d, d, REG_WORK1);

	LDR_rRI(REG_WORK1, s, 8);
	LDR_rRI(REG_WORK2, s, 12);
	PUSH_REGS((1<<REG_WORK1) | (1<<REG_WORK2)); // May be optimizable
	LDR_rR(REG_WORK1, s);
	LDR_rRI(REG_WORK2, s, 4);
	STR_rR(REG_WORK1, d);
	STR_rRI(REG_WORK2, d, 4);
	POP_REGS((1<<REG_WORK1) | (1<<REG_WORK2));
	STR_rRI(REG_WORK1, d, 8);
	STR_rRI(REG_WORK2, d, 12);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jnf_MOVE16,(RR4 d, RR4 s))

/*
 * MOVEA
 * Operand Syntax: 	<ea>, An
 *
 * Operand Size: 16,32
 *
 * Flags: Not affected.
 *
 */
MIDFUNC(2,jnf_MOVEA_w,(W4 d, RR2 s))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	SIGNED16_REG_2_REG(d, s);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jnf_MOVEA_w,(W4 d, RR2 s))

MIDFUNC(2,jnf_MOVEA_l,(W4 d, RR4 s))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	MOV_rr(d, s);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jnf_MOVEA_l,(W4 d, RR4 s))

/*
 * MULS
 * Operand Syntax: 	<ea>, Dn
 *
 * Operand Size: 16
 *
 * X Not affected.
 * N Set if the result is negative. Cleared otherwise.
 * Z Set if the result is zero. Cleared otherwise.
 * V Set if overflow. Cleared otherwise. (32 Bit multiply only)
 * C Always cleared.
 *
 */
MIDFUNC(2,jnf_MULS,(RW4 d, RR4 s))
{
	s = readreg(s, 4);
	d = rmw(d, 4, 4);

	SIGN_EXTEND_16_REG_2_REG(d, d);
	SIGN_EXTEND_16_REG_2_REG(REG_WORK1, s);
	MUL_rrr(d, d, REG_WORK1);

	unlock2(s);
	unlock2(d);
}
MENDFUNC(2,jnf_MULS,(RW4 d, RR4 s))

MIDFUNC(2,jff_MULS,(RW4 d, RR4 s))
{
	s = readreg(s, 4);
	d = rmw(d, 4, 4);

	SIGN_EXTEND_16_REG_2_REG(d, d);
	SIGN_EXTEND_16_REG_2_REG(REG_WORK1, s);

	MSR_CPSRf_i(0);
	MULS_rrr(d, d, REG_WORK1);

	unlock2(s);
	unlock2(d);
}
MENDFUNC(2,jff_MULS,(RW4 d, RR4 s))

MIDFUNC(2,jnf_MULS32,(RW4 d, RR4 s))
{
	s = readreg(s, 4);
	d = rmw(d, 4, 4);

	MUL_rrr(d, d, s);

	unlock2(s);
	unlock2(d);
}
MENDFUNC(2,jnf_MULS32,(RW4 d, RR4 s))

MIDFUNC(2,jff_MULS32,(RW4 d, RR4 s))
{
	s = readreg(s, 4);
	d = rmw(d, 4, 4);

	MSR_CPSRf_i(0);
	// L, H,
	SMULLS_rrrr(d, REG_WORK2, d, s);
	MRS_CPSR(REG_WORK1);
	TEQ_rrASRi(REG_WORK2, d, 31);
	CC_ORR_rri(NATIVE_CC_NE, REG_WORK1, REG_WORK1, ARM_V_FLAG);
	MSR_CPSRf_r(REG_WORK1);

	unlock2(s);
	unlock2(d);
}
MENDFUNC(2,jff_MULS32,(RW4 d, RR4 s))

MIDFUNC(2,jnf_MULS64,(RW4 d, RW4 s))
{
	s = rmw(s, 4, 4);
	d = rmw(d, 4, 4);

	// L, H,
	SMULL_rrrr(d, s, d, s);

	unlock2(s);
	unlock2(d);
}
MENDFUNC(2,jnf_MULS64,(RW4 d, RW4 s))

MIDFUNC(2,jff_MULS64,(RW4 d, RW4 s))
{
	s = rmw(s, 4, 4);
	d = rmw(d, 4, 4);

	MSR_CPSRf_i(0);
	// L, H,
	SMULLS_rrrr(d, s, d, s);
	MRS_CPSR(REG_WORK1);
	TEQ_rrASRi(s, d, 31);
	CC_ORR_rri(NATIVE_CC_NE, REG_WORK1, REG_WORK1, ARM_V_FLAG);
	MSR_CPSRf_r(REG_WORK1);

	unlock2(s);
	unlock2(d);
}
MENDFUNC(2,jff_MULS64,(RW4 d, RW4 s))

/*
 * MULU
 * Operand Syntax: 	<ea>, Dn
 *
 * Operand Size: 16
 *
 * X Not affected.
 * N Set if the result is negative. Cleared otherwise.
 * Z Set if the result is zero. Cleared otherwise.
 * V Set if overflow. Cleared otherwise. (32 Bit multiply only)
 * C Always cleared.
 *
 */
MIDFUNC(2,jnf_MULU,(RW4 d, RR4 s))
{
	s = readreg(s, 4);
	d = rmw(d, 4, 4);

	ZERO_EXTEND_16_REG_2_REG(d, d);
	ZERO_EXTEND_16_REG_2_REG(REG_WORK1, s);

	MUL_rrr(d, d, REG_WORK1);

	unlock2(s);
	unlock2(d);
}
MENDFUNC(2,jnf_MULU,(RW4 d, RR4 s))

MIDFUNC(2,jff_MULU,(RW4 d, RR4 s))
{
	s = readreg(s, 4);
	d = rmw(d, 4, 4);

	ZERO_EXTEND_16_REG_2_REG(d, d);
	ZERO_EXTEND_16_REG_2_REG(REG_WORK1, s);

	MSR_CPSRf_i(0);
	MULS_rrr(d, d, REG_WORK1);

	unlock2(s);
	unlock2(d);
}
MENDFUNC(2,jff_MULU,(RW4 d, RR4 s))

MIDFUNC(2,jnf_MULU32,(RW4 d, RR4 s))
{
	s = readreg(s, 4);
	d = rmw(d, 4, 4);

	MUL_rrr(d, d, s);

	unlock2(s);
	unlock2(d);
}
MENDFUNC(2,jnf_MULU32,(RW4 d, RR4 s))

MIDFUNC(2,jff_MULU32,(RW4 d, RR4 s))
{
	s = readreg(s, 4);
	d = rmw(d, 4, 4);

	// L, H,
	MSR_CPSRf_i(0);
	UMULLS_rrrr(d, REG_WORK2, d, s);
	MRS_CPSR(REG_WORK1);
	TST_rr(REG_WORK2, REG_WORK2);
	CC_ORR_rri(NATIVE_CC_NE, REG_WORK1, REG_WORK1, ARM_V_FLAG);
	MSR_CPSRf_r(REG_WORK1);

	unlock2(s);
	unlock2(d);
}
MENDFUNC(2,jff_MULU32,(RW4 d, RR4 s))

MIDFUNC(2,jnf_MULU64,(RW4 d, RW4 s))
{
	s = rmw(s, 4, 4);
	d = rmw(d, 4, 4);

	// L, H,
	UMULL_rrrr(d, s, d, s);

	unlock2(s);
	unlock2(d);
}
MENDFUNC(2,jnf_MULU64,(RW4 d, RW4 s))

MIDFUNC(2,jff_MULU64,(RW4 d, RW4 s))
{
	s = rmw(s, 4, 4);
	d = rmw(d, 4, 4);

	// L, H,
	MSR_CPSRf_i(0);
	UMULLS_rrrr(d, s, d, s);
	MRS_CPSR(REG_WORK1);
	TST_rr(s, s);
	CC_ORR_rri(NATIVE_CC_NE, REG_WORK1, REG_WORK1, ARM_V_FLAG);
	MSR_CPSRf_r(REG_WORK1);

	unlock2(s);
	unlock2(d);
}
MENDFUNC(2,jff_MULU64,(RW4 d, RW4 s))

/*
 * NEG
 * Operand Syntax: <ea>
 *
 * Operand Size: 8,16,32
 *
 * X Set the same as the carry bit.
 * N Set if the result is negative. Cleared otherwise.
 * Z Set if the result is zero. Cleared otherwise.
 * V Set if an overflow occurs. Cleared otherwise.
 * C Cleared if the result is zero. Set otherwise.
 *
 */
MIDFUNC(2,jnf_NEG,(W4 d, RR4 s))
{
	d = writereg(d, 4);
	s = readreg(s, 4);

	RSB_rri(d, s, 0);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jnf_NEG,(W4 d, RR4 s))

MIDFUNC(2,jff_NEG_b,(W4 d, RR1 s))
{
	d = writereg(d, 4);
	s = readreg(s, 4);

	SIGNED8_REG_2_REG(REG_WORK1, s);
	RSBS_rri(d, REG_WORK1, 0);
  
	MRS_CPSR(REG_WORK1);
	EOR_rri(REG_WORK1, REG_WORK1, ARM_C_FLAG);
	MSR_CPSR_r(REG_WORK1);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jff_NEG_b,(W4 d, RR1 s))

MIDFUNC(2,jff_NEG_w,(W4 d, RR2 s))
{
	d = writereg(d, 4);
	s = readreg(s, 4);

	SIGNED16_REG_2_REG(REG_WORK1, s);
	RSBS_rri(d, REG_WORK1, 0);

	MRS_CPSR(REG_WORK1);
	EOR_rri(REG_WORK1, REG_WORK1, ARM_C_FLAG);
	MSR_CPSR_r(REG_WORK1);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jff_NEG_w,(W4 d, RR2 s))

MIDFUNC(2,jff_NEG_l,(W4 d, RR4 s))
{
	d = writereg(d, 4);
	s = readreg(s, 4);

	RSBS_rri(d, s, 0);

	MRS_CPSR(REG_WORK1);
	EOR_rri(REG_WORK1, REG_WORK1, ARM_C_FLAG);
	MSR_CPSR_r(REG_WORK1);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jff_NEG_l,(W4 d, RR4 s))

/*
 * NEGX
 * Operand Syntax: <ea>
 *
 * Operand Size: 8,16,32
 *
 * X Set the same as the carry bit.
 * N Set if the result is negative. Cleared otherwise.
 * Z Cleared if the result is nonzero; unchanged otherwise.
 * V Set if an overflow occurs. Cleared otherwise.
 * C Cleared if the result is zero. Set otherwise.
 *
 * Attention: Z is cleared only if the result is nonzero. Unchanged otherwise
 *
 */
MIDFUNC(2,jnf_NEGX,(W4 d, RR4 s))
{
	d = writereg(d, 4);
	s = readreg(s, 4);

	RSC_rri(d, s, 0);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jnf_NEGX,(W4 d, RR4 s))

MIDFUNC(2,jff_NEGX_b,(W4 d, RR1 s))
{
	d = writereg(d, 4);
	s = readreg(s, 4);

	MRS_CPSR(REG_WORK2);
	CC_MVN_ri(NATIVE_CC_EQ, REG_WORK2, 0);
	CC_MVN_ri(NATIVE_CC_NE, REG_WORK2, ARM_Z_FLAG);

	SIGNED8_REG_2_REG(REG_WORK1, s);
	RSCS_rri(d, REG_WORK1, 0);

	MRS_CPSR(REG_WORK1);
	EOR_rri(REG_WORK1, REG_WORK1, ARM_C_FLAG);
	AND_rrr(REG_WORK1, REG_WORK1, REG_WORK2);
	MSR_CPSR_r(REG_WORK1);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jff_NEGX_b,(W4 d, RR1 s))

MIDFUNC(2,jff_NEGX_w,(W4 d, RR2 s))
{
	d = writereg(d, 4);
	s = readreg(s, 4);

	MRS_CPSR(REG_WORK2);
	CC_MVN_ri(NATIVE_CC_EQ, REG_WORK2, 0);
	CC_MVN_ri(NATIVE_CC_NE, REG_WORK2, ARM_Z_FLAG);

	SIGNED16_REG_2_REG(REG_WORK1, s);
	RSCS_rri(d, REG_WORK1, 0);

	MRS_CPSR(REG_WORK1);
	EOR_rri(REG_WORK1, REG_WORK1, ARM_C_FLAG);
	AND_rrr(REG_WORK1, REG_WORK1, REG_WORK2);
	MSR_CPSR_r(REG_WORK1);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jff_NEGX_w,(W4 d, RR2 s))

MIDFUNC(2,jff_NEGX_l,(W4 d, RR4 s))
{
	d = writereg(d, 4);
	s = readreg(s, 4);

	MRS_CPSR(REG_WORK2);
	CC_MVN_ri(NATIVE_CC_EQ, REG_WORK2, 0);
	CC_MVN_ri(NATIVE_CC_NE, REG_WORK2, ARM_Z_FLAG);

	RSCS_rri(d, s, 0);

	MRS_CPSR(REG_WORK1);
	EOR_rri(REG_WORK1, REG_WORK1, ARM_C_FLAG);
	AND_rrr(REG_WORK1, REG_WORK1, REG_WORK2);
	MSR_CPSR_r(REG_WORK1);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jff_NEGX_l,(W4 d, RR4 s))

/*
 * NOT
 * Operand Syntax: <ea>
 *
 * Operand Size: 8,16,32
 *
 * X Not affected.
 * N Set if the result is negative. Cleared otherwise.
 * Z Set if the result is zero. Cleared otherwise.
 * V Always cleared.
 * C Always cleared.
 *
 */
MIDFUNC(2,jnf_NOT,(W4 d, RR4 s))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	MVN_rr(d, s);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jnf_NOT,(W4 d, RR4 s))

MIDFUNC(2,jff_NOT_b,(W4 d, RR1 s))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	SIGNED8_REG_2_REG(d, s);
	MSR_CPSRf_i(0);
	MVNS_rr(d, d);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jff_NOT_b,(W4 d, RR1 s))

MIDFUNC(2,jff_NOT_w,(W4 d, RR2 s))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	SIGNED16_REG_2_REG(d, s);
	MSR_CPSRf_i(0);
	MVNS_rr(d, d);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jff_NOT_w,(W4 d, RR2 s))

MIDFUNC(2,jff_NOT_l,(W4 d, RR4 s))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	MSR_CPSRf_i(0);
	MVNS_rr(d, s);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jff_NOT_l,(W4 d, RR4 s))

/*
 * OR
 * Operand Syntax: 	<ea>, Dn
 *  				Dn, <ea>
 *
 * Operand Size: 8,16,32
 *
 * X Not affected.
 * N Set if the most significant bit of the result is set. Cleared otherwise.
 * Z Set if the result is zero. Cleared otherwise.
 * V Always cleared.
 * C Always cleared.
 *
 */
MIDFUNC(3,jnf_OR,(W4 d, RR4 s, RR4 v))
{
	if (isconst(s) && isconst(v)) {
		set_const(d, live.state[s].val | live.state[v].val);
		return;
	}

	v = readreg(v, 4);
	s = readreg(s, 4);
	d = writereg(d, 4);

	ORR_rrr(d, s, v);

	unlock2(v);
	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jnf_OR,(RW4 d, RR4 s, RR4 v))

MIDFUNC(3,jff_OR_b,(W4 d, RR1 s, RR1 v))
{
	v = readreg(v, 4);
	s = readreg(s, 4);
	d = writereg(d, 4);

	SIGNED8_REG_2_REG(REG_WORK1, s);
	SIGNED8_REG_2_REG(REG_WORK2, v);
	MSR_CPSRf_i(0);
	ORRS_rrr(d, REG_WORK1, REG_WORK2);

	unlock2(v);
	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jff_OR_b,(RW4 d, RR1 s, RR1 v))

MIDFUNC(3,jff_OR_w,(W4 d, RR2 s, RR2 v))
{
	v = readreg(v, 4);
	s = readreg(s, 4);
	d = writereg(d, 4);

	SIGNED16_REG_2_REG(REG_WORK1, s);
	SIGNED16_REG_2_REG(REG_WORK2, v);
	MSR_CPSRf_i(0);
	ORRS_rrr(d, REG_WORK1, REG_WORK2);

	unlock2(v);
	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jff_OR_w,(RW4 d, RR2 s, RR2 v))

MIDFUNC(3,jff_OR_l,(W4 d, RR4 s, RR4 v))
{
	v = readreg(v, 4);
	s = readreg(s, 4);
	d = writereg(d, 4);

	MSR_CPSRf_i(0);
	ORRS_rrr(d, s, v);

	unlock2(v);
	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jff_OR_l,(RW4 d, RR4 s, RR4 v))

/*
 * ORI
 * Operand Syntax: 	#<data>, CCR
 *
 * Operand Size: 8
 *
 * X — Set if bit 4 of immediate operand is one; unchanged otherwise.
 * N — Set if bit 3 of immediate operand is one; unchanged otherwise.
 * Z — Set if bit 2 of immediate operand is one; unchanged otherwise.
 * V — Set if bit 1 of immediate operand is one; unchanged otherwise.
 * C — Set if bit 0 of immediate operand is one; unchanged otherwise.
 *
 */
MIDFUNC(1,jff_ORSR,(IMM s, IMM x))
{
	MRS_CPSR(REG_WORK1);
	ORR_rri(REG_WORK1, REG_WORK1, s);
	MSR_CPSRf_r(REG_WORK1);

	if (x) {
		compemu_raw_mov_l_ri(REG_WORK1, (uintptr)live.state[FLAGX].mem);
		MOV_ri(REG_WORK2, 1);
		STRB_rR(REG_WORK2, REG_WORK1);
	}
}
MENDFUNC(1,jff_ORSR,(IMM s))

/*
 * ROL
 * Operand Syntax: 	Dx, Dy
 * 					#<data>, Dy
 *					<ea>
 *
 * Operand Size: 8,16,32
 *
 * X Not affected.
 * N Set if the most significant bit of the result is set. Cleared otherwise.
 * Z Set if the result is zero. Cleared otherwise.
 * V Always cleared.
 * C Set according to the last bit rotated out of the operand. Cleared when the rotate count is zero.
 *
 */
MIDFUNC(3,jnf_ROL_b_imm,(W4 d, RR4 s, IMM i))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	LSL_rri(d, s, 24);
	ORR_rrrLSRi(d, d, d, 8);
	ORR_rrrLSRi(d, d, d, 16);
	ROR_rri(d, d, (32 - (i & 0x1f)));

	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jnf_ROL_b_imm,(W4 d, RR4 s, IMM i))

MIDFUNC(3,jnf_ROL_w_imm,(W4 d, RR4 s, IMM i))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	LSL_rri(d, s, 16);
	ORR_rrrLSRi(d, d, d, 16);
	ROR_rri(d, d, (32 - (i & 0x1f)));

	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jnf_ROL_w_imm,(W4 d, RR4 s, IMM i))

MIDFUNC(3,jnf_ROL_l_imm,(W4 d, RR4 s, IMM i))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	ROR_rri(d, s, (32 - (i & 0x1f)));

	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jnf_ROL_l_imm,(W4 d, RR4 s, IMM i))

MIDFUNC(3,jff_ROL_b_imm,(W4 d, RR4 s, IMM i))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	LSL_rri(d, s, 24);
	ORR_rrrLSRi(d, d, d, 8);
	ORR_rrrLSRi(d, d, d, 16);
	MSR_CPSRf_i(0);
	if (i) {
		RORS_rri(d, d, (32 - (i & 0x1f)));

		MRS_CPSR(REG_WORK2);
		TST_ri(d, 1);
		CC_ORR_rri(NATIVE_CC_NE, REG_WORK2, REG_WORK2, ARM_C_FLAG);
		CC_BIC_rri(NATIVE_CC_EQ, REG_WORK2, REG_WORK2, ARM_C_FLAG);
		MSR_CPSR_r(REG_WORK2);
	} else {
		TST_rr(d, d);
	}

	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jff_ROL_b_imm,(W4 d, RR4 s, IMM i))

MIDFUNC(3,jff_ROL_w_imm,(W4 d, RR4 s, IMM i))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	LSL_rri(d, s, 16);
	ORR_rrrLSRi(d, d, d, 16);
	MSR_CPSRf_i(0);
	if (i) {
		RORS_rri(d, d, (32 - (i & 0x1f)));

		MRS_CPSR(REG_WORK2);
		TST_ri(d, 1);
		CC_ORR_rri(NATIVE_CC_NE, REG_WORK2, REG_WORK2, ARM_C_FLAG);
		CC_BIC_rri(NATIVE_CC_EQ, REG_WORK2, REG_WORK2, ARM_C_FLAG);
		MSR_CPSR_r(REG_WORK2);
	} else {
		TST_rr(d, d);
	}

	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jff_ROL_w_imm,(W4 d, RR4 s, IMM i))

MIDFUNC(3,jff_ROL_l_imm,(W4 d, RR4 s, IMM i))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	MSR_CPSRf_i(0);
	if (i) {
		RORS_rri(d, s, (32 - (i & 0x1f)));

		MRS_CPSR(REG_WORK2);
		TST_ri(d, 1);
		CC_ORR_rri(NATIVE_CC_NE, REG_WORK2, REG_WORK2, ARM_C_FLAG);
		CC_BIC_rri(NATIVE_CC_EQ, REG_WORK2, REG_WORK2, ARM_C_FLAG);
		MSR_CPSR_r(REG_WORK2);
	} else {
		MOVS_rr(d, s);
	}

	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jff_ROL_l_imm,(W4 d, RR4 s, IMM i))

MIDFUNC(3,jnf_ROL_b,(W4 d, RR4 s, RR4 i))
{
	if (isconst(i)) {
		COMPCALL(jnf_ROL_b_imm)(d, s, (uae_u8)live.state[i].val);
		return;
	}
	s = readreg(s, 4);
	i = readreg(i, 4);
	d = writereg(d, 4);

	AND_rri(REG_WORK1, i, 0x1f);
	RSB_rri(REG_WORK1, REG_WORK1, 32);

	LSL_rri(d, s, 24);
	ORR_rrrLSRi(d, d, d, 8);
	ORR_rrrLSRi(d, d, d, 16);
	ROR_rrr(d, d, REG_WORK1);

	unlock2(d);
	unlock2(s);
	unlock2(i);
}
MENDFUNC(3,jnf_ROL_b,(W4 d, RR4 s, RR4 i))

MIDFUNC(3,jnf_ROL_w,(W4 d, RR4 s, RR4 i))
{
	if (isconst(i)) {
		COMPCALL(jnf_ROL_w_imm)(d, s, (uae_u8)live.state[i].val);
		return;
	}
	s = readreg(s, 4);
	i = readreg(i, 4);
	d = writereg(d, 4);

	AND_rri(REG_WORK1, i, 0x1f);
	RSB_rri(REG_WORK1, REG_WORK1, 32);

	LSL_rri(d, s, 16);
	ORR_rrrLSRi(d, d, d, 16);
	ROR_rrr(d, d, REG_WORK1);

	unlock2(d);
	unlock2(s);
	unlock2(i);
}
MENDFUNC(3,jnf_ROL_w,(W4 d, RR4 s, RR4 i))

MIDFUNC(3,jnf_ROL_l,(W4 d, RR4 s, RR4 i))
{
	if (isconst(i)) {
		COMPCALL(jnf_ROL_l_imm)(d, s, (uae_u8)live.state[i].val);
		return;
	}
	s = readreg(s, 4);
	i = readreg(i, 4);
	d = writereg(d, 4);

	AND_rri(REG_WORK1, i, 0x1f);
	RSB_rri(REG_WORK1, REG_WORK1, 32);

	ROR_rrr(d, s, REG_WORK1);

	unlock2(d);
	unlock2(s);
	unlock2(i);
}
MENDFUNC(3,jnf_ROL_l,(W4 d, RR4 s, RR4 i))

MIDFUNC(3,jff_ROL_b,(W4 d, RR4 s, RR4 i))
{
	if (isconst(i)) {
		COMPCALL(jff_ROL_b_imm)(d, s, (uae_u8)live.state[i].val);
		return;
	}

	s = readreg(s, 4);
	i = readreg(i, 4);
	d = writereg(d, 4);

	AND_rri(REG_WORK1, i, 0x1f);
	RSB_rri(REG_WORK1, REG_WORK1, 32);

	LSL_rri(d, s, 24);
	ORR_rrrLSRi(d, d, d, 8);
	ORR_rrrLSRi(d, d, d, 16);
	MSR_CPSRf_i(0);
	RORS_rrr(d, d, REG_WORK1);

	MRS_CPSR(REG_WORK2);
	TST_ri(d, 1);
	CC_ORR_rri(NATIVE_CC_NE, REG_WORK2, REG_WORK2, ARM_C_FLAG);
	CC_BIC_rri(NATIVE_CC_EQ, REG_WORK2, REG_WORK2, ARM_C_FLAG);
	MSR_CPSR_r(REG_WORK2);

	unlock2(d);
	unlock2(s);
	unlock2(i);
}
MENDFUNC(3,jff_ROL_b,(W4 d, RR4 s, RR4 i))

MIDFUNC(3,jff_ROL_w,(W4 d, RR4 s, RR4 i))
{
	if (isconst(i)) {
		COMPCALL(jff_ROL_w_imm)(d, s, (uae_u8)live.state[i].val);
		return;
	}

	s = readreg(s, 4);
	i = readreg(i, 4);
	d = writereg(d, 4);

	AND_rri(REG_WORK1, i, 0x1f);
	RSB_rri(REG_WORK1, REG_WORK1, 32);

	LSL_rri(d, s, 16);
	ORR_rrrLSRi(d, d, d, 16);
	MSR_CPSRf_i(0);
	RORS_rrr(d, d, REG_WORK1);

	MRS_CPSR(REG_WORK2);
	TST_ri(d, 1);
	CC_ORR_rri(NATIVE_CC_NE, REG_WORK2, REG_WORK2, ARM_C_FLAG);
	CC_BIC_rri(NATIVE_CC_EQ, REG_WORK2, REG_WORK2, ARM_C_FLAG);
	MSR_CPSR_r(REG_WORK2);

	unlock2(d);
	unlock2(s);
	unlock2(i);
}
MENDFUNC(3,jff_ROL_w,(W4 d, RR4 s, RR4 i))

MIDFUNC(3,jff_ROL_l,(W4 d, RR4 s, RR4 i))
{
	if (isconst(i)) {
		COMPCALL(jff_ROL_l_imm)(d, s, (uae_u8)live.state[i].val);
		return;
	}

	s = readreg(s, 4);
	i = readreg(i, 4);
	d = writereg(d, 4);

	AND_rri(REG_WORK1, i, 0x1f);
	RSB_rri(REG_WORK1, REG_WORK1, 32);

	MSR_CPSRf_i(0);
	RORS_rrr(d, s, REG_WORK1);

	MRS_CPSR(REG_WORK2);
	TST_ri(d, 1);
	CC_ORR_rri(NATIVE_CC_NE, REG_WORK2, REG_WORK2, ARM_C_FLAG);
	CC_BIC_rri(NATIVE_CC_EQ, REG_WORK2, REG_WORK2, ARM_C_FLAG);
	MSR_CPSR_r(REG_WORK2);

	unlock2(d);
	unlock2(s);
	unlock2(i);
}
MENDFUNC(3,jff_ROL_l,(W4 d, RR4 s, RR4 i))

/*
 * ROLW
 * Operand Syntax: 	<ea>
 *
 * Operand Size: 16
 *
 * X Not affected.
 * N Set if the most significant bit of the result is set. Cleared otherwise.
 * Z Set if the result is zero. Cleared otherwise.
 * V Always cleared.
 * C Set according to the last bit rotated out of the operand. Cleared when the rotate count is zero.
 *
 */
MIDFUNC(2,jnf_ROLW,(W4 d, RR4 s))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	LSL_rri(d, s, 16);
	ORR_rrrLSRi(d, d, d, 16);
	ROR_rri(d, d, (32 - 1));

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jnf_ROLW,(W4 d, RR4 s))

MIDFUNC(2,jff_ROLW,(W4 d, RR4 s))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	LSL_rri(d, s, 16);
	ORR_rrrLSRi(d, d, d, 16);
	MSR_CPSRf_i(0);
	RORS_rri(d, d, (32 - 1));

	MRS_CPSR(REG_WORK2);
	TST_ri(d, 1);
	CC_ORR_rri(NATIVE_CC_NE, REG_WORK2, REG_WORK2, ARM_C_FLAG);
	CC_BIC_rri(NATIVE_CC_EQ, REG_WORK2, REG_WORK2, ARM_C_FLAG);
	MSR_CPSR_r(REG_WORK2);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jff_ROLW,(W4 d, RR4 s))

/*
 * RORW
 * Operand Syntax: 	<ea>
 *
 * Operand Size: 16
 *
 * X Not affected.
 * N Set if the most significant bit of the result is set. Cleared otherwise.
 * Z Set if the result is zero. Cleared otherwise.
 * V Always cleared.
 * C Set according to the last bit rotated out of the operand.
 *
 */
MIDFUNC(2,jnf_RORW,(W4 d, RR4 s))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	LSL_rri(d, s, 16);
	ORR_rrrLSRi(d, d, d, 16);
	ROR_rri(d, d, 1);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jnf_RORW,(W4 d, RR4 s))

MIDFUNC(2,jff_RORW,(W4 d, RR4 s))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	LSL_rri(d, s, 16);
	ORR_rrrLSRi(d, d, d, 16);
	MSR_CPSRf_i(0);
	RORS_rri(d, d, 1);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jff_RORW,(W4 d, RR4 s))

/*
 * ROXL
 * Operand Syntax: Dx, Dy
 * 				   #<data>, Dy
 *
 * Operand Size: 8,16,32
 *
 * X Set according to the last bit rotated out of the operand. Cleared when the rotate count is zero.
 * N Set if the most significant bit of the result is set. Cleared otherwise.
 * Z Set if the result is zero. Cleared otherwise.
 * V Always cleared.
 * C Set according to the last bit rotated out of the operand. Cleared when the rotate count is zero.
 *
 */
MIDFUNC(3,jnf_ROXL_b_imm,(W4 d, RR4 s, IMM i))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	if (i > 0) {
		UNSIGNED8_REG_2_REG(d, s);
		LSL_rri(d, d, i);
		CC_ORR_rri(NATIVE_CC_CS, d, d, (1 << (i - 1)));
		if (i > 1) ORR_rrrLSRi(d, d, d, 9);
	} else {
		MOV_rr(d, s);
	}

	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jnf_ROXL_b_imm,(W4 d, RR4 s, IMM i))

MIDFUNC(3,jnf_ROXL_w_imm,(W4 d, RR4 s, IMM i))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	if (i > 0) {
		UNSIGNED16_REG_2_REG(d, s);
		LSL_rri(d, d, i);
		CC_ORR_rri(NATIVE_CC_CS, d, d, (1 << (i - 1)));
		if (i > 1) ORR_rrrLSRi(d, d, d, 17);
	} else {
		MOV_rr(d, s);
	}

	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jnf_ROXL_w_imm,(W4 d, RR4 s, IMM i))

MIDFUNC(3,jnf_ROXL_l_imm,(W4 d, RR4 s, IMM i))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	if (i > 0) {
		LSL_rri(d, s, i);
		CC_ORR_rri(NATIVE_CC_CS, d, d, (1 << (i - 1)));
		if (i > 1) ORR_rrrLSRi(d, d, s, (32 - i));
	} else {
		MOV_rr(d, s);
	}

	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jnf_ROXL_l_imm,(W4 d, RR4 s, IMM i))

MIDFUNC(3,jff_ROXL_b_imm,(W4 d, RR4 s, IMM i))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	if (i > 0) {
		UNSIGNED8_REG_2_REG(d, s);
		LSL_rri(d, d, i);
		CC_ORR_rri(NATIVE_CC_CS, d, d, (1 << (i - 1)));
		if (i > 1) ORR_rrrLSRi(d, d, d, 9);
		TST_ri(s, (1 << (8 - i)));
		CC_MSR_CPSRf_i(NATIVE_CC_NE, ARM_C_FLAG);
		CC_MSR_CPSRf_i(NATIVE_CC_EQ, 0);
	} else {
		MOV_rr(d, s);
		MSR_CPSRf_i(0);
	}

	SIGNED8_REG_2_REG(d, d);
	TST_rr(d, d);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jff_ROXL_b_imm,(W4 d, RR4 s, IMM i))

MIDFUNC(3,jff_ROXL_w_imm,(W4 d, RR4 s, IMM i))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	if (i > 0) {
		UNSIGNED16_REG_2_REG(d, s);
		LSL_rri(d, d, i);
		CC_ORR_rri(NATIVE_CC_CS, d, d, (1 << (i - 1)));
		if (i > 1) ORR_rrrLSRi(d, d, d, 17);
		TST_ri(s, (1 << (16 - i)));
		CC_MSR_CPSRf_i(NATIVE_CC_NE, ARM_C_FLAG);
		CC_MSR_CPSRf_i(NATIVE_CC_EQ, 0);
	} else {
		MOV_rr(d, s);
		MSR_CPSRf_i(0);
	}

	SIGNED16_REG_2_REG(d, d);
	TST_rr(d, d);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jff_ROXL_w_imm,(W4 d, RR4 s, IMM i))

MIDFUNC(3,jff_ROXL_l_imm,(W4 d, RR4 s, IMM i)) 
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	if (i > 0) {
		LSL_rri(d, s, i);
		CC_ORR_rri(NATIVE_CC_CS, d, d, (1 << (i - 1)));
		if (i > 1) ORR_rrrLSRi(d, d, s, (32 - i));
		TST_ri(s, (1 << (32 - i)));
		CC_MSR_CPSRf_i(NATIVE_CC_NE, ARM_C_FLAG);
		CC_MSR_CPSRf_i(NATIVE_CC_EQ, 0);
	} else {
		MOV_rr(d, s);
		MSR_CPSRf_i(0);
	}

	TST_rr(d, d);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jff_ROXL_l_imm,(W4 d, RR4 s, IMM i))

MIDFUNC(3,jnf_ROXL_b,(W4 d, RR4 s, RR4 i))
{
	if (isconst(i)) {
		COMPCALL(jnf_ROXL_b_imm)(d, s, (uae_u8)live.state[i].val);
		return;
	}
	s = readreg(s, 4);
	i = readreg(i, 4);
	d = writereg(d, 4);

	MOV_rr(d, s);
	MRS_CPSR(REG_WORK2);

	AND_rri(REG_WORK1, i, 0x3f);
	CMP_ri(REG_WORK1, 36);
	CC_SUB_rri(NATIVE_CC_GE, REG_WORK1, REG_WORK1, 36);
	CMP_ri(REG_WORK1, 18);
	CC_SUB_rri(NATIVE_CC_GE, REG_WORK1, REG_WORK1, 18);
	CMP_ri(REG_WORK1, 9);
	CC_SUB_rri(NATIVE_CC_GE, REG_WORK1, REG_WORK1, 9);
	CMP_ri(REG_WORK1, 0);
#if defined(ARMV6_ASSEMBLY)
	BLE_i(8 - 1);
#else
	BLE_i(9 - 1);
#endif

	SUB_rri(REG_WORK1, REG_WORK1, 1);
	LSL_rri(d, d, 1);
	MSR_CPSRf_r(REG_WORK2);
	CC_ORR_rri(NATIVE_CC_CS, d, d, 1);
	LSL_rrr(d, d, REG_WORK1);
	RSB_rri(REG_WORK1, REG_WORK1, 8);
#if defined(ARMV6_ASSEMBLY)
	UXTB_rr(REG_WORK2, s);
#else
	ROR_rri(REG_WORK2, s, 8);
	LSR_rri(REG_WORK2, REG_WORK2, 24);
#endif
	ORR_rrrLSRr(d, d, REG_WORK2, REG_WORK1);

	unlock2(d);
	unlock2(s);
	unlock2(i);
}
MENDFUNC(3,jnf_ROXL_b,(W4 d, RR4 s, RR4 i))

MIDFUNC(3,jnf_ROXL_w,(W4 d, RR4 s, RR4 i))
{
	if (isconst(i)) {
		COMPCALL(jnf_ROXL_w_imm)(d, s, (uae_u8)live.state[i].val);
		return;
	}
	s = readreg(s, 4);
	i = readreg(i, 4);
	d = writereg(d, 4);

	UNSIGNED16_REG_2_REG(d, s);
	MRS_CPSR(REG_WORK2);

	CMP_ri(REG_WORK1, 34);
	CC_SUB_rri(NATIVE_CC_GE, REG_WORK1, REG_WORK1, 34);
	CMP_ri(REG_WORK1, 17);
	CC_SUB_rri(NATIVE_CC_GE, REG_WORK1, REG_WORK1, 17);
	CMP_ri(REG_WORK1, 0);
#if defined(ARMV6_ASSEMBLY)
	BLE_i(8 - 1);
#else
	BLE_i(9 - 1);
#endif

	SUB_rri(REG_WORK1, REG_WORK1, 1);
	LSL_rri(d, d, 1);
	MSR_CPSRf_r(REG_WORK2);
	CC_ORR_rri(NATIVE_CC_CS, d, d, 1);
	LSL_rrr(d, d, REG_WORK1);
	RSB_rri(REG_WORK1, REG_WORK1, 16);
#if defined(ARMV6_ASSEMBLY)
	UXTH_rr(REG_WORK2, s);
#else
	LSL_rri(REG_WORK2, s, 16);
	LSR_rri(REG_WORK2, REG_WORK2, 16);
#endif
	ORR_rrrLSRr(d, d, REG_WORK2,REG_WORK1);

	unlock2(d);
	unlock2(s);
	unlock2(i);
}
MENDFUNC(3,jnf_ROXL_w,(W4 d, RR4 s, RR4 i))

MIDFUNC(3,jnf_ROXL_l,(W4 d, RR4 s, RR4 i))
{
	if (isconst(i)) {
		COMPCALL(jnf_ROXL_l_imm)(d, s, (uae_u8)live.state[i].val);
		return;
	}
	s = readreg(s, 4);
	i = readreg(i, 4);
	d = writereg(d, 4);

	MOV_rr(d, s);
	MRS_CPSR(REG_WORK2);

	CMP_ri(REG_WORK1, 33);
	CC_SUB_rri(NATIVE_CC_GE, REG_WORK1, REG_WORK1, 33);
	CMP_ri(REG_WORK1, 0);
	BLE_i(7 - 1);

	SUB_rri(REG_WORK1, REG_WORK1, 1);
	LSL_rri(d, d, 1);
	MSR_CPSRf_r(REG_WORK2);
	CC_ORR_rri(NATIVE_CC_CS, d, d, 1);
	LSL_rrr(d, d, REG_WORK1);
	RSB_rri(REG_WORK1, REG_WORK1, 32);
	ORR_rrrLSRr(d, d, s, REG_WORK1);

	unlock2(d);
	unlock2(s);
	unlock2(i);
}
MENDFUNC(3,jnf_ROXL_l,(W4 d, RR4 s, RR4 i))

MIDFUNC(3,jff_ROXL_b,(W4 d, RR4 s, RR4 i))
{
	if (isconst(i)) {
		COMPCALL(jff_ROXL_b_imm)(d, s, (uae_u8)live.state[i].val);
		return;
	}

	s = readreg(s, 4);
	i = readreg(i, 4);
	d = writereg(d, 4);

	MOV_rr(d, s);
	MRS_CPSR(REG_WORK2);

	AND_rri(REG_WORK1, i, 0x3f);
	CMP_ri(REG_WORK1, 36);
	CC_SUB_rri(NATIVE_CC_GE, REG_WORK1, REG_WORK1, 36);
	CMP_ri(REG_WORK1, 18);
	CC_SUB_rri(NATIVE_CC_GE, REG_WORK1, REG_WORK1, 18);
	CMP_ri(REG_WORK1, 9);
	CC_SUB_rri(NATIVE_CC_GE, REG_WORK1, REG_WORK1, 9);
	CMP_ri(REG_WORK1, 0);
#if defined(ARMV6_ASSEMBLY)
	BLE_i(16 - 1); // label
#else
	BLE_i(17 - 1); // label
#endif

	SUB_rri(REG_WORK1, REG_WORK1, 1);
	LSL_rri(d, d, 1);
	MSR_CPSRf_r(REG_WORK2);
	CC_ORR_rri(NATIVE_CC_CS, d, d, 1);
	LSL_rrr(d, d, REG_WORK1);

	MOV_ri(REG_WORK2, 0x80);
	LSR_rrr(REG_WORK2, REG_WORK2, REG_WORK1);
	PUSH(REG_WORK2);

	RSB_rri(REG_WORK1, REG_WORK1, 8);
#if defined(ARMV6_ASSEMBLY)
	UXTB_rr(REG_WORK2, s);
#else
	ROR_rri(REG_WORK2, s, 8);
	LSR_rri(REG_WORK2, REG_WORK2, 24);
#endif
	ORR_rrrLSRr(d, d, REG_WORK2, REG_WORK1);

	POP(REG_WORK2);
	TST_rr(s, REG_WORK2);
	CC_MSR_CPSRf_i(NATIVE_CC_NE, ARM_C_FLAG);
	CC_MSR_CPSRf_i(NATIVE_CC_EQ, 0);
	B_i(0); // label2

// label:
	MSR_CPSRf_i(0);

// label2:
	SIGNED8_REG_2_REG(d, d);
	TST_rr(d, d);

	unlock2(d);
	unlock2(s);
	unlock2(i);
}
MENDFUNC(3,jff_ROXL_b,(W4 d, RR4 s, RR4 i))

MIDFUNC(3,jff_ROXL_w,(W4 d, RR4 s, RR4 i))
{
	if (isconst(i)) {
		COMPCALL(jff_ROXL_w_imm)(d, s, (uae_u8)live.state[i].val);
		return;
	}

	s = readreg(s, 4);
	i = readreg(i, 4);
	d = writereg(d, 4);

	MOV_rr(d, s);
	MRS_CPSR(REG_WORK2);

	AND_rri(REG_WORK1, i, 0x3f);
	CMP_ri(REG_WORK1, 34);
	CC_SUB_rri(NATIVE_CC_GE, REG_WORK1, REG_WORK1, 34);
	CMP_ri(REG_WORK1, 17);
	CC_SUB_rri(NATIVE_CC_GE, REG_WORK1, REG_WORK1, 17);
	CMP_ri(REG_WORK1, 0);
#if defined(ARMV6_ASSEMBLY)
	BLE_i(16 - 1); // label
#else
	BLE_i(17 - 1); // label
#endif

	SUB_rri(REG_WORK1, REG_WORK1, 1);
	LSL_rri(d, d, 1);
	MSR_CPSRf_r(REG_WORK2);
	CC_ORR_rri(NATIVE_CC_CS, d, d, 1);
	LSL_rrr(d, d, REG_WORK1);

	MOV_ri(REG_WORK2, 0x8000);
	LSR_rrr(REG_WORK2, REG_WORK2, REG_WORK1);
	PUSH(REG_WORK2);

#if defined(ARMV6_ASSEMBLY)
	UXTH_rr(REG_WORK2, s);
#else
	LSL_rri(REG_WORK2, s, 16);
	LSR_rri(REG_WORK2, REG_WORK2, 16);
#endif

	RSB_rri(REG_WORK1, REG_WORK1, 16);
	ORR_rrrLSRr(d, d, REG_WORK2, REG_WORK1);

	POP(REG_WORK2);
	TST_rr(s, REG_WORK2);
	CC_MSR_CPSRf_i(NATIVE_CC_NE, ARM_C_FLAG);
	CC_MSR_CPSRf_i(NATIVE_CC_EQ, 0);
	B_i(0); // label2

// label:
	MSR_CPSRf_i(0);

// label2:
	SIGNED16_REG_2_REG(d, d);
	TST_rr(d, d);

	unlock2(d);
	unlock2(s);
	unlock2(i);
}
MENDFUNC(3,jff_ROXL_w,(W4 d, RR4 s, RR4 i))

MIDFUNC(3,jff_ROXL_l,(W4 d, RR4 s, RR4 i))
{
	if (isconst(i)) {
		COMPCALL(jff_ROXL_l_imm)(d, s, (uae_u8)live.state[i].val);
		return;
	}

	s = readreg(s, 4);
	i = readreg(i, 4);
	d = writereg(d, 4);

	MOV_rr(d, s);
	MRS_CPSR(REG_WORK2);

	AND_rri(REG_WORK1, i, 0x3f);
	CMP_ri(REG_WORK1, 33);
	CC_SUB_rri(NATIVE_CC_GE, REG_WORK1, REG_WORK1, 33);
	CMP_ri(REG_WORK1, 0);
	BLE_i(13 - 1); // label

	SUB_rri(REG_WORK1, REG_WORK1, 1);
	LSL_rri(d, d, 1);
	MSR_CPSRf_r(REG_WORK2);
	CC_ORR_rri(NATIVE_CC_CS, d, d, 1);
	LSL_rrr(d, d, REG_WORK1);

	MOV_ri(REG_WORK2, 0x80000000);
	LSR_rrr(REG_WORK2, REG_WORK2, REG_WORK1);

	RSB_rri(REG_WORK1, REG_WORK1, 32);
	ORR_rrrLSRr(d, d, s, REG_WORK1);

	TST_rr(s, REG_WORK2);
	CC_MSR_CPSRf_i(NATIVE_CC_NE, ARM_C_FLAG);
	CC_MSR_CPSRf_i(NATIVE_CC_EQ, 0);
	B_i(0);// label2

// label:
	MSR_CPSRf_i(0);

// label2:
	TST_rr(d, d);

	unlock2(d);
	unlock2(s);
	unlock2(i);
}
MENDFUNC(3,jff_ROXL_l,(W4 d, RR4 s, RR4 i))

/*
 * ROXLW
 * Operand Syntax: 	<ea>
 *
 * Operand Size: 16
 *
 * X Set according to the last bit rotated out of the operand.
 * N Set if the most significant bit of the result is set. Cleared otherwise.
 * Z Set if the result is zero. Cleared otherwise.
 * V Always cleared.
 * C Set according to the last bit rotated out of the operand.
 *
 */
MIDFUNC(2,jnf_ROXLW,(W4 d, RR4 s))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	LSL_rri(d, s, 1);
	ADC_rri(d, d, 0);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jnf_ROXLW,(W4 d, RR4 s))

MIDFUNC(2,jff_ROXLW,(W4 d, RR4 s))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	LSL_rri(d, s, 1);
	ADC_rri(d, d, 0);
	MSR_CPSRf_i(0);
	LSLS_rri(d, d, 15);
	LSR_rri(d, d, 16);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jff_ROXLW,(W4 d, RR4 s))

/*
 * ROR
 * Operand Syntax: 	Dx, Dy
 * 					#<data>, Dy
 *					<ea>
 *
 * Operand Size: 8,16,32
 *
 * X Not affected.
 * N Set if the most significant bit of the result is set. Cleared otherwise.
 * Z Set if the result is zero. Cleared otherwise.
 * V Always cleared.
 * C Set according to the last bit rotated out of the operand. Cleared when the rotate count is zero.
 *
 */
MIDFUNC(3,jnf_ROR_b_imm,(W4 d, RR4 s, IMM i))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	LSL_rri(d, s, 24);
	ORR_rrrLSRi(d, d, d, 8);
	ORR_rrrLSRi(d, d, d, 16);
	ROR_rri(d, d, i);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jnf_ROR_b_imm,(W4 d, RR4 s, IMM i))

MIDFUNC(3,jnf_ROR_w_imm,(W4 d, RR4 s, IMM i))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	LSL_rri(d, s, 16);
	ORR_rrrLSRi(d, d, d, 16);
	ROR_rri(d, d, i);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jnf_ROR_w_imm,(W4 d, RR4 s, IMM i))

MIDFUNC(3,jnf_ROR_l_imm,(W4 d, RR4 s, IMM i))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	ROR_rri(d, s, i);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jnf_ROR_l_imm,(W4 d, RR4 s, IMM i))

MIDFUNC(3,jff_ROR_b_imm,(W4 d, RR4 s, IMM i))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	LSL_rri(d, s, 24);
	ORR_rrrLSRi(d, d, d, 8);
	ORR_rrrLSRi(d, d, d, 16);
	MSR_CPSRf_i(0);
	RORS_rri(d, d, i);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jff_ROR_b_imm,(W4 d, RR4 s, IMM i))

MIDFUNC(3,jff_ROR_w_imm,(W4 d, RR4 s, IMM i))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	LSL_rri(d, s, 16);
	ORR_rrrLSRi(d, d, d, 16);
	MSR_CPSRf_i(0);
	RORS_rri(d, d, i);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jff_ROR_w_imm,(W4 d, RR4 s, IMM i))

MIDFUNC(3,jff_ROR_l_imm,(W4 d, RR4 s, IMM i))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	MSR_CPSRf_i(0);
	RORS_rri(d, s, i);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jff_ROR_l_imm,(W4 d, RR4 s, IMM i))

MIDFUNC(3,jnf_ROR_b,(W4 d, RR4 s, RR4 i))
{
	if (isconst(i)) {
		COMPCALL(jnf_ROR_b_imm)(d, s, (uae_u8)live.state[i].val);
		return;
	}
	s = readreg(s, 4);
	i = readreg(i, 4);
	d = writereg(d, 4);

	LSL_rri(d, s, 24);
	ORR_rrrLSRi(d, d, d, 8);
	ORR_rrrLSRi(d, d, d, 16);
	ROR_rrr(d, d, i);

	unlock2(d);
	unlock2(s);
	unlock2(i);
}
MENDFUNC(3,jnf_ROR_b,(W4 d, RR4 s, RR4 i))

MIDFUNC(3,jnf_ROR_w,(W4 d, RR4 s, RR4 i))
{
	if (isconst(i)) {
		COMPCALL(jnf_ROR_w_imm)(d, s, (uae_u8)live.state[i].val);
		return;
	}
	s = readreg(s, 4);
	i = readreg(i, 4);
	d = writereg(d, 4);

	LSL_rri(d, s, 16);
	ORR_rrrLSRi(d, d, d, 16);
	ROR_rrr(d, d, i);

	unlock2(d);
	unlock2(s);
	unlock2(i);
}
MENDFUNC(3,jnf_ROR_w,(W4 d, RR4 s, RR4 i))

MIDFUNC(3,jnf_ROR_l,(W4 d, RR4 s, RR4 i))
{
	if (isconst(i)) {
		COMPCALL(jnf_ROR_l_imm)(d, s, (uae_u8)live.state[i].val);
		return;
	}
	s = readreg(s, 4);
	i = readreg(i, 4);
	d = writereg(d, 4);

	ROR_rrr(d, s, i);

	unlock2(d);
	unlock2(s);
	unlock2(i);
}
MENDFUNC(3,jnf_ROR_l,(W4 d, RR4 s, RR4 i))

MIDFUNC(3,jff_ROR_b,(W4 d, RR4 s, RR4 i))
{
	if (isconst(i)) {
		COMPCALL(jff_ROR_b_imm)(d, s, (uae_u8)live.state[i].val);
		return;
	}

	s = readreg(s, 4);
	i = readreg(i, 4);
	d = writereg(d, 4);

	LSL_rri(d, s, 24);
	ORR_rrrLSRi(d, d, d, 8);
	ORR_rrrLSRi(d, d, d, 16);
	MSR_CPSRf_i(0);
	AND_rri(REG_WORK1, i, 63);
	RORS_rrr(d, d, REG_WORK1);

	unlock2(d);
	unlock2(s);
	unlock2(i);
}
MENDFUNC(3,jff_ROR_b,(W4 d, RR4 s, RR4 i))

MIDFUNC(3,jff_ROR_w,(W4 d, RR4 s, RR4 i))
{
	if (isconst(i)) {
		COMPCALL(jff_ROR_w_imm)(d, s, (uae_u8)live.state[i].val);
		return;
	}

	s = readreg(s, 4);
	i = readreg(i, 4);
	d = writereg(d, 4);

	LSL_rri(d, s, 16);
	ORR_rrrLSRi(d, d, d, 16);
	MSR_CPSRf_i(0);
	AND_rri(REG_WORK1, i, 63);
	RORS_rrr(d, d, REG_WORK1);

	unlock2(d);
	unlock2(s);
	unlock2(i);
}
MENDFUNC(3,jff_ROR_w,(W4 d, RR4 s, RR4 i))

MIDFUNC(3,jff_ROR_l,(W4 d, RR4 s, RR4 i))
{
	if (isconst(i)) {
		COMPCALL(jff_ROR_l_imm)(d, s, (uae_u8)live.state[i].val);
		return;
	}

	s = readreg(s, 4);
	i = readreg(i, 4);
	d = writereg(d, 4);

	MSR_CPSRf_i(0);
	AND_rri(REG_WORK1, i, 63);
	RORS_rrr(d, s, REG_WORK1);

	unlock2(d);
	unlock2(s);
	unlock2(i);
}
MENDFUNC(3,jff_ROR_l,(W4 d, RR4 s, RR4 i))

/*
 * ROXR
 * Operand Syntax: Dx, Dy
 * 				   #<data>, Dy
 *
 * Operand Size: 8,16,32
 *
 * X Set according to the last bit rotated out of the operand. Cleared when the rotate count is zero.
 * N Set if the most significant bit of the result is set. Cleared otherwise.
 * Z Set if the result is zero. Cleared otherwise.
 * V Always cleared.
 * C Set according to the last bit rotated out of the operand. Cleared when the rotate count is zero.
 *
 */
MIDFUNC(3,jnf_ROXR_b_imm,(W4 d, RR4 s, IMM i)) 
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	if (i > 0) {
		LSR_rri(d, s, i);
		CC_ORR_rri(NATIVE_CC_CS, d, d, (0x80 >> (i - 1)));
		if (i > 1) ORR_rrrLSLi(d, d, s, (9 - i));
	} else {
		MOV_rr(d, s);
	}

	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jnf_ROXR_b_imm,(W4 d, RR4 s, IMM i))

MIDFUNC(3,jnf_ROXR_w_imm,(W4 d, RR4 s, IMM i))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	if (i > 0) {
		LSR_rri(d, s, i);
		CC_ORR_rri(NATIVE_CC_CS, d, d, (0x8000 >> (i - 1)));
		if (i > 1) ORR_rrrLSLi(d, d, s, (17 - i));
	} else {
		MOV_rr(d, s);
	}

	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jnf_ROXR_w_imm,(W4 d, RR4 s, IMM i))

MIDFUNC(3,jnf_ROXR_l_imm,(W4 d, RR4 s, IMM i))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	if (i > 0) {
		LSR_rri(d, s, i);
		CC_ORR_rri(NATIVE_CC_CS, d, d, (0x80000000 >> (i - 1)));
		if (i > 1) ORR_rrrLSLi(d, d, s, (33 - i));
	} else {
		MOV_rr(d, s);
	}

	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jnf_ROXR_l_imm,(W4 d, RR4 s, IMM i))

MIDFUNC(3,jff_ROXR_b_imm,(W4 d, RR4 s, IMM i))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	if (i > 0) {
		UNSIGNED8_REG_2_REG(d, s);
		LSR_rri(d, d, i);
		CC_ORR_rri(NATIVE_CC_CS, d, d, (0x80 >> (i - 1)));
		if (i > 1) ORR_rrrLSLi(d, d, s, (9 - i));
		TST_ri(s, (1 << (i - 1)));
		CC_MSR_CPSRf_i(NATIVE_CC_NE, ARM_C_FLAG);
		CC_MSR_CPSRf_i(NATIVE_CC_EQ, 0);
	} else {
		MOV_rr(d, s);
		MSR_CPSRf_i(0);
	}

	SIGNED8_REG_2_REG(d, d);
	TST_rr(d, d);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jff_ROXR_b_imm,(W4 d, RR4 s, IMM i))

MIDFUNC(3,jff_ROXR_w_imm,(W4 d, RR4 s, IMM i)) 
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	if (i > 0) {
		UNSIGNED16_REG_2_REG(d, s);
		LSR_rri(d, d, i);
		CC_ORR_rri(NATIVE_CC_CS, d, d, (0x8000 >> (i - 1)));
		if (i > 1) ORR_rrrLSLi(d, d, s, (17 - i));
		TST_ri(s, (1 << (i - 1)));
		CC_MSR_CPSRf_i(NATIVE_CC_NE, ARM_C_FLAG);
		CC_MSR_CPSRf_i(NATIVE_CC_EQ, 0);
	} else {
		MOV_rr(d, s);
		MSR_CPSRf_i(0);
	}

	SIGNED16_REG_2_REG(d, d);
	TST_rr(d, d);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jff_ROXR_w_imm,(W4 d, RR4 s, IMM i))

MIDFUNC(3,jff_ROXR_l_imm,(W4 d, RR4 s, IMM i))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	if (i > 0) {
		LSR_rri(d, s, i);
		CC_ORR_rri(NATIVE_CC_CS, d, d, (0x80000000 >> (i - 1)));
		if (i > 1) ORR_rrrLSLi(d, d, s, (33 - i));
		TST_ri(s, (1 << (i - 1)));
		CC_MSR_CPSRf_i(NATIVE_CC_NE, ARM_C_FLAG);
		CC_MSR_CPSRf_i(NATIVE_CC_EQ, 0);
	} else {
		MOV_rr(d, s);
		MSR_CPSRf_i(0);
	}

	TST_rr(d, d);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jff_ROXR_l_imm,(W4 d, RR4 s, IMM i))

MIDFUNC(3,jnf_ROXR_b,(W4 d, RR4 s, RR4 i))
{
	if (isconst(i)) {
		COMPCALL(jnf_ROXR_b_imm)(d, s, (uae_u8)live.state[i].val);
		return;
	}

	s = readreg(s, 4);
	i = readreg(i, 4);
	d = writereg(d, 4);

	UNSIGNED8_REG_2_REG(d, s);
	MRS_CPSR(REG_WORK2);

	AND_rri(REG_WORK1, i, 0x3f);
	CMP_ri(REG_WORK1, 36);
	CC_SUB_rri(NATIVE_CC_GE, REG_WORK1, REG_WORK1, 36);
	CMP_ri(REG_WORK1, 18);
	CC_SUB_rri(NATIVE_CC_GE, REG_WORK1, REG_WORK1, 18);
	CMP_ri(REG_WORK1, 9);
	CC_SUB_rri(NATIVE_CC_GE, REG_WORK1, REG_WORK1, 9);
	CMP_ri(REG_WORK1, 0);
	BLE_i(7 - 1);

	SUB_rri(REG_WORK1, REG_WORK1, 1);
	LSR_rri(d, d, 1);
	MSR_CPSRf_r(REG_WORK2);
	CC_ORR_rri(NATIVE_CC_CS, d, d,0x80);
	LSR_rrr(d, d, REG_WORK1);
	RSB_rri(REG_WORK1, REG_WORK1, 8);
	ORR_rrrLSLr(d, d, s, REG_WORK1);

	unlock2(d);
	unlock2(s);
	unlock2(i);
}
MENDFUNC(3,jnf_ROXR_b,(W4 d, RR4 s, RR4 i))

MIDFUNC(3,jnf_ROXR_w,(W4 d, RR4 s, RR4 i))
{
	if (isconst(i)) {
		COMPCALL(jnf_ROXR_w_imm)(d, s, (uae_u8)live.state[i].val);
		return;
	}

	s = readreg(s, 4);
	i = readreg(i, 4);
	d = writereg(d, 4);

	UNSIGNED16_REG_2_REG(d, s);
	MRS_CPSR(REG_WORK2);

	CMP_ri(REG_WORK1, 34);
	CC_SUB_rri(NATIVE_CC_GE, REG_WORK1, REG_WORK1, 34);
	CMP_ri(REG_WORK1, 17);
	CC_SUB_rri(NATIVE_CC_GE, REG_WORK1, REG_WORK1, 17);
	CMP_ri(REG_WORK1, 0);
	BLE_i(7 - 1);

	SUB_rri(REG_WORK1, REG_WORK1, 1);
	LSR_rri(d, d, 1);
	MSR_CPSRf_r(REG_WORK2);
	CC_ORR_rri(NATIVE_CC_CS, d, d, 0x8000);
	LSR_rrr(d, d, REG_WORK1);
	RSB_rri(REG_WORK1, REG_WORK1, 16);
	ORR_rrrLSLr(d, d, s, REG_WORK1);

	unlock2(d);
	unlock2(s);
	unlock2(i);
}
MENDFUNC(3,jnf_ROXR_w,(W4 d, RR4 s, RR4 i))

MIDFUNC(3,jnf_ROXR_l,(W4 d, RR4 s, RR4 i))
{
	if (isconst(i)) {
		COMPCALL(jnf_ROXR_l_imm)(d, s, (uae_u8)live.state[i].val);
		return;
	}

	s = readreg(s, 4);
	i = readreg(i, 4);
	d = writereg(d, 4);

	MOV_rr(d, s);
	MRS_CPSR(REG_WORK2);

	CMP_ri(REG_WORK1, 33);
	CC_SUB_rri(NATIVE_CC_GE, REG_WORK1, REG_WORK1, 33);
	CMP_ri(REG_WORK1, 0);
	BLE_i(7 - 1);

	SUB_rri(REG_WORK1, REG_WORK1, 1);
	LSR_rri(d, d, 1);
	MSR_CPSRf_r(REG_WORK2);
	CC_ORR_rri(NATIVE_CC_CS, d, d, 0x80000000);
	LSR_rrr(d, d, REG_WORK1);
	RSB_rri(REG_WORK1, REG_WORK1, 32);
	ORR_rrrLSLr(d, d, s, REG_WORK1);

	unlock2(d);
	unlock2(s);
	unlock2(i);
}
MENDFUNC(3,jnf_ROXR_l,(W4 d, RR4 s, RR4 i))

MIDFUNC(3,jff_ROXR_b,(W4 d, RR4 s, RR4 i))
{
	if (isconst(i)) {
		COMPCALL(jff_ROXR_b_imm)(d, s, (uae_u8)live.state[i].val);
		return;
	}

	s=readreg(s,4);
	i=readreg(i,4);
	d=writereg(d,4);

	UNSIGNED8_REG_2_REG(d, s);
	MRS_CPSR(REG_WORK2);

	AND_rri(REG_WORK1, i, 0x3f);
	CMP_ri(REG_WORK1, 36);
	CC_SUB_rri(NATIVE_CC_GE, REG_WORK1, REG_WORK1, 36);
	CMP_ri(REG_WORK1, 18);
	CC_SUB_rri(NATIVE_CC_GE, REG_WORK1, REG_WORK1, 18);
	CMP_ri(REG_WORK1, 9);
	CC_SUB_rri(NATIVE_CC_GE, REG_WORK1, REG_WORK1, 9);
	CMP_ri(REG_WORK1, 0);
	BLE_i(13 - 1); // label

	SUB_rri(REG_WORK1, REG_WORK1, 1);
	LSR_rri(d, d, 1);
	MSR_CPSRf_r(REG_WORK2);
	CC_ORR_rri(NATIVE_CC_CS, d, d, 0x80);
	LSR_rrr(d, d, REG_WORK1);

	MOV_ri(REG_WORK2, 1);
	LSL_rrr(REG_WORK2, REG_WORK2, REG_WORK1);

	RSB_rri(REG_WORK1, REG_WORK1, 8);
	ORR_rrrLSLr(d, d, s, REG_WORK1);

	TST_rr(s, REG_WORK2);
	CC_MSR_CPSRf_i(NATIVE_CC_NE, ARM_C_FLAG);
	CC_MSR_CPSRf_i(NATIVE_CC_EQ, 0);
	B_i(0); // label2

// label:
	MSR_CPSRf_i(0);

// label2:
	SIGNED8_REG_2_REG(d, d);
	TST_rr(d, d);

	unlock2(d);
	unlock2(s);
	unlock2(i);
}
MENDFUNC(3,jff_ROXR_b,(W4 d, RR4 s, RR4 i))

MIDFUNC(3,jff_ROXR_w,(W4 d, RR4 s, RR4 i))
{
	if (isconst(i)) {
		COMPCALL(jff_ROXR_w_imm)(d, s, (uae_u8)live.state[i].val);
		return;
	}

	s = readreg(s, 4);
	i = readreg(i, 4);
	d = writereg(d, 4);

	UNSIGNED16_REG_2_REG(d, s);
	MRS_CPSR(REG_WORK2);

	AND_rri(REG_WORK1, i, 0x3f);
	CMP_ri(REG_WORK1, 34);
	CC_SUB_rri(NATIVE_CC_GE, REG_WORK1, REG_WORK1, 34);
	CMP_ri(REG_WORK1, 17);
	CC_SUB_rri(NATIVE_CC_GE, REG_WORK1, REG_WORK1, 17);
	CMP_ri(REG_WORK1, 0);
	BLE_i(13 - 1); // label

	SUB_rri(REG_WORK1, REG_WORK1, 1);
	LSR_rri(d, d, 1);
	MSR_CPSRf_r(REG_WORK2);
	CC_ORR_rri(NATIVE_CC_CS, d, d, 0x8000);
	LSR_rrr(d, d, REG_WORK1);

	MOV_ri(REG_WORK2, 1);
	LSL_rrr(REG_WORK2, REG_WORK2, REG_WORK1);

	RSB_rri(REG_WORK1, REG_WORK1, 16);
	ORR_rrrLSLr(d, d, s, REG_WORK1);

	TST_rr(s, REG_WORK2);
	CC_MSR_CPSRf_i(NATIVE_CC_NE, ARM_C_FLAG);
	CC_MSR_CPSRf_i(NATIVE_CC_EQ, 0);
	B_i(0); // label2

// label:
	MSR_CPSRf_i(0);

// label2:
	SIGNED16_REG_2_REG(d, d);
	TST_rr(d, d);

	unlock2(d);
	unlock2(s);
	unlock2(i);
}
MENDFUNC(3,jff_ROXR_w,(W4 d, RR4 s, RR4 i))

MIDFUNC(3,jff_ROXR_l,(W4 d, RR4 s, RR4 i))
{
	if (isconst(i)) {
		COMPCALL(jff_ROXR_l_imm)(d, s, (uae_u8)live.state[i].val);
		return;
	}

	s = readreg(s ,4);
	i = readreg(i, 4);
	d = writereg(d, 4);

	MOV_rr(d, s);
	MRS_CPSR(REG_WORK2);

	AND_rri(REG_WORK1, i, 0x3f);
	CMP_ri(REG_WORK1, 33);
	CC_SUB_rri(NATIVE_CC_GE, REG_WORK1, REG_WORK1, 33);
	CMP_ri(REG_WORK1, 0);
	BLE_i(13 - 1); // label

	SUB_rri(REG_WORK1, REG_WORK1, 1);
	LSR_rri(d, d, 1);
	MSR_CPSRf_r(REG_WORK2);
	CC_ORR_rri(NATIVE_CC_CS, d, d, 0x80000000);
	LSR_rrr(d, d, REG_WORK1);

	MOV_ri(REG_WORK2, 1);
	LSL_rrr(REG_WORK2, REG_WORK2, REG_WORK1);

	RSB_rri(REG_WORK1, REG_WORK1, 32);
	ORR_rrrLSLr(d, d, s, REG_WORK1);

	TST_rr(s, REG_WORK2);
	CC_MSR_CPSRf_i(NATIVE_CC_NE, ARM_C_FLAG);
	CC_MSR_CPSRf_i(NATIVE_CC_EQ, 0);
	B_i(0); // label2

// label:
	MSR_CPSRf_i(0);

// label2:
	TST_rr(d, d);

	unlock2(d);
	unlock2(s);
	unlock2(i);
}
MENDFUNC(3,jff_ROXR_l,(W4 d, RR4 s, RR4 i))

/*
 * ROXRW
 * Operand Syntax: 	<ea>
 *
 * Operand Size: 16
 *
 * X Set according to the last bit rotated out of the operand.
 * N Set if the most significant bit of the result is set. Cleared otherwise.
 * Z Set if the result is zero. Cleared otherwise.
 * V Always cleared.
 * C Set according to the last bit rotated out of the operand.
 *
 */
MIDFUNC(2,jnf_ROXRW,(W4 d, RR4 s))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	LSL_rri(d, s, 16);
	RRX_rr(d, d);
	LSR_rri(d, d, 16);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jnf_ROXRW,(W4 d, RR4 s))

MIDFUNC(2,jff_ROXRW,(W4 d, RR4 s))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	LSL_rri(d, s, 16);
	ORR_rrrLSRi(d, d, d, 16);
	RRXS_rr(d, d);
	LSR_rri(d, d, 16);
	MRS_CPSR(REG_WORK1);
	BIC_rri(REG_WORK1, REG_WORK1, ARM_V_FLAG);
	MSR_CPSR_r(REG_WORK1);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jff_ROXRW,(W4 d, RR4 s))

/*
 * SUB
 * Operand Syntax: 	<ea>, Dn
 * 					Dn, <ea>
 *
 * Operand Size: 8,16,32
 *
 * X Set the same as the carry bit.
 * N Set if the result is negative. Cleared otherwise.
 * Z Set if the result is zero. Cleared otherwise.
 * V Set if an overflow is generated. Cleared otherwise.
 * C Set if a carry is generated. Cleared otherwise.
 *
 */
MIDFUNC(3,jnf_SUB_b_imm,(W4 d, RR4 s, IMM v))
{
	if (isconst(s)) {
		set_const(d, live.state[s].val - v);
		return;
	}

	s = readreg(s, 4);
	d = writereg(d, 4);

	UNSIGNED8_IMM_2_REG(REG_WORK1, (uae_u8)v);
	SUB_rrr(d, s, REG_WORK1);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jnf_SUB_b_imm,(W4 d, RR4 s, IMM v))

MIDFUNC(3,jnf_SUB_b,(W4 d, RR4 s, RR4 v))
{
	if (isconst(v)) {
		COMPCALL(jnf_SUB_b_imm)(d, s, live.state[v].val);
		return;
	}

	// d has to be different to s and v
	v = readreg(v, 4);
	s = readreg(s, 4);
	d = writereg(d, 4);

	SUB_rrr(d, s, v);

	unlock2(d);
	unlock2(s);
	unlock2(v);
}
MENDFUNC(3,jnf_SUB_b,(W4 d, RR4 s, RR4 v))

MIDFUNC(3,jnf_SUB_w_imm,(W4 d, RR4 s, IMM v))
{
	if (isconst(s)) {
		set_const(d, live.state[s].val - v);
		return;
	}

	s = readreg(s, 4);
	d = writereg(d, 4);

	UNSIGNED16_IMM_2_REG(REG_WORK1, (uae_u16)v);
	SUB_rrr(d, s, REG_WORK1);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jnf_SUB_w_imm,(W4 d, RR4 s, IMM v))

MIDFUNC(3,jnf_SUB_w,(W4 d, RR4 s, RR4 v))
{
	if (isconst(v)) {
		COMPCALL(jnf_SUB_w_imm)(d, s, live.state[v].val);
		return;
	}

	// d has to be different to s and v
	v = readreg(v, 4);
	s = readreg(s, 4);
	d = writereg(d, 4);

	SUB_rrr(d, s, v);

	unlock2(d);
	unlock2(s);
	unlock2(v);
}
MENDFUNC(3,jnf_SUB_w,(W4 d, RR4 s, RR4 v))

MIDFUNC(3,jnf_SUB_l_imm,(W4 d, RR4 s, IMM v))
{
	if (isconst(s)) {
		set_const(d, live.state[s].val - v);
		return;
	}

	s = readreg(s, 4);
	d = writereg(d, 4);

	compemu_raw_mov_l_ri(REG_WORK1, v);
	SUB_rrr(d, s, REG_WORK1);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jnf_SUB_l_imm,(W4 d, RR4 s, IMM v))

MIDFUNC(3,jnf_SUB_l,(W4 d, RR4 s, RR4 v))
{
	if (isconst(v)) {
		COMPCALL(jnf_SUB_l_imm)(d, s, live.state[v].val);
		return;
	}

	// d has to be different to s and v
	v = readreg(v, 4);
	s = readreg(s, 4);
	d = writereg(d, 4);

	SUB_rrr(d, s, v);

	unlock2(d);
	unlock2(s);
	unlock2(v);
}
MENDFUNC(3,jnf_SUB_l,(W4 d, RR4 s, RR4 v))

MIDFUNC(3,jff_SUB_b_imm,(W4 d, RR1 s, IMM v))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	LSL_rri(REG_WORK1, s, 24);
	SUBS_rri(d, REG_WORK1, (v << 24));
  ASR_rri(d, d, 24);

	MRS_CPSR(REG_WORK1);
	EOR_rri(REG_WORK1, REG_WORK1, ARM_C_FLAG);
	MSR_CPSR_r(REG_WORK1);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jff_SUB_b_imm,(W4 d, RR1 s, IMM v))

MIDFUNC(3,jff_SUB_b,(W4 d, RR1 s, RR1 v))
{
	if (isconst(v)) {
		COMPCALL(jff_SUB_b_imm)(d, s, live.state[v].val);
		return;
	}

	v = readreg(v, 4);
	s = readreg(s, 4);
	d = writereg(d, 4);

	LSL_rri(REG_WORK1, s, 24);
	SUBS_rrrLSLi(d, REG_WORK1, v, 24);
  ASR_rri(d, d, 24);

	MRS_CPSR(REG_WORK1);
	EOR_rri(REG_WORK1, REG_WORK1, ARM_C_FLAG);
	MSR_CPSR_r(REG_WORK1);

	unlock2(d);
	unlock2(s);
	unlock2(v);
}
MENDFUNC(3,jff_SUB_b,(W4 d, RR1 s, RR1 v))

MIDFUNC(3,jff_SUB_w_imm,(W4 d, RR2 s, IMM v))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

  uae_s32 offs = data_word_offs(v);
	LDR_rRI(REG_WORK1, RPC_INDEX, offs);
	LSL_rri(REG_WORK2, s, 16);
	SUBS_rrrLSLi(d, REG_WORK2, REG_WORK1, 16);
  ASR_rri(d, d, 16);

	MRS_CPSR(REG_WORK1);
	EOR_rri(REG_WORK1, REG_WORK1, ARM_C_FLAG);
	MSR_CPSR_r(REG_WORK1);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jff_SUB_w_imm,(W4 d, RR2 s, IMM v))

MIDFUNC(3,jff_SUB_w,(W4 d, RR2 s, RR2 v))
{
	if (isconst(v)) {
		COMPCALL(jff_SUB_w_imm)(d, s, live.state[v].val);
		return;
	}

	v = readreg(v, 4);
	s = readreg(s, 4);
	d = writereg(d, 4);

	LSL_rri(REG_WORK1, s, 16);
  SUBS_rrrLSLi(d, REG_WORK1, v, 16);
  ASR_rri(d, d, 16);

	MRS_CPSR(REG_WORK1);
	EOR_rri(REG_WORK1, REG_WORK1, ARM_C_FLAG);
	MSR_CPSR_r(REG_WORK1);

	unlock2(d);
	unlock2(s);
	unlock2(v);
}
MENDFUNC(3,jff_SUB_w,(W4 d, RR2 s, RR2 v))

MIDFUNC(3,jff_SUB_l_imm,(W4 d, RR4 s, IMM v))
{
	s = readreg(s, 4);
	d = writereg(d, 4);

	compemu_raw_mov_l_ri(REG_WORK2, v);
	SUBS_rrr(d, s, REG_WORK2);

	MRS_CPSR(REG_WORK1);
	EOR_rri(REG_WORK1, REG_WORK1, ARM_C_FLAG);
	MSR_CPSR_r(REG_WORK1);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(3,jff_SUB_l_imm,(W4 d, RR4 s, IMM v))

MIDFUNC(3,jff_SUB_l,(W4 d, RR4 s, RR4 v))
{
	if (isconst(v)) {
		COMPCALL(jff_SUB_l_imm)(d, s, live.state[v].val);
		return;
	}

	v = readreg(v, 4);
	s = readreg(s, 4);
	d = writereg(d, 4);

	SUBS_rrr(d, s, v);

	MRS_CPSR(REG_WORK1);
	EOR_rri(REG_WORK1, REG_WORK1, ARM_C_FLAG);
	MSR_CPSR_r(REG_WORK1);

	unlock2(d);
	unlock2(s);
	unlock2(v);
}
MENDFUNC(3,jff_SUB_l,(W4 d, RR4 s, RR4 v))

/*
 * SUBA
 *
 * Operand Syntax: 	<ea>, Dn
 *
 * Operand Size: 16,32
 *
 * Flags: Not affected.
 *
 */
MIDFUNC(2,jnf_SUBA_b,(W4 d, RR1 s))
{
	s = readreg(s, 4);
	d = rmw(d, 4, 4);

	SIGNED8_REG_2_REG(REG_WORK1, s);
	SUB_rrr(d, d, REG_WORK1);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jnf_SUBA_b,(W4 d, RR1 s))

MIDFUNC(2,jnf_SUBA_w,(W4 d, RR2 s))
{
	s = readreg(s, 4);
	d = rmw(d, 4, 4);

	SIGNED16_REG_2_REG(REG_WORK1, s);
	SUB_rrr(d, d, REG_WORK1);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jnf_SUBA_w,(W4 d, RR2 s))

MIDFUNC(2,jnf_SUBA_l,(W4 d, RR4 s))
{
	s = readreg(s, 4);
	d = rmw(d, 4, 4);

	SUB_rrr(d, d, s);

	unlock2(d);
	unlock2(s);
}
MENDFUNC(2,jnf_SUBA_l,(W4 d, RR4 s))

/*
 * SUBX
 * Operand Syntax: 	Dy, Dx
 * 					-(Ay), -(Ax)
 *
 * Operand Size: 8,16,32
 *
 * X Set the same as the carry bit.
 * N Set if the result is negative. Cleared otherwise.
 * Z Cleared if the result is nonzero. Unchanged otherwise.
 * V Set if an overflow is generated. Cleared otherwise.
 * C Set if a carry is generated. Cleared otherwise.
 *
 * Attention: Z is cleared only if the result is nonzero. Unchanged otherwise
 *
 */
MIDFUNC(3,jnf_SUBX,(W4 d, RR4 s, RR4 v))
{
	s = readreg(s, 4);
	v = readreg(v, 4);
	d = writereg(d, 4);

	SBC_rrr(d, s, v);

	unlock2(d);
	unlock2(s);
	unlock2(v);
}
MENDFUNC(3,jnf_SUBX,(W4 d, RR4 s, RR4 v))

MIDFUNC(3,jff_SUBX_b,(W4 d, RR1 s, RR1 v))
{
	s = readreg(s, 4);
	v = readreg(v, 4);
	d = writereg(d, 4);

	CC_MVN_ri(NATIVE_CC_EQ, REG_WORK2, 0);
	CC_MVN_ri(NATIVE_CC_NE, REG_WORK2, ARM_Z_FLAG);

	LSL_rri(REG_WORK1, s, 24);
	SBCS_rrrLSLi(d, REG_WORK1, v, 24);
  ASR_rri(d, d, 24);

	MRS_CPSR(REG_WORK1);
	EOR_rri(REG_WORK1, REG_WORK1, ARM_C_FLAG);
	AND_rrr(REG_WORK1, REG_WORK1, REG_WORK2);
	MSR_CPSR_r(REG_WORK1);

	unlock2(d);
	unlock2(s);
	unlock2(v);
}
MENDFUNC(3,jff_SUBX_b,(W4 d, RR1 s, RR1 v))

MIDFUNC(3,jff_SUBX_w,(W4 d, RR2 s, RR2 v))
{
	s = readreg(s, 4);
	v = readreg(v, 4);
	d = writereg(d, 4);

	CC_MVN_ri(NATIVE_CC_EQ, REG_WORK2, 0);
	CC_MVN_ri(NATIVE_CC_NE, REG_WORK2, ARM_Z_FLAG);

	LSL_rri(REG_WORK1, s, 16);
	SBCS_rrrLSLi(d,REG_WORK1,v, 16);
  ASR_rri(d, d, 16);

	MRS_CPSR(REG_WORK1);
	EOR_rri(REG_WORK1, REG_WORK1, ARM_C_FLAG);
	AND_rrr(REG_WORK1, REG_WORK1, REG_WORK2);
	MSR_CPSR_r(REG_WORK1);

	unlock2(d);
	unlock2(s);
	unlock2(v);
}
MENDFUNC(3,jff_SUBX_w,(W4 d, RR2 s, RR2 v))

MIDFUNC(3,jff_SUBX_l,(W4 d, RR4 s, RR4 v))
{
	s = readreg(s, 4);
	v = readreg(v, 4);
	d = writereg(d, 4);

	MRS_CPSR(REG_WORK2);
	CC_MVN_ri(NATIVE_CC_EQ, REG_WORK2, 0);
	CC_MVN_ri(NATIVE_CC_NE, REG_WORK2, ARM_Z_FLAG);

	SBCS_rrr(d, s, v);

	MRS_CPSR(REG_WORK1);
	EOR_rri(REG_WORK1, REG_WORK1, ARM_C_FLAG);
	AND_rrr(REG_WORK1, REG_WORK1, REG_WORK2);
	MSR_CPSR_r(REG_WORK1);

	unlock2(d);
	unlock2(s);
	unlock2(v);
}
MENDFUNC(3,jff_SUBX_l,(W4 d, RR4 s, RR4 v))

/*
 * SWAP
 * Operand Syntax: Dn
 *
 *  Operand Size: 16
 *
 * X Not affected.
 * N Set if the most significant bit of the 32-bit result is set. Cleared otherwise.
 * Z Set if the 32-bit result is zero. Cleared otherwise.
 * V Always cleared.
 * C Always cleared.
 *
 */
MIDFUNC(1,jnf_SWAP,(RW4 d))
{
	d = rmw(d, 4, 4);

	ROR_rri(d, d, 16);

	unlock2(d);
}
MENDFUNC(1,jnf_SWAP,(RW4 d))

MIDFUNC(1,jff_SWAP,(RW4 d))
{
	d = rmw(d, 4, 4);

	ROR_rri(d, d, 16);
	MSR_CPSRf_i(0);
	TST_rr(d, d);

	unlock2(d);
}
MENDFUNC(1,jff_SWAP,(RW4 d))

/*
 * TST
 * Operand Syntax: <ea>
 *
 *  Operand Size: 8,16,32
 *
 * X Not affected.
 * N Set if the operand is negative. Cleared otherwise.
 * Z Set if the operand is zero. Cleared otherwise.
 * V Always cleared.
 * C Always cleared.
 *
 */
MIDFUNC(1,jff_TST_b,(RR1 s))
{
	if (isconst(s)) {
		SIGNED8_IMM_2_REG(REG_WORK1, (uae_u8)live.state[s].val);
	} else {
		s = readreg(s, 4);
		SIGNED8_REG_2_REG(REG_WORK1, s);
		unlock2(s);
	}
	MSR_CPSRf_i(0);
	TST_rr(REG_WORK1, REG_WORK1);
}
MENDFUNC(1,jff_TST_b,(RR1 s))

MIDFUNC(1,jff_TST_w,(RR2 s))
{
	if (isconst(s)) {
		SIGNED16_IMM_2_REG(REG_WORK1, (uae_u16)live.state[s].val);
	} else {
		s = readreg(s, 4);
		SIGNED16_REG_2_REG(REG_WORK1, s);
		unlock2(s);
	}
	MSR_CPSRf_i(0);
	TST_rr(REG_WORK1, REG_WORK1);
}
MENDFUNC(1,jff_TST_w,(RR2 s))

MIDFUNC(1,jff_TST_l,(RR4 s))
{
	MSR_CPSRf_i(0);

	if (isconst(s)) {
		compemu_raw_mov_l_ri(REG_WORK1, live.state[s].val);
		TST_rr(REG_WORK1, REG_WORK1);
	}
	else {
		s = readreg(s, 4);
		TST_rr(s, s);
		unlock2(s);
	}
}
MENDFUNC(1,jff_TST_l,(RR4 s))

/*
 * Memory access functions
 *
 * Two versions: full address range and 24 bit address range
 *
 */
MIDFUNC(2,jnf_MEM_WRITE_OFF_b,(RR4 adr, RR4 b))
{
	uae_s32 offs = get_data_natmem();
	LDR_rRI(REG_WORK2, RPC_INDEX, offs);

  adr = readreg(adr, 4);
  b = readreg(b, 4);
  
  STRB_rRR(b, adr, REG_WORK2);
  
  unlock2(b);
  unlock2(adr);
}
MENDFUNC(2,jnf_MEM_WRITE_OFF_b,(RR4 adr, RR4 b))

MIDFUNC(2,jnf_MEM_WRITE_OFF_w,(RR4 adr, RR4 w))
{
	uae_s32 offs = get_data_natmem();
	LDR_rRI(REG_WORK2, RPC_INDEX, offs);

  adr = readreg(adr, 4);
  w = readreg(w, 4);
  
  REV16_rr(REG_WORK1, w);
  STRH_rRR(REG_WORK1, adr, REG_WORK2);
  
  unlock2(w);
  unlock2(adr);
}
MENDFUNC(2,jnf_MEM_WRITE_OFF_w,(RR4 adr, RR4 w))

MIDFUNC(2,jnf_MEM_WRITE_OFF_l,(RR4 adr, RR4 l))
{
	uae_s32 offs = get_data_natmem();
	LDR_rRI(REG_WORK2, RPC_INDEX, offs);

  adr = readreg(adr, 4);
  l = readreg(l, 4);
  
  REV_rr(REG_WORK1, l);
  STR_rRR(REG_WORK1, adr, REG_WORK2);
  
  unlock2(l);
  unlock2(adr);
}
MENDFUNC(2,jnf_MEM_WRITE_OFF_l,(RR4 adr, RR4 l))


MIDFUNC(2,jnf_MEM_READ_OFF_b,(W4 d, RR4 adr))
{
	uae_s32 offs = get_data_natmem();
	LDR_rRI(REG_WORK2, RPC_INDEX, offs);

  adr = readreg(adr, 4);
  d = writereg(d, 4);
  
  LDRB_rRR(d, adr, REG_WORK2);
  
  unlock2(d);
  unlock2(adr);
}
MENDFUNC(2,jnf_MEM_READ_OFF_b,(W4 d, RR4 adr))

MIDFUNC(2,jnf_MEM_READ_OFF_w,(W4 d, RR4 adr))
{
	uae_s32 offs = get_data_natmem();
	LDR_rRI(REG_WORK2, RPC_INDEX, offs);

  adr = readreg(adr, 4);
  d = writereg(d, 4);
  
  LDRH_rRR(REG_WORK1, adr, REG_WORK2);
  REV16_rr(d, REG_WORK1);
  
  unlock2(d);
  unlock2(adr);
}
MENDFUNC(2,jnf_MEM_READ_OFF_w,(W4 d, RR4 adr))

MIDFUNC(2,jnf_MEM_READ_OFF_l,(W4 d, RR4 adr))
{
	uae_s32 offs = get_data_natmem();
	LDR_rRI(REG_WORK2, RPC_INDEX, offs);

  adr = readreg(adr, 4);
  d = writereg(d, 4);
  
  LDR_rRR(REG_WORK1, adr, REG_WORK2);
  REV_rr(d, REG_WORK1);
  
  unlock2(d);
  unlock2(adr);
}
MENDFUNC(2,jnf_MEM_READ_OFF_l,(W4 d, RR4 adr))


MIDFUNC(2,jnf_MEM_WRITE24_OFF_b,(RR4 adr, RR4 b))
{
	uae_s32 offs = get_data_natmem();
	LDR_rRI(REG_WORK2, RPC_INDEX, offs);

  adr = readreg(adr, 4);
  b = readreg(b, 4);
  
  BIC_rri(REG_WORK1, adr, 0xff000000);
  STRB_rRR(b, REG_WORK1, REG_WORK2);
  
  unlock2(b);
  unlock2(adr);
}
MENDFUNC(2,jnf_MEM_WRITE24_OFF_b,(RR4 adr, RR4 b))

MIDFUNC(2,jnf_MEM_WRITE24_OFF_w,(RR4 adr, RR4 w))
{
	uae_s32 offs = get_data_natmem();
	LDR_rRI(REG_WORK2, RPC_INDEX, offs);

  adr = readreg(adr, 4);
  w = readreg(w, 4);
  
  BIC_rri(REG_WORK1, adr, 0xff000000);
  ADD_rrr(REG_WORK2, REG_WORK1, REG_WORK2);
  REV16_rr(REG_WORK1, w);
  STRH_rR(REG_WORK1, REG_WORK2);
  
  unlock2(w);
  unlock2(adr);
}
MENDFUNC(2,jnf_MEM_WRITE24_OFF_w,(RR4 adr, RR4 w))

MIDFUNC(2,jnf_MEM_WRITE24_OFF_l,(RR4 adr, RR4 l))
{
	uae_s32 offs = get_data_natmem();
	LDR_rRI(REG_WORK2, RPC_INDEX, offs);

  adr = readreg(adr, 4);
  l = readreg(l, 4);
  
  BIC_rri(REG_WORK1, adr, 0xff000000);
  ADD_rrr(REG_WORK2, REG_WORK1, REG_WORK2);
  REV_rr(REG_WORK1, l);
  STR_rR(REG_WORK1, REG_WORK2);
  
  unlock2(l);
  unlock2(adr);
}
MENDFUNC(2,jnf_MEM_WRITE24_OFF_l,(RR4 adr, RR4 l))


MIDFUNC(2,jnf_MEM_READ24_OFF_b,(W4 d, RR4 adr))
{
	uae_s32 offs = get_data_natmem();
	LDR_rRI(REG_WORK2, RPC_INDEX, offs);

  adr = readreg(adr, 4);
  d = writereg(d, 4);
  
  BIC_rri(REG_WORK1, adr, 0xff000000);
  LDRB_rRR(d, REG_WORK1, REG_WORK2);
  
  unlock2(d);
  unlock2(adr);
}
MENDFUNC(2,jnf_MEM_READ24_OFF_b,(W4 d, RR4 adr))

MIDFUNC(2,jnf_MEM_READ24_OFF_w,(W4 d, RR4 adr))
{
	uae_s32 offs = get_data_natmem();
	LDR_rRI(REG_WORK2, RPC_INDEX, offs);

  adr = readreg(adr, 4);
  d = writereg(d, 4);
  
  BIC_rri(REG_WORK1, adr, 0xff000000);
  LDRH_rRR(REG_WORK1, REG_WORK1, REG_WORK2);
  REV16_rr(d, REG_WORK1);
  
  unlock2(d);
  unlock2(adr);
}
MENDFUNC(2,jnf_MEM_READ24_OFF_w,(W4 d, RR4 adr))

MIDFUNC(2,jnf_MEM_READ24_OFF_l,(W4 d, RR4 adr))
{
	uae_s32 offs = get_data_natmem();
	LDR_rRI(REG_WORK2, RPC_INDEX, offs);

  adr = readreg(adr, 4);
  d = writereg(d, 4);
  
  BIC_rri(REG_WORK1, adr, 0xff000000);
  LDR_rRR(d, REG_WORK1, REG_WORK2);
  REV_rr(d, d);
  
  unlock2(d);
  unlock2(adr);
}
MENDFUNC(2,jnf_MEM_READ24_OFF_l,(W4 d, RR4 adr))


MIDFUNC(2,jnf_MEM_GETADR_OFF,(W4 d, RR4 adr))
{
	uae_s32 offs = get_data_natmem();
	LDR_rRI(REG_WORK2, RPC_INDEX, offs);

  adr = readreg(adr, 4);
  d = writereg(d, 4);
  
  ADD_rrr(d, adr, REG_WORK2);
  
  unlock2(d);
  unlock2(adr);
}
MENDFUNC(2,jnf_MEM_GETADR_OFF,(W4 d, RR4 adr))

MIDFUNC(2,jnf_MEM_GETADR24_OFF,(W4 d, RR4 adr))
{
	uae_s32 offs = get_data_natmem();
	LDR_rRI(REG_WORK2, RPC_INDEX, offs);

  adr = readreg(adr, 4);
  d = writereg(d, 4);
  
  BIC_rri(REG_WORK1, adr, 0xff000000);
  ADD_rrr(d, REG_WORK1, REG_WORK2);
  
  unlock2(d);
  unlock2(adr);
}
MENDFUNC(2,jnf_MEM_GETADR24_OFF,(W4 d, RR4 adr))


MIDFUNC(2,jnf_MEM_GETBANKFUNC,(W4 d, RR4 adr, IMM offset))
{
  adr = readreg(adr, 4);
  d = writereg(d, 4);
	
  uae_s32 offs = data_long_offs((uae_u32)mem_banks);
  LDR_rRI(REG_WORK2, RPC_INDEX, offs);
  LSR_rri(REG_WORK1, adr, 16);
  LDR_rRR_LSLi(d, REG_WORK2, REG_WORK1, 2);
  LDR_rRI(d, d, offset);
  
  unlock2(d);
  unlock2(adr);
}
MENDFUNC(2,jnf_MEM_GETBANKFUNC,(W4 d, RR4 adr, IMM offset))