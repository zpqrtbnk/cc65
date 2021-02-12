




#include "ea.h"
#include "instr.h"
#include "z8.h"
#include "6502z.h"

#include "expr.h"
#include "xmalloc.h"
#include "../chrcvt65/error.h"

/*****************************************************************************/
/*                                 Forwards                                  */
/*****************************************************************************/



static void PutPCRel8(const InsDesc* Ins);
/* Handle branches with a 8 bit distance */

static void PutAll(const InsDesc* Ins);
/* Handle all other instructions */

/* Instruction table for the 6502Z */
/* Copied from instr.c */
const InsTab6502Z_t InsTab6502Z = {
    sizeof(InsTab6502Z.Ins) / sizeof(InsTab6502Z.Ins[0]),
    {
        { "ADC",  0x080A26C, 0x60, 0, PutAll },
        { "AND",  0x080A26C, 0x20, 0, PutAll },
        { "ASL",  0x000006e, 0x02, 1, PutAll },
        { "BCC",  0x0020000, 0x90, 0, PutPCRel8 },
        { "BCS",  0x0020000, 0xb0, 0, PutPCRel8 },
        { "BEQ",  0x0020000, 0xf0, 0, PutPCRel8 },
        { "BIT",  0x000000C, 0x00, 2, PutAll },
        { "BMI",  0x0020000, 0x30, 0, PutPCRel8 },
        { "BNE",  0x0020000, 0xd0, 0, PutPCRel8 },
        { "BPL",  0x0020000, 0x10, 0, PutPCRel8 },
        { "BRK",  0x0000001, 0x00, 0, PutAll },
        { "BVC",  0x0020000, 0x50, 0, PutPCRel8 },
        { "BVS",  0x0020000, 0x70, 0, PutPCRel8 },
        { "CLC",  0x0000001, 0x18, 0, PutAll },
        { "CLD",  0x0000001, 0xd8, 0, PutAll },
        { "CLI",  0x0000001, 0x58, 0, PutAll },
        { "CLV",  0x0000001, 0xb8, 0, PutAll },
        { "CMP",  0x080A26C, 0xc0, 0, PutAll },
        { "CPX",  0x080000C, 0xe0, 1, PutAll },
        { "CPY",  0x080000C, 0xc0, 1, PutAll },
        { "DEC",  0x000006C, 0x00, 3, PutAll },
        { "DEX",  0x0000001, 0xca, 0, PutAll },
        { "DEY",  0x0000001, 0x88, 0, PutAll },
        { "EOR",  0x080A26C, 0x40, 0, PutAll },
        { "INC",  0x000006c, 0x00, 4, PutAll },
        { "INX",  0x0000001, 0xe8, 0, PutAll },
        { "INY",  0x0000001, 0xc8, 0, PutAll },
        { "JMP",  0x0000808, 0x4c, 6, PutAll },
        { "JSR",  0x0000008, 0x20, 7, PutAll },
        { "LDA",  0x080A26C, 0xa0, 0, PutAll },
        { "LDX",  0x080030C, 0xa2, 1, PutAll },
        { "LDY",  0x080006C, 0xa0, 1, PutAll },
        { "LSR",  0x000006F, 0x42, 1, PutAll },
        { "NOP",  0x0000001, 0xea, 0, PutAll },
        { "ORA",  0x080A26C, 0x00, 0, PutAll },
        { "PHA",  0x0000001, 0x48, 0, PutAll },
        { "PHP",  0x0000001, 0x08, 0, PutAll },
        { "PLA",  0x0000001, 0x68, 0, PutAll },
        { "PLP",  0x0000001, 0x28, 0, PutAll },
        { "ROL",  0x000006F, 0x22, 1, PutAll },
        { "ROR",  0x000006F, 0x62, 1, PutAll },
        { "RTI",  0x0000001, 0x40, 0, PutAll },
        { "RTS",  0x0000001, 0x60, 0, PutAll },
        { "SBC",  0x080A26C, 0xe0, 0, PutAll },
        { "SEC",  0x0000001, 0x38, 0, PutAll },
        { "SED",  0x0000001, 0xf8, 0, PutAll },
        { "SEI",  0x0000001, 0x78, 0, PutAll },
        { "STA",  0x000A26C, 0x80, 0, PutAll },
        { "STX",  0x000010c, 0x82, 1, PutAll },
        { "STY",  0x000002c, 0x80, 1, PutAll },
        { "TAX",  0x0000001, 0xaa, 0, PutAll },
        { "TAY",  0x0000001, 0xa8, 0, PutAll },
        { "TSX",  0x0000001, 0xba, 0, PutAll },
        { "TXA",  0x0000001, 0x8a, 0, PutAll },
        { "TXS",  0x0000001, 0x9a, 0, PutAll },
        { "TYA",  0x0000001, 0x98, 0, PutAll }
    }
};

