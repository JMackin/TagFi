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
#include <netdb.h>
#include "CryptOps.h"
#include "lattice_rsps.h"
#include "FiDiMasks.h"
#include "lattice_works.h"
#include "lattice_nodes.h"


#define CNFIGPTH "/home/ujlm/CLionProjects/TagFI/config"
#define DNCONFFI "dirnodes"
//#define SOCKET_NAME "/tmp/9Lq7BNBnBycd6nxy.socket"
#define CMDCNT 16
#define ARRSZ 256
#define FLGSCNT 16
#define MAX_EVENTS 64
#define PARAMX 65535

int erno;

uint estab_socket(void* base, char* dest_dir, char** nameout){
    uint strlen;
    uint pos;
    uint extlen = 7;
    char* ext = ".socket";
    if (dest_dir != NULL){
        if (check_dir_existence(dest_dir)){ fprintf(stderr,"Dir invalid - estab-socket()\n");return 1;}
        strlen = strnlen(dest_dir,ARRBUF_LEN-(ULONG_SZ*4));

    }else{
        dest_dir = "/tmp/";
        strlen = 5;
    }
    pos = strlen;

    *nameout = (char*) calloc(ARRBUF_LEN,UCHAR_SZ);
    char* name = gen_socket_name(base);

    memcpy((*nameout),dest_dir,strlen);
    strlen = strnlen(name,ULONG_SZ*4);
    if(pos+strlen > ARRBUF_LEN){
        fprintf(stderr,"Out of buffer - estab-socket()\n");
        free((*nameout));
        free(name);
        return 1;
    }
    memcpy((*nameout)+pos,name,strlen);
    pos += strlen;
    free(name);

    if(pos+extlen > ARRBUF_LEN){
        fprintf(stderr,"Out of buffer - estab-socket()\n");
        free((*nameout));
        return 1;
    }
    memcpy((*nameout)+pos,ext,extlen);
    pos += extlen;

    return pos;
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
    goto_chain_tail((*hashlattice)->chains, 1, NULL, NULL);
    if ((*hashlattice)->chains->vessel->left->tag == 7){
        free((*hashlattice)->chains->vessel->left);
    }

    DNode node;
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

        node = (*hashlattice)->chains->vessel;
        struct stat statbuf;

        //Prime fd's
        if ((node->node_gate->entrieslist_fd->prime_fd) > 2){
            if (fstat((node->node_gate->entrieslist_fd->prime_fd), &statbuf) != -1){
                close((node->node_gate->entrieslist_fd->prime_fd));
            }
            (node->node_gate->entrieslist_fd->prime_fd) = 0;
        }
        if ((node->node_gate->dirnode_fd->prime_fd) > 2){
            if (fstat((node->node_gate->dirnode_fd->prime_fd), &statbuf) != -1){
                close((node->node_gate->dirnode_fd->prime_fd));
            }
            (node->node_gate->dirnode_fd->prime_fd) = 0;
        }
        if ((node->node_gate->shm_fd->prime_fd) > 2){
            if (fstat((node->node_gate->shm_fd->prime_fd), &statbuf) != -1){
                close((node->node_gate->shm_fd->prime_fd));
            }
            (node->node_gate->shm_fd->prime_fd) = 0;
        }

        //Duped fd's
        if ((node->node_gate->entrieslist_fd->duped_fd) > 2){
            if (fstat((node->node_gate->entrieslist_fd->duped_fd), &statbuf) != -1){
                close((node->node_gate->entrieslist_fd->duped_fd));
            }
            (node->node_gate->entrieslist_fd->duped_fd) = 0;
        }
        if ((node->node_gate->dirnode_fd->duped_fd) > 2){
            if (fstat((node->node_gate->dirnode_fd->duped_fd), &statbuf) != -1){
                close((node->node_gate->dirnode_fd->duped_fd));
            }
            (node->node_gate->dirnode_fd->duped_fd) = 0;
        }
        if ((node->node_gate->shm_fd->duped_fd) > 2){
            if (fstat((node->node_gate->shm_fd->duped_fd), &statbuf) != -1){
                close((node->node_gate->shm_fd->duped_fd));
            }
            (node->node_gate->shm_fd->duped_fd) = 0;
        }

        if (node->node_gate->dirnode_fd->path != NULL){
            free(node->node_gate->dirnode_fd->path);
        }
        if (node->node_gate->entrieslist_fd->path != NULL){
            free(node->node_gate->entrieslist_fd->path);
        }
        if (node->node_gate->shm_fd->path != NULL){
            free(node->node_gate->entrieslist_fd->path);
        }

        free(node->node_gate->entrieslist_fd);
        free(node->node_gate->dirnode_fd);
        free(node->node_gate->shm_fd);
        //free(node->node_gate->path);
        free(node->node_gate);
        node->node_gate = NULL;

        for (k = 0; k < node->armatr->totsize; k++){
            if (node->armatr->entries[k].tag){
                free(node->armatr->entries[k].path);
                node->armatr->entries[k].path = NULL;
                node->armatr->entries[k].tag = 0;
            }
        }
        free(node->armatr->entries);
        free(node->armatr);
        node->armatr=NULL;

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
    goto_chain_tail((*hashlattice)->chains, 0, NULL, NULL);
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
    //free(tbl_list);
    //free(idxlst);

    if (lengths != NULL && paths != NULL) {
        free(lengths);
        free(paths);
    }
    if (cnfdir_fd > 0) {
        close(cnfdir_fd);
    }

}

