//
// Created by ujlm on 10/12/23.
//
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include "jlm_random.h"
#include "sodium.h"
#include "chkmk_didmap.h"
#include "fidi_masks.h"
#include "lattice_cmds.h"

#define REQ 0
#define RSP 1
#define INTARR 8
#define CHRARR 16
#define NOARR 0
#define SEQSTORE "sequences"
#define SELFRESET 2147483647
#define CMDCNT 16
#define ARRSIZE 256
#define CLOSEONFAIL 333


int is_init = 0;
//latticeCmd* lttccmd;
//reqFlag reqsarr[13] = {FFF,TTT,DFLT,NARR,GCSQ,VESL,GOTO,LIST,FIID,FINM,INFO,LEAD,END};
//rspFlag rspsarr[13] = {FFFF,NRSP,RARR,RLEN,STAS,CODE,CINT,DDDD,EEEE,GGGG,ZERO,ERRR,DONE};

enum ReqFlag reqFlag;
enum RspFlag rspFlag;

const unsigned long UISiZ = sizeof(unsigned int);
const unsigned long UCSiZ = sizeof(unsigned char);
const unsigned long LTYPsz = sizeof(LattTyps)+sizeof(unsigned int);
const unsigned long ltyp_s = sizeof(LattTyps);
const unsigned int rspszb = sizeof(unsigned int)+sizeof(unsigned int)+sizeof(LattTyps);
const unsigned int rspsz_b = sizeof(LattTyps)+sizeof(LattTyps);
const unsigned int arr_b = sizeof(LattTyps)+sizeof(LattTyps)+sizeof(unsigned int);


//// Secure zero and destroy CMD struct. Returns pointer to NULL.



void destroy_cmdstructures(unsigned char *buffer, unsigned char *respbuffer, unsigned char *carr, unsigned int *iarr, Resp_Tbl *rsp_tbl) {

    free(buffer);
    free(respbuffer);
    free(carr);
    free(iarr);

    free(((rsp_tbl)->rsp_funcarr));
    free(rsp_tbl);
}

unsigned int build_lead(const unsigned int *cmd_flags, unsigned int flg_cnt) {

    unsigned int cmd = LEAD;
    for (int i = 0; i < flg_cnt; i++) {
        cmd = cmd | *(cmd_flags + i);
    }
    return cmd;
}

/**
 * \SerializeCmdSequence
 *     cmd -> arr len -> arr -> end
 *<br>
 * -    For empty array set arr to NULL and arr_len to 0
 * */


unsigned long cnvrt_iarr_to_carr(unsigned int *intin, unsigned int iarr_len, unsigned char **char_buf) {
    int i;
    for (i = 0; i < iarr_len; i++) {
        if (*(*char_buf + (UISiZ * i))) {
            fprintf(stderr, "buff for int->char not empty @ index %d\n", i);
            return 0;
        } else {
            memcpy((*char_buf + (UISiZ * i)), intin + i, UISiZ);
        }
    }
    return (i * UISiZ);
}


/**
 *\HashCMDSequence
 * Hash cmd sequence for hash-table indexing
 *<br>
 *  -   Seq hash generated from flags, arr len, and first arr element
 * */


size_t read_seqs(unsigned char **seq_arr, int cnfdir_fd) {
    struct stat sb;

    int seqstr_fd = openat(cnfdir_fd, SEQSTORE, O_RDONLY);
    if (seqstr_fd == -1) {
        perror("Error opening sequence store\n");
        return -1;
    }

    if (fstat(seqstr_fd, &sb) == -1) {
        perror("Error stat-ing conf file\n");
        return -1;
    }

    *seq_arr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, seqstr_fd, 0);
    if (*seq_arr == MAP_FAILED) {
        perror("Error with mmap\n");
        return -1;
    }

    close(seqstr_fd);
    return sb.st_size;
}


