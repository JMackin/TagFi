//
// Created by ujlm on 11/2/23.
//

#include "lattice_signals.h"
#include <sodium.h>
#ifndef TAGFI_LATTICE_WORKS_H
#define TAGFI_LATTICE_WORKS_H
#include "FiOps.h"

#define INFARRLEN 18
#define RSPARRLEN 256
#define HASHSTRLEN 64
#define LKEYSZ 16

typedef unsigned char LatticeKey[LKEYSZ];
typedef unsigned char* LattcKey;

typedef struct FiEntry{
    //unsigned long long fiid;
    unsigned long hshno;
    unsigned char* finame;
    char* path;
    unsigned int tag;
}FiEntry;
typedef FiEntry * NEntries;

typedef struct DiNodeMap{
    LattFD entrieslist_fd;
    LattFD dirnode_fd;
    LattFD shm_fd;
} DiNodeMap;
typedef DiNodeMap * DNMap;

typedef struct FiNode{
    unsigned long long fiid;
    unsigned long fhshno;
} FiNode;

typedef FiNode* NodeFoliage;

typedef struct DiNode{
    struct DiNode* left;
    struct DiNode* right;
    unsigned long long did;
    unsigned char* diname;
    unsigned long tag;
}DiNode;
typedef DiNode* Vessel;

typedef struct Armature{
    LatticeKey lttc_key;
    NEntries entries;
    DNMap nodemap;
    DiNode* dinode;
    unsigned long totsize;
    unsigned int count;
} Armature;
typedef Armature* Armatr;

typedef struct DiChains{
    Vessel dir_head;
    Vessel vessel;
}DiChains;
typedef DiChains* DChains;

struct HashBridge;


typedef struct HashBridge* ParaBridge;

typedef struct HashBridge {
    unsigned char unid[16];
    ParaBridge parabridge;
    Vessel dirnode;
    NodeFoliage finode;
    u_long tag;
} HashBridge;

typedef struct HashLattice {
    DChains chains;
    HashBridge** bridges;
    Armatr armature;
    unsigned long count;
    unsigned long max;
    LattcKey lattcKey;
    LttcStt state;

} HashLattice;
typedef HashLattice* Lattice;

typedef struct TravelPath{
    Vessel origin;
    Vessel destination;
}TravelPath;

typedef RspFlag* RspArr;
typedef ReqFlag* ReqArr;

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
    unsigned int cat_pfx;
    unsigned int rsp_size;
    unsigned int trfidi[3];
    unsigned int sys_op;
    unsigned int qual;
    unsigned int arr_type; //0: none, 1: char, 2: int
    unsigned int arr_len;
    unsigned int flg_cnt;
    LttFlgs *flags;
    unsigned char *arr;
    Vessel* vessel;
} InfoFrame;

InfoFrame *init_infofrm(InfoFrame **info_frm, uint startup);
//uint reset_infofrm(InfoFrame **info_frm);

typedef struct LattStruct{
    Lattice lattice;
    StatFrame statFrame;
    unsigned long* itmID;
}LattStruct;

typedef struct stat* stptr;

double long *map_dir(StatFrame **statusFrame, const char *dir_path, unsigned int path_len, unsigned char *dirname,
                     unsigned int dnlen, HashLattice *hashlattice, Armature **fitbl, LattcKey latticeKey);

DiChains* init_dchains();

HashLattice *init_hashlattice(DChains *dirchains, LattcKey lattcKey, LttcStt lttcstt);

FiNode* mk_finnode(unsigned int nlen,
                   unsigned char* finame,
                   unsigned long long fiid,
                   unsigned  long long did,
                   unsigned long fhshno);


typedef unsigned int** RspMap;
typedef unsigned int (*RspFunc[RSPARRLEN])(StatFrame**, InfoFrame**, Lattice*, unsigned char**);

unsigned int getidx(unsigned long fhshno);

uint add_entry(FiNode* entry,
               Armature* fiTbl,
               PathParts pp);

uint return_to_origin(TravelPath *travelPath, DChains dirChains, LttSt lttSt);

void travel_dchains(Vessel *vessel, unsigned int lor, unsigned char steps, TravelPath **travelpath, LttSt lttSt);

void goto_chain_tail(DiChains *dirChains, unsigned int lor, TravelPath **travelpath, LttSt lttSt);

void goto_base(DChains dchns, TravelPath **travelpath, LttSt lttSt);

void switch_base(DChains dchns, TravelPath **travelpath, LttSt lttSt);

unsigned int travel_by_chnid(unsigned long chn_id, DiChains *dchns, TravelPath **travelpath, LttSt lttSt);

DiNode* add_dnode(unsigned long long did,
                  unsigned char* dname,
                  unsigned short nlen,
                  unsigned int mord,
                  Lattice* lattice,
                  Armatr *armatr);

HashBridge* yield_bridge(HashLattice* hashLattice,
                         unsigned char* filename,
                         unsigned int n_len,
                         DiNode* root_dnode);

HashBridge * yield_bridge_for_fihsh(Lattice lattice,unsigned long fiHsh);

void destoryhashbridge(HashBridge hashbridge);

void destryohashlattice(HashLattice* hashlattice);

void destroy_node(FiNode * node,
                 Armature* fiTbl);

int destroy_ent(Armatr armatr,  unsigned long idx);

void destroy_armatr(Armatr fitbl, HashLattice hashLattice);

void destroy_chains(DiChains* dirChains);

LattFD make_bridgeanchor(Armatr *armatr, DiNode **dirnode, char **path, unsigned int pathlen);

unsigned int travel_by_diid(unsigned long long did, DiChains *dchns, TravelPath **travelpath, LttSt lttSt);

void yield_dnhsh(DiNode** dirnode, unsigned char** dn_hash);

__attribute__((unused)) char *yield_dnhstr(DiNode** dirnode);

InfoFrame * parse_req(unsigned char* fullreqbuf,
                      InfoFrame **infofrm,
                      StatFrame** stsfrm,
                      LttFlgs* rqflgsbuf,
                      unsigned char* tmparrbuf,
                      unsigned char** req_arr_buf);

clock_t build_bridge(Armature* armatr,
                     FiNode* fiNode,
                     DiNode* dnode,
                     HashLattice* hashlattice,
                     unsigned char buf[16]);

void build_bridge2(
                   LatticeKey lattkey,
                   FiNode* fiNode,
                   DiNode* dnode,
                   HashLattice* hashlattice,
                   unsigned char obuf[16]);

unsigned int finode_idx(unsigned long fhshno);



#endif //TAGFI_LATTICE_WORKS_H