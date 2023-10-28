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

int is_init = 0;
//latticeCmd* lttccmd;
//reqFlag reqsarr[13] = {FFF,TTT,DFLT,NARR,GCSQ,VESL,GOTO,LIST,FIID,FINM,INFO,LEAD,END};
//rspFlag rspsarr[13] = {FFFF,NRSP,RARR,RLEN,STAS,CODE,CINT,DDDD,EEEE,GGGG,ZERO,ERRR,DONE};

enum ReqFlag reqFlag;
enum RspFlag rspFlag;

unsigned long UISiZ = sizeof(unsigned int);
unsigned long UCSiZ = sizeof(unsigned char);

Cmd_Seq *init_cmdseq(Cmd_Seq **cmdSeq, uniArr** arr, unsigned int type) {
    *cmdSeq = malloc(sizeof(Cmd_Seq));
    (*cmdSeq)->arr = *arr;
    //(*cmdSeq)->flags = (LttcFlags *) calloc(sizeof(LttcFlags), sizeof(UISiZ));
    (*cmdSeq)->arr_len = 0;
    (*cmdSeq)->seq_id = 0;
    (*cmdSeq)->flg_cnt = 0;
    (*cmdSeq)->type = type;
    return *cmdSeq;

}Cmd_Seq *reset_cmdseq(Cmd_Seq **cmdSeq, unsigned int type) {
//    *cmdSeq = malloc(sizeof(Cmd_Seq));
//    (*cmdSeq)->arr = (uniArr *) calloc(arrsize, sizeof(uniArr));
    //(*cmdSeq)->flags = (LttcFlags *) calloc(sizeof(LttcFlags), sizeof(UISiZ));
    bzero((*cmdSeq)->flags, ((*cmdSeq)->flg_cnt));
    bzero((*cmdSeq)->arr, ((*cmdSeq)->arr_len));
    (*cmdSeq)->arr_len = 0;
    (*cmdSeq)->seq_id = 0;
    (*cmdSeq)->flg_cnt = 0;
    (*cmdSeq)->type = type;
    return *cmdSeq;
}

// Secure zero and destroy CMD struct. Returns pointer to NULL.
Cmd_Seq *destroy_cmdseq(StatFrame **sts_frm, Cmd_Seq **cmdSeq) {

    (*cmdSeq)->arr_len = 0;
    (*cmdSeq)->seq_id = 0;
    (*cmdSeq)->flg_cnt = 0;
    free(*cmdSeq);
    *cmdSeq = NULL;
    return *cmdSeq;
}

