//
// Created by ujlm on 11/2/23.
//

#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include "lattice_works.h"
#include "fidi_masks.h"
#include "lattice_rsps.h"
#include "consts.h"



/*
 *  Response element size aliases:
 *--------------------------------
 * lattyp_sz = sizeof(LattType)
 * rspsz_b = 2*sizeof(LattType)
 * uint_s = sizeof(unsigned int)
 * uchar_s = sizeof(unsigned char)
 * arr_b = (2*sizeof(LattType))+sizeof(uint)
 *
 *
 * Response element positions:
 *----------------------------
 *\elem\:  \start-byte - end-byte\
 * ------   ----------    -------
 * lead :   0 - lattyp_sz
 * item :   lattyp_sz - rspsz_b
 * arrsz:   rspsz_b - [rspsz_b + uint_s]
 * arr  :   [rspsz_b+uint_s] - [rspsz_b+uint_s+arr_len*uchar_s]
 * DONE :   (rspsz_b)+uint_s+(arr_len*uchar_s) - rspsz_b+lattyp_sz+uint_s+(arr_len*uchar_s)
 *
 *
 * Note:
 *      Items are seperated with 4 bytes of two kinds,
 *      one denoting chars to follow and one denoting ints:
 *
 *      int:    '0xdbdbdbdb' (3688618971)
 *      char:   '0xbddbdbbd' (3185302461)
 */



/* * * * * * * * * * * * *
*  RESPONSE CONSTRUCTION *
* * * * * * * * * * * * **/

unsigned char* rsparr_pos(buff_arr buf)
{return *buf+arr_b;}

uint prpbuf(buff_arr buf,LattType lattItm) {
    lattItm.flg.uflg =  STRT;
    memcpy(*buf,&lattItm,lattyp_sz);
    return lattyp_sz;
}

uint endbuf(buff_arr buf,LattType lattItm, uint rsplen) {
    lattItm.flg.uflg =  DONE;
    memcpy((*buf)+rsplen,&lattItm,lattyp_sz);
    return lattyp_sz;
}

uint rsparr_len_inc(void* itm, uint cnt, uint mlti)
{return mlti ? ((mlti*sizeof(*itm))+cnt) : sizeof(*itm)+cnt;}

void rsparr_len_set(uint sz, buff_arr buf){memcpy(*buf+rspsz_b,&sz,uint_sz);}

void rsparr_out(buff_arr buf, uint arrlen){
    uint n = 0;
    while(putchar_unlocked(*(*buf + n))) {
        if (n == arrlen) {
            break;
        }
        ++n;
    }
}

// iorc =  0 : char,  1 : int
uint rsparr_addsep(buff_arr buf,uint offset,uint iorc)
{memcpy(rsparr_pos(buf)+offset,(iorc ? &isep : &csep),lattyp_sz);return lattyp_sz;}

uint rsparr_add_lt(LattType lt, buff_arr buf, uint offset)
{memcpy(rsparr_pos(buf)+offset+lattyp_sz,&lt,lattyp_sz);return rsparr_addsep(buf,offset,1)+lattyp_sz;}

uint rsparr_add_obj(buff_arr buf, LattType obj){
    return rsparr_add_lt((LattType) obj, buf, 0);
}

uint rsparr_add_lng(uint offset, LattLong lli, buff_arr buf){
    {memcpy(rsparr_pos(buf)+offset+(sizeof(LattLong)),&lli,sizeof(LattLong));
        return rsparr_addsep(buf,offset,1)+sizeof(unsigned long long int);}
}

uint rsparr_add_msg(buff_arr buf, char* msg, uint len, uint offst)
{memcpy(rsparr_pos(buf)+offst+lattyp_sz,msg,len);return rsparr_addsep(buf,offst,0)+len;}

uint rsparr_add_chrstr(buff_arr buf, uchar_arr msg, uint len, uint offst)
{memcpy(rsparr_pos(buf)+offst+lattyp_sz,msg,len);return rsparr_addsep(buf,offst,0)+len;}

uint pull_objid(InfoFrame** infoFrame, LattLong** itmId){


    memcpy((*itmId),(*infoFrame)->arr+(rspsz_b),(rspsz_b));

    return (*itmId)->l_ulonglong_ptr == NULL;

}



