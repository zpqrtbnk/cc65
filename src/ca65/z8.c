
#include "z8.h"

#include "instr.h"
#include "objcode.h"
#include "token.h"
#include "global.h"
#include "scanner.h"
#include "nexttok.h"
#include "expr.h"
#include "error.h"
#include "studyexpr.h"
#include <addrsize.h>
#include <stdio.h>
#include <stdlib.h>

static void PutZ8_IMPL(const InsDesc* Ins);
static void PutZ8_JUMP(const InsDesc* Ins);
static void PutZ8_RX(const InsDesc* Ins);
static void PutZ8_RXX(const InsDesc* Ins);
static void PutZ8_STR(const InsDesc* Ins);
static void PutZ8_LDR(const InsDesc* Ins);
static void PutZ8_BRANCH(const InsDesc* Ins);
static void PutZ8_SCF(const InsDesc* Ins);

static void PutZ8_IMPL_(const InsInfo* Info);
static void PutZ8_JUMP_(const InsInfo* Info);
static void PutZ8_RX_(const InsInfo* Info);
static void PutZ8_RXX_(const InsInfo* Info);
static void PutZ8_STR_(const InsInfo* Info);
static void PutZ8_LDR_(const InsInfo* Info);
static void PutZ8_BRANCH_(const InsInfo* Info);
static void PutZ8_SCF_(const InsInfo* Info);

/* Instruction table for z8 */
const InsTabZ8_t InsTabZ8 = {
	sizeof(InsTabZ8.Ins) / sizeof(InsTabZ8.Ins[0]),
	{
		/* Must be in ascending mnemonic order */
		/* Mnemonic, AM, code, extra, handler */

		{ "ADC", AMZ8_RBI | AMZ8_RWIB | AMZ8_RBR | AMZ8_RWR | AMZ8_RWRB | AMZ8_RWI, 0x68, 0x0, PutZ8_RXX },
		{ "AND", AMZ8_RBI | AMZ8_RBR | AMZ8_RWR | AMZ8_RWI, 0x78, 0x2, PutZ8_RXX },
		{ "ASL", AMZ8_RB | AMZ8_RW, 0x50, 0x54, PutZ8_RX },
		{ "BCC", AMZ8_L, 0xA8, 0x0, PutZ8_BRANCH },
		{ "BCS", AMZ8_L, 0xA9, 0x0, PutZ8_BRANCH },
		{ "BEQ", AMZ8_L, 0xAB, 0x0, PutZ8_BRANCH },
		{ "BIT", AMZ8_RBI | AMZ8_RBR | AMZ8_RWR | AMZ8_RWI, 0x60, 0x3, PutZ8_RXX },
		{ "BMI", AMZ8_L, 0xAC, 0x0, PutZ8_BRANCH },
		{ "BNE", AMZ8_L, 0xAA, 0x0, PutZ8_BRANCH },
		{ "BPL", AMZ8_L, 0xAD, 0x0, PutZ8_BRANCH },
		{ "BRK", AMZ8_M, 0x00, 0x0, PutZ8_IMPL },
		{ "CLF", AMZ8_IB, 0xA7, 0x0, PutZ8_SCF },
		{ "CMP", AMZ8_RBI | AMZ8_RBR | AMZ8_RWR | AMZ8_RWI, 0x64, 0x4, PutZ8_RXX },
		{ "DEC", AMZ8_RB | AMZ8_RW, 0x38, 0x3C, PutZ8_RX },
		{ "HLT", AMZ8_M, 0x02, 0x0, PutZ8_IMPL },
		{ "INC", AMZ8_RB | AMZ8_RW, 0x30, 0x34, PutZ8_RX },
		{ "JMP", AMZ8_N | AMZ8_A, 0x04, 0xD4, PutZ8_JUMP },
		{ "JSR", AMZ8_N | AMZ8_A, 0x08, 0xD5, PutZ8_JUMP },
		{ "LDR", AMZ8_RBI | AMZ8_RBN | AMZ8_RBNP | AMZ8_RBPN | AMZ8_RWN | AMZ8_RWNP | AMZ8_RWPN | AMZ8_RWI | AMZ8_RBA | AMZ8_RWA, 0x94, 0x0, PutZ8_LDR },
		{ "LSR", AMZ8_RB | AMZ8_RW, 0x58, 0x5C, PutZ8_RX },
		{ "NOP", AMZ8_M, 0x01, 0x0, PutZ8_IMPL },
		{ "OR", AMZ8_RBI | AMZ8_RBR | AMZ8_RWR | AMZ8_RWI, 0x7C, 0x5, PutZ8_RXX },
		{ "PLF", AMZ8_RB, 0x1D, 0x0, PutZ8_IMPL },
		{ "PLJ", AMZ8_RW, 0x1F, 0x0, PutZ8_IMPL },
		{ "PLR", AMZ8_RB | AMZ8_RW, 0x28, 0x2C, PutZ8_RX },
		{ "PSF", AMZ8_RB, 0x1C, 0x0, PutZ8_IMPL },
		{ "PSJ", AMZ8_RW, 0x1E, 0x0, PutZ8_IMPL },
		{ "PSR", AMZ8_RB | AMZ8_RW, 0x20, 0x24, PutZ8_RX },
		{ "ROL", AMZ8_RB | AMZ8_RW, 0x48, 0x4C, PutZ8_RX },
		{ "ROR", AMZ8_RB | AMZ8_RW, 0x40, 0x44, PutZ8_RX },
		{ "RTS", AMZ8_M, 0x03, 0x0, PutZ8_IMPL },
		{ "SBC", AMZ8_RBI | AMZ8_RWIB | AMZ8_RBR | AMZ8_RWR | AMZ8_RWRB | AMZ8_RWI, 0x70, 0x1, PutZ8_RXX },
		{ "STF", AMZ8_IB, 0xA6, 0x0, PutZ8_SCF },
		{ "STR", AMZ8_RBN | AMZ8_RBNP | AMZ8_RBPN | AMZ8_RWN | AMZ8_RWNP | AMZ8_RWPN | AMZ8_RBA | AMZ8_RWA, 0x9E, 0x0, PutZ8_STR },
		{ "TJR", AMZ8_RW, 0x14, 0x14, PutZ8_RX },
		{ "TRJ", AMZ8_RW, 0x18, 0x18, PutZ8_RX },
		{ "TRS", AMZ8_RW, 0x10, 0x10, PutZ8_RX },
		{ "TSR", AMZ8_RW, 0x0C, 0x0C, PutZ8_RX },
		{ "TXR", AMZ8_RBR | AMZ8_RWR, 0xA4, 0xA5, PutZ8_RX },
		{ "XOR", AMZ8_RBI | AMZ8_RBR | AMZ8_RWR | AMZ8_RWI, 0x80, 0x6, PutZ8_RXX },
	}
};

