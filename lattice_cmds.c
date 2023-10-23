//
// Created by ujlm on 10/12/23.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include "jlm_random.h"
#include "chkmk_didmap.h"
#include "fidi_masks.h"
#include "lattice_cmds.h"

#define REQ 0
#define RSP 1
#define INTARR 2
#define CHRARR 1
#define NOARR 0
#define SEQSTORE "sequences"
#define crypto_shorthash_KEYBYTES 16u
#define SELFRESET 2147483647
#define CMDCNT 14


int is_init = 0;
//latticeCmd* lttccmd;
//reqFlag reqsarr[13] = {FFF,TTT,DFLT,NARR,GCSQ,VESL,GOTO,LIST,FIID,FINM,INFO,LEAD,END};
//rspFlag rspsarr[13] = {FFFF,NRSP,RARR,RLEN,STAS,CODE,CINT,DDDD,EEEE,GGGG,ZERO,ERRR,DONE};

enum ReqFlag reqFlag;
enum RspFlag rspFlag;

unsigned long UISiZ  = sizeof(unsigned int);
unsigned long UCSiZ = sizeof(unsigned char);

void init_cmdseq(Cmd_Seq** cmdSeq, unsigned int arrsize, unsigned int type){
    *cmdSeq = malloc(sizeof(Cmd_Seq));
    (*cmdSeq)->arr = (uniArr*) calloc(arrsize,UCSiZ);
    (*cmdSeq)->flags = (LttcFlags*) calloc(sizeof(LttcFlags), sizeof(UISiZ));
    (*cmdSeq)->arr_len = 0;
    (*cmdSeq)->seq_id = 0;
    (*cmdSeq)->flg_cnt = 0;
    (*cmdSeq)->type = type;
}

void reset_cmdseq(Cmd_Seq** cmdSeq, unsigned int arrsize, unsigned int type){

}
//
//void init_lttccmd(){
//    if (is_init){
//        return;
//    }
//    lttccmd = (latticeCmd*) malloc(sizeof(latticeCmd));
//    lttccmd->rsps = malloc(sizeof(rspsarr));
//    lttccmd->reqs = malloc(sizeof(reqsarr));
//    lttccmd->rsps = rspsarr;
//    lttccmd->reqs = reqsarr;
//    lttccmd->n_req = 12;
//    is_init = 1;
//}
//
//void destroy_lttccmd(){
//
//    if (!is_init && lttccmd != NULL) {
//        free(lttccmd->reqs);
//        free(lttccmd->rsps);
//        free(lttccmd);
//    }
//    is_init = 0;
//}

void destroy_cmdstructures(unsigned char* buffer, unsigned char* respbuffer, unsigned char* carr, unsigned int* iarr, Seq_Tbl* seqTbl){

    free(buffer);
    free(respbuffer);
    free(carr);
    free(iarr);
    for (int i = 0; i < seqTbl->cnt; i++){
        free((seqTbl)->seq_map+i);
        free((*(seqTbl)->seq_map->cmd_seq+i)->arr);
        free((*seqTbl->seq_map->cmd_seq+i)->flags);
        free(*seqTbl->seq_map->cmd_seq+i);
    }
    free(seqTbl->cmd_key);
    free(seqTbl);
}


/*
 * Iterate through cmds, an array of cmd sequences, for each sequence unmask the leading 4-bytes
 * to parse info about the given command(s).
 * Split cmds into an array of sequence heads, cmds, char arrays, info codes and item counts
 * Cmds can be max of 8 counts of 32 bytes
 */
