#ifndef HID_KEYCODES_H
#define HID_KEYCODES_H

#include <Arduino.h>

// Modifier masks
#define KEY_MOD_LCTRL  0x01
#define KEY_MOD_LSHIFT 0x02
#define KEY_MOD_LALT   0x04
#define KEY_MOD_LMETA  0x08
#define KEY_MOD_RCTRL  0x10
#define KEY_MOD_RSHIFT 0x20
#define KEY_MOD_RALT   0x40
#define KEY_MOD_RMETA  0x80

// Keycodes
#define KEY_A 0x04
#define KEY_B 0x05
#define KEY_C 0x06
#define KEY_D 0x07
#define KEY_E 0x08
#define KEY_F 0x09
#define KEY_G 0x0A
#define KEY_H 0x0B
#define KEY_I 0x0C
#define KEY_J 0x0D
#define KEY_K 0x0E
#define KEY_L 0x0F
#define KEY_M 0x10
#define KEY_N 0x11
#define KEY_O 0x12
#define KEY_P 0x13
#define KEY_Q 0x14
#define KEY_R 0x15
#define KEY_S 0x16
#define KEY_T 0x17
#define KEY_U 0x18
#define KEY_V 0x19
#define KEY_W 0x1A
#define KEY_X 0x1B
#define KEY_Y 0x1C
#define KEY_Z 0x1D

#define KEY_1 0x1E
#define KEY_2 0x1F
#define KEY_3 0x20
#define KEY_4 0x21
#define KEY_5 0x22
#define KEY_6 0x23
#define KEY_7 0x24
#define KEY_8 0x25
#define KEY_9 0x26
#define KEY_0 0x27

#define KEY_ENTER      0x28
#define KEY_ESC        0x29
#define KEY_BACKSPACE  0x2A
#define KEY_TAB        0x2B
#define KEY_SPACE      0x2C
#define KEY_MINUS      0x2D
#define KEY_EQUAL      0x2E
#define KEY_LEFTBRACE  0x2F
#define KEY_RIGHTBRACE 0x30
#define KEY_BACKSLASH  0x31
#define KEY_SEMICOLON  0x33
#define KEY_QUOTE      0x34
#define KEY_GRAVE      0x35
#define KEY_COMMA      0x36
#define KEY_DOT        0x37
#define KEY_SLASH      0x38
#define KEY_CAPSLOCK   0x39

#define KEY_F1  0x3A
#define KEY_F2  0x3B
#define KEY_F3  0x3C
#define KEY_F4  0x3D
#define KEY_F5  0x3E
#define KEY_F6  0x3F
#define KEY_F7  0x40
#define KEY_F8  0x41
#define KEY_F9  0x42
#define KEY_F10 0x43
#define KEY_F11 0x44
#define KEY_F12 0x45

#define KEY_RIGHT 0x4F
#define KEY_LEFT  0x50
#define KEY_DOWN  0x51
#define KEY_UP    0x52
#define KEY_DELETE 0x4C

struct AsciiKey {
    uint8_t keycode;
    uint8_t modifier;
};

inline AsciiKey getAsciiKey(char c) {
    AsciiKey ak = {0, 0};
    if (c >= 'a' && c <= 'z') {
        ak.keycode = KEY_A + (c - 'a');
    } else if (c >= 'A' && c <= 'Z') {
        ak.keycode = KEY_A + (c - 'A');
        ak.modifier = KEY_MOD_LSHIFT;
    } else if (c >= '1' && c <= '9') {
        ak.keycode = KEY_1 + (c - '1');
    } else if (c == '0') {
        ak.keycode = KEY_0;
    } else if (c == ' ') {
        ak.keycode = KEY_SPACE;
    } else if (c == '\n') {
        ak.keycode = KEY_ENTER;
    } else if (c == '\t') {
        ak.keycode = KEY_TAB;
    } else {
        switch (c) {
            case '!': ak.keycode = KEY_1; ak.modifier = KEY_MOD_LSHIFT; break;
            case '@': ak.keycode = KEY_2; ak.modifier = KEY_MOD_LSHIFT; break;
            case '#': ak.keycode = KEY_3; ak.modifier = KEY_MOD_LSHIFT; break;
            case '$': ak.keycode = KEY_4; ak.modifier = KEY_MOD_LSHIFT; break;
            case '%': ak.keycode = KEY_5; ak.modifier = KEY_MOD_LSHIFT; break;
            case '^': ak.keycode = KEY_6; ak.modifier = KEY_MOD_LSHIFT; break;
            case '&': ak.keycode = KEY_7; ak.modifier = KEY_MOD_LSHIFT; break;
            case '*': ak.keycode = KEY_8; ak.modifier = KEY_MOD_LSHIFT; break;
            case '(': ak.keycode = KEY_9; ak.modifier = KEY_MOD_LSHIFT; break;
            case ')': ak.keycode = KEY_0; ak.modifier = KEY_MOD_LSHIFT; break;
            case '-': ak.keycode = KEY_MINUS; break;
            case '_': ak.keycode = KEY_MINUS; ak.modifier = KEY_MOD_LSHIFT; break;
            case '=': ak.keycode = KEY_EQUAL; break;
            case '+': ak.keycode = KEY_EQUAL; ak.modifier = KEY_MOD_LSHIFT; break;
            case '[': ak.keycode = KEY_LEFTBRACE; break;
            case '{': ak.keycode = KEY_LEFTBRACE; ak.modifier = KEY_MOD_LSHIFT; break;
            case ']': ak.keycode = KEY_RIGHTBRACE; break;
            case '}': ak.keycode = KEY_RIGHTBRACE; ak.modifier = KEY_MOD_LSHIFT; break;
            case '\\': ak.keycode = KEY_BACKSLASH; break;
            case '|': ak.keycode = KEY_BACKSLASH; ak.modifier = KEY_MOD_LSHIFT; break;
            case ';': ak.keycode = KEY_SEMICOLON; break;
            case ':': ak.keycode = KEY_SEMICOLON; ak.modifier = KEY_MOD_LSHIFT; break;
            case '\'': ak.keycode = KEY_QUOTE; break;
            case '"': ak.keycode = KEY_QUOTE; ak.modifier = KEY_MOD_LSHIFT; break;
            case '`': ak.keycode = KEY_GRAVE; break;
            case '~': ak.keycode = KEY_GRAVE; ak.modifier = KEY_MOD_LSHIFT; break;
            case ',': ak.keycode = KEY_COMMA; break;
            case '<': ak.keycode = KEY_COMMA; ak.modifier = KEY_MOD_LSHIFT; break;
            case '.': ak.keycode = KEY_DOT; break;
            case '>': ak.keycode = KEY_DOT; ak.modifier = KEY_MOD_LSHIFT; break;
            case '/': ak.keycode = KEY_SLASH; break;
            case '?': ak.keycode = KEY_SLASH; ak.modifier = KEY_MOD_LSHIFT; break;
            default: break;
        }
    }
    return ak;
}

#endif // HID_KEYCODES_H
