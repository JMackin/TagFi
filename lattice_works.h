//
// Created by ujlm on 11/2/23.
//

#include <sodium.h>


#ifndef TAGFI_LATTICE_WORKS_H
#define TAGFI_LATTICE_WORKS_H
#include "Consts.h"
#include "lattice_sessions.h"
#include "FiOps.h"


#define INFARRLEN 18
#define RSPARRLEN 256
#define HASHSTRLEN 64
#define LLKEYSZ 16
#define LKEYSZ 32u

#define AUTH_TAG_SIZE 32


typedef unsigned char LatticeLittleKey[LLKEYSZ];
typedef unsigned char* LatticeKey;

// 0 = sig, 1 = enc, 2 = hash
typedef LatticeKey (*LttcTriSubKey)[3];

typedef unsigned char* LttcHashKey;
typedef unsigned char* LttcSigKey;

typedef struct FiEntry{
    //unsigned long long fiid;
    unsigned long hshno;
    unsigned char* finame;
    char* path;
    unsigned int tag;
}FiEntry;
typedef FiEntry * NEntries;

typedef struct DiNodeGate{
    LattFD entrieslist_fd;
    LattFD dirnode_fd;
    LattFD shm_fd;
} DiNodeGate;
typedef DiNodeGate * DNGate;

typedef struct DNodeArmature{
    LatticeLittleKey dnode_key;
    NEntries entries;
    unsigned long totsize;
    unsigned int count;
} DNodeArmature;
typedef DNodeArmature* Armatr;

typedef struct FiNode{
    unsigned long long fiid;
    unsigned long fhshno;
} FiNode;

typedef FiNode* FNode;

typedef struct DiNode{
    unsigned long long did;
    unsigned char* diname;
    struct DiNode* left;
    struct DiNode* right;
    Armatr armatr;
    DNGate node_gate;
    unsigned long tag;
}DiNode;
typedef DiNode* Vessel;
typedef DiNode* DNode;

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
    FNode finode;
    u_long tag;
} HashBridge;

typedef struct HashLattice {
    DChains chains;
    HashBridge** bridges;
    unsigned long count;
    unsigned long max;
    LttcHashKey lhashKey;
    LState state;
} HashLattice;
typedef HashLattice* Lattice;
typedef HashLattice** Lattice_PTP;

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
    unsigned long* itmID;
    StatusFrame statusFrame;
    SSession session;
}LattStruct;

typedef unsigned int** RspMap;
typedef unsigned int (*RspFunc[RSPARRLEN])(SSession_PTP session, InfoFrame**, Lattice*, unsigned char**);

typedef struct Resp_Tbl{
    unsigned int fcnt;
    RspMap* rsp_map; // 3 x 3 x fcnt - 3D array: {LattReply,Mod,actIdx}
    RspFunc* rsp_funcarr;
} Resp_Tbl;
typedef Resp_Tbl* ResponseTable;
typedef Resp_Tbl** ResponseTable_PTP;


typedef struct stat* stptr;
typedef struct epoll_event* epEvent;
typedef InfoFrame** Info_Frame_PTP;
typedef StatusFrame** StsFrame_PTP;
typedef unsigned char** Std_Buffer_PTP; // Standard buffer, pointer-to-pointer

/**
 * Thread session toolkit and navigator
 * \Kit
 *  The SpawnKit.puml struct is attached to a threads SpawnSession by a thread-specific resource key
 *  and provides access to functional structs used in operations on the HashLattice and
 *  while handling requests.
 *
 * The SpawnKit.puml struct consists of the following components:
 *<li> Vessel: A pointer to a DiNode, marking the thread's position in the DirChains.
 *<li> LatticeLittleKey: A cryptographic value used to uniquely ID a user and aid in session persistence.
 *<li> TravelPath: This struct contains the last path traveled along the DirChains.
 *<li> BufferPool: A collection of buffers used throughout the thread's operations.
 *
 * <br><br><small><i>
 *  Dereffed Size:
 *  <br>             = 56 + 40 + 40 + 32 + 16 + 8
 *  <br>             = sizeof(HashLattice) + sizezof(BufferPool) + sizeof(DiNode) + sizeof(LatticeSessionTag) + sizeof(TravelPath) + sizeof(Tag)
 *  <br>             = 192
 */

typedef struct SpawnKit{
    Vessel vessel;
    Lattice latticeGate;
    LatticeSessionTag latticeTag;
    TravelPath travelPath;
    BufferPool bufferPool;
    Tag tag;
} SpawnKit;
typedef SpawnKit* Kit;


