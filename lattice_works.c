
//
// Created by ujlm on 8/6/23.
//
#include "lattice_works.h"
#include "fi_lattice.h"
#include "jlm_random.h"
#include "fiforms.h"
#include "fidi_masks.h"
#include "profiling.h"
#include "tagfi.h"
#include <sodium.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>


// ----------------------------

// ----------------------------


InfoFrame *init_infofrm(InfoFrame **info_frm, uint startup) {
    if (startup){*info_frm = (InfoFrame *) malloc(sizeof(InfoFrame));}
    (*info_frm)->rsp_size = 0;
    (*info_frm)->trfidi[0] = 0;
    (*info_frm)->trfidi[1] = 0;
    (*info_frm)->trfidi[2] = 0;
    (*info_frm)->sys_op = 0;
    (*info_frm)->qual = 0;
    (*info_frm)->arr_type = 0;
    (*info_frm)->arr_len = 0;
    (*info_frm)->arr = NULL;
    (*info_frm)->flags = NULL;
    (*info_frm)->vessel = NULL;

    return *info_frm;
}

void mk_one_dir_hashes(unsigned long long* iid, unsigned long long* hshno, const unsigned long ino){
    genrandsalt(hshno);

    *iid = ((((*hshno)>>DMASKSHFT)<<DMASKSHFT) ^ (ino<<DMASKSHFT));
}


void mk_hashes(unsigned long long* haidarr,
               unsigned long** fhshno_arr,
               const unsigned long* idarr,
               int n,int nd,
               const unsigned char *entype) {

    get_many_little_salts(fhshno_arr, (n-nd-1));

    int i;
    int j = 0;
    int k = 0;

    for (i = 0; i < n; i++) {
        if (entype[i] == TYPFIL){
            (*(fhshno_arr[j])) = clip_to_32((*(fhshno_arr[j])));

            haidarr[j] = msk_fino(idarr[i]);
            ++j;


        }
        if (entype[i] == TYPDIR) {
            k++;
        }
    }

}

void mk_tailnode(DiNode** tail){
    *tail = (DiNode*) malloc(sizeof(DiNode));
    (*tail)->diname = 0;
    (*tail)->did = 0;
    (*tail)->tag = 7;
}

DiChains* init_dchains() {
    const int init_nidx[3] = {1,2,4};
    const char* init_dnames[3] = {"HEAD","MEDA","DOCS"};
    //DiNode** init_dnodes[3] = {0,0,0};
    DiNode** init_dnodes = (DiNode**) malloc(sizeof(DiNode*)*3);
    DiChains* dirchains = (DiChains*) malloc(sizeof(DiChains));

    for (int i = 0; i < 3; i++) {
        init_dnodes[i] = (DiNode*) malloc(sizeof(DiNode));
        (init_dnodes[i])->diname = (unsigned char*) malloc(8);
        memcpy((init_dnodes[i])->diname, init_dnames[i],(8));
        (init_dnodes[i])->did = DGRPTMPL|init_nidx[i];
        (init_dnodes[i])->tag = init_nidx[i];
    }



    (init_dnodes[0])->right=(init_dnodes[1]);
    (init_dnodes[0])->left=init_dnodes[2]; // docs <-l head r-> meda
    (init_dnodes[1])->left=init_dnodes[0];// head <-l meda
    (init_dnodes[2])->right=init_dnodes[0];// docs r-> head
    mk_tailnode(&((init_dnodes[1])->right));
    mk_tailnode(&((init_dnodes[2])->left));

    dirchains->dir_head = (init_dnodes[0]);
    dirchains->vessel = dirchains->dir_head;

    free(init_dnodes);

    return dirchains;
}


uint return_to_origin(TravelPath *travelPath, DChains dirChains, LttSt lttSt) {
    if (dirChains->vessel->did != travelPath->destination->did){
        return 1;
    }
    dirChains->vessel = travelPath->origin;
    (*lttSt)->cwdnode = dirChains->vessel->did;
    return 0;
}

//enroute: 1 = at origin, 0 = at destination
void chart_travelpath(TravelPath **travelpath, Vessel vessel, int enroute, LttSt lttSt) {
    if (!enroute){
        (*travelpath) = (TravelPath*) malloc(sizeof(TravelPath));
        (*travelpath)->origin = vessel;
    }else{
        (*travelpath)->destination = vessel;
        (*lttSt)->cwdnode = vessel->did;

    }

}

