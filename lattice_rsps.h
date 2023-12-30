//
// Created by ujlm on 11/2/23.
//

#ifndef TAGFI_LATTICE_RSPS_H
#define TAGFI_LATTICE_RSPS_H
#include "lattice_works.h"


RspFunc *rsp_act(RspMap rspMap, InfoFrame **inf_frm, RspFunc (*funarr));

void init_rsptbl(int cnfg_fd, Resp_Tbl **rsp_tbl, InfoFrame **inf_frm, DChains *dchns, Lattice *hltc);

LattType dtrm_rsp(SSession_PTP sts_frm,
                  InfoFrame **inf_frm,
                  LattType);

typedef unsigned int (*InfoFunc[INFARRLEN])(unsigned char **buf, LattType lattItm, LattStruct lattStruct);


unsigned int respond(Resp_Tbl *rsp_tbl,
                     SSession_PTP session,
                     InfoFrame **inf_frm,
                     DChains *dchns,
                     Lattice *hltc,
                     uchar_arr rsp_buf);


void destroy_metastructures(LState latticestate);

/** Response actions */

unsigned int rsp_sts(SSession_PTP sts_frm, InfoFrame **inf_frm, Lattice *hltc, unsigned char **buf);

unsigned int rsp_nfo(SSession_PTP sts_frm, InfoFrame **inf_frm, Lattice *hltc, unsigned char **buf);

unsigned int rsp_err(SSession_PTP sts_frm, InfoFrame **inf_frm, Lattice *hltc, unsigned char **buf);

unsigned int rsp_und(SSession_PTP sts_frm, InfoFrame **inf_frm, Lattice *hltc, unsigned char **buf);

unsigned int rsp_gond(SSession_PTP sts_frm, InfoFrame **inf_frm, Lattice *hltc, unsigned char **buf);

unsigned int rsp_gohd(SSession_PTP sts_frm, InfoFrame **inf_frm, Lattice *hltc, unsigned char **buf);

unsigned int rsp_dsch(SSession_PTP sts_frm, InfoFrame **inf_frm, Lattice *hltc, unsigned char **buf);

unsigned int rsp_vvvv(SSession_PTP sts_frm, InfoFrame **inf_frm, Lattice *hltc, unsigned char **buf);

unsigned int rsp_diid(SSession_PTP sts_frm, InfoFrame **inf_frm, Lattice *hltc, unsigned char **buf);

unsigned int rsp_jjjj(SSession_PTP sts_frm, InfoFrame **inf_frm, Lattice *hltc, unsigned char **buf);

unsigned int rsp_dcls(SSession_PTP sts_frm, InfoFrame **inf_frm, Lattice *hltc, unsigned char **buf);

unsigned int rsp_dnls(SSession_PTP sts_frm, InfoFrame **inf_frm, Lattice *hltc, unsigned char **buf);

unsigned int rsp_fiid(SSession_PTP sts_frm, InfoFrame **inf_frm, Lattice *hltc, unsigned char **buf);

unsigned int rsp_frdn(SSession_PTP sts_frm, InfoFrame **inf_frm, Lattice *hltc, unsigned char **buf);

unsigned int rsp_iiii(SSession_PTP sts_frm, InfoFrame **inf_frm, Lattice *hltc, unsigned char **buf);

unsigned int rsp_fyld(SSession_PTP sts_frm, InfoFrame **inf_frm, Lattice *hltc, unsigned char **buf);


#endif //TAGFI_LATTICE_RSPS_H