/* Cross-reference table for z8 */
const InsTabXRef_t InsTabXRef = {
	sizeof(InsTabXRef.Ins) / sizeof(InsTabXRef.Ins[0]),
	{
		/* Must be in ascending mnemonic order */
		/* Mnemonic, AM, code, extra, handler */

		{ "ADC", PutZ8_RXX_ },
		{ "AND", PutZ8_RXX_ },
		{ "ASL", PutZ8_RX_ },
		{ "BCC", PutZ8_BRANCH_ },
		{ "BCS", PutZ8_BRANCH_ },
		{ "BEQ", PutZ8_BRANCH_ },
		{ "BIT", PutZ8_RXX_ },
		{ "BMI", PutZ8_BRANCH_ },
		{ "BNE", PutZ8_BRANCH_ },
		{ "BPL", PutZ8_BRANCH_ },
		{ "BRK", PutZ8_IMPL_ },
		{ "CLF", PutZ8_SCF_ },
		{ "CMP", PutZ8_RXX_ },
		{ "DEC", PutZ8_RX_ },
		{ "HLT", PutZ8_IMPL_ },
		{ "INC", PutZ8_RX_ },
		{ "JMP", PutZ8_JUMP_ },
		{ "JSR", PutZ8_JUMP_ },
		{ "LDR", PutZ8_LDR_ },
		{ "LSR", PutZ8_RX_ },
		{ "NOP", PutZ8_IMPL_ },
		{ "OR", PutZ8_RXX_ },
		{ "PLF", PutZ8_IMPL_ },
		{ "PLJ", PutZ8_IMPL_ },
		{ "PLR", PutZ8_RX_ },
		{ "PSF", PutZ8_IMPL_ },
		{ "PSJ", PutZ8_IMPL_ },
		{ "PSR", PutZ8_RX_ },
		{ "ROL", PutZ8_RX_ },
		{ "ROR", PutZ8_RX_ },
		{ "RTS", PutZ8_IMPL_ },
		{ "SBC", PutZ8_RXX_ },
		{ "STF", PutZ8_SCF_ },
		{ "STR", PutZ8_STR_ },
		{ "TJR", PutZ8_RX_ },
		{ "TRJ", PutZ8_RX_ },
		{ "TRS", PutZ8_RX_ },
		{ "TSR", PutZ8_RX_ },
		{ "TXR", PutZ8_RX_ },
		{ "XOR", PutZ8_RXX_ },

	}
};

