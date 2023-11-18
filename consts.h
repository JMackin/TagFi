#ifndef TAGFI_FI_CONSTS_H
#define TAGFI_FI_CONSTS_H

#include "fi_lattice.h"

typedef union LattType{
    LattErr err;
    LattReply rpl;
    LattAct act;
    LattObj obj;
    LattStts sts;
    LttFlg flg;
    int n_int;
    unsigned int n_uint;
    unsigned char n_uchar;
}LattType;

typedef union LattLong{
    unsigned long l_ulong;
    unsigned long * l_ulong_ptr;
    unsigned long long int l_ulonglong;
    unsigned long long int* l_ulonglong_ptr;
    unsigned long* l_doub_ulong_ptr[2];
    unsigned char l_uchar_16[16];
}LattLong;

typedef unsigned int uint;
typedef unsigned char uchar;
typedef unsigned long ulong;
typedef const unsigned int cnst_uint;
typedef unsigned char** buff_arr;
typedef unsigned char* uchar_arr;
typedef unsigned int* ptr_uint;

extern const unsigned int uint_sz;
extern const unsigned int uchar_sz;
extern const unsigned int ulong_sz;
extern const unsigned int lattyp_sz;
extern const LattType isep;
extern const LattType csep;

#endif