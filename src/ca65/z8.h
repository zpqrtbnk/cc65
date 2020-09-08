#pragma once

#ifndef Z8_H
#define Z8_H

#include "exprdefs.h"
#include "instr.h"

// our own instruction info structure
// which complements / replaces InsDesc
typedef struct InsInfo InsInfo;
struct InsInfo {
	unsigned char BaseCode;
	unsigned char ExtCode;

	unsigned long AddrMode;

	ExprNode* Expr1;
	ExprNode* Expr2;
};

// size of z8 instructions table
#define INSTABZ8_COUNT 36

// define z8 instructions table
typedef struct {
	unsigned Count;
	InsDesc  Ins[INSTABZ8_COUNT];
} InsTabZ8_t;

// declare z8 instruction table
// implemented in z8.c
extern const InsTabZ8_t InsTabZ8;

// define xref desc
typedef struct XRefDesc {
	char                Mnemonic[5];
	void                (*Put) (const InsInfo*);/* Handler function */
} XRefDesc;

// define xref instructions table
typedef struct {
	unsigned Count;
	XRefDesc Ins[INSTABZ8_COUNT];
} InsTabXRef_t;

// declare xref instruction table
// implemented in z8.c
extern const InsTabXRef_t InsTabXRef;

// declare xref method
void PutZ8(const char* OpCode, InsInfo* Info);

// define the addressing modes

#define AMZ8_ERR            0x000000000UL

#define AMZ8_ABS			0x000000001UL	// a
#define AMZ8_IMM            0x000000002UL   // #i
#define AMZ8_IMPL			0x000000004UL	//
#define AMZ8_IND			0x000000008UL	// (r)
#define AMZ8_RABSB			0x000000010UL	// `[]r, a
#define AMZ8_RABSW			0x000000020UL	// `r, a
#define AMZ8_REGB			0x000000040UL	// `[]r
#define AMZ8_REGW			0x000000080UL	// `r
#define AMZ8_REL			0x000000100UL	// o
#define AMZ8_RIMMB			0x000000200UL	// `[]r, #i
#define AMZ8_RIMMW			0x000000400UL	// `r, #i
#define AMZ8_RINDB			0x000000800UL	// `[]r, (r1)
#define AMZ8_RINDW			0x000001000UL	// `r, (r1)
#define AMZ8_RREGB			0x000002000UL	// `[]r, `[]r1
#define AMZ8_RREGWB			0x000004000UL	// `r, `[]r1
#define AMZ8_RREGWW         0x000008000UL	// `r, `r1

#endif Z8_H
