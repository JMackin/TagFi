//
// Created by ujlm on 10/6/23.
//
#include "sodium.h"
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
#include "chkmk_didmap.h"
#include "lattice_cmds.h"
#include "jlm_random.h"

#define CNFIGPTH "/home/ujlm/CLionProjects/TagFI/config"
#define DNCONFFI "dirnodes"
#define SOCKET_NAME "/tmp/9Lq7BNBnBycd6nxy.socket"


//void load_dir_targets(){
//
//}
unsigned long UISZ = sizeof(unsigned int);
unsigned long UCSZ = sizeof(unsigned char);


StatFrame* init_stat_frm(StatFrame** status_frm)
{
    *status_frm = (StatFrame*) malloc(sizeof(StatFrame));
    (*status_frm)->status=RESET;
    (*status_frm)->err_code=IMFINE;
    (*status_frm)->modr=0;

    return *status_frm;
}


void cleanup(HashLattice* hashlattice,
             Dir_Chains* dirchains,
             Fi_Tbl** tbl_list,
             unsigned char* dnconf_addr,
             size_t dn_size,
             unsigned char* seqstr_addr,
             size_t sq_size,
             int* lengths,
             unsigned char** paths,
             int dn_cnt,
             const int cnfdir_fd) {

    // Cleanup
    munmap(dnconf_addr,dn_size);
    munmap(seqstr_addr,sq_size);
    destryohashlattice(hashlattice);
    if (tbl_list != NULL) {
       for (int i = 0; i < dn_cnt; i++) {
            if (tbl_list[i] != 0) {
                free(tbl_list[i]->entries);
                free(tbl_list[i]);
            }
        }
    }
    free(tbl_list);
    destroy_chains(dirchains);
    free(dirchains);
    if (lengths != NULL && paths != NULL) {
        free(lengths);
        free(paths);
    }
         if (cnfdir_fd > 0){
            close(cnfdir_fd);
        }
}


