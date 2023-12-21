
//
// Created by ujlm on 11/2/23.
//

#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <malloc.h>
#include <fcntl.h>
#include "lattice_signals.h"
#include "reply_tools.h"

#define SELFRESET 333
#define SPAWNTAG_CEIL 0x80000000 // Last tag in sequence to be assigned before rolling over to 1 - (1<<31)
#define THREADCNT_MAX 0x7f  // Threadpool threads capacity - (127)
#define TASKQUEUE_MAX 0x1ff // Threadpool Tasks capacity - (511)
#define ISTAG 0
#define ISID 1

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

    bundle.raised = 0;
    bundle._internal = 0; // 0: normal status   // 7

    // attr #:
    bundle.ltcstate = NULL;
    bundle.erno = 0;               // 0
    bundle.relvval = 0;            // 1
    bundle.ltcerr = 0;             // 2
    bzero(bundle.func,32);         // 3
    bzero(bundle.relvval_desc,64); // 4
    bzero(bundle.note,128);        // 5
    bzero(bundle.msg,256);         // 6

    return bundle;
}

SttsFrame mk_dummy_stsframe(){
    SttsFrame dummy_sf = malloc(sizeof(StatFrame));
    dummy_sf->status = STERR;
    dummy_sf->modr = 0;
    dummy_sf->err_code = NOINFO;
    dummy_sf->act_id = ZZZZ;

    return dummy_sf;
}

LttcState mk_dummy_ltcstate(){
    LttcState dummystate = malloc(sizeof(LatticeState));

    dummystate->frame = mk_dummy_stsframe();
    dummystate->cwdnode = 0;
    dummystate->tag = 0;
    dummystate->misc = 0;

    return dummystate;
}

ErrorBundle
bundle_addglob(ErrorBundle bundle, LattErr ltcerr, LttcState ltcstate, char *msg, unsigned long relvval,
               const char *relvval_desc, char *func, const char *note, int erno) {


    if(erno){(bundle).erno = erno;}
    if (relvval){(bundle).relvval = relvval;}
    if(func != NULL) {
        bzero(bundle.func, 32);
        strncpy(bundle.func, func, 32);
    }
    if(relvval_desc != NULL) {
        bzero(bundle.relvval_desc, 64);
        strncpy(bundle.relvval_desc, relvval_desc, 64);
    }
    if(note != NULL) {
        bzero(bundle.note, 128);
        strncpy(bundle.note, note, 128);
    }
    if (msg != NULL) {
        bzero(bundle.msg, 256);
        strncpy(bundle.msg, msg, 256);
    }
    if(ltcerr != 0) {
        bundle.ltcerr = ltcerr;
    }
    if(ltcstate != NULL){
        if (bundle._internal == 2){
            if (bundle.ltcstate != NULL) {
                free(bundle.ltcstate);
            }
            bundle._internal = 3;
        }
        bundle.ltcstate = ltcstate;
    }else{
        bundle.ltcstate = mk_dummy_ltcstate();
        bundle._internal = 2; // 2: dummy_ltcstate in-place
    }
    bundle.raised = 0;
    return bundle;

}

ErrorBundle * bundle_add(ErrBundle* bundle, uint attr, void* val){
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
            strncpy((*bundle)->msg,(char*)val,32);
            break;
        case 6:
            (*bundle)->ltcerr = *((LattErr*) val);
            break;
        case 7:
            (*bundle)->_internal = *((uint*) val);

            break;
        default:
            break;
    }
    (*bundle)->raised = 0;
    return *bundle;
}

