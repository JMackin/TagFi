//
// Created by ujlm on 8/6/23.
//

#ifndef TAGFI_CHKMK_DIDMAP_H
#define TAGFI_CHKMK_DIDMAP_H

typedef enum FITYPES {
    //( statbuf->st_mode & S_IFMT )>>13
    TFIO = 0, //FIFO/pipe
    TCHR = 1, //character device
    TDIR = 2, //directory
    TBLK = 3, //block device
    TREG = 4, //regular file
    TSYM = 5, //symlink
    TSCK = 6 //socket
} fitypes;

typedef struct stat* stptr;


fitypes chk_fd(stptr statbuf, char* dir_path, int opt);
//void stat_fd(char* dir_path, stptr statptr);

void void_mkmap(const char* dir_path);


typedef struct FiMap{
    unsigned long long fiid;
    unsigned long did;
    unsigned long fhshno;
    unsigned char* finame;
} FiMap;


typedef struct Fi_Tbl{
    FiMap** entries;
    unsigned long totsize;
    int count;
} Fi_Tbl;

// Inititalized dir ring will consist of three nodes: head(1), media base(2), doc base(4)
// Dir nodeno begin at 8(0b1000) and are masked to id their parent base (media: 0b100 doc: 0b010)
// MEDIA dirs will be chained LEFT of the head
// DOC dirs will be chained RIGHT of the head
// nodes are doubly-linked
/*
          /    (head)    \
       r |/    / - \     \| l
           (media) (docs)
             |      |
           (dirs) (dirs)

**/

typedef struct Dir_Node{
    struct Dir_Node* left;
    struct Dir_Node* right;
    unsigned long long did;
    unsigned char* diname;
}Dir_Node;

typedef struct Dir_Chains{
    Dir_Node* dir_head;
    Dir_Node* vessel;
}Dir_Chains;




#endif //TAGFI_CHKMK_DIDMAP_H
