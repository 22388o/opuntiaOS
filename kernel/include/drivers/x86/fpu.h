/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_DRIVERS_X86_FPU_H
#define _KERNEL_DRIVERS_X86_FPU_H

#include <libkern/types.h>
#include <platform/x86/registers.h>

typedef struct {
    uint8_t buffer[512];
} __attribute__((aligned(16))) fpu_state_t;

void fpu_handler();
void fpu_init();
void fpu_init_state(fpu_state_t* new_fpu_state);

static inline void fpu_save(fpu_state_t* fpu_state)
{
    asm volatile("fxsave %0"
                 : "=m"(*fpu_state));
}

static inline void fpu_restore(fpu_state_t* fpu_state)
{
    asm volatile("fxrstor %0"
                 :
                 : "m"(*fpu_state));
}

static inline int fpu_is_avail()
{
    return (((read_cr0() >> 3) & 0b1) == 0b0);
}

static inline void fpu_make_avail()
{
    asm volatile("clts");
}

static inline void fpu_make_unavail()
{
    // Set TS bit of cr0 to 1.
    uint32_t cr0 = read_cr0() | (1 << 3);
    write_cr0(cr0);
}

#endif //_KERNEL_DRIVERS_X86_FPU_H
