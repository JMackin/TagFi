//
// Created by ujlm on 11/2/23.
//

#include "lattice_signals.h"
#include <sodium.h>

#ifndef TAGFI_LATTICE_WORKS_H
#define TAGFI_LATTICE_WORKS_H
#define INFARRLEN 18
#define RSPARRLEN 256
#define HASHSTRLEN 64


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

typedef RspFlag* RspArr;
typedef ReqFlag* ReqArr;

typedef union LttFlg{
    ReqFlag req;
    RspFlag rsp;
    int flg;
    unsigned int uflg;
}LttFlg;
//SPLITTO works

typedef LttFlg* LttcFlags;


/**
 *<h4>
 * \InformationFrame
 *</h4><code>
 *<verbatim>
 *<br>  [ rsp size | req size |
 *<br>  |trvl_op | fi_op | di_op|
 *<br>  | sys_op  |  qual |
 *<br>  | arr type | arr len |
 *<br>  | CmdSeq ptr ]
 */
typedef struct InfoFrame {
    unsigned int lead;
    unsigned int cat_pfx;
    unsigned int rsp_size;
    unsigned int req_size;
    unsigned int trfidi[3];
    unsigned int sys_op;
    unsigned int qual;
    unsigned int arr_type; //0: none, 1: char, 2: int
    unsigned int arr_len;
    unsigned int flg_cnt;
    LttcFlags *flags;
    unsigned char *arr;
} InfoFrame;

InfoFrame *init_info_frm(InfoFrame **info_frm);



typedef struct LattStruct{
    Lattice lattice;
    DChains dirChains;
    Fi_Tbl* fiTbl;
    unsigned long* itmID;
}LattStruct;


typedef struct stat* stptr;


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


typedef unsigned int** RspMap;
typedef unsigned int (*RspFunc[RSPARRLEN])(StatFrame**, InfoFrame**, DChains*, Lattice*, unsigned char**);

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


InfoFrame * parse_req(const unsigned char* fullreqbuf,
                      InfoFrame **infofrm,
                      StatFrame** stsfrm,
                      LttcFlags* rqflgsbuf,
                      unsigned char* tmparrbuf,
                      unsigned char** req_arr_buf);





#endif //TAGFI_LATTICE_WORKS_H
