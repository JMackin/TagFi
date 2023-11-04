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
#define LKEYSZ 16


typedef unsigned char LatticeKey[LKEYSZ];

typedef struct FiNode{
    unsigned long long fiid;
    unsigned long fhshno;
    unsigned char* finame;
} FiNode;

typedef struct Armature{
    LatticeKey lttc_key;
    FiNode** entries;
    unsigned long totsize;
    unsigned int count;
} Armature;

typedef struct DiNode{
    struct DiNode* left;
    struct DiNode* right;
    unsigned long long did;
    unsigned char* diname;
}DiNode;

typedef struct DiChains{
    DiNode* dir_head;
    DiNode* vessel;
}DiChains;

typedef struct HashBridge {
    unsigned char unid[16];
    DiNode* dirnode;
    Armature* fitable;
    FiNode* finode;
} HashBridge;

typedef struct HashLattice {
    HashBridge** bridges;
    unsigned long count;
    unsigned long max;
} HashLattice;

typedef HashLattice* Lattice;
typedef DiChains* DChains;

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
    Armature* fiTbl;
    unsigned long* itmID;
}LattStruct;


typedef struct stat* stptr;


double long* map_dir(const char* dir_path,
            unsigned int path_len,
            unsigned char* dirname,
            unsigned int dnlen,
            DiChains* dirchains,
            HashLattice* hashlattice,
            Armature** fitbl);

DiChains* init_dchains();

HashLattice * init_hashlattice();

FiNode* mk_finnode(unsigned int nlen,
                   unsigned char* finame,
                   unsigned long long fiid,
                   unsigned  long long did,
                   unsigned long fhshno);


typedef unsigned int** RspMap;
typedef unsigned int (*RspFunc[RSPARRLEN])(StatFrame**, InfoFrame**, DChains*, Lattice*, unsigned char**);

unsigned int getidx(FiNode* fimap);

void add_entry(FiNode* fimap,
               Armature* fiTbl);

void travel_dchains(DiChains* dirChains,
                    unsigned int lor,
                    unsigned char steps);

void goto_chain_tail(DiChains* dirChains,
                     unsigned int lor);

DiNode* add_dnode(unsigned long long did,
                  unsigned char* dname,
                  unsigned short nlen,
                  unsigned int mord,
                  DiChains* dirchains);

HashBridge* yield_bridge(HashLattice* hashLattice,
                         unsigned char* filename,
                         unsigned int n_len,
                         DiNode* root_dnode);

void destoryhashbridge(HashBridge* hashbridge);

void destryohashlattice(HashLattice* hashlattice);

void destroy_ent(FiNode* fimap,
                 Armature* fiTbl);

void destroy_tbl(Armature* fitbl);

void destroy_chains(DiChains* dirChains);

int make_bridgeanchor(DiNode** dirnode,
                      char** path,
                      unsigned int pathlen);

unsigned int gotonode(unsigned long long did, DiChains* dchns);

void yield_dnhsh(DiNode** dirnode, unsigned char** dn_hash);
char *yield_dnhstr(DiNode** dirnode);


InfoFrame * parse_req(const unsigned char* fullreqbuf,
                      InfoFrame **infofrm,
                      StatFrame** stsfrm,
                      LttcFlags* rqflgsbuf,
                      unsigned char* tmparrbuf,
                      unsigned char** req_arr_buf);

clock_t build_bridge(Armature* armatr,
                  FiNode* fiNode,
                  DiNode* dnode,
                  HashLattice* hashlattice,
                  unsigned char buf[16]);

clock_t build_bridge2(Armature* armatr,
                  FiNode* fiNode,
                  DiNode* dnode,
                  HashLattice* hashlattice,
                      unsigned char obuf[16]);





#endif //TAGFI_LATTICE_WORKS_H