void travel_dchains(Vessel *vessel, unsigned int lor, unsigned char steps, TravelPath **travelpath, LttSt lttSt) {

    if (travelpath != NULL){ chart_travelpath(travelpath, *vessel, 0, lttSt);}
    while (steps>0){
        if (lor) {
            if((*vessel)->right->did == 0){
                break;
            } else {
                (*vessel) = (*vessel)->right;
            }
        } else {
            if((*vessel)->left->did == 0){
                break;
            } else {
                (*vessel) = (*vessel)->left;
            }
        }
        steps--;
    }
    if (travelpath != NULL){ chart_travelpath(travelpath, *vessel, 1, lttSt);}
}

void goto_chain_tail(DiChains *dirChains, unsigned int lor, TravelPath **travelpath, LttSt lttSt) {

    if (travelpath != NULL){ chart_travelpath(travelpath, dirChains->vessel, 0, lttSt);}
    (dirChains)->vessel = dirChains->dir_head;
    if (lor) {
        do{
            dirChains->vessel = dirChains->vessel->right;
        }while (dirChains->vessel->right->did != 0);
    }else {
        do{
            dirChains->vessel = dirChains->vessel->left;
        }while (dirChains->vessel->left->did != 0);
    }
    if (travelpath != NULL){ chart_travelpath(travelpath, dirChains->vessel, 1, lttSt);}

}

void goto_base(DChains dchns, TravelPath **travelpath, LttSt lttSt) {
    if (travelpath != NULL){ chart_travelpath(travelpath, dchns->vessel, 0, lttSt);}
    if (is_base(dchns->vessel)){
        if (travelpath != NULL){ chart_travelpath(travelpath, dchns->vessel, 1, lttSt);}
        return;
    }
    if (travelpath != NULL){ chart_travelpath(travelpath, dchns->vessel, 1, lttSt);}
    dchns->vessel = expo_dirbase(dchns->vessel->did) ? dchns->dir_head->left : dchns->dir_head->right;
}

void switch_base(DChains dchns, TravelPath **travelpath, LttSt lttSt) {
    if (travelpath != NULL){ chart_travelpath(travelpath, dchns->vessel, 0, lttSt);}
    dchns->vessel = expo_dirbase(dchns->vessel->did) ? dchns->dir_head->left : dchns->dir_head->right;
    if (travelpath != NULL){ chart_travelpath(travelpath, dchns->vessel, 1, lttSt);}
}

unsigned int travel_by_chnid(unsigned long chn_id, DiChains *dchns, TravelPath **travelpath, LttSt lttSt) {
    if (travelpath != NULL){ chart_travelpath(travelpath, dchns->vessel, 0, lttSt);}

    unsigned int lor = expo_dirbase(dchns->vessel->did);
    unsigned int pos_base = 0;
    unsigned int pos_base2 = 0;

    goto_base(dchns, NULL, lttSt);
    while(!pos_base) {
        if (pos_base2){
            pos_base = 1;
        }
        while (dchns->vessel->right->did != 0 && dchns->vessel->left->did != 0) {
            dchns->vessel = (lor) ? dchns->vessel->left : dchns->vessel->right;
            if (expo_dirchnid(dchns->vessel->did) == chn_id) {
                if (travelpath != NULL){ chart_travelpath(travelpath, dchns->vessel, 1, lttSt);}
                return 0;

            }
        }
        switch_base(dchns, NULL, lttSt);
        lor = !lor;
        pos_base2 = 1;
    }
    if (travelpath != NULL){ chart_travelpath(travelpath, dchns->vessel, 1, lttSt);}
    return 1;
}

unsigned int travel_by_diid(unsigned long long did, DiChains *dchns, TravelPath **travelpath, LttSt lttSt) {
    if (travelpath != NULL){ chart_travelpath(travelpath, dchns->vessel, 0, lttSt);}
    Vessel origin = (dchns)->vessel;
    (*travelpath)->origin = origin;

    unsigned int lor = expo_dirbase(did);
    unsigned int dnpos = 0;

    dchns->vessel = dchns->dir_head;
    if (lor) {
        do {
            dchns->vessel = dchns->vessel->right;
            dnpos++;
        } while (dchns->vessel->did != did && dchns->vessel->did != 0);

        if (!(did && dchns->vessel->did)) {
            return 1;
        }
        //DOCS
    }
    else {
        do {
            dchns->vessel = dchns->vessel->left;
            dnpos++;
        } while (dchns->vessel->did != did && dchns->vessel->did != 0);

        if (!(dchns->vessel->did)) {
            return 1;
        }
    }

    if (travelpath != NULL){ chart_travelpath(travelpath, dchns->vessel, 1, lttSt);}
    return dnpos;
}

