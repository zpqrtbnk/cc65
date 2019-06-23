
#include "z8.h"

#include "instr.h"
#include "objcode.h"
#include "token.h"
#include "global.h"
#include "scanner.h"
#include "nexttok.h"
#include "error.h"
#include "studyexpr.h"
#include <addrsize.h>

static void PutZ8_IMPL(const InsDesc* Ins);    // implicit
static void PutZ8_JUMP(const InsDesc* Ins);    // jmp and jsr
static void PutZ8_V8(const InsDesc* Ins);      // <opcode> <value>
static void PutZ8_R8(const InsDesc* Ins);      // <opcode/register>
static void PutZ8_RV8(const InsDesc* Ins);     // <opcode/register> <value> OR <opcopde> <register/register>
static void PutZ8_LDSTW(const InsDesc* Ins);   // ldw and stw
static void PutZ8_ADW(const InsDesc* Ins);     // adw
static void PutZ8_STR(const InsDesc* Ins);     // str
static void PutZ8_LDR(const InsDesc* Ins);     // ldr
static void PutZ8_BRANCH(const InsDesc* Ins);  // all relative branches

/* Instruction table for the z8 */
const InsTabZ8_t InsTabZ8 = {
	sizeof(InsTabZ8.Ins) / sizeof(InsTabZ8.Ins[0]),
	{
		/* Must be in ascending mnemonic order */
		/* Mnemonic, AM, code, extra, handler */
		{ "ADD", AMZ8_RIMM|AMZ8_RREG, 0x10, 0b10011011, PutZ8_RV8 },
		{ "ADW", AMZ8_RREG|AMZ8_RIMM, 0x1C, 0x1A, PutZ8_ADW },
		{ "AND", AMZ8_RIMM|AMZ8_RREG, 0x11, 0b10101001, PutZ8_RV8 },
		{ "ASL", AMZ8_REG, 0x70, 0, PutZ8_R8 },
		{ "BCC", AMZ8_ABS|AMZ8_IMM, 0x12, 0, PutZ8_BRANCH },
		{ "BCS", AMZ8_ABS|AMZ8_IMM, 0x13, 0, PutZ8_BRANCH },
		{ "BEQ", AMZ8_ABS|AMZ8_IMM, 0x15, 0, PutZ8_BRANCH },
		{ "BIT", AMZ8_RIMM|AMZ8_RREG, 0x10, 0b10001001, PutZ8_RV8 },
		{ "BMI", AMZ8_ABS|AMZ8_IMM, 0x16, 0, PutZ8_BRANCH },
		{ "BNE", AMZ8_ABS|AMZ8_IMM, 0x14, 0, PutZ8_BRANCH },
		{ "BPL", AMZ8_ABS|AMZ8_IMM, 0x17, 0, PutZ8_BRANCH },
		{ "BRK", AMZ8_IMPL, 0x00, 0, PutZ8_IMPL },
		{ "CLF", AMZ8_IMM, 0x19, 0, PutZ8_V8 },
		{ "CMP", AMZ8_RIMM|AMZ8_RREG, 0x10, 0b10010010, PutZ8_RV8 },
		{ "DEC", AMZ8_REG, 0x48, 0, PutZ8_R8 },
		{ "HLT", AMZ8_IMPL, 0x02, 0, PutZ8_IMPL },
		{ "INC", AMZ8_REG, 0x40, 0, PutZ8_R8 },
		{ "JMP", AMZ8_ABS|AMZ8_IND, 0x00, 0, PutZ8_JUMP },
		{ "JSR", AMZ8_ABS|AMZ8_IND, 0x00, 1, PutZ8_JUMP },
		{ "LDR", AMZ8_RABS|AMZ8_RIND|AMZ8_RREG|AMZ8_RIMM, 0x00, 0, PutZ8_LDR },
		{ "LDW", AMZ8_RABS|AMZ8_RIMM_LONG, 0x38, 0x3C, PutZ8_LDSTW },
		{ "LSR", AMZ8_REG, 0x78, 0, PutZ8_R8 },
		{ "NOP", AMZ8_IMPL, 0x01, 0, PutZ8_IMPL },
		{ "OR",  AMZ8_RIMM|AMZ8_RREG, 0x11, 0b10110010, PutZ8_RV8 },
		{ "PLL", AMZ8_REG, 0x58, 0, PutZ8_R8 },
		{ "PSH", AMZ8_REG, 0x50, 0, PutZ8_R8 },
		{ "ROL", AMZ8_REG, 0x68, 0, PutZ8_R8 },
		{ "ROR", AMZ8_REG, 0x60, 0, PutZ8_R8 },
		{ "RTS", AMZ8_IMPL, 0x03, 0, PutZ8_IMPL },
		{ "STF", AMZ8_IMM, 0x18, 0, PutZ8_V8 },
		{ "STR", AMZ8_RABS|AMZ8_RIND, 0x28, 0xE0, PutZ8_STR },
		{ "STW", AMZ8_RABS, 0x34, 0, PutZ8_LDSTW },
		{ "SUB", AMZ8_RIMM|AMZ8_RREG, 0x11, 0b10100000, PutZ8_RV8 },
		{ "XOR", AMZ8_RIMM|AMZ8_RREG, 0x11, 0b10111011, PutZ8_RV8 },
	}
};

