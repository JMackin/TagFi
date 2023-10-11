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
#include <unistd.h>
#include <string.h>
#include "chkmk_didmap.h"

#define DNCONFFI "/home/ujlm/CLionProjects/TagFI/config/dirnodes"
#define SOCKET_NAME "/tmp/9Lq7BNBnBycd6nxy.socket"


//void load_dir_targets(){
//
//}

void cleanup(HashLattice* hashlattice,
             Dir_Chains* dirchains,
             Fi_Tbl** tbl_list,
             unsigned char* dnconf_addr,
             size_t dn_size,
             int* lengths,
             unsigned char** paths,
             int dn_cnt) {
    // Cleanup
    munmap(dnconf_addr,dn_size);
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

}


size_t read_conf(unsigned char** dnconf_addr){
    struct stat sb;
    size_t length;

    int dnconf_fd = open(DNCONFFI, O_RDONLY);

    if (dnconf_fd == -1) {
        fprintf(stderr,"Error opening config file\n");
        return -1;
    }

    if (fstat(dnconf_fd, &sb) == -1) {

        fprintf(stderr, "Error stat-ing conf file\n");
        return -1;
    }

    *dnconf_addr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, dnconf_fd, 0);

    if (*dnconf_addr == MAP_FAILED) {
        fprintf(stderr, "Error with mmap\n");
        return -1;
    }

//    ssize_t s = write(STDOUT_FILENO,*dnconf_addr,sb.st_size);
//
//    if (s != sb.st_size) {
//        fprintf(stderr, "partial write");
//        return -1;
//    }


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

int spin_up(unsigned char** cbuffer, unsigned long** bigbuffer){

    int connection_socket;
    ssize_t ret;
    const int buf_len = 8;
    const int cbuf_len = 8;
    unsigned char cout = 0;
    int res;

    int cin_len;
    size_t cout_len;
    char* last_idx;
    int i = 0;
    int data_socket;
    int down_flag = 0;
    int end_flag = 0;
    int exit_flag = 0;
    int resp_flag = 0;
    int char_follow = 0;
    char cin;
    int cbuf_readin_len = 0;
    char buffer[9] = {0};
    *cbuffer = (unsigned char*) calloc(cbuf_len,sizeof(unsigned char));
    *bigbuffer = (unsigned long*) calloc(8,sizeof(unsigned long));
    char respbuffer[9] = {0};

    struct sockaddr_un name;
    name.sun_family = AF_UNIX;
    strncpy(name.sun_path, SOCKET_NAME, sizeof(name.sun_path) - 1);

    connection_socket = socket(AF_UNIX, SOCK_SEQPACKET, 0);

    if (connection_socket == -1) {
        perror("socket");
        return -1;
    }
    memset(&name, 0, sizeof(name));

    name.sun_family = AF_UNIX;

    strncpy(name.sun_path, SOCKET_NAME, sizeof(name.sun_path) - 1);

    ret = bind(connection_socket, (const struct sockaddr *) &name,
               sizeof(name));


    if (ret == -1) {
        perror("bind");
        return -1;
    }



    while(!exit_flag) {
        i = 0;
        bzero(buffer,9);
        printf("<Outer>\n");
        ret = listen(connection_socket, 20);
        if (ret == -1) {
            perror("listen");
            return -1;
        }

        data_socket = accept(connection_socket, NULL, NULL);
        if (data_socket == -1) {
            perror("accept");
            return -1;
        }

        /* Wait for next data packet. */
        ret = read(data_socket, buffer, buf_len * sizeof(int));


        while (!end_flag) {
            printf("<Inner>\n");
            if (i > buf_len-1){
                end_flag = 1;
                break;
            }

            printf("%d\n",buffer[i]);
            printf("i = %d\n",i);
            if (ret == -1) {
                perror("read");
                return -1;
            }

            switch (buffer[i]) {
                case 38: //&
                    end_flag = 1;
                    break;
                case 57: //9
                    memccpy(respbuffer,"Bye\0&",'&',8);
                    cout_len = 8;
//                    resp_flag = 1;
                    down_flag = 1;
                    break;
                case 11:
                    char_follow = 1;
                    i++;
                    cin_len = buffer[i] + 1;
                    ret = read(data_socket, *cbuffer, cin_len);
                    break;
                case 90: //Z
                    resp_flag = 1;
                    memccpy(respbuffer,"foobar\0&",'&',8);
                    cout_len = 8;
                    break;
                default:
                    break;
            }

            if (resp_flag) {
                ret = write(data_socket, respbuffer, cout_len);
                if (ret == -1) {
                    perror("write");
                    return -1;
                }
                resp_flag = 0;

//                respbuffer = memset(*respbuffer,0,cout_len);
            }

            if (char_follow) {
                for (int k = 0; k < cin_len; k++) {
                    putchar(*(*cbuffer + k));
                }
                char_follow = 0;
            }
            i++;

            if (down_flag && end_flag){
                exit_flag = 1;
            }

        }
        end_flag = 0;

        bzero(respbuffer,8);
//bzero(buffer,8);
        close(data_socket);


        /* Close socket. */
        if (exit_flag) {

            close(connection_socket);
            /* Unlink the socket. */
            unlink(SOCKET_NAME);
        }
    }
    return 0;

}

void summon_lattice(){

    int naclinit = sodium_init();
    if( naclinit != 0){
        fprintf(stderr, "Sodium init failed: %d",naclinit);
    }

    Dir_Chains* dirchains = init_dchains();
    HashLattice* hashlattice = init_hashlattice();

    unsigned char* dn_conf;
    int dn_cnt;
    int* lengths;
    int nm_len;
    unsigned char** paths;
//char * buffer;
    unsigned char* cbuffer;
    char* respbuffer;
    unsigned long *bigbuffer;


    size_t dn_size = read_conf(&dn_conf);
    if (dn_size == -1) {
        cleanup(hashlattice,dirchains,NULL,dn_conf,dn_size,NULL, NULL, 0);
        exit(-1);
    }
    dn_cnt = nodepaths(dn_conf, &lengths, &paths);
    Fi_Tbl** tbl_list = (Fi_Tbl**) calloc(dn_cnt,sizeof(Fi_Tbl*));
    int res = 0;
    for (int i = 0; i < dn_cnt; i++){
        nm_len = extract_name(*(paths+i),*(lengths+i));
        res = map_dir((const char*) *(paths+i),(*(paths+i)+nm_len),(*(lengths+i)-nm_len),dirchains,hashlattice,&(tbl_list[i]));
        printf(">>%d\n",res);
    }


    //printf("\n\nCOUNT: %d",fitbl->count);

    if (spin_up(&cbuffer, &bigbuffer) < 0) {

        fprintf(stderr,"Failure");
    }
//    if(*buffer != '\0'){
//        free(buffer);
//    }
    free(cbuffer);
//    if(*respbuffer != '\0') {
//        free(respbuffer);
//    }
    if(*bigbuffer != '\0') {
        free(bigbuffer);
    }


    cleanup(hashlattice, dirchains, tbl_list, dn_conf, dn_size, lengths, paths, dn_cnt);


}