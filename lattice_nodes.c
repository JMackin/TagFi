#include "lattice_nodes.h"
#include "CryptOps.h"
#include <sodium.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>


//TODO: implement Async IO
// #include <aio.h> --> https://www.gnu.org/savannah-checkouts/gnu/libc/manual/html_node/Asynchronous-I_002fO.html


const char r_strm = 'r';  const int r_mv_fd = O_RDONLY;
const char w_strm = 'w';  const int w_mv_fd = O_WRONLY | O_CREAT | O_TRUNC;
const char a_strm = 'a';  const int a_mv_fd = O_WRONLY | O_CREAT | O_APPEND;
const char R_strm = 'R'; const int R_mv_fd = O_RDWR;
const char W_strm = 'W'; const int W_mv_fd = O_RDWR | O_CREAT | O_TRUNC;
const char A_strm = 'A'; const int A_mv_fd = O_RDWR | O_CREAT | O_APPEND;

const char stream_mode_vars[6] = {r_strm,w_strm,a_strm,R_strm,W_strm,A_strm};
const char stream_mode_str_vars[6][3] = {{'r','\0','\0'},{'w','\0','\0'},{'a','\0','\0'},{'r','+','\0'},{'w','+','\0'},{'a','+','\0'}};
const int fd_mode_vars[6] = {r_mv_fd,w_mv_fd,a_mv_fd,R_mv_fd,W_mv_fd,A_mv_fd};

/*
┼───────┼────────────┼───────────────────────────────┼
│   1   │     r      │ O_RDONLY                      │
┼───────┼────────────┼───────────────────────────────┼
│   2   │     w      │ O_WRONLY | O_CREAT | O_TRUNC  │
┼───────┼────────────┼───────────────────────────────┼
│   3   │     a      │ O_WRONLY | O_CREAT | O_APPEND │
┼───────┼────────────┼───────────────────────────────┼
│   4   │     r+     │ O_RDWR                        │
┼───────┼────────────┼───────────────────────────────┼
│   5   │     w+     │ O_RDWR | O_CREAT | O_TRUNC    │
┼───────┼────────────┼───────────────────────────────┼
│   6   │     a+     │ O_RDWR | O_CREAT | O_APPEND   │
┼───────┼────────────┼───────────────────────────────┼
 */



MultiFormMode ret_oflags_ofaform(uint form, StreamMode streammode){
    //void* mode = NULL;
    MultiFormMode mode;

    if (streammode > 6 || streammode < 1 || form > 2){
        mode.i = -1;
        return mode;
    }

    switch (form) {
        case SINGLECHAR_MODEFORM:
            mode.c = stream_mode_vars[streammode-1];
            break;
        case CHARSTRING_MODEFORM:
            mode.ca[0] = stream_mode_str_vars[streammode - 1][0];
            mode.ca[1] = stream_mode_str_vars[streammode - 1][1];
            mode.ca[2] = stream_mode_str_vars[streammode - 1][2];
            break;
        case INTBITMASK_MODEFORM:
            mode.i = fd_mode_vars[streammode-1];
            break;
        default:
            mode.i = -1;
            return mode;
    }
    return mode;
}

StreamMode set_streammode(LattFD *lattFd, StreamMode streammode) {

    if ((streammode < 7) && (streammode > 0)){
        (*lattFd)->stream_mode = streammode;
    } else {
        return -1;
    }

    MultiFormMode formmode = ret_oflags_ofaform(INTBITMASK_MODEFORM,streammode);
    if (((*lattFd)->o_flgs & formmode.i) !=  formmode.i){
        fprintf(stderr,"streammode specified is incompatible with FD flags.");
        return -1;
    }

    return streammode;
}


__attribute__((unused)) int scanfi(const char* dir_path){
    struct dirent **elist;
    int n;

    n = scandir(dir_path, &elist, NULL, alphasort);
    if (n == -1) {
        perror("scandir");
        return -1;
    }

    while (n--) {
        printf("%s\n", elist[n]->d_name);
        free(elist[n]);
    }
    free(elist);

    return 0;

}

__attribute__((unused)) char chk_fmt(const char** dir_path, int op) {
    return 0;
}

int fd_getstat(const char* dir_path, int op) {


    int dir_fd = open(dir_path, O_DIRECTORY | O_RDONLY | O_NONBLOCK );
    int type;
    struct stat *statbuf = (struct stat*) malloc(sizeof(struct stat));

    if(fstat(dir_fd, statbuf) == -1){
        fprintf(stderr, "errno = %d", errno);
        return -1;
    }

    switch (statbuf->st_mode & S_IFMT) {                                 // x >> 13
        case S_IFBLK:  printf("block device\n");    type = 3; break;//0060000 3
        case S_IFCHR:  printf("character device\n");type = 1; break;//0020000 1
        case S_IFDIR:  printf("directory\n");       type = 2; break;//0040000 2
        case S_IFIFO:  printf("FIFO/pipe\n");       type = 0; break;//0010000 0
        case S_IFLNK:  printf("symlink\n");         type = 5; break;//0120000 5
        case S_IFREG:  printf("regular file\n");    type = 4; break;//0100000 4
        case S_IFSOCK: printf("socket\n");          type = 6; break;//0140000 6
        default:       printf("unknown?\n");        type = -1; break;
    }

    free(statbuf);
    close(dir_fd);

    return type;
}

