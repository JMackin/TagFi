//
// Created by ujlm on 8/25/23.
//
#include <string.h>
#include "fiforms.h"

FiForms get_formid(unsigned char* fext, int extlen){


    if (extlen < EXTMAXLEN) {
        unsigned char buf[extlen];

        for (int i = 0; i < extlen; i++){
            buf[i] = *fext+i;
        }
    }


}

FiForms get_formext(int){

}