/*
int unmask_cmds(unsigned int** cmds,
                unsigned int** leads,
                unsigned int** lens,
                unsigned char*** seqs,
                unsigned char*** arrs,
                int cmdcnt){

    int i = 0;
//        *(leads+i) = (unsigned int*) calloc(6, sizeof(unsigned int));
//        *(lens+i) = (unsigned int*) calloc(5, sizeof(unsigned int));
//        *(*seqs+i) = (unsigned char*) calloc(32, sizeof(unsigned char));
//        *(*arrs+i) = (unsigned char*) calloc(32, sizeof(unsigned char));

    int k = 0; // Corr ID
    int cl = 0; //Tracks positions of count cmds following NARR or GCSQ
    int len_flag = 0; // Flag to prevent repeat len marks. 1 for set 2 for NARR, 4 for GCSQ, 6 for both.
    int dir_flag = 0; // Flag to indicate directory cmds were issued;
    int fiop_flag = 0; // Flag if file cmds are issued
    int info_flag = 0; // Flag to indicate info code request in next byte
    int arr_flag[2] = {0};  // Flag to indicate an array follows. 1 = set, 2 = singular value, >2 = lengths. Precendence: Lens->CMD->Arr
    int chk_default = 0; // Flag to indicate the next bits meaning given a following zero byte. 1 = to be checked, 2 = verified.
    int yield_flag = 0; //set when FIID is set to switch FINM from yeilding a file to returning a filename when given a fiid
    //int masked = MASK; //Bit to signal a masked byte, or to end seq if no other bytes


    // Every cmd sequence must have a corresponding masked-byte cmd, even if empty (i.e. == DFLT)
    for (i = 0; i < cmdcnt; i++) {
        len_flag = 0; dir_flag = 0; info_flag = 0; fiop_flag = 0; yield_flag=0;
        if (chk_default > 12){
            chk_default = 0;
        }

        k = 0; cl = 0;
//        //next cmd
//        (*leads+i)[0] = MASK;

        // Handle masked cmds
        do{
            if (((*(*cmds+i)+k) & DFLT)){
                if (chk_default){
                    (*leads+i)[2] = chk_default;
                    chk_default<<3;
                }
            }

            // Mark cmd position if a length bit is set
            // 4 = precedes array len param, 2 = precedes cmd seq param, 6 = both
            if (((*(*cmds+i)+k) & NARR)) {
                len_flag = len_flag | 4;
                arr_flag[1] = 1;
            }
            if (((*(*cmds+i)+k) & GCSQ)) {
                len_flag = len_flag | 2;
                arr_flag[0] = 1;
            }
            if (len_flag) {
                (*lens + cl)[0] = k;
                (*leads+i)[4] = len_flag > 0 ? 1 : 0;
                (*lens + cl)[1] = len_flag;
            }
            // Set flag if directory cmds are masked
            // 1 = Print_CWD, 2 = goto node, next seq is filepath, 4 = list dir contents
            if (((*(*cmds+i)+k) & LIST)){
                dir_flag = dir_flag | 4;
                chk_default = chk_default | 4;
            }
            if (((*(*cmds+i)+k) & GOTO)){
                dir_flag = dir_flag | 2;
                chk_default = chk_default | 2;
            }
            if (((*(*cmds+i)+k) & VESL)){
                dir_flag = dir_flag | 1;
                chk_default = chk_default ? 1 : chk_default;
            }

            // Set flag if file cmds are masked
            //  1 = get file id, 2 = proffer file, 4 both
            if (((*(*cmds+i)+k) & FIID)) {
                fiop_flag = fiop_flag | 2;
                chk_default = chk_default ? chk_default : 8;
                yield_flag = 1;
            }
            if (((*(*cmds+i)+k) & FINM)) {
                fiop_flag = fiop_flag | 1;
                yield_flag = yield_flag ? 2 : 0;
            }
            if (((*(*cmds+i)+k) & INFO)){
                if (!len_flag){
                    info_flag = 1;
                }
            }

            (*leads+i)[2] = fiop_flag ? fiop_flag : dir_flag;

//            *(*outp+i) = *(*outp+i) | *(*(*cmds+i)+k);
//            printf(">> %d",*(*(*cmds+i)+k));
//            (*(*(*cmds+i)+k) =

            printf(">>%d\n",(*(*cmds+i)+k));
            k++;

        }while(!((*(*cmds+i)+k)^((*(*cmds+i)+k) & END)) && k < 16);


//        sodium_bin2hex(*outp,32,(cmds+i),(cmdcnt*sizeof(int)));
    }
}
*/

