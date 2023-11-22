#ifndef TAGFI_FI_CONSTS_H
#define TAGFI_FI_CONSTS_H

#include "fi_lattice.h"

typedef union LattType{
    LattErr err;
    LattReply rpl;
    LattAct act;
    LattObj obj;
    LattStts sts;
    LattFlag flg;
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
typedef InfoFrame** Info_F;
typedef StatFrame** Status_F;

extern const unsigned int UINT_SZ;
extern const unsigned int UCHAR_SZ;
extern const unsigned int ULONG_SZ;
extern const unsigned int LATTTYP_SZ;
extern const unsigned int FLAG_SZ;

extern const unsigned int ARR_POS;
extern const unsigned int ARRSZ_POS;
extern const unsigned int ITEM_POS;

extern const LattType INT_SEP;
extern const LattType CHAR_SEP;
extern const LattType LONG_SEP;

#endif