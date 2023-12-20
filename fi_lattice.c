//
// Created by ujlm on 10/6/23.
//
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <sys/epoll.h>
#include <errno.h>
#include <pthread.h>
#include "jlm_random.h"
#include "lattice_rsps.h"
#include "fidi_masks.h"
#include "lattice_works.h"



#define CNFIGPTH "/home/ujlm/CLionProjects/TagFI/config"
#define DNCONFFI "dirnodes"
#define SOCKET_NAME "/tmp/9Lq7BNBnBycd6nxy.socket"
#define CMDCNT 16
#define ARRSZ 256
#define FLGSCNT 16
#define PARAMX 65535

int erno;

LttcState init_latticestate(void){
    SttsFrame status_frm = (SttsFrame) malloc(sizeof(StatFrame));
    LttcState lattst = (LttcState) malloc(sizeof(LatticeState));
    (status_frm)->status=RESET;
    (status_frm)->err_code=IMFINE;
    (status_frm)->modr=0;
    lattst->frame = status_frm;
    lattst->cwdnode = 1;
    lattst->misc = 0;

    return lattst;
}


void disassemble(Lattice* hashlattice,
                 DChains * dirchains,
                 Armatr* tbl_list,
                 unsigned char* dnconf_addr,
                 size_t dn_size,
                 unsigned char* seqstr_addr,
                 size_t sq_size,
                 int* lengths,
                 unsigned char** paths,
                 int dn_cnt,
                 const int cnfdir_fd) {
    int brdg_cnt = 0;
    int hb_cnt = 0;
    int entr_cnt;
    int i;
    int k;
    unsigned int cnt;

    munmap(dnconf_addr, dn_size);
    munmap(seqstr_addr, sq_size);

    //unsigned long* idxlst = calloc((PARAMX+(*hashlattice)->max),sizeof(unsigned long));

    for (i =0; i < (*hashlattice)->max; i++){
        if (((*hashlattice)->bridges[i]) != NULL){
            //*(idxlst+brdg_cnt) = (*((((*hashlattice)->bridges)[i]))->finode).fhshno;
            ++brdg_cnt;

//            free((*(((*hashlattice)->bridges)[i])->finode).finame);
//            free((*((*hashlattice)->bridges)[i]).finode);

            /*
             * AI GENERATED ----------------------\
             */
//            uchar* finm = (*((*hashlattice)->bridges[i])->finode).finame;
//            if(finm != NULL) {
//                free(finm);
//                (*((*hashlattice)->bridges[i])->finode).finame = NULL; // Avoid dangling pointer
//            }

            FiNode* fiode = &(*((*hashlattice)->bridges[i])->finode);
            if(fiode != NULL) {
                free(fiode);
                (((*hashlattice)->bridges[i])->finode) = NULL; // Avoid dangling pointer
            }
            /*
             * * AI GENERATED ----------------------/
             */

        }
    }

    //MEDA
    (*hashlattice)->chains->vessel = (*hashlattice)->chains->dir_head->right;
    cnt = expo_basedir_cnt((*hashlattice)->chains->vessel->did);
    goto_chain_tail((*hashlattice)->chains, 1, NULL, &(*hashlattice)->state);
    if ((*hashlattice)->chains->vessel->left->tag == 7){
        free((*hashlattice)->chains->vessel->left);
    }

    Armatr armatr_hold;
    entr_cnt = 0;
    for (i = 0; i< cnt; i++){
        if (is_base((*hashlattice)->chains->vessel)) {
            printf("bb");
            break;
        }

        //printf("<<FREED: %d\ncount: %d\n>>",entr_cnt,(*dirchains)->vessel->armature->count);

        if (!i){
            free((*hashlattice)->chains->vessel->right);
        }

        armatr_hold = tbl_list[i];
        struct stat statbuf;

        //Prime fd's
        if ((armatr_hold->nodemap->entrieslist_fd->prime_fd) > 2){
            if (fstat((armatr_hold->nodemap->entrieslist_fd->prime_fd),&statbuf) != -1){
                close((armatr_hold->nodemap->entrieslist_fd->prime_fd));
            }
            (armatr_hold->nodemap->entrieslist_fd->prime_fd) = 0;
        }
        if ((armatr_hold->nodemap->dirnode_fd->prime_fd) > 2){
            if (fstat((armatr_hold->nodemap->dirnode_fd->prime_fd),&statbuf) != -1){
                close((armatr_hold->nodemap->dirnode_fd->prime_fd));
            }
            (armatr_hold->nodemap->dirnode_fd->prime_fd) = 0;
        }
        if ((armatr_hold->nodemap->shm_fd->prime_fd) > 2){
            if (fstat((armatr_hold->nodemap->shm_fd->prime_fd),&statbuf) != -1){
                close((armatr_hold->nodemap->shm_fd->prime_fd));
            }
            (armatr_hold->nodemap->shm_fd->prime_fd) = 0;
        }

        //Duped fd's
        if ((armatr_hold->nodemap->entrieslist_fd->duped_fd) > 2){
            if (fstat((armatr_hold->nodemap->entrieslist_fd->duped_fd),&statbuf) != -1){
                close((armatr_hold->nodemap->entrieslist_fd->duped_fd));
            }
            (armatr_hold->nodemap->entrieslist_fd->duped_fd) = 0;
        }
        if ((armatr_hold->nodemap->dirnode_fd->duped_fd) > 2){
            if (fstat((armatr_hold->nodemap->dirnode_fd->duped_fd),&statbuf) != -1){
                close((armatr_hold->nodemap->dirnode_fd->duped_fd));
            }
            (armatr_hold->nodemap->dirnode_fd->duped_fd) = 0;
        }
        if ((armatr_hold->nodemap->shm_fd->duped_fd) > 2){
            if (fstat((armatr_hold->nodemap->shm_fd->duped_fd),&statbuf) != -1){
                close((armatr_hold->nodemap->shm_fd->duped_fd));
            }
            (armatr_hold->nodemap->shm_fd->duped_fd) = 0;
        }

        if (armatr_hold->nodemap->dirnode_fd->path != NULL){
            free(armatr_hold->nodemap->dirnode_fd->path);
        }
        if (armatr_hold->nodemap->entrieslist_fd->path != NULL){
            free(armatr_hold->nodemap->entrieslist_fd->path);
        }
        if (armatr_hold->nodemap->shm_fd->path != NULL){
            free(armatr_hold->nodemap->entrieslist_fd->path);
        }

        free(armatr_hold->nodemap->entrieslist_fd);
        free(armatr_hold->nodemap->dirnode_fd);
        free(armatr_hold->nodemap->shm_fd);
        //free(armatr_hold->nodemap->path);
        free(armatr_hold->nodemap);
        armatr_hold->nodemap = NULL;

        for (k = 0; k < armatr_hold->totsize; k++){
            if (armatr_hold->entries[k].tag){
                free(armatr_hold->entries[k].path);
                armatr_hold->entries[k].path = NULL;
                armatr_hold->entries[k].tag = 0;
            }
        }
        free(armatr_hold->entries);
        free(armatr_hold);
        armatr_hold=NULL;

//       free((*hashlattice)->armature->entries);
//        free((*hashlattice)->armature);
        (*hashlattice)->chains->vessel = (*hashlattice)->chains->vessel->left;
        free((*hashlattice)->chains->vessel->right->diname);
        free((*hashlattice)->chains->vessel->right);
    }

    entr_cnt = 0;
    //DOCS
    (*hashlattice)->chains->vessel = (*dirchains)->dir_head->left;
    cnt = expo_basedir_cnt((*hashlattice)->chains->vessel->did);
    goto_chain_tail((*hashlattice)->chains, 0, NULL, &(*hashlattice)->state);
    if ((*hashlattice)->chains->vessel->left->tag == 7){
        free((*hashlattice)->chains->vessel->left);
    }


    for (i = 0; i< cnt; i++) {
        if (is_base((*hashlattice)->chains->vessel)){
            break;
        }

       // printf("<<FREED: %d\ncount: %d\n>>", entr_cnt, (*hashlattice)->chains->vessel->armature->count);

        if (!i) {
            free((*hashlattice)->chains->vessel->left);
        }

//        free((*hashlattice)->chains->vessel->armature->entries);
//        free((*hashlattice)->chains->vessel->armature);
        (*hashlattice)->chains->vessel = (*hashlattice)->chains->vessel->right;
        free((*hashlattice)->chains->vessel->left->diname);
        free((*hashlattice)->chains->vessel->left);
    }

    (*hashlattice)->chains->vessel = (*hashlattice)->chains->dir_head;
    free((*hashlattice)->chains->vessel->right->diname);
    free((*hashlattice)->chains->vessel->right);
    free((*hashlattice)->chains->vessel->left->diname);
    free((*hashlattice)->chains->vessel->left);
    free((*hashlattice)->chains->vessel->diname);
    free((*hashlattice)->chains->vessel);
    free((*hashlattice)->chains);

    ParaBridge parabrg;
    ParaBridge pbmark = NULL;

    for (i = 0; i<(*hashlattice)->max;i++){
        if((*hashlattice)->bridges[i] != 0){
            if ((*(*hashlattice)->bridges[i]).parabridge != NULL) {
                parabrg = ((*hashlattice)->bridges[i]);
                while (((parabrg)) != NULL) {
                    parabrg = (parabrg)->parabridge;
                    if ((parabrg)->parabridge != NULL){
                        pbmark = parabrg->parabridge;
                    } else
                    {
                        pbmark = NULL;
                    }
                    if ((parabrg)->finode != NULL){
                        //free((parabrg)->finode->finame);
                        //(parabrg)->finode->finame = NULL;
                        free((parabrg)->finode);
                        (parabrg)->finode = NULL;
                    }
                    free(parabrg);
                    parabrg = pbmark;
                    hb_cnt++;
                }
            }
            free((*hashlattice)->bridges[i]);
            hb_cnt++;
        }
    }


    printf("\n-----\nfreed: %d\ncount: %lu\n-----\n",hb_cnt,(*hashlattice)->count);
    free((*hashlattice)->bridges);
    free(*hashlattice);
    free(tbl_list);
    //free(idxlst);

    if (lengths != NULL && paths != NULL) {
        free(lengths);
        free(paths);
    }
    if (cnfdir_fd > 0) {
        close(cnfdir_fd);
    }

}