unsigned int build_lead(const unsigned int* cmd_flags, unsigned int flg_cnt) {

    unsigned int cmd = LEAD;
    for (int i=0;i<flg_cnt;i++){
        cmd = cmd | *(cmd_flags+i);
    }
    return cmd;
}

/**
 * \SerializeCmdSequence
 *<br>
 * -    cmd -> arr len -> arr -> end
 *<br>
 * -    For empty array set arr to NULL and arr_len to 0
 * */
unsigned int serial_seq(unsigned char* seq_out,
                        Cmd_Seq** cmd_seq){

    unsigned int flgcnt = ((*cmd_seq)->flg_cnt);
    unsigned int flg;
    unsigned int byte_cnt;

    byte_cnt = (UISiZ * 3) + (UCSiZ*((*cmd_seq)->arr_len));

    flg = build_lead(((unsigned int*) (*cmd_seq)->flags),flgcnt);

    memcpy((seq_out),&flg,UISiZ);
    memcpy((seq_out+UISiZ),&((*cmd_seq)->arr_len),UCSiZ);

    if (((*cmd_seq)->arr_len) && ((*cmd_seq)->arr) != NULL) {
        memcpy(seq_out+(2*UISiZ), ((*cmd_seq)->arr), ((*cmd_seq)->arr_len)*UCSiZ);
    }

    flg = END;
    memcpy(seq_out+(2*UISiZ)+(UCSiZ*((*cmd_seq)->arr_len)),&flg,UISiZ);
    free(*cmd_seq);

    return byte_cnt;
}


unsigned int init_seqtbl(Seq_Tbl** seq_tbl, unsigned long mx_sz) {
    *seq_tbl = (Seq_Tbl*) malloc(sizeof(Seq_Tbl));
    ((*seq_tbl)->seq_map) = (SeqMap*) calloc(mx_sz,sizeof(SeqMap));
    ((*seq_tbl)->cmd_key) = (unsigned char*) malloc(crypto_shorthash_KEYBYTES);
    (*seq_tbl)->mx_sz = mx_sz;
    mk_little_hash_key(&(*seq_tbl)->cmd_key);

    return sizeof(seq_tbl);
}


unsigned long cnvrt_iarr_to_carr(unsigned int* intin, unsigned int iarr_len, unsigned char** char_buf){
    int i;
    for (i =0; i < iarr_len; i++){
        if (*(*char_buf+(UISiZ*i))){
            fprintf(stderr,"buff for int->char not empty @ index %d\n",i);
            return 0;
        }
        else {
            memcpy((*char_buf+(UISiZ*i)),intin+i,UISiZ);
        }
    }
    return (i*UISiZ);
}


/**
 *\HashCMDSequence
 * Hash cmd sequence for hash-table indexing
 *<br>
 *  -   Seq hash generated from flags, arr len, and first arr element
 * */
