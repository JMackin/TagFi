
//
// Created by ujlm on 8/6/23.
//
#include "chkmk_didmap.h"
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
#include <time.h>


// ----------------------------

// ----------------------------


#define DNKEYPATH "/home/ujlm/Code/Clang/C/TagFI/keys"
#define NDENTRYPRFX_SZ 8


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
            j++;
        }
        if (entype[i] == TYPDIR) {

            k++;
        }
    }
}

DiNode* mk_tailnode(){
    DiNode* tail = (DiNode*) malloc(sizeof(DiNode));
    tail->diname = 0;
    tail->did = 0;
    return tail;
}

DiChains* init_dchains() {
    const int init_nidx[3] = {1,2,4};
    const char* init_dnames[3] = {"HEAD","MEDA","DOCS"};
    DiNode** init_dnodes = (DiNode**) calloc(3,sizeof(DiNode*));
    DiChains* dirchains = (DiChains*) malloc(sizeof(DiChains));

    for (int i = 0; i < 3; i++) {

        init_dnodes[i] = (DiNode*) malloc(sizeof(DiNode));
        init_dnodes[i]->diname = (unsigned char*) malloc(8);
        memcpy(init_dnodes[i]->diname, init_dnames[i],(8));
        init_dnodes[i]->did = DGRPTMPL|init_nidx[i];
    }

    init_dnodes[0]->right=init_dnodes[1];
    init_dnodes[0]->left=init_dnodes[2]; // docs <-l head r-> meda
    init_dnodes[1]->left=init_dnodes[0];// head <-l meda
    init_dnodes[2]->right=init_dnodes[0];// docs r-> head
    init_dnodes[1]->right = mk_tailnode();
    init_dnodes[2]->left = mk_tailnode();

    dirchains->dir_head = init_dnodes[0];
    dirchains->vessel = dirchains->dir_head;


    return dirchains;
}

void travel_dchains(Vessel* vessel, unsigned int lor, unsigned char steps) {

    while (steps>0){
        if (lor) {
            if((*vessel)->right->did == 0){
                break;
            }else
            {
                (*vessel) = (*vessel)->right;
            }
        }else
        {
            if((*vessel)->left->did == 0){
                break;
            }else{
                (*vessel) = (*vessel)->left;
            }
        }
        steps--;
    }
}

void goto_chain_tail(DChains dirChains, unsigned int lor){
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
}

void traverse_dchains(DiNode* dnode) {

    while(dnode->did != (DROOTDID)){
        printf("%s\n", dnode->diname);
        dnode = dnode->left;
    }
}

unsigned int gotonode(unsigned long long did, DiChains* dchns){

    unsigned int lor = expo_dirbase(did);
    unsigned int dnpos = 0;

    dchns->vessel = dchns->dir_head;
    if (lor) {
        do {
            dchns->vessel = dchns->vessel->right;
            dnpos++;
        } while (dchns->vessel->did != did && dchns->vessel->did != 0);

        if (!(did && dchns->vessel->did)) {
            return -1;
        }
        //DOCS
    }
    else {
        do {
            dchns->vessel = dchns->vessel->left;
            dnpos++;
        } while (dchns->vessel->did != did && dchns->vessel->did != 0);

        if (!(dchns->vessel->did)) {
            return -1;
        }
    }
    return dnpos;
}

