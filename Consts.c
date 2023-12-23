#include "Consts.h"

// [ lead -> item -> arrsz -> arr -> DONE ]

const unsigned int UINT_SZ = sizeof(unsigned int);
const unsigned int UCHAR_SZ = sizeof(unsigned char);
const unsigned int ULONG_SZ = sizeof(unsigned long);
const unsigned int VULONG_SZ = 2*sizeof(unsigned long);
const unsigned int LATTTYP_SZ = sizeof(LattType);
const unsigned int FLAG_SZ = sizeof(LattFlag);

const unsigned int ARR_POS = sizeof(LattFlag) + sizeof(LattType) + sizeof(unsigned int);
const unsigned int ARRSZ_POS = sizeof(LattFlag) + sizeof(LattType);
const unsigned int ITEM_POS = sizeof(LattFlag);

const LattType INT_SEP = (LattType) 0xdbdbdbdb;
const LattType CHAR_SEP = (LattType) 0xbddbdbbd;
const LattType LONG_SEP = (LattType) 0xddddbbbb;

const char potntl_args[POTENTL_ARG_CNT][ARGS_MAX_LEN] = {{'t','e','s','t'},
                                                         {0,0,0,0,0,0,0,0},
                                                         {0,0,0,0,0,0,0,0},
                                                         {0,0,0,0,0,0,0,0},
                                                         {0,0,0,0,0,0,0,0},
                                                         {0,0,0,0,0,0,0,0},
                                                         {0,0,0,0,0,0,0,0},
                                                         {'i','n','i','t'}};

const LattErr latt_err_arr[LERR_CNT] = {IMFINE, MALREQ, MISSNG, UNKNWN, NOINFO, BADCON, BADSOK, MISMAP, STFAIL, FIFAIL, MISCLC,
                                         BADHSH, SODIUM, FULLUP, ADFAIL, COLISN, ILMMOP, EPOLLE, BADCNF,MISSPK,MISVEN};

const char latt_err_strs[LERR_CNT][LERR_CHARCNT] = {"IMFINE", "MALREQ", "MISSNG", "UNKNWN", "NOINFO", "BADCON", "BADSOK", "MISMAP", "STFAIL", "FIFAIL", "MISCLC",
                                         "BADHSH", "SODIUM", "FULLUP", "ADFAIL", "COLISN", "ILMMOP", "EPOLLE", "BADCNF", "MISSPK", "MISVEN"};

const LattStts latt_sts_arr[LSTS_CNT] = {NOTHN, LISTN, CNNIN, REQST, RCVCM, RESPN, UPDAT, TRVLD, SHTDN, RESET, STERR, SLEEP};
const char latt_stts_strs[LSTS_CNT][LSTS_CHARCNT] = {"NOTHN","LISTN","CNNIN","REQST","RCVCM","RESPN","UPDAT","TRVLD","SHTDN","RESET","STERR","SLEEP"};

// Index: 1 = char, 2 = int, 3 = long
LattType sep_tup[3] = {0xbddbdbbd,0xdbdbdbdb,0xddddbbbb};

const int BUF_LEN = 256;
const int ARRBUF_LEN = 128;