extern int EvalEA(const InsDesc* Ins, EffAddr* A);

static InsInfo* Info(InsInfo* info,
    const unsigned long addrMode,
    ExprNode* expr1,
    ExprNode* expr2) {
    info->AddrMode = addrMode;
    info->Expr1 = expr1;
    info->Expr2 = expr2;
    return info;
}

static void PutPCRel8(const InsDesc* Ins) {

    //EmitPCRel(Ins->BaseCode, GenBranchExpr(2), 1);

	//InsInfo info;
    //ExprNode* expr = Expression();

    Error("NOT IMPLEMENTED");

	/*
	switch (Ins->BaseCode)
	{
    case 0x90: //BCC
        PutZ8("BCC", Info(&info, AMZ8_REL, NULL, NULL));
        break;
    case 0xb0: //BCS
        PutZ8("BCS", Info(&info, AMZ8_REL, NULL, NULL));
        break;
    case 0xf0: //BEQ
        PutZ8("BEQ", Info(&info, AMZ8_REL, NULL, NULL));
        break;
    case 0x30: //BMI
        PutZ8("BMI", Info(&info, AMZ8_REL, NULL, NULL));
        break;
    case 0xd0: //BNE
        PutZ8("BNE", Info(&info, AMZ8_REL, NULL, NULL));
        break;
    case 0x10: //BPL
        PutZ8("BPL", Info(&info, AMZ8_REL, NULL, NULL));
        break;
    case 0x50: //BVC
        // not supported for now
        Error("Unsupported opcode BVC");
        break;
    case 0x70: //BVS
        // not supported for now
        Error("Unsupported opcode BVS");
        break;
	}
	*/
}

//#define INF PutZ8("CLF", X(fX));
#define INF PutZ8("CLF", Info(&info, AMZ8_IMM, X(fX), NULL));
//#define ALF PutZ8("STF", X(fX));
#define ALF PutZ8("STF", Info(&info, AMZ8_IMM, X(fX), NULL));

// [ [IND+X] ]
// T <- [[ABS+X]]
#define LDT_IX \
	INF \
	PutZ8("LDR", Info(&info, AMZ8_RIMMW, X(rI), A.Expr)); /* ldr `I, #<address> */ \
	PutZ8("ADD", Info(&info, AMZ8_RREGWB, X(rI), X(rX)));   /* add `I, `X         */ \
	PutZ8("LDR", Info(&info, AMZ8_RINDW, X(rI), X(rI)));    /* ldr `I, (I)        */ \
	PutZ8("LDR", Info(&info, AMZ8_RINDB, X(rT), X(rI)));    /* ldr `T, (I)        */ \
	ALF

// [ [IND] + Y ]
// T <- [[ABS]+Y]
#define LDT_IY \
	INF \
	PutZ8("LDR", Info(&info, AMZ8_RABSW, X(rI), A.Expr)); /* ldr `I, <address>  */ \
	PutZ8("ADD", Info(&info, AMZ8_RREGWB, X(rI), X(rY)));   /* add `I, `Y         */ \
	PutZ8("LDR", Info(&info, AMZ8_RINDB, X(rT), X(rI)));    /* ldr `T, (I)        */ \
	ALF

// T <- [ABS]
#define LDT_A \
	INF \
	PutZ8("LDR", Info(&info, AMZ8_RABSB, X(rT), A.Expr)); /* ldr `T, <address>  */ \
	ALF

// T <- [ABS+X]
#define LDT_AX \
	INF \
	PutZ8("LDR", Info(&info, AMZ8_RIMMW, X(rI), A.Expr)); /* ldr `I, #<address> */ \
	PutZ8("ADD", Info(&info, AMZ8_RREGWB, X(rI), X(rX)));   /* add `I, `X         */ \
	PutZ8("LDR", Info(&info, AMZ8_RINDB, X(rT), X(rI)));    /* ldr `T, (I)        */ \
	ALF

// T <- [ABS+Y]
#define LDT_AY \
	INF \
	PutZ8("LDR", Info(&info, AMZ8_RIMMW, X(rI), A.Expr)); /* ldr `I, #<address> */ \
	PutZ8("ADD", Info(&info, AMZ8_RREGWB, X(rI), X(rY)));   /* add `I, `Y         */ \
	PutZ8("LDR", Info(&info, AMZ8_RINDB, X(rT), X(rI)));    /* ldr `T, (I)        */ \
	ALF

static ExprNode* X(ExprNode n)
{
	// see:
    return xdup(&n, sizeof(n));

    /*ExprNode* m = NULL;
    m->Op = n.Op;
    m->V.IVal = n.V.IVal;
    return m;*/
}

