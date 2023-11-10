
//
// Created by ujlm on 11/2/23.
//

#include <stdio.h>
#include <time.h>
#include <errno.h>
#include "lattice_signals.h"

#define SELFRESET 333
#define RSPARRLEN 16
#define INFARRLEN 19

StatFrame statFrame;

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

//TODO: OPTIMIZE THIS
long stsErno(LattErr ltcerr, StatFrame **sts_frm, int erno, long misc, char *msg, char *function, char *miscdesc) {
    clock_t ca = clock();
    fprintf(stderr,"\n\t---------- Error ----------\n\t");
    (*sts_frm)->err_code = ltcerr;
    perror(" ");
    fprintf(stderr, "\t : %s"
                    "\n\t\t------------------\n", msg);
    fprintf(stderr, "\t\t [ LttcErr: %d ]", (*sts_frm)->err_code);
    fprintf(stderr, "\n\t\t   [ Errno: %d ]", erno);

    fprintf(stderr, "\n\t\t------------------");

    fprintf(stderr, "\n\t\\status:\n\t\t\t  [ %d ]\n", (*sts_frm)->status);
    if (misc){
        fprintf(stderr,"\t\\note:\n\t\t\t  [ %ld ]\n", misc);
    }
    if ((*sts_frm)->modr){
        fprintf(stderr, "\t\\modr:\n\t\t\t  [ %d ]\n", (*sts_frm)->modr);
    }
    if (function != NULL){
        fprintf(stderr,"\t\\function:\n\t\t> %s", function);
    }
    if (miscdesc){
        fprintf(stderr, "\n\t\t------------------\n");
        fprintf(stderr,"\t NOTE:  \n\t\t\t%s\n", miscdesc);
    }
    clock_t cb = clock();
    fprintf(stderr,"\n\t\t <<ErrorTime: %ld>>",cb-ca);

    fprintf(stderr, "\n\t---------------------------\n"
                    "\t| | | | | | | | | | | | | |\n");

    (*sts_frm)->status = STERR;
    (*sts_frm)->modr = erno;





    if ((*sts_frm)->act_id == GBYE || (misc == 333)) {
        (*sts_frm)->status = SHTDN;
        return 1;
    }else
    {
        return 0;
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
    printf("\n--------------\n");
    printf("[ Status: %d ]\n", (*sts_frm)->status);
    if ((*sts_frm)->status == SHTDN) {
        fprintf(stdout, "\n<< GoodBye >>\n");

    }
}

/** Output error code
 *<br>
 *  - Optionally, exit if StatusFrame Act-code is set to GBYE.
 *<br>
 *  - A message string can be passed in for display or pass in NULL
 *  for none;
 * */
void serrOut(StatFrame **sts_frm, char *msg) {
    stsErno((*sts_frm)->err_code, sts_frm, errno, 0, msg, NULL, NULL);
}