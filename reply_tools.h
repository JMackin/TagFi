//
// Created by ujlm on 11/22/23.
//

#ifndef TAGFI_REPLY_TOOLS_H
#define TAGFI_REPLY_TOOLS_H
#include <string.h>

#include "consts.h"
#include <stdint.h>

void rsparr_out(unsigned char ** buf, unsigned int arrlen);
unsigned char* rsparr_pos(buff_arr buf);
void rsparr_len_set(unsigned int sz, unsigned char ** buf);
RspFlag rsplead_addflg(RspFlag flags, unsigned char ** buf);
unsigned int prpbuf(unsigned char ** buf, LattType lattItm);
unsigned int endbuf(unsigned char ** buf,LattType lattItm, unsigned int rsplen);

uint check_end_flg(uchar_arr fullreqbuf, InfoFrame** infoFrame);

unsigned int rsparr_len_inc(void* itm, unsigned int cnt, unsigned int mlti);
unsigned int rsparr_addsep(unsigned int offset, LattType sep, unsigned char ** buf);
unsigned int rsp_add_arrobj(LattObj lobj, unsigned char ** buf);
unsigned int rsp_add_replyitm(unsigned char ** buf, LattReply replyobj);
unsigned int rsparr_add_lt(LattType lt, unsigned char ** buf, unsigned int offset);
unsigned int rsparr_add_lng(unsigned int offset, LattLong lli, unsigned char ** buf);
unsigned int rsparr_add_lnglng(unsigned int offset, unsigned long long lli, unsigned char ** buf);
unsigned int rsparr_add_msg(unsigned char ** buf, char* msg, unsigned int len, unsigned int offst);
unsigned int rsparr_add_chrstr(unsigned char ** buf, unsigned char * msg, unsigned int len, unsigned int offst);
unsigned int rsparr_add_travelpath(TravelPath* travelpath, buff_arr buf, uint offst);

LattObj pull_arrObj(Info_F inf_frm);
LattReply pull_replyitm(buff_arr buf);
uint pull_arrsz(buff_arr buf);
uint nxt_arrelem_type(buff_arr buf, uint offset);
LattLong pull_objid(InfoFrame** infoFrame, LattLong itmId, size_t long_sz);
unsigned char* pull_arr(buff_arr buf);
unsigned int pull_arrtyp(LattFlag lattflg);

char* convertLattErr(const LattErr* latterr_itr);
char* convertLattSts(const LattStts * lattsts_itr);


#endif //TAGFI_REPLY_TOOLS_H