unsigned long cwd_ino(const char* dir_path) {

    int dir_fd = openat(AT_FDCWD, dir_path, O_DIRECTORY | O_RDONLY | O_NONBLOCK );

    struct stat *statbuf = (struct stat*) malloc(sizeof(struct stat));
    if(fstat(dir_fd, statbuf) == -1){
        fprintf(stderr, "errno = %d", errno);
        free(statbuf);
        close(dir_fd);
        return -1;
    }

    unsigned long ino = statbuf->st_ino;
    free(statbuf);
    close(dir_fd);
    return ino;
}

int open_dnode_fd(char* path, int add_modes, int res_dnodefd){

    if (path == NULL){
        fprintf(stderr,"Provided path is null @open_dnode_fd\n");
        return -1;
    }
    if (res_dnodefd == 0){
        res_dnodefd = AT_FDCWD;
    }

    int dnode_fd = openat(res_dnodefd, path, O_DIRECTORY, add_modes);

    if (dnode_fd < 0 ){
        perror("Failed to produce directory fd @open_dnode_fd");
        fprintf(stderr,"\t>Failed path: %s\n",path);
        return -1;
    }else{
        return dnode_fd;
    }
}

unsigned int check_dir_existence(const char* path){
    if (fd_getstat(path,0) != 2){
        return 1;
    }else{
        return 0;
    }
}

int open_low_fd(char* fipath, int add_ops, int add_modes, int res_dnodefd){

    if (fipath == NULL){
        fprintf(stderr,"Provided path is NULL. @open_low_fd\n");
        return -1;
    }
    if (res_dnodefd == 0){
        res_dnodefd = AT_FDCWD;
    }

    int dnode_fd = openat(res_dnodefd, fipath, add_ops, add_modes);

    if (dnode_fd < 0 ){
        perror("Failed to produce fd @open_low_fd");
        fprintf(stderr,"\t>Failed path: %s\n",fipath);
        return -1;
    }else{
        return dnode_fd;
    }
}

void set_lattfd_empty(LattFD* lattFd){

    if((*lattFd)->path != NULL){
        free((*lattFd)->path);
        (*lattFd)->path=NULL;
    }

    if(((*lattFd))->prime_fd > 2){
        close(((*lattFd))->prime_fd);
    }
    if(((*lattFd))->duped_fd > 2){
        close(((*lattFd))->duped_fd);
    }
    if(((*lattFd))->dir_fd > 2){
        close(((*lattFd))->dir_fd);
    }

    ((*lattFd))->prime_fd = 0;
    ((*lattFd))->duped_fd = 0;
    ((*lattFd))->modes = 0;
    ((*lattFd))->o_flgs = 0;
    ((*lattFd))->dir_fd = 0;
    ((*lattFd))->stream_mode = 0;
    (*lattFd)->tag = 0;
    (*lattFd)->len = 0;
    (*lattFd)->addr = NULL;
}

LattFD open_blank_lattfd(void){
    LattFD lattfd = malloc(sizeof(Lattice_FD));
    lattfd->path = NULL;
    lattfd->addr = NULL;
    lattfd->prime_fd = 0;
    lattfd->duped_fd = 0;
    lattfd->dir_fd = 0;
    set_lattfd_empty(&lattfd);
    return lattfd;
}

int cycle_nodeFD(LattFD* lattFd){

    if ((*lattFd)->duped_fd > 2){
        struct stat statbuf;
        if (fstat((*lattFd)->duped_fd,&statbuf) != -1){
            close((*lattFd)->duped_fd);
        }
        (*lattFd)->duped_fd = 0;
    }

    (*lattFd)->duped_fd = dup((*lattFd)->prime_fd);

    if((*lattFd)->duped_fd == -1){
        perror("fd cycle failed @cycle_nodeFD");
        //fprintf(stderr,"\nPath: %s\n",(*lattFd)->path);
        (*lattFd)->duped_fd;
        return -1;
    }

    return (*lattFd)->duped_fd;
}


/*

    r       │ O_RDONLY
────────────┼───────────────────────────────
    w       │ O_WRONLY | O_CREAT | O_TRUNC
────────────┼───────────────────────────────
    a       │ O_WRONLY | O_CREAT | O_APPEND
────────────┼───────────────────────────────
    r+      │ O_RDWR
────────────┼───────────────────────────────
    w+      │ O_RDWR | O_CREAT | O_TRUNC
────────────┼───────────────────────────────
    a+      │ O_RDWR | O_CREAT | O_APPEND

 */