void destroy_metastructures(Resp_Tbl *rsp_tbl, InfoFrame *infoFrame, LttFlgs reqflg_arr, unsigned char *req_buf,
                            unsigned char *req_arr_buf, unsigned char *tmparrbuf, unsigned char *rsp_buf) {
    if ((rsp_tbl)->rsp_funcarr != NULL){free(((rsp_tbl)->rsp_funcarr));free(rsp_tbl);}
    if (req_arr_buf != NULL) {free(req_buf);}
    if (req_arr_buf != NULL){free(req_arr_buf);}
   // if (tmparrbuf != NULL){free(tmparrbuf);}
    if (rsp_buf != NULL){free(rsp_buf);}
    if (reqflg_arr != NULL){free(reqflg_arr);}
    if (infoFrame != NULL){free(infoFrame);}
}


size_t read_conf(unsigned char **dnconf_addr, int cnfdir_fd, LttcState ltcSt) {
    struct stat sb;
    size_t length;
    ErrorBundle errBndl = init_errorbundle();

    int dnconf_fd = openat(cnfdir_fd, DNCONFFI, O_RDONLY);
    if (dnconf_fd == -1) {
        perror("Error opening config file\n");
        return -1;
    }

    if (fstat(dnconf_fd, &sb) == -1) {
        perror("Error stat-ing conf file\n");
        return -1;
    }

    *dnconf_addr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, dnconf_fd, 0);

    if (*dnconf_addr == MAP_FAILED) {
        errBndl = bundle_addglob(errBndl, MISMAP, "Config mem map failed", 0, 0, "read_conf", NULL, errno);
        close(dnconf_fd);
        return -1;
    }

    close(dnconf_fd);
    return sb.st_size;
}