// lead | item | arrsz | arr | DONE


/* * * * * * * * * * * * * * *
 * INFO RESPONSE FUNCTIONS  *
 * * * * * * * * * * * * * **/

uint inf_NADA(buff_arr buf, LattType lattItm, LattStruct lattStruct){

    return 0;

}
uint inf_LTTC(buff_arr buf, LattType lattItm, LattStruct lattStruct){
    uint bcnt;
    lattItm.obj = LTTC;
    bcnt = rsparr_add_lt(lattItm,buf,0);

    bcnt += rsparr_add_msg(buf,"CNT",4,bcnt);
    lattItm.n_uint = lattStruct.lattice->count;
    bcnt += rsparr_add_lt(lattItm,buf,bcnt);

    bcnt += rsparr_add_msg(buf,"MAX",4,bcnt);
    lattItm.n_uint = lattStruct.lattice->max;
    bcnt += rsparr_add_lt(lattItm,buf,bcnt);

    return bcnt;

}
uint inf_BRDG(buff_arr buf, LattType lattItm, LattStruct lattStruct){
    uint bcnt;
    lattItm.obj = BRDG;
    bcnt = rsparr_add_obj(buf,lattItm);
    LattLong brdg_id;
    brdg_id.l_ulong_ptr= lattStruct.itmID;

    HashBridge * hshBrdg = yield_bridge_for_fihsh(lattStruct.lattice, *brdg_id.l_ulong_ptr);
    if(hshBrdg == NULL){
        return 1;
    }

    bcnt += rsparr_add_msg(buf, "UID",4,bcnt);
    bcnt+= rsparr_add_chrstr(buf,hshBrdg->unid,(uint_sz*4),bcnt);

    bcnt += rsparr_add_msg(buf, "FID",4,bcnt);
    bcnt+= rsparr_add_lng(bcnt,(LattLong) hshBrdg->finode->fiid,buf);

    bcnt += rsparr_add_msg(buf, "UID",4,bcnt);
    bcnt+= rsparr_add_lng(bcnt,(LattLong) hshBrdg->dirnode->did,buf);

    return bcnt;


    //TODO error response

}
uint inf_DIRN(buff_arr buf, LattType lattItm, LattStruct lattStruct){
    lattItm.obj = DIRN;
    rsparr_add_lt(lattItm,buf,0);

    return 0;
}
uint inf_FTBL(buff_arr buf, LattType lattItm, LattStruct lattStruct){
    lattItm.obj = FTBL;
    rsparr_add_lt(lattItm,buf,0);
    return 0;
}
uint inf_FIMP(buff_arr buf, LattType lattItm, LattStruct lattStruct){
    lattItm.obj = FIMP;


    rsparr_add_lt(lattItm,buf,0);
    return 0;
}
uint inf_LFLG(buff_arr buf, LattType lattItm, LattStruct lattStruct){
    lattItm.obj = LFLG;
    rsparr_add_lt(lattItm,buf,0);
    return 0;
}
uint inf_SFRM(buff_arr buf, LattType lattItm, LattStruct lattStruct){
    lattItm.obj = SFRM;
    rsparr_add_lt(lattItm,buf,0);
    return 0;
}
uint inf_IFRM(buff_arr buf, LattType lattItm, LattStruct lattStruct){
    lattItm.obj = IFRM;
    rsparr_add_lt(lattItm,buf,0);
    return 0;
}
uint inf_SEQT(buff_arr buf, LattType lattItm, LattStruct lattStruct){
    lattItm.obj = SEQT;
    rsparr_add_lt(lattItm,buf,0);
    return 0;
}
uint inf_CMSQ(buff_arr buf, LattType lattItm, LattStruct lattStruct){
    lattItm.obj = CMSQ;
    rsparr_add_lt(lattItm,buf,0);
    return 0;
}
uint inf_ICAR(buff_arr buf, LattType lattItm, LattStruct lattStruct){
    lattItm.obj = ICAR;
    rsparr_add_lt(lattItm,buf,0);
    return 0;
}
uint inf_VSSL(buff_arr buf, LattType lattItm, LattStruct lattStruct){
    lattItm.obj = VSSL;
    rsparr_add_lt(lattItm,buf,0);
    return 0;
}
uint inf_FIOB(buff_arr buf, LattType lattItm, LattStruct lattStruct){
    lattItm.obj = FIOB;
    rsparr_add_lt(lattItm,buf,0);
    return 0;
}
uint inf_IDID(buff_arr buf, LattType lattItm, LattStruct lattStruct){
    lattItm.obj = IDID;
    rsparr_add_lt(lattItm,buf,0);
    return 0;
}
uint inf_NMNM(buff_arr buf, LattType lattItm, LattStruct lattStruct){
    lattItm.obj = NMNM;
    rsparr_add_lt(lattItm,buf,0);
    return 0;
}
uint inf_FIDE(buff_arr buf, LattType lattItm, LattStruct lattStruct){
    lattItm.obj = FIDE;
    rsparr_add_lt(lattItm,buf,0);
    return 0;
}
uint inf_DCHN(buff_arr buf, LattType lattItm, LattStruct lattStruct){
    lattItm.obj = DCHN;
    rsparr_add_lt(lattItm,buf,0);
    return 0;
}


