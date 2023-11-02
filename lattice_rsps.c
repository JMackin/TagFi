//
// Created by ujlm on 11/2/23.
//

#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include "lattice_works.h"
#include "lattice_rsps.h"

const LattType isep = (LattType) 0xdbdbdbdb;
const LattType csep = (LattType) 0xbddbdbbd;

/*
 *  Response element size aliases:
 *
 *
 * ltyp_s = sizeof(LattType)
 * rspsz_b = 2*sizeof(LattType)
 * uint_s = sizeof(unsigned int)
 * uchar_s = sizeof(unsigned char)
 * arr_b = (2*sizeof(LattType))+sizeof(uint)
 *
 *
 * Response element positions:
 *----------------------------
 *\elem\:  \start-byte - end-byte\
 * lead :   0 - ltyp_s
 * item :   ltyp_s - rspsz_b
 * arrsz:   rspsz_b - (rspsz_b)+uint_s
 * arr  :   (rspsz_b)+uint_s - (rspsz_b)+uint_s+(arr_len*uchar_s)
 * DONE :   (rspsz_b)+uint_s+(arr_len*uchar_s) - rspsz_b+ltyp_s+uint_s+(arr_len*uchar_s)
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

unsigned char* rsparr_pos(unsigned char** buf)
{return *buf+arr_b;}

unsigned int prpbuf(unsigned char **buf,LattType lattItm) {
    lattItm.flg.uflg =  STRT;
    memcpy(*buf,&lattItm,ltyp_s);
    return ltyp_s;
}

unsigned int endbuf(unsigned char **buf,LattType lattItm, unsigned int rsplen) {
    lattItm.flg.uflg =  DONE;
    memcpy((*buf)+rsplen,&lattItm,ltyp_s);
    return ltyp_s;
}

unsigned int rsparr_len_inc(void* itm, unsigned int cnt, unsigned int mlti)
{return mlti ? ((mlti*sizeof(*itm))+cnt) : sizeof(*itm)+cnt;}

void rsparr_len_set(unsigned int sz, unsigned char **buf){memcpy(*buf+rspsz_b,&sz,UISiZ);}

void rsparr_out(unsigned char **buf, unsigned int arrlen){
    uint n = 0;
    while(putchar_unlocked(*(*buf + n))) {
        if (n == arrlen) {
            break;
        }
        ++n;
    }
}

// iorc =  0 : char,  1 : int
unsigned int rsparr_addsep(unsigned char** buf,unsigned int offset,unsigned int iorc)
{memcpy(rsparr_pos(buf)+offset,(iorc ? &isep : &csep),ltyp_s);return ltyp_s;}

unsigned int rsparr_add_lt(LattType lt, unsigned char** buf, unsigned int offset)
{memcpy(rsparr_pos(buf)+offset+ltyp_s,&lt,ltyp_s);return rsparr_addsep(buf,offset,1)+ltyp_s;}

unsigned int rsparr_add_replobj(unsigned char** buf, LattReply obj){
    return rsparr_add_lt((LattType) obj, buf, 0);
}

unsigned int rsparr_add_msg(unsigned char **buf, char* msg, unsigned int len, unsigned int offst)
{memcpy(rsparr_pos(buf)+offst+ltyp_s,msg,len);return rsparr_addsep(buf,offst,0)+len;}

// lead | item | arrsz | arr | DONE




/* * * * * * * * * * * * * * *
 * INFO RESPONSE FUNCTIONS  *
 * * * * * * * * * * * * * **/