int nodepaths(unsigned char* dn_conf_addr, int** lengths, unsigned char*** paths){

    int dn_count = *(dn_conf_addr+3);
    *lengths = calloc(dn_count,sizeof(int));
    int byte_strcnt = 7;
    int i;
    int addn = 0;

    int tot_len = 0;

    for (i = 0; i < dn_count; i++){

        *(*lengths+i) = (int) *(dn_conf_addr+byte_strcnt);
        tot_len += *(*lengths+i)+4;
        byte_strcnt += 2;
    }

    byte_strcnt += 6;

    *paths = (unsigned char**) calloc(dn_count, sizeof(unsigned char*));

    for (i = 0; i < dn_count; i++) {

        *(*paths+i) = dn_conf_addr+byte_strcnt;
        byte_strcnt += *(*lengths+i)+1;
    }

    return dn_count;
}

int extract_name(const unsigned char* path, int length) {
    int fs_pos;
    for (int i = 0; i < length; i++) {
        if (*(path+i) == '/') {
            fs_pos = i;
        }
    }
    return fs_pos + 1;
}
//
//// 1 Lattice_PTP hashLattice,
//// 2 Std_Buffer_PTP request_buf,
//// 3 Std_Buffer_PTP response_buf,
//// 4 Std_Buffer_PTP requestArr_buf,
//// 5 Std_Buffer_PTP tempArr_buf,
//// 6 Flags_Buffer_PTP flags_buf,
//// 7 Info_Frame_PTP infoFrame,
//// 8 ResponseTable_PTP responseTable,
//// 9 epEvent epollEvent_IN,
//// 10 pthread_t tid
//// 11 int epollFD,
//// 12 int dataSocket,
//// 13 int buf_len,
//// 14 int tag
//
//void update_SOA(int opt, SOA_Pack* soaPack, void* new_val){
//
//    switch (opt){
//        case 1:
//            (*soaPack)->hashLattice = (Lattice_PTP) new_val;
//            break;
//        case 2:
//            (*soaPack)->request_buf = (Std_Buffer_PTP) new_val;
//            break;
//        case 3:
//            (*soaPack)->response_buf = (Std_Buffer_PTP) new_val;
//            break;
//        case 4:
//            (*soaPack)->requestArr_buf = (Std_Buffer_PTP) new_val;
//            break;
//        case 5:
//            (*soaPack)->tempArr_buf = (Std_Buffer_PTP) new_val;
//            break;
//        case 6:
//            (*soaPack)->flags_buf =  (Flags_Buffer_PTP) new_val;
//            break;
//        case 7:
//            (*soaPack)->infoFrame = (Info_Frame_PTP) new_val;
//            break;
//        case 8:
//            (*soaPack)->responseTable = (ResponseTable_PTP) new_val;
//            break;
//        case 9:
//            (*soaPack)->epollEvent_IN = (epEvent) new_val;
//            break;
//        case 10:
//            (*soaPack)->tid = *((pthread_t*) new_val);
//            break;
//        case 11:
//            (*soaPack)->epollFD = *((int*) new_val);
//            break;
//        case 12:
//            (*soaPack)->dataSocket = *((int*) new_val);
//            break;
//        case 13:
//            (*soaPack)->buf_len = *((int*) new_val);
//            break;
//        case 14:
//            (*soaPack)->tag = *((int*) new_val);
//            break;
//        default:
//            return;
//    }
//}