ErrorBundle raiseErr(ErrorBundle bundle) {
    const char* errno_msg = (bundle).erno ? strerror(bundle.erno) : "";
    clock_t ca = clock();
    SttsFrame sttsFrm;

    bundle.raised = 1;

    char* itm_hold;
    char* errmsg_lnbuf = (char*) malloc(32*UCHAR_SZ);
    char* errmsg = (char*) malloc(ERRBUFLEN*UCHAR_SZ);

    if (bundle._internal != 3) {
        sttsFrm = bundle.ltcstate->frame;
    }else{
        sttsFrm = mk_dummy_stsframe();
        bundle._internal = 7; // 7: dummy sttsframe in use during raise_err
    }

    unsigned int idx = sprintf(errmsg,"\n\n\n\t---------- Error ----------\n\t");

    if((bundle).erno){ idx += sprintf(errmsg+idx,"perror: %s\n",errno_msg);}


    idx+=sprintf(errmsg+idx, "\tlattice: %s"
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
        idx += sprintf(errmsg + idx, "\n\t\\status:\n\t\t   [ %s ]\n", itm_hold);
    }
    free(itm_hold);
    itm_hold=NULL;

    if ((sttsFrm)->modr){
        idx+=sprintf(errmsg+idx, "\t\\modr:\n\t\t  [ %d ]\n", (sttsFrm)->modr);
    }
    if (bundle.func[0] != 0){
        idx+=sprintf(errmsg+idx,"\t\\function:\n\t\t  [ %s ]\n", bundle.func);
    }
    if (bundle.relvval != 0){
        idx+=sprintf(errmsg+idx,"\t\\%s:\n\t\t  [ %ld ]\n", bundle.relvval_desc, bundle.relvval);
    }
    if (bundle.note[0] != 0){
        idx+=sprintf(errmsg+idx, "\t\t------------------\n");
        idx+=sprintf(errmsg+idx,"\t NOTE:  \n\t\t\t%s\n", bundle.note);
    }

    clock_t cb = clock();
    idx+=sprintf(errmsg+idx,"\t\t------------------\n"
                            "\t\t  <<ErrTime: %ld>>\n",cb-ca);

    idx+=sprintf(errmsg+idx, "\t---------------------------\n"
                             "\t| | | | | | | | | | | | | |\n\n");

    unsigned int i = idx;
    while(--i){
        putc_unlocked((*(errmsg+(idx-i))),stderr);
    }
    free(errmsg);
    free(errmsg_lnbuf);

    if(bundle._internal & 7){
        if(bundle._internal == 2){
            free(bundle.ltcstate->frame);
            free(bundle.ltcstate);
            bundle._internal = 3;
        }else if(bundle._internal == 7){
            free(sttsFrm);
            bundle._internal = 3;
        }
        bundle.ltcstate = NULL;
    }

    bundle.raised = 1;

    return bundle;
}


ErrorBundle bundle_and_raise(ErrorBundle bundle, LattErr ltcerr, LttcState ltcstate, char *msg, unsigned long relvval,
                             const char *relvval_desc, char *func, const char *note, int erno){

    bundle = raiseErr(bundle_addglob(bundle, ltcerr, ltcstate, msg, relvval,relvval_desc, func, note, erno));

    if (bundle._internal == 2){
        free(bundle.ltcstate->frame);
        free(bundle.ltcstate);
    }

    return bundle;
}

/*
    1, Lattice_PTP hashLattice, SOA_HASHLATTICE
    2, Std_Buffer_PTP request_buf, SOA_REQBUF
    4, Std_Buffer_PTP response_buf, SOA_RESPBUF
    8, Std_Buffer_PTP requestArr_buf, SOA_REQARRBUF
    16, Std_Buffer_PTP tempArr_buf, SOA_TEMPARRBUF
    32, Flags_Buffer_PTP flags_buf, SOA_FLAGSBUF
    64, Info_Frame_PTP infoFrame, SOA_INFOFRAME
    128, ResponseTable_PTP responseTable, SOA_RESPTBL
    256, epEvent epollEvent_IN, SOA_EPEVENT
    512,  pthread_t tid, SOA_THREADID
    1024, int epollFD, SOA_EPOLLFD
    2048, int dataSocket, SOA_DATASOCKET
    4096, int BUF_LEN, SOA_BUFLEN
    8192, int BUF_LEN, SOA_TAG
 */
void update_SOA(SOA_OPTS opt, SOA_Pack* soaPack, void* new_val){

    switch (opt){
        case SOA_HASHLATTICE:
            (*soaPack)->hashLattice = (Lattice_PTP) new_val;
            break;
        case SOA_REQBUF:
            (*soaPack)->request_buf = (Std_Buffer_PTP) new_val;
            break;
        case SOA_RESPBUF:
            (*soaPack)->response_buf = (Std_Buffer_PTP) new_val;
            break;
        case SOA_REQARRBUF:
            (*soaPack)->requestArr_buf = (Std_Buffer_PTP) new_val;
            break;
        case SOA_TEMPARRBUF:
            (*soaPack)->tempArr_buf = (Std_Buffer_PTP) new_val;
            break;
        case SOA_FLAGSBUF:
            (*soaPack)->flags_buf =  (Flags_Buffer_PTP) new_val;
            break;
        case SOA_INFOFRAME:
            (*soaPack)->infoFrame = (Info_Frame_PTP) new_val;
            break;
        case SOA_RESPTBL:
            (*soaPack)->responseTable = (ResponseTable_PTP) new_val;
            break;
        case SOA_EPEVENT:
            (*soaPack)->epollEvent_IN = (epEvent) new_val;
            break;
        case SOA_THREADID:
            (*soaPack)->tid = *((pthread_t*) new_val);
            break;
        case SOA_EPOLLFD:
            (*soaPack)->epollFD = *((int*) new_val);
            break;
        case SOA_DATASOCKET:
            (*soaPack)->dataSocket = *((int*) new_val);
            break;
        case SOA_BUFLEN:
            (*soaPack)->buf_len = *((int*) new_val);
            break;
        case SOA_TAG:
            (*soaPack)->tag = *((int*) new_val);
            break;
        default:
            return;
    }
}


