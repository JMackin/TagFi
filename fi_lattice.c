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
#include "lattice_works.h"
#include "jlm_random.h"
#include "lattice_rsps.h"
#include "lattice_signals.h"
#include "fidi_masks.h"


#define CNFIGPTH "/home/ujlm/CLionProjects/TagFI/config"
#define DNCONFFI "dirnodes"
#define SOCKET_NAME "/tmp/9Lq7BNBnBycd6nxy.socket"
#define CMDCNT 16
#define ARRSZ 256
#define FLGSCNT 16
#define PARAMX 65535

//void load_dir_targets(){
//
//}
unsigned long UISZ = sizeof(unsigned int);
unsigned long UCSZ = sizeof(unsigned char);
int erno;
StatFrame* statusFrame;

StatFrame* init_stat_frm(StatFrame** status_frm)
{
    *status_frm = (StatFrame*) malloc(sizeof(StatFrame));
    (*status_frm)->status=RESET;
    (*status_frm)->err_code=IMFINE;
    (*status_frm)->modr=0;

    return *status_frm;
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
    int entr_cnt = 0;
    int i;
    int j;

    munmap(dnconf_addr, dn_size);
    munmap(seqstr_addr, sq_size);

    unsigned long* idxlst = calloc((PARAMX+(*hashlattice)->max),sizeof(unsigned long));

    for (i =0; i < (*hashlattice)->max; i++){
        if (((*hashlattice)->bridges[i]) != NULL){
            *(idxlst+brdg_cnt) = (*((((*hashlattice)->bridges)[i]))->finode).fhshno;
            ++brdg_cnt;
            free((*(((*hashlattice)->bridges)[i])->finode).finame);
            free((*((*hashlattice)->bridges)[i]).finode);
            //free((((*hashlattice)->chains->vessel->armature->entries)[HTMASK&(*idxlst+brdg_cnt)])->finame);

        }
    }

    //MEDA
    (*hashlattice)->chains->vessel = (*dirchains)->dir_head->right;
    unsigned int cnt = expo_basedir_cnt((*hashlattice)->chains->vessel->did);
    goto_chain_tail((*hashlattice)->chains,1);
    entr_cnt = 0;
    for (i = 0; i< cnt; ++i){

        if ((*hashlattice)->chains->vessel->did == DGMEDAID || (*dirchains)->vessel->did == DGDOCSID) {
            printf("bb");
            break;
        }
        printf("<<FREED: %d\ncount: %d\n>>",entr_cnt,(*dirchains)->vessel->armature->count);

        if (!i){
            free((*hashlattice)->chains->vessel->right);
        }

        //free((*hashlattice)->chains->vessel->armature->entries->finame);
        free((*hashlattice)->chains->vessel->armature->entries);
        free((*hashlattice)->chains->vessel->armature);
        (*hashlattice)->chains->vessel = (*hashlattice)->chains->vessel->left;
        free((*hashlattice)->chains->vessel->right->diname);
        free((*hashlattice)->chains->vessel->right);
    }

    entr_cnt = 0;
    //DOCS
    (*hashlattice)->chains->vessel = (*dirchains)->dir_head->left;
    cnt = expo_basedir_cnt((*hashlattice)->chains->vessel->did);
    goto_chain_tail((*dirchains),0);

    for (i = 0; i< cnt; ++i) {
        if ((*hashlattice)->chains->vessel->did == DGMEDAID || (*dirchains)->vessel->did == DGDOCSID) {
            break;
        }
        entr_cnt = 0;
//        if ((*hashlattice)->chains->vessel->did == DGMEDAID || (*dirchains)->vessel->did == DGDOCSID) {
//            break;
//        }
//        for (int j = 0; j < brdg_cnt; ++j) {
//            if(((*hashlattice)->chains->vessel->armature->entries)[(idxlst[j]) & HTMASK].hshno != 0) {
//                free(((*hashlattice)->chains->vessel->armature->entries)[(idxlst[j]) & HTMASK]);
//                entr_cnt++;
//            }
//
//        }
        printf("<<FREED: %d\ncount: %d\n>>", entr_cnt, (*hashlattice)->chains->vessel->armature->count);

        if (!i){
            free((*hashlattice)->chains->vessel->left);
        }
        free((*hashlattice)->chains->vessel->armature->entries);
        free((*hashlattice)->chains->vessel->armature);
        (*hashlattice)->chains->vessel = (*hashlattice)->chains->vessel->right;
        free((*hashlattice)->chains->vessel->left->diname);
        free((*hashlattice)->chains->vessel->left);
    }

    (*hashlattice)->chains->vessel = (*hashlattice)->chains->dir_head;
    free((*hashlattice)->chains->vessel->right);
    free((*hashlattice)->chains->vessel->left);
    free((*hashlattice)->chains->vessel);
    free((*hashlattice)->chains);

    ParaBridge *parabrg = malloc(sizeof(parabrg));
    ParaBridge* pbmark;

    for (i = 0; i<(*hashlattice)->max;i++){
        if((*hashlattice)->bridges[i] != 0){
            if ((*(*hashlattice)->bridges[i]).parabridge != NULL) {
                *parabrg = &(*(*hashlattice)->bridges[i]);
                while (((*parabrg)->parabridge) != NULL) {
                    pbmark = parabrg;
                    *parabrg = (*parabrg)->parabridge;
                    free((*pbmark)->finode->finame);
                    free((*pbmark)->finode);
                    free(*pbmark);
                    hb_cnt++;
                }
            }
            free((*hashlattice)->bridges[i]);
            hb_cnt++;
        }
    }
    free(parabrg);


    printf("\n-----\nfreed: %d\ncount: %lu\n-----\n",hb_cnt,(*hashlattice)->count);
    free((*hashlattice)->bridges);
    free(*hashlattice);
    free(tbl_list);
    free(idxlst);

    if (lengths != NULL && paths != NULL) {
        free(lengths);
        free(paths);
    }
    if (cnfdir_fd > 0) {
        close(cnfdir_fd);
    }

}