/** Initialize InfoFrame */
InfoFrame *init_info_frm(InfoFrame **info_frm, uniArr **seqArr) {
    *info_frm = (InfoFrame *) malloc(sizeof(InfoFrame));
    unsigned int cat_sffx = 0;
    (*info_frm)->req_size = 0;
    (*info_frm)->rsp_size = 0;
    (*info_frm)->trfidi[0] = 0;
    (*info_frm)->trfidi[1] = 0;
    (*info_frm)->trfidi[2] = 0;
    (*info_frm)->sys_op = 0;
    (*info_frm)->qual = 0;
    (*info_frm)->arr_type = 0;
    (*info_frm)->arr_len = 0;
    (*info_frm)->arr = *seqArr;

    return *info_frm;

}

/**
 * Reset Info Frame, set StatusFrame to error code, set modifier is set to previous status code.
 */
void err_info_frm(InfoFrame *info_frm, StatFrame **stats_frm, LattErr errcode, unsigned int modr) {

    stsReset(stats_frm);
    setErr(stats_frm, errcode, modr);

    info_frm->arr_len = 0;
    info_frm->req_size = 0;
    info_frm->arr_type = 0;
}

/**
 * Reset InfoFrame
 */
void info_frm_rst(InfoFrame **info_frm) {
    unsigned int cat_sffx = 0;
    (*info_frm)->req_size = 0;
    (*info_frm)->rsp_size = 0;
    (*info_frm)->trfidi[0] = 0;
    (*info_frm)->trfidi[1] = 0;
    (*info_frm)->trfidi[2] = 0;
    (*info_frm)->sys_op = 0;
    (*info_frm)->qual = 0;
    (*info_frm)->arr_type = 0;
    (*info_frm)->arr_len = 0;
};


/* * * * * * * * * *
* Request Parsing *
* * * * * * * * * */

/**
 * \SplitCategories
 *  Recursively iterate array of masked integers representing the OR'd value for
 *      the flags in each of the 4 categories defined in ParseLead. Pass each value into the
 *      DivideFlags recursive sub-process.
 *<br><br>
 *<sub>
 *   Note: Variable 'trfidi' is passed in to inform
 *      the process of which of the three exclusive categories are present if any.
 *      <br>(0 = TravelOps, 1 = FileOps, 2 = DirNode Ops)
 *</sub>
 */
unsigned int split_cats(const unsigned int *lead_flags,
                        LttcFlags* flag_list,
                        LttFlg *flg_itr,
                        unsigned int s_itr,
                        unsigned int l_itr,
                        unsigned int cnt,
                        unsigned int trfidi,
                        unsigned int *subflg);

/**
* \DivideFlags
*  Receives each combined flag value from SplitCategories and recursively apply
*      each of the 4 flag values in that category as an AND mask. If the AND operation results true,
*      the flag is appended to an array of ReqFlags point to by 'flag_list'.
**/
unsigned int div_flgs(const unsigned int *lead_flags,
                      LttcFlags* flag_list,
                      LttFlg *flg_itr,
                      unsigned int s_itr,
                      unsigned int l_itr,
                      unsigned int cnt,
                      unsigned int trfidi,
                      unsigned int *subflg) {

    if (s_itr > 3) {
        return cnt;
    }
    if (*(lead_flags + l_itr) & flg_itr->req) {
        ((*flag_list + cnt)->req)  = ((flg_itr)->req);
        ++(cnt);
        if (*subflg==7) {
            (*subflg = s_itr);
        }
    }
    flg_itr->req <<= 1;
    div_flgs(lead_flags, flag_list, flg_itr, ++s_itr, l_itr, cnt, trfidi, subflg);

}

unsigned int split_cats(const unsigned int *lead_flags,
                        LttcFlags* flag_list,
                        LttFlg *flg_itr,
                        unsigned int s_itr,
                        unsigned int l_itr,
                        unsigned int cnt,
                        unsigned int trfidi,
                        unsigned int *subflg) {

    if (l_itr > 5 || (flg_itr->req) >= UUUU) {
        return cnt;
    } else {
         if (l_itr == 2) {
            if (*subflg == 15) {
                *subflg >>= 1;
            }
            if (trfidi) {
                flg_itr->req <<= (4 * trfidi);
            }
        } else if (l_itr == 3) {
            flg_itr->req = SAVE;
        }
        cnt = div_flgs(lead_flags, flag_list, flg_itr, s_itr, l_itr, cnt, trfidi, subflg);
        (++l_itr);
        split_cats(lead_flags, flag_list, flg_itr, 0, l_itr, cnt, trfidi, subflg);

    }
    //TODO: Implement return 0 in default cases, and flgcnt updatted with '+=' rather than assignment.
}

