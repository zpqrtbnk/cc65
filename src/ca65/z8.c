
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

static void PutZ8_IMPL(const InsDesc* Ins);
static void PutZ8_JUMP(const InsDesc* Ins);
static void PutZ8_RB(const InsDesc* Ins);
static void PutZ8_RW(const InsDesc* Ins);
static void PutZ8_RR(const InsDesc* Ins);
static void PutZ8_STR(const InsDesc* Ins);
static void PutZ8_LDR(const InsDesc* Ins);
static void PutZ8_BRANCH(const InsDesc* Ins);
static void PutZ8_SCF(const InsDesc* Ins);

/* Instruction table for z8 */
const InsTabZ8_t InsTabZ8 = {
	sizeof(InsTabZ8.Ins) / sizeof(InsTabZ8.Ins[0]),
	{
		/* Must be in ascending mnemonic order */
		/* Mnemonic, AM, code, extra, handler */

		{ "ADD", AMZ8_RIMMB | AMZ8_RREGB | AMZ8_RREGWW | AMZ8_RREGWB | AMZ8_RIMMW, 0, 0, PutZ8_RR },
		{ "AND", AMZ8_RIMMB | AMZ8_RREGB, 0, 3, PutZ8_RR },
		{ "ASL", AMZ8_REGB, 0x50, 0, PutZ8_RB },
		{ "BCC", AMZ8_REL, 0xb0, 0, PutZ8_BRANCH },
		{ "BCS", AMZ8_REL, 0xb1, 0, PutZ8_BRANCH },
		{ "BEQ", AMZ8_REL, 0xb3, 0, PutZ8_BRANCH },
		{ "BIT", AMZ8_RIMMB | AMZ8_RREGB, 0, 6, PutZ8_RR },
		{ "BMI", AMZ8_REL, 0xb4, 0, PutZ8_BRANCH },
		{ "BNE", AMZ8_REL, 0xb2, 0, PutZ8_BRANCH },
		{ "BPL", AMZ8_REL, 0xb5, 0, PutZ8_BRANCH },
		{ "BRK", AMZ8_IMPL, 0x00, 0, PutZ8_IMPL },
		{ "CLF", AMZ8_IMM, 0xab, 0, PutZ8_SCF },
		{ "CMP", AMZ8_RIMMB | AMZ8_RREGB, 0, 7, PutZ8_RR },
		{ "DEC", AMZ8_REGW | AMZ8_REGB, 0x28, 0, PutZ8_RB },
		{ "GSR", AMZ8_REGW, 0x0c, 0, PutZ8_RW },
		{ "HLT", AMZ8_IMPL, 0x02, 0, PutZ8_IMPL },
		{ "INC", AMZ8_REGW | AMZ8_REGB, 0x20, 0, PutZ8_RB },
		{ "JMP", AMZ8_IND | AMZ8_ABS, 0, 0, PutZ8_JUMP },
		{ "JSR", AMZ8_IND | AMZ8_ABS, 0, 1, PutZ8_JUMP },
		{ "LDR", AMZ8_RIMMB | AMZ8_RIND | AMZ8_RABSB | AMZ8_RABSW | AMZ8_RIMMW, 0, 0, PutZ8_LDR },
		{ "LSR", AMZ8_REGB, 0x58, 0, PutZ8_RB },
		{ "NOP", AMZ8_IMPL, 0x01, 0, PutZ8_IMPL },
		{ "OR", AMZ8_RIMMB | AMZ8_RREGB, 0, 4, PutZ8_RR },
		{ "PLL", AMZ8_REGB, 0x38, 0, PutZ8_RB },
		{ "PSH", AMZ8_REGB, 0x30, 0, PutZ8_RB },
		{ "ROL", AMZ8_REGB, 0x48, 0, PutZ8_RB },
		{ "ROR", AMZ8_REGB, 0x40, 0, PutZ8_RB },
		{ "RTS", AMZ8_IMPL, 0x03, 0, PutZ8_IMPL },
		{ "SSR", AMZ8_REGW, 0x10, 0, PutZ8_RW },
		{ "STF", AMZ8_IMM, 0xaa, 0, PutZ8_SCF },
		{ "STR", AMZ8_RIND | AMZ8_RABSB | AMZ8_RABSW, 0, 0, PutZ8_STR },
		{ "SUB", AMZ8_RIMMB | AMZ8_RREGB | AMZ8_RREGWW | AMZ8_RREGWB | AMZ8_RIMMW, 0, 1, PutZ8_RR },
		{ "TXR", AMZ8_RREGB, 0, 2, PutZ8_RR },
		{ "XOR", AMZ8_RIMMB | AMZ8_RREGB, 0, 5, PutZ8_RR },

	}
};

