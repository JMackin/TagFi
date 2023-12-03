#include "tagfi.h"
#include <sodium.h>
#include <stdio.h>
#include <dirent.h>
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

int scanfiinodes(const char* dir_path) {

    struct dirent *entry;
    DIR *dp;

    dp = opendir(dir_path);

    if (dp == NULL) {
        perror("opendir");
        return -1;
    }

    while ((entry = readdir(dp)))
        if(entry == -1){
            fprintf(stderr, "errno = %d", errno);
            return -1;
        }else {
            printf("%lu\n", (entry->d_ino));
        }

    closedir(dp);
    return 0;
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

int open_node_fd(char* path, int add_modes, int res_dnodefd){

    if (res_dnodefd == 0){
        res_dnodefd = AT_FDCWD;
    }

    int dnode_fd = openat(res_dnodefd, path, O_DIRECTORY, add_modes);

    if (dnode_fd < 0 ){perror("open_node_fd failed");return -1;
    }else{
        return dnode_fd;
    }

}

int open_low_fd(char* fipath, int add_ops, int add_modes, int res_dnodefd){

    if (res_dnodefd == 0){
        res_dnodefd = AT_FDCWD;
    }

    int dnode_fd = openat(res_dnodefd, fipath, add_ops, add_modes);

    if (dnode_fd < 0 ){perror("open_node_fd failed");return -1;
    }else{
        return dnode_fd;
    }
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

LattFD open_lattfd(char *fipath, int res_dnodefd, int add_ops, int add_modes, StreamMode streammode) {
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
//    lattfd->stream_mode = 0;

    return lattfd;
}

LattFD open_dir_lattfd(char* fipath, int res_dnodefd, int add_modes){
    int fd_hold;
    LattFD lattfd = malloc(sizeof(Lattice_FD));

    fd_hold = open_node_fd(fipath,add_modes,res_dnodefd);

    if (fd_hold == -1){fprintf(stderr,"Failed to open LattFD prime");return NULL;}
    lattfd->prime_fd = fd_hold;
    lattfd->modes = add_modes;
    lattfd->o_flgs = O_DIRECTORY;
    lattfd->duped_fd = 0;
    lattfd->dir_fd = res_dnodefd;
    lattfd->path = fipath;
    lattfd->stream_mode = 0;

    return lattfd;
}

unsigned int mk_node_list(int entrycnt, LattFD nodeanchor, unsigned char** e_names, unsigned long** e_hashnos){

    int i;
    int idx = 0;
    int j = 0;
    //int nanch_fd = cycle_nodeFD(&nodeanchor);
    //if (nanch_fd == -1){ perror("Error duplicating node anchor fd");return 1;}

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
