#ifndef __TYPES_H__
#define __TYPES_H__


typedef unsigned char U8;
typedef unsigned int U16;
typedef struct Cmd23 { U8 byte[3]; } Cmd23;    //23 bit data type
typedef struct Synch { U8 byte[9]; } Synch;    //nine 6-bit cells
typedef struct UCmd28 { U8 byte[4]; } UCmd28;  //28 bit data type


#pragma pack(1)


#endif
