//
// Created by ujlm on 11/2/23.
//
#include "lattice_works.h"
#include "Lattice.h"

#ifndef TAGFI_LATTICE_RSPS_H
#define TAGFI_LATTICE_RSPS_H


RspFunc *rsp_act(RspMap rspMap, InfoFrame **inf_frm, RspFunc (*funarr));

void init_rsptbl(int cnfg_fd, Resp_Tbl **rsp_tbl, InfoFrame **inf_frm, DChains *dchns, Lattice *hltc);

LattType dtrm_rsp(LSession_PTP sts_frm,
                  InfoFrame** inf_frm,
                  LattType);

typedef unsigned int (*InfoFunc[INFARRLEN])(unsigned char **buf, LattType lattItm, LattStruct lattStruct);


unsigned int respond(Resp_Tbl *rsp_tbl,
                     LSession_PTP session,
                     InfoFrame **inf_frm,
                     DChains *dchns,
                     Lattice *hltc,
                     uchar_arr rsp_buf);


void destroy_metastructures(LState latticestate);
/** Response actions */

unsigned int  rsp_sts(LSession_PTP sts_frm, InfoFrame **inf_frm, Lattice* hltc, unsigned char **buf);
unsigned int  rsp_nfo(LSession_PTP sts_frm, InfoFrame **inf_frm, Lattice* hltc, unsigned char **buf);
unsigned int  rsp_err(LSession_PTP sts_frm, InfoFrame **inf_frm, Lattice* hltc, unsigned char **buf);
unsigned int  rsp_und(LSession_PTP sts_frm, InfoFrame **inf_frm, Lattice* hltc, unsigned  char**buf);

unsigned int  rsp_gond(LSession_PTP sts_frm, InfoFrame** inf_frm, Lattice* hltc, unsigned char **buf);
unsigned int  rsp_gohd(LSession_PTP sts_frm, InfoFrame** inf_frm, Lattice* hltc, unsigned char **buf);
unsigned int  rsp_dsch(LSession_PTP sts_frm, InfoFrame** inf_frm, Lattice* hltc, unsigned char **buf);
unsigned int  rsp_vvvv(LSession_PTP sts_frm, InfoFrame** inf_frm, Lattice* hltc, unsigned char **buf);
unsigned int  rsp_diid(LSession_PTP sts_frm, InfoFrame** inf_frm, Lattice* hltc, unsigned char **buf);
unsigned int  rsp_jjjj(LSession_PTP sts_frm, InfoFrame** inf_frm, Lattice* hltc, unsigned char **buf);
unsigned int  rsp_dcls(LSession_PTP sts_frm, InfoFrame** inf_frm, Lattice* hltc, unsigned char **buf);
unsigned int  rsp_dnls(LSession_PTP sts_frm, InfoFrame** inf_frm, Lattice* hltc, unsigned char **buf);

unsigned int rsp_fiid (LSession_PTP sts_frm, InfoFrame **inf_frm, Lattice* hltc, unsigned char **buf);
unsigned int rsp_frdn (LSession_PTP sts_frm, InfoFrame **inf_frm, Lattice* hltc, unsigned char **buf);
unsigned int rsp_iiii (LSession_PTP sts_frm, InfoFrame **inf_frm, Lattice* hltc, unsigned char **buf);
unsigned int rsp_fyld (LSession_PTP sts_frm, InfoFrame **inf_frm, Lattice* hltc, unsigned char **buf);


#endif //TAGFI_LATTICE_RSPS_H
