
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
static void PutZ8_RB(const InsDesc* Ins);
static void PutZ8_RW(const InsDesc* Ins);
static void PutZ8_RBW(const InsDesc* Ins);
static void PutZ8_RR(const InsDesc* Ins);
static void PutZ8_STR(const InsDesc* Ins);
static void PutZ8_LDR(const InsDesc* Ins);
static void PutZ8_BRANCH(const InsDesc* Ins);
static void PutZ8_SCF(const InsDesc* Ins);

static void PutZ8_IMPL_(const InsInfo* Info);
static void PutZ8_JUMP_(const InsInfo* Info);
static void PutZ8_RB_(const InsInfo* Info);
static void PutZ8_RW_(const InsInfo* Info);
static void PutZ8_RBW_(const InsInfo* Info);
static void PutZ8_RR_(const InsInfo* Info);
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
		{ "DEC", AMZ8_REGW | AMZ8_REGB, 0, 0, PutZ8_RB },
		{ "HLT", AMZ8_IMPL, 0x02, 0, PutZ8_IMPL },
		{ "INC", AMZ8_REGW | AMZ8_REGB, 0, 1, PutZ8_RBW },
		{ "JMP", AMZ8_IND | AMZ8_ABS, 0, 0, PutZ8_JUMP },
		{ "JSR", AMZ8_IND | AMZ8_ABS, 0, 1, PutZ8_JUMP },
		{ "LDR", AMZ8_RIMMB | AMZ8_RINDB | AMZ8_RINDW | AMZ8_RABSB | AMZ8_RABSW | AMZ8_RIMMW, 0, 0, PutZ8_LDR },
		{ "LSR", AMZ8_REGB, 0x58, 0, PutZ8_RB },
		{ "NOP", AMZ8_IMPL, 0x01, 0, PutZ8_IMPL },
		{ "OR", AMZ8_RIMMB | AMZ8_RREGB, 0, 4, PutZ8_RR },
		{ "PLF", AMZ8_REGB, 0x17, 0, PutZ8_IMPL },
		{ "PLR", AMZ8_REGB, 0x38, 0, PutZ8_RB },
		{ "PSF", AMZ8_REGB, 0x16, 0, PutZ8_IMPL },
		{ "PSR", AMZ8_REGB, 0x30, 0, PutZ8_RB },
		{ "ROL", AMZ8_REGB, 0x48, 0, PutZ8_RB },
		{ "ROR", AMZ8_REGB, 0x40, 0, PutZ8_RB },
		{ "RTS", AMZ8_IMPL, 0x03, 0, PutZ8_IMPL },
		{ "STF", AMZ8_IMM, 0xaa, 0, PutZ8_SCF },
		{ "STR", AMZ8_RINDB | AMZ8_RINDW | AMZ8_RABSB | AMZ8_RABSW, 0, 0, PutZ8_STR },
		{ "SUB", AMZ8_RIMMB | AMZ8_RREGB | AMZ8_RREGWW | AMZ8_RREGWB | AMZ8_RIMMW, 0, 1, PutZ8_RBW },
		{ "TRS", AMZ8_REGW, 0x10, 0, PutZ8_RW },
		{ "TSR", AMZ8_REGW, 0x0c, 0, PutZ8_RW },
		{ "TXR", AMZ8_RREGB, 0, 2, PutZ8_RR },
		{ "XOR", AMZ8_RIMMB | AMZ8_RREGB, 0, 5, PutZ8_RR },
	}
};