void update_SOA_DS(SOA_Pack* soaPack, int datasocket){(*soaPack)->dataSocket = datasocket;}

ThreadSpawn spawn_thread(pthread_t thread_id, uint alpha_tag){
    ThreadSpawn spawn;
    spawn.thread = thread_id;
    spawn.tag[0] = alpha_tag;
    spawn.tag[1] = 0;

    return spawn;
}

SPool init_spawnpool(int thread_count, int queue_size){
    SPool spawnpool = malloc(sizeof(SpawnPool));
    spawnpool->spawn = NULL;
    spawnpool->running_tag = 0;
    spawnpool->_internal = SPS_INIT;   // 0: Init / all normal
    spawnpool->count = 0;
    spawnpool->head = 0;
    spawnpool->queue_size = 0;
    spawnpool->stop = 0;
    spawnpool->tail = 0;
    spawnpool->task_queue = 0;
    spawnpool->thread_count = 0;
    return spawnpool;
}

// tag_or_id: ISTAG = 0, ISID = 1
Spawn find_spawn(uint tag_or_id, ulong tagid, SPool sPool){

    uint pos = 0;

    if (tag_or_id){
        pthread_t id;
        do{
            if (sPool->spawn+(pos) == NULL){
                return NULL;
            }
            id = ((sPool->spawn+(pos++)))->thread;
        }while (id != tagid && id != 0);
        if (id == 0){
            return NULL;
        }
    }else{
        uint tag;
        do{
            if (sPool->spawn+(pos) == NULL){
                return NULL;
            }
            tag = ((sPool->spawn+(pos++)))->tag[0];
        }while (tag != tagid);
    }

    return (sPool->spawn+(pos++));
}

uint push_spawn_task(SPool_PTP spawnpool, SpawnAct task, uint pos, uint tag){

    if ((*spawnpool)->queue_size - 1 >= TASKQUEUE_MAX){
        (*spawnpool)->_internal = SPS_ATTASKMAX;  // 8: Full task capacity
        return 1;
    }
    Spawn spawn_pos = find_spawn(ISTAG,tag,*spawnpool);
    if (spawn_pos == NULL){
        (*spawnpool)->_internal = SPS_SPAWNNOTFOUND;
        return 1;
    }
    if (spawn_pos->tag[0] != task.tag[0]){
        (*spawnpool)->_internal = SPS_TAGMISMATCH;
        return 1;
    }
    if(spawn_pos->tag[1] > 0){
        ++spawn_pos->tag[1];
        task.tag[1] = spawn_pos->tag[1];
    }
    *((*spawnpool)->task_queue+((*spawnpool)->queue_size++)) = task;

    return 0;

}

int add_spawn(SPool_PTP spawnpool, pthread_t thread, void *arg, SpawnAct spawnAct) {

    if((*spawnpool)->thread_count-1 >= THREADCNT_MAX){
        (*spawnpool)->_internal = SPS_ATTHREADMAX;  // 2: Full thread capacity
        return 1;
    }if ((*spawnpool)->queue_size - 1 >= TASKQUEUE_MAX){
        (*spawnpool)->_internal = SPS_ATTASKMAX;  // 8: Full task capacity
        return 1;
    }

    (*spawnpool)->thread_count++;
    (*spawnpool)->running_tag++;

    if (((*spawnpool)->running_tag) >= SPAWNTAG_CEIL){
        (*spawnpool)->running_tag = 1;
        (*spawnpool)->_internal = SPS_TAGROLLOVER; // 4: Running_tag rolled over
    }

    uint newtag = ++(*spawnpool)->running_tag;
    *((*spawnpool)->spawn + (*spawnpool)->thread_count) = spawn_thread(thread, (newtag));
    push_spawn_task(spawnpool,spawnAct,0,newtag);

    return 0;
}


SOA_Pack pack_SpinOff_Args(pthread_t tid,
                           Lattice_PTP hashLattice,
                           Std_Buffer_PTP request_buf,
                           Std_Buffer_PTP response_buf,
                           Std_Buffer_PTP requestArr_buf,
                           Std_Buffer_PTP tempArr_buf,
                           Flags_Buffer_PTP flags_buf,
                           Info_Frame_PTP infoFrame,
                           ResponseTable_PTP responseTable,
                           epEvent epollEvent_IN,
                           int epollFD,
                           int dataSocket,
                           int buf_len,
                           int tag){

    SOA_Pack soaPack = (SOA_Pack) malloc(sizeof(SpinOffArgsPack));
    uint cnt = 5;
    uint flip = 0;
    Std_Buffer_PTP bufs[4] = {request_buf,
            response_buf,
            requestArr_buf,
            tempArr_buf};

    soaPack->hashLattice = hashLattice;

    while((--cnt)){

        if(flip){
            bufs[cnt] = NULL;
        }
        else {
            if (bufs[cnt] == NULL) {
                cnt = 5;
                flip = 1;
                continue;
            }
        }
    }
    if (flip){ init_SOA_bufs(&soaPack);}

    soaPack->flags_buf = flags_buf;
    soaPack->infoFrame = infoFrame;
    soaPack->responseTable = responseTable;
    soaPack->epollEvent_IN = epollEvent_IN;
    soaPack->epollFD = epollFD;
    soaPack->dataSocket = dataSocket;
    soaPack->buf_len = buf_len;
    soaPack->tid = tid;
    if(tag){soaPack->tag = tag;}else{soaPack->tag = 1;}

    return soaPack;
}