void update_SOA_DS(SOA_Pack* soaPack, int datasocket){(*soaPack)->dataSocket = datasocket;}

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

    soaPack->hashLattice = hashLattice;
    soaPack->request_buf = request_buf;
    soaPack->response_buf = response_buf;
    soaPack->requestArr_buf = requestArr_buf;
    soaPack->tempArr_buf = tempArr_buf;
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

 /* * * * * * * * * * * * * *
 *  Spawn thread & respond *
* * * * * * * * * * * * * **/

 void* spin_off(void* sp){
/*    typedef struct SpinOffArgsPack{
        Lattice_PTP hashLattice;
        Std_Buffer_PTP request_buf;
        Std_Buffer_PTP response_buf;
        Std_Buffer_PTP requestArr_buf;
        Std_Buffer_PTP tempArr_buf;
        Flags_Buffer_PTP flags_buf;
        Info_Frame_PTP infoFrame;
        epEvent epollEvent_IN;
        int dataSocket;
        int buf_len;
*/

     clock_t ca = clock();
     SOA_Pack soa_pack = (SOA_Pack) sp;

     int exit_flag = 0;
     ErrorBundle errBndl = init_errorbundle();

     LttcState_PTP latticeState =  &(*soa_pack->hashLattice)->state;
     Status_Frame_PTP statusFrame = &(*latticeState)->frame;
     epEvent epoll_event = soa_pack->epollEvent_IN;

     /**
      * READ REQUEST INTO BUFFER
      * */
     ssize_t ret = read(soa_pack->dataSocket, *soa_pack->request_buf, soa_pack->buf_len);  // READ 'R'
     if (ret == -1) {
         errBndl = bundle_addglob(errBndl, BADSOK, "Issue reading from data socket", 0, 0, "read", NULL, errno);
         raiseErr(latticeState,errBndl);
         return NULL;
     }
     (*statusFrame)->status <<= 1;

/**
 * CMD RECEIVED
 * */
     /**
      * PARSE REQUEST
      * */
     *soa_pack->infoFrame = parse_req(*soa_pack->request_buf,   // <<< Raw Request
                                      soa_pack->infoFrame,
                                      statusFrame,
                                      soa_pack->flags_buf,
                                      *soa_pack->tempArr_buf, //TODO: Cleanup data types here
                                      soa_pack->requestArr_buf); // >>> Extracted array

     if ((*statusFrame)->err_code) {
         if ((*statusFrame)->act_id == GBYE){
             exit_flag = 1;
             goto endpoint;
         }
         setErr(statusFrame, MALREQ, 0);
         serrOut(statusFrame, "Failed to process request.");
         goto endpoint;

         // TODO: replace w/ better option.
     }

     /** DETERMINE RESPONSE */
     if (respond(*soa_pack->responseTable,
                 statusFrame,
                 soa_pack->infoFrame,
                 &(*soa_pack->hashLattice)->chains,
                 soa_pack->hashLattice,
                 *soa_pack->response_buf)){
         setErr(statusFrame, MISSPK, 0);
         serrOut(statusFrame, "Failed to stage a response");
         // TODO: replace w/ better option.
     }
     //TODO: Optimize response processing time


     if ((*statusFrame)->act_id == GBYE) {
         /**
          * ACTION:
          *  shutdown
          * */
         setSts(statusFrame, SHTDN, 0);
         exit_flag = 1;
     }else {
         setSts(statusFrame, RESPN, 0);

         /** WRITEOUT REPLY */
         // TODO: Need to handle multiple users using their own buffers or assign them to a queue for writing out.
         if (epoll_wait(soa_pack->epollFD,epoll_event,2,1000)){
             if ((epoll_event->events & EPOLLERR) == EPOLLERR) {
                 bundle_addglob(errBndl, EPOLLE, "Issue detected by EPOLLE", epoll_event->events, NULL, "epoll_wait - out", NULL, errno);
                 raiseErr(latticeState,errBndl);
                 return NULL;
             }
             if (write(soa_pack->dataSocket, soa_pack->response_buf, soa_pack->buf_len) == -1){
                 bundle_addglob(errBndl, BADSOK, "Error writiting response to buffer", soa_pack->dataSocket, "datascoket", "epoll_wait - out", NULL, errno);
                 raiseErr(latticeState,errBndl);
                 return NULL;
             }
         }
     }

     /**
      * ACTION:
      *  save received sequence
      * */
     //TODO: Implement cmd saving

     /**
      * CLEAR CONNECTION
      * */

     clock_t cb = clock();
     printf("\n<<TIME: %lu>>\n",cb-ca);

     endpoint:
     init_infofrm(soa_pack->infoFrame,0);
     bzero(*soa_pack->request_buf, soa_pack->buf_len - 1);
     bzero(soa_pack->flags_buf,FLGSCNT);
     bzero(soa_pack->response_buf, soa_pack->buf_len - 1);
     fsync(soa_pack->dataSocket);
     if (close(soa_pack->dataSocket) == -1){
         bundle_addglob(errBndl, BADSOK, "Error writiting response to buffer", soa_pack->dataSocket, "datasocket", "endpoint", NULL, errno);
         raiseErr(latticeState,errBndl);
         return NULL;
     }

     return soa_pack;
}


 /* * * * * * * * * *
 * Init and listen *
* * * * * * * * * */