// parse input

static void _EvalInfo(const InsDesc* Ins, InsInfo* Info, char noExpr) {

	token_t IndirectEnter;
	token_t IndirectLeave;
	const char* IndirectExpect;

	/* Choose syntax for indirection */
	if (BracketAsIndirect) {
		IndirectEnter = TOK_LBRACK;
		IndirectLeave = TOK_RBRACK;
		IndirectExpect = "']' expected";
	}
	else {
		IndirectEnter = TOK_LPAREN;
		IndirectLeave = TOK_RPAREN;
		IndirectExpect = "')' expected";
	}

	Info->BaseCode = Ins->BaseCode;
	Info->ExtCode = Ins->ExtCode;
	Info->AddrMode = 0;
	Info->Expr1 = NULL;
	Info->Expr2 = NULL;

	// (nothing) eg 'RTS'
	if (TokIsSep(CurTok.Tok)) {
		Info->AddrMode = AMZ8_M; // implicit
	}

	// # => IMM
	else if (CurTok.Tok == TOK_HASH) {
		NextTok();
		Info->Expr1 = Expression();
		Info->AddrMode = AMZ8_IB;
	}

	// ( => IND
	else if (CurTok.Tok == IndirectEnter) {
		NextTok();
		Info->Expr1 = Expression();
		Consume(IndirectLeave, IndirectExpect);
		Info->AddrMode = AMZ8_N;
	}

	// `X
	else if (CurTok.Tok == TOK_REGW) {
		NextTok();

		Info->Expr1 = Expression();

		// `X,
		if (CurTok.Tok == TOK_COMMA) {
			NextTok();

			// `X, #
			if (CurTok.Tok == TOK_HASH) {
				NextTok();
				Info->Expr2 = Expression();
				Info->AddrMode = AMZ8_RWI;
			}

			// `X, (Y)
			else if (CurTok.Tok == IndirectEnter) {
				Info->AddrMode = 0;
				NextTok();
				if (CurTok.Tok == TOK_PDEC) {
					Info->AddrMode = AMZ8_RWPN;
					NextTok();
				}
				Info->Expr2 = Expression();
				if (CurTok.Tok == TOK_PINC) {
					Info->AddrMode = AMZ8_RWNP;
					NextTok();
				}
				Consume(IndirectLeave, IndirectExpect);
				if (Info->AddrMode == 0)
					Info->AddrMode = AMZ8_RWN;
			}

			// `X, `Y
			else if (CurTok.Tok == TOK_REGW) {
				NextTok();
				Info->Expr2 = Expression();
				Info->AddrMode = AMZ8_RWR;
			}
			// `X, `]Y
			else if (CurTok.Tok == TOK_REGH || CurTok.Tok == TOK_REGL) {
				unsigned op = CurTok.Tok == TOK_REGL ? EXPR_REGL : EXPR_REGH;
				NextTok();
				Info->Expr2 = GenRegExpr(op);
				Info->AddrMode = AMZ8_RWRB;
			}
			// `X, a
			else {
				Info->Expr2 = Expression();
				Info->AddrMode = AMZ8_RWA;
			}
		}
		else {
			Info->AddrMode = AMZ8_RW;
		}
	}

	// `]X
	else if (CurTok.Tok == TOK_REGH || CurTok.Tok == TOK_REGL) {

		unsigned op = CurTok.Tok == TOK_REGL ? EXPR_REGL : EXPR_REGH;
		NextTok();

		Info->Expr1 = GenRegExpr(op);

		// `]X,
		if (CurTok.Tok == TOK_COMMA) {
			NextTok();

			// `]X, #
			if (CurTok.Tok == TOK_HASH) {
				NextTok();
				Info->Expr2 = Expression();
				Info->AddrMode = AMZ8_RBI;
			}

			// `]X, (Y)
			else if (CurTok.Tok == IndirectEnter) {
				Info->AddrMode = 0;
				NextTok();
				if (CurTok.Tok == TOK_PDEC) {
					Info->AddrMode = AMZ8_RBPN;
					NextTok();
				}
				Info->Expr2 = Expression();
				if (CurTok.Tok == TOK_PINC) {
					Info->AddrMode = AMZ8_RBNP;
					NextTok();
				}
				Consume(IndirectLeave, IndirectExpect);
				if (Info->AddrMode == 0)
					Info->AddrMode = AMZ8_RBN;
			}

			// `[], `
			// not supported

			// `]X, `]Y
			else if (CurTok.Tok == TOK_REGH || CurTok.Tok == TOK_REGL) {
				op = CurTok.Tok == TOK_REGL ? EXPR_REGL : EXPR_REGH;
				NextTok();
				Info->Expr2 = GenRegExpr(op);
				Info->AddrMode = AMZ8_RBR;
			}

			// `], a
			else {
				Info->Expr2 = Expression();
				Info->AddrMode = AMZ8_RBA;
			}
		}
		else {
			Info->AddrMode = AMZ8_RB;
		}
	}

	// (value)
	else {
		// fixme explain NoExpr?!
		if (!noExpr)
			Info->Expr1 = Expression(); // has to be an 8bit offset or 16bit address
		Info->AddrMode = AMZ8_A | AMZ8_L; // absolute, relative
	}

	Info->AddrMode &= Ins->AddrMode;

	//if ((Ins->AddrMode & Info->AddrMode) == 0) {
	if (Info->AddrMode == 0) {
		Error("Illegal addressing mode");
		return;
	}

	if (Info->Expr1) {
		ExprDesc ED;
		ED_Init(&ED);
		StudyExpr(Info->Expr1, &ED);
		Info->Expr1 = SimplifyExpr(Info->Expr1, &ED);
		ED_Done(&ED);
	}

	if (Info->Expr2) {
		ExprDesc ED;
		ED_Init(&ED);
		StudyExpr(Info->Expr2, &ED);
		Info->Expr2 = SimplifyExpr(Info->Expr2, &ED);
		ED_Done(&ED);
	}
}

