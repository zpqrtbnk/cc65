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

	unsigned long AddrMode; // all supported addressing modes

	ExprNode* Expr1;
	ExprNode* Expr2;
};

// size of z8 instructions table
#define INSTABZ8_COUNT 40

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

#define AMZ8_ERR        0x000000000UL

#define AMZ8_A          0x00000001UL   // Absolute
#define AMZ8_IB         0x00000002UL   // Immediate Byte
#define AMZ8_L          0x00000004UL   // reLative
#define AMZ8_M          0x00000008UL   // iMplicit
#define AMZ8_N          0x00000010UL   // iNdexed
#define AMZ8_RB         0x00000020UL   // Register Byte
#define AMZ8_RBA        0x00000040UL   // Register Byte Absolute
#define AMZ8_RBI        0x00000080UL   // Register Byte Immediate
#define AMZ8_RBN        0x00000100UL   // Register Byte iNdexed
#define AMZ8_RBNP       0x00000200UL   // Register Byte iNdexed Post
#define AMZ8_RBPN       0x00000400UL   // Register Byte Pre iNdexed
#define AMZ8_RBR        0x00000800UL   // Register Byte Register
#define AMZ8_RW         0x00001000UL   // Register Word
#define AMZ8_RWA        0x00002000UL   // Register Word Absolute
#define AMZ8_RWI        0x00004000UL   // Register Word Immediate
#define AMZ8_RWIB       0x00008000UL   // Register Word Immediate Byte
#define AMZ8_RWN        0x00010000UL   // Register Word iNdexed
#define AMZ8_RWNP       0x00020000UL   // Register Word iNdexed Post
#define AMZ8_RWPN       0x00040000UL   // Register Word Pre iNdexed
#define AMZ8_RWR        0x00080000UL   // Register Word Register
#define AMZ8_RWRB       0x00100000UL   // Register Word Register Byte

#endif Z8_H