/**
 *
 * \GenerateInfoFunctionArray
 * Generate function array for info response operations */
void gen_infofunc_arr(InfoFunc** infofuncarr){

    uint (*nada)(buff_arr buf, LattType lattItm, LattStruct lattStruct);
    uint (*seqt)(buff_arr buf, LattType lattItm, LattStruct lattStruct);
    uint (*lttc)(buff_arr buf, LattType lattItm, LattStruct lattStruct);
    uint (*cmsq)(buff_arr buf, LattType lattItm, LattStruct lattStruct);
    uint (*brdg)(buff_arr buf, LattType lattItm, LattStruct lattStruct);
    uint (*icar)(buff_arr buf, LattType lattItm, LattStruct lattStruct);
    uint (*dirn)(buff_arr buf, LattType lattItm, LattStruct lattStruct);
    uint (*vssl)(buff_arr buf, LattType lattItm, LattStruct lattStruct);
    uint (*ftbl)(buff_arr buf, LattType lattItm, LattStruct lattStruct);
    uint (*fiob)(buff_arr buf, LattType lattItm, LattStruct lattStruct);
    uint (*fimp)(buff_arr buf, LattType lattItm, LattStruct lattStruct);
    uint (*idid)(buff_arr buf, LattType lattItm, LattStruct lattStruct);
    uint (*lflg)(buff_arr buf, LattType lattItm, LattStruct lattStruct);
    uint (*nmnm)(buff_arr buf, LattType lattItm, LattStruct lattStruct);
    uint (*sfrm)(buff_arr buf, LattType lattItm, LattStruct lattStruct);
    uint (*fide)(buff_arr buf, LattType lattItm, LattStruct lattStruct);
    uint (*ifrm)(buff_arr buf, LattType lattItm, LattStruct lattStruct);
    uint (*dchn)(buff_arr buf, LattType lattItm, LattStruct lattStruct);

    nada = &inf_NADA; seqt = &inf_SEQT;
    lttc = &inf_LTTC; cmsq = &inf_CMSQ;
    brdg = &inf_BRDG; icar = &inf_ICAR;
    dirn = &inf_DIRN; vssl = &inf_VSSL;
    ftbl = &inf_FTBL; fiob = &inf_FIOB;
    fimp = &inf_FIMP; idid = &inf_IDID;
    lflg = &inf_LFLG; nmnm = &inf_NMNM;
    sfrm = &inf_SFRM; fide = &inf_FIDE;
    ifrm = &inf_IFRM; dchn = &inf_DCHN;

    (**infofuncarr)[0] = nada; (**infofuncarr)[9] = dchn;
    (**infofuncarr)[1] = lttc; (**infofuncarr)[10] = seqt;
    (**infofuncarr)[2] = brdg; (**infofuncarr)[11] = cmsq;
    (**infofuncarr)[3] = dirn; (**infofuncarr)[12] = icar;
    (**infofuncarr)[4] = ftbl; (**infofuncarr)[13] = vssl;
    (**infofuncarr)[5] = fimp; (**infofuncarr)[14] = fiob;
    (**infofuncarr)[6] = lflg; (**infofuncarr)[15] = idid;
    (**infofuncarr)[7] = sfrm; (**infofuncarr)[16] = nmnm;
    (**infofuncarr)[8] = ifrm; (**infofuncarr)[17] = fide;

}

