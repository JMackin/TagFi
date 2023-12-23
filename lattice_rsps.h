//
// Created by ujlm on 11/2/23.
//
#include "lattice_works.h"
#include "Lattice.h"

#ifndef TAGFI_LATTICE_RSPS_H
#define TAGFI_LATTICE_RSPS_H


RspFunc* rsp_act(
        RspMap rspMap,
        StatFrame** sts_frm,
        InfoFrame** inf_frm,
        RspFunc* (funarr));

void init_rsptbl(int cnfg_fd,
                 Resp_Tbl **rsp_tbl,
                 StatFrame **sts_frm,
                 InfoFrame **inf_frm,
                 DChains *dchns,
                 Lattice *hltc);

LattType dtrm_rsp(StatFrame** sts_frm,
                  InfoFrame** inf_frm,
                  LattType);

typedef unsigned int (*InfoFunc[INFARRLEN])(unsigned char **buf, LattType lattItm, LattStruct lattStruct);


unsigned int respond(Resp_Tbl *rsp_tbl,
                     StatFrame **sts_frm,
                     InfoFrame **inf_frm,
                     DChains *dchns,
                     Lattice *hltc,
                     unsigned char *resp_buf);


void destroy_metastructures(LttcState latticestate);
/** Response actions */

unsigned int  rsp_sts(StatFrame **sts_frm, InfoFrame **inf_frm, Lattice* hltc, unsigned char **buf);
unsigned int  rsp_nfo(StatFrame **sts_frm, InfoFrame **inf_frm, Lattice* hltc, unsigned char **buf);
unsigned int  rsp_err(StatFrame **sts_frm, InfoFrame **inf_frm, Lattice* hltc, unsigned char **buf);
unsigned int  rsp_und(StatFrame** sts_frm, InfoFrame **inf_frm, Lattice* hltc, unsigned  char**buf);

unsigned int  rsp_gond(StatFrame** sts_frm, InfoFrame** inf_frm, Lattice* hltc, unsigned char **buf);
unsigned int  rsp_gohd(StatFrame** sts_frm, InfoFrame** inf_frm, Lattice* hltc, unsigned char **buf);
unsigned int  rsp_dsch(StatFrame** sts_frm, InfoFrame** inf_frm, Lattice* hltc, unsigned char **buf);
unsigned int  rsp_vvvv(StatFrame** sts_frm, InfoFrame** inf_frm, Lattice* hltc, unsigned char **buf);
unsigned int  rsp_diid(StatFrame** sts_frm, InfoFrame** inf_frm, Lattice* hltc, unsigned char **buf);
unsigned int  rsp_jjjj(StatFrame** sts_frm, InfoFrame** inf_frm, Lattice* hltc, unsigned char **buf);
unsigned int  rsp_dcls(StatFrame** sts_frm, InfoFrame** inf_frm, Lattice* hltc, unsigned char **buf);
unsigned int  rsp_dnls(StatFrame** sts_frm, InfoFrame** inf_frm, Lattice* hltc, unsigned char **buf);

unsigned int rsp_fiid(StatFrame **sts_frm, InfoFrame **inf_frm, Lattice* hltc, unsigned char **buf);
unsigned int rsp_frdn(StatFrame **sts_frm, InfoFrame **inf_frm, Lattice* hltc, unsigned char **buf);
unsigned int rsp_iiii(StatFrame **sts_frm, InfoFrame **inf_frm, Lattice* hltc, unsigned char **buf);
unsigned int rsp_fyld(StatFrame **sts_frm, InfoFrame **inf_frm, Lattice* hltc, unsigned char **buf);


#endif //TAGFI_LATTICE_RSPS_H
