#pragma once

#ifndef _6502Z_H
#define _6502Z_H

// size of z8 instructions table
#define INSTAB6502Z_COUNT 56

// define z8 instructions table
typedef struct {
	unsigned Count;
	InsDesc  Ins[INSTAB6502Z_COUNT];
} InsTab6502Z_t;

// declare z8 instruction table
// implemented in z8.c
extern const InsTab6502Z_t InsTab6502Z;

#endif