//mord: 0=docs, 1=media
DiNode* add_dnode(unsigned long long did, unsigned char* dname, unsigned short nlen, unsigned int mord, Lattice* lattice, Armatr *armatr) {

    (*lattice)->chains->vessel = (*lattice)->chains->dir_head;
    DiNode *dnode = (DiNode *) (malloc(sizeof(DiNode)));
    DiNode *base = (mord) ? (*lattice)->chains->vessel->right : (*lattice)->chains->vessel->left;

    dnode->diname = (unsigned char *) calloc(1+nlen, UCHAR_SZ);
    dnode->diname = memcpy(dnode->diname, dname, nlen * UCHAR_SZ);
    unsigned long cnt = expo_basedir_cnt(base->did);

    dnode->did = (did | ((nlen) << DNAMESHFT) | ((mord) ? MEDABASEM : DOCSBASEM) | ++cnt);

    base->did += (16);

    (*lattice)->chains->vessel = ((*lattice)->chains->dir_head);
    goto_chain_tail((*lattice)->chains, mord, NULL, (*lattice)->state);


    //MEDA
    if (mord) {
        dnode->left = (*lattice)->chains->vessel;
        dnode->right = (*lattice)->chains->vessel->right;
        (*lattice)->chains->vessel->right = dnode;
        dnode->tag = 8;
    }
        //DOCS
    else {
        dnode->right = (*lattice)->chains->vessel;
        dnode->left = (*lattice)->chains->vessel->left;
        (*lattice)->chains->vessel->left = dnode;
        dnode->tag = 16;
    }

    (*armatr)->dinode=dnode;

//    dnode->armature = armatr;

    return dnode;
}

void yield_dnhsh(DiNode** dirnode, unsigned char** dn_hash) {

    int dnkeyfd = openat(AT_FDCWD,getenv("DNKEYPATH"),O_DIRECTORY);

    if (dnkeyfd < 0 )
    {
        perror("make_bridgeanchor/dnkeyfd");
        close(dnkeyfd);
    }

    *dn_hash = (unsigned char*) malloc(crypto_generichash_BYTES);
    unsigned char* dkey = (unsigned char*) malloc(crypto_shorthash_KEYBYTES);

    //TODO: OPTIMIZE THIS
    recv_little_hash_key(dnkeyfd,((*dirnode)->diname),expo_dirnmlen((*dirnode)->did),dkey);

    real_hash_keyfully((&(*dirnode)->diname), dn_hash, expo_dirnmlen((*dirnode)->did), (const unsigned char **) &dkey, crypto_shorthash_KEYBYTES);

    free(dkey);
    close(dnkeyfd);
}

__attribute__((unused)) char* yield_dnhstr(DiNode** dirnode){
    char* str_buf = (char*) malloc(crypto_generichash_BYTES*2+1);

    unsigned char *hsh_buf;
    yield_dnhsh(dirnode, &hsh_buf);

    dn_hdid_str(&hsh_buf,&str_buf);
    free(hsh_buf);

    return str_buf;
}

DNMap chart_node(uint dirpathlen, uint entriesnamelen, char **entriesname, char **dirpath, LattFD entrieslist_lfd, LattFD dirnodefd) {
    DNMap dnMap = malloc(sizeof(DiNodeMap));

    char* pathbuf = calloc(entriesnamelen+dirpathlen+2,UCHAR_SZ);
    char* pathbufB = calloc(ANCHORNMLEN,UCHAR_SZ);

    dnMap->shm_fd = open_blank_lattfd();
    dnMap->entrieslist_fd = entrieslist_lfd;
    dnMap->entrieslist_fd->path = NULL;
    dnMap->dirnode_fd = dirnodefd;
    dnMap->dirnode_fd->path = NULL;
    dnMap->entrieslist_fd->dir_fd = cycle_nodeFD(&dirnodefd);

    memcpy(pathbuf,*dirpath,dirpathlen);
    memset(pathbuf+dirpathlen+1,'/',1);
    memcpy(pathbuf+dirpathlen+1,*entriesname,64);
    memcpy(pathbufB,*entriesname,entriesnamelen);

    dnMap->entrieslist_fd->path = pathbuf;
    dnMap->dirnode_fd->path = pathbufB;

    return dnMap;
}

