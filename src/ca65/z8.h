#pragma once

#ifndef Z8_H
#define Z8_H

#include "instr.h"

// size of z8 instructions table
#define INSTABZ8_COUNT 32

// define z8 instructions table
typedef struct {
	unsigned Count;
	InsDesc  Ins[INSTABZ8_COUNT];
} InsTabZ8_t;

// declare z8 instruction table
// implemented in z8.c
extern const InsTabZ8_t InsTabZ8;

// define the addressing modes

#define AMZ8_ERR            0x000000000UL

#define AMZ8_ABS			0x000000001UL	// a
#define AMZ8_IMPL			0x000000002UL	//
#define AMZ8_IND			0x000000004UL	// (r)
#define AMZ8_RABSB			0x000000008UL	// `r, a
#define AMZ8_RABSW			0x000000010UL	// ``r, a
#define AMZ8_REGB			0x000000020UL	// `r
#define AMZ8_REGW			0x000000040UL	// ``r
#define AMZ8_REL			0x000000080UL	// o
#define AMZ8_RIMMB			0x000000100UL	// `r, #i
#define AMZ8_RIMMW			0x000000200UL	// ``r, #i
#define AMZ8_RIND			0x000000400UL	// `r, (r1)
#define AMZ8_RREGB			0x000000800UL	// `r, `r1
#define AMZ8_RREGWB			0x000001000UL	// ``r, `r1
#define AMZ8_RREGWW         0x000002000UL	// ``r, ``r1


#endif Z8_H
