//
// Created by ujlm on 8/6/23.
//
#include "chkmk_didmap.h"
#include "jlm_random.h"
#include "fiforms.h"
#include "fidi_masks.h"
#include "tagfi.h"
#include <sodium.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>


#define DNKEYPATH "/home/ujlm/Code/Clang/C/TagFI/keys"


void mk_one_dir_hashes(unsigned long long* iid, unsigned long long* hshno, const unsigned long ino){
    genrandsalt(hshno);
    *iid = ((((*hshno)>>DMASKSHFT)<<DMASKSHFT) ^ (ino<<DMASKSHFT));
}




void mk_hashes(unsigned long long* haidarr,
               unsigned long** fhshno_arr,
               //unsigned long long* dhaidarr,
               //unsigned long long** dhshno_arr,
               const unsigned long* idarr,
               int n,int nd,
               const unsigned char *entype) {

    get_many_little_salts(fhshno_arr, (n-nd-1));
    //get_many_big_salts(dhshno_arr, nd-1);

    int i;
    int j = 0;
    int k = 0;

    for (i = 0; i < n; i++) {
        if (entype[i] == TYPFIL){


            (*(fhshno_arr[j])) = clip_to_32((*(fhshno_arr[j])));

            haidarr[j] = msk_fino((*(fhshno_arr[j])), idarr[i]);
            printf("\n%llu\n\n", haidarr[j]);

            j++;
        }
        if (entype[i] == TYPDIR) {

            //dhaidarr[k] = ((((*dhshno_arr[k])>>DMASKSHFT)<<DMASKSHFT) ^ (idarr[i]<<DMASKSHFT));

            k++;
        }
    }
}

Dir_Node* mk_tailnode(){
    Dir_Node* tail = (Dir_Node*) malloc(sizeof(Dir_Node));
    tail->diname = 0;
    tail->did = 0;

    return tail;
}

Dir_Chains* init_dchains() {
    const int init_nidx[3] = {1,2,4};
    const char* init_dnames[3] = {"HEAD","MEDA", "DOCS"};
    Dir_Node** init_dnodes = (Dir_Node**) calloc(3, 3*sizeof(Dir_Node*));
    Dir_Chains* dirchains = (Dir_Chains*) malloc(sizeof(Dir_Chains));
 //dirchains->vessel = (Dir_Node*) malloc(sizeof(Dir_Node));
//dirchains->dir_head = (Dir_Node*) malloc(sizeof(Dir_Node));


    for (int i = 0; i < 3; i++) {

        init_dnodes[i] = (Dir_Node*) malloc(sizeof(Dir_Node));
        init_dnodes[i]->diname = (unsigned char*) malloc(8);
        memcpy(init_dnodes[i]->diname, init_dnames[i],(8));
        init_dnodes[i]->did = DGRPTMPL|init_nidx[i];
    }

    init_dnodes[0]->right=init_dnodes[1]; init_dnodes[0]->left=init_dnodes[2]; // docs <-l head r-> meda
    init_dnodes[1]->left=init_dnodes[0];// head <-l meda
    init_dnodes[2]->right=init_dnodes[0];// docs r-> head
    init_dnodes[1]->right = mk_tailnode();
    init_dnodes[2]->left = mk_tailnode();

    dirchains->dir_head = init_dnodes[0];
    dirchains->vessel = dirchains->dir_head;

    free(init_dnodes);
    return dirchains;
}

void travel_dchains(Dir_Chains* dirChains, unsigned int lor, unsigned char steps) {

    while (steps > 0){
        if (lor) {
            if(dirChains->vessel->right->did == 0){
                break;
            }else
            {
                dirChains->vessel = dirChains->vessel->right;
            }
        }else
        {
            if(dirChains->vessel->left->did == 0){
                break;
            }else{
                dirChains->vessel = dirChains->vessel->left;
            }
        }
        steps--;
    }
}

