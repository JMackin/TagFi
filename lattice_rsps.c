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
#include "reply_tools.h"



/*
 *  Response element size aliases:
 *--------------------------------

 *
 * Note:
 *      Items are seperated with 4 bytes of two kinds,
 *      one denoting chars to follow and one denoting ints:
 *
 *      int:    '0xdbdbdbdb' (3688618971)
 *      char:   '0xbddbdbbd' (3185302461)
 */

/*
 * [ lead -> reply item -> arrsz -> {arr} -> DONE ]
 *                                 \__-> { LattObj | (sep) | content }
 *
* */


/* * * * * * * * * * * * *
*  RESPONSE CONSTRUCTION *
* * * * * * * * * * * * **/

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
    bcnt = rsp_add_arrobj(lattItm.obj,buf);
    LattLong brdg_id;
    brdg_id.l_ulong_ptr= lattStruct.itmID;

    HashBridge * hshBrdg = yield_bridge_for_fihsh(lattStruct.lattice, *brdg_id.l_ulong_ptr);
    if(hshBrdg == NULL){
        return 1;
    }

    bcnt += rsparr_add_msg(buf, "UID",4,bcnt);
    bcnt+= rsparr_add_chrstr(buf, hshBrdg->unid, (UINT_SZ * 4), bcnt);

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


    memcpy(&lattItm.obj, ((*inf_frm)->arr), LATTTYP_SZ);
    //TODO: Fail on malformed request

    if (lattItm.obj>FIDE || (lattItm.n_uint & (lattItm.n_uint - 1)) ){
        stsErno(MALREQ, sts_frm, "Invalid object ID provided.", lattItm.n_uint,
                "object id value", "response::info", NULL, errno);
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
        stsErno(MISSNG, sts_frm, "Info requested couldn't be retrieved.", 0, NULL, "resp::info", NULL, errno);
        return 0;
    }
    if (respsz == 1) {
        stsErno(UNKNWN, sts_frm, "Info requested for unknown object.", 0, NULL, "resp::info", NULL, errno);
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
    bcnt = rsp_add_arrobj(SFRM,buf);

    bcnt += rsparr_addsep(bcnt, CHAR_SEP, buf);
    bcnt += rsparr_add_msg(buf, "STS", LATTTYP_SZ, bcnt);
    lattitm.sts = (*sts_frm)->status;
    bcnt += rsparr_addsep(bcnt, INT_SEP, buf);
    bcnt += rsparr_add_lt(lattitm,buf,bcnt);

    bcnt += rsparr_addsep(bcnt, CHAR_SEP, buf);
    bcnt += rsparr_add_msg(buf, "ERR", LATTTYP_SZ, bcnt);
    lattitm.err = (*sts_frm)->err_code;
    bcnt += rsparr_addsep(bcnt, INT_SEP, buf);
    bcnt += rsparr_add_lt(lattitm,buf,bcnt);

    bcnt += rsparr_addsep(bcnt, CHAR_SEP, buf);
    bcnt += rsparr_add_msg(buf, "ACT", LATTTYP_SZ, bcnt);
    lattitm.act = (*sts_frm)->act_id;
    bcnt += rsparr_addsep(bcnt, INT_SEP, buf);
    bcnt += rsparr_add_lt(lattitm,buf,bcnt);

    bcnt += rsparr_addsep(bcnt, CHAR_SEP, buf);
    bcnt += rsparr_add_msg(buf, "MOD", LATTTYP_SZ, bcnt);
    lattitm.n_uint = (*sts_frm)->modr;
    bcnt += rsparr_addsep(bcnt, INT_SEP, buf);
    bcnt += rsparr_add_lt(lattitm,buf,bcnt);

    return bcnt;

    //TODO: Test and verify this function
}

uint rsp_und(StatFrame **sts_frm, InfoFrame **inf_frm, Lattice * hltc, buff_arr buf) {


    return 0;
}