/* * * * * * * * * * * * * * * *
 * RESPONSE AVENUES AND ACTION *
 * * * * * * * * * * * * * * * */


uint rsp_err(StatFrame **sts_frm, InfoFrame **inf_frm,  Lattice* hltc, buff_arr buf) {

    uint ercode = (*sts_frm)->err_code;
    memcpy(buf,&ercode,sizeof(LattErr));

    return sizeof(LattErr);
}

/** Info funcs */
uint rsp_nfo(StatFrame **sts_frm, InfoFrame **inf_frm, Lattice * hltc, buff_arr buf) {

    LattType lattItm;
    lattItm.n_uint = 0;
    printf("Response: info");
    size_t bcnt = arr_b;
    u_long itmID;
    size_t arrlen = 0;
    uint respsz;
    uint i;
    LattObj objeid;
    InfoFunc* infofuncarr;
    LattStruct lattStruct;

    infofuncarr = (InfoFunc*) malloc(sizeof(InfoFunc));

    lattStruct.lattice = *hltc;
    lattStruct.statFrame = **sts_frm;

    gen_infofunc_arr(&infofuncarr);

    // Extract the desired object ID from the first byte of the request array,
    // and the array content, which can be an ID, etc. depending on the subject.
    //
    // Fail if no code found


    memcpy(&lattItm.obj,((*inf_frm)->arr),lattyp_sz);
    //TODO: Fail on malformed request

    if (lattItm.obj>FIDE || (lattItm.n_uint & (lattItm.n_uint - 1)) ){
        stsErno(MALREQ, sts_frm, errno, lattItm.n_uint, "Invalid object ID provided.", "response::info", "object id value");
        return 1;
        //TODO: Better error value for failed response processing.
    }

    // query requested object and return response size in bytes.

    i = __builtin_ctz(lattItm.n_uint) + 1;   // Info Req subject index = count of its values trailing zeros.
    if (i > 18){
        respsz = 1;
    }else {
        // Extract identifier which would be in the request array at position array start+8
        memcpy(&itmID,(*inf_frm)->arr+(rspsz_b),(rspsz_b));
        (lattStruct.itmID) = &itmID;
        respsz = ((*infofuncarr)[i](buf, lattItm, lattStruct));
    }
    free(*infofuncarr);

    // return of 0 means the object is there but failed to be queried.
    // return of 1 means the object is missing entirely.
    if (!respsz){
        stsErno(MISSNG,sts_frm,errno,0,"Info requested couldn't be retrieved.","resp::info",NULL);
        return 0;
    }
    if (respsz == 1) {
        stsErno(UNKNWN,sts_frm,errno,0,"Info requested for unknown object.","resp::info",NULL);
        return 0;
    }

    return respsz+bcnt;

}
//uint bcnt;
//lattItm.obj = LTTC;
//bcnt = rsparr_add_lt(lattItm,buf,0);
uint rsp_sts(StatFrame **sts_frm, InfoFrame **inf_frm, Lattice * hltc, buff_arr buf) {
    printf("Response: Status frame");
    uint bcnt;
    LattType lattitm;
    lattitm.obj = SFRM;
    bcnt = rsparr_add_lt(lattitm,buf,0);

    bcnt += rsparr_add_msg(buf,"STS",lattyp_sz,bcnt);
    lattitm.sts =(*sts_frm)->status;
    bcnt += rsparr_add_lt(lattitm,buf,bcnt);

    bcnt += rsparr_add_msg(buf,"ERR",lattyp_sz,bcnt);
    lattitm.err = (*sts_frm)->err_code;
    bcnt += rsparr_add_lt(lattitm,buf,bcnt);

    bcnt += rsparr_add_msg(buf,"ACT",lattyp_sz,bcnt);
    lattitm.act = (*sts_frm)->act_id;
    bcnt += rsparr_add_lt(lattitm,buf,bcnt);

    bcnt += rsparr_add_msg(buf,"MOD",lattyp_sz,bcnt);
    lattitm.n_uint = (*sts_frm)->modr;
    bcnt += rsparr_add_lt(lattitm,buf,bcnt);

    return bcnt;

    //TODO: Test and verify this function
}