unsigned int inf_NADA(unsigned char **buf, LattType lattItm, LattStruct lattStruct){

    return 0;

}
unsigned int inf_LTTC(unsigned char **buf, LattType lattItm, LattStruct lattStruct){
    uint bcnt;
    lattItm.obj = LTTC;
    bcnt = rsparr_add_lt(lattItm,buf,0);

    bcnt += rsparr_add_msg(buf,"CNT",4,bcnt);
    lattItm.nui = lattStruct.lattice->count;
    bcnt += rsparr_add_lt(lattItm,buf,bcnt);

    bcnt += rsparr_add_msg(buf,"MAX",4,bcnt);
    lattItm.nui = lattStruct.lattice->max;
    bcnt += rsparr_add_lt(lattItm,buf,bcnt);

    return bcnt;

}
unsigned int inf_BRDG(unsigned char **buf, LattType lattItm, LattStruct lattStruct){
    lattItm.obj = BRDG;


}
unsigned int inf_DIRN(unsigned char **buf, LattType lattItm, LattStruct lattStruct){
    lattItm.obj = DIRN;

    rsparr_add_lt(lattItm,buf,0);
    return 0;
}
unsigned int inf_FTBL(unsigned char **buf, LattType lattItm, LattStruct lattStruct){
    lattItm.obj = FTBL;
    rsparr_add_lt(lattItm,buf,0);
    return 0;
}
unsigned int inf_FIMP(unsigned char **buf, LattType lattItm, LattStruct lattStruct){
    lattItm.obj = FIMP;
    rsparr_add_lt(lattItm,buf,0);
    return 0;
}
unsigned int inf_LFLG(unsigned char **buf, LattType lattItm, LattStruct lattStruct){
    lattItm.obj = LFLG;
    rsparr_add_lt(lattItm,buf,0);
    return 0;
}
unsigned int inf_SFRM(unsigned char **buf, LattType lattItm, LattStruct lattStruct){
    lattItm.obj = SFRM;
    rsparr_add_lt(lattItm,buf,0);
    return 0;
}
unsigned int inf_IFRM(unsigned char **buf, LattType lattItm, LattStruct lattStruct){
    lattItm.obj = IFRM;
    rsparr_add_lt(lattItm,buf,0);
    return 0;
}
unsigned int inf_SEQT(unsigned char **buf, LattType lattItm, LattStruct lattStruct){
    lattItm.obj = SEQT;
    rsparr_add_lt(lattItm,buf,0);
    return 0;
}
unsigned int inf_CMSQ(unsigned char **buf, LattType lattItm, LattStruct lattStruct){
    lattItm.obj = CMSQ;
    rsparr_add_lt(lattItm,buf,0);
    return 0;
}
unsigned int inf_ICAR(unsigned char **buf, LattType lattItm, LattStruct lattStruct){
    lattItm.obj = ICAR;
    rsparr_add_lt(lattItm,buf,0);
    return 0;
}
unsigned int inf_VSSL(unsigned char **buf, LattType lattItm, LattStruct lattStruct){
    lattItm.obj = VSSL;
    rsparr_add_lt(lattItm,buf,0);
    return 0;
}
unsigned int inf_FIOB(unsigned char **buf, LattType lattItm, LattStruct lattStruct){
    lattItm.obj = FIOB;
    rsparr_add_lt(lattItm,buf,0);
    return 0;
}
unsigned int inf_IDID(unsigned char **buf, LattType lattItm, LattStruct lattStruct){
    lattItm.obj = IDID;
    rsparr_add_lt(lattItm,buf,0);
    return 0;
}
unsigned int inf_NMNM(unsigned char **buf, LattType lattItm, LattStruct lattStruct){
    lattItm.obj = NMNM;
    rsparr_add_lt(lattItm,buf,0);
    return 0;
}
unsigned int inf_FIDE(unsigned char **buf, LattType lattItm, LattStruct lattStruct){
    lattItm.obj = FIDE;
    rsparr_add_lt(lattItm,buf,0);
    return 0;
}
unsigned int inf_DCHN(unsigned char **buf, LattType lattItm, LattStruct lattStruct){
    lattItm.obj = DCHN;
    rsparr_add_lt(lattItm,buf,0);
    return 0;
}


/**
 *
 * \GenerateInfoFunctionArray
 * Generate function array for info response operations */