LattFD make_bridgeanchor(Armatr *armatr, DiNode **dirnode, char **path, unsigned int pathlen) {

    LattFD lattFd_entr;
    LattFD lattFd_dir;

    int dnkeyfd = openat(AT_FDCWD,getenv("DNKEYPATH"),O_DIRECTORY);

    if (dnkeyfd < 0 )
    {
        perror("make_bridgeanchor/dnkeyfd");
        close(dnkeyfd);
        return NULL;
    }

    lattFd_dir = open_dir_lattfd(*path, 0, 0, 0);


    if (lattFd_dir->prime_fd < 0 )
    {
        perror("make_bridgeanchor/dnfd");
        close(lattFd_dir->prime_fd);
        free(lattFd_dir);
        close(dnkeyfd);
        return NULL;
    }

    unsigned char* dn_hash = (unsigned char*) sodium_malloc(crypto_generichash_BYTES);
    unsigned char* dkey = (unsigned char*) sodium_malloc(crypto_shorthash_KEYBYTES);
    char* dn_hdid = (char*) malloc(ANCHORNMLEN+1);

    recv_little_hash_key(dnkeyfd,
                         ((*dirnode)->diname),
                         expo_dirnmlen((*dirnode)->did),
                         dkey);

    real_hash_keyfully((&(*dirnode)->diname),
                       &dn_hash, expo_dirnmlen((*dirnode)->did),
                       (const unsigned char **) &dkey,
                       crypto_shorthash_KEYBYTES);

    dn_hdid_str(&dn_hash, &dn_hdid);

    sodium_free(dkey);
    sodium_free(dn_hash);
    close(dnkeyfd);

   // cycle_nodeFD(&lattFd_entr);
    lattFd_entr = open_lattfd(dn_hdid, cycle_nodeFD(&lattFd_dir), O_CREAT | O_APPEND | O_RDWR, S_IRWXU, 0, 0);

    if (lattFd_dir->prime_fd == -1){ perror("Failed to open dir anchor fd.");
        return NULL;}

    //TODO: handle write error for bridge anchor
    if (write(cycle_nodeFD(&lattFd_entr), *path, (pathlen+expo_dirnmlen((*dirnode)->did))) == -1){
        perror("Write to bridgeanchor failed.");
        free(dn_hdid);
        return NULL;
    }
    fsync(lattFd_entr->duped_fd);

    close(lattFd_entr->duped_fd);

    (*armatr)->nodemap = chart_node(pathlen+expo_dirnmlen((*dirnode)->did)+1, ANCHORNMLEN, &dn_hdid, path, lattFd_entr, lattFd_dir);
    free(dn_hdid);

    return lattFd_entr;
}

void init_armatr(Armature*** armatr, unsigned int size, unsigned char** hkey){

    (**armatr) = (Armature*) malloc(sizeof(Armature));

    (**armatr)->totsize=size;
    (**armatr)->count=0;
    ((**armatr)->entries) = (FiEntry*) calloc(size, sizeof(FiEntry));

    for(int i = 0; i<(**armatr)->totsize; i++){
        (**armatr)->entries->hshno = 0;
        (**armatr)->entries->tag = 0;
        (**armatr)->entries->path = NULL;
    }

    memcpy((&(**armatr)->lttc_key),*hkey,LKEYSZ);
    (**armatr)->nodemap = NULL;
}

FiNode* mk_finnode(unsigned int nlen, unsigned char* finame,
                   unsigned long long fiid, unsigned long long did,
                   unsigned long fhshno){

    FiNode* fimap;
    fimap = (FiNode*) malloc(sizeof(FiNode));

//    fimap->finame = (unsigned char*) calloc(nlen+1, UCHAR_SZ);
  //  memcpy(fimap->finame,finame,(nlen * UCHAR_SZ));

    fiid = msk_finmlen(fiid,nlen);
    fiid = msk_format(fiid, grab_ffid(finame,nlen));
    fiid = msk_resdir(fiid, expo_dirchnid(did));
    fiid = msk_dirgrp(fiid, expo_dirbase(did));

    fimap->fiid = fiid;
    fimap->fhshno = finode_idx(fhshno);

    return fimap;
}

