//
// Created by ujlm on 12/5/23.
//

#include "Options.h"
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>

unsigned int opsflags[OP_FLG_CNT];

void init_ops_flags(void){
    memset(opsflags,0xff,OP_FLG_CNT);
}

unsigned int flip_flags(const unsigned int flag_choice[OP_FLG_CNT]){

    unsigned int opts_cnt = 0;
    for (unsigned int i = 0; i < OP_FLG_CNT; i++){
        if (flag_choice[i] <= OP_FLG_CNT){
            opsflags[i] = flag_choice[i];
            ++opts_cnt;
        }
    }

    return opts_cnt;
}

size_t save_ops_settings(unsigned int mod){
    if(mod == 1){
        init_ops_flags();
    }

    FILE* confout = fopen(getenv("OPS_SETTINGS"),"w");
    unsigned char* opsflgs_buf = (unsigned char*) malloc((OP_FLG_CNT*4)+1);
    unsigned char* idx = opsflgs_buf;
    size_t strsize = 0;

    for (unsigned int i = 0; i < OP_FLG_CNT; i++){

        if (idx+sizeof(opsflags[i]) > (opsflgs_buf+(OP_FLG_CNT*4)+1))
        {
            free(opsflgs_buf);
            return -1;
        }
        if (opsflags[i] == 0){
            opsflags[i] = 0xff;
        }
        idx = mempcpy(idx,&opsflags[i],sizeof(opsflags[i]));
    }
    strsize = idx - opsflgs_buf;


    fwrite_unlocked(opsflgs_buf,strsize,sizeof(unsigned char),confout);
    fflush_unlocked(confout);
    free(opsflgs_buf);
    fclose(confout);


    return strsize;
}

size_t read_ops_settings(void){
    FILE* confin = fopen(getenv("OPS_SETTINGS"),"r");
    unsigned int* opsflgs_buf = (unsigned int*) calloc(OP_FLG_CNT+1,sizeof(unsigned int));
    size_t res_sz;
    res_sz = fread(opsflgs_buf,sizeof(unsigned int),OP_FLG_CNT,confin);

    if (res_sz == OP_FLG_CNT){
        for (unsigned int i = 0; i < OP_FLG_CNT; i++){
            opsflags[i] = opsflgs_buf[i];
        }
    }else{
        free(opsflgs_buf);
        return -1;
    }

    free(opsflgs_buf);
    fclose(confin);
    return res_sz;

}