int spin_up(unsigned char **rsp_buf, unsigned char **req_arr_buf, unsigned char **req_buf, InfoFrame **infofrm,
            Resp_Tbl **rsp_tbl, HashLattice **hashlattice, DiChains **dirchains, LttFlgs *flgsbuf, const int *cnfdir_fd,
            unsigned char **tmparrbuf, LttcState ltcSt) {

    ErrorBundle errBndl = init_errorbundle();
    pthread_t tid; // Thread id;
    void *t_ret = NULL;  // Returned value from thread.

//    ErrBundle* eb_addr = &errBndl;

    /**
     * INFO AND STATUS VARS
     * */

    *infofrm = init_infofrm(infofrm, 1); // Request/Response Info Frame
    (*dirchains)->vessel = (*dirchains)->dir_head;
    (*infofrm)->vessel = &(*dirchains)->vessel;
    
    int exit_flag = 0;
    int i = 0;
    int k, res;
    unsigned int j;
    ssize_t ret;

    /**
     * BUFFERS INIT
     * */
    const int buf_len = 256;
    const int arrbuf_len = 128;
    *req_buf = (unsigned char *) calloc(buf_len, sizeof(unsigned char));     // client request -> buffer
    *req_arr_buf = (unsigned char *) calloc(arrbuf_len, sizeof(unsigned char));
    *rsp_buf = (unsigned char *) calloc(arrbuf_len, sizeof(unsigned char));
    *flgsbuf = (LttFlgs) calloc(arrbuf_len, sizeof(LattFlag));

    /**
     * SOCKET INIT
     * */
    struct sockaddr_un name;
    int connection_socket;
    int data_socket;

    name.sun_family = AF_UNIX;
    strncpy(name.sun_path, SOCKET_NAME, sizeof(name.sun_path) - 1);

    /**
     * OPEN CONNECTION SOCKET
     * */
    connection_socket = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    if (connection_socket == -1) {
        perror("socket");
        setErr(&ltcSt->frame, BADCON, 'I');
        setAct(&ltcSt->frame, GBYE, 0, 0);
        return 1;
    }
    memset(&name, 0, sizeof(name));
    name.sun_family = AF_UNIX;
    strncpy(name.sun_path, SOCKET_NAME, sizeof(name.sun_path) - 1);

    /**
     * BIND SOCKET TO LISTENING FD
     * */
    ret = bind(connection_socket, (const struct sockaddr *) &name, sizeof(name));
    if (ret == -1) {
        perror("bind");
        setErr(&ltcSt->frame, BADCON, 'B');// 76 = B -> bind step
        setAct(&ltcSt->frame, GBYE, 0, 0);
        return 1;
    }

    /**
     * INIT EPOLL
     * */
    int efd = epoll_create(1);
    if (efd == -1) {
        perror("epoll");
        setErr(&ltcSt->frame, EPOLLE, 'C');
        return 1;
    }
     epEvent epINevent;
     epINevent = (struct epoll_event*) malloc(sizeof(struct epoll_event)*3);
     epINevent->events = EPOLLIN | EPOLLOUT;

    /**
    * LISTEN ON A SOCKET
    * */
    ret = listen(connection_socket, 10);    // LISTEN 'L'
    if (ret == -1) {
        perror("listen: ");
        setErr(&ltcSt->frame, BADCON, 'L'); // 76 = L -> listen step
        setAct(&ltcSt->frame, GBYE, 0, 0);
        return 1;
    }

      /* * * * * * * * *
     *   MAIN START   *
    * * * * * * * * */
    while (!exit_flag) {
        i = 0;
        stsReset(&ltcSt->frame);
        SOA_Pack soaPack = pack_SpinOff_Args(
                0,
                hashlattice,
                req_buf,
                rsp_buf,
                req_arr_buf,
                tmparrbuf,
                flgsbuf,
                infofrm,
                rsp_tbl,
                epINevent,
                efd,
                0,
                buf_len,
                0);

        /**
         * ATTACH EPOLL
         * */
        setSts(&ltcSt->frame, LISTN, 0);
        /**
         * RECIEVE
         * */
        data_socket = accept(connection_socket, NULL, NULL);    // ACCEPT 'A'
        epoll_ctl(efd, EPOLL_CTL_ADD, data_socket, epINevent);

        if (data_socket == -1) {
            perror("accept: ");
            setErr(&ltcSt->frame, BADCON, 'B');// 65 = 'A' -> accept step
        }
        (ltcSt->frame)->status <<= 1;
        update_SOA_DS(&soaPack,data_socket);


    /** Read and Parse */
        /**
         * EPOLL MONITOR DATA CONN
         * */
        if (epoll_wait(efd,epINevent,3,500) > 0){
            if ((epINevent->events&EPOLLERR) == EPOLLERR) {
                errBndl = bundle_addglob(errBndl, EPOLLE, "Error waiting on epoll", epINevent->events, 0, "epoll_wait - in", NULL, errno);
            }
        }
        /** SPAWN THREAD UPON CONN */

        if(pthread_create(&tid, NULL, spin_off, (void*)(soaPack))){
            perror("Error creating thread.");
            exit_flag = 1;
            return 1;
        }
        pthread_join(tid,&t_ret);

        if (ltcSt->frame->status == SHTDN){
            if(soaPack->tag > 1){
                fprintf(stderr,"\n>> pthread (%ld) ret: %lx\n\t>> >> %s\n\n", tid, *((long*) t_ret), (unsigned char*) t_ret);
                pthread_exit(0);
            }
        }

        fprintf(stderr,"\n>> pthread (%ld) ret: %lx\n\t>> >> %s\n\n", tid, *((long*) t_ret), (unsigned char*) t_ret);

    }// END WHILE !EXITFLAG

    epoll_ctl(efd, EPOLL_CTL_DEL, data_socket, epINevent);
    close(connection_socket);
    free(epINevent);

    unlink(SOCKET_NAME);

    return 0;
}
    /* * * * *
      * CLOSE *
       * * * * */