void destroy_metastructures(Resp_Tbl *rsp_tbl,
                            InfoFrame *infoFrame,
                            LttcFlags reqflg_arr,
                            unsigned char* req_buf,
                            unsigned char* req_arr_buf,
                            unsigned char* tmparrbuf,
                            unsigned char* rsp_buf) {
    if ((rsp_tbl)->rsp_funcarr != NULL){free(((rsp_tbl)->rsp_funcarr));}
    if (rsp_tbl != NULL){free(rsp_tbl);}
    if (req_arr_buf != NULL) {free(req_buf);}
    if (req_arr_buf != NULL){free(req_arr_buf);}
   // if (tmparrbuf != NULL){free(tmparrbuf);}
    if (rsp_buf != NULL){free(rsp_buf);}
    if (reqflg_arr != NULL){free(reqflg_arr);}
    if (infoFrame != NULL){free(infoFrame);}
}


size_t read_conf(unsigned char** dnconf_addr, int cnfdir_fd){
    struct stat sb;
    size_t length;

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
        stsErno(EPOLLE, &statusFrame, errno, EPOLLERR, "Issue detected by EPOLLE", "epoll_wait", 0);
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

//VER F.
//StatFrame * spin_up(unsigned int **iarr, unsigned char **req_arr_buf, unsigned char **fullreqbuf, unsigned char **respbuffer,
//                    StatFrame **stsfrm, InfoFrame **infofrm, Seq_Tbl **seq_tbl, Resp_Tbl **rsp_tbl, HashLattice **hashlattice,
//                    Dir_Chains **dirchains, LttcFlags *flgsbuf, const int *cnfdir_fd, unsigned int **tmparrbuf, uniArr **cmdseqarr)

StatFrame * spin_up(unsigned char **rsp_buf, unsigned char **req_arr_buf, unsigned char **req_buf,
                    StatFrame **statusFrame, InfoFrame **infofrm, Resp_Tbl **rsp_tbl, HashLattice **hashlattice,
                    DiChains **dirchains, LttcFlags *flgsbuf, const int *cnfdir_fd, unsigned char **tmparrbuf) {

    /**
     * INFO AND STATUS VARS
     * */

// VER F
//    Cmd_Seq *cmd_seq = NULL;
//    cmd_seq = init_cmdseq(&cmd_seq,cmdseqarr,2);           // Request and Response Frame
    //Cmd_Seq *prev_seq = NULL;          // Keep last request frame.

//    VER G
//    *infofrm = init_info_frm(infofrm,cmdseqarr); // Request/Response Info Frame
    *infofrm = init_info_frm(infofrm); // Request/Response Info Frame
    (*dirchains)->vessel = (*dirchains)->dir_head;
    (*infofrm)->vessel = &(*dirchains)->vessel;

    int exit_flag = 0;
    int i = 0;
    int k, res;
    unsigned int j;
    ssize_t ret;

    int resp_len = 0;

    /**
     * BUFFERS INIT
     * */
    const int buf_len = 256;
    const int arrbuf_len = 128;
    *req_buf = (unsigned char *) calloc(buf_len, sizeof(unsigned char));     // client request -> buffer
    *req_arr_buf = (unsigned char *) calloc(arrbuf_len, sizeof(unsigned char));
    *rsp_buf = (unsigned char *) calloc(arrbuf_len, sizeof(unsigned char));
    *flgsbuf = (LttcFlags) calloc(arrbuf_len, sizeof(LttFlg));

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
        setErr(statusFrame, BADCON, 'I');
        setAct(statusFrame, GBYE, 0, 0);
        return *statusFrame;
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
        setErr(statusFrame, BADCON, 'B');// 76 = B -> bind step
        setAct(statusFrame, GBYE, 0, 0);
        return *statusFrame;
    }

    /**
     * INIT EPOLL
     * */
    int efd = epoll_create(1);
    if (efd == -1) {
        perror("epoll");
        setErr(statusFrame, EPOLLE, 'C');
        return *statusFrame;
    }
     struct epoll_event *epINevent;
     epINevent = (struct epoll_event*) malloc(sizeof(struct epoll_event)*3);
     epINevent->events = EPOLLIN | EPOLLOUT;

    /**
    * LISTEN ON A SOCKET
    * */
    ret = listen(connection_socket, 10);    // LISTEN 'L'
    if (ret == -1) {
        perror("listen: ");
        setErr(statusFrame, BADCON, 'L'); // 76 = L -> listen step
        setAct(statusFrame, GBYE, 0, 0);
        return *statusFrame;
    }

      /* * * * * * * * *
     *   MAIN START   *
    * * * * * * * * */
    while (!exit_flag) {
        i = 0;
        stsReset(statusFrame);


        /**
         * ATTACH EPOLL
         * */
        setSts(statusFrame, LISTN, 0);
        clock_t clka = clock();
        /**
         * RECIEVE
         * */
        data_socket = accept(connection_socket, NULL, NULL);    // ACCEPT 'A'
        epoll_ctl(efd, EPOLL_CTL_ADD, data_socket, epINevent);

        if (data_socket == -1) {
            perror("accept: ");
            setErr(statusFrame, BADCON, 'B');// 65 = 'A' -> accept step
            return *statusFrame;
        }
        (*statusFrame)->status <<= 1;
        clock_t clkb = clock();


    /** Read and Parse */
        /**
         * EPOLL MONITOR DATA CONN
         * */
        if (epoll_wait(efd,epINevent,3,500) > 0){
            if ((epINevent->events&EPOLLERR) == EPOLLERR) {
                stsErno(EPOLLE, statusFrame, errno, epINevent->events, "Issue detected by EPOLLE", "epoll_wait - in", 0);
            }
        }
        clock_t clkc = clock();

        /**
         * READ REQUEST INTO BUFFER
         * */
        ret = read(data_socket, *req_buf, buf_len);  // READ 'R'
        if (ret == -1) {
            stsErno(BADSOK, statusFrame, errno, 0, "Issue reading from data socket", "read", 0);
            return *statusFrame;
        }
        (*statusFrame)->status <<= 1;
        clock_t clkd = clock();

        /**
         * CMD RECEIVED
         * */


        /**|  |||  |||  |||  |||  |||  |||  |||  |||
         * <br>
         * VERSION SPLIT POINT:
         *<br>
         * ALTERNATE REQUEST FLOW
         * <br>
         *|  |||  |||  |||  |||  |||  |||  |||  |||*/

        /**
         * PARSE REQUEST
         * */
        *infofrm = parse_req(*req_buf,   // <<< Raw Request
                             infofrm,
                             statusFrame,
                             flgsbuf,
                             *tmparrbuf,
                             req_arr_buf); // >>> Extracted array

        if ((*statusFrame)->err_code) {
            setErr(statusFrame,MALREQ,0);
            serrOut(statusFrame,"Failed to process request.");
            goto endpoint;

             // TODO: replace w/ better option.
        }

        clock_t clke = clock();

        /** DETERMINE RESPONSE */
        if (respond(*rsp_tbl,
                     statusFrame,
                     infofrm,
                     dirchains,
                     hashlattice,
                     *rsp_buf)){
            setErr(statusFrame,MISSPK,0);
            serrOut(statusFrame,"Failed to stage a response");
            // TODO: replace w/ better option.
        }
        //TODO: Optimize response processing time
        clock_t clkf = clock();


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
                if (epoll_wait(efd,epINevent,2,1000)){
                    if ((epINevent->events&EPOLLERR) == EPOLLERR) {
                        stsErno(EPOLLE, statusFrame, errno, epINevent->events,
                                "Issue detected by EPOLLE", "epoll_wait - in", 0);
                   }
                    ret = write(data_socket, *rsp_buf, buf_len);
                }
        }
        clock_t clkg = clock();


        /**
         * ACTION:
         *  save received sequence
         * */
        //TODO: Implement cmd saving
//        if ((*stsfrm)->act_id == SVSQ) {
//            save_seq(cmd_seq, *cnfdir_fd);
//        }
// VER F


        i++;

        /**
         * CLEAR CONNECTION
         * */

        endpoint:
        bzero(*req_buf, buf_len - 1);
        bzero(*flgsbuf,FLGSCNT);
        bzero(*rsp_buf, arrbuf_len-1);
//VER G
//        bzero(*tmparrbuf,arrbuf_len-1);
        //prev_seq = copy_cmdseq(0,&cmd_seq,&prev_seq,stsfrm);
        //cmd_seq = destroy_cmdseq(stsfrm,&cmd_seq);
//        cmd_seq = reset_cmdseq(&cmd_seq,2);
// VER. F

        close(data_socket);

        /**
         * CLOSE ON SHTDN CMD
         * */
        if ((*statusFrame)->err_code && (*statusFrame)->act_id == GBYE) {
            exit_flag = 1;
            serrOut(statusFrame, NULL);
        }
        clock_t clkh = clock();

        printf("\n-----------------\ntimes:\n accept%ld\n",(clkb-clka));
        printf("epoll:%ld\n",clkc-clkb);
        printf("read:%ld\n",(clkd-clkc));
        printf("parse:%ld\n",(clke-clkd));
        printf("respond:%ld\n",(clkf-clke));
        printf("clearout:%ld\n",(clkg-clkf));
    }// END WHILE !EXITFLAG

    epoll_ctl(efd, EPOLL_CTL_DEL, data_socket, epINevent);
    close(connection_socket);
    free(epINevent);
