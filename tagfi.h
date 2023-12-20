#ifndef TAGFILES_TAGFILES_H
#define TAGFILES_TAGFILES_H
#include <sys/types.h>
#include "FiOps.h"
#include "consts.h"

#define SINGLECHAR_MODEFORM 0
#define CHARSTRING_MODEFORM 1
#define INTBITMASK_MODEFORM 2

/*
┼───────┼────────────┼────────────┼───────────────────────────────┼
│   1   │     r      │     r      │ O_RDONLY                      │
┼───────┼────────────┼────────────┼───────────────────────────────┼
│   2   │     w      │     w      │ O_WRONLY | O_CREAT | O_TRUNC  │
┼───────┼────────────┼────────────┼───────────────────────────────┼
│   3   │     a      │     a      │ O_WRONLY | O_CREAT | O_APPEND │
┼───────┼────────────┼────────────┼───────────────────────────────┼
│   4   │     R      │     r+     │ O_RDWR                        │
┼───────┼────────────┼────────────┼───────────────────────────────┼
│   5   │     W      │     w+     │ O_RDWR | O_CREAT | O_TRUNC    │
┼───────┼────────────┼────────────┼───────────────────────────────┼
│   6   │     A      │     a+     │ O_RDWR | O_CREAT | O_APPEND   │
┼───────┼────────────┼────────────┼───────────────────────────────┼
 */
extern const char stream_mode_vars[6]; // Single Char representation of io modes
extern const char stream_mode_str_vars[6][3]; // Full char string values that can be passed into fdopen(), etc.
extern const int fd_mode_vars[6]; // OR'd O-flags for use in opening file descriptors, can be passed into open(), etc.

extern const char r_strm; extern  const int r_mv_fd;
extern const char w_strm; extern  const int w_mv_fd;
extern const char a_strm; extern  const int a_mv_fd;
extern const char R_strm; extern const int R_mv_fd;
extern const char W_strm; extern const int W_mv_fd;
extern const char A_strm; extern const int A_mv_fd;

typedef union MultiFormMode{
    char c;
    int i;
    char ca[3];
}MultiFormMode;


int scanfiinodes(const char* dir_path);
int fd_getstat(const char* dir_path, int op);
int scanfi(const char* dir_path);
unsigned long cwd_ino(const char* dir_path);
int open_dnode_fd(char *path, int add_modes, int res_dnodefd);
unsigned int mk_node_list(int entrycnt, LattFD nodeanchor, unsigned char** e_names, unsigned long** e_hashnos);
LattFD open_lattfd(char *fipath, int res_dnodefd, int add_ops, int add_modes, uint fi_sz, StreamMode streammode);
int cycle_nodeFD(LattFD* lattFd);
int open_low_fd(char* path, int add_ops, int add_modes, int res_dnodefd);
LattFD open_blank_lattfd(void);
LattFD open_dir_lattfd(char *fipath, int res_dnodefd, int add_modes, uint entry_cnt);
uint close_shm_lattfd(LattFD* shm_lattfd);
uint lattice_span(DNMap* DNMap);
uint set_fisz(LattFD* lattfd,unsigned int sz);


__attribute__((unused)) char chk_fmt(const char** dir_path, int op);

#endif //TAGFILES_TAGFILES_H

