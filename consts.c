#include "consts.h"

// [ lead -> item -> arrsz -> arr -> DONE ]

const unsigned int UINT_SZ = sizeof(unsigned int);
const unsigned int UCHAR_SZ = sizeof(unsigned char);
const unsigned int ULONG_SZ = sizeof(unsigned long);
const unsigned int LATTTYP_SZ = sizeof(LattType);
const unsigned int FLAG_SZ = sizeof(LattFlag);

const unsigned int ARR_POS = sizeof(LattFlag) + sizeof(LattType) + sizeof(unsigned int);
const unsigned int ARRSZ_POS = sizeof(LattFlag) + sizeof(LattType);
const unsigned int ITEM_POS = sizeof(LattFlag);

const LattType INT_SEP = (LattType) 0xdbdbdbdb;
const LattType CHAR_SEP = (LattType) 0xbddbdbbd;
const LattType LONG_SEP = (LattType) 0xddddbbbb;