static void EvalInfo(const InsDesc* Ins, InsInfo* Info) {
	_EvalInfo(Ins, Info, 0);
}

// get registers

static char GetRegW(const ExprNode* expr) {
	long v;
	if (IsEasyConst(expr, &v)) {
		if (v >= 0 && v <= 3) {
			// return a char with bits [1:0] representing the regW
			// and all other bits being zeroes
			return (char)v;
		}
		else {
			Error("RegW error (%ld is not 0..3)", v);
			return 0xff;
		}
	}
	else {
		Error("RegW error (register expression is not a constant)");
		return 0xff;
	}
}

static char GetRegB(const ExprNode* expr) {
	long v;
	if (IsEasyConst(expr, &v)) {
		// see StudyReg in studyexpr.c
		// if it's a const then study has validated [0..3]
		// and the value is 0000_0wxx where
		//  w  is 1 if '[ else 0
		//  xx is 0-3 register number
		// here we want a byte register, so we have to play a little bit:
		char bit0 = v & 0b00000001;
		char bit1 = v & 0b00000010;
		char bit2 = v & 0b00000100;
		if (bit1 > 0) { // 2-3 not accepted
			Error("RegB error (%ld is not 0..1)", v);
			return 0xff;
		}
		//printf("v: %x -> %x\n", v, ((bit0 << 1) | (bit2 >> 2)));
		return (char) ((bit0<<1) | (bit2 >> 2));
	}
	else if (expr->Op == EXPR_REGH || expr->Op == EXPR_REGL) {
		// if it's still a register expression,
		// then study has not validated [0..3] and we have an error
		if (IsEasyConst(expr->Left, &v)) {
			Error("RegB error (%ld is not 0..1)", v);
			return 0xff;
		}
		else {
			Error("RegB error (register expression is not a constant)");
			return 0xff;
		}
	}
	else {
		Error("RegB error (register expression is not a constant)");
		return 0xff;
	}
}