unsigned int getidx(unsigned long fhshno)
{
    return clip_to_32(fhshno & HTMASK);
}

unsigned int finode_idx(unsigned long fhshno)
{
    fhshno = (unsigned int) clip_to_32(fhshno);
    unsigned int clk;
    clk = fhshno & 15;
    unsigned int msk = 1431655765;
    msk = (clk > 7) ? msk << clk : msk >> clk;
    fhshno ^= (msk);
    return fhshno;
}

unsigned int cat_path(char** catpath_out, PathParts pp){
    char* resdir_name = pp.resdir_name;
    uint resdirname_len = pp.resdirname_len;
    uint running_len = 0;

    /*  Total PathLength =
            resident-dirnode's parent pat-length
            + res-dirnode's name-length
            + file name-length
            + 2 '/' chars + '\0'.
     */

    uint tot_pathlen = pp.pathlen + resdirname_len + pp.namelen + 2;
    *catpath_out = (char*) calloc((tot_pathlen),UCHAR_SZ);

    memcpy(*catpath_out+running_len,pp.parentpath,pp.pathlen);
    running_len = pp.pathlen;

    memcpy(*catpath_out+running_len,resdir_name,resdirname_len);
    running_len += resdirname_len;

    memset(*catpath_out+running_len,'/',1);
    running_len += 1;

    memcpy(*catpath_out+running_len,pp.name,pp.namelen);
    running_len += pp.namelen;

    memset(*catpath_out+running_len,'\0',1);
    running_len += 1;

    return tot_pathlen == running_len ? tot_pathlen : 1;
}

uint add_entry(FiNode* finode, Armature* armatr, PathParts pathparts) {
    int x = 0;
    timr_st();
    FiEntry ne;
    char* fullfiname;

    if(cat_path(&fullfiname,pathparts)==1){
        return 1;
    }

    if ((armatr->entries[getidx(finode->fhshno)]).hshno != 0) {
        while (((armatr->entries)[getidx(finode->fhshno)].hshno) != 0) {
            (finode->fhshno) = finode_idx((finode->fhshno));
            printf("EC! X:%lu\n",HTMASK&(finode->fhshno));
            coll_upup();
            if(++x > 3){
                (rando_sf(&finode->fhshno));
                if (x > 9){
                    return 1;
                }
            }
        }
    }

    ne.hshno = finode->fhshno;
    //ne.fiid = finode->fiid;
    ne.tag = 1;
    ne.path = fullfiname;
    ne.finame = (uchar*) (fullfiname - pathparts.namelen);
    //free(fullfiname);
    armatr->entries[getidx(finode->fhshno)] = ne;
    armatr->count++;
    inc_collcntr();
    timr_hlt();
    return 0;
}

HashLattice *init_hashlattice(DChains *dirchains, LattcKey lattcKey, LttcStt lttcstt) {


    HashLattice* hashlattice = (HashLattice *) malloc(sizeof(HashLattice));
    hashlattice->max = LTTCMX;
    hashlattice->count = 0;

    hashlattice->bridges = (HashBridge **) calloc(LTTCMX, sizeof(HashBridge*));
    for(int i = 0; i < hashlattice->max;i++){
        hashlattice->bridges[i] = NULL;
    }

    hashlattice->chains = *dirchains;
    hashlattice->chains->vessel = hashlattice->chains->dir_head;

    hashlattice->lattcKey = lattcKey;

    hashlattice->state = lttcstt;
    return hashlattice;
}

