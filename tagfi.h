#ifndef TAGFILES_TAGFILES_H
#define TAGFILES_TAGFILES_H
#include <sys/types.h>

int scanfiinodes(const char* dir_path);
int fd_getstat(const char* dir_path, int op);
int scanfi(const char* dir_path);


__attribute__((unused)) char chk_fmt(const char** dir_path, int op);

#endif //TAGFILES_TAGFILES_H