void destroy_metastructures(StatFrame* statFrame,
                            InfoFrame* infoFrame){
    free(statFrame);

    if (infoFrame->cmdSeq){
        free(infoFrame->cmdSeq);
    }
    free(infoFrame);
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
        perror("Error with mmap\n");
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


StatFrame* spin_up(unsigned int** iarr,
            unsigned char** carr,
            unsigned char** buffer,
            unsigned char** respbuffer,
            StatFrame** status_frm,
            InfoFrame** info_frm,
            Seq_Tbl** seq_tbl,
            const int* cnfdir_fd) {

    /**
     * INFO AND STATUS VARS
     * */
    Cmd_Seq *cmd_seq;           // Request and Response Frame
    *info_frm = init_info_frm(info_frm); // Request/Response Info Frame

    int exit_flag = 0;
    int i = 0;
    int k;
    ssize_t ret;
    int resp_len = 0;

    /**
     * BUFFERS INIT
     * */
    const int buf_len = 256;
    const int arrbuf_len = 128;
    *respbuffer = (unsigned char *) calloc((buf_len), sizeof(unsigned char));
    *buffer = (unsigned char *) calloc(buf_len, sizeof(unsigned char));
    *iarr = (unsigned int *) calloc(arrbuf_len, sizeof(unsigned int));
    *carr = (unsigned char *) calloc(arrbuf_len, sizeof(unsigned char));

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
        setErr(status_frm, BADCON, 'I');
        setAct(status_frm, GBYE, 0, 0);
        return *status_frm;
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
        setErr(status_frm, BADCON, 'B');// 76 = B -> bind step
        setAct(status_frm, GBYE, 0, 0);
        return *status_frm;
    }

    /**
     * INIT EPOLL
     * */
    int efd = epoll_create(1);
    if (efd == -1) {
        perror("epoll");
        setErr(status_frm, EPOLLE, 'C');
        return *status_frm;
    }
    struct epoll_event *epevent;
    epevent = malloc(sizeof(struct epoll_event) * 3);
    epevent->events = EPOLLIN | EPOLLOUT;


      /* * * * * * * * *
     *   MAIN START   *
    * * * * * * * * */
    while (!exit_flag) {
        i = 0;
        stsReset(status_frm);

        /**
         * LISTEN ON A SOCKET
         * */
        ret = listen(connection_socket, 20);    // LISTEN 'L'
        if (ret == -1) {
            perror("listen: ");
            setErr(status_frm, BADCON, 'L'); // 76 = L -> listen step
            setAct(status_frm, GBYE, 0, 0);
            return *status_frm;
        }

        /**
         * ATTACH EPOLL
         * */
        setSts(status_frm, LISTN, 0);
        epoll_ctl(efd, EPOLL_CTL_ADD, data_socket, epevent);
        clock_t clka = clock();
        /**
         * RECIEVE
         * */
        data_socket = accept(connection_socket, NULL, NULL);    // ACCEPT 'A'
        if (data_socket == -1) {
            perror("accept: ");
            setErr(status_frm, BADCON, 'B');// 65 = 'A' -> accept step
            return *status_frm;
        }
        (*status_frm)->status <<= 1;
        clock_t clkb = clock();


        /**
         * EPOLL MONITOR DATA CONN
         * */
        k = 0;
        if (epoll_wait(efd, epevent, 2, 1000) > 0) {
            do {
                if ((epevent->events & EPOLLERR) == EPOLLERR) {
                    perror("Epoll at read\n");
                    fprintf(stderr, "> %d\n", (epevent->events));
                    setErr(status_frm, EPOLLE, k++);
                    if ((*status_frm)->err_code == EPOLLE && k > 2) {
                        setErr(status_frm, EPOLLE, EPOLLERR);
                        setAct(status_frm, GBYE, 0, 0);
                        exit_flag = 1;
                        break;
                    }
                }
                break;
            } while ((epevent->events & EPOLLIN) != EPOLLIN);
        }
        clock_t clkc = clock();

        /**
         * READ REQUEST INTO BUFFER
         * */
        ret = read(data_socket, *buffer, buf_len);  // READ 'R'
        if (ret == -1) {
            perror("read: ");
            setErr(status_frm, BADSOK, 'R');
            return *status_frm;
        }
        (*status_frm)->status <<= 1;
        clock_t clkd = clock();

        /**
         * CMD RECEIVED
         * */
        *info_frm = parse_req(*buffer, &cmd_seq, *info_frm, status_frm); // PARSE

        clock_t clke = clock();

        if ((*status_frm)->err_code) {
            fprintf(stderr, "Error processing request: %d\n", (*status_frm)->err_code);
            fprintf(stderr, "lead: %d\nbuffer: %s", (*cmd_seq).lead, *buffer);
        }

        /** Determine response*/


        /**
         * ACTION:
         *  shutdown
         * */
        if ((*status_frm)->act_id == GBYE) {
            setSts(status_frm, SHTDN, 0);
            exit_flag = 1;
            return *status_frm;
        }

        /**
         * ACTION:
         *  save received sequence
         * */
        if ((*status_frm)->act_id == SVSQ) {
            save_seq(cmd_seq, seq_tbl, *cnfdir_fd);
        }

        /**
         * ACTION:
         * prepare response
         * */
        if ((*status_frm)->act_id == FRSP) {
            //prepresp()

            ret = write(data_socket, respbuffer, resp_len);
            if (ret == -1) {
                setErr(status_frm, BADSOK, 'W'); // W = write op
                perror("write");
                return *status_frm;
            }
        }
        i++;

        /**
         * CLEAR CONNECTION
         * */
        bzero(*buffer, buf_len);
        bzero(*respbuffer, buf_len);
        bzero(*iarr, arrbuf_len);
        bzero(*carr, arrbuf_len);
        close(data_socket);


        /**
         * CLOSE ON SHTDN CMD
         * */
        if ((*status_frm)->err_code && (*status_frm)->act_id == GBYE) {
            exit_flag = 1;
            serrOut(status_frm,NULL);
        }
        clock_t clkf = clock();

        printf("time accept%ld\n",(clkb-clka));
        printf("time epoll:%ld\n",(clkc-clkb));
        printf("time read:%ld\n",(clkd-clkc));
        printf("time parse:%ld\n",(clke-clkd));
        printf("time close:%ld\n",(clkf-clke));


    }// END WHILE !EXITFLAG

    close(connection_socket);
    unlink(SOCKET_NAME);
    free(cmd_seq);
    return *status_frm;
}   /* * * * *
      * CLOSE *
       * * * * */



