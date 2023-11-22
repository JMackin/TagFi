//
// Created by ujlm on 11/22/23.
//

#include "reply_tools.h"
#include <string.h>


inline unsigned char* rsparr_pos(buff_arr buf);
unsigned char *rsparr_pos(buff_arr buf) {return *buf + ARR_POS;}

uint prpbuf(buff_arr buf,LattType lattItm) {
    lattItm.flg.uflg_flg =  STRT;
    memcpy(*buf, &lattItm, LATTTYP_SZ);
    return LATTTYP_SZ;
}

uint endbuf(buff_arr buf,LattType lattItm, uint rsplen) {
    lattItm.flg.uflg_flg =  DONE;
    memcpy((*buf)+rsplen, &lattItm, FLAG_SZ);
    return FLAG_SZ;
}

uint rsparr_len_inc(void* itm, uint cnt, uint mlti)
{return mlti ? ((mlti*sizeof(*itm))+cnt) : sizeof(*itm)+cnt;}

void rsparr_len_set(uint sz, buff_arr buf){memcpy((*buf+ARRSZ_POS), &sz, UINT_SZ);}

void rsparr_out(buff_arr buf, uint arrlen){
    uint n = 0;
    while(putchar_unlocked(*(*buf + n))) {
        if (n == arrlen) {
            break;
        }
        ++n;
    }
}

// sep = INT_SEP or CHAR_SEP
uint rsparr_addsep(uint offset, LattType sep, buff_arr buf)
{memcpy(rsparr_pos(buf)+offset, &sep, LATTTYP_SZ);return LATTTYP_SZ;}

uint rsp_add_arrobj(LattObj lobj, buff_arr buf)
{memcpy(rsparr_pos(buf), &lobj, LATTTYP_SZ);return LATTTYP_SZ;}

uint rsp_add_replyitm(buff_arr buf, LattReply replyobj)
{memcpy((*buf+ITEM_POS),&replyobj,LATTTYP_SZ);return LATTTYP_SZ;}

uint rsparr_add_lt(LattType lt, buff_arr buf, uint offset)
{memcpy(rsparr_pos(buf)+offset, &lt, LATTTYP_SZ);return LATTTYP_SZ;}

uint rsparr_add_lng(uint offset, LattLong lli, buff_arr buf){
    {memcpy(rsparr_pos(buf)+offset, &lli, ULONG_SZ);
        return ULONG_SZ;}
}

uint rsparr_add_lnglng(uint offset, unsigned long long lli, buff_arr buf){
    {memcpy(rsparr_pos(buf)+offset,&lli,sizeof(unsigned long long));
        return sizeof(unsigned long long int);}
}

uint rsparr_add_msg(buff_arr buf, char* msg, uint len, uint offst)
{
    char* newmsg = NULL;
    if (*(msg+len) != '\0'){
        newmsg = (char*) calloc(len+1,sizeof(char));
        memcpy(newmsg,msg,len);
        memset(newmsg+len,'\0',1);
        msg = newmsg;
        ++len;
    }
    memcpy(rsparr_pos(buf)+offst,msg,len+1);
    if (newmsg != NULL){
        free(newmsg);
    }
    return len;
}

uint rsparr_add_chrstr(buff_arr buf, uchar_arr msg, uint len, uint offst)
{memcpy(rsparr_pos(buf) + offst + LATTTYP_SZ, msg, len);return len;}

LattLong pull_objid(InfoFrame** infoFrame, LattLong itmId, size_t long_sz){

    if (long_sz == 8){
        memcpy((&itmId.l_ulong),(*infoFrame)->arr+8,(ULONG_SZ));
    }
    else if (long_sz == 16){
        memcpy(itmId.l_uchar_16,&(*infoFrame)->arr+(rspsz_b),16);
    }
    else{
        itmId.l_ulong = 1;
    }

    return itmId;
}

uint pull_arrtyp(LattFlag lattflg){
    return (lattflg.req & 65535)>>15;
}

RspFlag rsplead_addflg(RspFlag flags, buff_arr buf){
    RspFlag o_flgs;
    memcpy(&o_flgs, *buf, FLAG_SZ);
    o_flgs |= flags;
    memcpy(*buf, &o_flgs, FLAG_SZ);
    return o_flgs;
}