/**
 *\ParseLead
 * Convert request-lead to an array of flags using bit-masking and return the flag count.
 */
unsigned int parse_lead(const unsigned int lead,
                        LttcFlags *flg_list,
                        StatFrame **sts_frm,
                        InfoFrame **inf_frm) {
    unsigned int k = 1920;
    unsigned int i = 0;
    unsigned int flg_suffx = 15;
    unsigned int flgcnt = 0;

    /** Alloc buffer to hold OR'd category flag values
     * and an array for the final parsed flag list.
     * */
    unsigned int *lead_flags = (unsigned int *) calloc(4,UISiZ);    // [ quals | trvl/fiops/dirops/ | sysops | arrsigs ]
    //*flg_list = (ReqFlag *) (calloc(CMDCNT,sizeof(ReqFlag)));

    /** Extract qualifier flags */
    *lead_flags = lead & QUALIFIR;
    (*inf_frm)->qual = *lead_flags;

    /** Extract array op flags*/
    *(lead_flags + 1) = lead & ARRAYOPS;
    (*inf_frm)->arr_type = *(lead_flags + 1);

    /** Extract op flags from one of three possible categories:
     * <br> Travel, File, or DirNode*/
    do {
        *(lead_flags + 2) = (lead & k);
        k <<= 4;
        (*inf_frm)->trfidi[i] = *(lead_flags + 2);
        i++;
        if (i > 2) {
            break;
        }
    } while (!(*(lead_flags + 2)));

    /** Extract system op flags */
    *(lead_flags + 3) = lead & SYSTMOPS;
    (*inf_frm)->sys_op = *(lead_flags + 3);

    /** Call for further processing to divide the OR'd category values into their individual flags  */
    LttFlg flg_itr = (LttFlg) TTT;
    flgcnt = split_cats(lead_flags, flg_list, (&flg_itr), 1, 0, 0, (i - 1), &flg_suffx);

    free(lead_flags);
    //*flg_list = (ReqFlag *) reallocarray(*flg_list, flgcnt, sizeof(ReqFlag));
    (*inf_frm)->cat_pfx = flg_suffx;

    return flgcnt;
}

/**
 *\ParseRequest
 *  Convert char buffer with a request to a CMD Sequence struct
 *  and return an InfoFrame with request metadata
 *  and a pointer to the CMD structure.
 */