typedef struct InsInfo InsInfo;
struct InsInfo {
	unsigned long AddrMode;
	ExprNode* Expr1;
	ExprNode* Expr2;
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

	Info->AddrMode = 0;
	Info->Expr1 = NULL;
	Info->Expr2 = NULL;

	// (nothing) => IMPL
	if (TokIsSep(CurTok.Tok)) {
		Info->AddrMode = AMZ8_IMPL;
	}

	// # => IMM
	else if (CurTok.Tok == TOK_HASH) {
		NextTok();
		Info->Expr1 = Expression();
		Info->AddrMode = AMZ8_IMM;
	}

	// ( or [ => IND
	else if (CurTok.Tok == IndirectEnter) {
		NextTok();
		Info->Expr1 = Expression();
		Consume(IndirectLeave, IndirectExpect);
		Info->AddrMode = AMZ8_IND;
	}

	// `
	else if (CurTok.Tok == TOK_REGW) {
		NextTok();

		Info->Expr1 = Expression();

		if (CurTok.Tok == TOK_COMMA) {
			NextTok();

			// `, #
			if (CurTok.Tok == TOK_HASH) {
				NextTok();
				Info->Expr2 = Expression();
				Info->AddrMode = AMZ8_RIMMW;
			}

			// `, ()
			// not supported

			// `, `
			else if (CurTok.Tok == TOK_REGW) {
				NextTok();
				Info->Expr2 = Expression();
				Info->AddrMode = AMZ8_RREGWW;
			}
			// `, `[
			else if (CurTok.Tok == TOK_REGH) {
				NextTok();
				Info->Expr2 = GenRegExpr(EXPR_REGH);
				Info->AddrMode = AMZ8_RREGWB;
			}
			// `, `]
			else if (CurTok.Tok == TOK_REGL) {
				NextTok();
				Info->Expr2 = GenRegExpr(EXPR_REGL);
				Info->AddrMode = AMZ8_RREGWB;
			}
			// `, a
			else {
				Info->Expr2 = Expression();
				Info->AddrMode = AMZ8_RABSW;
			}
		}
		else {
			Info->AddrMode = AMZ8_REGW;
		}
	}

	// `[]
	else if (CurTok.Tok == TOK_REGH || CurTok.Tok == TOK_REGL) {
		NextTok();

		Info->Expr1 = GenRegExpr(CurTok.Tok == TOK_REGL ? EXPR_REGL : EXPR_REGH);

		if (CurTok.Tok == TOK_COMMA) {
			NextTok();

			// `[], #
			if (CurTok.Tok == TOK_HASH) {
				NextTok();
				Info->Expr2 = Expression();
				Info->AddrMode = AMZ8_RIMMB;
			}

			// `[], ()
			else if (CurTok.Tok == IndirectEnter) {
				NextTok();
				Info->Expr2 = Expression();
				Consume(IndirectLeave, IndirectExpect);
				Info->AddrMode = AMZ8_RIND;
			}

			// `[], `
			// not supported

			// `[], `[]
			else if (CurTok.Tok == TOK_REGH || CurTok.Tok == TOK_REGL) {
				NextTok();
				Info->Expr2 = GenRegExpr(CurTok.Tok == TOK_REGL ? EXPR_REGL : EXPR_REGH);
				Info->AddrMode = AMZ8_RREGWB;
			}

			// `[], a
			else {
				Info->Expr2 = Expression();
				Info->AddrMode = AMZ8_RABSB;
			}
		}
		else {
			Info->AddrMode = AMZ8_REGB;
		}
	}

