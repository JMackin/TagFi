//
// Created by ujlm on 10/6/23.
//
#ifndef TAGFI_FI_LATTICE_H
#define TAGFI_FI_LATTICE_H

#define UISiZ 4
#define UCSiZ 1
#define ltyp_s 4
#define rspsz_b 8
#define arr_b 12


#include "lattice_signals.h"
#include "lattice_works.h"
typedef union LattType{
    LattErr err;
    LattReply rpl;
    LattAct act;
    LattObj obj;
    LattStts sts;
    LttFlg flg;
    int ni;
    unsigned int nui;
    unsigned char nuc;
}LattType;


//const unsigned long UISiZ = sizeof(unsigned int);
//const unsigned long UCSiZ = sizeof(unsigned char);
//const unsigned long LTYPsz = sizeof(LattType) + sizeof(unsigned int);
//const unsigned long ltyp_s = sizeof(LattType);
//const unsigned int  rspszb = sizeof(unsigned int)+sizeof(unsigned int)+sizeof(LattType);
//const unsigned int  rspsz_b = sizeof(LattType) + sizeof(LattType);
//const unsigned int  arr_b = sizeof(LattType) + sizeof(LattType) + sizeof(unsigned int);


void summon_lattice();

#endif //TAGFI_FI_LATTICE_H