void summon_lattice() {

    /**
     * START SODIUM
     * */
    int naclinit = sodium_init();
    if (naclinit != 0) {
        fprintf(stderr, "Sodium init failed: %d", naclinit);
    }

    /**
     * DECLARATIONS
     * */
    int res = 0;
    int i = 0;
    unsigned int seqtbl_sz;  // Sequence Table size
    unsigned char *seqstr_addr; // Stored sequences memory mapping

    unsigned char *dn_conf; // Dirnode config memory mapping
    int dn_cnt;             // DirNode count
    unsigned char **paths;  // DirNode filepaths
    int *lengths;           // DirNode filepath lengths
    int nm_len;             // Iterating variable for DirNode name

    unsigned char *buffer;       // Incoming commands
    unsigned int *iarr_buf;     // int strings
    unsigned char *carr_buf;     // char strings
    unsigned char *respbuffer;  // Outgoing responses

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
        Dir_Chains *dirchains = init_dchains();
        //  HashBridge lattice
        HashLattice *hashlattice = init_hashlattice();
        //  Size of config file in bytes
        Seq_Tbl *seqTbl;
        init_seqtbl(&seqTbl, 32);
        //  Response array

        /**
         * OPEN CONFIG
         * */
        int cnfdir_fd = openat(AT_FDCWD, CNFIGPTH, O_RDONLY | O_DIRECTORY);
        if (cnfdir_fd == -1) {
            perror("Error in fd: cnfdir_fd: %d");
            setErr(&status_frm, FIFAIL, 'd'); // 'd' = confid directory
            setAct(&status_frm, GBYE, 0, 0);
            cleanup(hashlattice, dirchains, NULL,
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
            perror("Error dn_conf: ");
            setErr(&status_frm, BADCNF, 0); // 11 = dirnode conf
            setAct(&status_frm, GBYE, 0, 0);
            cleanup(hashlattice, dirchains, NULL,
                    dn_conf, dn_size, NULL, 0,
                    NULL, NULL, 0, cnfdir_fd);
            serrOut(&status_frm,NULL);
            break;
        }
        //    if (sq_size == -1 ) {
        //        perror("Error seqstr_addr: ");
        //        close(cnfdir_fd);
        //        cleanup(hashlattice, dirchains, NULL,
        //                dn_conf, dn_size, seqstr_addr, sq_size,
        //                NULL, NULL, 0);
        //        exit(-1);
        //    }    //  Number of DirNodes

        dn_cnt = nodepaths(dn_conf, &lengths, &paths);
        // Array of FileTables connected to DirNodes
        Fi_Tbl **tbl_list = (Fi_Tbl **) calloc(dn_cnt, sizeof(Fi_Tbl *));


           /* * * * * * * * * *
          *  BUILD LATTICE  *
         * * * * * * * * **/
        for (i = 0; i < dn_cnt; i++) {
            nm_len = extract_name(*(paths + i), *(lengths + i));
            //  Build structures and map each DirNode.
            if (map_dir((const char *) *(paths + i),
                        nm_len,
                        (*(paths + i) + nm_len),
                        (*(lengths + i) - nm_len),
                        dirchains,
                        hashlattice,
                        &(tbl_list[i])) < 0) {
                cleanup(hashlattice, dirchains, tbl_list, dn_conf, dn_size, NULL, 0, lengths, paths, dn_cnt, cnfdir_fd);
                perror("DirNode mapping failed\n");
                setErr(&status_frm, MISMAP, 0);
                setAct(&status_frm, GBYE, 0, 0);
                serrOut(&status_frm,NULL);
                destroy_cmdstructures(buffer, respbuffer, carr_buf, iarr_buf, NULL, seqTbl);
                destroy_metastructures(status_frm, info_frm);
                return;
            }
        }
         /**
          *  INIT FUNC ARRAY
          * */
         Resp_Tbl* rsp_tbl;

         init_rsptbl(cnfdir_fd, &rsp_tbl, &status_frm, &info_frm, &dirchains, &hashlattice, (uniArr *) respbuffer);


           /* * * * * * * * * * *
          *   EXECUTE SERVER   *
         ** * * * * * * * * **/
         status_frm = spin_up(&iarr_buf, &carr_buf, &buffer, &respbuffer, &status_frm, &info_frm, &seqTbl,
                              &cnfdir_fd);
         if (status_frm->err_code) {
             fprintf(stderr, "Failure:"
                             "\nCode: %d"
                             "\nAct id: %d"
                             "\nModr: %c\n", status_frm->status, status_frm->act_id, status_frm->modr);
         }

         /**
          * CLEANUP
          * */
         destroy_cmdstructures(buffer, respbuffer, carr_buf, iarr_buf, rsp_tbl, seqTbl);
         cleanup(hashlattice, dirchains, tbl_list, dn_conf, dn_size, NULL, 0, lengths, paths, dn_cnt, cnfdir_fd);

    }while (status_frm->status != SHTDN);
    /* * * * * * *
       *   EXIT   *
         * * * * * **/

    destroy_metastructures(status_frm, info_frm);
}