#ifndef CODES_H
#define CODES_H

#include "usb_hid_keys.h"

#define KEY_TABLE_SIZE           54 
#define ALT_KEY_TABLE_SIZE        7

/* 
 * Tables to map code sequences to keys
 *
 * Table Elements Array: 
 * [1] key sequence in reverse order, the first bit is a 1 to mark the start of the sequence
 * [2] key code to use
 * [3] if the key uses shift
 */

// table of standard keys
uint16_t key_table[KEY_TABLE_SIZE][3] =
{
 {0b110,        KEY_A,            false},     // a
 {0b10001,      KEY_B,            false},     // b
 {0b10101,      KEY_C,            false},     // c
 {0b1001,       KEY_D,            false},     // d
 {0b10,         KEY_E,            false},     // e
 {0b10100,      KEY_F,            false},     // f
 {0b1011,       KEY_G,            false},     // g
 {0b10000,      KEY_H,            false},     // h
 {0b100,        KEY_I,            false},     // i
 {0b11110,      KEY_J,            false},     // j
 {0b1101,       KEY_K,            false},     // k
 {0b10010,      KEY_L,            false},     // l
 {0b111,        KEY_M,            false},     // m
 {0b101,        KEY_N,            false},     // n
 {0b1111,       KEY_O,            false},     // o
 {0b10110,      KEY_P,            false},     // p
 {0b11011,      KEY_Q,            false},     // q
 {0b1010,       KEY_R,            false},     // r
 {0b1000,       KEY_S,            false},     // s
 {0b11,         KEY_T,            false},     // t
 {0b1100,       KEY_U,            false},     // u
 {0b11000,      KEY_V,            false},     // v
 {0b1110,       KEY_W,            false},     // w
 {0b11001,      KEY_X,            false},     // x
 {0b11101,      KEY_Y,            false},     // y
 {0b10011,      KEY_Z,            false},     // z
 {0b111110,     KEY_1,            false},     // 1
 {0b111100,     KEY_2,            false},     // 2
 {0b111000,     KEY_3,            false},     // 3
 {0b110000,     KEY_4,            false},     // 4
 {0b100000,     KEY_5,            false},     // 5
 {0b100001,     KEY_6,            false},     // 6
 {0b100011,     KEY_7,            false},     // 7
 {0b100111,     KEY_8,            false},     // 8
 {0b101111,     KEY_9,            false},     // 9
 {0b111111,     KEY_0,            false},     // 0
 {0b1101010,    KEY_DOT,          false},     // .
 {0b1110011,    KEY_COMMA,        false},     // ,
 {0b110101,     KEY_SEMICOLON,    false},     // ;
 {0b101001,     KEY_SLASH,        false},     // /
 {0b1011110,    KEY_APOSTROPHE,   false},     // '
 {0b1100001,    KEY_MINUS,        false},     // -
 {0b110001,     KEY_EQUAL,        false},     // =
 {0b1001100,    KEY_SLASH,        true},      // ?
 {0b1000111,    KEY_SEMICOLON,    true},      // :
 {0b1101100,    KEY_MINUS,        true},      // _
 {0b101101,     KEY_9,            true},      // (
 {0b1101101,    KEY_0,            true},      // )
 {0b1010110,    KEY_2,            true},      // @
 {0b1110101,    KEY_1,            true},      // !
 {0b100010,     KEY_7,            true},      // &
 {0b1010010,    KEY_APOSTROPHE,   true},      // "
 {0b101010,     KEY_EQUAL,        true},      // +
 {0b11001000,   KEY_4,            true}       // $
};

// table of alt keys (access by pressing both buttons together)
uint16_t alt_key_table[ALT_KEY_TABLE_SIZE][3] =
{
 {0b10,         KEY_SPACE,        false},     // space
 {0b100,        KEY_ENTER,        false},     // enter
 {0b11,         KEY_BACKSPACE,    false},     // backspace
 {0b1110,       KEY_LEFT,         false},     // move left
 {0b11110,      KEY_RIGHT,        false},     // move right
 {0b110,        KEY_MOD_LSHIFT,   false},     // shift
 {0b111,        KEY_ESC,          false}      // escape
};

#endif // CODES_H
