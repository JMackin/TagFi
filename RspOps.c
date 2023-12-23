//
// Created by ujlm on 11/22/23.
//

// lead | item | arrsz | arr | DONE


#include "RspOps.h"
#include "lattice_signals.h"
#include "Consts.h"
#include "Lattice.h"
#include <string.h>

uchar_arr rsparr_pos(buff_arr buf) {return *buf + ARR_POS;}

uint pull_arrtyp(LattFlag lattflg){
    return (lattflg.req & 65535)>>15;
}

uchar_arr pull_arr(buff_arr buf){return *buf + ARR_POS;}

LattObj pull_arrObj(Info_Frame_PTP inf_frm){
    LattObj arrobj;
    memcpy(&arrobj, (*inf_frm)->arr, LATTTYP_SZ);
    return arrobj;
}

LattReply pull_replyitm(buff_arr buf){
    LattReply replitm;
    memcpy(&replitm, rsparr_pos(buf),LATTTYP_SZ);

    return replitm;
}

LattLong pull_objid(InfoFrame** infoFrame, LattLong itmId, size_t long_sz){

    if (long_sz == 8){
        unsigned long ulong_hold = *((unsigned long*)((*infoFrame)->arr+8));
        memcpy((&itmId.l_ulong),&ulong_hold,(ULONG_SZ));
    }
    else if (long_sz == 16){
        unsigned char* vlong = malloc(VULONG_SZ);
        memcpy(vlong,(*infoFrame)->arr+(rspsz_b),VULONG_SZ);
        memcpy(itmId.l_uchar_16,vlong,VULONG_SZ);
        free(vlong);
    }
    else{
        itmId.l_ulong = 1;
    }
    return itmId;
}

uint pull_arrsz(buff_arr buf) {
    uint sz;
    memcpy(&sz, *buf + ARRSZ_POS, UINT_SZ);
    return  sz;
}

// returns: 1 = char_arr 2 = int_arr 3 = long_arr 0 = err
uint nxt_arrelem_type(buff_arr buf, uint offset){
    for (int i = 0; i < 3; i++){
        if (memcmp(&INT_SEP,*buf+offset,LATTTYP_SZ) != 0){
            return i+1;
        }
    }
    return 0;
}

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

uint check_end_flg(uchar_arr fullreqbuf, InfoFrame** infoFrame){
    uint end;           // END flag
    // 4B + 4b + 4B + 8B + (misc len) + 4B
    memcpy(&end, (fullreqbuf + FLAG_SZ + LATTTYP_SZ + UINT_SZ + ((*infoFrame)->arr_len)*UINT_SZ), FLAG_SZ);
    return (end != END) ? 1 : 0;
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
        newmsg = (char*) calloc(len+2,sizeof(char));
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
{memcpy(rsparr_pos(buf) + offst, msg, len);return len;}

uint rsparr_add_travelpath(TravelPath* travelpath, buff_arr buf, uint offst){
    uint len = offst;
    unsigned long long origid = travelpath->origin->did;
    unsigned long long destid = travelpath->destination->did;
    uchar charstr[17] = {'<','-','-','o','r','i','g',':',':','d','e','s','t','-','-','>','\0',};

    len += rsparr_addsep(len,LONG_SEP,buf);
    len += rsparr_add_lnglng(len,origid,buf);
    len += rsparr_addsep(len,CHAR_SEP,buf);
    len += rsparr_add_chrstr(buf,charstr,17,len);
    len += rsparr_addsep(len,LONG_SEP,buf);
    len += rsparr_add_lnglng(len,destid,buf);

    return len;
}

RspFlag rsplead_addflg(RspFlag flags, buff_arr buf){
    RspFlag o_flgs;
    memcpy(&o_flgs, *buf, FLAG_SZ);
    o_flgs |= flags;
    memcpy(*buf, &o_flgs, FLAG_SZ);
    uint i = 0;
    return o_flgs;
}

char* convertLattErr(const LattErr* _latterr){
    char* laterrstr = (char*) malloc(LERR_CHARCNT);
    LattErr _latterr_itr;
    _latterr_itr = LERR_LASTELEM;
    uint i = LERR_CNT;

    while (_latterr_itr != *_latterr) {
        _latterr_itr >>= 1;
        if (!--i) { return NULL; }
    }
    memcpy(laterrstr,latt_err_strs[i-1],LERR_CHARCNT);
    return laterrstr;
}

char* convertLattSts(const LattStts * _lattsts){
    char* lsttsstr = (char*) malloc(LSTS_CHARCNT);
    LattStts _lattsts_itr;
    _lattsts_itr = LSTS_LASTELEM;
    uint i = LSTS_CNT;

    while (_lattsts_itr != *_lattsts) {
        _lattsts_itr >>= 1;
        if (!--i) { return NULL; }
    }
    i=LSTS_CNT-i;
    memcpy(lsttsstr,latt_stts_strs[i],LSTS_CHARCNT);
    return lsttsstr;
}

unsigned int add_LattErr_to_buf(buff_arr buf,LattErr lerr) {

    LattErr latterr_itr;
    uint i = LERR_CNT;
    latterr_itr = MALREQ;
    while (latterr_itr != *latt_err_arr) {
        latterr_itr >>= 1;
        if (!--i) { return 1; }
    }
    memcpy((*buf), convertLattErr(&lerr), LATTTYP_SZ);
    return 0;
}

