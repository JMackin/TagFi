//
// Created by ujlm on 8/6/23.
//

#ifndef TAGFI_CHKMK_DIDMAP_H
#define TAGFI_CHKMK_DIDMAP_H

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

typedef HashLattice* Lattice;
typedef Dir_Chains* DChains;



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

unsigned int gotonode(unsigned long long did, Dir_Chains* dchns);

void yield_dnhsh(Dir_Node** dirnode, unsigned char** dn_hash);
char *yield_dnhstr(Dir_Node** dirnode);

/**
 *
 *<br>    Dir nodeno begin at 8(0b1000) and are masked to id their parent base
 * <br>  (media: 0b100 doc: 0b010)
 * <br>
 *<br>   MEDIA dirs will be chained LEFT of the head
 *<br>   DOC dirs will be chained RIGHT of the head
 * <br>
 *<br>   nodes are doubly-linked
 *<br>   each node points to their respective fitbl
 *<br>   each node also points to a hash bridge structure, from which
 *<br>   file indexes can be extracted given the directory and file name
 *<br>   if the index is not already known
 *<br>
 *<br>   In this way a file table can be access from a directory node
 *<br>   but not vice-versa. However the filemap IDs are masked
 *<br>   with a number to ID their resident directory.
 *<br>
 *<br>   This goes the same for the hashlattices
 *<br>   which are meant to provide an easy translation
 *<br>   from a file name to an index, as well as well as natural crossing from
 *<br>   directory to file node, especially if accessing from several levels
 *<br>   away or when the files hashno and index are unknown.
 *<br>
 *<br>   Dir nodes are accessed with "vessel" a dir node pointer
 *<br>   that walks up and down the chains. The vessel can switch
 *<br>   between the two chains by crossing over the head node.
 *<br>   The tails nodes do not link anywhere and are pointed to
 *<br>   by the most recently added directory on a given chain
 *<br>   and serve to provide an unambiguous end point.
 **/

/**
 * \verbatim
 *

                   (vessel)
                 R/   *    \L
                 /  (head)  \
                    /  \
     {ftbl}   (media)  (docs)  {ftbl}
      ^    \     /   |      \   /  ^
      |     (dir)  -(*)-  (dir)    |
      |     /   \         /   \    |
      [hltc]  (...)    (...)   [hltc]
                |        |
             (tail)   (tail)

**/

#endif //TAGFI_CHKMK_DIDMAP_H