void build_bridge2(LatticeKey lattkey, FiNode* fiNode, DiNode* dnode, HashLattice* hashlattice, unsigned char* obuf){

    timr_st();
    if (hashlattice->count > hashlattice->max-3){
        return;
    }
    HashBridge* hshbrg = (HashBridge*) malloc(sizeof(HashBridge));

    unsigned long long int inid = (clip_to_32(fiNode->fiid) | clip_to_32(dnode->did));
    uchar* idbuf = (uchar*) calloc(9,UCHAR_SZ);

    memcpy(idbuf,&inid,8);
    memcpy(hshbrg->unid ,idbuf,8);

    unsigned long idx = latt_hsh_idx(lattkey, fiNode->fhshno, obuf);
    //1111411
    hshbrg->dirnode = dnode;
    hshbrg->finode = fiNode;
    hshbrg->parabridge = NULL;

    if (hashlattice->bridges[idx] == NULL){
        hashlattice->bridges[idx] = hshbrg;
    }else {
        ParaBridge parabrg = hashlattice->bridges[idx];
        int x = 0;
        printf("\nBridgeCollision!\n");
        while (((parabrg)->parabridge) != NULL){
            (parabrg) = (parabrg)->parabridge;
            fprintf(stderr,"para-level: %d", x);
        }
        (parabrg)->parabridge = hshbrg;
    }
    hashlattice->count++;

    free(idbuf);

    //printf("[%.2ld]\n",cb-ca);
    timr_hlt();
}

HashBridge *yield_bridge_for_fihsh(Lattice lattice,unsigned long fiHsh){

    HashBridge * hshBrdg;
    unsigned char oBuf [16] = {0};
    unsigned long long int biidBufL;

    unsigned long brdgIdx = latt_hsh_idx(lattice->lattcKey,fiHsh,oBuf);
    hshBrdg = lattice->bridges[brdgIdx];

    if (hshBrdg == NULL){
        return NULL;
    }

    memcpy(&biidBufL,&(hshBrdg->unid),8);

    while (hshBrdg->parabridge != NULL){
        hshBrdg = hshBrdg->parabridge;
        memcpy(&biidBufL,hshBrdg->unid,8);
        if ((biidBufL & fiHsh) == fiHsh){
            break;
        }
    }

    return hshBrdg;
}

__attribute__((unused)) int filter_dirscan(const struct dirent *entry) {
    return ((entry->d_name[0] == 46)) || (strnlen(entry->d_name, 4) > 3);
}

