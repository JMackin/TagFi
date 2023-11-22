//
// Created by ujlm on 11/22/23.
//

#ifndef TAGFI_REPLY_TOOLS_H
#define TAGFI_REPLY_TOOLS_H

#include "consts.h"
#include <stdint.h>

inline unsigned int rsparr_addsep(unsigned int offset, LattType sep, unsigned char ** buf);

unsigned char* rsparr_pos(unsigned char ** buf);
unsigned int prpbuf(unsigned char ** buf, LattType lattItm);
unsigned int endbuf(unsigned char ** buf,LattType lattItm, unsigned int rsplen);
unsigned int rsparr_len_inc(void* itm, unsigned int cnt, unsigned int mlti);
void rsparr_len_set(unsigned int sz, unsigned char ** buf);
void rsparr_out(unsigned char ** buf, unsigned int arrlen);
unsigned int rsp_add_arrobj(LattObj lobj, unsigned char ** buf);
unsigned int rsp_add_replyitm(unsigned char ** buf, LattReply replyobj);
unsigned int rsparr_add_lt(LattType lt, unsigned char ** buf, unsigned int offset);
unsigned int rsparr_add_lng(unsigned int offset, LattLong lli, unsigned char ** buf);
unsigned int rsparr_add_lnglng(unsigned int offset, unsigned long long lli, unsigned char ** buf);
unsigned int rsparr_add_msg(unsigned char ** buf, char* msg, unsigned int len, unsigned int offst);
unsigned int rsparr_add_chrstr(unsigned char ** buf, unsigned char * msg, unsigned int len, unsigned int offst);
LattLong pull_objid(InfoFrame** infoFrame, LattLong itmId, size_t long_sz);
unsigned int pull_arrtyp(LattFlag lattflg);
RspFlag rsplead_addflg(RspFlag flags, unsigned char ** buf);


#endif //TAGFI_REPLY_TOOLS_H
