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

__attribute__((unused)) int fd_getstat(const char* dir_path, int op) {


    int dir_fd = open(dir_path, O_DIRECTORY | O_RDONLY | O_NONBLOCK );
    struct stat *statbuf = (struct stat*) malloc(sizeof(struct stat));

    if(fstat(dir_fd, statbuf) == -1){
        fprintf(stderr, "errno = %d", errno);
        return -1;
    }

    switch (statbuf->st_mode & S_IFMT) {                                 // x >> 13
        case S_IFBLK:  printf("block device\n");            break;//0060000 3
        case S_IFCHR:  printf("character device\n");        break;//0020000 1
        case S_IFDIR:  printf("directory\n");               break;//0040000 2
        case S_IFIFO:  printf("FIFO/pipe\n");               break;//0010000 0
        case S_IFLNK:  printf("symlink\n");                 break;//0120000 5
        case S_IFREG:  printf("regular file\n");            break;//0100000 4
        case S_IFSOCK: printf("socket\n");                  break;//0140000 6
        default:       printf("unknown?\n");                break;
    }

    free(statbuf);
    close(dir_fd);

    return 0;
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

    if (res_dnodefd == 0){
        res_dnodefd = AT_FDCWD;
    }

    int dnode_fd = openat(res_dnodefd, path, O_DIRECTORY, add_modes);

    if (dnode_fd < 0 ){perror("open_dnode_fd failed");return -1;
    }else{
        return dnode_fd;
    }

}

int open_low_fd(char* fipath, int add_ops, int add_modes, int res_dnodefd){

    if (res_dnodefd == 0){
        res_dnodefd = AT_FDCWD;
    }

    int dnode_fd = openat(res_dnodefd, fipath, add_ops, add_modes);

    if (dnode_fd < 0 ){perror("open_dnode_fd failed");return -1;
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
        close(((*lattFd))->duped_fd);
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
        perror("fd cycle failed");
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
            fprintf(stderr,"Error setting streammode to open stream");
        }
    }else{
        if (lattfd->stream_mode == 0){
            set_streammode(&lattfd,A_STRM);
        }
    }

    MultiFormMode mode = ret_oflags_ofaform(CHARSTRING_MODEFORM,lattfd->stream_mode);

    FILE* stream = fdopen(cycle_nodeFD(&lattfd),mode.ca);

    if (stream == NULL){ perror("Failed to make stream from fd held in LattFD"); return NULL;}
    return stream;

}

uint set_fisz(LattFD* lattfd, uint sz){
    if (sz != 0) {
        (*lattfd)->len = sz;
    }else{
        struct stat fi_stat;
        if (fstat(cycle_nodeFD(lattfd), &fi_stat)==-1) {
            perror("Error stat-ing Provided lattFd\n");
            return 1;
        }
        long fi_sz = fi_stat.st_size;
        (*lattfd)->len = fi_sz;
    }

    return 0;
}

LattFD open_lattfd(char *fipath, int res_dnodefd, int add_ops, int add_modes, uint fi_sz, StreamMode streammode) {
    int fd_hold;
    LattFD lattfd = malloc(sizeof(Lattice_FD));

    if (res_dnodefd == 0){
        res_dnodefd = AT_FDCWD;
    }
    if (streammode){
        MultiFormMode modeform;
        modeform = ret_oflags_ofaform(INTBITMASK_MODEFORM,streammode);
        if (modeform.i == -1){fprintf(stderr,"Invalid streammode arg.");return NULL;}
        add_ops |= modeform.i;
    }

    fd_hold = open_low_fd(fipath,add_ops,add_modes,res_dnodefd);
    if (fd_hold == -1){fprintf(stderr,"Failed to open LattFD prime");return NULL;}


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
            perror("Error stat-ing Provided lattFd\n");
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


uint lattice_span(DNMap* DNMap){
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