// emit code

static int CmpName(const void* Key, const void* Desc) {
	return strcmp((const char*)Key, ((const XRefDesc*)Desc)->Mnemonic);
}

void PutZ8(const char* OpCode, InsInfo* Info) {

	const InsDesc* desc;
	const XRefDesc* xdesc;

	desc = bsearch(OpCode, InsTabZ8.Ins, InsTabZ8.Count, sizeof(InsDesc), CmpName);
	xdesc = bsearch(OpCode, InsTabXRef.Ins, InsTabXRef.Count, sizeof(XRefDesc), CmpName);

	if (desc == NULL) Error("Illegal Z8 opcode %s", OpCode);
	if (xdesc == NULL) Error("Illegal ZX opcode %s", OpCode);

	Info->BaseCode = desc->BaseCode;
	Info->ExtCode = desc->ExtCode;

	xdesc->Put(Info);
}

// everything below is used by lines in InsTabZ8

static void ErrorAm(const InsInfo *Info, const char* source)
{
	Error("Unsupported addressing mode %x (%s).", Info->AddrMode, source);
}

static void PutZ8_IMPL_(const InsInfo* Info) {

	Emit0(Info->BaseCode);
}

static void PutZ8_IMPL(const InsDesc* Ins) {

	InsInfo Info;
	Info.BaseCode = Ins->BaseCode;
	PutZ8_IMPL_(&Info);
}

static void PutZ8_JUMP_(const InsInfo* Info) {

	if (Info->AddrMode == 0) return;

	if ((Info->AddrMode & AMZ8_A) > 0) {
		// absolute
		Emit2(Info->ExtCode, Info->Expr1);
	}
	else if ((Info->AddrMode & AMZ8_N) > 0) {
		// indexed
		const char rW = GetRegW(Info->Expr1);
		if (rW != 0xff) {
			Emit0(Info->BaseCode | rW);
		}
	}
	else {
		ErrorAm(Info, "jump");
	}
}

static void PutZ8_JUMP(const InsDesc* Ins) {

	InsInfo Info;
	EvalInfo(Ins, &Info);
	PutZ8_JUMP_(&Info);
}

static void PutZ8_RX_(const InsInfo* Info) {

	if (Info->AddrMode == 0) return;

	if ((Info->AddrMode & AMZ8_RB) > 0)	{
		char r0B = GetRegB(Info->Expr1);
		if (r0B != 0xff) {
			Emit0(Info->BaseCode | r0B);
		}
	}
	else {
		char r0W = GetRegW(Info->Expr1);
		if (r0W != 0xff) {
			Emit0(Info->ExtCode | r0W);
		}
	}
}

static void PutZ8_RX(const InsDesc* Ins) {

	InsInfo Info;
	EvalInfo(Ins, &Info);
	PutZ8_RX_(&Info);
}

