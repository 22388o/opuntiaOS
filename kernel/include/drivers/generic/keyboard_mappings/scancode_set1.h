/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <drivers/generic/keyboard.h>

static key_t _scancode_set1[] = {
    KEY_UNKNOWN, //0
    KEY_ESCAPE, //1
    KEY_1, //2
    KEY_2, //3
    KEY_3, //4
    KEY_4, //5
    KEY_5, //6
    KEY_6, //7
    KEY_7, //8
    KEY_8, //9
    KEY_9, //0xa
    KEY_0, //0xb
    KEY_MINUS, //0xc
    KEY_EQUAL, //0xd
    KEY_BACKSPACE, //0xe
    KEY_TAB, //0xf
    KEY_Q, //0x10
    KEY_W, //0x11
    KEY_E, //0x12
    KEY_R, //0x13
    KEY_T, //0x14
    KEY_Y, //0x15
    KEY_U, //0x16
    KEY_I, //0x17
    KEY_O, //0x18
    KEY_P, //0x19
    KEY_LEFTBRACKET, //0x1a
    KEY_RIGHTBRACKET, //0x1b
    KEY_RETURN, //0x1c
    KEY_LCTRL, //0x1d
    KEY_A, //0x1e
    KEY_S, //0x1f
    KEY_D, //0x20
    KEY_F, //0x21
    KEY_G, //0x22
    KEY_H, //0x23
    KEY_J, //0x24
    KEY_K, //0x25
    KEY_L, //0x26
    KEY_SEMICOLON, //0x27
    KEY_QUOTE, //0x28
    KEY_GRAVE, //0x29
    KEY_LSHIFT, //0x2a
    KEY_BACKSLASH, //0x2b
    KEY_Z, //0x2c
    KEY_X, //0x2d
    KEY_C, //0x2e
    KEY_V, //0x2f
    KEY_B, //0x30
    KEY_N, //0x31
    KEY_M, //0x32
    KEY_COMMA, //0x33
    KEY_DOT, //0x34
    KEY_SLASH, //0x35
    KEY_RSHIFT, //0x36
    KEY_KP_ASTERISK, //0x37
    KEY_RALT, //0x38
    KEY_SPACE, //0x39
    KEY_CAPSLOCK, //0x3a
    KEY_F1, //0x3b
    KEY_F2, //0x3c
    KEY_F3, //0x3d
    KEY_F4, //0x3e
    KEY_F5, //0x3f
    KEY_F6, //0x40
    KEY_F7, //0x41
    KEY_F8, //0x42
    KEY_F9, //0x43
    KEY_F10, //0x44
    KEY_KP_NUMLOCK, //0x45
    KEY_SCROLLLOCK, //0x46
    KEY_HOME, //0x47
    KEY_KP_8, //0x48	//keypad up arrow
    KEY_PAGEUP, //0x49
    KEY_KP_2, //0x50	//keypad down arrow
    KEY_KP_3, //0x51	//keypad page down
    KEY_KP_0, //0x52	//keypad insert key
    KEY_KP_DECIMAL, //0x53	//keypad delete key
    KEY_UNKNOWN, //0x54
    KEY_UNKNOWN, //0x55
    KEY_UNKNOWN, //0x56
    KEY_F11, //0x57
    KEY_F12 //0x58
};

static key_t _scancode_set1_upper[] = {
    KEY_UNKNOWN, //0
    KEY_UNKNOWN, //1
    KEY_UNKNOWN, //2
    KEY_UNKNOWN, //3
    KEY_UNKNOWN, //4
    KEY_UNKNOWN, //5
    KEY_UNKNOWN, //6
    KEY_UNKNOWN, //7
    KEY_UNKNOWN, //8
    KEY_UNKNOWN, //9
    KEY_UNKNOWN, //0xa
    KEY_UNKNOWN, //0xb
    KEY_UNKNOWN, //0xc
    KEY_UNKNOWN, //0xd
    KEY_UNKNOWN, //0xe
    KEY_UNKNOWN, //0xf
    KEY_PREV_TRACK, //0x10
    KEY_UNKNOWN, //0x11
    KEY_UNKNOWN, //0x12
    KEY_UNKNOWN, //0x13
    KEY_UNKNOWN, //0x14
    KEY_UNKNOWN, //0x15
    KEY_UNKNOWN, //0x16
    KEY_UNKNOWN, //0x17
    KEY_UNKNOWN, //0x18
    KEY_NEXT_TRACK, //0x19
    KEY_UNKNOWN, //0x1a
    KEY_UNKNOWN, //0x1b
    KEY_KP_ENTER, //0x1c
    KEY_RCTRL, //0x1d
    KEY_UNKNOWN, //0x1e
    KEY_UNKNOWN, //0x1f
    KEY_MUTE, //0x20
    KEY_CALC, //0x21
    KEY_PLAY, //0x22
    KEY_UNKNOWN, //0x23
    KEY_STOP, //0x24
    KEY_UNKNOWN, //0x25
    KEY_UNKNOWN, //0x26
    KEY_UNKNOWN, //0x27
    KEY_UNKNOWN, //0x28
    KEY_UNKNOWN, //0x29
    KEY_UNKNOWN, //0x2a
    KEY_UNKNOWN, //0x2b
    KEY_UNKNOWN, //0x2c
    KEY_UNKNOWN, //0x2d
    KEY_VOL_DOWN, //0x2e
    KEY_UNKNOWN, //0x2f
    KEY_VOL_UP, //0x30
    KEY_UNKNOWN, //0x31
    KEY_WWW_HOME, //0x32
    KEY_UNKNOWN, //0x33
    KEY_UNKNOWN, //0x34
    KEY_NUMKEYCODES, //0x35
    KEY_UNKNOWN, //0x36
    KEY_UNKNOWN, //0x37
    KEY_RALT, //0x38
    KEY_UNKNOWN, //0x39
    KEY_UNKNOWN, //0x3a
    KEY_UNKNOWN, //0x3b
    KEY_UNKNOWN, //0x3c
    KEY_UNKNOWN, //0x3d
    KEY_UNKNOWN, //0x3e
    KEY_UNKNOWN, //0x3f
    KEY_UNKNOWN, //0x40
    KEY_UNKNOWN, //0x41
    KEY_UNKNOWN, //0x42
    KEY_UNKNOWN, //0x43
    KEY_UNKNOWN, //0x44
    KEY_UNKNOWN, //0x45
    KEY_UNKNOWN, //0x46
    KEY_HOME, //0x47
    KEY_UP, //0x48
    KEY_PAGEUP, //0x49
    KEY_UNKNOWN, //0x4a
    KEY_LEFT, //0x4b
    KEY_UNKNOWN, //0x4c
    KEY_RIGHT, //0x4d
    KEY_UNKNOWN, //0x4e
    KEY_END, //0x4f
    KEY_DOWN, //0x50
    KEY_PAGEDOWN, //0x51
    KEY_INSERT, //0x52
    KEY_DELETE //0x53
};

static inline key_t generic_keyboard_get_keycode_set1(uint32_t scancode)
{
    key_t* lookup_table[] = { _scancode_set1, _scancode_set1_upper };
    return lookup_table[(scancode >> 8) & 1][scancode & ~((uint32_t)0x100)];
}