uint rsp_und(StatFrame **sts_frm, InfoFrame **inf_frm, Lattice * hltc, buff_arr buf) {


    return 0;
}

uint rsp_fiid(StatFrame **sts_frm, InfoFrame **inf_frm, Lattice * hltc, buff_arr buf) {
    printf("Response: Fiid for hashno");



    LattLong* itmID = malloc(ulong_sz);
    LattType lattitm;
    lattitm.obj = IDID;
    uint bcnt;

    bcnt = rsparr_add_lt(lattitm,buf,0);
    memcpy(itmID,(*inf_frm)->arr+(rspsz_b),(rspsz_b));


    if (pull_objid(inf_frm, &(itmID))){
        stsErno(MALREQ, sts_frm, errno, itmID->l_ulong, "Given fiID is invalid or mangled.", "rsp_fiid", "parsed id");
        return 1;
    }

    Armatr armatr = (*hltc)->chains->vessel->armature;

    if (armatr == NULL){
        stsErno(NOINFO,sts_frm,errno,(*hltc)->chains->vessel->did,"Vessel is likely located in a base node.","rsp_fiid","vessel diID");
        return 1;
    }

    itmID->l_ulong = armatr->entries[getidx(itmID->l_ulong)].fiid;

    bcnt += rsparr_add_lng(bcnt,*itmID,buf);

    free(itmID);

    return bcnt;
}

uint rsp_diid(StatFrame **sts_frm, InfoFrame **inf_frm, Lattice * hltc, buff_arr buf) {
    printf("Response: Dir ID");

    LattLong* itmID = malloc(sizeof(unsigned long));
    LattType lattitm;
    lattitm.obj = DIRN;
    uint bcnt;

    bcnt = rsparr_add_lt(lattitm,buf,0);

    memcpy(&itmID->l_ulong, (*inf_frm)->arr + (rspsz_b), (rspsz_b));

    if (pull_objid(inf_frm, &(itmID))){
        stsErno(MALREQ, sts_frm, errno, itmID->l_ulong, "Given dirID is invalid or mangled.", "rsp_diid", "parsed id");
        return 1;
    }

    LattLong diID;
    diID.l_ulonglong = (*hltc)->chains->vessel->did;
    bcnt += rsparr_add_lng(bcnt, diID, buf);
    free(itmID);
    return bcnt;
}

uint rsp_frdn(StatFrame **sts_frm, InfoFrame **inf_frm, Lattice * hltc, buff_arr buf) {
    printf("Response: Resident Dir");
}

uint rsp_gond(StatFrame **sts_frm, InfoFrame **inf_frm, Lattice * hltc, buff_arr buf) {
    printf("Response: Goto node");
}

uint rsp_fyld(StatFrame **sts_frm, InfoFrame **inf_frm, Lattice * hltc, buff_arr buf) {
    printf("Response: Yield object");
}

uint rsp_jjjj(StatFrame **sts_frm, InfoFrame **inf_frm, Lattice * hltc, buff_arr buf) {
    printf("Response: Empty");
}

uint rsp_dsch(StatFrame **sts_frm, InfoFrame **inf_frm, Lattice * hltc, buff_arr buf) {
    printf("Response: Search for object");
}

uint rsp_iiii(StatFrame **sts_frm, InfoFrame **inf_frm, Lattice * hltc, buff_arr buf) {
    printf("Response: Empty");
}

uint rsp_dcls(StatFrame **sts_frm, InfoFrame **inf_frm, Lattice *hltc, buff_arr buf) {
    printf("Response: List chain nodes");
}

uint rsp_gohd(StatFrame **sts_frm, InfoFrame **inf_frm, Lattice *hltc, buff_arr buf) {
    printf("Response: Go to chain head");
}

uint rsp_dnls(StatFrame **sts_frm, InfoFrame **inf_frm, Lattice *hltc, buff_arr buf) {
    printf("Response: List dir ");
}

uint rsp_vvvv(StatFrame **sts_frm, InfoFrame **inf_frm, Lattice *hltc, buff_arr buf) {
    printf("Response: Empty");
}


