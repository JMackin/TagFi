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

/*
 *
 * Info Requests must follow this format:
 *
 * @byte  |_________________________
 *  1:    | [OR'd lead flags]
 *  4:    |    -> [Arr-Len]
 *  8:    |    -> { Array: ========{ Request subject |x| subject ID/name |x| Misc/Qualifier }
 * 8+len: |    -> [END/DONE flag]
 *
 *       |x| = seperator:
 *                 \ \_ int:  '0xdbdbdbdb' (3688618971)
 *                  \____ char: '0xbddbdbbd' (3185302461)
 */


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
//unsigned int split_cats(const unsigned int *lead_flags,
//                        LttcFlags* flag_list,
//                        LttFlg *flg_itr,
//                        unsigned int s_itr,
//                        unsigned int l_itr,
//                        unsigned int cnt,
//                        unsigned int trfidi,
//                        unsigned int *subflg);
//
///**
//* \DivideFlags
//*  Receives each combined flag value from SplitCategories and recursively apply
//*      each of the 4 flag values in that category as an AND mask. If the AND operation results true,
//*      the flag is appended to an array of ReqFlags point to by 'flag_list'.
//**/
//unsigned int div_flgs(const unsigned int *lead_flags,
//                      LttcFlags* flag_list,
//                      LttFlg *flg_itr,
//                      unsigned int s_itr,
//                      unsigned int l_itr,
//                      unsigned int cnt,
//                      unsigned int trfidi,
//                      unsigned int *subflg) {
//
//    if (s_itr > 3) {
//        return cnt;
//    }
//    if (*(lead_flags + l_itr) & flg_itr->req) {
//        ((*flag_list + cnt)->req)  = ((flg_itr)->req);
//        ++(cnt);
//        if (*subflg==7) {
//            (*subflg = s_itr);
//        }
//    }
//    flg_itr->req <<= 1;
//    div_flgs(lead_flags, flag_list, flg_itr, ++s_itr, l_itr, cnt, trfidi, subflg);
//
//}
//
//unsigned int split_cats(const unsigned int *lead_flags,
//                        LttcFlags* flag_list,
//                        LttFlg *flg_itr,
//                        unsigned int s_itr,
//                        unsigned int l_itr,
//                        unsigned int cnt,
//                        unsigned int trfidi,
//                        unsigned int *subflg) {
//
//    if (l_itr > 5 || (flg_itr->req) >= UUUU) {
//        return cnt;
//    } else {
//        if (l_itr == 2) {
//            if (*subflg == 15) {
//                *subflg >>= 1;
//            }
//            if (trfidi) {
//                flg_itr->req <<= (4 * trfidi);
//            }
//        } else if (l_itr == 3) {
//            flg_itr->req = SAVE;
//        }
//        cnt = div_flgs(lead_flags, flag_list, flg_itr, s_itr, l_itr, cnt, trfidi, subflg);
//        (++l_itr);
//        split_cats(lead_flags, flag_list, flg_itr, 0, l_itr, cnt, trfidi, subflg);
//
//    }
//    //TODO: Implement return 0 in default cases, and flgcnt updatted with '+=' rather than assignment.
//}

/*
 * AI GENERATED
 */
uint div_flgs(ptr_uint lead_flags,
              LttcFlags* flag_list,
              LttFlg *flg_itr,
              uint s_itr,
              uint l_itr,
              uint cnt,
              uint trfidi,
              ptr_uint subflg) {

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
    return div_flgs(lead_flags, flag_list, flg_itr, ++s_itr, l_itr, cnt, trfidi, subflg);

}

uint split_cats(ptr_uint lead_flags,
                LttcFlags* flag_list,
                LttFlg *flg_itr,
                uint s_itr,
                uint l_itr,
                uint cnt,
                uint trfidi,
                ptr_uint subflg) {

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
        return split_cats(lead_flags, flag_list, flg_itr, s_itr, ++l_itr, cnt, trfidi, subflg);
    }

    //TODO: Implement return 0 in default cases, and flgcnt updated with '+=' rather than assignment.
}
/**
 *\ParseLead
 * Convert request-lead to an array of flags using bit-masking and return the flag count.
 */