//mord: 0=docs, 1=media
DiNode* add_dnode(unsigned long long did, unsigned char* dname, unsigned short nlen, unsigned int mord, Lattice* lattice, Armatr armatr) {

    (*lattice)->chains->vessel = (*lattice)->chains->dir_head;
    DiNode *dnode = (DiNode *) (malloc(sizeof(DiNode)));
    DiNode *base = (mord) ? (*lattice)->chains->vessel->right : (*lattice)->chains->vessel->left;

    dnode->diname = (unsigned char *) calloc(1+nlen,sizeof(unsigned char));
    dnode->diname = memcpy(dnode->diname, dname, nlen * sizeof(unsigned char));
    unsigned long cnt = ((base->did) & DGCNTMASK) >> DGCNTSHFT;

    dnode->did = (did | ((nlen) << DNAMESHFT) | ((mord) ? MEDABASEM : DOCSBASEM) | ++cnt);
    base->did += (16);

    (*lattice)->chains->vessel = ((*lattice)->chains->dir_head);
    goto_chain_tail((*lattice)->chains, mord);

    //MEDA
    if (mord) {
        dnode->left = (*lattice)->chains->vessel;
        dnode->right = (*lattice)->chains->vessel->right;
        (*lattice)->chains->vessel->right = dnode;
    }
        //DOCS
    else {
        dnode->right = (*lattice)->chains->vessel;
        dnode->left = (*lattice)->chains->vessel->left;
        (*lattice)->chains->vessel->left = dnode;
    }

    dnode->armature = armatr;

    return dnode;
}

