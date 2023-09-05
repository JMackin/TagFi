//
// Created by ujlm on 8/25/23.
//

//
// Created by ujlm on 8/25/23.
//

#include <malloc.h>
#include <string.h>
#include "fiforms.h"
#include <string.h>

unsigned char form_exts[FORMCOUNT][EXTMAXLEN] = {


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
		"NONE"

};



//enum FiFormId determ_form(unsigned char* fext, int extlen){
//
//    if (extlen < EXTMAXLEN) {
//        unsigned char buf[extlen];
//
//        for (int i = 0; i < extlen; i++){
//            buf[i] = *fext+i;
//        }
//    }
//
//}


unsigned int grab_ffid(unsigned char* fname, unsigned int nlen) {

    enum FiFormId res = NONE;

    unsigned char* buf = (unsigned char*) calloc(nlen, sizeof(unsigned char));
    unsigned char* ext_buf;
    memcpy(buf, fname, (nlen*sizeof(unsigned char)));

    int n = 0;
    int i;
    int dotpos = 0;
    int cmpres = 0;
    int dotfound = 0;


    // NOTE: Will ned to filter file names with > 1 '.' for this to work, and for titles with dots > half-len.
    while (n < nlen ){

        if (buf[n] == 46) {
            dotpos = n;
            break;
        }
        n++;
    }

    if (n == nlen || n == 0){
        return NONE;
    }

    unsigned int extlen = nlen - dotpos;
    ext_buf = (unsigned char*) calloc(extlen,sizeof(unsigned char));
    memcpy(ext_buf, buf+dotpos+1, sizeof(unsigned char)*(extlen));

    int score;
    for (i = 0; i < FORMCOUNT; i++) {
        score=0;
        for (n = 0; n < extlen; n++){
            if (ext_buf[n] == form_exts[i][n]) {
                score++;
            }
            if (score == extlen) {

                res = i;
                free(buf);
                free(ext_buf);

                return res;

            }

            if (ext_buf[n] != form_exts[i][n]) {
                res = NONE;
            }
        }

//        if (memcmp((unsigned char*) ext_buf, (unsigned char*)form_exts[i], (nlen-dotpos)*sizeof(unsigned char)) != 0){
//            res = i;
//
//        }

    }

    free(buf);
    free(ext_buf);

    return res;

}