void summon_lattice() {
    ErrorBundle errBndl = init_errorbundle();

    /**
     * INIT GLOBAL STATFRAME
     */
    LttcState latticestate = init_latticestate();

    /**
     * START SODIUM
     * */
    int naclinit = sodium_init();
    if (naclinit != 0) {
        //TODO: REPLACE stsErno with errBundle functions
        bundle_addglob(errBndl, SODIUM, "Sodium init failed", naclinit, 0, "summon_lattice", NULL, errno);
        raiseErr(&latticestate, errBndl);
    }


    /**
     * DECLARATIONS
     * */
    int i = 0;
    int errno_hold;

    unsigned int arrbuf_len = 256;
    unsigned int seqtbl_sz;  // Sequence Table size
    unsigned char *seqstr_addr; // Stored sequences memory mapping

    unsigned char *dn_conf; // Dirnode config memory mapping
    int dn_cnt;             // DirNode count
    unsigned char **paths;  // DirNode filepaths
    int *lengths;           // DirNode filepath lengths
    int nm_len;             // Iterating variable for DirNode name

    unsigned char *req_buf;       // Incoming commands
    unsigned char *tmparrbuf;     // int strings
    unsigned char *req_arr_buf;     // char strings
    unsigned char *rsp_buf;  // Outgoing responses

    LttFlgs reqflg_arr;
    InfoFrame *info_frm;    // Frame for storing request info

    LttcKey latticeKey = sodium_malloc(ULONG_SZ * 2);
    unsigned char* headname = malloc(sizeof(unsigned char)*5);
    unsigned char headnm[5] = {'H','E','A','D','\0'};
    memcpy(headname,headnm,5);

    mk_little_hash_key(&latticeKey);
    dump_little_hash_key(latticeKey,headname,5);
    free(headname);


       /* * * * * * *
      *   BOOT UP  *
     * * * * * * */
    do {
        /**
         * STRUCTURES
         * */
        //  DirNode chains
        DiChains *dirchains = init_dchains();
        //  HashBridge lattice
        HashLattice *hashlattice = init_hashlattice(&dirchains, latticeKey, latticestate);
        //  Size of config file in bytes

        /**
         * OPEN CONFIG
         * */
        int cnfdir_fd = openat(AT_FDCWD, CNFIGPTH, O_RDONLY | O_DIRECTORY);
        if (cnfdir_fd == -1) {
            errno_hold = errno;
            perror("Error in fd: cnfdir_fd: %d");
            setAct(&latticestate->frame, GBYE, NOTHN, 0);
            //stsErno(perror, ltcerr, **sts_frm, erno, misc, *msg, *function, *miscdesc);
//            stsErno(BADCNF, &latticestate->frame, "Failed reading config", 333, 0, NULL, NULL, errno);
            errBndl = bundle_addglob(errBndl,BADCNF, "Failed reading config", 333, 0, NULL, NULL, errno_hold);
            errBndl = raiseErr(&latticestate, errBndl);
            disassemble(&hashlattice, &dirchains, NULL,
                        NULL, 0, NULL, 0,
                        NULL, NULL, 0, 0);
            destroy_metastructures(NULL,
                                   info_frm,
                                   reqflg_arr,
                                   NULL,
                                   req_arr_buf,
                                   tmparrbuf,
                                   NULL);

            break;
        }

        /**
         * READ CONFIG
         * */
        size_t dn_size = read_conf(&dn_conf, cnfdir_fd, NULL);
        if (dn_size == -1) {
            errBndl = bundle_addglob(errBndl, BADCNF,"Failed reading config",dn_size,"dirnode size", "Summon Lattice", NULL, errno);
            errBndl = raiseErr(&latticestate, errBndl);
            destroy_metastructures(NULL,
                                   info_frm,
                                   reqflg_arr,
                                   NULL,
                                   req_arr_buf,
                                   tmparrbuf,
                                   NULL);
            disassemble(&hashlattice,
                        &dirchains,
                        NULL,
                        dn_conf,
                        dn_size,
                        NULL,
                        0,
                        NULL,
                        NULL,
                        0, cnfdir_fd);
            break;
        }

        dn_cnt = nodepaths(dn_conf, &lengths, &paths);
        // Array of FileTables connected to DirNodes
        Armature **tbl_list = (Armature **) calloc(dn_cnt, sizeof(Armature *));


//        errBndl = bundle_addglob(errBndl, ESHTDN, "DirectoryMapping failed", 333, 0, "map_dir", NULL, 0);
//        raiseErr(&latticestate,errBndl);

           /* * * * * * * * * *
          *  BUILD LATTICE  *
         * * * * * * * * **/

        /** BUILD STRUCTURES
         * AND MAP DIRECTORIES.
         **/
        for (i = 0; i < dn_cnt; i++) {
            nm_len = extract_name(*(paths + i), *(lengths + i));

            if (map_dir(&latticestate->frame,
                        (const char *) *(paths + i),
                        nm_len,
                        (*(paths + i) + nm_len),
                        (*(lengths + i) - nm_len),
                        hashlattice,
                        &(tbl_list[i]),
                        latticeKey) < 0){
                errno_hold = errno;
                errBndl = bundle_addglob(errBndl, ESHTDN, "DirectoryMapping failed", 333, 0, "map_dir", NULL, errno_hold);
                raiseErr(&latticestate, errBndl);
                disassemble(&hashlattice,&dirchains,tbl_list,dn_conf,dn_size,NULL,0,lengths,paths,dn_cnt,cnfdir_fd);
                destroy_metastructures(NULL, info_frm, reqflg_arr, req_buf, req_arr_buf, tmparrbuf, rsp_buf);
                return;
            }


        }
         /**
          *  INIT FUNC ARRAY
          * */
         Resp_Tbl* rsp_tbl;
        init_rsptbl(cnfdir_fd,
                    &rsp_tbl,
                    &latticestate->frame,
                    &info_frm,
                    &dirchains,
                    &hashlattice);


         /* * * * * * * * * **
         *  EXECUTE SERVER  *
        * * * * * * * * * **/

        spin_up(&rsp_buf,
                &req_arr_buf,
                &req_buf,
                &info_frm,
                &rsp_tbl,
                &hashlattice,
                &dirchains,
                &reqflg_arr,
                &cnfdir_fd,
                &tmparrbuf,
                latticestate);

        if (latticestate->frame->err_code) {
             fprintf(stderr, "Failure:\nCode: %d\nAct id: %d\nModr: %c\n",
                     latticestate->frame->status, latticestate->frame->act_id, latticestate->frame->modr);
        }

         /**
          * CLEANUP
          * */
        destroy_metastructures(rsp_tbl,
                               info_frm,
                               reqflg_arr,
                               req_buf,
                               req_arr_buf,
                               tmparrbuf,
                               rsp_buf);
        disassemble(&hashlattice,
                    &dirchains,
                    tbl_list,
                    dn_conf,
                    dn_size,
                    NULL,
                    0,
                    lengths,
                    paths,
                    dn_cnt,
                    cnfdir_fd);

    }while (latticestate->frame->status != SHTDN);

    /* * * * * * *
       *   EXIT   *
         * * * * * **/

    stsOut(&latticestate->frame);
    free(latticestate->frame);
    sodium_free(latticeKey);
    pthread_exit(NULL);
}