uint discard_SpinOff_Args(SOA_Pack* soaPack){

    if ((*soaPack) == NULL){
        return 1;
    }else {
        if (!(*soaPack)->tag){return 1;}

        (*soaPack)->hashLattice = NULL;
        (*soaPack)->request_buf = NULL;
        (*soaPack)->response_buf = NULL;
        (*soaPack)->requestArr_buf = NULL;
        (*soaPack)->tempArr_buf = NULL;
        (*soaPack)->flags_buf = NULL;
        (*soaPack)->infoFrame = NULL;
        (*soaPack)->responseTable = NULL;
        (*soaPack)->epollEvent_IN = NULL;
        (*soaPack)->epollFD = 0;
        (*soaPack)->dataSocket = 0;
        (*soaPack)->buf_len = 0;
        (*soaPack)->tag = 0;
        (*soaPack)->tid = 0;

        free((*soaPack));
        (*soaPack) = NULL;

        return 0;
    }
}

void init_SOA_bufs(SOA_Pack* soaPack){

    (*soaPack)->flags_buf = (Flags_Buffer_PTP) malloc(sizeof(LttFlgs));
    *((*soaPack)->flags_buf) = (LttFlgs) calloc(ARRBUF_LEN, sizeof(LattFlag));

    (*soaPack)->request_buf = (Std_Buffer_PTP) malloc(ULONG_SZ);
    *((*soaPack)->request_buf) = (unsigned char *) calloc(BUF_LEN, sizeof(unsigned char));     // client request -> buffer

    (*soaPack)->response_buf = (Std_Buffer_PTP) malloc(ULONG_SZ);
    *((*soaPack)->response_buf) = (unsigned char *) calloc(BUF_LEN, sizeof(unsigned char));

    (*soaPack)->tempArr_buf = (Std_Buffer_PTP) malloc(ULONG_SZ);
    *((*soaPack)->tempArr_buf) = (unsigned char *) calloc(BUF_LEN, sizeof(unsigned char));

    (*soaPack)->requestArr_buf = (Std_Buffer_PTP) malloc(ULONG_SZ);
    *((*soaPack)->requestArr_buf) = (unsigned char *) calloc(ARRBUF_LEN, sizeof(unsigned char));

}

void destroy_SOA_bufs(SOA_Pack* soaPack){
    if(*(*soaPack)->request_buf != NULL){free(*(*soaPack)->request_buf);*(*soaPack)->request_buf = NULL;}
    if((*soaPack)->request_buf != NULL){free((*soaPack)->request_buf);(*soaPack)->request_buf = NULL;}

    if(*(*soaPack)->requestArr_buf != NULL){free(*(*soaPack)->requestArr_buf);(*soaPack)->requestArr_buf = NULL;}
    if((*soaPack)->requestArr_buf != NULL){free((*soaPack)->requestArr_buf);(*soaPack)->requestArr_buf = NULL;}

    if(*(*soaPack)->response_buf != NULL){free(*(*soaPack)->response_buf);(*soaPack)->response_buf = NULL;}
    if((*soaPack)->response_buf != NULL){free((*soaPack)->response_buf);(*soaPack)->response_buf = NULL;}

    if(*(*soaPack)->tempArr_buf != NULL){free(*(*soaPack)->tempArr_buf);}(*soaPack)->tempArr_buf = NULL;
    if((*soaPack)->tempArr_buf != NULL){free((*soaPack)->tempArr_buf);}(*soaPack)->tempArr_buf = NULL;

    if(*(*soaPack)->flags_buf != NULL){free(*(*soaPack)->flags_buf);(*soaPack)->flags_buf = NULL;}
    if((*soaPack)->flags_buf != NULL){free((*soaPack)->flags_buf);(*soaPack)->flags_buf = NULL;}
}

int make_socket_non_blocking(int sfd) {
    int flags, s;

    flags = fcntl(sfd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl");
        return -1;
    }

    flags |= O_NONBLOCK;
    s = fcntl(sfd, F_SETFL, flags);
    if (s == -1) {
        perror("fcntl");
        return -1;
    }
    return 0;
}