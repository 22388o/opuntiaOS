/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_PLATFORM_X86_PMM_SETTINGS_H
#define _KERNEL_PLATFORM_X86_PMM_SETTINGS_H

#include <platform/x86/memmap.h>

#define MEMORY_MAP_REGION 0xA00
#define PMM_BLOCK_SIZE (4096)
#define PMM_BLOCK_SIZE_KB (4)
#define PMM_BLOCKS_PER_BYTE (8)

#endif /* _KERNEL_PLATFORM_X86_PMM_SETTINGS_H */