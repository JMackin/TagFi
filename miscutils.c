//
// Created by ujlm on 8/25/23.
//

#include <malloc.h>
#include <string.h>
#include "miscutils.h"

fiForms grab_fnm(const unsigned char* fname, unsigned int nlen) {

    unsigned char* buf = (unsigned char*) calloc(nlen, sizeof(unsigned char));
    memcpy(buf, fname, (nlen*sizeof(unsigned char)));
    int n = 0;
    do {
        n++;
    } while(n < nlen && buf[n] != 46);

    if (n == nlen){
        return NONETYPE;
    }
    else:









}