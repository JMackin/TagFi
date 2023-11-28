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