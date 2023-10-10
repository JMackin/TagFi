//
// Created by ujlm on 10/6/23.
//
#include "sodium.h"
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include "chkmk_didmap.h"

#define DNCONFFI "/home/ujlm/CLionProjects/TagFI/config/dirnodes"


//void load_dir_targets(){
//
//}

void cleanup(HashLattice* hashlattice,
             Dir_Chains* dirchains,
             Fi_Tbl** tbl_list,
             unsigned char* dnconf_addr,
             size_t dn_size,
             int* lengths,
             unsigned char** paths) {
    // Cleanup
    munmap(dnconf_addr,dn_size);

    destryohashlattice(hashlattice);
    for (int i = 0; i < 63; i++) {
        if (tbl_list[i] != 0) {
            free(tbl_list[i]->entries);
        }
    }
//    free(tbl_list);
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

    *paths = calloc(sizeof(unsigned char*), dn_count);

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

void summon_lattice(){

    int naclinit = sodium_init();
    if( naclinit != 0){
        fprintf(stderr, "Sodium init failed: %d",naclinit);
    }

    Dir_Chains* dirchains = init_dchains();
    HashLattice* hashlattice = init_hashlattice();
    Fi_Tbl* tbl_list[63] = {0};
    unsigned char* dn_conf;
    int dn_cnt;
    int* lengths;
    int nm_len;
    unsigned char** paths;

    size_t dn_size = read_conf(&dn_conf);
    if (dn_size == -1) {
        cleanup(hashlattice,dirchains,tbl_list,dn_conf,dn_size,NULL, NULL);
        exit(-1);
    }
    dn_cnt = nodepaths(dn_conf, &lengths, &paths);

    for (int i = 0; i < dn_cnt; i++){
        nm_len = extract_name(*(paths+i),*(lengths+i));
        tbl_list[i] = map_dir(*(paths+i),(*(paths+i)+nm_len),(*(lengths+i)-nm_len),dirchains,hashlattice);
    }

    //printf("\n\nCOUNT: %d",fitbl->count);

    cleanup(hashlattice, dirchains, tbl_list, dn_conf, dn_size, lengths, paths);


}