uint parse_lead(cnst_uint lead,
                LttcFlags *flg_list,
                StatFrame **sts_frm,
                InfoFrame **inf_frm) {

    uint k = 1920;
    uint i = 0;
    uint flg_suffx = 15;
    uint flgcnt = 0;

    /** Alloc buffer to hold OR'd category flag values
     * and an array for the final parsed flag list.
     * */
    ptr_uint lead_flags = (ptr_uint ) calloc(4,uint_sz);    // [ quals | trvl/fiops/dirops/ | sysops | arrsigs ]
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


InfoFrame *parse_req(uchar_arr fullreqbuf, //<-- same name in spin up
                     InfoFrame **infofrm,
                     StatFrame **stsfrm,
                     LttcFlags* rqflgsbuf,       //<-- flgsbuf in spin up
                     uchar_arr tmparrbuf,
                     buff_arr req_arr_buf) //<-- req_arr_buf in spin up
{

    uint exit_flg = 1;
    int k = 0;
    uint is_arr = 0; //1 = char, 2 = int
    uint flag;
    uint flgcnt;
    uint end;
    uint dflt_flg = 0;

    /**
     * Parse request sequence-lead and init CMD struct
     * */
    memcpy(&flag, fullreqbuf, uint_sz);

    flgcnt = parse_lead(flag, rqflgsbuf, stsfrm, infofrm);

    if (!flgcnt) {
        stsErno(MALREQ,stsfrm,errno,flag,"Malformed request, error parsing lead","parse_req->parse_lead","misc - lead");
      //  err_info_frm(*infofrm, stsfrm, MALREQ, 'l'); // l = parsing lead
        return *infofrm;
    }

    if ((*infofrm)->qual == 2){
        dflt_flg = 1;
    }

    /**
     * Build CMD struct
     * */
    ((*infofrm)->flags) = (rqflgsbuf); //Note: *((*cmdseq)->flags)+X to access flags
    (*infofrm)->flg_cnt = flgcnt;


    /**
     * Check carrier byte
     * */
    if (flag >> 29 != 1) {
        stsErno(MALREQ,stsfrm,errno,flag,"Malformed request, bad carry flag","parse_req","misc - lead");
       // err_info_frm(*infofrm, stsfrm, MALREQ, 'l'); //
        return *infofrm;
    }

    /**
     * Check for Int arr
     * */
    if ((*infofrm)->arr_type == INTARR) {
        memcpy(&((*infofrm)->arr_len), (fullreqbuf + uint_sz), uint_sz); // Set InfoFrame -> arr length

        // 4B + 4b + 4B + 8B + (misc len) + 4B
        memcpy(&end, (fullreqbuf + (uint_sz * 2) + (uint_sz * (*infofrm)->arr_len)), uint_sz);  // Calc request endpoint

        /**
         * Check tail byte
         * */
        if (end != END) {
            stsErno(MALREQ,stsfrm,errno,end,"Request structure malformed","parse_req","misc - tail byte");
            return (NULL);
        }

        exit_flg = flag == ENDBYTES ? (exit_flg << 1) : 1; //   EXIT trigger 1

        /**
         * Alloc and populate int buffer with the cmd sequence
         * */
//        memcpy(tmparrbuf, fullreqbuf + (uint_sz * 2), (uint_sz * (*infofrm)->arr_len));
        tmparrbuf = (fullreqbuf + (uint_sz * 2));
        (*infofrm)->arr = tmparrbuf;

        exit_flg = *((*infofrm)->arr+(2*uint_sz)) == 255 ? (exit_flg << 1) : 1; // EXIT trigger 2
        exit_flg = *tmparrbuf == SHTDN ? (exit_flg << 1) : 1;    //  EXIT trigger 3

        /**
         * Exit for shutdown upon receiving three shutdown triggers
         * */
        if (exit_flg == 8) {
            setAct(stsfrm, GBYE, SHTDN, ESHTDN);
            return (*infofrm);
        }
    }
        /**
         * Check for char arr
         * */
    else if ((*infofrm)->arr_type == CHRARR) {
        memcpy(&((*infofrm)->arr_len), (fullreqbuf + uint_sz), uint_sz);
        memcpy(&end, (fullreqbuf + (uint_sz * 2) + (uchar_sz * (*infofrm)->arr_len)), uint_sz); //Calc request endpoint

        if (end != END) {
            stsErno(MALREQ,stsfrm,errno,end,"Computed position for tail byte doesnt match that of request tail","parse req","calcd end value");
            return (*infofrm);
        }

        memcpy(req_arr_buf, fullreqbuf + (uint_sz * 2), (uchar_sz * (*infofrm)->arr_len));
        (*infofrm)->arr = *req_arr_buf;
    }

    (*stsfrm)->status <<= 1;

    return (*infofrm);
}