/** Set flip to 0 when using.*/
Cmd_Seq *copy_cmdseq(unsigned int flip, Cmd_Seq **cmdSeq, Cmd_Seq **copy, StatFrame **sts_frm) {

    if (*cmdSeq != NULL) {
        if (*copy != NULL) {
            if (!flip) {
                *copy = destroy_cmdseq(sts_frm, copy);
                copy_cmdseq(1, cmdSeq, copy, sts_frm);
            } else {
                setErr(sts_frm, ADFAIL, flip);
                serrOut(sts_frm, "Recursion failed in copyCmdSeq");
                return NULL;
            }
        } else {
            size_t arrsize = (*cmdSeq)->type == 8 ? (*cmdSeq)->arr_len * UCSiZ : (*cmdSeq)->arr_len * UISiZ;
           // *copy = init_cmdseq(copy, (*cmdSeq)->arr_len, (*cmdSeq)->type);
            memcpy((*copy)->arr, (*cmdSeq)->arr, arrsize);
            ((*copy)->flags) = (*cmdSeq)->flags;
            //memcpy((*copy)->flags, (*cmdSeq)->flags, (*cmdSeq)->flg_cnt * UISiZ);
            (*copy)->arr_len = (*cmdSeq)->arr_len;
            (*copy)->flg_cnt = (*cmdSeq)->flg_cnt;
            (*copy)->seq_id = (*cmdSeq)->seq_id;

            return *copy;
        }
    } else {
        setErr(sts_frm, ILMMOP, 0);
        serrOut(sts_frm, "CMD struct to be copied points to NULL");
        return NULL;
    }
    return *copy;
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

void destroy_cmdstructures(unsigned char *buffer, unsigned char *respbuffer, unsigned char *carr, unsigned int *iarr,
                           Resp_Tbl *rsp_tbl, Seq_Tbl *seqTbl) {

    free(buffer);
    free(respbuffer);
    free(carr);
    free(iarr);
    for (int i = 0; i < seqTbl->cnt; i++) {
        free((*(seqTbl)->seq_map->cmd_seq + i)->arr);
        free((*seqTbl->seq_map->cmd_seq + i)->flags);
        free(*seqTbl->seq_map->cmd_seq + i);
        free((seqTbl)->seq_map + i);

    }
    free(seqTbl->seq_map);
    free(seqTbl->cmd_key);

    free(seqTbl);

    // free(((rsp_tbl)->rsp_map));
    free(((rsp_tbl)->rsp_funcarr));
    free(rsp_tbl);
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
unsigned int serial_seq(unsigned char *seq_out,
                        Cmd_Seq **cmd_seq) {

    unsigned int flgcnt = ((*cmd_seq)->flg_cnt);
    unsigned int flg;
    unsigned int byte_cnt;

    byte_cnt = (UISiZ * 3) + (UCSiZ * ((*cmd_seq)->arr_len));

    flg = build_lead(((unsigned int *) (*cmd_seq)->flags), flgcnt);

    memcpy((seq_out), &flg, UISiZ);
    memcpy((seq_out + UISiZ), &((*cmd_seq)->arr_len), UCSiZ);

    if (((*cmd_seq)->arr_len) && ((*cmd_seq)->arr) != NULL) {
        memcpy(seq_out + (2 * UISiZ), ((*cmd_seq)->arr), ((*cmd_seq)->arr_len) * UCSiZ);
    }

    flg = END;
    memcpy(seq_out + (2 * UISiZ) + (UCSiZ * ((*cmd_seq)->arr_len)), &flg, UISiZ);
    free(*cmd_seq);

    return byte_cnt;
}


unsigned int init_seqtbl(Seq_Tbl **seq_tbl, unsigned long mx_sz) {
    *seq_tbl = (Seq_Tbl *) malloc(sizeof(Seq_Tbl));
    ((*seq_tbl)->seq_map) = (SeqMap *) calloc(mx_sz, sizeof(SeqMap));
    ((*seq_tbl)->cmd_key) = (unsigned char *) malloc(crypto_shorthash_KEYBYTES);
    (*seq_tbl)->mx_sz = mx_sz;
    (*seq_tbl)->cnt = 0;
    mk_little_hash_key(&(*seq_tbl)->cmd_key);

    return sizeof(seq_tbl);
}


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
unsigned long mk_seq_hash(Cmd_Seq **cmdSeq, Seq_Tbl **seqTbl) {

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
    posi_tmp = cnvrt_iarr_to_carr((unsigned int *) &(*cmdSeq)->flags + posi, (*cmdSeq)->flg_cnt, &seq_buf);
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
unsigned long save_seq(Cmd_Seq *cmd_seq, Seq_Tbl **seq_tbl, int cnfdir_fd) {

    int seqstr_fd = openat(cnfdir_fd, SEQSTORE, O_APPEND | O_RDWR);
    if (seqstr_fd == -1) {
        perror("Error opening sequence store\n");
        return -1;
    }

    unsigned int smapcnt = (*seq_tbl)->cnt;

    cmd_seq->seq_id = mk_seq_hash(&cmd_seq, seq_tbl);
    (((*seq_tbl)->seq_map + smapcnt)->seq_id) = cmd_seq->seq_id;
    ((*seq_tbl)->seq_map + smapcnt)->cmd_seq = &cmd_seq;
    (*seq_tbl)->cnt++;

    printf("%lu: seq_tbl\n", (unsigned long) (((*seq_tbl)->seq_map + smapcnt)->seq_id));
    printf("%lu: cmd_seq\n", (unsigned long) (*((*seq_tbl)->seq_map + smapcnt)->cmd_seq)->seq_id);

    close(seqstr_fd);

    return cmd_seq->seq_id;

}

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
InfoFrame *init_info_frm(InfoFrame **info_frm) {
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
    (*info_frm)->cmdSeq = NULL;

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
    info_frm->cmdSeq = NULL;
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
    (*info_frm)->cmdSeq = NULL;
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
                     Cmd_Seq **cmdseq,
                     InfoFrame *infofrm,
                     StatFrame **stsfrm,
                     LttcFlags rqflgsbuf,
                     unsigned char **carr_buf) {


    unsigned int exit_flg = 1;
    int k = 0;
    unsigned int is_arr = 0; //1 = char, 2 = int
    unsigned int flag;
    unsigned int flgcnt;
    unsigned int end;

    //VER B
    // unsigned char* carr_buf;
    unsigned int *iarr_buf;

    /**
     * Parse request sequence-lead and init CMD struct
     * */
    memcpy(&flag, fullreqbuf, UISiZ);

    flgcnt = parse_lead(flag, &rqflgsbuf, stsfrm, &infofrm);

    if (!flgcnt) {
        fprintf(stderr, "Malformed request: Error parsing lead.\n");
        err_info_frm(infofrm, stsfrm, MALREQ, 'l'); // l = parsing lead
        return infofrm;
    }

    /**
     * Build CMD struct
     * */
    ((*cmdseq)->flags) = (&rqflgsbuf); //Note: *((*cmdseq)->flags)+X to access flags
    (*cmdseq)->lead = flag;
    (*cmdseq)->flg_cnt = flgcnt;


    /**
     * Check carrier byte
     * */
    if (flag >> 29 != 1) {
        fprintf(stderr, "Malformed request>\n> %d\n", flag);
        err_info_frm(infofrm, stsfrm, MALREQ, 'l'); //
        return infofrm;
    }

    /**
     * Check for Int arr
     * */
    if (infofrm->arr_type == INTARR) {
        memcpy(&(infofrm->arr_len), (fullreqbuf + UISiZ), UISiZ); // Set InfoFrame -> arr length
        (*cmdseq)->arr_len = infofrm->arr_len; // Set CMD -> arr length
        memcpy(&end, (fullreqbuf + (UISiZ * 2) + (UISiZ * infofrm->arr_len)), UISiZ);  // Calc request endpoint

        exit_flg = infofrm->arr_len == 1 ? (exit_flg << 1) : 1; // EXIT trigger 1

        infofrm->req_size = (UISiZ * 3) + (UISiZ * infofrm->arr_len); // Set InfoFrame -> request size

        /**
         * Check tail byte
         * */
        if (end != END) {
            fprintf(stderr, "Malformed request>\n> %d\n> %d\n> %d\n", flag, end, infofrm->arr_len);
            err_info_frm(infofrm, stsfrm, MALREQ, 't'); // 't' = tail
            return infofrm;
        }

        exit_flg = flag == ENDBYTES ? (exit_flg << 1) : 1; //   EXIT trigger 2

        /**
         * Alloc and populate int buffer with the cmd sequence
         * */
        iarr_buf = (unsigned int *) calloc(infofrm->arr_len, UISiZ);
        memcpy(iarr_buf, fullreqbuf + (UISiZ * 2), (UISiZ * infofrm->arr_len));
        (*cmdseq)->arr->iarr = iarr_buf;

        exit_flg = *iarr_buf == SHTDN ? (exit_flg << 1) : 1;    //  EXIT trigger 3

        /**
         * Exit for shutdown upon receiving three shutdown triggers
         * */
        if (exit_flg == 8) {
            setAct(stsfrm, GBYE, SHTDN, SELFRESET);
            return infofrm;
        }
    }
        /**
         * Check for char arr
         * */
    else if (infofrm->arr_type == CHRARR) {
        memcpy(&(infofrm->arr_len), (fullreqbuf + UISiZ), UISiZ);
        (*cmdseq)->arr_len = infofrm->arr_len;
        memcpy(&end, (fullreqbuf + (UISiZ * 2) + (UCSiZ * infofrm->arr_len)), UISiZ); //Calc request endpoint


        infofrm->req_size = (UISiZ * 3) + (UISiZ * infofrm->arr_len); // Set InfoFrame -> fullreqbuf size

        if (end != END) {
            fprintf(stderr, "Malformed request>\n> %d\n> %d\n> %d\n", flag, end,
                    infofrm->arr_len); // Init int arr buffer if iarr follows
            err_info_frm(infofrm, stsfrm, MALREQ, 0);
            return infofrm;
        }

//VER B
//        carr_buf = (unsigned char*) calloc(infofrm->arr_len,UCSiZ); // Init char arr buffer if carr follows

        memcpy(carr_buf, fullreqbuf + (UISiZ * 2), (UCSiZ * infofrm->arr_len));
        (*cmdseq)->arr->carr = *carr_buf;
//VER B
//        (*cmdseq)->arr->carr = carr_buf;
    }

    (infofrm->cmdSeq) = *cmdseq;
    (*stsfrm)->status <<= 1;

    return infofrm;
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
    printf("Status:%d\n", (*sts_frm)->status);
}

/** Output error code
 *<br>
 *  - Optionally, exit if StatusFrame Act-code is set to GBYE.
 *<br>
 *  - A message string can be passed in for display or pass in NULL
 *  for none;
 * */
void serrOut(StatFrame **sts_frm, char *msg) {
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

void rsp_err(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *chns, Lattice*hltc, uniArr **buf) {
    printf("Response: Error frame");
 }

 void rsp_nfo(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *dchns, Lattice * hltc, uniArr **buf) {
    printf("Response: info");
 }

 void rsp_sts(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *dchns, Lattice * hltc, uniArr **buf) {
    printf("Response: Status frame");
 }

 void rsp_und(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *dchns, Lattice * hltc, uniArr **buf) {
    printf("Response: Undefined");
 }

 void rsp_fiid(StatFrame **sts_frm, InfoFrame **inf_frm, DChains*dchns, Lattice* hltc, uniArr **buf) {
    printf("Response: File ID");
 }

 void rsp_diid(StatFrame **sts_frm, InfoFrame **inf_frm, DChains*dchns, Lattice* hltc, uniArr **buf) {
    printf("Response: Dir ID");
 }

 void rsp_frdn(StatFrame **sts_frm, InfoFrame **inf_frm, DChains*dchns, Lattice* hltc, uniArr **buf) {
    printf("Response: Resident Dir");
 }

 void rsp_gond(StatFrame **sts_frm, InfoFrame **inf_frm, DChains*dchns, Lattice* hltc, uniArr **buf) {
    printf("Response: Goto node");
 }

 void rsp_fyld(StatFrame **sts_frm, InfoFrame **inf_frm, DChains*dchns, Lattice* hltc, uniArr **buf) {
    printf("Response: Yield object");}

 void rsp_jjjj(StatFrame **sts_frm, InfoFrame **inf_frm, DChains*dchns, Lattice* hltc, uniArr **buf) {
    printf("Response: Empty");
 }

 void rsp_dsch(StatFrame **sts_frm, InfoFrame **inf_frm, DChains*dchns, Lattice* hltc, uniArr **buf) {
    printf("Response: Search for object");
 }

 void rsp_iiii(StatFrame **sts_frm, InfoFrame **inf_frm, DChains*dchns, Lattice* hltc, uniArr **buf) {
    printf("Response: Empty");
 }

 void rsp_dcls(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *dchns, Lattice *hltc, uniArr **buf) {
    printf("Response: List chain nodes");
 }

 void rsp_gohd(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *dchns, Lattice *hltc, uniArr **buf) {
    printf("Response: Go to chain head");
 }

 void rsp_dnls(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *dchns, Lattice *hltc, uniArr **buf) {
    printf("Response: List dir ");
 }

 void rsp_vvvv(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *dchns, Lattice *hltc, uniArr **buf) {
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
// VER. A
//              {RspFunc* rsp_act(int cnfg_fd,
//              RspMap rsp_map,
//              StatFrame** sts_frm,
//              InfoFrame** inf_frm,
//              DChains* dchns,
//              Lattice* hltc,
//              uniArr* buf,
//              RspFunc* funarr)

//Info
    void (*und)(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *dchns, Lattice *hltc, uniArr **buf);
    void (*err)(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *dchns, Lattice *hltc, uniArr **buf);
    void (*nfo)(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *dchns, Lattice *hltc, uniArr **buf);
    void (*sts)(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *dchns, Lattice *hltc, uniArr **buf);
//Travel
    void (*fiid)(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *dchns, Lattice *hltc, uniArr* *buf);
    void (*diid)(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *dchns, Lattice *hltc, uniArr* *buf);
    void (*frdn)(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *dchns, Lattice *hltc, uniArr* *buf);
    void (*gond)(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *dchns, Lattice *hltc, uniArr* *buf);
//Dir
    void (*fyld)(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *dchns, Lattice *hltc, uniArr* *buf);
    void (*jjjj)(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *dchns, Lattice *hltc, uniArr* *buf);
    void (*dsch)(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *dchns, Lattice *hltc, uniArr* *buf);
    void (*iiii)(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *dchns, Lattice *hltc, uniArr* *buf);
//File
    void (*dcls)(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *dchns, Lattice *hltc, uniArr* *buf);
    void (*gohd)(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *dchns, Lattice *hltc, uniArr* *buf);
    void (*dnls)(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *dchns, Lattice *hltc, uniArr* *buf);
    void (*vvvv)(StatFrame **sts_frm, InfoFrame **inf_frm, DChains *dchns, Lattice *hltc, uniArr* *buf);

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
                 uniArr **buf) {

    unsigned int *rsp_map;
    unsigned int fcnt = RSPARR;
    RspFunc *rsp_func;

    //rsp_map = (unsigned int *) sodium_malloc(sizeof(unsigned int) * (fcnt * 3 * 3));
    *rsp_tbl = (Resp_Tbl*) malloc(sizeof(Resp_Tbl));
    rsp_func = (RspFunc*) malloc(sizeof(RspFunc));

//VER. A
//    (*rsp_tbl)->rsp_funcarr = rsp_act(cnfg_fd,&rsp_map,sts_frm,inf_frm,dchns,hltc,buf,rsp_func);
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
                        uniArr **buf) {

    if (rsp > rsp_tbl->fcnt) {
        setErr(sts_frm, MISCLC, rsp);
        serrOut(sts_frm, "LattReply for response processing outside defined functionality.");
        return 0;
    }

    (*rsp_tbl->rsp_funcarr)[rsp](sts_frm, inf_frm, dchns, hltc, buf);

    setSts(sts_frm, RESPN, 0);

    return 1;
}

unsigned int respond(Resp_Tbl *rsp_tbl,
                      StatFrame **sts_frm,
                      InfoFrame **inf_frm,
                      DChains *dchns,
                      Lattice *hltc,
                      uniArr **resp_buf) {


    LattReply rsp = dtrm_rsp(sts_frm,inf_frm);

    unsigned int res =  proc_rsp(rsp_tbl,rsp,sts_frm,inf_frm,dchns,hltc,resp_buf);

    return res;

}
