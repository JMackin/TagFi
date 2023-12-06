
//
// Created by ujlm on 11/2/23.
//

#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <malloc.h>
#include "lattice_signals.h"

#include "reply_tools.h"

#define SELFRESET 333

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
long
stsErno(LattErr ltcerr, StatFrame **sts_frm, char *msg,
        unsigned long misc, char *miscdesc,
        char *function, char *note, int erno) {

    char* errmsg = malloc(ERRBUFLEN);
    unsigned int idx = sprintf(errmsg,"\n\n\n\t---------- Error ----------\n\t");
    (*sts_frm)->err_code = ltcerr;

    if(erno){perror(" ");}

    idx+=sprintf(errmsg+idx, ":%s"
                    "\n\t\t------------------\n", msg);

    idx+=sprintf(errmsg+idx, "\t\t[ LttcErr: %s ]", convertLattErr(&(*sts_frm)->err_code));

    if(erno) {idx+=sprintf(errmsg+idx, "\n\t\t [ Errno: %d ]", erno);}

    idx+=sprintf(errmsg+idx, "\n\t\t------------------");

    idx+=sprintf(errmsg+idx, "\n\t\\status:\n\t\t    [ %s ]\n", convertLattSts(&(*sts_frm)->status));
    if ((*sts_frm)->modr){
        idx+=sprintf(errmsg+idx, "\t\\modr:\n\t\t   [ %d ]\n", (*sts_frm)->modr);
    }
    if (function != NULL){
        idx+=sprintf(errmsg+idx,"\t\\function:\n\t\t   [ %s ]\n", function);
    }
    if (misc){
        idx+=sprintf(errmsg+idx,"\t\\%s:\n\t\t[ %ld ]\n", miscdesc, misc);
    }
    if (note != NULL){
        idx+=sprintf(errmsg+idx, "\t\t------------------\n");
        idx+=sprintf(errmsg+idx,"\t NOTE:  \n\t\t\t%s\n", note);
    }
    idx+=sprintf(errmsg+idx, "\t---------------------------\n"
                    "\t| | | | | | | | | | | | | |\n\n");

    unsigned int i = idx;

    while(--i){
        putc_unlocked((*(errmsg+(idx-i))),stderr);
    }

    (*sts_frm)->status = STERR;
    if (erno){(*sts_frm)->modr = erno;}


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
    if (modr == ESHTDN){
        (*sts_frm)->err_code=ESHTDN;
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
    stsErno((*sts_frm)->err_code, sts_frm, msg, 0, NULL, NULL, NULL, errno);
}

ErrorBundle init_errorbundle(){
    ErrorBundle bundle;

    bundle.raised=0;
                                    // attr #:
    bundle.erno=0;                 // 0
    bundle.relvval= 0;             // 1
    bundle.ltcerr=0;               // 2
    bzero(bundle.func,32);         // 3
    bzero(bundle.relvval_desc,64); // 4
    bzero(bundle.note,128);        // 5
    bzero(bundle.msg,256);         // 6

    return bundle;
}
ErrorBundle bundle_addglob(ErrorBundle bundle, LattErr ltcerr, char *msg,
                           unsigned long relvval, char *relvval_desc,
                           char *func, char *note, int erno){


    if(erno){(bundle).erno = erno;}
    if (relvval){(bundle).relvval = relvval;}
    if(func != NULL) {
        bzero(bundle.func, 32);
        strncpy(bundle.func, func, 32);
    }
    if(relvval_desc != NULL) {
        bzero(bundle.relvval_desc, 64);
        strncpy(bundle.relvval_desc, func, 64);
    }
    if(note != NULL) {
        bzero(bundle.note, 128);
        strncpy(bundle.note, func, 128);
    }
    if (msg != NULL) {
        bzero(bundle.msg, 256);
        strncpy(bundle.msg, msg, 256);
    }
    if(ltcerr != 0) {
        bundle.ltcerr = ltcerr;
    }
    return bundle;

}

uint bundle_add(ErrBundle* bundle, uint attr, void* val){
    switch(attr){
        case 0:
            (*bundle)->erno = *((int*) val);
            break;
        case 1:
            (*bundle)->relvval = *((ulong*) val);
            break;
        case 2:
            bzero((*bundle)->func,32);
            strncpy((*bundle)->func,(char*)val,32);
            break;
        case 3:
            bzero((*bundle)->relvval_desc,64);
            strncpy((*bundle)->relvval_desc,(char*)val,64);
            break;
        case 4:
            bzero((*bundle)->note,128);
            strncpy((*bundle)->note,(char*)val,128);
            break;
        case 5:
            bzero((*bundle)->msg,256);
            strncpy((*bundle)->func,(char*)val,256);
            break;
        case 6:
            (*bundle)->ltcerr = *((LattErr*) val);
            break;
        default:
            return 1;
    }
    return 0;
}




ErrorBundle raiseErr(LttSt lttSt, ErrorBundle bundle){
    clock_t ca = clock();

    bundle.raised = 1;

    char* itm_hold;
    char* errmsg_lnbuf = (char*) malloc(32*UCHAR_SZ);
    char* errmsg = (char*) malloc(ERRBUFLEN*UCHAR_SZ);
    SttsFrm sttsFrm = (*lttSt)->frame;


    unsigned int idx = sprintf(errmsg,"\n\n\n\t---------- Error ----------\n\t");

    if((bundle).erno){perror(" ");}

    idx+=sprintf(errmsg+idx, ":%s"
                             "\n\t\t------------------\n", bundle.msg);

    itm_hold = convertLattErr(&(bundle).ltcerr);
    if(itm_hold != NULL) {
        memcpy(errmsg_lnbuf, itm_hold, LERR_CHARCNT);
        idx += sprintf(errmsg + idx, "\t\t[ LttcErr: %s ]", itm_hold);
    }
    free(itm_hold);
    itm_hold=NULL;

    if(bundle.erno) {idx+=sprintf(errmsg+idx, "\n\t\t [ Errno: %d ]", bundle.erno);}

    idx+=sprintf(errmsg+idx, "\n\t\t------------------");

    itm_hold = convertLattSts(&(sttsFrm)->status);
    if(itm_hold != NULL) {
        memcpy(errmsg_lnbuf, itm_hold, LSTS_CHARCNT);
        idx += sprintf(errmsg + idx, "\n\t\\status:\n\t\t    [ %s ]\n", itm_hold);
    }
    free(itm_hold);
    itm_hold=NULL;

    if ((sttsFrm)->modr){
        idx+=sprintf(errmsg+idx, "\t\\modr:\n\t\t   [ %d ]\n", (sttsFrm)->modr);
    }
    if (bundle.func[0] != 0){
        idx+=sprintf(errmsg+idx,"\t\\function:\n\t\t   [ %s ]\n", bundle.func);
    }
    if (bundle.relvval != 0){
        idx+=sprintf(errmsg+idx,"\t\\%s:\n\t\t[ %ld ]\n", bundle.relvval_desc, bundle.relvval);
    }
    if (bundle.note[0] != 0){
        idx+=sprintf(errmsg+idx, "\t\t------------------\n");
        idx+=sprintf(errmsg+idx,"\t NOTE:  \n\t\t\t%s\n", bundle.note);
    }
    idx+=sprintf(errmsg+idx, "\t---------------------------\n"
                             "\t| | | | | | | | | | | | | |\n\n");

    unsigned int i = idx;
    while(--i){
        putc_unlocked((*(errmsg+(idx-i))),stderr);
    }
    free(errmsg);
    free(errmsg_lnbuf);
    clock_t cb = clock();
    fprintf(stderr,"\t\t<<ErrTime: %ld>>\n",cb-ca);
}