InfoFrame *parse_req(const unsigned char *fullreqbuf,
    InfoFrame **infofrm,
    StatFrame **stsfrm,
    LttcFlags rqflgsbuf,
    unsigned int** tmparrbuf,
    unsigned char **carr_buf) {

    unsigned int exit_flg = 1;
    int k = 0;
    unsigned int is_arr = 0; //1 = char, 2 = int
    unsigned int flag;
    unsigned int flgcnt;
    unsigned int end;



    /**
     * Parse request sequence-lead and init CMD struct
     * */
    memcpy(&flag, fullreqbuf, UISiZ);

    flgcnt = parse_lead(flag, &rqflgsbuf, stsfrm, infofrm);

    if (!flgcnt) {
        fprintf(stderr, "Malformed request: Error parsing lead.\n");
        err_info_frm(*infofrm, stsfrm, MALREQ, 'l'); // l = parsing lead
        return *infofrm;
    }

    /**
     * Build CMD struct
     * */
    ((*infofrm)->flags) = (&rqflgsbuf); //Note: *((*cmdseq)->flags)+X to access flags
    (*infofrm)->lead = flag;
    (*infofrm)->flg_cnt = flgcnt;


    /**
     * Check carrier byte
     * */
    if (flag >> 29 != 1) {
        fprintf(stderr, "Malformed request>\n> %d\n", flag);
        err_info_frm(*infofrm, stsfrm, MALREQ, 'l'); //
        return *infofrm;
    }

    /**
     * Check for Int arr
     * */
    if ((*infofrm)->arr_type == INTARR) {
        memcpy(&((*infofrm)->arr_len), (fullreqbuf + UISiZ), UISiZ); // Set InfoFrame -> arr length

        memcpy(&end, (fullreqbuf + (UISiZ * 2) + (UISiZ * (*infofrm)->arr_len)), UISiZ);  // Calc request endpoint

        exit_flg = (*infofrm)->arr_len == 1 ? (exit_flg << 1) : 1; // EXIT trigger 1

        (*infofrm)->req_size = (UISiZ * 3) + (UISiZ * (*infofrm)->arr_len); // Set InfoFrame -> request size

        /**
         * Check tail byte
         * */
        if (end != END) {
            fprintf(stderr, "Malformed request>\n> %d\n> %d\n> %d\n", flag, end, (*infofrm)->arr_len);
            err_info_frm((*infofrm), stsfrm, MALREQ, 't'); // 't' = tail
            return (*infofrm);
        }

        exit_flg = flag == ENDBYTES ? (exit_flg << 1) : 1; //   EXIT trigger 2

        /**
         * Alloc and populate int buffer with the cmd sequence
         * */
        memcpy(*tmparrbuf, fullreqbuf + (UISiZ * 2), (UISiZ * (*infofrm)->arr_len));
        (*infofrm)->arr->iarr = *tmparrbuf;

        exit_flg = **tmparrbuf == SHTDN ? (exit_flg << 1) : 1;    //  EXIT trigger 3

        /**
         * Exit for shutdown upon receiving three shutdown triggers
         * */
        if (exit_flg == 8) {
            setAct(stsfrm, GBYE, SHTDN, SELFRESET);
            return (*infofrm);
        }
    }
        /**
         * Check for char arr
         * */
    else if ((*infofrm)->arr_type == CHRARR) {
        memcpy(&((*infofrm)->arr_len), (fullreqbuf + UISiZ), UISiZ);
        (*infofrm)->arr_len = (*infofrm)->arr_len;
        memcpy(&end, (fullreqbuf + (UISiZ * 2) + (UCSiZ * (*infofrm)->arr_len)), UISiZ); //Calc request endpoint


        (*infofrm)->req_size = (UISiZ * 3) + (UISiZ * (*infofrm)->arr_len); // Set InfoFrame -> fullreqbuf size

        if (end != END) {
            fprintf(stderr, "Malformed request>\n> %d\n> %d\n> %d\n", flag, end,
                    (*infofrm)->arr_len); // Init int arr buffer if iarr follows
            err_info_frm((*infofrm), stsfrm, MALREQ, 0);
            return (*infofrm);
        }


        memcpy(carr_buf, fullreqbuf + (UISiZ * 2), (UCSiZ * (*infofrm)->arr_len));
        (*infofrm)->arr->carr = *carr_buf;

    }


    (*stsfrm)->status <<= 1;

    return (*infofrm);
}


/* * * * * * * * * * * * * *
*  StatusFrame Functions   *
* * * * * * * * * * * * * */

/** Set error */
void setErr(StatFrame **sts_frm, LattErr ltcerr, unsigned int modr) {
    (*sts_frm)->status = STERR;
    (*sts_frm)->err_code = ltcerr;
    if (modr) {
        (*sts_frm)->modr = modr;
    }
}

/** Set status */
void setSts(StatFrame **sts_frm, LattStts ltcst, unsigned int modr) {
    (*sts_frm)->status = ltcst;
    if (modr) {
        (*sts_frm)->modr = modr;
    }
    if (modr == SELFRESET) {
        (*sts_frm)->modr = 0;
    }
}

long stsErno(StatFrame** sts_frm, LattErr ltcerr, char* msg, char* function, int erno, long misc){

    (*sts_frm)->err_code = ltcerr;

    fprintf(stderr, "Error:"
                    "\t[ lttc_err: %d ]\n", (*sts_frm)->err_code);
    fprintf(stderr, "\t[ errno: %d ]\n", (*sts_frm)->status);
    fprintf(stderr, "\n- last status > %d\n", (*sts_frm)->status);
    if ((*sts_frm)->modr){
        fprintf(stderr, "- last modr > %d\n", (*sts_frm)->modr);
    }
    if (function != NULL){
        fprintf(stderr,"- erring function > %s\n", function);
    }
    if (misc){
        fprintf(stderr,"- misc > %ld\n", misc);
    }
    if (msg != NULL) {
        fprintf(stdout, "---------\n\t%s\n---------\n", msg);
    }

    (*sts_frm)->status = STERR;
    (*sts_frm)->modr = erno;

    if ((*sts_frm)->act_id == GBYE || (misc == 333)) {
        (*sts_frm)->status = SHTDN;
        return 1;
    }else
    {
        return 0;
    }


}