/**
 * \ResponseAction
 *    Initialize and return ResponseMap struct.
 * <br>
 *        Call fucntions with:
 * <br><br>
 * <code>
 *        (*funarr[cmd])(sts_frm, inf_frm, dchns, hltc, buf);
* */
RspFunc *rsp_act(
        RspMap rsp_map,
        StatFrame **sts_frm,
        InfoFrame **inf_frm,
        RspFunc *funarr){

//Info
    uint(*und)(StatFrame **sts_frm, InfoFrame **inf_frm, Lattice *hltc, buff_arr buf);
    uint(*err)(StatFrame **sts_frm, InfoFrame **inf_frm, Lattice *hltc, buff_arr buf);
    uint(*nfo)(StatFrame **sts_frm, InfoFrame **inf_frm, Lattice *hltc, buff_arr buf);
    uint(*sts)(StatFrame **sts_frm, InfoFrame **inf_frm, Lattice *hltc, buff_arr buf);
//Travel*
    uint (*fiid)(StatFrame **sts_frm, InfoFrame **inf_frm, Lattice *hltc, buff_arr buf);
    uint (*diid)(StatFrame **sts_frm, InfoFrame **inf_frm, Lattice *hltc, buff_arr buf);
    uint (*frdn)(StatFrame **sts_frm, InfoFrame **inf_frm, Lattice *hltc, buff_arr buf);
    uint (*gond)(StatFrame **sts_frm, InfoFrame **inf_frm, Lattice *hltc, buff_arr buf);
//Dir*
    uint (*fyld)(StatFrame **sts_frm, InfoFrame **inf_frm, Lattice *hltc, buff_arr buf);
    uint (*jjjj)(StatFrame **sts_frm, InfoFrame **inf_frm, Lattice *hltc, buff_arr buf);
    uint (*dsch)(StatFrame **sts_frm, InfoFrame **inf_frm, Lattice *hltc, buff_arr buf);
    uint (*iiii)(StatFrame **sts_frm, InfoFrame **inf_frm, Lattice *hltc, buff_arr buf);
//File*
    uint (*dcls)(StatFrame **sts_frm, InfoFrame **inf_frm, Lattice *hltc, buff_arr buf);
    uint (*gohd)(StatFrame **sts_frm, InfoFrame **inf_frm, Lattice *hltc, buff_arr buf);
    uint (*dnls)(StatFrame **sts_frm, InfoFrame **inf_frm, Lattice *hltc, buff_arr buf);
    uint (*vvvv)(StatFrame **sts_frm, InfoFrame **inf_frm, Lattice *hltc, buff_arr buf);

    und = &rsp_und;
    err = &rsp_err;
    nfo = &rsp_nfo;
    sts = &rsp_sts;
    fiid = &rsp_fiid;
    diid = &rsp_diid;
    frdn = &rsp_frdn;
    gond = &rsp_gond;
    fyld = &rsp_fyld;
    jjjj = &rsp_jjjj;
    dsch = &rsp_dsch;
    iiii = &rsp_iiii;
    dcls = &rsp_dcls;
    gohd = &rsp_gohd;
    dnls = &rsp_dnls;
    vvvv = &rsp_vvvv;

/* Info ops*/
    (*funarr)[0] = fiid; // File id
    (*funarr)[1] = diid; // DirNode id
    (*funarr)[2] = frdn; // Return resident dn of given
    (*funarr)[3] = gond; // Go to: given dirnode.
    (*funarr)[4] = fyld; // Yeild object
    (*funarr)[5] = jjjj; // Empty
    (*funarr)[6] = und;  // Undefined
    (*funarr)[7] = dsch; // Search for resident dir of given
    (*funarr)[8] = iiii; // Empty
    (*funarr)[9] = dcls; // List dirchains nodes
    (*funarr)[10] = err; // LattErr error code
    (*funarr)[11] = gohd; // Go to: head node or switch base
    (*funarr)[12] = nfo; // Info string for given
    (*funarr)[13] = dnls; // List dn contents
    (*funarr)[14] = sts; // current status frame
    (*funarr)[15] = vvvv; //

/* Goto resident location of a given ffile */

/* DirNode ops */

/* File ops */

    return funarr;
}


/* * * * * * * * * * * **
*  Response processing  *
* * * * * * * * * * * * */


/**
 * Init response table
 */