static void PutAll(const InsDesc* Ins)
/* Handle all other instructions */
{
  //  EffAddr A;
  //  InsInfo info;

  //  /* Evaluate the addressing mode used */
  //  if (EvalEA(Ins, &A)) {
  //      /* No error, output code */
  //      //EmitCode(&A);

  //      ExprNode r0, r1, r2, r3;
  //      r0.Op = r1.Op = r2.Op = r3.Op = EXPR_LITERAL;
  //      r0.Left = r1.Left = r2.Left = r3.Left = NULL;
  //      r0.Right = r1.Right = r2.Right = r3.Right = NULL;
  //      r0.V.IVal = 0;
  //      r1.V.IVal = 1;
  //      r2.V.IVal = 2;
  //      r3.V.IVal = 3;

  //  	// r0: T, A
  //  	// r1: X, Y
  //  	// r2: I
  //  	// r3:

  //      ExprNode rT, rA, rX, rY, rI;
  //      rT.Op = rA.Op = rX.Op = rY.Op = rI.Op = EXPR_LITERAL;
  //      rT.Left = rA.Left = rX.Left = rY.Left = rI.Left = NULL;
  //      rT.Right = rA.Right = rX.Right = rY.Right = rI.Right = NULL;
  //      rT.V.IVal = r0.V.IVal + 4; // &0b100;
  //      rA.V.IVal = r0.V.IVal;
  //      rX.V.IVal = r1.V.IVal + 4; // &0b100;
  //      rY.V.IVal = r1.V.IVal;
  //      rI.V.IVal = r2.V.IVal;

  //      ExprNode fX, fN, fC, fZ;
  //      fX.Op = fN.Op = fC.Op = fZ.Op = EXPR_LITERAL;
  //      fX.Left = fN.Left = fC.Left = fZ.Left = NULL;
  //      fX.Right = fN.Right = fC.Right = fZ.Right = NULL;
  //      fX.V.IVal = 0x80;
  //      fN.V.IVal = 0x01; // FIXME ACTUAL?
  //      fZ.V.IVal = 0x02;
  //      fC.V.IVal = 0x04;

  //  	switch (A.Opcode)
  //  	{
  //      case 0x00: // BRK
  //          PutZ8("BRK", Info(&info, AMZ8_IMPL, NULL, NULL));
  //          break;

  //      case 0x01: // ORA (IND,X)
  //          LDT_IX
  //          PutZ8("OR", Info(&info, AMZ8_RREGB, X(rA), X(rT)));     // or `A, `T
  //          break;

  //      case 0x05: // ORA ZP
  //      case 0x0D: // ORA ABS
  //          LDT_A
  //          PutZ8("OR", Info(&info, AMZ8_RREGB, X(rA), X(rT)));     // or `A, `T
  //          break;

  //  	case 0x06: // ASL ZP
  //      case 0x0E: // ASL ABS
  //          PutZ8("LDR", Info(&info, AMZ8_RABSB, X(rT), A.Expr)); // ldr `T, <address>
  //          PutZ8("ASL", Info(&info, AMZ8_REGB, X(rT), NULL));    // asl `T
  //          PutZ8("STR", Info(&info, AMZ8_RABSB, X(rT), A.Expr)); // str `T, <address>
  //          break;

  //      case 0x08: // PHP
  //          PutZ8("PSF", Info(&info, AMZ8_IMPL, NULL, NULL));   // psf
  //          break;

  //      case 0x09: // ORA IMM
  //          PutZ8("OR", Info(&info, AMZ8_RIMMB, X(rA), A.Expr));    // or `A, #<value>
  //          break;

  //      case 0x0A: // ASL A
  //          PutZ8("ASL", Info(&info, AMZ8_REGB, X(rA), NULL));    // asl `A
  //          break;

  //      //case 0x0D: 0x05
  //  	//case 0x0E: 0x06

  //  	case 0x11: // ORA (IND),Y
  //          LDT_IY
  //          PutZ8("OR", Info(&info, AMZ8_RREGB, X(rA), X(rT)));     // or `A, `T
  //          break;

  //      case 0x16: // ASL ZP,X
  //      case 0x1E: // ASL ABS,X
  //  		LDT_AX
  //          PutZ8("ASL", Info(&info, AMZ8_REGB, X(rT), NULL));    // asl `T
  //          PutZ8("STR", Info(&info, AMZ8_RINDB, X(rT), X(rI)));    // str `T, (I)
  //          break;

  //      case 0x18: // CLC
  //          PutZ8("CLF", Info(&info, AMZ8_IMM, X(fC), NULL));     // clf #FLAGS_C
  //          break;

  //      case 0x19: // ORA ABS,Y
  //          LDT_AY
  //          PutZ8("OR", Info(&info, AMZ8_RREGB, X(rA), X(rT)));     // or `A, `T
  //          break;

  //      //case 0x1E: 0x16

  //      case 0x20: // JSR ABS
  //          PutZ8("JSR", Info(&info, AMZ8_ABS, A.Expr, NULL));  // jsr <address>
  //          break;

  //      case 0x21: // AND (IND,X)
  //          LDT_IX
  //          PutZ8("AND", Info(&info, AMZ8_RREGB, X(rA), X(rT)));    // and `A, `T
  //          break;

  //  	case 0x25: // AND ZP
  //      case 0x2D: // AND ABS
  //  		LDT_A
  //          PutZ8("AND", Info(&info, AMZ8_RREGB, X(rA), X(rT)));    // and `A, `T
  //          break;

  //      case 0x26: // ROL ZP
  //      case 0x2E: // ROL ABS
  //          PutZ8("LDR", Info(&info, AMZ8_RABSB, X(rT), A.Expr)); // ldr `T, <address>
  //          PutZ8("ROL", Info(&info, AMZ8_REGB, X(rT), NULL));    // rol `T
  //          PutZ8("STR", Info(&info, AMZ8_RABSB, X(rT), A.Expr)); // str `T, <address>
  //          break;

  //      case 0x28: // PLP
  //          PutZ8("PLF", Info(&info, AMZ8_IMPL, NULL, NULL));   // plf
  //          break;

  //  	case 0x29: // AND IMM
  //          PutZ8("AND", Info(&info, AMZ8_RIMMB, X(rA), A.Expr));   // and `A, #<value>
  //          break;

  //      case 0x2A: // ROL A
  //          PutZ8("ROL", Info(&info, AMZ8_IMPL, X(rA), NULL));   // rol `A
  //          break;

  //      //case 0x2D: 0x25
  //  	//case 0x2E: 0x26

  //      case 0x31: // AND (IND),Y
  //          LDT_IY
  //          PutZ8("AND", Info(&info, AMZ8_RREGB, X(rA), X(rT)));    // and `A, `T
  //          break;

  //  	case 0x35: // AND ZP,X
  //      case 0x3D: // AND ABS,X
  //  		LDT_AX
  //          PutZ8("AND", Info(&info, AMZ8_RREGB, X(rA), X(rT)));    // and `A, `T
  //          break;

  //      case 0x38: // SEC
  //          PutZ8("STF", Info(&info, AMZ8_IMM, X(fC), NULL));   // stf #FLAG_C
  //          break;

  //  		case 0x36: // ROL ZP,X
  //      case 0x3E: // ROL ABS,X
  //          LDT_AX
  //          PutZ8("ROL", Info(&info, AMZ8_REGB, X(rT), NULL));    // rol `T
  //          PutZ8("STR", Info(&info, AMZ8_RINDB, X(rT), X(rI)));    // str `T, (I)
  //          break;

  //      case 0x39: // AND ABS,Y
  //          LDT_AY
  //          PutZ8("AND", Info(&info, AMZ8_RREGB, X(rA), X(rT)));    // and `A, `T
  //          break;

  //  	//case 0x3D: 0x35
  //      //case 0x3E: 0x36

  //      case 0x40: // RTI
  //          PutZ8("NOP", Info(&info, AMZ8_IMPL, NULL, NULL));   // n/a
  //          break;

  //      case 0x41: // EOR (IND,X)
  //          LDT_IX
  //          PutZ8("XOR", Info(&info, AMZ8_RREGB, X(rA), X(rT)));    // xor `A, `T
  //          break;

  //      case 0x45: // EOR ZP
  //      case 0x4D: // EOR ABS
  //          LDT_A
  //          PutZ8("XOR", Info(&info, AMZ8_RREGB, X(rA), X(rT)));    // xor `A, `T
  //          break;

  //      case 0x46: // LSR ZP
  //      case 0x4E: // LSR ABS
  //          PutZ8("LDR", Info(&info, AMZ8_RABSB, X(rT), A.Expr)); // ldr `T, <address>
  //          PutZ8("LSR", Info(&info, AMZ8_REGB, X(rT), NULL));    // lsr `T
  //          PutZ8("STR", Info(&info, AMZ8_RABSB, X(rT), A.Expr)); // str `T, <address>
  //          break;

  //      case 0x48: // PHA
  //          PutZ8("PSR", Info(&info, AMZ8_REGB, X(rA), NULL));    // psr `A
  //          break;

  //  	case 0x49: // EOR IMM
  //          PutZ8("XOR", Info(&info, AMZ8_RIMMB, X(rA), A.Expr));   // xor `A, #<value>
  //          break;

  //      case 0x4A: // LSR
  //          PutZ8("LSR", Info(&info, AMZ8_REGB, X(rA), NULL));   // lsr `A
  //          break;

  //  	case 0x4C: // JMP ABS
  //          PutZ8("JMP", Info(&info, AMZ8_ABS, A.Expr, NULL));   // jmp <address>
  //          break;

  //  	//case 0x4D: 0x45
  //      //case 0x4E: 0x46

  //      case 0x51: // EOR (IND),Y
  //          LDT_IY
  //          PutZ8("XOR", Info(&info, AMZ8_RREGB, X(rA), X(rT)));    // xor `A, `T
  //          break;

  //      case 0x55: // EOR ZP,X
  //      case 0x5D: // EOR ABS,X
  //          LDT_AX
  //          PutZ8("XOR", Info(&info, AMZ8_RREGB, X(rA), X(rT)));    // xor `A, `T
  //          break;

  //      case 0x56: // LSR ZP,X
  //      case 0x5E: // LSR ABS,X
  //          LDT_AX
  //          PutZ8("LSR", Info(&info, AMZ8_REGB, X(rT), NULL));    // lsr `T
  //          PutZ8("STR", Info(&info, AMZ8_RINDB, X(rT), X(rI)));    // str `T, (I)
  //          break;

  //      case 0x58: // CLI
  //          // not supported
  //          break;

  //      case 0x59: // EOR ABS,Y
  //          LDT_AY
  //          PutZ8("XOR", Info(&info, AMZ8_RREGB, X(rA), X(rT)));    // xor `A, `T
  //          break;

  //  	//case 0x5D: 0x55
  //      //case 0x5E: 0x56

  //      case 0x60: // RTS
  //          PutZ8("RTS", Info(&info, AMZ8_IMPL, NULL, NULL));   // rts
  //          break;

  //  	case 0x61: // ADC (IND,X)
  //          LDT_IX
  //          PutZ8("ADD", Info(&info, AMZ8_RREGB, X(rA), X(rT)));    // add `A, `T
  //          break;

  //      case 0x65: // ADC ZP
  //      case 0x6D: // ADC ABS
  //  		LDT_A
  //          PutZ8("ADD", Info(&info, AMZ8_RREGB, X(rA), X(rT)));    // add `A, `T
  //          break;

  //      case 0x66: // ROR ZP
  //      case 0x6E: // ROR ABS
  //          PutZ8("LDR", Info(&info, AMZ8_RABSB, X(rT), A.Expr)); // ldr `T, <address>
  //          PutZ8("ROR", Info(&info, AMZ8_REGB, X(rT), NULL));    // ror `T
  //          PutZ8("STR", Info(&info, AMZ8_RABSB, X(rT), A.Expr)); // str `T, <address>
  //          break;

  //      case 0x68: // PLA
  //          PutZ8("PLR", Info(&info, AMZ8_REGB, X(rA), NULL));    // plr `A
  //          break;

		//case 0x69: // ADC IMM
  //          PutZ8("ADD", Info(&info, AMZ8_RIMMB, X(rA), A.Expr)); // add `A, #<value>
  //          break;

  //      case 0x6A: // ROR A
  //          PutZ8("ROR", Info(&info, AMZ8_IMPL, X(rA), NULL));   // ror `A
  //          break;

  //      case 0x6C: // JMP IND
  //          INF
  //          PutZ8("LDR", Info(&info, AMZ8_RABSW, X(rI), A.Expr));   // ldr `I, <address>
  //          ALF
  //          PutZ8("JMP", Info(&info, AMZ8_IND, X(rI), NULL));     // jmp (I)
  //          break;

  //      //case 0x6D: 0x65
  //      //case 0x6E: 0x66

  //      case 0x71: // ADC (IND),Y
  //          LDT_IY
  //          PutZ8("ADD", Info(&info, AMZ8_RREGB, X(rA), X(rT)));    // add `A, `T
  //          break;

  //  	case 0x75: // ADC ZP,X
  //      case 0x7D: // ADC ABS,X
  //  		LDT_AX
  //          PutZ8("ADD", Info(&info, AMZ8_RREGB, X(rA), X(rT)));    // add `A, `T
  //          break;

  //      case 0x76: // ROR ZP,X
  //      case 0x7E: // ROR ABS,X
  //          LDT_AX
  //          PutZ8("ROR", Info(&info, AMZ8_REGB, X(rT), NULL));    // ror `T
  //          PutZ8("STR", Info(&info, AMZ8_RINDB, X(rT), X(rI)));    // str `T, (I)
  //          break;

  //      case 0x78: // SEI
  //          PutZ8("NOP", Info(&info, AMZ8_IMPL, NULL, NULL));   // nop
  //          break;

  //  	case 0x79: // ADC ABS,Y
  //          LDT_AY
  //          PutZ8("ADD", Info(&info, AMZ8_RREGB, X(rA), X(rT)));    // add `A, `T
  //          break;

  //      //case 0x7D: 0x75
  //      //case 0x7E: 0x76

  //      case 0x81: // STA (IND,X)
  //          LDT_IX
  //          PutZ8("STR", Info(&info, AMZ8_RINDB, X(rA), X(rI)));    // str `A, (I)
  //          break;

  //      case 0x84: // STY ZP
  //      case 0x8C: // STY ABS
  //          PutZ8("STR", Info(&info, AMZ8_RABSB, X(rY), A.Expr));   // str `Y, <address>
  //          break;

  //  	case 0x85: // SDA ZP
  //      case 0x8D: // SDA ABS
  //          PutZ8("STR", Info(&info, AMZ8_RABSB, X(rA), A.Expr));   // str `A, <address>
  //          break;

  //      case 0x86: // STX ZP
  //      case 0x8E: // STX ABS
  //          PutZ8("STR", Info(&info, AMZ8_RABSB, X(rX), A.Expr));   // str `X, <address>
  //          break;

  //      case 0x88: // DEY IMPL
  //          PutZ8("DEC", Info(&info, AMZ8_REGB, X(rY), NULL));    // dec `Y
  //          break;

  //      case 0x8A: // TXA
  //          PutZ8("TXR", Info(&info, AMZ8_RREGB, X(rA), X(rX)));    // txr `A, `X
  //          break;

  //      //case 0x8C: 0x84
  //      //case 0x8D: 0x85
  //      //case 0x8E: 0x86

  //      case 0x91: // STA (IND),Y
  //          LDT_IY
  //          PutZ8("STR", Info(&info, AMZ8_RINDB, X(rA), X(rI)));    // str `A, (I)
  //          break;

  //      case 0x94: // STY ZP,X
  //          LDT_AX
  //          PutZ8("STR", Info(&info, AMZ8_RINDB, X(rY), X(rI)));    // str `Y, (I)
  //          break;

  //  	case 0x95: // STA ZP,X
  //      case 0x9D: // STA ABS,X
  //  		LDT_AX
  //          PutZ8("STR", Info(&info, AMZ8_RINDB, X(rA), X(rI)));    // str `A, (I)
  //          break;

  //      case 0x96: // STX ZP,Y
  //          LDT_AY
  //          PutZ8("STR", Info(&info, AMZ8_RINDB, X(rX), X(rI)));    // str `X, (I)
  //          break;

  //      case 0x98: // TYA
  //          PutZ8("TXR", Info(&info, AMZ8_RREGB, X(rA), X(rY)));    // txr `A, `Y
  //          break;

  //      case 0x99: // STA ABS,Y
  //          LDT_AY
  //          PutZ8("STR", Info(&info, AMZ8_RINDB, X(rA), X(rI)));    // str `A, (I)
  //          break;

  //      case 0x9A: // TXS
  //          PutZ8("TSR", Info(&info, AMZ8_REGW, X(r3), NULL));    // tsr `r3
  //          PutZ8("TXR", Info(&info, AMZ8_RREGB, X(rX), X(r3)));    // txr `A, `]r3
  //          break;

  //  	//case 0x9D: 0x95

  //      case 0xA0: // LDY IMM
  //          PutZ8("LDR", Info(&info, AMZ8_RIMMB, X(rY), A.Expr));   // ldr `Y, #<value>
  //          break;

  //  	case 0xA1: // LDA (IND,X)
  //          LDT_IX
  //          PutZ8("LDR", Info(&info, AMZ8_RINDB, X(rA), X(rI)));    // ldr `A, (I)
  //          break;

  //      case 0xA2: // LDX IMM
  //          PutZ8("LDR", Info(&info, AMZ8_RIMMB, X(rX), A.Expr));   // ldr `X, #<value>
  //          break;

  //      case 0xA4: // LDY ZP
  //      case 0xAC: // LDY ABS
  //          PutZ8("LDR", Info(&info, AMZ8_RABSB, X(rY), A.Expr));   // ldr `Y, <address>
  //          break;

  //      case 0xA5: // LDA ZP
  //      case 0xAD: // LDA ABS
  //          PutZ8("LDR", Info(&info, AMZ8_RABSB, X(rA), A.Expr));   // ldr `A, <address>
  //          break;

  //      case 0xA6: // LDX ZP
  //      case 0xAE: // LDX ABS
  //          PutZ8("LDR", Info(&info, AMZ8_RABSB, X(rX), A.Expr));   // ldr `X, <address>
  //          break;

  //      case 0xA8: // TAY
  //          PutZ8("TXR", Info(&info, AMZ8_RREGB, X(rY), X(rA)));    // txr `Y, `A
  //          break;

  //  	case 0xA9: // LDA IMM
  //          PutZ8("LDR", Info(&info, AMZ8_RIMMB, X(rA), A.Expr));   // ldr `A, #<value>
  //          break;

  //      case 0xAA: // TAX
  //          PutZ8("TXR", Info(&info, AMZ8_RREGB, X(rX), X(rA)));    // txr `X, `A
  //          break;

	 //   //case 0xAC: 0xA4
	 //   //case 0xAD: 0xA5
  //  	//case 0xAE: 0xA6

  //      case 0xB1: // LDA (ABS),Y
  //          LDT_IY
  //          PutZ8("LDR", Info(&info, AMZ8_RINDB, X(rA), X(rI)));    // ldr `A, (I)
  //          break;

  //      case 0xB4: // LDY ZP,X
  //      case 0xBC: // LDY ABS,X
  //  		LDT_AX
  //          PutZ8("LDR", Info(&info, AMZ8_RINDB, X(rY), X(rI)));    // ldr `Y, (I)
  //          break;

  //  	case 0xB5: // LDA ZP,X
  //      case 0xBD: // LDA ABS,X
  //          LDT_AX
  //          PutZ8("LDR", Info(&info, AMZ8_RINDB, X(rA), X(rI)));    // ldr `A, (I)
  //          break;

  //      case 0xB6: // LDX ZP,Y
  //      case 0xBE: // LDX ABS,Y
  //          LDT_AY
  //          PutZ8("LDR", Info(&info, AMZ8_RINDB, X(rX), X(rI)));    // ldr `X, (I)
  //          break;

  //      case 0xB8: // CLV
  //          // not supported
  //          break;

  //      case 0xB9: // LDA ABS,Y
  //          LDT_AY
  //          PutZ8("LDR", Info(&info, AMZ8_RINDB, X(rA), X(rI)));    // ldr `A, (I)
  //          break;

  //      case 0xBA: // TSX
  //          PutZ8("TXR", Info(&info, AMZ8_RREGB, X(r3), X(rX)));     // txr `]r3, `X
  //          PutZ8("TRS", Info(&info, AMZ8_REGW, X(r3), NULL));     // trs `r3
  //          break;

  //      //case 0xBC: 0xB4
  //  	//case 0xBD: 0xB5
  //      //case 0xBE: 0xB6

  //      case 0xC0: // CPY IMM
  //          PutZ8("CMP", Info(&info, AMZ8_RIMMB, X(rY), A.Expr)); // cmp `Y, #<value>
  //          break;

  //  	case 0xC1: // CMP (IND,X)
  //          LDT_IX
  //          PutZ8("CMP", Info(&info, AMZ8_RREGB, X(rA), X(rT)));     // cmp `A, `T
  //          break;

  //      case 0xC4: // CPY ZP
  //      case 0xCC: // CPY ABS
  //          LDT_A
  //              PutZ8("CMP", Info(&info, AMZ8_RREGB, X(rY), X(rT))); // cmp `Y, `T
  //          break;

  //      case 0xC5: // CMP ZP
  //      case 0xCD: // CMP ABS
  //  		LDT_A
  //          PutZ8("CMP", Info(&info, AMZ8_RREGB, X(rA), X(rT)));     // cmp `A, `T
  //          break;

  //      case 0xC6: // DEC ZP
  //      case 0xCE: // DEC ABS
  //          PutZ8("LDR", Info(&info, AMZ8_RABSB, X(rT), A.Expr)); // ldr `T, <address>
  //  		PutZ8("DEC", Info(&info, AMZ8_REGB, X(rT), NULL));    // dec `T
  //          PutZ8("STR", Info(&info, AMZ8_RABSB, X(rT), A.Expr)); // str `T, <address>
  //          break;

  //      case 0xC8: // INY IMPL
  //          PutZ8("INC", Info(&info, AMZ8_REGB, X(rY), NULL));    // inc `Y
  //          break;

  //  	case 0xC9: // CMP IMM
  //          PutZ8("CMP", Info(&info, AMZ8_RIMMB, X(rA), A.Expr)); // cmp `A, #<value>
  //          break;

  //      case 0xCA: // DEX IMPL
  //          PutZ8("DEC", Info(&info, AMZ8_REGB, X(rX), NULL));    // dec `X
  //          break;

  //      //case 0xCC: 0xC4
  //      //case 0xCD: 0xC5
  //      //case 0xCE: 0xC6

  //      case 0xD1: // CMP (IND),Y
  //          LDT_IY
  //          PutZ8("CMP", Info(&info, AMZ8_RREGB, X(rA), X(rT)));     // cmp `A, `T
  //          break;

  //      case 0xD5: // CMD ZP,X
  //      case 0xDD: // CMP ABS,X
  //  		LDT_AX
  //          PutZ8("CMP", Info(&info, AMZ8_RREGB, X(rA), X(rT)));     // cmp `A, `T
  //          break;

  //      case 0xD6: // DEC ZP,X
  //      case 0xDE: // DEC ABS,X
  //          LDT_AX
  //          PutZ8("DEC", Info(&info, AMZ8_RREGB, X(rT), NULL));   // dec `T
  //          PutZ8("STR", Info(&info, AMZ8_RINDB, X(rT), X(rI)));    // str `T, (I)
  //          break;

  //      case 0xD8: // CLD
  //          // not supported
  //          break;

  //      case 0xD9: // CMP ABS,Y
  //          LDT_AY
  //          PutZ8("CMP", Info(&info, AMZ8_RREGB, X(rA), X(rT)));     // cmp `A, `T
  //          break;

  //  	//case 0xDD: 0xD5
  //  	//case 0xDE: 0xD6

  //      case 0xE0: // CPX IMM
  //          PutZ8("CMP", Info(&info, AMZ8_RIMMB, X(rX), A.Expr)); // cmp `X, #<value>
  //          break;

  //  	case 0xE1: // SBC (IND,X)
  //          LDT_IX
  //          PutZ8("SUB", Info(&info, AMZ8_RREGB, X(rA), X(rT)));    // sub `A, `T
  //          break;

  //      case 0xE4: // CPX ZP
  //      case 0xEC: // CPX ABS
  //          LDT_A
  //          PutZ8("CMP", Info(&info, AMZ8_RREGB, X(rX), X(rT)));     // cmp `A, `T
  //          break;

  //      case 0xE5: // SBC ZP
  //      case 0xED: // SBC ABS
  //          LDT_A
  //          PutZ8("SUB", Info(&info, AMZ8_RREGB, X(rA), X(rT)));    // sub `A, `T
  //          break;

  //      case 0xE6: // INC ZP
  //      case 0xEE: // INC ABS
  //          PutZ8("LDR", Info(&info, AMZ8_RABSB, X(rT), A.Expr)); // ldr `T, <address>
  //          PutZ8("INC", Info(&info, AMZ8_REGB, X(rT), NULL));    // inc `T
  //          PutZ8("STR", Info(&info, AMZ8_RABSB, X(rT), A.Expr)); // str `T, <address>
  //          break;

  //      case 0xE8: // INX IMPL
  //          PutZ8("INC", Info(&info, AMZ8_REGB, X(rX), NULL));    // inc `X
  //          break;

  //      case 0xE9: // SBC IMM
  //          PutZ8("SUB", Info(&info, AMZ8_RIMMB, X(rA), A.Expr)); // sub `A, #<value>
  //          break;

  //  	case 0xEA: // NOP
  //          PutZ8("NOP", Info(&info, AMZ8_IMPL, NULL, NULL));    // nop
  //          break;

  //  	//case 0xEC: 0xE4
  //      //case 0xED: 0xE5
  //      //case 0xEE: 0xE6

  //      case 0xF1: // SBC (IND),Y
  //          LDT_IY
  //          PutZ8("SUB", Info(&info, AMZ8_RREGB, X(rA), X(rT)));    // sub `A, `T
  //          break;

  //      case 0xF5: // SBC ZP,X
  //      case 0xFD: // SBC ABS,X
  //          LDT_AX
  //          PutZ8("SUB", Info(&info, AMZ8_RREGB, X(rA), X(rT)));    // sub `A, `T
  //          break;

  //  	case 0xF6: // INC ZP,X
  //      case 0xFE: // INC ABS,X
  //          LDT_AX
  //          PutZ8("INC", Info(&info, AMZ8_RREGB, X(rT), NULL));   // inc `T
  //          PutZ8("STR", Info(&info, AMZ8_RINDB, X(rT), X(rI)));    // str `T, (I)
  //          break;

  //      case 0xF8: // SED
  //          PutZ8("NOP", Info(&info, AMZ8_IMPL, NULL, NULL));   // nop
  //          break;

  //  	case 0xF9: // SBC ABS,Y
  //          LDT_AY
  //          PutZ8("SUB", Info(&info, AMZ8_RREGB, X(rA), X(rT)));    // sub `A, `T
  //          break;

  //  	//case 0xFE: 0xF6
  //      }
  //  }
}
