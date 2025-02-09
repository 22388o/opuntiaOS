/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <fs/vfs.h>
#include <libkern/bits/errno.h>
#include <libkern/log.h>
#include <mem/kmalloc.h>
#include <tasking/proc.h>

/**
 * PROC ZONING
 */

static inline bool _proc_zones_intersect(uint32_t start1, uint32_t size1, uint32_t start2, uint32_t size2)
{
    uint32_t end1 = start1 + size1 - 1;
    uint32_t end2 = start2 + size2 - 1;
    return (start1 <= start2 && start2 <= end1) || (start1 <= end2 && end2 <= end1) || (start2 <= start1 && start1 <= end2) || (start2 <= end1 && end1 <= end2);
}

static inline bool _proc_can_fixup_zone(proc_t* proc, uint32_t* start_ptr, int* len_ptr)
{
    uint32_t zones_count = proc->zones.size;

    for (uint32_t i = 0; i < zones_count; i++) {
        proc_zone_t* zone = (proc_zone_t*)dynamic_array_get(&proc->zones, i);
        if (_proc_zones_intersect(*start_ptr, *len_ptr, zone->start, zone->len)) {
            if (*start_ptr >= zone->start) {
                int move = (zone->start + zone->len) - (*start_ptr);
                *start_ptr += move;
                *len_ptr -= move;
            } else {
                int move = (*start_ptr + *len_ptr) - zone->start;
                *len_ptr -= move;
            }

            if (*len_ptr <= 0) {
                return false;
            }
        }
    }

    return true;
}

static inline bool _proc_can_add_zone(proc_t* proc, uint32_t start, uint32_t len)
{
    uint32_t zones_count = proc->zones.size;

    for (uint32_t i = 0; i < zones_count; i++) {
        proc_zone_t* zone = (proc_zone_t*)dynamic_array_get(&proc->zones, i);
        if (_proc_zones_intersect(start, len, zone->start, zone->len)) {
            return false;
        }
    }

    return true;
}

static inline void _proc_swap_zones(proc_zone_t* one, proc_zone_t* two)
{
    proc_zone_t tmp = *one;
    one->file = two->file;
    one->flags = two->flags;
    one->len = two->len;
    one->offset = two->offset;
    one->start = two->start;
    one->type = two->type;

    two->file = tmp.file;
    two->flags = tmp.flags;
    two->len = tmp.len;
    two->offset = tmp.offset;
    two->start = tmp.start;
    two->type = tmp.type;
}

/**
 * Inserts zone, which won't overlap with existing ones.
 */
proc_zone_t* proc_extend_zone(proc_t* proc, uint32_t start, uint32_t len)
{
    len += (start & (VMM_PAGE_SIZE - 1));
    start &= ~(VMM_PAGE_SIZE - 1);
    if (len % VMM_PAGE_SIZE) {
        len += VMM_PAGE_SIZE - (len % VMM_PAGE_SIZE);
    }

    proc_zone_t new_zone = { 0 };
    new_zone.type = 0;
    new_zone.flags = ZONE_USER;

    if (_proc_can_fixup_zone(proc, &start, (int*)&len)) {
        new_zone.start = start;
        new_zone.len = len;
        if (dynamic_array_push(&proc->zones, &new_zone) != 0) {
            return 0;
        }
        return (proc_zone_t*)dynamic_array_get(&proc->zones, proc->zones.size - 1);
    }

    return 0;
}

proc_zone_t* proc_new_zone(proc_t* proc, uint32_t start, uint32_t len)
{
    len += (start & (VMM_PAGE_SIZE - 1));
    start &= ~(VMM_PAGE_SIZE - 1);
    if (len % VMM_PAGE_SIZE) {
        len += VMM_PAGE_SIZE - (len % VMM_PAGE_SIZE);
    }

    proc_zone_t new_zone = { 0 };
    new_zone.start = start;
    new_zone.len = len;
    new_zone.type = 0;
    new_zone.flags = ZONE_USER;

    if (_proc_can_add_zone(proc, start, len)) {
        if (dynamic_array_push(&proc->zones, &new_zone) != 0) {
            return 0;
        }
        return (proc_zone_t*)dynamic_array_get(&proc->zones, proc->zones.size - 1);
    }

    return 0;
}

/* FIXME: Think of more efficient way */
proc_zone_t* proc_new_random_zone(proc_t* proc, uint32_t len)
{
    if (len % VMM_PAGE_SIZE) {
        len += VMM_PAGE_SIZE - (len % VMM_PAGE_SIZE);
    }

    uint32_t zones_count = proc->zones.size;

    /* Check if we can put it at the beginning */
    proc_zone_t* ret = proc_new_zone(proc, 0, len);
    if (ret) {
        return ret;
    }

    uint32_t min_start = 0xffffffff;

    for (uint32_t i = 0; i < zones_count; i++) {
        proc_zone_t* zone = (proc_zone_t*)dynamic_array_get(&proc->zones, i);
        if (_proc_can_add_zone(proc, zone->start + zone->len, len)) {
            if (min_start > zone->start + zone->len) {
                min_start = zone->start + zone->len;
            }
        }
    }

    if (min_start == 0xffffffff) {
        return 0;
    }

    return proc_new_zone(proc, min_start, len);
}

/* FIXME: Think of more efficient way */
proc_zone_t* proc_new_random_zone_backward(proc_t* proc, uint32_t len)
{
    if (len % VMM_PAGE_SIZE) {
        len += VMM_PAGE_SIZE - (len % VMM_PAGE_SIZE);
    }

    uint32_t zones_count = proc->zones.size;

    /* Check if we can put it at the end */
    proc_zone_t* ret = proc_new_zone(proc, KERNEL_BASE - len, len);
    if (ret) {
        return ret;
    }

    uint32_t max_end = 0;

    for (uint32_t i = 0; i < zones_count; i++) {
        proc_zone_t* zone = (proc_zone_t*)dynamic_array_get(&proc->zones, i);
        if (_proc_can_add_zone(proc, zone->start - len, len)) {
            if (max_end < zone->start) {
                max_end = zone->start;
            }
        }
    }

    if (max_end == 0) {
        return 0;
    }

    return proc_new_zone(proc, max_end - len, len);
}

proc_zone_t* proc_find_zone_no_proc(dynamic_array_t* zones, uint32_t addr)
{
    uint32_t zones_count = zones->size;

    for (uint32_t i = 0; i < zones_count; i++) {
        proc_zone_t* zone = (proc_zone_t*)dynamic_array_get(zones, i);
        if (zone->start <= addr && addr < zone->start + zone->len) {
            return zone;
        }
    }

    return 0;
}

proc_zone_t* proc_find_zone(proc_t* proc, uint32_t addr)
{
    return proc_find_zone_no_proc(&proc->zones, addr);
}

int proc_delete_zone_no_proc(dynamic_array_t* zones, proc_zone_t* givzone)
{
    uint32_t zones_count = zones->size;

    for (uint32_t i = 0; i < zones_count; i++) {
        proc_zone_t* zone = (proc_zone_t*)dynamic_array_get(zones, i);
        if (givzone == zone) {
            _proc_swap_zones(zone, dynamic_array_get(zones, zones_count - 1));
            dynamic_array_pop(zones);
            return 0;
        }
    }

    return -EALREADY;
}

int proc_delete_zone(proc_t* proc, proc_zone_t* givzone)
{
    return proc_delete_zone_no_proc(&proc->zones, givzone);
}