unsigned long mk_seq_hash(Cmd_Seq** cmdSeq, Seq_Tbl** seqTbl) {

    unsigned int posi = 0;
    unsigned int posi_tmp = 0;
    unsigned long long seq_idx = 0;

    unsigned int sb_len = (2 * UISiZ) + ((*cmdSeq)->flg_cnt * UISiZ);

    unsigned char *seq_buf = (unsigned char *) calloc(sb_len, UCSiZ);

    bzero(seq_buf, sb_len);

    posi_tmp = cnvrt_iarr_to_carr(&(*cmdSeq)->arr_len, 1, &seq_buf);
    if (posi_tmp == 0) {
        free(seq_buf);
        return 0;
    } else {
        posi += posi_tmp;
    }
    posi_tmp = cnvrt_iarr_to_carr((unsigned int *) &(*cmdSeq)->arr[0], 1, &seq_buf);
    if (posi_tmp == 0) {
        free(seq_buf);
        return 0;
    } else {
        posi += posi_tmp;
    }
    posi_tmp = cnvrt_iarr_to_carr((unsigned int *) &(*cmdSeq)->flags + posi,(*cmdSeq)->flg_cnt, &seq_buf);
    if (posi_tmp == 0) {
        free(seq_buf);
        return 0;
    } else {
        posi += posi_tmp;
    }

    if (sb_len != posi) {
        fprintf(stderr, "Final seq convert length mismatch: \ncalcd: %d\n actual: %d\n", sb_len, posi);
        return 0;
    }

    seq_idx = little_hsh_llidx((*seqTbl)->cmd_key, seq_buf, sb_len, 0);
    if (seq_idx == 1 || seq_idx == 0) {
        fprintf(stderr, "cmd_seq indexing failed");
        return 0;
    } else {
        bzero(seq_buf, sb_len);
        return seq_idx;
    }
}


//FREE seq_buf
unsigned long save_seq(Cmd_Seq* cmd_seq, Seq_Tbl** seq_tbl, int cnfdir_fd){

    int seqstr_fd = openat(cnfdir_fd,SEQSTORE, O_APPEND|O_RDWR);
    if (seqstr_fd == -1) {
        perror("Error opening sequence store\n");
        return -1;
    }

    unsigned int smapcnt = (*seq_tbl)->cnt;

    cmd_seq->seq_id =  mk_seq_hash(&cmd_seq,seq_tbl);
    (((*seq_tbl)->seq_map+smapcnt)->seq_id) = cmd_seq->seq_id;
    ((*seq_tbl)->seq_map+smapcnt)->cmd_seq = &cmd_seq;
    (*seq_tbl)->cnt++;

    printf("%lu: seq_tbl\n",(unsigned long) (((*seq_tbl)->seq_map+smapcnt)->seq_id));
    printf("%lu: cmd_seq\n",(unsigned long) (*((*seq_tbl)->seq_map+smapcnt)->cmd_seq)->seq_id );

    close(seqstr_fd);

    return cmd_seq->seq_id;

}


unsigned int* flagcnt(ReqFlag** flags_branch, unsigned int* itr, unsigned int* cntr, unsigned int* flg, unsigned int** flag_list){

    if(((**(flags_branch)+(*itr)) << 1) & (*flg)){
        *(flag_list + *itr) = flg;
        cntr++;
    }
    if (*itr > 2){
        return cntr;
    }

    return flagcnt(flags_branch, (++itr), cntr, flg,flag_list);
}

unsigned int cntflags( ReqFlag lead_flags[4], ReqFlag *flag_list){

    unsigned int itr = 0;
    int idx= 0;

    unsigned int cntr = 0;
    unsigned int flg = 128;

    while(!flagcnt((&lead_flags+(idx++)), &itr, &cntr, &flg, &flag_list)){
         if (idx > 3 || itr > 20)
        {
            return 0;
        }
         else {
             idx++;
         }
    }
    return idx;

    printf("FLGCNT <<%d>>\n", idx);
}