static void PutZ8_RXX_(const InsInfo* Info) {

	if (Info->AddrMode == 0) return;

	//       RBI RWIB RBR RWR RWRB RWI
	// ADC 0 68  6C   88  89  8A   E0
	// SBC 1 70  74   8B  8C  8D   E4

	//       RBI RBR RWR RWI
	// AND 2 78  8E  8F  E8
	// BIT 3 60  84  85  D8
	// CMP 4 64  86  87  DC
	// OR  5 7C  90  91  EC
	// XOR 6 80  92  93  F0

	if ((Info->AddrMode & AMZ8_RBI) > 0) {
		char opc =
			Info->ExtCode == 0 ? 0x68 :
			Info->ExtCode == 1 ? 0x70 :
			Info->ExtCode == 2 ? 0x78 :
			Info->ExtCode == 3 ? 0x60 :
			Info->ExtCode == 4 ? 0x64 :
			Info->ExtCode == 5 ? 0x7C :
			Info->ExtCode == 6 ? 0x80 :
			0x00;
		char r0B = GetRegB(Info->Expr1);
		Emit1(opc | r0B, Info->Expr2);
	}
	else if ((Info->AddrMode & AMZ8_RWI) > 0) {
		char opc =
			Info->ExtCode == 0 ? 0xE0 :
			Info->ExtCode == 1 ? 0xE4 :
			Info->ExtCode == 2 ? 0xE8 :
			Info->ExtCode == 3 ? 0xD8 :
			Info->ExtCode == 4 ? 0xDC :
			Info->ExtCode == 5 ? 0xEC :
			Info->ExtCode == 6 ? 0xF0 :
			0x00;
		char r0W = GetRegW(Info->Expr1);
		Emit2(opc | r0W, Info->Expr2);
	}
	else if ((Info->AddrMode & AMZ8_RBR) > 0) {
		char opc =
			Info->ExtCode == 0 ? 0x88 :
			Info->ExtCode == 1 ? 0x8B :
			Info->ExtCode == 2 ? 0x8E :
			Info->ExtCode == 3 ? 0x84 :
			Info->ExtCode == 4 ? 0x86 :
			Info->ExtCode == 5 ? 0x90 :
			Info->ExtCode == 6 ? 0x92 :
			0x00;
		char r0B = GetRegB(Info->Expr1);
		char r1B = GetRegB(Info->Expr2);
		Emit0(opc);
		Emit0(r1B << 2 | r0B);
	}
	else if ((Info->AddrMode & AMZ8_RWR) > 0) {
		char opc =
			Info->ExtCode == 0 ? 0x89 :
			Info->ExtCode == 1 ? 0x8C :
			Info->ExtCode == 2 ? 0x8F :
			Info->ExtCode == 3 ? 0x85 :
			Info->ExtCode == 4 ? 0x87 :
			Info->ExtCode == 5 ? 0x91 :
			Info->ExtCode == 6 ? 0x93 :
			0x00;
		char r0W = GetRegW(Info->Expr1);
		char r1W = GetRegW(Info->Expr2);
		Emit0(opc);
		Emit0(r1W << 2 | r0W);
	}
	else if ((Info->AddrMode & AMZ8_RWRB) > 0) {
		char opc =
			Info->ExtCode == 0 ? 0x8A :
			Info->ExtCode == 1 ? 0x8D :
			0x00;
		char r0W = GetRegW(Info->Expr1);
		char r1B = GetRegB(Info->Expr2);
		Emit0(opc);
		Emit0(r1B << 2 | r0W);
	}
	else if ((Info->AddrMode & AMZ8_RWIB) > 0) {
		char opc =
			Info->ExtCode == 0 ? 0x6C :
			Info->ExtCode == 1 ? 0x74 :
			0x00;
		char r0W = GetRegW(Info->Expr1);
		Emit1(opc | r0W, Info->Expr2);
	}
	else {
		ErrorAm(Info, "rr");
	}
}

static void PutZ8_RXX(const InsDesc* Ins) {

	InsInfo Info;
	EvalInfo(Ins, &Info);
	PutZ8_RXX_(&Info);
}

static void PutZ8_BRANCH_(const InsInfo* Info) {

	// instr.c:1280 does not _EvalInfo etc
	// but it does no do it... everywhere
	// we seem to be different?

	if (Info->AddrMode == 0) return;

	// beware! we write BCC $33 - what does this mean?
	// we want to go to $33 and asm will generate the diff!

	// NOTE: we cannot know here the distance of the branch, and
	// therefore whether it's reachable with our 8bit offset, so
	// we cannot replace *here* the short branch with an alternate
	// combination that would to a long jump = we cannot auto-fix
	// branches!

	//EmitPCRel(Ins->BaseCode, GenBranchExpr(2), 1);
	EmitPCRel(Info->BaseCode, GenBranchExprN(Info->Expr1, 2), 1);
}

static void PutZ8_BRANCH(const InsDesc* Ins) {

	InsInfo Info;
	_EvalInfo(Ins, &Info, 1);
	PutZ8_BRANCH_(&Info);
}