FILE* open_lattfdstream(LattFD lattfd, StreamMode streammode){
    if (streammode){
        if (set_streammode(&lattfd,streammode) == -1){
            fprintf(stderr,"Error setting streammode to open stream. @open_lattfdstream\n");
        }
    }else{
        if (lattfd->stream_mode == 0){
            set_streammode(&lattfd,A_STRM);
        }
    }

    MultiFormMode mode = ret_oflags_ofaform(CHARSTRING_MODEFORM,lattfd->stream_mode);

    FILE* stream = fdopen(cycle_nodeFD(&lattfd),mode.ca);

    if (stream == NULL){ perror("Failed to make stream from fd held in LattFD. @open_lattfdstream\n"); return NULL;}
    return stream;
}

uint set_fisz(LattFD* lattfd, uint sz){
    if (sz != 0) {
        (*lattfd)->len = sz;
    }else{
        struct stat fi_stat;
        if (fstat(cycle_nodeFD(lattfd), &fi_stat)==-1) {
            perror("Error stat-ing Provided lattFd. @set_fisz\n");
            return 1;
        }
        long fi_sz = fi_stat.st_size;
        (*lattfd)->len = fi_sz;
    }

    return 0;
}

LattFD open_lattfd(char *fipath, int res_dnodefd, int add_ops, int add_modes, uint fi_sz, StreamMode streammode) {
    int fd_hold;

    if(fipath == NULL){
        fprintf(stderr,"Provided path is NULL. @open_lattfd.\n");
        return NULL;
    }

    LattFD lattfd = malloc(sizeof(Lattice_FD));

    if (res_dnodefd == 0){
        res_dnodefd = AT_FDCWD;
    }
    if (streammode){
        MultiFormMode modeform;
        modeform = ret_oflags_ofaform(INTBITMASK_MODEFORM,streammode);
        if (modeform.i == -1){
            fprintf(stderr,"Invalid streammode arg. @open_lattfd\n");
            free(lattfd);
            return NULL;}
        add_ops |= modeform.i;
    }

    fd_hold = open_low_fd(fipath,add_ops,add_modes,res_dnodefd);
    if (fd_hold == -1){
        fprintf(stderr,"Failed to open LattFD prime. @open_lattfd\n");
        fprintf(stderr,"\t>Failed path: %s\n",fipath);
        free(lattfd);
        return NULL;
    }


    lattfd->prime_fd = fd_hold;
    lattfd->modes = add_modes;
    lattfd->o_flgs = add_ops;
    lattfd->duped_fd = 0;
    lattfd->dir_fd = res_dnodefd;
    lattfd->path = fipath;
    lattfd->tag = 1;
    lattfd->addr = NULL;
    lattfd->stream_mode = 0;

    if (fi_sz == 0) {
        struct stat fi_stat;
        if (fstat(cycle_nodeFD(&lattfd), &fi_stat)==-1) {
            perror("Error stat-ing provided lattFd. @open_lattfd\n");
            free(lattfd);
            return NULL;
        }
        fi_sz = fi_stat.st_size;
        lattfd->len = fi_sz;

    }
    return lattfd;
}

uint count_direntrys(LattFD lattFd){

    int cnt = 0;
    //struct dirent *ep;

    if (lattFd->tag != 2){
        fprintf(stderr,"Must provide a type 2 (tag == 2) latt_fd\n");
        return 0;

    }
    DIR *dp = fdopendir(cycle_nodeFD(&lattFd));
    if (dp == NULL){
        perror("Failed to open directory stream for counting");
        return 0;
    }
    while(readdir(dp)){
        ++cnt;
    }
    if(closedir(dp) == -1){
        perror("Failed to close directory stream.");
        return 0;
    }

    return cnt;
}

LattFD open_dir_lattfd(char *fipath, int res_dnodefd, int add_modes, uint entry_cnt) {
    int fd_hold;
    LattFD lattfd = malloc(sizeof(Lattice_FD));

    fd_hold = open_dnode_fd(fipath, add_modes, res_dnodefd);

    if (fd_hold == -1){fprintf(stderr,"Failed to open LattFD prime");return NULL;}
    lattfd->prime_fd = fd_hold;
    lattfd->modes = add_modes;
    lattfd->o_flgs = O_DIRECTORY;
    lattfd->duped_fd = 0;
    lattfd->dir_fd = res_dnodefd;
    lattfd->path = fipath;
    lattfd->stream_mode = 0;
    lattfd->tag = 2;
    lattfd->addr = NULL;

    if(entry_cnt == 0){
        entry_cnt = count_direntrys(lattfd);
    }
    lattfd->len = entry_cnt;


    return lattfd;
}