uint rsp_fiid(StatFrame **sts_frm, InfoFrame **inf_frm, Lattice * hltc, buff_arr buf) {
    printf("Response: Fiid for hashno");
    LattLong* itmID = malloc(ULONG_SZ);
    LattType lattitm;
    uint bcnt;
    rsplead_addflg(LARR,buf);

    lattitm.obj = pull_arrObj(inf_frm);

    bcnt = rsp_add_arrobj(lattitm.obj,buf);

    itmID->l_ulong = pull_objid(inf_frm, (*itmID), 8).l_ulong;
    if ( itmID->l_ulong == 0){
        stsErno(MALREQ, sts_frm, "Given fiID is invalid or mangled.", itmID->l_ulong, "parsed id", "rsp_fiid",
                NULL, errno);
        return 1;
    }

    HashBridge* hashbridge = (yield_bridge_for_fihsh(*hltc,itmID->l_ulong));

//    Armatr armatr = (*hltc)->chains->vessel->armature;
    if (hashbridge == NULL){
        stsErno(NOINFO, sts_frm, "No bridge found for given fiid", itmID->l_ulong, "fiid", "rsp_fiid", NULL, errno);
        return 1;
    }

    if (lattitm.obj == NMNM){
        stsErno(NOINFO, sts_frm, "Can't produce a filename this way at present.", 0, NULL, "rsp_fiid", NULL, errno);
        return 1;
    } else if (lattitm.obj == IDID){
//        itmID->l_ulong = fiid->entries[getidx(itmID->l_ulong)].fiid;
        itmID->l_ulonglong = hashbridge->finode->fiid;
    } else {
        stsErno(NOINFO, sts_frm, "Invalid itm specified in exchange for fihash.", lattitm.n_uint,
                "misc: resp-itm", "rsp_fiid", NULL, errno);
        return 1;
    }

    bcnt += rsparr_addsep(bcnt, LONG_SEP, buf);
    bcnt += rsparr_add_lng(bcnt,*itmID,buf);

    free(itmID);
    return bcnt;
}

uint rsp_diid(StatFrame **sts_frm, InfoFrame **inf_frm, Lattice * hltc, buff_arr buf) {
    printf("Response: DirID for chain number");

    LattLong* itmID = malloc(sizeof(unsigned long));
    LattLong chainID;
    LattType lattitm;
    LattLong diID;
    TravelPath* travelPath;
    rsplead_addflg(LARR,buf);

    lattitm.obj = DIRN | IDID;
    uint bcnt;
    bcnt = rsp_add_arrobj(lattitm.obj,buf);

    bcnt+= rsparr_addsep(bcnt,LONG_SEP,buf);
    if ((*inf_frm)->qual == 2){
        diID.l_ulonglong = (*hltc)->chains->vessel->did;
        bcnt += rsparr_add_lng(bcnt, diID, buf);
    } else {
        chainID.l_ulong = pull_objid(inf_frm, (*itmID), 8).l_ulong;
        if (chainID.l_ulong == 0){

            stsErno(MALREQ, sts_frm, "Given dirID is invalid or mangled.", itmID->l_ulong,
                    "parsed id", "rsp_diid", NULL, errno);
            return 1;
        }

        if (travel_by_chnid(chainID.l_ulong, (*hltc)->chains, &travelPath)){
            free(travelPath);
            stsErno(NOINFO, sts_frm, "Provided chain ID not found", itmID->l_ulong,
                    "misc - chain id", "rsp_diid", NULL, errno);
            return 1;
        }

        diID.l_ulonglong = (*hltc)->chains->vessel->did;
        bcnt += rsparr_add_lng(bcnt,diID,buf);
        uint ret_status = return_to_origin(travelPath,(*hltc)->chains);
        if(ret_status){
            stsErno(MISVEN, sts_frm, "Return to origin failed", ret_status,
                    "return status", "rsp_diid::return_to_origin", NULL, 0);
        }

        free(travelPath);
    }

    free(itmID);
    return bcnt;
}

uint rsp_frdn(StatFrame **sts_frm, InfoFrame **inf_frm, Lattice * hltc, buff_arr buf) {
    printf("Response: Resident Dir for fihashno");
    LattType lattitm;
    LattLong *fiID =(LattLong*) malloc(ULONG_SZ);
    uint bcnt;

    lattitm.obj = DIRN;
    bcnt = rsp_add_arrobj(lattitm.obj,buf);


    fiID->l_ulong = pull_objid(inf_frm, (*fiID), 8).l_ulong;
    if (fiID->l_ulong == 0){
        stsErno(MALREQ, sts_frm,
                "Given fiID is invalid or mangled.", fiID->l_ulong, "parsed id", "rsp_frdn", NULL, errno);return 1;}

    HashBridge* hashbridge = (yield_bridge_for_fihsh(*hltc,fiID->l_ulong));
    if (hashbridge == NULL){
        stsErno(BADHSH, sts_frm,
                "Hashing did not yield a bridge for given fihashno.", fiID->l_ulong, "misc: fiID",
                "rsp_frdn/yield_bridge_for_fihsh", NULL, errno);return 1;}

    lattitm.obj = pull_arrObj(inf_frm);

    if (lattitm.obj == NMNM){
        rsplead_addflg(CARR,buf);
        bcnt += rsparr_addsep(bcnt,CHAR_SEP,buf);
        bcnt += rsparr_add_msg(buf, (char*) hashbridge->dirnode->diname, expo_dirnmlen(hashbridge->dirnode->did), ULONG_SZ);

    } else if (lattitm.obj == IDID){
        rsplead_addflg(LARR,buf);
        bcnt += rsparr_addsep(bcnt,LONG_SEP,buf);
        bcnt += rsparr_add_lnglng(bcnt,hashbridge->dirnode->did,buf);

    } else {
        free(fiID->l_ulong_ptr);
        stsErno(NOINFO, sts_frm, "Invalid itm specified in exchange for fihash.", lattitm.n_uint,
                "misc: resp-itm", "rsp_frdn", NULL, errno);
        return 1;
    }

    return bcnt;

}