size_t read_seqs(unsigned char** seq_arr, int cnfdir_fd){
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


//FREE: flag_list
unsigned int parse_lead(const unsigned int lead, ReqArr* flag_list, StatFrame** sts_frm){
    unsigned int flg_cnt = 0;
    unsigned int lslicr = 0;
    ReqFlag i;
    unsigned int k = 120;
    unsigned int j = 1;
    unsigned int l;

    // [ quals | trvl/fiops/dirops/ | sysops | arrsigs ]
    ReqFlag lead_flags[4] = {0,0,0,0};

    ReqFlag flg;
    *flag_list = (ReqArr) calloc(CMDCNT, UISiZ);

    lead_flags[0] = lead & QUALIFIR;
    lead_flags[1] = lead & ARRAYOPS;
    lead_flags[2] = lead & SYSTMOPS;

    while (!lead_flags[3]) {
        lead_flags[3] = (lead & k);
        k <<= 4;
        j++;
        if (j > 4) {
            break;
        }
    }

    for (l = 0; l < 4; l++) {
        flg_cnt += cntflags(lead_flags, *flag_list);
    }
    return flg_cnt;
//
//    for (l = 0; l < 4; l++) {
//
//        for (i = TTT; i < LEAD; i <<= 1) {
//
//        }
//    }

//
//    if (flg_cnt != 0 && flg_cnt < CMDCNT){
//        *flag_list = reallocarray(*flag_list,flg_cnt,UISiZ);
//        return flg_cnt;
//    }else {
//        free(flag_list);
//        setErr(sts_frm, MISCLC, 'p'); // p = parsing lead
//        return 0;
//    }
}


InfoFrame* init_info_frm(InfoFrame** info_frm){
    *info_frm = (InfoFrame*) malloc(sizeof(InfoFrame));
    (*info_frm)->arr_len=0;
    (*info_frm)->req_size=0;
    (*info_frm)->rsp_size=0;
    (*info_frm)->cmdSeq=NULL;

    return *info_frm;

}

/**
 * StatusFrame error code is set, modifier is set to previous status code.
 */
void err_info_frm(InfoFrame* info_frm, StatFrame** stats_frm, LattErr errcode, unsigned int modr){

    stsReset(stats_frm);
    setErr(stats_frm,errcode,modr);

    info_frm->arr_len = 0;
    info_frm->req_size = 0;
    info_frm->arr_type = 0;
    info_frm->cmdSeq = NULL;
}


//FREE: rinfo, cmd_seq
InfoFrame* parse_req(const unsigned char* req,
                     Cmd_Seq** cmd_seq,
                     InfoFrame* rinfo,
                     StatFrame** sts_frm){


    unsigned int exit_flg = 1;
    int k = 0;
    unsigned int is_arr = 0; //1 = char, 2 = int
    unsigned int arrlen;
    unsigned int flag;
    unsigned int flgcnt = 0;
    unsigned int end;
    unsigned char* carr_buf;
    unsigned int* iarr_buf;

    memcpy(&flag,req,UISiZ);
    ReqArr req_arr;

    flgcnt = parse_lead(flag, &req_arr, sts_frm); //Lead -> flag arr. Returns flag count
    init_cmdseq(cmd_seq,flgcnt,REQ);

    if (!flgcnt){
        fprintf(stderr,"Malformed request: Error parsing lead.\n");
        err_info_frm(rinfo,sts_frm,MALREQ,'l'); // l = parsing lead
        return rinfo;
    }
    (*cmd_seq)->flags = (LttcFlags *) req_arr;
    (*cmd_seq)->lead = flag;
    (*cmd_seq)->flg_cnt = flgcnt;

    if ( flag>>29 != 1) //Check carrier byte
    {
        fprintf(stderr,"Malformed request>\n> %d\n",flag);
        err_info_frm(rinfo,sts_frm,MALREQ,'l'); //
        return rinfo;
    }

    if (flag & NARR & END) {    //Check for Int arr
        memcpy(&(rinfo->arr_len),(req+UISiZ),UISiZ);
        rinfo->arr_type=INTARR;
        (*cmd_seq)->arr_len = rinfo->arr_len;
        memcpy(&end,(req+(UISiZ*2)+(UISiZ*rinfo->arr_len)),UISiZ);  //Calc request endpoint

        exit_flg = rinfo->arr_len == 1 ? (exit_flg << 1 ) : 1; // EXIT 1

        rinfo->req_size = (UISiZ*3)+(UISiZ*rinfo->arr_len);

        if (end != END) {   //Check tail byte
            fprintf(stderr,"Malformed request>\n> %d\n> %d\n> %d\n",flag,end,rinfo->arr_len);
            err_info_frm(rinfo,sts_frm,MALREQ,'t'); // 't' = tail
            return rinfo;
        }

        exit_flg = flag == ENDBYTES ? (exit_flg << 1) : 1; //   EXIT 2


        iarr_buf = (unsigned int*) calloc(rinfo->arr_len,UISiZ);    // Init int arr buffer if iarr follows. Tobe FREEd
        memcpy(iarr_buf,req+(UISiZ*2),(UISiZ*rinfo->arr_len));
        (*cmd_seq)->arr->iarr = iarr_buf;

        /*Exit for shutdown upon receiving three shutdown triggers*/
        exit_flg = *iarr_buf == SHTDN ? (exit_flg << 1) : 1;    //  EXIT 3

        if (exit_flg == 8) {

            setAct(sts_frm,GBYE,SHTDN,SELFRESET);
            return rinfo;
        }
    }

    else if(flag & GCSQ & END) {
        memcpy(&(rinfo->arr_len),(req+UISiZ),UISiZ);
        rinfo->arr_type=CHRARR;
        (*cmd_seq)->arr_len = rinfo->arr_len;
        memcpy(&end,(req+(UISiZ*2)+(UCSiZ*rinfo->arr_len)),UISiZ); //Calc request endpoint

        rinfo->req_size = (UISiZ*3)+(UISiZ*rinfo->arr_len);

        if (end != END) {
            fprintf(stderr,"Malformed request>\n> %d\n> %d\n> %d\n",flag,end,rinfo->arr_len); // Init int arr buffer if iarr follows
            err_info_frm(rinfo,sts_frm,MALREQ,0);
            return rinfo;
        }
        carr_buf = (unsigned char*) calloc(rinfo->arr_len,UCSiZ); // Init char arr buffer if carr follows
        memcpy(carr_buf,req+(UISiZ*2),(UCSiZ*rinfo->arr_len));
        (*cmd_seq)->arr->carr = carr_buf;
    }

    (rinfo->cmdSeq) = *cmd_seq;
    (*sts_frm)->status <<= 1;

    return rinfo;
}


void setErr(StatFrame** sts_frm, LattErr ltcerr, unsigned int modr){
    (*sts_frm)->status = STERR;
    (*sts_frm)->err_code = ltcerr;
    if (modr){
        (*sts_frm)->modr = modr;
    }
}

void setSts(StatFrame** sts_frm, LattStts ltcst, unsigned int modr){
    (*sts_frm)->status = ltcst;
    if (modr){
        (*sts_frm)->modr = modr;
    }
    if (modr == SELFRESET) {
        (*sts_frm)->modr = 0;
    }
}

void setAct(StatFrame** sts_frm, LattAct lttact, LattStts ltsts, unsigned int modr)
{
    (*sts_frm)->act_id = lttact;
    if(ltsts != NOTHN){
        (*sts_frm)->status = ltsts;
    }
    if (modr) {
        (*sts_frm)->modr = modr;
    }
    if (modr == SELFRESET) {
        (*sts_frm)->modr = 0;
    }
}

void setMdr(StatFrame** sts_frm, unsigned int modr){
    (*sts_frm)->modr = modr ? modr : ++((*sts_frm)->modr);
    if (modr == SELFRESET) {
        (*sts_frm)->modr = 0;
    }
}

void stsReset(StatFrame** sts_frm)
{
    (*sts_frm)->status=LISTN;
    (*sts_frm)->act_id=ZZZZ;
    (*sts_frm)->err_code=IMFINE;
    (*sts_frm)->modr=0;
}

void stsOut(StatFrame** sts_frm)
{
    printf("Status:%d\n",(*sts_frm)->status);
}

void serrOut(StatFrame** sts_frm)
{
    fprintf(stderr,"[ Error Code: %d ]", (*sts_frm)->err_code);
    fprintf(stderr,"< %d >\n", (*sts_frm)->modr);
    if ((*sts_frm)->act_id == GBYE){
        fprintf(stderr,"Shutting down\n");
        (*sts_frm)->status = SHTDN;
    }
}


/**
 *  <h2>ResponseCMDs
 * */
/**
 *  <b>Travel ops
 * */
//void rsp_cwnd(StatFrame** sts_frm, InfoFrame** inf_frm, DChains* dchns, Lattice* hltc, unsigned char ** nm_buf){

void rsp_cwnd(StatFrame** sts_frm, InfoFrame** inf_frm, DChains* dchns, Lattice* hltc, uniArr* buf){

//    unsigned char* dn_hash;
//    HashBridge* hbrg;
//    yield_dnhsh(&((*dchns)->vessel), &dn_hash);
//    hbrg = yield_bridge(*hltc, dn_hash,crypto_shorthash_KEYBYTES,((*dchns)->vessel));
//
    buf->carr = (unsigned char*) ((*dchns)->vessel)->diname;
    printf("\n>> %s\n",buf->carr);

    setSts(sts_frm,RESPN,OBJNM);


//    yield_bridge(hltc,)
}

void rsp_gotond(StatFrame** sts_frm, InfoFrame** inf_frm, DChains* dchns, Lattice* hltc, uniArr* buf){

    unsigned long long* dest_id = (unsigned long long*) malloc(sizeof(unsigned long long));

    memcpy(dest_id, (*inf_frm)->cmdSeq->arr->carr, sizeof(unsigned long long));
    unsigned int stepcnt;
    stepcnt = gotonode(*dest_id,*dchns);

    setSts(sts_frm,TRVLD,stepcnt); //Modr set to step count
    buf = (uniArr*) &((*dchns)->vessel->diname);

//    unsigned char* dn_hash;
//
//    HashBridge* hbrg;
//    yield_bridge()
//     expo_dirbase(*dchns)->vessel

}

void rsp_listnd(StatFrame** sts_frm, InfoFrame** inf_frm, DChains* dchns, Lattice* hltc, uniArr* buf){
}
void rsp_status(StatFrame** sts_frm, InfoFrame** inf_frm, DChains* dchns, Lattice* hltc, uniArr* buf){
}
void rsp_fiid(StatFrame** sts_frm, InfoFrame** inf_frm, DChains* dchns, Lattice* hltc, uniArr* buf){
}

/**
 * \ResponseActions
 * Initialize and return arr of response action functions
 */
void* rsp_acts(StatFrame** sts_frm,
              InfoFrame** inf_frm,
              DChains* dchns,
              Lattice* hltc,
              int cnfg_fd,
              uniArr* buf,
               void (*funarr[5])(StatFrame**, InfoFrame* *, DChains*, Lattice*, uniArr*)){

    int fcnt = 1;

    void (*cwdn)(StatFrame** sts_frm, InfoFrame** inf_frm, DChains* dchns, Lattice* hltc, uniArr* buf); // return CWD
    cwdn = rsp_cwnd;
    void (*gond)(StatFrame** sts_frm, InfoFrame** inf_frm, DChains* dchns, Lattice* hltc, uniArr* buf); // Goto dirndode
    gond = &rsp_gotond;
    void (*lsnd)(StatFrame** sts_frm, InfoFrame** inf_frm, DChains* dchns, Lattice* hltc, uniArr* buf); // list dirnode contents
    lsnd = &rsp_listnd;
    void (*psts)(StatFrame** sts_frm, InfoFrame** inf_frm, DChains* dchns, Lattice* hltc, uniArr* buf); // return status
    psts = &rsp_status;
    void (*fiid)(StatFrame** sts_frm, InfoFrame** inf_frm, DChains* dchns, Lattice* hltc, uniArr* buf); // return fiid for given filename
    fiid = &rsp_fiid;

    (funarr)[0] = cwdn;
    (funarr)[1] = gond;
    (funarr)[2] = lsnd;
    (funarr)[3] = psts;
    (funarr)[4] = fiid;

    return funarr;

    //(*funarr[cmd])(sts_frm, inf_frm, dchns, hltc, buf);
}