void destroy_rsp_items(Resp_Tbl *rsp_tbl, InfoFrame *infoFrame){
    if (infoFrame != NULL){free(infoFrame);infoFrame=NULL;}
    if ((rsp_tbl)->rsp_funcarr != NULL){free(((rsp_tbl)->rsp_funcarr));free(rsp_tbl);}
}

void destroy_metastructures(LState latticestate) {
    if (latticestate != NULL){free(latticestate);latticestate=NULL;}
}


size_t read_conf(unsigned char **dnconf_addr, int cnfdir_fd, LState ltcSt) {
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
        errBndl = bundle_addglob(errBndl, MISMAP, NULL, "Config mem map failed", 0, 0, "read_conf", NULL, errno);
        close(dnconf_fd);
        return -1;
    }

    close(dnconf_fd);
    return sb.st_size;
}



int nodepaths(unsigned char* dn_conf_addr, int** lengths, unsigned char*** paths){

    //TODO: Update function so as not to use magic numbers

    int dn_count;
    //TODO: Fix error surrounding endianess
    memcpy(&dn_count,(dn_conf_addr+7), UCHAR_SZ);   //Node count @ pos 4-7

    *lengths = (int*) calloc(dn_count,UINT_SZ);

    int byte_strcnt = 12;    // 12 = Lengths section start pos.
    int i;
    int tot_len = 0;

    for (i = 0; i < dn_count; i++){
        //*(*lengths+i) = (int) *(dn_conf_addr+byte_strcnt);
        // Each length is a 4-byte int followed by a single-byte NULL
        memcpy((*lengths+i),(dn_conf_addr+byte_strcnt),UCHAR_SZ);
        tot_len += *(*lengths+i)+2;
        byte_strcnt += 2;
    }
    // End of lengths segment. Lengths tail marker = 4 bytes.

    // Begin Item value segment. Item value flank marker = 4 bytes
    byte_strcnt += 8;
    // 8 bytes = length-seg tail width + item-value starting marker width

    // Paths is a pointer to an array of pointers to item values mem-mapped from the config file
    *paths = (unsigned char**) calloc(dn_count, sizeof(unsigned char*));

    for (i = 0; i < dn_count; i++) {
        // Each value is a pointer to a location in the mmap'd config containing a char-array w/ the node paths.
        *(*paths+i) = dn_conf_addr+byte_strcnt;
        // Each subsequent value exists at len(item)+1 bytes from the start of the previous
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

SpawnKit set_spawnkit(Lattice lattice){
    Vessel vessel;
    TravelPath travelPath;
    BufferPool bufferPool;


    vessel = (Vessel) malloc(sizeof(Vessel));
    travelPath.origin = lattice->chains->dir_head;
    travelPath.destination = NULL;
    init_bufferpool(&bufferPool);
}

 /* * * * * * * * * * * * * *
 *  SPool thread & respond *
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
        int BUF_LEN;
*/
     clock_t ca = clock();

     SOA_Pack soa_pack = (SOA_Pack) sp;
     int exit_flag = 0;
     //init_SOA_bufs(&soa_pack);
     ErrorBundle errBndl = init_errorbundle();
     SSession session = soa_pack->session;
     SState latticeState =  (session)->state;
     StsFrame statusFrame = (latticeState)->frame;
     epEvent epoll_event = soa_pack->epollEvent_IN;
     pthread_mutex_t* lock = soa_pack->lock;

     pthread_mutex_lock(lock);
     /**
      * READ REQUEST INTO BUFFER
      * */
     ssize_t ret = read(soa_pack->dataSocket, *soa_pack->request_buf, soa_pack->buf_len);  // READ 'R'
     if (ret == -1) {
         errBndl = bundle_addglob(errBndl, BADSOK, NULL, "Issue reading from data socket", soa_pack->dataSocket,
                                  "datasocket",
                                  "read", NULL, errno);
         raiseErr(errBndl);
         return NULL;
     }
     pthread_mutex_unlock(lock);
     (statusFrame)->status <<= 1;

/**
 * CMD RECEIVED
 * */
     /**
      * PARSE REQUEST
      * */
     *soa_pack->infoFrame = parse_req(*soa_pack->request_buf,   // <<< Raw Request
                                      soa_pack->infoFrame,
                                      &statusFrame,
                                      soa_pack->flags_buf,
                                      *soa_pack->tempArr_buf, //TODO: Cleanup data types here
                                      soa_pack->requestArr_buf); // >>> Extracted array

     if ((statusFrame)->err_code) {
         if ((statusFrame)->act_id == GBYE){
             (session)->op == SESH_ST_SHTDN;
             exit_flag = 1;
             setSts(&statusFrame, SHTDN, 0);
             goto endpoint;
         }
         setErr(&statusFrame, MALREQ, 0);
         serrOut(&statusFrame, "Failed to process request.");
         goto endpoint;

         // TODO: replace w/ better option.
     }


     /** DETERMINE RESPONSE */
     if (respond(*soa_pack->responseTable,
                 &soa_pack->session,
                 soa_pack->infoFrame,
                 &(*soa_pack->hashLattice)->chains,
                 soa_pack->hashLattice,
                 *soa_pack->response_buf)){
         setErr(&statusFrame, MISSPK, 0);
         serrOut(&statusFrame, "Failed to stage a response");
         // TODO: replace w/ better option.
     }
     //TODO: Optimize response processing time

     if ((statusFrame)->act_id == GBYE) {
         /**
          * ACTION:
          *  shutdown
          * */
         setSts(&statusFrame, SHTDN, 0);
         (session)->op == SESH_ST_SHTDN;
         exit_flag = 1;
     }else {
         setSts(&statusFrame, RESPN, 0);

         /** WRITEOUT REPLY */
         // TODO: Need to handle multiple users using their own buffers or assign them to a queue for writing out.
         //pthread_mutex_lock(lock);
         if (epoll_wait(soa_pack->epollFD,epoll_event,0,100)){
             fprintf(stderr,"\n1!\n");
             if ((epoll_event->events & EPOLLERR) == EPOLLERR) {
                 bundle_addglob(errBndl, EPOLLE, NULL, "Issue detected by EPOLLE", epoll_event->events, NULL,
                                "epoll_wait - out", NULL, errno);
                 raiseErr(errBndl);
                 return NULL;
             }
             fprintf(stderr,"\n2!\n");
             if (write(soa_pack->dataSocket, *(soa_pack->response_buf), soa_pack->buf_len) == -1){
                 bundle_addglob(errBndl, BADSOK, NULL, "Error writiting response to buffer", soa_pack->dataSocket,
                                "datascoket", "epoll_wait - out", NULL, errno);
                 raiseErr(errBndl);
                 return NULL;
             }
             printf("\n>>%s<<\n",*(soa_pack->response_buf));
             fprintf(stderr,"\n3!\n");

         }
         //pthread_mutex_unlock(lock);
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
     if(exit_flag){
         soa_pack->_internal.shtdn=1;
         (session)->op == SESH_ST_SHTDN;
     }

     init_infofrm(soa_pack->infoFrame,0);

     if (close(soa_pack->dataSocket) == -1){
         bundle_addglob(errBndl, BADSOK, NULL, "Error writiting response to buffer", soa_pack->dataSocket, "datasocket",
                        "endpoint", NULL, errno);
         raiseErr(errBndl);
         return NULL;
     }

     pthread_mutex_lock(lock);
     epoll_ctl(soa_pack->epollFD, EPOLL_CTL_DEL, soa_pack->dataSocket, soa_pack->epollEvent_IN);
     pthread_mutex_unlock(lock);

     //destroy_SOA_bufs(&soa_pack);
     discard_SpinOff_Args(&soa_pack);
     end_session(&session);
     pthread_mutex_destroy(lock);

     pthread_exit(&exit_flag);
}


 /* * * * * * * * * *
 * Init and listen *
* * * * * * * * * */

 int spin_up(unsigned char **rsp_buf, unsigned char **req_arr_buf, unsigned char **req_buf, InfoFrame **infofrm,
             Resp_Tbl **rsp_tbl, HashLattice **hashlattice, DiChains **dirchains, const int *cnfdir_fd,
             unsigned char **tmparrbuf, LState ltcSt) {

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

    char* socketName;
    uint namelen = estab_socket(NULL,NULL,&socketName);
    if(namelen == 1){
        bundle_and_raise(errBndl,MISCLC,ltcSt,"Error establishing a named socket. Likely ran out of buffer."
                         ,ARRBUF_LEN,"buffer-length-max",
                         "estab_socket",NULL,0);
        return 1;
    }
    if(dump_socketname(*cnfdir_fd, socketName, NULL, NULL, namelen)){
        free(socketName);
        return 1;
    }


         int exit_flag = 0;
    int i = 0;
    int n_fds;
    int k, res;
    unsigned int j;
    ssize_t ret;
    epEvent epINIT_event;
    epEvent epEvents;

    /**
     * BUFFERS INIT
     * */
    /**
     * SOCKET INIT
     * */
    struct sockaddr_un name;
    int connection_socket;
    int data_socket;

    name.sun_family = AF_UNIX;
    strncpy(name.sun_path, socketName, sizeof(name.sun_path) - 1);

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
    strncpy(name.sun_path, socketName, sizeof(name.sun_path) - 1);

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
     * EPOLL
     */
    make_socket_non_blocking(connection_socket);


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
     /**
  * INIT EPOLL
  * */
     int efd = epoll_create1(0);
     if (efd == -1) {
         perror("epoll");
         setErr(&ltcSt->frame, EPOLLE, 'C');
         return 1;
     }
     epEvents = (struct epoll_event*) malloc(sizeof(struct epoll_event) * MAX_EVENTS);
     epINIT_event = (struct epoll_event*) malloc(sizeof(struct epoll_event) * MAX_EVENTS);

     (epINIT_event)->events = EPOLLIN | EPOLLET;
     epEvents->events = EPOLLIN | EPOLLET;
     epINIT_event->data.fd = connection_socket;

     pthread_mutex_t ep_lock;
     if (pthread_mutex_init(&ep_lock, NULL) != 0) {
         printf("Mutex init has failed\n");
         return 1;
     }

     if (epoll_ctl(efd, EPOLL_CTL_ADD, connection_socket, epINIT_event) == -1) {
         perror("epoll_ctl");
         return 1;
     }

      /* * * * * * * * *
     *   MAIN START   *
    * * * * * * * * */
     while (!exit_flag) {
         i = 0;
         stsReset(&ltcSt->frame);

         /**
          * ATTACH EPOLL
          * */
         setSts(&ltcSt->frame, LISTN, 0);
//         if (epoll_ctl(efd, EPOLL_CTL_ADD, connection_socket, epINIT_event) == -1){
//             perror("epoll_ctl");
//             exit(1);
//         }

         n_fds = epoll_wait(efd, epINIT_event, MAX_EVENTS, -1);
         if ((epINIT_event->events & EPOLLERR) == EPOLLERR ) {
             errBndl = bundle_and_raise(errBndl, EPOLLE, ltcSt, "Error waiting on epoll", epEvents->events,
                                        "epEvents->events",
                                        "epoll_wait - in", NULL, errno);
         }

         for (; i < n_fds; i++){
             if ((epINIT_event[i].events & EPOLLERR) == EPOLLERR) {
                 perror("epoll error\n");
                 close(epINIT_event[i].data.fd);
                 continue;
             }else if (epINIT_event[i].data.fd == connection_socket) {

                 while (1) {
                     struct sockaddr in_addr;
                     socklen_t in_len;
                     int infd;
                     in_len = sizeof in_addr;
                     infd = accept(connection_socket, &in_addr, &in_len);
                     if (infd == -1) {
                         if ((errno == EAGAIN) ||
                             (errno == EWOULDBLOCK)) {
                             /* We have processed all incoming
                                connections. */
                             break;
                         } else {
                             perror("accept");
                             break;
                         }
                     }
                     // Make the incoming socket non-blocking and add it to the list of fds to monitor.
                     make_socket_non_blocking(infd);
                     epEvents->data.fd = infd;
                     epEvents->events = EPOLLIN | EPOLLET | EPOLLOUT;
                     epoll_ctl(efd, EPOLL_CTL_ADD, infd, epEvents);
                 }
                 continue;
             }else{
                 ssize_t count;
                 pthread_t thread;
                 int spawn_socket = epINIT_event[i].data.fd;

                 pthread_mutex_t spawn_lock;
                 if (pthread_mutex_init(&spawn_lock,NULL) != 0){
                     perror(("Error making spawn lock."));
                     abort();
                 }
                 pthread_mutex_lock(&spawn_lock);

                 // TESTING
                 SpawnSecrets secrets;
                 Kit kit = malloc(sizeof(SpawnKit));
//               Spawn spawn_thread(pthread_t thread_id, SpawnLock lock, SpawnNotify notify, uint tag_alpha){
                     //init_spawn_fi2(&id);

                 SSession session = init_session(secrets);
                 pthread_mutex_unlock(&spawn_lock);
                 session = tailor_session(&session, spawn_socket, NULL, NULL);
                 SOA_Pack soaPack = pack_SpinOff_Args(0, hashlattice,
                                                    NULL, NULL, NULL, NULL, NULL,
                                                    infofrm, rsp_tbl, epINIT_event,
                                                    efd, spawn_socket, BUF_LEN, 0, &spawn_lock, session);
                 pthread_mutex_unlock(&spawn_lock);

                 if(pthread_create(&thread, NULL, spin_off, (void*)(soaPack))){
                     perror("pthread");
                     abort();
                 }
                 if(pthread_detach(thread)){
                     perror("pthread detach");
                     abort();
                 }

                 if(ltcSt->frame->status == SHTDN){
                     pthread_mutex_lock(&ep_lock);
                     exit_flag=1;
                     ltcSt->frame->status = SHTDN;
                     //destroy_SOA_bufs(&soaPack);
                     pthread_mutex_unlock(&ep_lock);
                     if (pthread_self() != 1){
                         if (soaPack != NULL) {free(soaPack);soaPack = NULL;}
                         break;
                     }
                 }
                 //discard_SpinOff_Args(&soaPack);
                 //if (soaPack != NULL) {free(soaPack);soaPack = NULL;}
                 continue;
             }
         }
         //(ltcSt->frame)->status <<= 1;

        if (ltcSt->frame->status == SHTDN){
                fprintf(stderr,"\n>> 123\n\n");
        }

    }// END WHILE !EXITFLAG


    pthread_mutex_destroy(&ep_lock);
    fprintf(stderr,"CLOSING\n");
    //close(connection_socket);
    close(efd);
    closefrom(connection_socket);
    free(epINIT_event);
    free(epEvents);
    fprintf(stderr,"CLOSING\n");

     unlink(socketName);
     free(socketName);
     return 0;
}

LState init_lattstate() {
    LState lState = (LState) malloc(sizeof(LatticeState));
    lState->frame = (StsFrame) malloc(sizeof(StatusFrame));
    stsReset(&lState->frame);
    init_Tag(&lState->tag); //TODO: impliment a consistent tag system
    lState->misc = 0;

    return lState;
}


void summon_lattice() {
    ErrorBundle errBndl = init_errorbundle();

    /**
     * INIT GLOBAL STATFRAME
     */
    LState latticestate = init_lattstate();

    /**
     * START SODIUM
     * */
    int naclinit = sodium_init();
    if (naclinit != 0) {
        //TODO: REPLACE stsErno with errBundle functions
        bundle_addglob(errBndl, SODIUM, NULL, "Sodium init failed", naclinit, 0, "summon_lattice", NULL, errno);
        raiseErr(errBndl);
    }

    /**
     * DECLARATIONS
     * */
    int i = 0;
    int errno_hold;

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

    InfoFrame *info_frm;    // Frame for storing request info

    LttcHashKey latticeKey = sodium_malloc(ULONG_SZ * 2);
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
            errBndl = bundle_addglob(errBndl, BADCNF, NULL, "Failed reading config", 333, 0, NULL, NULL, errno_hold);
            errBndl = raiseErr(errBndl);
            disassemble(&hashlattice, &dirchains, NULL,
                        NULL, 0, NULL, 0,
                        NULL, NULL, 0, 0);
            destroy_metastructures(latticestate);

            break;
        }

        /**
         * READ CONFIG
         * */
        size_t dn_size = read_conf(&dn_conf, cnfdir_fd, NULL);
        if (dn_size == -1) {
            errBndl = bundle_addglob(errBndl, BADCNF, NULL, "Failed reading config", dn_size, "dirnode size",
                                     "Summon Lattice", NULL, errno);
            errBndl = raiseErr(errBndl);
            destroy_metastructures(latticestate);
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
        DNodeArmature **tbl_list = (DNodeArmature **) calloc(dn_cnt, sizeof(DNodeArmature *));


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
                errBndl = bundle_addglob(errBndl, ESHTDN, NULL, "DirectoryMapping failed", 333, 0, "map_dir", NULL,
                                         errno_hold);
                raiseErr(errBndl);
                disassemble(&hashlattice,&dirchains,tbl_list,dn_conf,dn_size,NULL,0,lengths,paths,dn_cnt,cnfdir_fd);
                destroy_metastructures(latticestate);
                return;
            }

        }
         /**
          *  INIT FUNC ARRAY
          * */
         Resp_Tbl* rsp_tbl;
        init_rsptbl(cnfdir_fd,
                    &rsp_tbl,
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
                &cnfdir_fd,
                &tmparrbuf,
                latticestate);

        if (latticestate->frame->err_code) {
             fprintf(stderr, "Failure:\nCode: %d\nAct id: %d\nModr: %c\n",
                     latticestate->frame->status, latticestate->frame->act_id, latticestate->frame->modr);
        }
        if (latticestate->frame->act_id == GBYE) {
            fprintf(stderr, "Shutting down, so long.\n");
//                    latticestate->frame->status, latticestate->frame->act_id, latticestate->frame->modr);
            latticestate->frame->status = SHTDN;
        }

         /**
          * CLEANUP
          * */
//        destroy_metastructures(rsp_tbl,
//                               info_frm,
//                               reqflg_arr,
//                               req_buf,
//                               req_arr_buf,
//                               tmparrbuf,
//                               rsp_buf);
        destroy_rsp_items(rsp_tbl,info_frm);
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
    free(latticestate);
    sodium_free(latticeKey);
    pthread_exit(NULL);
}