/** Set action id*/
void setAct(StatFrame **sts_frm, LattAct lttact, LattStts ltsts, unsigned int modr) {
    (*sts_frm)->act_id = lttact;
    if (ltsts != NOTHN) {
        (*sts_frm)->status = ltsts;
    }
    if (modr) {
        (*sts_frm)->modr = modr;
    }
    if (modr == SELFRESET) {
        (*sts_frm)->modr = 0;
    }
}

/** Set modifier */
void setMdr(StatFrame **sts_frm, unsigned int modr) {
    (*sts_frm)->modr = modr ? modr : ++((*sts_frm)->modr);
    if (modr == SELFRESET) {
        (*sts_frm)->modr = 0;
    }
}

/** Reset StatusFrame fields*/
void stsReset(StatFrame **sts_frm) {
    (*sts_frm)->status = LISTN;
    (*sts_frm)->act_id = ZZZZ;
    (*sts_frm)->err_code = IMFINE;
    (*sts_frm)->modr = 0;
}

/** Output current StatusFrame */
void stsOut(StatFrame **sts_frm) {
    printf("\n--------------\n");
    printf("[ Status: %d ]\n", (*sts_frm)->status);
    if ((*sts_frm)->status == SHTDN) {
        fprintf(stdout, "\n<< GoodBye >>\n");

    }
}

/** Output error code
 *<br>
 *  - Optionally, exit if StatusFrame Act-code is set to GBYE.
 *<br>
 *  - A message string can be passed in for display or pass in NULL
 *  for none;
 * */
void serrOut(StatFrame **sts_frm, char *msg){
    fprintf(stderr, "[ Error Code: %d ]\n", (*sts_frm)->err_code);
    fprintf(stderr, "< %d >\n", (*sts_frm)->modr);

    if ((*sts_frm)->act_id == GBYE) {
        fprintf(stderr, "Shutting down\n");
        (*sts_frm)->status = SHTDN;
    }
    if (msg != NULL) {
        fprintf(stdout, "---------\n\t%s\n---------\n", msg);
    }
}


/* * * * * * * * * *
*  Response CMDS  *
* * * * * * * * **/



unsigned int serialz(unsigned char **buf, LattTyps itm, unsigned int offst){
    unsigned char sz = sizeof(itm);
    memcpy((*buf),&itm,sz);
    return offst+sz;

}

unsigned int prpbuf(unsigned char **buf){
    printf("%s\n: ",(*buf));
    LattTyps lead = (LattTyps) LEAD;
    memcpy(*buf,&lead.flg,ltyp_s);

    return ltyp_s;
}

unsigned int insz(unsigned char **buf,unsigned int sz){
    unsigned int osz;
    unsigned int nsz;
    memcpy(&osz,buf+rspsz_b,UISiZ);
    nsz = osz+sz;
    memcpy(*buf+rspsz_b,&nsz,UISiZ);
    return osz+sz;
}

unsigned int incnt(void* itm, unsigned int cnt, unsigned int mlti){
    return mlti ? ((mlti*sizeof(*itm))+cnt) : sizeof(*itm)+cnt;
}

unsigned int setsz(unsigned int sz, unsigned char **buf){
    memcpy(*buf+rspsz_b,&sz,UISiZ);
    return sz;
}

unsigned int msgmk(unsigned char **buf, unsigned char* msg, unsigned int len, unsigned int offst, StatFrame **sts_frm){

    offst += arr_b;
    if (len+offst > 256 || len < 1){
        setErr(sts_frm,MALREQ,len+offst);
        serrOut(sts_frm,"invalid msg length");
        return 0;
    }
    memcpy(*buf+offst,msg,UCSiZ*len);

    return ((UCSiZ*len)+offst);
}