void goto_chain_tail(Dir_Chains* dirChains, unsigned int lor){
    dirChains->vessel = dirChains->dir_head;

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


void traverse_dchains(Dir_Node* dnode) {

    while(dnode->did != (DROOTDID)){
        printf("%s\n", dnode->diname);
        dnode = dnode->left;
    }
}

unsigned int gotonode(unsigned long long did, Dir_Chains* dchns){

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
Dir_Node* add_dnode(unsigned long long did, unsigned char* dname, unsigned short nlen, unsigned int mord, Dir_Chains* dirchains) {

    Dir_Node *dnode = (Dir_Node *) (malloc(sizeof(Dir_Node)));
    Dir_Node *base = (mord) ? dirchains->dir_head->right : dirchains->dir_head->left;
    dnode->diname = (unsigned char *) calloc(1+nlen,sizeof(unsigned char));
    dnode->diname = memcpy(dnode->diname, dname, nlen * sizeof(unsigned char));
    unsigned long cnt = ((base->did) & DGCNTMASK) >> DGCNTSHFT;

    dnode->did = (did | ((nlen) << DNAMESHFT) | ((mord) ? MEDABASEM : DOCSBASEM) | ++cnt);
    base->did += (64);

    dirchains->vessel = dirchains->dir_head;
    travel_dchains(dirchains, mord, (cnt));

    //MEDA
    if (mord) {
        dnode->left = dirchains->vessel;
        dnode->right = dirchains->vessel->right;
        dirchains->vessel->right = dnode;
    }
        //DOCS
    else {
        dnode->right = dirchains->vessel;
        dnode->left = dirchains->vessel->left;
        dirchains->vessel->left = dnode;
    }
    return dnode;
}

void yield_dnhsh(Dir_Node** dirnode, unsigned char** dn_hash) {

    int dnkeyfd = openat(AT_FDCWD,DNKEYPATH,O_DIRECTORY);

    if (dnkeyfd < 0 )
    {
        perror("make_bridgeanchor/dnkeyfd");
        close(dnkeyfd);
    }

    *dn_hash = (unsigned char*) sodium_malloc(crypto_generichash_BYTES);
    unsigned char* dkey = (unsigned char*) sodium_malloc(crypto_shorthash_KEYBYTES);

    //TODO: OPTIMIZE THIS
    recv_little_hash_key(dnkeyfd,((*dirnode)->diname),expo_dirnmlen((*dirnode)->did),dkey);

    real_hash_keyfully((&(*dirnode)->diname), dn_hash, expo_dirnmlen((*dirnode)->did), (const unsigned char **) &dkey, crypto_shorthash_KEYBYTES);

    sodium_free(dkey);
    close(dnkeyfd);
}

char* yield_dnhstr(Dir_Node** dirnode){
    char* str_buf = (char*) sodium_malloc(crypto_generichash_BYTES*2+1);

    unsigned char *hsh_buf;
    yield_dnhsh(dirnode, &hsh_buf);

    dn_hdid_str(&hsh_buf,&str_buf);
    sodium_free(hsh_buf);

    return str_buf;

}


int make_bridgeanchor(Dir_Node** dirnode, char** path, unsigned int pathlen) {

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


void init_fitbl(Fi_Tbl*** fitbl, unsigned int size){

    (**fitbl) = (Fi_Tbl*) malloc(sizeof(Fi_Tbl));

    (**fitbl)->totsize=size;
    (**fitbl)->count=0;
    (**fitbl)->entries = (FiMap **) calloc(size+1,sizeof(FiMap*));

    for (int i = 0; i < size;i++)
    {
        (**fitbl)->entries[i] = NULL;
    }

}

FiMap* mk_fimap(unsigned int nlen, unsigned char* finame,
                unsigned long long fiid, unsigned  long long did,
                unsigned long fhshno){

    FiMap* fimap = (FiMap*) malloc(sizeof(FiMap));

    fimap->finame = (unsigned char*) calloc(nlen+1, sizeof(unsigned char));
    memcpy(fimap->finame,finame,(nlen*sizeof(unsigned char)));

    fiid = msk_finmlen(fiid,nlen);
    fiid = msk_format(fiid, grab_ffid(fimap->finame,nlen));
    fiid = msk_redir(fiid, did);

    fimap->fiid = fiid;
    fimap->fhshno = fhshno;

    return fimap;
}

unsigned int getidx(FiMap* fimap)
{
    return fimap->fhshno & HTMASK;
}

//unsigned long getfino(FiMap* fimap)
//{
//    return ((fimap->fhshno))^((fimap->fiid));
//}

void add_entry(FiMap* fimap, Fi_Tbl* fiTbl) {

    if (fiTbl->entries[getidx(fimap)] != NULL) {

        unsigned long* fino = (unsigned long*) sodium_malloc(sizeof(unsigned long));
        unsigned long* hshno = (unsigned long*) sodium_malloc(sizeof (unsigned long));

        *hshno = (fimap->fhshno);
        *fino = expo_fino(fimap->fhshno,expo_finmlen(fimap->fiid));
        sodium_mprotect_readonly(fino);

        while (fiTbl->entries[*hshno & HTMASK] != NULL) {
            printf("!Collision!");
            (*hshno)++;
        }

        fimap->fhshno = *hshno;
        fimap->fiid = (*hshno>>1 ^ *fino>>1);

        sodium_free(hshno);
        sodium_free(fino);
    }

    fiTbl->entries[getidx(fimap)] = fimap;
    fiTbl->count++;
}

HashLattice * init_hashlattice() {

    //maxsize = max dirs * max files

    HashLattice* hashlattice = (HashLattice *) malloc(sizeof(HashLattice));
    hashlattice->max = LTTCMX;
    hashlattice->count = 0;

    hashlattice->bridges = (HashBridge **) calloc(LTTCMX, sizeof(HashBridge));

    for (int i = 0; i < hashlattice->count; i++) {
        hashlattice->bridges[i] = NULL;
    }

    return hashlattice;
}

void make_bridge(Fi_Tbl* fitbl, FiMap* fimap, Dir_Node* dnode,HashLattice* hashlattice, unsigned char* hkey){

    if (hashlattice->count > hashlattice->max-3){
        fprintf(stderr, "Hash lattice full\n");
    }

    HashBridge* hshbrg = (HashBridge*) malloc(sizeof(HashBridge));

    unsigned long idx = little_hsh_llidx(hkey, fimap->finame, expo_finmlen(fimap->fiid), dnode->did) & LTTCMX;

    //        printf(">>>>>%s\n",idx);

    int i = 0;
    while ((hashlattice->bridges[idx]) != 0 && (hashlattice->bridges[idx]) != NULL) {
        printf("Collision!inc: %d", i);
        i++;
        idx += i;
    }

        hshbrg->dirnode = dnode;
        hshbrg->finode = fimap;
        hshbrg->fitable = fitbl;
        (hshbrg->unid) = (unsigned long long*) sodium_malloc(sizeof(unsigned long long));
        *(hshbrg->unid) = ((fimap->fiid) ^ dnode->did);
        hashlattice->bridges[idx] = hshbrg;
        hashlattice->count++;
}

HashBridge* yield_bridge(HashLattice* hashLattice, unsigned char* filename, unsigned int n_len, Dir_Node* root_dnode) {

    unsigned char *kbuf = (unsigned char*) sodium_malloc(crypto_shorthash_KEYBYTES);
    int dnkeyfd = openat(AT_FDCWD,DNKEYPATH,O_DIRECTORY,O_RDONLY);

    recv_little_hash_key(dnkeyfd, root_dnode->diname, expo_dirnmlen(root_dnode->did), kbuf);

    close(dnkeyfd);
    unsigned long idx = little_hsh_llidx(kbuf, filename, n_len, root_dnode->did) & LTTCMX;

    return hashLattice->bridges[idx];
}

void destoryhashbridge(HashBridge* hashbridge){
    sodium_free(hashbridge->unid);
    free(hashbridge);
}

void destryohashlattice(HashLattice* hashlattice) {
    for (int i = 0; i < hashlattice->max-1; i++){
        if ((hashlattice->bridges[i]) != NULL) {
            destroy_tbl(hashlattice->bridges[i]->fitable);
           destoryhashbridge((hashlattice->bridges[i]));
        }
    }
    free(hashlattice->bridges);
    free(hashlattice);
}

void destroy_ent(FiMap* fimap, Fi_Tbl* fiTbl) {
    unsigned short idx = fimap->fhshno & HTMASK;
    free(fimap->finame);
    free(fimap);

    fiTbl->entries[idx] = NULL;
    fiTbl->count--;
}

void destroy_tbl(Fi_Tbl* fitbl) {
    for (int i =0; i < fitbl->totsize; i++) {
        if (fitbl->entries[i] != NULL) {
            destroy_ent(fitbl->entries[i],fitbl);
        }
    }
    if (fitbl->count > 0) {
        fprintf(stderr, "Entry destruction failed. Table count: %d\n", fitbl->count);
    }
}

void destroy_chains(Dir_Chains* dirChains) {
    int i;

    dirChains->vessel = dirChains->dir_head->right;
    unsigned char nmnodes = (dirChains->vessel->did & DGCNTMASK) >> DGCNTSHFT;
    goto_chain_tail(dirChains, 1);
    free(dirChains->vessel->right);
    for (i = 0; i < nmnodes; i++) {
        dirChains->vessel = dirChains->vessel->left;
        free(dirChains->vessel->right->diname);
        free(dirChains->vessel->right);
    }

    dirChains->vessel = dirChains->dir_head;
    unsigned char ndnodes = (dirChains->vessel->did & DGCNTMASK) >> DGCNTSHFT;
    goto_chain_tail(dirChains, 0);
    free(dirChains->vessel->left);
    for (i = 0; i < ndnodes; i++) {
        dirChains->vessel = dirChains->vessel->right;
        free(dirChains->vessel->left->diname);
        free(dirChains->vessel->left);
    }

    dirChains->vessel = dirChains->dir_head;
    free(dirChains->vessel->right->diname);
    free(dirChains->vessel->left->diname);
    free(dirChains->vessel->right);
    free(dirChains->vessel->left);
    free(dirChains->dir_head->diname);
    //free(dirChains->vessel);
}

int filter_dirscan(const struct dirent *entry) {
    return ((entry->d_name[0] == 46)) || (strnlen(entry->d_name, 4) > 3);
}

int map_dir(const char* dir_path,
            unsigned int path_len,
            unsigned char* rootdirname,
            unsigned int dnlen,
            Dir_Chains* dirchains,
            HashLattice* hashlattice,
            Fi_Tbl** fitbl) {

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
    Dir_Node* dirNode_ptr;

    // Gen numbers for the root first, to be used in the creation of values for its entries
    mk_one_dir_hashes(&rootiid,&roothshno,rootino);

    // Root node is created and added to the chain
    dirNode_ptr = add_dnode(rootiid,rootdirname,dnlen,1,dirchains);

    // A unique hashkey is created and stored for the dir node
    mk_little_hash_key(&hkey);
    dump_little_hash_key(hkey,rootdirname,dnlen);

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

    // Vars for root, i.e. the directory currently being mapped
//    unsigned long long rootiid;
//    unsigned long long roothshno;
//    unsigned long rootino = cwd_ino(".");
//
//    // Gen numbers for the root first, to be used in the creation of values for its entries
//    mk_one_dir_hashes(&rootiid,&roothshno,rootino);
    // Gen numbers for the entries
    mk_hashes(haidarr, fhshno_arr, idarr, n, dir_cnt, entype);

    // Initialize file table assigned to the root
    init_fitbl(&fitbl,HTSIZE);

    // Each entry (file) is represented by a Filemap containing fiid, fhshno, finame
    FiMap* fimap_ptr;
//    // Each dir node is a link in the dir chains
//    Dir_Node* dirNode_ptr;

//    unsigned char* hkey = (unsigned char*) sodium_malloc(sizeof(unsigned char)*crypto_shorthash_KEYBYTES);
//
//    // Root node is created and added to the chain
//    dirNode_ptr = add_dnode(rootiid,rootdirname,dnlen,1,dirchains);
//
//    // A unique hashkey is created and stored for the dir node
//    mk_little_hash_key(&hkey);
//    dump_little_hash_key(hkey,rootdirname,dnlen);
//
//
//    if (make_bridgeanchor(&dirNode_ptr, (char **) &dir_path, path_len) < 0)
//    {
//        perror("Failed making bridge anchor\n");
//    }

    // For each entry in the root...
    for (i=0;i<n;i++) {

        // If it's a directory
        if (entype[i] == TYPDIR){

            k++;
        }
        // If it's a file
        else {

            // Make a filemap object for it
            fimap_ptr = mk_fimap(nlens[i],farr[i],haidarr[j],rootino,*fhshno_arr[j]);
            // Add the entry to the file/hash table
            add_entry(fimap_ptr,*fitbl);
            // Add a lattice bridge to connect filemap object to the dir node
            make_bridge(*fitbl, fimap_ptr,dirNode_ptr,hashlattice, hkey);
            sodium_free(fhshno_arr[j]);

            j++;

        }
        free(farr[i]);
    }


/**
 *
 *  Begin test section...
 *  -------------------------------
 */

    for (i = 0; i<(*fitbl)->totsize; i++){
        if ((*fitbl)->entries[i] != NULL){
                        printf("\n%lu: ", expo_fino((*fitbl)->entries[i]->fhshno,(*fitbl)->entries[i]->fiid));
            printf("%s\n",(*fitbl)->entries[i]->finame);
            printf("fiid: %llu\n",(*fitbl)->entries[i]->fiid);
            printf("fhshno: %lu\n",(*fitbl)->entries[i]->fhshno);
            printf("nmlen: %u\n", expo_finmlen((*fitbl)->entries[i]->fiid));
            printf("format: %u\n",expo_format((*fitbl)->entries[i]->fiid));
            printf("Resident dir: %u\n", expo_redir((*fitbl)->entries[i]->fiid));

            printf("%lu\n",(*fitbl)->entries[i]->fhshno);
        }
    }

    dirchains->vessel = dirchains->dir_head;
    for (i=0;i<7;i++) {
        travel_dchains(dirchains, 1, 1);
        printf("%s\n", dirchains->vessel->diname);
        printf("%llu\n", dirchains->vessel->did);
        printf("%llu\n", dirchains->vessel->did & DCHNSMASK);
        printf("%llu\n", (dirchains->vessel->did & DBASEMASK) >> DBASESHFT);
        printf("%llu\n", (dirchains->vessel->did & DNAMEMASK) >> DNAMESHFT);
        printf("%llu\n", (dirchains->vessel->did & DNAMEMASK) >> DNAMESHFT);

    }

    printf("\n\n");
    printf("%s\n",dirchains->dir_head->diname);
    printf("%llu\n",dirchains->dir_head->did);

    //unsigned char* bridgefiid = (yield_bridge(hashlattice, "sandpit.tar.gpg", 15, dirNode_ptr, "/home/ujlm/CLionProjects/TagFI/keys/Tech.lhsk"))->dirnode->diname;
    //printf("BRIDGE >> %s", bridgefiid);



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