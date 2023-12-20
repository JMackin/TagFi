#ifndef TAGFI_FI_CONSTS_H
#define TAGFI_FI_CONSTS_H

#include "lattice_signals.h"

#define LERR_CNT 21
#define LERR_CHARCNT 8
#define LERR_LASTELEM 1048576
#define ERRBUFLEN 1024

#define LSTS_CNT 13
#define LSTS_CHARCNT 6
#define LSTS_LASTELEM 1024

#define POTENTL_ARG_CNT 8
#define ARGS_MAX_LEN 8


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



extern const unsigned int UINT_SZ;
extern const unsigned int UCHAR_SZ;
extern const unsigned int ULONG_SZ;
extern const unsigned int VULONG_SZ;

extern const unsigned int LATTTYP_SZ;
extern const unsigned int FLAG_SZ;

extern const unsigned int ARR_POS;
extern const unsigned int ARRSZ_POS;
extern const unsigned int ITEM_POS;

extern const LattType INT_SEP;
extern const LattType CHAR_SEP;
extern const LattType LONG_SEP;

extern const LattErr latt_err_arr[LERR_CNT];
extern const char latt_err_strs[LERR_CNT][LERR_CHARCNT];
extern const LattStts latt_sts_arr[LSTS_CNT];
extern const char latt_stts_strs[LSTS_CNT][LSTS_CHARCNT];

extern const char potntl_args[POTENTL_ARG_CNT][ARGS_MAX_LEN];

#endif