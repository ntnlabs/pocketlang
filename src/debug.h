/*
 *  Copyright (c) 2021 Thakee Nathees
 *  Licensed under: MIT License
 */

#ifndef DEBUG_H
#define DEBUG_H

#include "common.h"

// Dump the value of the [value] without a new line at the end.
void dumpValue(PKVM* vm, Var value);

// Dump opcodes of the given function.
void dumpInstructions(PKVM* vm, Function* func);

// Print stack track.
void reportStackTrace(PKVM* vm);

#endif // DEBUG_H