unsigned int mk_node_list(int entrycnt, LattFD nodeanchor, unsigned char** e_names, unsigned long** e_hashnos){

    int i;
    int idx = 0;
    int j;

    FILE* nanch_strm = open_lattfdstream(nodeanchor,A_STRM);
    if (nanch_strm == NULL){
        return 1;
    }

    char fiIn_buf[512];
    memset(fiIn_buf,0,512);

    for (i = 0; i < entrycnt; i++){
        if(!i){
            memcpy(fiIn_buf,"\n-------\n\0",9);
            idx = 9;
        }
        idx += sprintf(fiIn_buf+idx,"%lu :: ", (*e_hashnos[i]));
        idx += sprintf(fiIn_buf+idx,"%s\n", (e_names[i]));
        j = 0;

        while(idx-j>0){
            fputc(*(fiIn_buf+j),nanch_strm);
            ++j;
        }
        memset(fiIn_buf,0,idx);
        free(*(e_names+i));
        *(e_names+i) = NULL;
        free(*(e_hashnos+i));
        *(e_hashnos+i) = NULL;
        idx = 0;
    }
    fsync(nodeanchor->duped_fd);
    fclose(nanch_strm);
    return 0;
}

/*
 * PROT_FLGS: (default = PROT_READ | PROT_WRITE)
      They include PROT_READ, PROT_WRITE, and PROT_EXEC.
      The special flag PROT_NONE reserves a region of address space for future use.
      The mprotect function can be used to change the protection flags
 * MAP_TYPE: (default = MAP_SHARED)
      contains flags that control the nature of the map. One of MAP_SHARED or MAP_PRIVATE must be specified.
      Others: MAP_FIXED, MAP_ANONYMOUS, MAP_ANON, MAP_HUGETLB, etc...

 * Reference:
      https://www.gnu.org/software/libc/manual/html_node/Memory_002dmapped-I_002fO.html
*/

uint open_shm_lattfd(LattFD *lattFd, void *pref_addr, uint fi_sz, int o_flgs, int mode, int map_type, int prot_flgs) {
    const uint bytesbufmax = 16;
    char* name_buf;
    struct stat shm_stat;
    LattFD newlattFd;
    uint ih_flg = 0;

    if (lattFd == NULL){
        ih_flg = 1;
        newlattFd = (LattFD) malloc(sizeof(Lattice_FD));
        lattFd = &newlattFd;
    }

    ulong* bytes_buf = (ulong*) malloc(bytesbufmax);
    unsigned char* uc_bytes_buf = (unsigned char*) calloc(bytesbufmax+1,UCHAR_SZ);

    rando_sf(bytes_buf);
    memcpy(uc_bytes_buf,bytes_buf,bytesbufmax);
    size_t nmlen = bytes_tostr(&name_buf,uc_bytes_buf,bytesbufmax);
    free(uc_bytes_buf);
    free(bytes_buf);
    if (nmlen == 1){
        fprintf(stderr,"Failed to generate name from bytes.");
        free(name_buf);
        if (ih_flg) {
            free(newlattFd);
        }
        return 1;
    }
    char* name = calloc(nmlen+2,UCHAR_SZ);
    memset(name,'/',1);
    memcpy(name,name_buf,nmlen);
    free(name_buf);

    if ((o_flgs&O_TRUNC)==O_TRUNC || (o_flgs&O_WRONLY)==O_WRONLY){
        fprintf(stderr,"lattFd can't have been opened with O_TRUNC or O_WRONLY flags, or with StreamMode w/w+/a)\n");
        if (ih_flg) {
            free(newlattFd);
        }
        free(name);
        return 2;
    }
//    if ((baseFd->dir_fd == 0) || baseFd->prime_fd == 0 || baseFd->o_flgs == 0){
//        fprintf(stderr,"Undeveloped lattFd used to stage shm.\n");
//        if (ih_flg) {
//            free(lattFd);
//        }
//        free(name);
//        return 3;
//    }
//    if((baseFd->tag) != 1){
//        fprintf(stderr,"lattFd must be of type 1 (tag==1), i.e. connected to a regular file/ directory entry.");
//    }
    if (fi_sz == 0) {
        if (fstat(cycle_nodeFD(lattFd), &shm_stat)) {
            perror("Error stat-ing Provided lattFd\n");
            if (ih_flg) {
                free(lattFd);
            }
            free(name);
            return 4;
        }
        fi_sz = shm_stat.st_size;
    }

    if (map_type == 0){
        map_type = MAP_SHARED;
    }
    if(prot_flgs == 0){
        prot_flgs = PROT_READ | PROT_WRITE;
    }
    (*lattFd)->prime_fd = 0;
    (*lattFd)->modes = mode;
    (*lattFd)->o_flgs = o_flgs;
    (*lattFd)->duped_fd = 0;
    (*lattFd)->dir_fd = 0;
    (*lattFd)->stream_mode = 0;
    (*lattFd)->tag = 8;
    (*lattFd)->addr = NULL;
    (*lattFd)->path = name;
    (*lattFd)->len = fi_sz;

    (*lattFd)->prime_fd = shm_open((*lattFd)->path,(*lattFd)->o_flgs,(*lattFd)->modes);

    if ((*lattFd)->prime_fd == -1){
        perror("Failed to create shm - lattfd");
        free(name);
        if (ih_flg) {
            free(lattFd);
        }
        return 5;
    }

    (*lattFd)->addr = mmap(pref_addr, fi_sz, prot_flgs, map_type, (cycle_nodeFD(lattFd)),0);

    if((*lattFd)->addr == MAP_FAILED){
        perror("mmaping on lattfd shm failed.");
        free(name);
        if (ih_flg) {
            free(lattFd);
        }
        return 6;
    }

    return 0;

}