uint rsp_gond(StatFrame **sts_frm, InfoFrame **inf_frm, Lattice * hltc, buff_arr buf) {
    printf("Response: Goto node");
    LattType lattitm;
    uint bcnt;
    LattLong *dnode = malloc(sizeof(LattLong));
    TravelPath* travelpath;

    lattitm.obj = VSSL;
    bcnt = rsp_add_arrobj(lattitm.obj,buf);

    lattitm.obj = pull_arrObj(inf_frm);

    if (lattitm.obj == DCHN){
        dnode->l_ulong = pull_objid(inf_frm, *dnode, ULONG_SZ).l_ulong;
        if (travel_by_chnid(dnode->l_ulong, (*hltc)->chains, &travelpath)){
            free(travelpath);
            stsErno(MISVEN, sts_frm, "Travel by chain ID failed", dnode->l_ulong, "dnode::chain id", "rsp_gond",
                    NULL, errno);
            return 1;
        }
        bcnt += rsparr_add_travelpath(travelpath,buf,bcnt);
    }else if (lattitm.obj == IDID){
        dnode->l_ulonglong = pull_objid(inf_frm,(*dnode),8).l_ulonglong;
        if(travel_by_diid(dnode->l_ulonglong, (*hltc)->chains, &travelpath)==1){
            free(travelpath);
            stsErno(MISVEN, sts_frm, "Travel by diid failed", dnode->l_ulonglong, "dnode::diid", "rsp_gond",
                    NULL, errno);
            return 1;
        }
        bcnt += rsparr_add_travelpath(travelpath,buf,bcnt);
    }

    return bcnt;

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
    uint (*und)(StatFrame **sts_frm, InfoFrame **inf_frm, Lattice *hltc, buff_arr buf);
    uint (*err)(StatFrame **sts_frm, InfoFrame **inf_frm, Lattice *hltc, buff_arr buf);
    uint (*nfo)(StatFrame **sts_frm, InfoFrame **inf_frm, Lattice *hltc, buff_arr buf);
    uint (*sts)(StatFrame **sts_frm, InfoFrame **inf_frm, Lattice *hltc, buff_arr buf);
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


    und = &rsp_und; err = &rsp_err; nfo = &rsp_nfo; dnls = &rsp_dnls;
    sts = &rsp_sts; fiid = &rsp_fiid; diid = &rsp_diid; vvvv = &rsp_vvvv;
    frdn = &rsp_frdn; gond = &rsp_gond; fyld = &rsp_fyld; jjjj = &rsp_jjjj;
    dsch = &rsp_dsch; iiii = &rsp_iiii; dcls = &rsp_dcls; gohd = &rsp_gohd;


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
                     SttsFrm *sts_frm,
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
    printf("%u",(*sts_frm)->err_code);

    uint rspsz=12;
    // Update status and return 1 on failure
    // if the value of reply object is > the
    // num of function in the array.
    if (lattItm.n_uint > rsp_tbl->fcnt) {
        setErr(sts_frm, MISCLC, lattItm.n_uint);
        serrOut(sts_frm, "LattReply for response processing outside defined functionality.");
        return 1;
    }

    // Insert reply item into buffer at +sizeof(LattTyp) offset
    rsp_add_replyitm(&rsp_buf,lattItm.rpl);
    // Call function at index 'rsp' (the reply object value) from function array.
    rspsz += (*rsp_tbl->rsp_funcarr)[lattItm.rpl](sts_frm, inf_frm, hltc, &rsp_buf);
    if (!rspsz){
        return 1;
    }


    rspsz += endbuf(&rsp_buf,lattItm,rspsz);

    // Get array len from response sequence


    // Update InfoFrame
    (*inf_frm)->arr_len = rspsz-12;
    (*inf_frm)->rsp_size = rspsz;
    rsparr_len_set((*inf_frm)->arr_len,&rsp_buf);


    // Update status and return 0 on success
    setSts(sts_frm, RESPN, 0);
    return 0;
}

