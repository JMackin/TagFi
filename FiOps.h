#ifndef TAGFI_FIOPS_H
#define TAGFI_FIOPS_H


#define ANCHORNMLEN 65

typedef enum StreamMode{
    // "r"
    r_STRM = 1,
    // "w"
    w_STRM = 2,
    // "a"
    a_STRM = 3,
    // "r+"
    R_STRM = 4,
    // "w+"
    W_STRM = 5,
    // "a+"
    A_STRM = 6
}StreamMode;

typedef struct PathParts {
    unsigned int namelen;
    unsigned int pathlen;
    unsigned int resdirname_len;
    unsigned char* name;
    const char* parentpath;
    char* resdir_name;
}PathParts;

typedef struct Lattice_FD{
    int prime_fd;
    int duped_fd;
    int o_flgs;
    int modes;
    int dir_fd;
    int tag; // 0 = none, 1 = reg, 2 = dir, 4 = cwd-dir, 8 = shm
    long len;
    char* path;
    unsigned char* addr;
    StreamMode stream_mode;

}Lattice_FD;
typedef Lattice_FD* LattFD;
typedef Lattice_FD** LattFD_PTP;

#endif