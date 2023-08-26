//
// Created by ujlm on 8/25/23.
//

#include <malloc.h>
#include <string.h>
#include "fiforms.h"
#include <string.h>

FiFormExtArr forms = {


		"py",
		"pyc",
		"txt",
		"pem",
		"exe",
		"xml",
		"RECORD",
		"WHEEL",
		"nu",
		"pth",
		"tmpl",
		"NONETYPE"


};

FiFormItm determ_form(unsigned char* fext, int extlen){

    if (extlen < EXTMAXLEN) {
        unsigned char buf[extlen];

        for (int i = 0; i < extlen; i++){
            buf[i] = *fext+i;
        }
    }

}


enum FiFormId grab_ffid(const unsigned char* fname, unsigned int nlen) {

    enum FiFormId res = NONETYPE;

    unsigned char* buf = (unsigned char*) calloc(nlen, sizeof(unsigned char));
    memcpy(buf, fname, (nlen*sizeof(unsigned char)));

    int n = 0;
    int i = 0;
    int cmpres = 0;

    while(n < nlen && buf[n] != 46) {
        n++;
    }
    if (n == nlen){
        res = NONETYPE;
    }

    unsigned char* extbuf = (unsigned char*) calloc(nlen-n, sizeof (unsigned char));

    for (i = n; i < nlen; i++){
        extbuf[n-i] = buf[i];
    }

    for (i = 0; i < FORMCOUNT; i++) {
        if (memcmp(buf, forms[i], nlen-n) == 0){
            res = i;
            break;
        }
    }

    return res;
}