typedef struct InsInfo InsInfo;
struct InsInfo {
	unsigned long AddrMode;
	ExprNode* Expr1;
	ExprNode* Expr2;
};

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

	if (TokIsSep(CurTok.Tok)) {
		Info->AddrMode = AMZ8_IMPL;
	}
	else if (CurTok.Tok == TOK_HASH) {
		NextTok();
		Info->Expr1 = Expression(); // has to be an 8bit value
		Info->AddrMode = AMZ8_IMM;
	}
	else if (CurTok.Tok == IndirectEnter) {
		NextTok();
		Info->Expr1 = Expression(); // has to be a reg 0,2,4,6
		Consume(IndirectLeave, IndirectExpect);
		Info->AddrMode = AMZ8_IND;
	}
	else if (CurTok.Tok == TOK_BACKTICK) {
		NextTok();
		Info->Expr1 = Expression(); // has to be a reg 0-7 or a reg 0,2,4,6

		if (CurTok.Tok == TOK_COMMA) {
			NextTok();

			if (CurTok.Tok == TOK_HASH) {
				NextTok();
				Info->Expr2 = Expression(); // has to be an 8bit value or a 16bit value
				Info->AddrMode = AMZ8_RIMM | AMZ8_RIMM_LONG;
			}
			else if (CurTok.Tok == IndirectEnter) {
				NextTok();
				Info->Expr2 = Expression(); // has to be a reg 0,2,4,6
				Consume(IndirectLeave, IndirectExpect);
				Info->AddrMode = AMZ8_RIND;
			}
			else if (CurTok.Tok == TOK_BACKTICK) {
				NextTok();
				Info->Expr2 = Expression(); // has to be a reg 0-7 or a reg 0,2,4,6 depending
				Info->AddrMode = AMZ8_RREG;
			}
			else {
				Info->Expr2 = Expression(); // has to be a 16bit address
				Info->AddrMode = AMZ8_RABS;
			}
		}
		else {
			Info->AddrMode = AMZ8_REG;
		}
	}
	else {
		// fixme but EmitPcRel will read the expression!
		if (!noExpr)
			Info->Expr1 = Expression(); // has to be an 8bit offset or 16bit address
		Info->AddrMode = AMZ8_ABS; // | AMZ8_REL; // fixme kill REL? would be IMM really?
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

static char GetReg16(const ExprNode* expr) {
	long v;
	if (IsEasyConst(expr, &v)) {
		if (v == 0 || v == 2 || v == 4 || v == 6) {
			return ((char)v) >> 1;
		}
		else {
			Error("Reg16 error (%ld is not 0,2,4,6)", v);
			return 0xff;
		}
	}
	else {
		Error("Reg16 error (register expression is not a constant)");
		return 0xff;
	}
}

static char GetReg8(const ExprNode* expr) {
	long v;
	if (IsEasyConst(expr, &v)) {
		if (v >= 0 && v <= 7) {
			return (char)v;
		}
		else {
			Error("Reg8 error (%ld is not 0..7)", v);
			return 0xff;
		}
	}
	else {
		Error("Reg8 error (register expression is not a constant)");
		return 0xff;
	}
}

static void PutZ8_IMPL(const InsDesc* Ins) {

	Emit0(Ins->BaseCode);
}

static void PutZ8_JUMP(const InsDesc* Ins) {

	InsInfo Info;
	EvalInfo(Ins, &Info);
	if (Info.AddrMode == 0) return;

	if ((Info.AddrMode & AMZ8_ABS) > 0) {
		Emit2(Ins->ExtCode == 0 ? 0b00110000 : 0b00110001, Info.Expr1);
	}
	else {
		char r16 = GetReg16(Info.Expr1);
		if (r16 != 0xff) {
			Emit0((Ins->ExtCode == 0 ? 0b00001000 : 0b00001100) | r16);
		}
	}
}

static void PutZ8_R8(const InsDesc* Ins) {

	InsInfo Info;
	EvalInfo(Ins, &Info);
	if (Info.AddrMode == 0) return;

	char r8 = GetReg8(Info.Expr1);
	if (r8 != 0xff) {
		Emit0(Ins->BaseCode | r8);
	}
}

static void PutZ8_V8(const InsDesc* Ins) {

	InsInfo Info;
	EvalInfo(Ins, &Info);
	if (Info.AddrMode == 0) return;

	Emit1(Ins->BaseCode, Info.Expr1);
}

static void PutZ8_BRANCH(const InsDesc* Ins) {

	InsInfo Info;
	_EvalInfo(Ins, &Info, 1);
	if (Info.AddrMode == 0) return;

	if ((Info.AddrMode & AMZ8_IMM) > 0) {
		Emit1(Ins->BaseCode, Info.Expr1);
	}
	else {
		EmitPCRel(Ins->BaseCode, GenBranchExpr(2), 1);
	}
}

static void PutZ8_RV8(const InsDesc* Ins) {

	InsInfo Info;
	EvalInfo(Ins, &Info);
	if (Info.AddrMode == 0) return;

	if ((Info.AddrMode & AMZ8_RREG) > 0) {
		char r8a = GetReg8(Info.Expr1);
		char r8b = GetReg8(Info.Expr2);
		Emit0(Ins->BaseCode);
		Emit0((r8a << 5) | (r8b << 2) | (Ins->ExtCode & 0b00000011));
	}
	else {
		char r8 = GetReg8(Info.Expr1);
		Emit1((Ins->ExtCode & 0b11111000) | r8, Info.Expr2);
	}
}

static void PutZ8_LDSTW(const InsDesc* Ins) {

	InsInfo Info;
	EvalInfo(Ins, &Info);
	if (Info.AddrMode == 0) return;

	char r16 = GetReg16(Info.Expr1);

	if ((Info.AddrMode & AMZ8_RABS) > 0) {
		Emit2(Ins->BaseCode | r16, Info.Expr2);
	}
	else {
		Emit2(Ins->ExtCode | r16, Info.Expr2);
	}
}

static void PutZ8_ADW(const InsDesc* Ins) {

	InsInfo Info;
	EvalInfo(Ins, &Info);
	if (Info.AddrMode == 0) return;

	char r16 = GetReg16(Info.Expr1);

	if ((Info.AddrMode & AMZ8_RIMM) > 0) {
		Emit1(Ins->BaseCode | r16, Info.Expr2);
	}
	else {
		char r8 = GetReg8(Info.Expr2);
		Emit0(Ins->ExtCode);
		Emit0((r8 << 5) | (r16 << 3));
	}
}

static void PutZ8_STR(const InsDesc* Ins) {

	InsInfo Info;
	EvalInfo(Ins, &Info);
	if (Info.AddrMode == 0) return;

	char r8 = GetReg8(Info.Expr1);

	if ((Info.AddrMode & AMZ8_RABS) > 0) {
		Emit2(Ins->BaseCode | r8, Info.Expr2);
	}
	else {
		char r16 = GetReg16(Info.Expr2);
		Emit0(Ins->ExtCode | (r16 << 3) | r8);
	}
}

static void PutZ8_LDR(const InsDesc* Ins)
{
	InsInfo Info;
	EvalInfo(Ins, &Info);
	if (Info.AddrMode == 0) return;

	char r8 = GetReg8(Info.Expr1);

	if ((Info.AddrMode & AMZ8_RABS) > 0) {
		Emit2(0x20 | r8, Info.Expr2);
	}
	else if ((Info.AddrMode & AMZ8_RIND) > 0) {
		char r16 = GetReg16(Info.Expr2);
		Emit0(0xc0 | (r16 << 3) | r8);
	}
	else if ((Info.AddrMode & AMZ8_RREG) > 0) {
		char r8b = GetReg8(Info.Expr2);
		Emit0(0x10);
		Emit0((r8 << 5) | (r8b << 2));
	}
	else { // _RIMM
		Emit1(0x80, Info.Expr2);
	}
}