static void PutZ8_STR_(const InsInfo* Info) {

	if (Info->AddrMode == 0) return;

	// RBN RBNP RBPN RWN RWNP RWPN RBA RWA
	// 9E  9F   A0   A1  A2   A3   CC  D0

	if ((Info->AddrMode & AMZ8_RBA) > 0) {
		char r0B = GetRegB(Info->Expr1);
		Emit2(0xCC | r0B, Info->Expr2);
	}
	else if ((Info->AddrMode & AMZ8_RWA) > 0) {
		char r0W = GetRegW(Info->Expr2);
		Emit2(0xD0 | r0W, Info->Expr2);
	}
	else if ((Info->AddrMode & (AMZ8_RBN | AMZ8_RBNP | AMZ8_RBPN)) > 0) {
		char r0B = GetRegB(Info->Expr1);
		char r1W = GetRegW(Info->Expr2);
		if ((Info->AddrMode & AMZ8_RBN) > 0)  Emit0(0x9E); else
		if ((Info->AddrMode & AMZ8_RBNP) > 0) Emit0(0x9F); else
		if ((Info->AddrMode & AMZ8_RBNP) > 0) Emit0(0xA0);
		Emit0((r1W << 2) | r0B);
	}
	else if ((Info->AddrMode & (AMZ8_RWN | AMZ8_RWNP | AMZ8_RWPN)) > 0) {
		char r0W = GetRegW(Info->Expr1);
		char r1W = GetRegW(Info->Expr2);
		if ((Info->AddrMode & AMZ8_RWN) > 0)  Emit0(0xA1); else
		if ((Info->AddrMode & AMZ8_RWNP) > 0) Emit0(0xA2); else
		if ((Info->AddrMode & AMZ8_RWNP) > 0) Emit0(0xA3);
		Emit0((r1W << 2) | r0W);
	}
	else {
		ErrorAm(Info, "str");
	}
}

static void PutZ8_STR(const InsDesc* Ins) {

	InsInfo Info;
	EvalInfo(Ins, &Info);
	PutZ8_STR_(&Info);
}

static void PutZ8_LDR_(const InsInfo* Info) {

	if (Info->AddrMode == 0) return;

	// RBI RBN RBNP RBPN RWN RWNP RWPN RWI RBA RWA
	// 94  98  99   9A   9B  9C   9D   C0  C4  C8

	if ((Info->AddrMode & AMZ8_RBA) > 0) {
		char r0B = GetRegB(Info->Expr1);
		Emit2(0xC4 | r0B, Info->Expr2);
	}
	else if ((Info->AddrMode & AMZ8_RWA) > 0) {
		char r0W = GetRegW(Info->Expr1);
		Emit2(0xC8 | r0W, Info->Expr2);
	}
	else if ((Info->AddrMode & (AMZ8_RBN | AMZ8_RBNP | AMZ8_RBPN)) > 0) {
		char r0B = GetRegB(Info->Expr1);
		char r1W = GetRegW(Info->Expr2);
		if ((Info->AddrMode & AMZ8_RBN) > 0)  Emit0(0x98); else
		if ((Info->AddrMode & AMZ8_RBNP) > 0) Emit0(0x99); else
		if ((Info->AddrMode & AMZ8_RBNP) > 0) Emit0(0x9A);
		Emit0((r1W << 2) | r0B);
	}
	else if ((Info->AddrMode & (AMZ8_RWN | AMZ8_RWNP | AMZ8_RWPN)) > 0) {
		char r0W = GetRegB(Info->Expr1);
		char r1W = GetRegW(Info->Expr2);
		if ((Info->AddrMode & AMZ8_RBN) > 0)  Emit0(0x9B); else
		if ((Info->AddrMode & AMZ8_RBNP) > 0) Emit0(0x9C); else
		if ((Info->AddrMode & AMZ8_RBNP) > 0) Emit0(0x9D);
		Emit0((r1W << 2) | r0W);
	}
	else if ((Info->AddrMode & AMZ8_RBI) > 0) {
		char r0B = GetRegB(Info->Expr1);
		//printf("r0B: %x -> emit %x\n", r0B, 0x94|r0B);
		Emit1(0x94 | r0B, Info->Expr2);
	}
	else if ((Info->AddrMode & AMZ8_RWI) > 0) {
		char r0W = GetRegW(Info->Expr1);
		Emit2(0xC0 | r0W, Info->Expr2);
	}
	else {
		ErrorAm(Info, "ldr");
	}
}

static void PutZ8_LDR(const InsDesc* Ins) {

	InsInfo Info;
	EvalInfo(Ins, &Info);
	PutZ8_LDR_(&Info);
}

static void PutZ8_SCF_(const InsInfo* Info) {

	if (Info->AddrMode == 0) return;

	Emit1(Info->BaseCode, Info->Expr1);
}

static void PutZ8_SCF(const InsDesc* Ins) {

	InsInfo Info;
	EvalInfo(Ins, &Info);
	PutZ8_SCF_(&Info);
}