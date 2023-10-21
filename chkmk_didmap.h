//
// Created by ujlm on 8/6/23.
//

#ifndef TAGFI_CHKMK_DIDMAP_H
#define TAGFI_CHKMK_DIDMAP_H

//typedef enum FITYPES {
//    //( statbuf->st_mode & S_IFMT )>>13
//    TFIO = 0, //FIFO/pipe
//    TCHR = 1, //character device
//    TDIR = 2, //directory
//    TBLK = 3, //block device
//    TREG = 4, //regular file
//    TSYM = 5, //symlink
//    TSCK = 6 //socket
//} fitypes;

typedef struct stat* stptr;

typedef struct FiMap{
    unsigned long long fiid;
    unsigned long fhshno;
    unsigned char* finame;
} FiMap;

typedef struct Fi_Tbl{
    unsigned long ftblid; // hash ino of resident dir
    FiMap** entries;
    unsigned long totsize;
    int count;
} Fi_Tbl;

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

typedef struct HashBridge {
    unsigned long long* unid;
    Dir_Node* dirnode;
    Fi_Tbl* fitable;
    FiMap* finode;
} HashBridge;

typedef struct HashLattice {
    HashBridge** bridges;
    unsigned long count;
    unsigned long max;
} HashLattice;


int map_dir(const char* dir_path,
            unsigned int path_len,
            unsigned char* dirname,
            unsigned int dnlen,
            Dir_Chains* dirchains,
            HashLattice* hashlattice,
            Fi_Tbl** fitbl);

Dir_Chains* init_dchains();

HashLattice * init_hashlattice();

FiMap* mk_fimap(unsigned int nlen,
                unsigned char* finame,
                unsigned long long fiid,
                unsigned  long long did,
                unsigned long fhshno);

unsigned int getidx(FiMap* fimap);

void add_entry(FiMap* fimap,
               Fi_Tbl* fiTbl);

void travel_dchains(Dir_Chains* dirChains,
                    unsigned int lor,
                    unsigned char steps);

void goto_chain_tail(Dir_Chains* dirChains,
                     unsigned int lor);

Dir_Node* add_dnode(unsigned long long did,
                    unsigned char* dname,
                    unsigned short nlen,
                    unsigned int mord,
                    Dir_Chains* dirchains);

HashBridge* yield_bridge(HashLattice* hashLattice,
                         unsigned char* filename,
                         unsigned int n_len,
                         Dir_Node* root_dnode);

void destoryhashbridge(HashBridge* hashbridge);

void destryohashlattice(HashLattice* hashlattice);

void destroy_ent(FiMap* fimap,
                 Fi_Tbl* fiTbl);

void destroy_tbl(Fi_Tbl* fitbl);

void destroy_chains(Dir_Chains* dirChains);

int make_bridgeanchor(Dir_Node** dirnode,
                      char** path,
                      unsigned int pathlen);

/**
 *     Dir nodeno begin at 8(0b1000) and are masked to id their parent base (media: 0b100 doc: 0b010)
 *    MEDIA dirs will be chained LEFT of the head
 *    DOC dirs will be chained RIGHT of the head
 *    nodes are doubly-linked
 *    each node points to their respective fitbl
 *    each node also points to a hash bridge structure, from which
 *    file indexes can be extracted given the directory and file name
 *    if the index is not already known#@T 9ra
 *
 *    In this way a file table can be access from a directory node
 *    but not vice-versa. However the filemap IDs are masked
 *    with a number to ID their resident directory.
 *
 *    This goes the same for the hashlattices
 *    which are meant to provide an easy translation
 *    from a file name to an index, as well as well as natural crossing from
 *    directory to file node, especially if accessing from several levels
 *    away or when the files hashno and index are unknown.
 *
 *    Dir nodes are accessed with "vessel" a dir node pointer
 *    that walks up and down the chains. The vessel can switch
 *    between the two chains by crossing over the head node.
 *    The tails nodes do not link anywhere and are pointed to
 *    by the most recently added directory on a given chain
 *    and serve to provide an unambiguous end point.
 **/

/**

                         (vessel)
                       R/   *    \L
                      /  (head)   \
                          /  \
           {ftbl}   (media)  (docs)  {ftbl}
            ^    \     /   |      \   /  ^
            |     (dir)  -(*)-  (dir)    |
            |     /   \         /   \    |
          <hshltc>   (...)  (...)    <hshltc>
                      |        |
                   (tail)   (tail)
**/




#endif //TAGFI_CHKMK_DIDMAP_H
