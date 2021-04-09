#ifndef __MK61_H__
#define __MK61_H__

#include "types.h"


const U8 SegmentData[16] PROGMEM = {
	/*Segment format: bgedafcp */
	0xBE,0x82,0xF8,0xDA,0xC6,0x5E,0x7E,0x8A,
	0xFE,0xDE,0x40,0x34,0x3C,0x2C,0x7C,0x00
};

volatile U8 Display[12] = "";
volatile U8 Column = 0;

volatile U8 KeyStatus = 0;
#define KEYS_UP    0
#define KEYS_DOWN  1
volatile U8 KeyValue = 0;
#define KEY_NONE   0
#define KEY_F      1
#define KEY_SST    2
#define KEY_BST    3
#define KEY_RTN    4
#define KEY_RS     5
#define KEY_K      6
#define KEY_RCL    7
#define KEY_STO    8
#define KEY_GTO    9
#define KEY_GSB    10
#define KEY_7      11
#define KEY_8      12
#define KEY_9      13
#define KEY_SUB    14
#define KEY_DIV    15
#define KEY_4      16
#define KEY_5      17
#define KEY_6      18
#define KEY_ADD    19
#define KEY_MUL    20
#define KEY_1      21
#define KEY_2      22
#define KEY_3      23
#define KEY_XY     24
#define KEY_ENT    25
#define KEY_0      26
#define KEY_DP     27
#define KEY_NEG    28
#define KEY_EXP    29
#define KEY_CX     30

const U8 KeyMapX[31] PROGMEM = {
   0,
   11,  7,  9,  4,  2,
   10,  8,  6,  3,  5,
    9, 10, 11,  3,  5,
    6,  7,  8,  2,  4,
    3,  4,  5,  6, 11,
    2,  7,  8,  9, 10
};
const U8 KeyMapY[31] PROGMEM = {
   0,
   9,  9,  9,  9,  9,
   9,  9,  9,  9,  9,
   1,  1,  1,  8,  8,
   1,  1,  1,  8,  8,
   1,  1,  1,  8,  8,
   1,  8,  8,  8,  8
};

volatile char SwitchValue = 0;
#define SWITCH_RAD 10
#define SWITCH_DEG 11
#define SWITCH_GRD 12

#endif