typedef struct SpinOffArgsPack{
    pthread_t tid;
    Lattice_PTP hashLattice;
    Std_Buffer_PTP request_buf;
    Std_Buffer_PTP response_buf;
    Std_Buffer_PTP requestArr_buf;
    Std_Buffer_PTP tempArr_buf;
    Flags_Buffer_PTP flags_buf;
    Info_Frame_PTP infoFrame;
    ResponseTable_PTP responseTable;       // Not implemented?
    epEvent epollEvent_IN;
    pthread_mutex_t* lock;
    SSession session;
    int dataSocket;
    int epollFD;
    int buf_len;
    unsigned int tag;
    struct SOA_internal _internal;
}SpinOffArgsPack;
typedef SpinOffArgsPack* SOA_Pack;


double long *map_dir(StatusFrame **statusFrame, const char *dir_path, unsigned int path_len, unsigned char *dirname,
                     unsigned int dnlen, HashLattice *hashlattice, DNodeArmature **fitbl, LttcHashKey latticeKey);

DiChains* init_dchains();

HashLattice *init_hashlattice(DChains *dirchains, LttcHashKey lattcKey, LState lttcstt);

FiNode* mk_finnode(unsigned int nlen,
                   unsigned char* finame,
                   unsigned long long fiid,
                   unsigned  long long did,
                   unsigned long fhshno);

unsigned int getidx(unsigned long fhshno);

uint add_entry(FiNode* entry,
               DNodeArmature* fiTbl,
               PathParts pp);

uint return_to_origin(TravelPath *travelPath, DChains dirChains, SState_PTP lttSt);

void travel_dchains(Vessel *vessel, unsigned int lor, unsigned char steps, TravelPath **travelpath, SState_PTP lttSt);

void goto_chain_tail(DiChains *dirChains, unsigned int lor, TravelPath **travelpath, SState_PTP lttSt);

void goto_base(DChains dchns, TravelPath **travelpath, SState_PTP lttSt);

void switch_base(DChains dchns, TravelPath **travelpath, SState_PTP lttSt);

unsigned int travel_by_chnid(unsigned long chn_id, DiChains *dchns, TravelPath **travelpath, SState_PTP lttSt);

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

//void destoryhashbridge(HashBridge hashbridge);
//
//void destryohashlattice(HashLattice* hashlattice);
//
//void destroy_node(FiNode * node,
//                 DNodeArmature* fiTbl);
//
//int destroy_ent(Armatr armatr,  unsigned long idx);
//
//void destroy_armatr(Armatr fitbl, HashLattice hashLattice);
//
//void destroy_chains(DiChains* dirChains);

LattFD make_bridgeanchor(Armatr *armatr, DiNode **dirnode, char **path, unsigned int pathlen);

unsigned int travel_by_diid(unsigned long long did, DiChains *dchns, TravelPath **travelpath, SState_PTP lttSt);

void yield_dnhsh(DiNode** dirnode, unsigned char** dn_hash);

__attribute__((unused)) char *yield_dnhstr(DiNode** dirnode);

InfoFrame * parse_req(unsigned char* fullreqbuf,
                      InfoFrame **infofrm,
                      StatusFrame** stsfrm,
                      LttFlgs* rqflgsbuf,
                      unsigned char* tmparrbuf,
                      unsigned char** req_arr_buf);

clock_t build_bridge(DNodeArmature* armatr,
                     FiNode* fiNode,
                     DiNode* dnode,
                     HashLattice* hashlattice,
                     unsigned char buf[16]);

void build_bridge2(
        LatticeLittleKey lattkey,
        FiNode* fiNode,
        DiNode* dnode,
        HashLattice* hashlattice,
        unsigned char obuf[16]);

unsigned int finode_idx(unsigned long fhshno);


SOA_Pack
pack_SpinOff_Args(pthread_t tid, Lattice_PTP hashLattice, Std_Buffer_PTP request_buf, Std_Buffer_PTP response_buf,
                  Std_Buffer_PTP requestArr_buf, Std_Buffer_PTP tempArr_buf, Flags_Buffer_PTP flags_buf,
                  Info_Frame_PTP infoFrame, ResponseTable_PTP responseTable, epEvent epollEvent_IN, int epollFD,
                  int dataSocket, int buf_len, int tag, pthread_mutex_t *lock, SSession session);
uint discard_SpinOff_Args(SOA_Pack* soaPack);

void update_SOA_DS(SOA_Pack* soaPack, int datasocket);

void update_SOA(SOA_OPTS opt, SOA_Pack* soaPack, void* new_val);

int make_socket_non_blocking(int sfd);
void destroy_bufferpool(BufferPool * bufferPool);
void init_bufferpool(BufferPool * bufferPool);
void init_Tag(Tag* tag);

#endif //TAGFI_LATTICE_WORKS_H