InfoFunc* gen_infofunc_arr(){

    InfoFunc* infofuncarr = (InfoFunc*) malloc(sizeof(InfoFunc));

    unsigned int (*nada)(unsigned char **buf, LattType lattItm, LattStruct lattStruct);
    unsigned int (*seqt)(unsigned char **buf, LattType lattItm, LattStruct lattStruct);
    unsigned int (*lttc)(unsigned char **buf, LattType lattItm, LattStruct lattStruct);
    unsigned int (*cmsq)(unsigned char **buf, LattType lattItm, LattStruct lattStruct);
    unsigned int (*brdg)(unsigned char **buf, LattType lattItm, LattStruct lattStruct);
    unsigned int (*icar)(unsigned char **buf, LattType lattItm, LattStruct lattStruct);
    unsigned int (*dirn)(unsigned char **buf, LattType lattItm, LattStruct lattStruct);
    unsigned int (*vssl)(unsigned char **buf, LattType lattItm, LattStruct lattStruct);
    unsigned int (*ftbl)(unsigned char **buf, LattType lattItm, LattStruct lattStruct);
    unsigned int (*fiob)(unsigned char **buf, LattType lattItm, LattStruct lattStruct);
    unsigned int (*fimp)(unsigned char **buf, LattType lattItm, LattStruct lattStruct);
    unsigned int (*idid)(unsigned char **buf, LattType lattItm, LattStruct lattStruct);
    unsigned int (*lflg)(unsigned char **buf, LattType lattItm, LattStruct lattStruct);
    unsigned int (*nmnm)(unsigned char **buf, LattType lattItm, LattStruct lattStruct);
    unsigned int (*sfrm)(unsigned char **buf, LattType lattItm, LattStruct lattStruct);
    unsigned int (*fide)(unsigned char **buf, LattType lattItm, LattStruct lattStruct);
    unsigned int (*ifrm)(unsigned char **buf, LattType lattItm, LattStruct lattStruct);
    unsigned int (*dchn)(unsigned char **buf, LattType lattItm, LattStruct lattStruct);

    nada = &inf_NADA; seqt = &inf_SEQT;
    lttc = &inf_LTTC; cmsq = &inf_CMSQ;
    brdg = &inf_BRDG; icar = &inf_ICAR;
    dirn = &inf_DIRN; vssl = &inf_VSSL;
    ftbl = &inf_FTBL; fiob = &inf_FIOB;
    fimp = &inf_FIMP; idid = &inf_IDID;
    lflg = &inf_LFLG; nmnm = &inf_NMNM;
    sfrm = &inf_SFRM; fide = &inf_FIDE;
    ifrm = &inf_IFRM; dchn = &inf_DCHN;

    (*infofuncarr)[0] = nada; (*infofuncarr)[9] = dchn;
    (*infofuncarr)[1] = lttc; (*infofuncarr)[10] = seqt;
    (*infofuncarr)[2] = brdg; (*infofuncarr)[11] = cmsq;
    (*infofuncarr)[3] = dirn; (*infofuncarr)[12] = icar;
    (*infofuncarr)[4] = ftbl; (*infofuncarr)[13] = vssl;
    (*infofuncarr)[5] = fimp; (*infofuncarr)[14] = fiob;
    (*infofuncarr)[6] = lflg; (*infofuncarr)[15] = idid;
    (*infofuncarr)[7] = sfrm; (*infofuncarr)[16] = nmnm;
    (*infofuncarr)[8] = ifrm; (*infofuncarr)[17] = fide;

    return infofuncarr;
}



/* * * * * * * * * * * * * * * *
 * RESPONSE AVENUES AND ACTION *
 * * * * * * * * * * * * * * * */



unsigned int rsp_err(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *chns, Lattice* hltc, unsigned char **buf) {

    unsigned int ercode = (*sts_frm)->err_code;
    memcpy(buf,&ercode,sizeof(LattErr));

    return sizeof(LattErr);
}