/* Cross-reference table for z8 */
const InsTabXRef_t InsTabXRef = {
	sizeof(InsTabXRef.Ins) / sizeof(InsTabXRef.Ins[0]),
	{
		/* Must be in ascending mnemonic order */
		/* Mnemonic, AM, code, extra, handler */

		{ "ADD", PutZ8_RR_ },
		{ "AND", PutZ8_RR_ },
		{ "ASL", PutZ8_RB_ },
		{ "BCC", PutZ8_BRANCH_ },
		{ "BCS", PutZ8_BRANCH_ },
		{ "BEQ", PutZ8_BRANCH_ },
		{ "BIT", PutZ8_RR_ },
		{ "BMI", PutZ8_BRANCH_ },
		{ "BNE", PutZ8_BRANCH_ },
		{ "BPL", PutZ8_BRANCH_ },
		{ "BRK", PutZ8_IMPL_ },
		{ "CLF", PutZ8_SCF_ },
		{ "CMP", PutZ8_RR_ },
		{ "DEC", PutZ8_RBW_ },
		{ "HLT", PutZ8_IMPL_ },
		{ "INC", PutZ8_RBW_ },
		{ "JMP", PutZ8_JUMP_ },
		{ "JSR", PutZ8_JUMP_ },
		{ "LDR", PutZ8_LDR_ },
		{ "LSR", PutZ8_RB_ },
		{ "NOP", PutZ8_IMPL_ },
		{ "OR", PutZ8_RR_ },
		{ "PLF", PutZ8_IMPL_ },
		{ "PLR", PutZ8_RB_ },
		{ "PSF", PutZ8_IMPL_ },
		{ "PSR", PutZ8_RB_ },
		{ "ROL", PutZ8_RB_ },
		{ "ROR", PutZ8_RB_ },
		{ "RTS", PutZ8_IMPL_ },
		{ "STF", PutZ8_SCF_ },
		{ "STR", PutZ8_STR_ },
		{ "SUB", PutZ8_RR_ },
		{ "TRS", PutZ8_RW_ },
		{ "TSR", PutZ8_RW_ },
		{ "TXR", PutZ8_RR_ },
		{ "XOR", PutZ8_RR_ },
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
			else if (CurTok.Tok == IndirectEnter) {
				NextTok();
				Info->Expr2 = Expression();
				Consume(IndirectLeave, IndirectExpect);
				Info->AddrMode = AMZ8_RINDW;
			}

			// `, `
			else if (CurTok.Tok == TOK_REGW) {
				NextTok();
				Info->Expr2 = Expression();
				Info->AddrMode = AMZ8_RREGWW;
			}
			// `, `[]
			else if (CurTok.Tok == TOK_REGH || CurTok.Tok == TOK_REGL) {
				unsigned op = CurTok.Tok == TOK_REGL ? EXPR_REGL : EXPR_REGH;
				NextTok();
				Info->Expr2 = GenRegExpr(op);
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

		unsigned op = CurTok.Tok == TOK_REGL ? EXPR_REGL : EXPR_REGH;
		NextTok();

		Info->Expr1 = GenRegExpr(op);

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
				Info->AddrMode = AMZ8_RINDB;
			}

			// `[], `
			// not supported

			// `[], `[]
			else if (CurTok.Tok == TOK_REGH || CurTok.Tok == TOK_REGL) {
				op = CurTok.Tok == TOK_REGL ? EXPR_REGL : EXPR_REGH;
				NextTok();
				Info->Expr2 = GenRegExpr(op);
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
		// if it's a const then study has validated [0..3] so no need to check
		return (char)v;
	}
	else if (expr->Op == EXPR_REGH || expr->Op == EXPR_REGL) {
		// if it's still a register expression,
		// then study has not validated [0..3] and we have an error
		if (IsEasyConst(expr->Left, &v)) {
			Error("RegB error (%ld is not 0..3)", v);
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

	if ((Info->AddrMode & AMZ8_ABS) > 0) {
		Emit2(Info->ExtCode == 0 ? 0xf0 : 0xf1, Info->Expr1);
	}
	else if ((Info->AddrMode & AMZ8_IND) > 0) {
		const char rW = GetRegW(Info->Expr1);
		if (rW != 0xff) {
			Emit0((Info->ExtCode == 0 ? 0x04 : 0x08) | rW);
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

static void PutZ8_RB_(const InsInfo* Info) {

	if (Info->AddrMode == 0) return;

	char r0B = GetRegB(Info->Expr1);
	if (r0B != 0xff) {
		Emit0(Info->BaseCode | r0B);
	}
}

static void PutZ8_RB(const InsDesc* Ins) {

	InsInfo Info;
	EvalInfo(Ins, &Info);
	PutZ8_RB_(&Info);
}

static void PutZ8_RW_(const InsInfo* Info) {

	if (Info->AddrMode == 0) return;

	char r0W = GetRegW(Info->Expr1);
	if (r0W != 0xff) {
		Emit0(Info->BaseCode | r0W);
	}
}

static void PutZ8_RW(const InsDesc* Ins) {

	InsInfo Info;
	EvalInfo(Ins, &Info);
	PutZ8_RW_(&Info);
}

static void PutZ8_RBW_(const InsInfo* Info) {

	if (Info->AddrMode == 0) return;

	if ((Info->AddrMode & AMZ8_REGB) > 0) {
		char opc =
			Info->ExtCode == 0 ? 0x28 : // dec
			Info->ExtCode == 1 ? 0x20 : // inc
			0x00;
		char r0B = GetRegB(Info->Expr1);
		if (r0B != 0xff) {
			Emit0(opc | r0B);
		}
	}
	else if ((Info->AddrMode & AMZ8_REGW) > 0) {
		char opc =
			Info->ExtCode == 0 ? 0x1c :
			Info->ExtCode == 1 ? 0x18 :
			0x00;
		char r0W = GetRegW(Info->Expr1);
		if (r0W != 0xff) {
			Emit0(opc | r0W);
		}
	}
	else {
		ErrorAm(Info, "rbw");
	}
}

static void PutZ8_RBW(const InsDesc* Ins) {

	InsInfo Info;
	EvalInfo(Ins, &Info);
	PutZ8_RBW_(&Info);
}

static void PutZ8_RR_(const InsInfo* Info) {

	if (Info->AddrMode == 0) return;

	if ((Info->AddrMode & AMZ8_RIMMB) > 0) {
		char opc =
			Info->ExtCode == 0 ? 0x78 :
			Info->ExtCode == 1 ? 0x80 :
			Info->ExtCode == 3 ? 0x88 :
			Info->ExtCode == 4 ? 0x90 :
			Info->ExtCode == 5 ? 0x98 :
			Info->ExtCode == 6 ? 0x68 :
			Info->ExtCode == 7 ? 0x70 :
			0x00;
		char r0B = GetRegB(Info->Expr1);
		Emit1(opc | r0B, Info->Expr2);
	}
	else if ((Info->AddrMode & AMZ8_RIMMW) > 0) {
		char opc =
			Info->ExtCode == 0 ? 0xc4 :
			Info->ExtCode == 1 ? 0xc8 :
			0x00;
		char r0W = GetRegW(Info->Expr1);
		Emit2(opc | r0W, Info->Expr2);
	}
	else if ((Info->AddrMode & AMZ8_RREGB) > 0) {
		char opc =
			Info->ExtCode == 0 ? 0xa5 :
			Info->ExtCode == 1 ? 0xa6 :
			Info->ExtCode == 2 ? 0xa2 :
			Info->ExtCode == 3 ? 0xa7 :
			Info->ExtCode == 4 ? 0xa8 :
			Info->ExtCode == 5 ? 0xa9 :
			Info->ExtCode == 6 ? 0xa3 :
			Info->ExtCode == 7 ? 0xa4 :
			0x00;
		char r0B = GetRegB(Info->Expr1);
		char r1B = GetRegB(Info->Expr2);
		Emit0(opc);
		Emit0(r0B << 3 | r1B);
	}
	else if ((Info->AddrMode & AMZ8_RREGWB) > 0) {
		char opc =
			Info->ExtCode == 0 ? 0xc1 :
			Info->ExtCode == 0 ? 0xc3 :
			0x00;
		char r0W = GetRegW(Info->Expr1);
		char r1B = GetRegB(Info->Expr2);
		Emit0(opc);
		Emit0(r0W << 3 | r1B);
	}
	else if ((Info->AddrMode & AMZ8_RREGWW) > 0) {
		char opc =
			Info->ExtCode == 0 ? 0xc0 :
			Info->ExtCode == 1 ? 0xc2 :
			0x00;
		char r0W = GetRegW(Info->Expr1);
		char r1W = GetRegW(Info->Expr2);
		Emit0(opc);
		Emit0(r0W << 3 | r1W);
	}
	else {
		ErrorAm(Info, "rr");
	}
}

static void PutZ8_RR(const InsDesc* Ins) {

	InsInfo Info;
	EvalInfo(Ins, &Info);
	PutZ8_RR_(&Info);
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

	if ((Info->AddrMode & AMZ8_RABSB) > 0) {
		char r0B = GetRegB(Info->Expr1);
		Emit2(0xe8 | r0B, Info->Expr2);
	}
	else if ((Info->AddrMode & AMZ8_RABSW) > 0) {
		char r0W = GetRegW(Info->Expr2);
		Emit2(0xf4 | r0W, Info->Expr2);
	}
	else if ((Info->AddrMode & AMZ8_RINDB) > 0) {
		char r0B = GetRegB(Info->Expr1);
		char r1W = GetRegW(Info->Expr2);
		Emit0(0xa1);
		Emit0((r0B << 3) | r1W);
	}
	else if ((Info->AddrMode & AMZ8_RINDW) > 0) {
		char r0W = GetRegW(Info->Expr1);
		char r1W = GetRegW(Info->Expr2);
		Emit0(0xad);
		Emit0((r0W << 3) | r1W);
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

	if ((Info->AddrMode & AMZ8_RABSB) > 0) {
		char r0B = GetRegB(Info->Expr1);
		Emit2(0xe0 | r0B, Info->Expr2);
	}
	else if ((Info->AddrMode & AMZ8_RABSW) > 0) {
		char r0W = GetRegW(Info->Expr1);
		Emit2(0xf8 | r0W, Info->Expr2);
	}
	else if ((Info->AddrMode & AMZ8_RINDB) > 0) {
		char r0B = GetRegB(Info->Expr1);
		char r1W = GetRegW(Info->Expr2);
		Emit0(0xa0);
		Emit0((r0B << 3) | r1W);
	}
	else if ((Info->AddrMode & AMZ8_RINDW) > 0) {
		char r0W = GetRegB(Info->Expr1);
		char r1W = GetRegW(Info->Expr2);
		Emit0(0xac);
		Emit0((r0W << 3) | r1W);
	}
	else if ((Info->AddrMode & AMZ8_RIMMB) > 0) {
		char r0B = GetRegB(Info->Expr1);
		Emit1(0x60 | r0B, Info->Expr2);
	}
	else if ((Info->AddrMode & AMZ8_RIMMW) > 0) {
		char r0W = GetRegW(Info->Expr1);
		Emit2(0xfc | r0W, Info->Expr2);
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