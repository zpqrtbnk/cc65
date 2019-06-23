#pragma once

#ifndef Z8_H
#define Z8_H

#include "instr.h"

#define INSTABZ8_COUNT 34

typedef struct {
	unsigned Count;
	InsDesc  Ins[INSTABZ8_COUNT];
} InsTabZ8_t;

extern const InsTabZ8_t InsTabZ8;

#define AMZ8_IMPL           0x00000001UL     // implicit
#define AMZ8_IND            0x00000002UL     // (r)
#define AMZ8_RREG           0x00000004UL     // r, `r1
#define AMZ8_IMM            0x00000008UL     // #i          // i is 8 bits
#define AMZ8_RIMM           0x00000010UL     // r, #i       // i is 8 bits
#define AMZ8_RABS           0x00000020UL     // r, a        // a is 16 bits
#define AMZ8_REG            0x00000040UL     // r
#define AMZ8_RIND           0x00000080UL     // r, (r1)
#define AMZ8_REL            0x00000100UL     // o           // o is 8 bits
#define AMZ8_ABS            0x00000200UL     // a           // a is 16 bits
#define AMZ8_RIMM_LONG      0x00000400UL     // r, #i       // i is 16 buts

#endif Z8_H