//    cmd_seq = destroy_cmdseq(stsfrm, &cmd_seq);
    //destroy_cmdseq(stsfrm,&prev_seq);

    unlink(SOCKET_NAME);
    return *statusFrame;
}   /* * * * *
      * CLOSE *
       * * * * */



void summon_lattice() {

    /**
     * INIT GLOBAL STATFRAME
     */
    statusFrame = init_stat_frm(&statusFrame);

    /**
     * START SODIUM
     * */
    int naclinit = sodium_init();
    if (naclinit != 0) {
        setAct(&statusFrame,GBYE,NOTHN,0);
        stsErno(SODIUM, &statusFrame, errno, naclinit, "Sodium init failed", "summon_lattice", 0);
    }

    /**
     * DECLARATIONS
     * */
    int i = 0;


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

    LttcFlags reqflg_arr;
    InfoFrame *info_frm;    // Frame for storing request info
    StatFrame *status_frm; // Frame for storing sys info
    status_frm = init_stat_frm(&status_frm);


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
        HashLattice *hashlattice = init_hashlattice(&dirchains);
        //  Size of config file in bytes


        /**
         * OPEN CONFIG
         * */
        int cnfdir_fd = openat(AT_FDCWD, CNFIGPTH, O_RDONLY | O_DIRECTORY);
        if (cnfdir_fd == -1) {
            perror("Error in fd: cnfdir_fd: %d");
            setAct(&statusFrame,GBYE,NOTHN,0);
            //stsErno(perror, ltcerr, **sts_frm, erno, misc, *msg, *function, *miscdesc);
            stsErno(BADCNF, &statusFrame, errno, 333, "Failed reading config", NULL, 0);
            disassemble(&hashlattice, &dirchains, NULL,
                        NULL, 0, NULL, 0,
                        NULL, NULL, 0, 0);
            serrOut(&status_frm,NULL);
            break;
        }

        /**
         * READ CONFIG
         * */
        size_t dn_size = read_conf(&dn_conf, cnfdir_fd);
        if (dn_size == -1) {
            stsErno(BADCNF, &statusFrame, errno, 333, "Failed reading config", NULL, 0);
            destroy_metastructures(NULL,
                                   info_frm,
                                   reqflg_arr,
                                   NULL,
                                   req_arr_buf,
                                   tmparrbuf,
                                   NULL);
                                 disassemble(&hashlattice, &dirchains, NULL,
                                 dn_conf, dn_size, NULL, 0,
                                 NULL, NULL, 0, cnfdir_fd);
            break;
        }

        dn_cnt = nodepaths(dn_conf, &lengths, &paths);
        // Array of FileTables connected to DirNodes
        Armature **tbl_list = (Armature **) calloc(dn_cnt, sizeof(Armature *));

//        Seq_Tbl *seqTbl;
//        init_seqtbl(&seqTbl, 32);
//VER. F

           /* * * * * * * * * *
          *  BUILD LATTICE  *
         * * * * * * * * **/

        for (i = 0; i < dn_cnt; i++) {
            nm_len = extract_name(*(paths + i), *(lengths + i));

            /** BUILD STRUCTURES
             * AND MAP DIRECTORIES.
             **/
            if (map_dir((const char *) *(paths + i),
                        nm_len,
                        (*(paths + i) + nm_len),
                        (*(lengths + i) - nm_len),
                        dirchains,
                        hashlattice,
                        &(tbl_list[i]))< 0){

                stsErno(MISMAP, &statusFrame, errno, 333, "Big fail", "map_dir", 0);
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
                    &status_frm,
                    &info_frm,
                    &dirchains,
                    &hashlattice);


         /* * * * * * * * * **
         *  EXECUTE SERVER  *
        * * * * * * * * * **/

        status_frm = spin_up(&rsp_buf,
                            &req_arr_buf,
                            &req_buf,
                            &status_frm,
                            &info_frm,
                            &rsp_tbl,
                            &hashlattice,
                            &dirchains,
                            &reqflg_arr,
                            &cnfdir_fd,
                            &tmparrbuf);

        if (status_frm->err_code) {
             fprintf(stderr, "Failure:\nCode: %d\nAct id: %d\nModr: %c\n",
                     status_frm->status, status_frm->act_id, status_frm->modr);
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

    }while (status_frm->status != SHTDN);

    /* * * * * * *
       *   EXIT   *
         * * * * * **/

    stsOut(&status_frm);
    free(status_frm);
}