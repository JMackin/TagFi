//
// Created by ujlm on 11/2/23.
//

#include "fidi_masks.h"
#include "fi_lattice.h"
#include "lattice_works.h"
#include "lattice_rsps.h"
#include "lattice_signals.h"
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
#define HASHSTRLEN 64
#define ENDBYTES 538968075


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


InfoFrame *parse_req(const unsigned char *fullreqbuf, //<-- same name in spin up
                     InfoFrame **infofrm,
                     StatFrame **stsfrm,
                     LttcFlags* rqflgsbuf,       //<-- flgsbuf in spin up
                     unsigned char* tmparrbuf,
                     unsigned char **req_arr_buf) //<-- req_arr_buf in spin up
{

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

    flgcnt = parse_lead(flag, rqflgsbuf, stsfrm, infofrm);

    if (!flgcnt) {
        fprintf(stderr, "Malformed request: Error parsing lead.\n");
      //  err_info_frm(*infofrm, stsfrm, MALREQ, 'l'); // l = parsing lead
        return *infofrm;
    }

    /**
     * Build CMD struct
     * */
    ((*infofrm)->flags) = (rqflgsbuf); //Note: *((*cmdseq)->flags)+X to access flags
    (*infofrm)->lead = flag;
    (*infofrm)->flg_cnt = flgcnt;


    /**
     * Check carrier byte
     * */
    if (flag >> 29 != 1) {
        fprintf(stderr, "Malformed request>\n> %d\n", flag);
       // err_info_frm(*infofrm, stsfrm, MALREQ, 'l'); //
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

            stsErno(MALREQ,stsfrm,errno,end,"Request structure malformed","parse_req","misc - tail byte");
            return (NULL);
        }

        exit_flg = flag == ENDBYTES ? (exit_flg << 1) : 1; //   EXIT trigger 2

        /**
         * Alloc and populate int buffer with the cmd sequence
         * */
//        memcpy(tmparrbuf, fullreqbuf + (UISiZ * 2), (UISiZ * (*infofrm)->arr_len));
        tmparrbuf = (fullreqbuf + (UISiZ * 2));
        (*infofrm)->arr = tmparrbuf;

        exit_flg = *tmparrbuf == SHTDN ? (exit_flg << 1) : 1;    //  EXIT trigger 3

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
        memcpy(&end, (fullreqbuf + (UISiZ * 2) + (UCSiZ * (*infofrm)->arr_len)), UISiZ); //Calc request endpoint


        (*infofrm)->req_size = (UISiZ * 3) + (UISiZ * (*infofrm)->arr_len); // Set InfoFrame -> fullreqbuf size

        if (end != END) {
            stsErno(MALREQ,stsfrm,errno,end,"Computed position for tail byte doesnt match that of request tail","parse req","calcd end value");
            return (*infofrm);
        }


        memcpy(req_arr_buf, fullreqbuf + (UISiZ * 2), (UCSiZ * (*infofrm)->arr_len));
        (*infofrm)->arr = *req_arr_buf;

    }


    (*stsfrm)->status <<= 1;

    return (*infofrm);
}