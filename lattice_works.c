//
// Created by ujlm on 11/2/23.
//

#include "lattice_works.h"

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
    (*info_frm)->arr = NULL;
    (*info_frm)->flags = NULL;

    return *info_frm;
}