void
init_rsptbl(int cnfg_fd, Resp_Tbl **rsp_tbl, StatFrame **sts_frm, InfoFrame **inf_frm, DChains *dchns, Lattice *hltc) {
    uint *rsp_map;
    uint fcnt = RSPARRLEN;
    RspFunc *rsp_func;

    *rsp_tbl = (Resp_Tbl*) malloc(sizeof(Resp_Tbl));
    rsp_func = (RspFunc*) malloc(sizeof(RspFunc));

    (*rsp_tbl)->rsp_funcarr = rsp_act(&rsp_map, sts_frm, inf_frm, rsp_func);
    (*rsp_tbl)->rsp_map = (RspMap *) &rsp_map;
    (*rsp_tbl)->fcnt = fcnt;
}


/**
 * \DetermineResponse
 *  Returns a LattReply struct that determines the response avenue given output from request parsing.
 */
LattType dtrm_rsp(StatFrame **sts_frm,
                  InfoFrame **inf_frm,
                  LattType reply) {

    reply.n_uint = 0; //LattReply to determine response avenues
    uint sys = 0; //Request is a system op


    if ((*inf_frm)->trfidi[0]) {
        reply.rpl = DIRID | DNODE | (((*inf_frm)->cat_pfx)<<2);  //Travel op -> masked w/ DNODE qualifier and bit #1 : 0011
    } else {
        if ((*inf_frm)->trfidi[2]) {
            reply.rpl = DIRID | (((*inf_frm)->cat_pfx)<<2);    //Dir Op -> masked with bit #1 : 0001
        } else {
            if (!(*inf_frm)->trfidi[1]) {    //Not a file op.
                if ((*inf_frm)->sys_op) {
                    if ((*inf_frm)->qual == DFLT) {
                        reply.rpl = STATS;  // Info op + DFLT flag -> status frame
                    } else {
                        reply.rpl = OINFO;    // Info op -> masked with bits #3 and #4 : 1100
                    }
                } else {
                    sys = 1;    // System op -> unique handling. : 0110/1010
                }
            } else {// File op, bit #1 stays False, and no more than 1 bit set True : 0000
                reply.rpl = ((1<<((*inf_frm)->cat_pfx))&~1);
            }
        }
    }

    if(reply.n_uint > 15 || reply.n_int < 0){
        setErr(sts_frm,MISCLC,reply.rpl);
        serrOut(sts_frm,"Response determination failed.");
        reply.rpl = ERRCD;
    }

    return reply;
}

/**
 * \ProcessResponse
 */
uint respond(Resp_Tbl *rsp_tbl,
                     StatFrame **sts_frm,
                     InfoFrame **inf_frm,
                     DChains *dchns,
                     Lattice *hltc,
                     uchar_arr rsp_buf) {

    //TODO: Reset infoframe, implement and include.
    LattType lattItm;
    lattItm.n_uint =  0;

    prpbuf(&rsp_buf,lattItm);
    /** Determine response avenue and return a reply object */
    lattItm = dtrm_rsp(sts_frm,inf_frm,lattItm);  // Reply object returned from 'determine response'

    uint rspsz;
    uint rsparrsz;
    // Update status and return 1 on failure
    // if the value of reply object is > the
    // num of function in the array.
    if (lattItm.n_uint > rsp_tbl->fcnt) {
        setErr(sts_frm, MISCLC, lattItm.n_uint);
        serrOut(sts_frm, "LattReply for response processing outside defined functionality.");
        return 1;
    }

    // Insert reply object into buffer at +sizeof(LattTyp) offset
    memcpy(rsp_buf+uint_sz,&lattItm,lattyp_sz);

    // Call function at index 'rsp' (the reply object value) from function array.
    rspsz = (*rsp_tbl->rsp_funcarr)[lattItm.rpl](sts_frm, inf_frm, hltc, &rsp_buf);
    if (!rspsz){
        return 1;
    }
    rspsz += endbuf(&rsp_buf,lattItm,rspsz);

    // Get array len from response sequence
    rsparrsz = rspsz - (4*lattyp_sz);

    rsparr_len_set(rsparrsz,&rsp_buf);

    // Update InfoFrame
    (*inf_frm)->arr_len = rsparrsz;
    (*inf_frm)->rsp_size = rspsz;

    // Update status and return 0 on success
    setSts(sts_frm, RESPN, 0);
    return 0;
}