uint close_shm_lattfd(LattFD* shm_lattfd){

    if (shm_unlink((*shm_lattfd)->path) == -1){
        perror("Failed to unlink shm object from shm_lattfd");
        return 1;
    }
    if(munmap((*shm_lattfd)->addr,(*shm_lattfd)->len)==-1){
        perror("Error un-mmap-ing shm_lattfd");
        return 1;
    }

    set_lattfd_empty(shm_lattfd);
    return 0;
}


uint lattice_span(DNGate* DNMap){
    if ((*DNMap)->shm_fd->tag != 0){
        fprintf(stderr,"Shm already in place for given lattfd, close it first with 'close_shm_lattfd'\n");
        return 1;
    }

    uint res = open_shm_lattfd(&((*DNMap)->shm_fd),
                    NULL,
                    (*DNMap)->entrieslist_fd->len,
                    (*DNMap)->entrieslist_fd->o_flgs,
                    (*DNMap)->entrieslist_fd->modes,
                    0,0);

    if (res != 0){
        fprintf(stderr,"Failed to expand lattice.\n Error: #%d \n",res);
        close_shm_lattfd(&((*DNMap)->shm_fd));
        return 1;
    }
    return 0;
}

uint dump_socketname(int confdir_fd, char* name, char* outfile_name, void* opts, uint namelen){

    if (outfile_name == NULL){
        outfile_name = "cw_socket";
    }

    int fiout = openat(confdir_fd,outfile_name,O_CREAT|O_TRUNC|O_WRONLY,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    if (fiout == -1){
        fprintf(stderr,"Error writing out socketname\n");
        return 1;
    }

    if (write(fiout,name,namelen)==-1){
        perror("Error dumpingsocket name.");
        return 1;
    }

    return 0;
}

/* free_buf:
*      1 = function will free buffer passed in after writing,
*      0 = function will write and return w/o free'ing.
 * */
ssize_t writeto_lattfd(LattFD lattFd, void* out, size_t out_len, uint free_buf){
    ssize_t res;
    uchar_arr wo = (uchar_arr) malloc(out_len);
    memcpy(wo,out,out_len);
    int fd_out = cycle_nodeFD(&lattFd);
    if(fd_out == -1){
        fprintf(stderr,"Failed to cycle lattFd @writeto_lattfd.\n"
                       "Nothing written out.\n");
        if(free_buf){fprintf(stderr,"Output buffer free'd anyway.\n");free(out);}
        free(wo);
        return -1;
    }

    res = write(fd_out,wo,out_len);
    if(res==-1){ perror("Failed writing to lattfd.");}
    else{
        fsync(fd_out);
    }

    if(close(fd_out) == -1){ perror("Failed to close output lattFd @writeto_lattfd.");res = -1;}
    if(free_buf){free(out);}
    free(wo);
    (lattFd)->duped_fd = 0;

    return res;
}

/*
 * close_resdir:
 *      1: Close the FD for the resident directory if there is one
 *      0: Destroy LattFD but don't close resident dir
 */
uint close_and_destroy_lattFD(LattFD_PTP lattfd, uint close_resdir){

    if ((*lattfd)->tag == 8){
        fprintf(stderr,"This function isn't meant to be used on type 8 LattFd's (SHM type). @close_and_destroy_lattFD\n");
        return 1; // 1 = General error, nothing done
    }

    uint res_field = 0;
    uint res_mask = 14;

    if((*lattfd)->prime_fd > 2){
        if(close((*lattfd)->prime_fd)==-1){
            perror("Failed to close prime_fd @ close_and_destroy_lattFD. @close_and_destroy_lattFD");
            res_field |= 2;   // 2 = error cloeing prime fd
        }else{
            (*lattfd)->prime_fd = 0;
        }
    }
    if((*lattfd)->duped_fd > 2){
        if(close((*lattfd)->duped_fd)==-1){
            perror("Failed to close duped_fd @ close_and_destroy_lattFD. @close_and_destroy_lattFD");
            res_field |= 4;   // 4 = error closing dup'ed fd
        }else{
            (*lattfd)->duped_fd = 0;
        }
    }
    if(close_resdir){
        if((*lattfd)->dir_fd > 2){
            if(close((*lattfd)->dir_fd)==-1){
                perror("Failed to close dir_fd @ close_and_destroy_lattFD. @close_and_destroy_lattFD");
                res_field |= 8;   // 8 = error closing dir_fd fd
            }else{
                (*lattfd)->dir_fd = 0;
            }

        }
    }

    if (!(res_field&res_mask)) {
        if ((*lattfd)->path != NULL) {
            free((*lattfd)->path);
            (*lattfd)->path = NULL;
        }
        if ((*lattfd)->addr != NULL) {
            free(((*lattfd)->addr));
            (*lattfd)->addr = NULL;
        }
    }

    ((*lattfd))->modes = 0;
    ((*lattfd))->o_flgs = 0;
    ((*lattfd))->stream_mode = 0;
    (*lattfd)->tag = 0;
    (*lattfd)->len = 0;

    if(!(res_field&res_mask)){
        free(*lattfd);
        lattfd=NULL;
    }
    return res_field;
}



LattFD open_spawnfi(LatticeID_PTA id){

    //char out_path[SPAWNFI_NAME_LEN] = {0};
    char* out_path = (char*) calloc(SPAWNFI_NAME_LEN,UCHAR_SZ);

    memcpy(out_path,*id,LATT_AUTH_BYTES);
    memcpy(out_path+LATT_AUTH_BYTES,SPAWNPIECES_EXT,SPAWNPIECES_EXT_LEN);

    int spawnDir_fd = open_dnode_fd(getenv("SPAWN_DIR"),S_IRWXU,0);
    if (spawnDir_fd == -1){
        fprintf(stderr,"Failed to open spawn directory. @open_spawnfi\n");
        fprintf(stderr,"\t>Failed path: %s\n",getenv("SPAWN_DIR"));
        return NULL;
    }

    LattFD spawnfi_fd = open_lattfd(out_path,spawnDir_fd,O_CREAT|O_RDWR,S_IRWXU,0,0);
    if (spawnfi_fd == NULL){
        fprintf(stderr,"Failed to open LattFD for SpawnFile. @open_spawnfi\n");
        fprintf(stderr,"\t>Failed path: %s\n",out_path);
        return NULL;
    }

    return spawnfi_fd;
}


LattFD open_spawnfi_keyfi(LatticeID_PTA id, uint init){

    //char out_path[SPAWNFI_NAME_LEN] = {0};
    char* out_path = (char*) calloc(SPAWNFI_NAME_LEN,UCHAR_SZ);

    memcpy(out_path,*id,LATT_AUTH_BYTES);
    memcpy(out_path+LATT_AUTH_BYTES,SPAWNPIECES_EXT,SPAWNPIECES_EXT_LEN);

    int spawnDir_fd = open_dnode_fd(getenv("SPAWN_DIR"),S_IRWXU,0);
    if (spawnDir_fd == -1){
        fprintf(stderr,"Failed to open spawn directory. @open_spawnfi\n");
        fprintf(stderr,"\t>Failed path: %s\n",getenv("SPAWN_DIR"));
        free(out_path);
        return NULL;
    }

    LattFD spawnfi_fd = open_lattfd(out_path,spawnDir_fd,O_CREAT|O_RDWR,S_IRWXU,0,0);
    if (spawnfi_fd == NULL){
        fprintf(stderr,"Failed to open LattFD for SpawnFile. @open_spawnfi\n");
        fprintf(stderr,"\t>Failed path: %s\n",out_path);
        close(spawnDir_fd);
        free(out_path);
        return NULL;
    }

    return spawnfi_fd;
}


//#define LEAD_BMARK 0x1f10081f
//#define LEN_HEAD_BMARK 0x7f40207f
//#define LEN_TAIL_BMARK 0x1ff601ff
//#define ITEM_VALS_BMARK 0xffef0b03
//#define ITEM_VALS_SUBLIST_BMARK 0xc0600c18
//#define TERMINATOR_BMARK 0x31a81431
//#define DELIM_BMARK 0x00

// TODO: Implement
//uint dumpto_spawn_fi(LatticeID id){
//
//    ConfigMultiTool cnfmt = init_CNFGMT(SPAWNPIECES_CNT);
//
//}

uint init_spawn_fi2(LatticeID_PTA id){

    uint total_lens[SPAWNPIECES_CNT] = {0};

    ConfigMultiTool cnfmt = init_CNFGMT(SPAWNPIECES_CNT);
    ConfigMT cmt = &cnfmt;
    SpawnPieces piece = PIECE_BODY;
    SpawnPieces* piece_ptr = &piece;
    uint subheader = 1;
    uint subsize = 0;
    uint sizehold;

    LattFD spawnfi_fd = open_spawnfi(id);
    if (spawnfi_fd == NULL){
        fprintf(stderr,"Failed opening/init'ing spawnfi @init_spawn_fi\n");
        return 1;
    }

    // Lead - config start
    cnfmt.add_conf_lead(cmt);
    // Lens - Spawn item lengths
    cnfmt.add_len_seg(cmt);


    // body
    sizehold = 1;
    subheader = 1;
    cnfmt.into_buffer(piece_ptr,cnfmt.ret_typesize(UINT_F),cmt);
    cnfmt.add_newline(cmt);
    cnfmt.add_marker(cmt,SUBLIST);

    cnfmt.into_buffer(&subheader,cnfmt.ret_typesize(UCHAR_F),cmt);  //None
    cnfmt.add_delim(cmt);
    cnfmt.into_buffer(&sizehold,cnfmt.ret_typesize(UINT_F),cmt);
    cnfmt.add_delim(cmt);

    cnfmt.add_marker(cmt,SUBLIST);
    cnfmt.add_newline(cmt);
    total_lens[0] = subsize+2;


    // Sessions
    piece <<= 1;
    cnfmt.into_buffer(piece_ptr,cnfmt.ret_typesize(UINT_F),cmt);
    cnfmt.add_newline(cmt);
    cnfmt.add_marker(cmt,SUBLIST);

    sizehold = cnfmt.ret_typesize(UINT_F);
    cnfmt.into_buffer(&subheader,cnfmt.ret_typesize(UCHAR_F),cmt);  //SessionOps
    cnfmt.add_delim(cmt);
    cnfmt.into_buffer(&sizehold,cnfmt.ret_typesize(UINT_F),cmt);
    cnfmt.add_delim(cmt);
    subsize += sizehold;

    subheader <<= 1;
    sizehold = cnfmt.ret_typesize(ULONG_F);
    cnfmt.into_buffer(&subheader,cnfmt.ret_typesize(UCHAR_F),cmt); // LastRequest
    cnfmt.add_delim(cmt);
    cnfmt.into_buffer(&sizehold,cnfmt.ret_typesize(UINT_F),cmt);
    cnfmt.add_delim(cmt);
    subsize += sizehold;

    subheader <<= 1;
    sizehold = sizeof(SpawnSession);
    cnfmt.into_buffer(&subheader,cnfmt.ret_typesize(UCHAR_F),cmt);  // LatticeState
    cnfmt.add_delim(cmt);
    cnfmt.into_buffer(&sizehold,cnfmt.ret_typesize(UINT_F),cmt);
    cnfmt.add_delim(cmt);
    subsize += sizehold;

    cnfmt.add_marker(cmt,SUBLIST);
    cnfmt.add_newline(cmt);
    total_lens[1] = subsize+6;
    subsize = 0;


    // State
    piece <<= 1;
    cnfmt.into_buffer(piece_ptr,cnfmt.ret_typesize(UINT_F),cmt);
    cnfmt.add_newline(cmt);
    cnfmt.add_marker(cmt,SUBLIST);

    subheader = 1;
    sizehold = sizeof(StatusFrame);
    cnfmt.into_buffer(&subheader,cnfmt.ret_typesize(UCHAR_F),cmt);  //StatusFrame
    cnfmt.add_delim(cmt);
    cnfmt.into_buffer(&sizehold,cnfmt.ret_typesize(UINT_F),cmt);
    cnfmt.add_delim(cmt);
    subsize += sizehold;

    subheader <<= 1;
    sizehold = cnfmt.ret_typesize(ULONG_F);
    cnfmt.into_buffer(&subheader,cnfmt.ret_typesize(UCHAR_F),cmt); // Cur working Dirnode
    cnfmt.add_delim(cmt);
    cnfmt.into_buffer(&sizehold,cnfmt.ret_typesize(UINT_F),cmt);
    cnfmt.add_delim(cmt);
    subsize += sizehold;

    subheader <<= 1;
    sizehold = cnfmt.ret_typesize(ULONG_F);
    cnfmt.into_buffer(&subheader,cnfmt.ret_typesize(UCHAR_F),cmt);  // misc
    cnfmt.add_delim(cmt);
    cnfmt.fill_with_temp(cmt,cnfmt.ret_typesize(UINT_F));
    cnfmt.add_delim(cmt);
    subsize += sizehold;

    cnfmt.add_marker(cmt,SUBLIST);
    cnfmt.add_newline(cmt);
    total_lens[2] = subsize+6;
    subsize = 0;


    // TagMap
    piece <<= 1;
    sizehold = 64;
    cnfmt.into_buffer(piece_ptr,cnfmt.ret_typesize(UINT_F),cmt);
    cnfmt.add_newline(cmt);
    cnfmt.add_marker(cmt,SUBLIST);

    subheader = 1;
    cnfmt.into_buffer(&subheader,cnfmt.ret_typesize(UCHAR_F),cmt);  //spawnbodyBase
    cnfmt.add_delim(cmt);
    cnfmt.into_buffer(&sizehold,cnfmt.ret_typesize(UINT_F),cmt);
    cnfmt.add_delim(cmt);
    subsize += sizehold;

    subheader <<= 1;
    cnfmt.into_buffer(&subheader,cnfmt.ret_typesize(UCHAR_F),cmt);  //kitBase
    cnfmt.add_delim(cmt);
    cnfmt.into_buffer(&sizehold,cnfmt.ret_typesize(UINT_F),cmt);
    cnfmt.add_delim(cmt);
    subsize += sizehold;

    subheader <<= 1;
    cnfmt.into_buffer(&subheader,cnfmt.ret_typesize(UCHAR_F),cmt);  //miscBase
    cnfmt.add_delim(cmt);
    cnfmt.into_buffer(&sizehold,cnfmt.ret_typesize(UINT_F),cmt);
    cnfmt.add_delim(cmt);
    subsize += sizehold;

    cnfmt.add_marker(cmt,SUBLIST);
    cnfmt.add_newline(cmt);
    total_lens[3] = subsize+6;
    subsize = 0;


    // Kit
    piece <<= 1;
    sizehold = 1;
    subheader = 1;
    cnfmt.into_buffer(piece_ptr,cnfmt.ret_typesize(UINT_F),cmt);
    cnfmt.add_newline(cmt);
    cnfmt.add_marker(cmt,SUBLIST);

    cnfmt.into_buffer(&subheader,cnfmt.ret_typesize(UCHAR_F),cmt);  //None
    cnfmt.add_delim(cmt);
    cnfmt.into_buffer(&sizehold,cnfmt.ret_typesize(UINT_F),cmt);
    cnfmt.add_delim(cmt);

    cnfmt.add_marker(cmt,SUBLIST);
    cnfmt.add_newline(cmt);
    total_lens[4] = subsize+2;


    // Latt ID
    piece <<= 1;
    sizehold = 32;
    cnfmt.into_buffer(piece_ptr,cnfmt.ret_typesize(UINT_F),cmt);
    cnfmt.add_newline(cmt);
    cnfmt.add_marker(cmt,SUBLIST);

    cnfmt.into_buffer(&subheader,cnfmt.ret_typesize(UCHAR_F),cmt);
    cnfmt.add_delim(cmt);
    cnfmt.into_buffer(&sizehold,cnfmt.ret_typesize(UINT_F),cmt);
    cnfmt.add_delim(cmt);
    total_lens[5] = subsize+2;

    cnfmt.add_marker(cmt,SUBLIST);
    cnfmt.add_newline(cmt);


    // LattAuthTag64
    piece <<= 1;
    cnfmt.into_buffer(piece_ptr,cnfmt.ret_typesize(UINT_F),cmt);
    cnfmt.add_newline(cmt);
    cnfmt.add_marker(cmt,SUBLIST);

    cnfmt.into_buffer(&subheader,cnfmt.ret_typesize(UCHAR_F),cmt);
    cnfmt.add_delim(cmt);
    cnfmt.into_buffer(&sizehold,cnfmt.ret_typesize(UINT_F),cmt);
    cnfmt.add_delim(cmt);
    total_lens[6] = subsize + 2;

    cnfmt.add_marker(cmt,SUBLIST);
    cnfmt.add_newline(cmt);


    // END LENGTH SEGMENT
    cnfmt.add_marker(cmt,LENTAIL);
    cnfmt.add_newline(cmt);


    /* Item Values Segment */
    piece = 1;
    cnfmt.add_marker(cmt,ITMVALS);
    cnfmt.add_newline(cmt);


    for (int i = 0; i < SPAWNPIECES_CNT; i++) {

        cnfmt.into_buffer(piece_ptr, cnfmt.ret_typesize(UINT_F), cmt);
        cnfmt.add_newline(cmt);
        cnfmt.add_marker(cmt, SUBLIST);
        cnfmt.add_delim(cmt);
        cnfmt.fill_with_temp(cmt, total_lens[i]);
        cnfmt.add_delim(cmt);
        cnfmt.add_marker(cmt, SUBLIST);
        cnfmt.add_newline(cmt);

        piece <<= 1;
    }

    cnfmt.add_marker(cmt,ITMVALS);
    cnfmt.add_newline(cmt);
    cnfmt.add_marker(cmt,TERMINTR);
    cnfmt.add_newline(cmt);

    uint res;
    if (writeto_lattfd(spawnfi_fd,cnfmt.output_buf,cnfmt.pos,1) == -1){
        perror("Error writing to file @init_spawnFI");
        res = 1;
    }else{
        res = 0;
    }

    uint cadlfd_res = close_and_destroy_lattFD(&spawnfi_fd,1);
    if(cadlfd_res !=0 ){
        res |= cadlfd_res;
    }

    return res;

}