unsigned int adddone(unsigned char **buf, unsigned int bcnt){

    LattTyps dneflg = (LattTyps) DONE;

    memcpy(*buf+bcnt,&(dneflg.flg),ltyp_s);

    return bcnt+ltyp_s;
}

unsigned int outarr(unsigned char **buf, unsigned int arrlen){

    for (uint i = arr_b; i < arrlen+arr_b; i++ ){

        if (putchar(*(*buf + i) < 0)){
            return errno;
        }
    }
    return 0;
}

    // lead | item | arrsz | arr | DONE

/**
 * <verbatim><code>
 *<br>
 *<br> ltyp_s = sizeof(LattTyps)
 *<br> rspsz_b = 2*sizeof(LattTyps)
 *<br> uint_s = sizeof(unsigned int)
 *<br> uchar_s = sizeof(unsigned char)
 *<br> arr_b = (2*sizeof(LattTyps))+sizeof(uint)
 *<br>
 *<br> >Response string element positions:
 *<br>
 *<br> bytes start - end
 *<br> -----------------
 *<br> lead: 0 - ltyp_s
 *<br> item: ltyp_s - rspsz_b
 *<br> arrsz: rspsz_b - (rspsz_b)+uint_s
 *<br> arr: (rspsz_b)+uint_s - (rspsz_b)+uint_s+(arr_len*uchar_s)
 *<br> DONE: (rspsz_b)+uint_s+(arr_len*uchar_s) - rspsz_b+ltyp_s+uint_s+(arr_len*uchar_s)
 */