double long *map_dir(StatFrame **statusFrame, const char *dir_path, unsigned int path_len, unsigned char *rootdirname,
                     unsigned int dnlen, HashLattice *hashlattice, Armature **armatr, LattcKey latticeKey) {
    int i;
    int j=0;
    int k=0;
    uint TEST_flg  = 0;
    PathParts pp;
    pp.parentpath = dir_path;
    pp.pathlen = path_len;


    // Open the directory and read in the contents
    struct dirent ***dentrys = (struct dirent***) malloc(sizeof(struct dirent**));
    int n;
    int dir_cnt = 0;
    uint anchorsz = 0;

    unsigned char* hkey = (unsigned char*) sodium_malloc(UCHAR_SZ * crypto_shorthash_KEYBYTES);

    unsigned long long rootiid;
    unsigned long long roothshno;
    unsigned long rootino = cwd_ino(".");
    // Each dir node is a link in the dir chains
    DiNode* dirNode_ptr;

    // Gen numbers for the root first, to be used in the creation of values for its entries
    mk_one_dir_hashes(&rootiid,&roothshno,rootino);

    // Root node is created and added to the chain

    // A unique hashkey is created and stored for the dir node
    mk_little_hash_key(&hkey);
    dump_little_hash_key(hkey,rootdirname,dnlen);

    // Initialize file table assigned to the root
    init_armatr(&armatr,HTSIZE,&hkey);
    dirNode_ptr = add_dnode(rootiid,rootdirname,dnlen,1,&hashlattice,armatr);
    pp.resdir_name = (char*) dirNode_ptr->diname;
    pp.resdirname_len = expo_dirnmlen(dirNode_ptr->did);

    LattFD lattFd  = make_bridgeanchor(armatr, &dirNode_ptr, (char **) &dir_path, path_len);
    if (lattFd  == NULL){perror("Failed making bridge anchor\n");exit(EXIT_FAILURE);}

    // scandir returns number of entrys in the directory
    n = scandir(dir_path, dentrys, NULL, alphasort);

    // Alloc arrays for filenames, file ids, entry type, and length of filename
    unsigned char** farr = (unsigned char **) calloc(n, 256 * UCHAR_SZ);
    unsigned long* idarr = (unsigned long*) calloc(n, ULONG_SZ);
    unsigned char* entype = (unsigned char*) calloc(n, UCHAR_SZ);
    unsigned long* nlens = (unsigned long*) calloc(n, ULONG_SZ);
    unsigned char* dubbuf = (unsigned char*) calloc(17,UCHAR_SZ);


    // For each entry in the directory...
    for (i=0;i<n;i++){
        // Filename length
        nlens[i] = strnlen((*dentrys)[i]->d_name, FINMMAXL);
        // Filename char arr
        farr[i] = (unsigned char*) calloc((nlens[i]+1), UCHAR_SZ);
        memcpy(farr[i], (const unsigned char*)(*dentrys)[i]->d_name,nlens[i]+1);
        // File inode number
        idarr[i] = (*dentrys)[i]->d_ino;
        // Filetype (i.e. directory or file)
        entype[i] = (*dentrys)[i]->d_type;
        if (entype[i] == TYPDIR){
            dir_cnt++;
        }
        anchorsz += nlens[i]+1;

        free((*dentrys)[i]);
    }
    free(*dentrys);

    set_fisz(&lattFd,anchorsz);

    // Alloc arrays for file/dir hash value (assigned random num), and id number derived from inode and hash value.
    unsigned long** fhshno_arr = (unsigned long**) calloc((n-dir_cnt),sizeof(unsigned long*));
    unsigned long long* haidarr = (unsigned long long*) calloc((n-dir_cnt),sizeof(unsigned long long));


    // Gen numbers for the entries
    mk_hashes(haidarr, fhshno_arr, idarr, n, dir_cnt, entype);

    // Each entry (file) is represented by a Filemap containing fiid, fhshno, finame
    FiNode* fimap_ptr;

    // For each entry in the root...
    for (i=0;i<n;i++) {

        // If it's a directory
        if (entype[i] == TYPDIR){
            k++;
        }

        // If it's a file
        else {

            // Make a filemap object for it
            fimap_ptr = mk_finnode(nlens[i], farr[i], haidarr[j], dirNode_ptr->did, *fhshno_arr[j]);
            pp.name = farr[i];
            pp.namelen = nlens[i];

            // Add the entry to the file/hash table
            add_entry(fimap_ptr,*armatr,pp);

            // Add a lattice bridge to connect filemap object to the dir node
            build_bridge2(latticeKey, fimap_ptr,dirNode_ptr, hashlattice, dubbuf);
            //free(fhshno_arr[j]);
            j++;

            if(i < 10){
                printf("file: %llu :: %lu\n",fimap_ptr->fiid,fimap_ptr->fhshno);
            }
        }
        //This will be used to mk entry list
//        free(farr[i]);
    }
    uint res = mk_node_list(j, lattFd, farr, fhshno_arr);
    if (res){
        return NULL;
    }


    time_supr();

//*
// *
// *  Begin test section...
// *  -------------------------------
// */


    hashlattice->chains->vessel = hashlattice->chains->dir_head;
    for (i=0;i<7;i++) {
        if (hashlattice->chains->vessel->did == 0){break;}
        travel_dchains(&(hashlattice->chains->vessel), 1, 1, NULL, &hashlattice->state);
        printf("%s\n", hashlattice->chains->vessel->diname);
        printf("^^%llu\n", (hashlattice->chains->vessel->did));
        printf("^^%u\n", expo_dirnmlen(hashlattice->chains->vessel->did));
        printf("%u\n",   expo_dirchnid(hashlattice->chains->vessel->did));
        printf("%u\n",   expo_dirbase(hashlattice->chains->vessel->did));
        printf("%u\n",   expo_basedir_cnt(hashlattice->chains->vessel->did));
    }

    printf("\n\n");
    printf("%s\n",hashlattice->chains->dir_head->diname);
    printf("%llu\n",hashlattice->chains->dir_head->did);


    //unsigned char* bridgefiid = (yield_bridge(hashlattice, "sandpit.tar.gpg", 15, dirNode_ptr, "/home/ujlm/CLionProjects/TagFI/keys/Tech.lhsk"))->dirnode->diname;
    //printf("BRIDGE >> %s", bridgefiid);

    p_colrat();

/**
 * -------------------------------
 * End of test section
 *
 */

    // Cleanup
    free(dentrys);
    for (i = 0; i < n; i++){
        if (farr[i] != NULL) {
            free((farr[i]));
        }
    }


    sodium_free(hkey);
    free(dubbuf);
    free(farr);
    free(idarr);
    free(entype);
    free(nlens);
    free(haidarr);
    free(fhshno_arr);


    return 0;

}