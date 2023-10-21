//
// Created by ujlm on 10/19/23.
//

#include "latticecomm.h"
#include "lattice_cmds.h"
#include <stdio.h>

void setErr(StatFrame** sts_frm, LattErr ltcerr, unsigned int modr){
    (*sts_frm)->status = STERR;
    (*sts_frm)->err_code = ltcerr;
    if (modr){
        (*sts_frm)->modr = modr;
    }
}

void setSts(StatFrame** sts_frm, LattStts ltcst, unsigned int modr){
    (*sts_frm)->status = ltcst;
    if (modr){
        (*sts_frm)->modr = modr;
    }
    if (modr == SELFRESET) {
        (*sts_frm)->modr = 0;
    }
}

void setAct(StatFrame** sts_frm, LattAct lttact, LattStts ltsts, unsigned int modr)
{
    (*sts_frm)->act_id = lttact;
    if(ltsts != NOTHN){
        (*sts_frm)->status = ltsts;
    }
    if (modr) {
        (*sts_frm)->modr = modr;
    }
    if (modr == SELFRESET) {
        (*sts_frm)->modr = 0;
    }
}

void setMdr(StatFrame** sts_frm, unsigned int modr){
    (*sts_frm)->modr = modr ? modr : ++((*sts_frm)->modr);
    if (modr == SELFRESET) {
        (*sts_frm)->modr = 0;
    }
}

void stsReset(StatFrame** sts_frm)
{
    (*sts_frm)->status=LISTN;
    (*sts_frm)->act_id=ZZZZ;
    (*sts_frm)->err_code=IMFINE;
    (*sts_frm)->resp_id=SILNT;
    (*sts_frm)->modr=0;
}

void stsOut(StatFrame** sts_frm)
{
    printf("Status:%d\n",(*sts_frm)->status);
}

void serrOut(StatFrame** sts_frm)
{
    fprintf(stderr,"[ Error Code: %d ]", (*sts_frm)->err_code);
    fprintf(stderr,"< %d >\n", (*sts_frm)->modr);
    if ((*sts_frm)->act_id == GBYE){
        fprintf(stderr,"Shutting down\n");
        (*sts_frm)->status = SHTDN;
    }
}