void yield_dnhsh(DiNode** dirnode, unsigned char** dn_hash) {

    int dnkeyfd = openat(AT_FDCWD,DNKEYPATH,O_DIRECTORY);

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

char* yield_dnhstr(DiNode** dirnode){
    char* str_buf = (char*) sodium_malloc(crypto_generichash_BYTES*2+1);

    unsigned char *hsh_buf;
    yield_dnhsh(dirnode, &hsh_buf);

    dn_hdid_str(&hsh_buf,&str_buf);
    free(hsh_buf);

    return str_buf;

}


int make_bridgeanchor(DiNode** dirnode, char** path, unsigned int pathlen) {

    int dnkeyfd = openat(AT_FDCWD,DNKEYPATH,O_DIRECTORY);

    if (dnkeyfd < 0 )
    {
        perror("make_bridgeanchor/dnkeyfd");
        close(dnkeyfd);
        return -1;
    }

    int dnfd = openat(AT_FDCWD,*path,O_DIRECTORY);


    if (dnfd < 0 )
    {
        perror("make_bridgeanchor/dnfd");

        close(dnfd);
        close(dnkeyfd);
        return -1;
    }

    unsigned char* dn_hash = (unsigned char*) sodium_malloc(crypto_generichash_BYTES);
    unsigned char* dkey = (unsigned char*) sodium_malloc(crypto_shorthash_KEYBYTES);
    char* dn_hdid = (char*) sodium_malloc(crypto_generichash_BYTES*2+1);

    recv_little_hash_key(dnkeyfd,((*dirnode)->diname),expo_dirnmlen((*dirnode)->did),dkey);
    real_hash_keyfully((&(*dirnode)->diname), &dn_hash, expo_dirnmlen((*dirnode)->did), (const unsigned char **) &dkey, crypto_shorthash_KEYBYTES);
    dn_hdid_str(&dn_hash, &dn_hdid);


    sodium_free(dkey);
    sodium_free(dn_hash);
    close(dnkeyfd);

    int diranch = openat(dnfd, (char*) dn_hdid, O_CREAT|O_RDWR, S_IRWXU);

    //TODO: handle write error for bridge anchor
    write(diranch, *path, (pathlen+expo_dirnmlen((*dirnode)->did)));
    fsync(diranch);


    if (diranch < 0 )
    {
        close(diranch);
        close(dnfd);
        sodium_free(dn_hdid);
        return -1;
    }

    return 0;
}


void init_armatr(Armature*** armatr, unsigned int size, unsigned char** hkey){

    (**armatr) = (Armature*) malloc(sizeof(Armature));

    (**armatr)->totsize=size;
    (**armatr)->count=0;
    ((**armatr)->entries) = (NodeEntries*) calloc(size, sizeof(NodeEntries));

    for(int i = 0; i<(**armatr)->totsize; i++){
        (**armatr)->entries->hshno = 0;
    }

    memcpy((&(**armatr)->lttc_key),*hkey,LKEYSZ);
}

FiNode* mk_finnode(unsigned int nlen, unsigned char* finame,
                   unsigned long long fiid, unsigned long long did,
                   unsigned long fhshno){

    FiNode* fimap = (FiNode*) malloc(sizeof(FiNode));

    fimap->finame = (unsigned char*) calloc(nlen+1, sizeof(unsigned char));
    memcpy(fimap->finame,finame,(nlen*sizeof(unsigned char)));

    fiid = msk_finmlen(fiid,nlen);
    fiid = msk_format(fiid, grab_ffid(fimap->finame,nlen));
    fiid = msk_resdir(fiid, did);

    fimap->fiid = fiid;
    fimap->fhshno = finode_idx(fhshno);

    printf("%lu\n",fimap->fhshno);

    return fimap;
}

unsigned int getidx(unsigned long fhshno)
{
    return fhshno & HTMASK;
}

unsigned int finode_idx(unsigned long fhshno)
{
    unsigned long rnd;
    unsigned int clk;
    clk = fhshno & 15;
    unsigned int msk = 1431655765;
    msk = (clk > 7) ? msk << clk : msk >> clk;
    fhshno ^= (msk);
    return fhshno;
}


void add_entry(FiNode* finode, Armature* armatr) {
    int x = 0;
    timr_st();
    NodeEntries ne;

    if ((armatr->entries[getidx(finode->fhshno)]).hshno != 0) {

        while (((armatr->entries)[getidx(finode->fhshno)].hshno) != 0) {
            (finode->fhshno) = finode_idx((finode->fhshno));
            printf("EC! X:%lu\n",HTMASK&(finode->fhshno));
            coll_upup();
            if(++x > 3){
                rando_sf(&finode->fhshno);
                if (x > 9){
                    return;
                }
            }
        }
        //finode->fiid = ((finode->fhshno)>>1);
    }

    ne.hshno = finode->fhshno;
    ne.fiid = finode->fiid;
    armatr->entries[getidx(finode->fhshno)] = ne;
    armatr->count++;
    inc_collcntr();
    timr_hlt();
}

HashLattice * init_hashlattice(DChains * diChains) {

    //maxsize = max dirs * max files


    HashLattice* hashlattice = (HashLattice *) malloc(sizeof(HashLattice));
    hashlattice->max = LTTCMX;
    hashlattice->count = 0;

    hashlattice->bridges = (HashBridge **) calloc(LTTCMX, sizeof(HashBridge));

    hashlattice->chains = *diChains;
    hashlattice->chains->vessel = hashlattice->chains->dir_head;

    return hashlattice;
}

void build_bridge2(Armature* armatr, FiNode* fiNode, DiNode* dnode, HashLattice* hashlattice, unsigned char obuf[16]){

    timr_st();
    if (hashlattice->count > hashlattice->max-3){
        fprintf(stderr, "Hash lattice full\n");
    }

    HashBridge* hshbrg = (HashBridge*) malloc(sizeof(HashBridge));

    unsigned long long int inid = (fiNode->fiid ^ dnode->did);
    memcpy(hshbrg->unid,&inid,8);

    unsigned long idx = latt_hsh_idx((armatr), fiNode->fhshno, obuf);
    //unsigned long idx = little_hsh_llidx((armatr->lttc_key), fiNode->finame, 16, dnode->did) & LTTCMX;

    hshbrg->dirnode = dnode;
    hshbrg->finode = fiNode;
    hshbrg->parabridge = NULL;
    //hashlattice->chains->vessel->armature = armatr;

    if (hashlattice->bridges[idx] == 0){
        hashlattice->bridges[idx] = hshbrg;
    }else {
        ParaBridge parabrg = hashlattice->bridges[idx];
        int x = 0;
        printf("\nBridgeCollision!\n");
        while (((parabrg)->parabridge) != NULL){
            (parabrg) = (parabrg)->parabridge;
            printf("para-level: %d", x);
        }
        (parabrg)->parabridge = hshbrg;
    }
    hashlattice->count++;

    //printf("[%.2ld]\n",cb-ca);
    timr_hlt();
}

HashBridge *yield_bridge_for_fihsh(Lattice lattice,unsigned long fiHsh){

    HashBridge * hshBrdg;
    unsigned int fflg = 0;
    unsigned char oBuf [16] = {0};
    unsigned char idBuf [8] = {0};
    unsigned long long int biidBufL;

    unsigned long long int biid = lattice->chains->vessel->armature->entries[getidx(fiHsh)].fiid;
    if (biid == 0){
        return NULL;
    }else {
        biid ^= lattice->chains->vessel->did;
    }


    unsigned long brdgIdx = latt_hsh_idx(lattice->chains->vessel->armature,fiHsh,oBuf);
    hshBrdg = lattice->bridges[brdgIdx];


    memcpy(&biidBufL,&(hshBrdg->unid),8);

    if (biidBufL != biid){
        while (hshBrdg->parabridge != NULL){
            hshBrdg = hshBrdg->parabridge;
            memcpy(&biidBufL,hshBrdg->unid,8);
            if (biidBufL == biid){
                fflg = 1;
                break;
            }
        }
    }else{
        fflg = 1;
    }

    if (!fflg){
        return NULL;
        //TODO: implament global status frame.
    }else{
        return hshBrdg;
    }
}

/*
*HashBridge* yield_bridge(HashLattice* hashLattice, unsigned char* fiid, unsigned int n_len, DiNode* root_dnode) {
*
*    unsigned char *kbuf = (unsigned char*) sodium_malloc(crypto_shorthash_KEYBYTES);
*    int dnkeyfd = openat(AT_FDCWD,DNKEYPATH,O_DIRECTORY,O_RDONLY);
*
*    recv_little_hash_key(dnkeyfd, root_dnode->diname, expo_dirnmlen(root_dnode->did), kbuf);
*
*    close(dnkeyfd);
*    unsigned long idx = little_hsh_llidx(kbuf, fiid, n_len, root_dnode->did) & LTTCMX;
*
*    return hashLattice->bridges[idx];
*}
*
*HashBridge* yield_hshbrg(HashLattice* hashLattice, Armature* armatr, unsigned long fhshno, unsigned int n_len, unsigned char obuf[16]) {
*
*
*    unsigned long idx = latt_hsh_idx(armatr, fhshno, obuf);
*
*    return hashLattice->bridges[idx];
*}
*
*void destoryhashbridge(HashBridge hashbridge){
*    free((hashbridge).finode);
*
*}
*
*void destryohashlattice(HashLattice* hashlattice) {
*    int i = 0;
*    for (i = 0; i < hashlattice->max-1; i++){
*        if ((hashlattice->bridges[i]) != 0 && hashlattice->bridges[i] != NULL) {
*       //    destoryhashbridge((*(hashlattice)->bridges)[i]);
*        }
*    }
*    for (i =0; i< PARAMX; i++ ){
*     //   destoryhashbridge((*(hashlattice)->para_bridges)[i]);
*    }
*    free(hashlattice);
*}
*
*void destroy_node(FiNode* finode, Armature* armatr) {
*    unsigned short idx = finode->fhshno & HTMASK;
*    free(finode->finame);
*    free(finode);
*
*    //armatr->entries[idx] = NULL;
*    armatr->count--;
*}
*
*int destroy_ent(Armatr armatr, unsigned long idx){
*    //free(armatr->entries[idx]);
*    //return armatr->entries[idx] ? 1 : 0;
*}
*
*void destroy_armatr(Armatr armatr, HashLattice hashLattice) {
*
*    for (int i =0; i < armatr->totsize; i++) {
*
*
*
*
*    }
*    free(armatr->entries);
*
*    if (armatr->count > 0) {
*        fprintf(stderr, "Entry destruction failed. Table count: %d\n", armatr->count);
*    }else{
*    }
*    free(armatr);
*
*
*}
*
*void destroy_chains(DChains dirChains) {
*    int i;
*
*    dirChains->vessel = dirChains->dir_head->right;
*    unsigned char nmnodes = (dirChains->vessel->did & DGCNTMASK) >> DGCNTSHFT;
*    goto_chain_tail(dirChains, 1);
*    free(dirChains->vessel->right);
*    for (i = 0; i < nmnodes; i++) {
*        dirChains->vessel = dirChains->vessel->left;
*        free(dirChains->vessel->right->diname);
*        free(dirChains->vessel->right);
*    }
*
*    dirChains->vessel = dirChains->dir_head->left;
*    unsigned char ndnodes = (dirChains->vessel->did & DGCNTMASK) >> DGCNTSHFT;
*    goto_chain_tail(dirChains, 0);
*    free(dirChains->vessel->left);
*    for (i = 0; i < ndnodes; i++) {
*        dirChains->vessel = dirChains->vessel->right;
*        free(dirChains->vessel->left->diname);
*        free(dirChains->vessel->left);
*    }
*
*    dirChains->vessel = dirChains->dir_head;
*    free(dirChains->vessel->right->diname);
*    free(dirChains->vessel->left->diname);
*    free(dirChains->vessel->right);
*    free(dirChains->vessel->left);
*    free(dirChains->dir_head->diname);
*    //free(dirChains->vessel);
*}
*/

int filter_dirscan(const struct dirent *entry) {
    return ((entry->d_name[0] == 46)) || (strnlen(entry->d_name, 4) > 3);
}

double long* map_dir(const char* dir_path,
            unsigned int path_len,
            unsigned char* rootdirname,
            unsigned int dnlen,
            DiChains* dirchains,
            HashLattice* hashlattice,
            Armature** armatr) {

    int i;
    int j=0;
    int k=0;

    // Open the directory and read in the contents

    struct dirent ***dentrys = (struct dirent***) sodium_malloc(sizeof(struct dirent**));
    int n;
    int dir_cnt = 0;

    unsigned char* hkey = (unsigned char*) sodium_malloc(sizeof(unsigned char)*crypto_shorthash_KEYBYTES);

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
    dirNode_ptr = add_dnode(rootiid,rootdirname,dnlen,1,&hashlattice,*armatr);


    if (make_bridgeanchor(&dirNode_ptr, (char **) &dir_path, path_len) < 0)
    {
        perror("Failed making bridge anchor\n");
    }

    // scandir returns number of entrys in the directory
    n = scandir(dir_path, dentrys, NULL, alphasort);

    // Alloc arrays for filenames, file ids, entry type, and length of filename
    unsigned char** farr = (unsigned char **) calloc(n, 256*sizeof(unsigned char));
    unsigned long* idarr = (unsigned long*) calloc(n, sizeof(unsigned long));
    unsigned char* entype = (unsigned char*) calloc(n, sizeof (unsigned char));
    unsigned long* nlens= (unsigned long*) calloc(n, sizeof(unsigned long));
    unsigned char dubbuf[16] = {0};

    // For each entry in the directory...
    for (i=0;i<n;i++){
        // Filename length
        nlens[i] = strnlen((*dentrys)[i]->d_name, FINMMAXL);
        // Filename char arr
        farr[i] = (unsigned char*) calloc((nlens[i]+1),sizeof (unsigned char));
        memcpy(farr[i], (const unsigned char*)(*dentrys)[i]->d_name,nlens[i]+1);
        // File inode number
        idarr[i] = (*dentrys)[i]->d_ino;
        // Filetype (i.e. directory or file)
        entype[i] = (*dentrys)[i]->d_type;
        if (entype[i] == TYPDIR){
            dir_cnt++;
        }
        free((*dentrys)[i]);
    }
    free(*dentrys);

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
            fimap_ptr = mk_finnode(nlens[i], farr[i], haidarr[j], rootino, *fhshno_arr[j]);
            // Add the entry to the file/hash table
            add_entry(fimap_ptr,*armatr);
            // Add a lattice bridge to connect filemap object to the dir node
            build_bridge2(*armatr, fimap_ptr,dirNode_ptr, hashlattice, dubbuf);
            sodium_free(fhshno_arr[j]);
            j++;

        }
        free(farr[i]);
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
        travel_dchains(&(hashlattice->chains->vessel), 1, 1);
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
    sodium_free(dentrys);
    sodium_free(hkey);
    free(farr);
    free(idarr);
    free(entype);
    free(nlens);
    free(haidarr);
    free(fhshno_arr);

    return 0;

}