	// (value)
	else {
		// fixme explain NoExpr?!
		if (!noExpr)
			Info->Expr1 = Expression(); // has to be an 8bit offset or 16bit address
		Info->AddrMode = AMZ8_ABS | AMZ8_REL;
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
		if (v >= 0 && v <= 7) {
			// return a char with bits [1:0] representing the regW and bit 2 representing B/W
			char b = (v & 0b00000001) << 2;
			v = (v >> 1) | b;
			return (char)v;
		}
		else {
			Error("Reg8 error (%ld is not 0..7)", v);
			return 0xff;
		}
	}
	else {
		Error("RegB error (register expression is not a constant)");
		return 0xff;
	}
}

// emit code

static void PutZ8_IMPL(const InsDesc* Ins) {

	Emit0(Ins->BaseCode);
}

static void PutZ8_JUMP(const InsDesc* Ins) {

	InsInfo Info;
	EvalInfo(Ins, &Info);
	if (Info.AddrMode == 0) return;

	if ((Info.AddrMode & AMZ8_ABS) > 0) {
		Emit2(Ins->ExtCode == 0 ? 0xf0 : 0xf1, Info.Expr1);
	}
	else if ((Info.AddrMode & AMZ8_IND) > 0) {
		const char rW = GetRegW(Info.Expr1);
		if (rW != 0xff) {
			Emit0((Ins->ExtCode == 0 ? 0x04 : 0x08) | rW);
		}
	}
	else {
		Error("Unsupported addressing mode %x.", Info.AddrMode);
	}
}

static void PutZ8_RB(const InsDesc* Ins) {

	InsInfo Info;
	EvalInfo(Ins, &Info);
	if (Info.AddrMode == 0) return;

	char r0B = GetRegB(Info.Expr1);
	if (r0B != 0xff) {
		Emit0(Ins->BaseCode | r0B);
	}
}

static void PutZ8_RW(const InsDesc* Ins) {

	InsInfo Info;
	EvalInfo(Ins, &Info);
	if (Info.AddrMode == 0) return;

	char r0W = GetRegW(Info.Expr1);
	if (r0W != 0xff) {
		Emit0(Ins->BaseCode | r0W);
	}
}

static void PutZ8_RR(const InsDesc* Ins) {

	InsInfo Info;
	EvalInfo(Ins, &Info);
	if (Info.AddrMode == 0) return;

	if ((Info.AddrMode & AMZ8_RIMMB) > 0) {
		char opc =
			Ins->ExtCode == 0 ? 0x78 :
			Ins->ExtCode == 1 ? 0x80 :
			Ins->ExtCode == 3 ? 0x88 :
			Ins->ExtCode == 4 ? 0x90 :
			Ins->ExtCode == 5 ? 0x98 :
			Ins->ExtCode == 6 ? 0x68 :
			Ins->ExtCode == 7 ? 0x70 :
			0x00;
		char r0B = GetRegB(Info.Expr1);
		Emit1(opc | r0B, Info.Expr2);
	}
	else if ((Info.AddrMode & AMZ8_RIMMW) > 0) {
		char opc =
			Ins->ExtCode == 0 ? 0xc4 :
			Ins->ExtCode == 1 ? 0xc8 :
			0x00;
		char r0W = GetRegW(Info.Expr1);
		Emit2(opc | r0W, Info.Expr2);
	}
	else if ((Info.AddrMode & AMZ8_RREGB) > 0) {
		char opc =
			Ins->ExtCode == 0 ? 0xa5 :
			Ins->ExtCode == 1 ? 0xa6 :
			Ins->ExtCode == 2 ? 0xa2 :
			Ins->ExtCode == 3 ? 0xa7 :
			Ins->ExtCode == 4 ? 0xa8 :
			Ins->ExtCode == 5 ? 0xa9 :
			Ins->ExtCode == 6 ? 0xa3 :
			Ins->ExtCode == 7 ? 0xa4 :
			0x00;
		char r0B = GetRegB(Info.Expr1);
		char r1B = GetRegB(Info.Expr2);
		Emit0(opc);
		Emit0(r0B << 3 | r1B);
	}
	else if ((Info.AddrMode & AMZ8_RREGWB) > 0) {
		char opc =
			Ins->ExtCode == 0 ? 0xc1 :
			Ins->ExtCode == 0 ? 0xc3 :
			0x00;
		char r0W = GetRegW(Info.Expr1);
		char r1B = GetRegB(Info.Expr2);
		Emit0(opc);
		Emit0(r0W << 3 | r1B);
	}
	else if ((Info.AddrMode & AMZ8_RREGWW) > 0) {
		char opc =
			Ins->ExtCode == 0 ? 0xc0 :
			Ins->ExtCode == 1 ? 0xc2 :
			0x00;
		char r0W = GetRegW(Info.Expr1);
		char r1W = GetRegW(Info.Expr2);
		Emit0(opc);
		Emit0(r0W << 3 | r1W);
	}
	else {
		Error("Unsupported addressing mode %x.", Info.AddrMode);
	}
}