unsigned int rsp_err(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *chns, Lattice* hltc, unsigned char **buf) {

    unsigned int ercode = (*sts_frm)->err_code;
    memcpy(buf,&ercode,sizeof(LattErr));

    return sizeof(LattErr);
 }

 unsigned int rsp_nfo(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *dchns, Lattice * hltc, unsigned char **buf) {
    printf("Response: info");
    LattTyps xx = (LattTyps) LTTC;
    size_t bcnt = prpbuf(buf);
    size_t arrlen = 0;
    LattObj i;
    LattObj objs[19] = {NADA, LTTC, BRDG, DIRN, FTBL, FIMP, LFLG, SFRM, IFRM,
                        SEQT, CMSQ, ICAR, VSSL, FIOB, IDID, NMNM, FIDE, DCHN,NADA};

    // Extract the desired subject of information which should be the sole item in the request array.
    // Fail if no code found
    LattObj objeid;
    memcpy(&objeid,(*inf_frm)->arr,(*inf_frm)->arr_len);

    for (i = LTTC; i != BUFF; i<<=1){
        if (i == objeid){
            break;
        }
    }

    if (i>FIDE){
        setErr(sts_frm,MALREQ,objeid);
        serrOut(sts_frm,"Invalid object ID provided.");
        return rsp_err(sts_frm, inf_frm, dchns, hltc, buf);
    }

    switch(objeid) {
       case NADA:

            break;

       case LTTC:
           break;

       case BRDG:
           break;

       case DIRN:
           break;

       case FTBL:
           break;
       case FIMP:
           break;
       case LFLG:
           break;
       case SFRM:
           break;
       case IFRM:
           break;
       case SEQT:
           break;
       case CMSQ:
           break;
       case ICAR:
           break;
       case VSSL:
           break;
       case FIOB:
           break;
       case IDID:
           break;
       case NMNM:
           break;
       case FIDE:
           break;
       case DCHN:
           break;
        default:
            break;
    }



     if (outarr(buf,arrlen)) {
         perror("arrout: ");
     }
    return bcnt;
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

/* * * * * * * * * * * **
*  Response processing  *
* * * * * * * * * * * * */

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
    unsigned int (*err)(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *dchns, Lattice *hltc, unsigned char **buf);
    unsigned int (*nfo)(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *dchns, Lattice *hltc, unsigned char **buf);
    unsigned int (*sts)(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *dchns, Lattice *hltc, unsigned char **buf);
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

/**
 * Init response table
 * */
void init_rsptbl(int cnfg_fd,
                 Resp_Tbl **rsp_tbl,
                 StatFrame **sts_frm,
                 InfoFrame **inf_frm,
                 DChains *dchns,
                 Lattice *hltc,
                 unsigned char *buf) {

    unsigned int *rsp_map;
    unsigned int fcnt = RSPARR;
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
LattReply dtrm_rsp(StatFrame **sts_frm,
                   InfoFrame **inf_frm) {

    unsigned char reply = 0; //LattReply to determine response avenues
    unsigned int sys; //Request is a system op


    if ((*inf_frm)->trfidi[0]) {
        reply = DIRID | DNODE | (((*inf_frm)->cat_pfx)<<2);  //Travel op -> masked w/ DNODE qualifier and bit #1 : 0011

    } else {
        if ((*inf_frm)->trfidi[2]) {
            reply = DIRID | (((*inf_frm)->cat_pfx)<<2);    //Dir Op -> masked with bit #1 : 0001
        } else {
            if (!(*inf_frm)->trfidi[1]) {    //Not a file op.
                if ((*inf_frm)->sys_op) {
                    reply = OINFO;    // Info op -> masked with bits #3 and #4 : 1100
                } else {
                    sys = 1;    // System op -> unique handling. : 0110/1010
                }
            }else{// File op, bit #1 stays False, and no more than 1 bit set True : 0000
                reply = ((1<<((*inf_frm)->cat_pfx))&~1);
            }
        }
    }

    if(reply > 15 || reply < 0){
        setErr(sts_frm,MISCLC,reply);
        serrOut(sts_frm,"Response determination failed.");
        return ERRCD;
    }
    printf("Action: %d\n", reply);
    return reply;
}

/**
 * \ProcessResponse
 */
unsigned int proc_rsp(Resp_Tbl *rsp_tbl,
                        LattReply rsp,
                        StatFrame **sts_frm,
                        InfoFrame **inf_frm,
                        DChains *dchns,
                        Lattice *hltc,
                        unsigned char **buf) {
    unsigned int rspsz;
    unsigned int rsparrsz;
    // Update status and return 1 on failure
    // if the value of reply object is > the
    // num of function in the array.
    if (rsp > rsp_tbl->fcnt) {
        setErr(sts_frm, MISCLC, rsp);
        serrOut(sts_frm, "LattReply for response processing outside defined functionality.");
        return 1;
    }

    // Insert reply object into buffer at +sizeof(LattTyp) offset
    memcpy(*buf+ltyp_s,&rsp,sizeof(LattReply));

    // Call function at index 'rsp' (the reply object value) from function array.
    rspsz = (*rsp_tbl->rsp_funcarr)[rsp](sts_frm, inf_frm, dchns, hltc, buf);

    // Get array len from response sequence
    rsparrsz = rspsz - (3*ltyp_s)-(UCSiZ);

    // Update InfoFrame
    (*inf_frm)->arr_len = rspsz;
    (*inf_frm)->rsp_size = rsparrsz;

    // Update status and return 0 on success
    setSts(sts_frm, RESPN, 0);
    return 0;
}

InfoFrame* respond(Resp_Tbl *rsp_tbl,
                      StatFrame **sts_frm,
                      InfoFrame **inf_frm,
                      DChains *dchns,
                      Lattice *hltc,
                      unsigned char **resp_buf) {

    //TODO: Reset infoframe, implement and include.

    /** Determine response avenue and return a reply object */
    LattReply rsp = dtrm_rsp(sts_frm,inf_frm);  // Reply object returned from 'determine response'
    bzero(*resp_buf,ARRSIZE-1);                 // Zero response buffer


    /** Process response; build response sequence based on results of determination process */
    if (proc_rsp(rsp_tbl,rsp,sts_frm,inf_frm,dchns,hltc,resp_buf)){
        int iern = errno;
        setErr(sts_frm,NOINFO,rsp);
        serrOut(sts_frm,"Resp processing failed");
        (*inf_frm)->qual = iern;
        return *inf_frm;
    }

    fprintf(stdout,"\nres:%u\n",rsp);
    return *inf_frm;
}