/** Info funcs */
unsigned int rsp_nfo(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *dchns, Lattice * hltc, unsigned char** buf) {

    LattType lattItm;
    lattItm.nui = 0;
    printf("Response: info");
    size_t bcnt = arr_b;
    u_long itmID;
    size_t arrlen = 0;
    uint respsz;
    uint i;
    LattObj objeid;
    InfoFunc* funarr;
    LattStruct lattStruct;


    lattStruct.dirChains =  *dchns;
    lattStruct.lattice = *hltc;

    char* dnhshstr;
    dnhshstr = yield_dnhstr(&(*dchns)->vessel);

    (lattStruct.fiTbl) =
            ((yield_bridge(*hltc,       // HashLattice* hashlattice
                           (unsigned char*) dnhshstr,  // uchar* filename
                           HASHSTRLEN,                         // uint namelength
                           (*dchns)->vessel))          // Dir_Node* residentDirnode
                    ->fitable);

    funarr = gen_infofunc_arr();

    // Extract the desired object ID from the first byte of the request array,
    // and the array content, which can be an ID, etc. depending on the subject.
    //
    // Fail if no code found

    memcpy(&lattItm.obj,((*inf_frm)->arr),ltyp_s);

    //TODO: Fail on malformed request

    if (lattItm.obj>FIDE || (lattItm.nui & (lattItm.nui-1)) ){
        stsErno(MALREQ,sts_frm,errno,lattItm.nui,"Invalid object ID provided.","response::info","object id value");
        return 1;
        //TODO: Better error value for failed response processing.
    }

    // query requested object and return response size in bytes.

    i = __builtin_ctz(lattItm.nui)+1;   // Info Req subject index = count of its values trailing zeros.
    if (i > 18){
        respsz = 1;
    }else {
        // Extract identifier which would be in the request array at position array start+8
        memcpy(&itmID,(*inf_frm)->arr+(rspsz_b),(rspsz_b));
        (lattStruct.itmID) = &itmID;
        respsz = ((*funarr)[i](buf, lattItm, lattStruct));

        sodium_free(dnhshstr);
        free(funarr);
    }

    // return of 0 means the object is there but failed to be queried.
    // return of 1 means the object is tmissing entirely.
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

unsigned int rsp_sts(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *dchns, Lattice * hltc, unsigned char **buf) {
    printf("Response: Status frame");

}

unsigned int rsp_und(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *dchns, Lattice * hltc, unsigned char **buf) {
    printf("Response: Undefined");
}

unsigned int rsp_fiid(StatFrame **sts_frm, InfoFrame **inf_frm, DChains*dchns, Lattice* hltc, unsigned char **buf) {
    printf("Response: File ID");
}

unsigned int rsp_diid(StatFrame **sts_frm, InfoFrame **inf_frm, DChains*dchns, Lattice* hltc, unsigned char **buf) {
    printf("Response: Dir ID");
}

unsigned int rsp_frdn(StatFrame **sts_frm, InfoFrame **inf_frm, DChains*dchns, Lattice* hltc, unsigned char **buf) {
    printf("Response: Resident Dir");
}

unsigned int rsp_gond(StatFrame **sts_frm, InfoFrame **inf_frm, DChains*dchns, Lattice* hltc, unsigned char **buf) {
    printf("Response: Goto node");
}

unsigned int rsp_fyld(StatFrame **sts_frm, InfoFrame **inf_frm, DChains*dchns, Lattice* hltc, unsigned char **buf) {
    printf("Response: Yield object");}

unsigned int rsp_jjjj(StatFrame **sts_frm, InfoFrame **inf_frm, DChains*dchns, Lattice* hltc, unsigned char **buf) {
    printf("Response: Empty");
}

unsigned int rsp_dsch(StatFrame **sts_frm, InfoFrame **inf_frm, DChains*dchns, Lattice* hltc, unsigned char **buf) {
    printf("Response: Search for object");
}

unsigned int rsp_iiii(StatFrame **sts_frm, InfoFrame **inf_frm, DChains*dchns, Lattice* hltc, unsigned char **buf) {
    printf("Response: Empty");
}

unsigned int rsp_dcls(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *dchns, Lattice *hltc, unsigned char **buf) {
    printf("Response: List chain nodes");
}

unsigned int rsp_gohd(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *dchns, Lattice *hltc, unsigned char **buf) {
    printf("Response: Go to chain head");
}

unsigned int rsp_dnls(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *dchns, Lattice *hltc, unsigned char **buf) {
    printf("Response: List dir ");
}

unsigned int rsp_vvvv(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *dchns, Lattice *hltc, unsigned char **buf) {
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
    unsigned int  (*und)(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *dchns, Lattice *hltc, unsigned char **buf);
    unsigned int (*err)(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *dchns, Lattice *hltc, unsigned char**buf);
    unsigned int (*nfo)(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *dchns, Lattice *hltc, unsigned char**buf);
    unsigned int (*sts)(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *dchns, Lattice *hltc, unsigned char**buf);
//Travel*
    unsigned int (*fiid)(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *dchns, Lattice *hltc, unsigned char **buf);
    unsigned int (*diid)(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *dchns, Lattice *hltc, unsigned char **buf);
    unsigned int (*frdn)(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *dchns, Lattice *hltc, unsigned char **buf);
    unsigned int (*gond)(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *dchns, Lattice *hltc, unsigned char **buf);
//Dir*
    unsigned int (*fyld)(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *dchns, Lattice *hltc, unsigned char **buf);
    unsigned int (*jjjj)(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *dchns, Lattice *hltc, unsigned char **buf);
    unsigned int (*dsch)(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *dchns, Lattice *hltc, unsigned char **buf);
    unsigned int (*iiii)(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *dchns, Lattice *hltc, unsigned char **buf);
//File*
    unsigned int (*dcls)(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *dchns, Lattice *hltc, unsigned char **buf);
    unsigned int (*gohd)(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *dchns, Lattice *hltc, unsigned char **buf);
    unsigned int (*dnls)(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *dchns, Lattice *hltc, unsigned char **buf);
    unsigned int (*vvvv)(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *dchns, Lattice *hltc, unsigned char **buf);

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
    unsigned int *rsp_map;
    unsigned int fcnt = RSPARRLEN;
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

    reply.nui = 0; //LattReply to determine response avenues
    unsigned int sys; //Request is a system op


    if ((*inf_frm)->trfidi[0]) {
        reply.rpl = DIRID | DNODE | (((*inf_frm)->cat_pfx)<<2);  //Travel op -> masked w/ DNODE qualifier and bit #1 : 0011
    } else {
        if ((*inf_frm)->trfidi[2]) {
            reply.rpl = reply.rpl | (((*inf_frm)->cat_pfx)<<2);    //Dir Op -> masked with bit #1 : 0001
        } else {
            if (!(*inf_frm)->trfidi[1]) {    //Not a file op.
                if ((*inf_frm)->sys_op) {
                    reply.rpl = OINFO;    // Info op -> masked with bits #3 and #4 : 1100

                } else {
                    sys = 1;    // System op -> unique handling. : 0110/1010
                }
            }else{// File op, bit #1 stays False, and no more than 1 bit set True : 0000
                reply.rpl = ((1<<((*inf_frm)->cat_pfx))&~1);
            }
        }
    }

    if(reply.nui > 15 || reply.ni < 0){
        setErr(sts_frm,MISCLC,reply.rpl);
        serrOut(sts_frm,"Response determination failed.");
        reply.rpl = ERRCD;
    }
    printf("Action: %d\n", reply.nui);
    return reply;
}


/**
 * \ProcessResponse
 */
unsigned int respond(Resp_Tbl *rsp_tbl,
                     StatFrame **sts_frm,
                     InfoFrame **inf_frm,
                     DChains *dchns,
                     Lattice *hltc,
                     unsigned char *rsp_buf) {

    //TODO: Reset infoframe, implement and include.
    LattType lattItm;
    lattItm.nui =  0;

    prpbuf(&rsp_buf,lattItm);
    /** Determine response avenue and return a reply object */
    lattItm = dtrm_rsp(sts_frm,inf_frm,lattItm);  // Reply object returned from 'determine response'

    unsigned int rspsz;
    unsigned int rsparrsz;
    // Update status and return 1 on failure
    // if the value of reply object is > the
    // num of function in the array.
    if (lattItm.nui > rsp_tbl->fcnt) {
        setErr(sts_frm, MISCLC, lattItm.nui);
        serrOut(sts_frm, "LattReply for response processing outside defined functionality.");
        return 1;
    }

    // Insert reply object into buffer at +sizeof(LattTyp) offset
    memcpy(rsp_buf+UISiZ,&lattItm,ltyp_s);

    // Call function at index 'rsp' (the reply object value) from function array.
    rspsz = (*rsp_tbl->rsp_funcarr)[lattItm.rpl](sts_frm, inf_frm, dchns, hltc, &rsp_buf);
    if (!rspsz){
        return 1;
    }
    rspsz += endbuf(&rsp_buf,lattItm,rspsz);

    // Get array len from response sequence
    rsparrsz = rspsz - (4*ltyp_s);

    rsparr_len_set(rsparrsz,&rsp_buf);

    // Update InfoFrame
    (*inf_frm)->arr_len = rsparrsz;
    (*inf_frm)->rsp_size = rspsz;

    // Update status and return 0 on success
    setSts(sts_frm, RESPN, 0);
    return 0;
}

void destroy_cmdstructures(unsigned char *buffer, unsigned char *respbuffer, unsigned char *carr, Resp_Tbl *rsp_tbl) {

    free(buffer);
    free(respbuffer);
    free(carr);

    free(((rsp_tbl)->rsp_funcarr));
    free(rsp_tbl);
}