static void PutZ8_BRANCH(const InsDesc* Ins) {

	// instr.c:1280 does not _EvalInfo etc
	// but it does no do it... everywhere
	// we seem to be different?

	InsInfo Info;
	_EvalInfo(Ins, &Info, 1);
	if (Info.AddrMode == 0) return;

	// beware! we write BCC $33 - what does this mean?
	// we want to go to $33 and asm will generate the diff!

	//EmitPCRel(Ins->BaseCode, GenBranchExpr(2), 1);
	EmitPCRel(Ins->BaseCode, GenBranchExprN(Info.Expr1, 2), 1);
}

static void PutZ8_STR(const InsDesc* Ins) {

	InsInfo Info;
	EvalInfo(Ins, &Info);
	if (Info.AddrMode == 0) return;

	if ((Info.AddrMode & AMZ8_RABSB) > 0) {
		char r0B = GetRegB(Info.Expr1);
		Emit2(0xe8 | r0B, Info.Expr2);
	}
	else if ((Info.AddrMode & AMZ8_RABSW) > 0) {
		char r0W = GetRegW(Info.Expr2);
		Emit2(0xf4 | r0W, Info.Expr2);
	}
	else if ((Info.AddrMode & AMZ8_RIND) > 0) {
		char r0B = GetRegB(Info.Expr1);
		char r1W = GetRegW(Info.Expr2);
		Emit0(0xa1);
		Emit0((r0B << 3) | r1W);
	}
	else {
		Error("Unsupported addressing mode %x.", Info.AddrMode);
	}
}

static void PutZ8_LDR(const InsDesc* Ins) {
	InsInfo Info;
	EvalInfo(Ins, &Info);
	if (Info.AddrMode == 0) return;

	if ((Info.AddrMode & AMZ8_RABSB) > 0) {
		char r0B = GetRegB(Info.Expr1);
		Emit2(0xe0 | r0B, Info.Expr2);
	}
	else if ((Info.AddrMode & AMZ8_RABSW) > 0) {
		char r0W = GetRegW(Info.Expr2);
		Emit2(0xf8 | r0W, Info.Expr2);
	}
	else if ((Info.AddrMode & AMZ8_RIND) > 0) {
		char r0B = GetRegB(Info.Expr1);
		char r1W = GetRegW(Info.Expr2);
		Emit0(0xa0);
		Emit0((r0B << 3) | r1W);
	}
	else if ((Info.AddrMode & AMZ8_RIMMB) > 0) {
		char r0B = GetRegB(Info.Expr1);
		Emit1(0x60 | r0B, Info.Expr2);
	}
	else if ((Info.AddrMode & AMZ8_RIMMW) > 0) {
		char r0W = GetRegW(Info.Expr1);
		Emit2(0xfc | r0W, Info.Expr2);
	}
	else {
		Error("Unsupported addressing mode %x.", Info.AddrMode);
	}
}

static void PutZ8_SCF(const InsDesc* Ins) {
	InsInfo Info;
	EvalInfo(Ins, &Info);
	if (Info.AddrMode == 0) return;

	Emit1(Ins->BaseCode, Info.Expr1);
}