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

void mk_one_dir_hashes(unsigned long long* iid, unsigned long long* hshno, const unsigned long ino){
    genrandsalt(hshno);
    *iid = ((((*hshno)>>DMASKSHFT)<<DMASKSHFT) ^ (ino<<DMASKSHFT));
}

void mk_hashes(unsigned long long* haidarr,
               unsigned long** fhshno_arr,
               unsigned long long* dhaidarr,
               unsigned long long** dhshno_arr,
               const unsigned long* idarr,
               int n,int nd,
               const unsigned char *entype) {

    get_many_little_salts(fhshno_arr, (n-nd-1));
    get_many_big_salts(dhshno_arr, nd-1);

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

            dhaidarr[k] = ((((*dhshno_arr[k])>>DMASKSHFT)<<DMASKSHFT) ^ (idarr[i]<<DMASKSHFT));

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


//lor: 0=left/docs, 1=right/media
//void travel_dchains(Dir_Node* dnode, unsigned char lor, unsigned char steps) {
//
//    if (steps > 0) {
//        dnode = (lor) ? dnode->left : dnode->right;
//        steps--;
//        travel_dchains(dnode, lor, steps);
//    }
//}

////lor: 0=left/docs, 1=right/media
//void walk_dchains(Dir_Node * dnode, unsigned char lor, unsigned char steps) {
//    while (steps > 0){
//        dnode = (lor) ? (dnode)->left : (dnode)->right;
//        steps--;
//    }
//}

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

//lor: 0=left/docs, 1=right/media,
void search_dchains(Dir_Node* dnode, unsigned long long did, unsigned int lor){

    while ((dnode->did) != did){
        *dnode = (lor) ? *dnode->left : *dnode->right;
    }
}

//lor: 0=left/docs, 1=right/media,
void search_dchns_byino(Dir_Node* dnode, unsigned long long did, unsigned int lor){
    while ((dnode->did) != did){

        *dnode = (lor) ? *dnode->left : *dnode->right;
    }
}



void traverse_dchains(Dir_Node* dnode) {

    while(dnode->did != (DROOTDID)){
        printf("%s\n", dnode->diname);
        dnode = dnode->left;

    }
}

//mord: 0=docs, 1=media
Dir_Node* add_dnode(unsigned long long did, unsigned char* dname, unsigned short nlen, unsigned int mord, Dir_Chains* dirchains) {

    Dir_Node *dnode = (Dir_Node *) (malloc(sizeof(Dir_Node)));
    Dir_Node *base = (mord) ? dirchains->dir_head->right : dirchains->dir_head->left;
    dnode->diname = (unsigned char *) malloc(nlen * sizeof(unsigned char));
    dnode->diname = memcpy(dnode->diname, dname, nlen * sizeof(unsigned char));
    unsigned long cnt = ((base->did) & DGCNTMASK) >> DGCNTSHFT;

    dnode->did = did | ((nlen & FNLENMSK) << DNAMESHFT) | ((mord) ? MEDABASEM : DOCSBASEM) | ++cnt;
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


Fi_Tbl* init_fitbl(unsigned int size){

    Fi_Tbl* fitbl = (Fi_Tbl*) malloc(sizeof(Fi_Tbl));

    fitbl->totsize=size;
    fitbl->count=0;
    fitbl->entries = (FiMap **) calloc(size+1,sizeof(FiMap*));

    for (int i = 0; i < size;i++)
    {
        fitbl->entries[i] = NULL;
    }

    return fitbl;
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
        fimap->fiid = *hshno ^ *fino;


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

void make_bridge(FiMap* fimap, Dir_Node* dnode,HashLattice* hashlattice, unsigned char* hkey){

    if (hashlattice->count > hashlattice->max-3){
        fprintf(stderr, "Hash lattice full\n");
    }

    HashBridge* hshbrg = (HashBridge*) sodium_malloc(sizeof(HashBridge));

    unsigned long idx = little_hsh_llidx(hkey, fimap->finame, expo_finmlen(fimap->fiid), dnode->did) & LTTCMX;

    //        printf(">>>>>%s\n",idx);

    int i = 0;
    while ((hashlattice->bridges[idx]) != 0 || (hashlattice->bridges[idx]) != NULL) {
        printf("Collision!inc: %d", i);
        i++;
        idx += i;
    }



        hshbrg->dirnode = dnode;
        hshbrg->finode = fimap;
        hshbrg->unid = sodium_malloc(sizeof(unsigned long long));
        hashlattice->bridges[idx] = hshbrg;
        hashlattice->count++;
}

void destoryhashbridge(HashBridge* hashbridge){
    sodium_free(hashbridge->unid);

    sodium_free(hashbridge);
}

void destryohashlattice(HashLattice* hashlattice) {
    for (int i = 0; i < hashlattice->max-1; i++){
        if ((hashlattice->bridges[i]) != NULL) {
           destoryhashbridge((hashlattice->bridges[i]));
        }
    }
    free(hashlattice->bridges);
    free(hashlattice);

}
//void add_bridge(FiMap* fimap, HashBridge* hashbridge) {
//    BridgeNode bridx = sodium_malloc(crypto_generichash_BYTES);
//
//    if ((fimap->finame,bridx,(expo_finmlen(fimap->fiid)*sizeof(unsigned char))) != 0){
//        fprintf(stderr, "Bridge hash failed\n");
//    }
//    else {
//        hashbridge->&bridx) =
//    }
//
//
//
//}
//
//unsigned long bridge_hash(char* finame) {
//    return 0;
//}


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
    free(fitbl->entries);
    //free(fitbl);
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

void void_mkmap(const char* dir_path, unsigned char* rootdirname, unsigned int dnlen){

    int i;
    int j=0;
    int k=0;
    int naclinit = sodium_init();
    if( naclinit != 0){
        fprintf(stderr, "Sodium init failed: %d",naclinit);
    }

    struct dirent ***dentrys = (struct dirent***) sodium_malloc(sizeof (struct dirent***));
    int n;
    int dir_cnt = 0;

    n = scandir(dir_path, dentrys, NULL, alphasort);

    unsigned char** farr = (unsigned char **) calloc(n, 256*sizeof(unsigned char));
    unsigned long* idarr = (unsigned long*) calloc(n, sizeof(unsigned long));
    unsigned char* entype = (unsigned char*) calloc(n, sizeof (unsigned char));
    unsigned long* nlens= (unsigned long*) calloc(n, sizeof(unsigned long));


    for (i=0;i<n;i++){
        nlens[i] = strnlen((*dentrys)[i]->d_name, FINMMAXL);
        farr[i] = (unsigned char*) calloc((nlens[i]+1),sizeof (unsigned char));
        memcpy(farr[i], (const unsigned char*)(*dentrys)[i]->d_name,nlens[i]+1);
        idarr[i] = (*dentrys)[i]->d_ino;
        entype[i] = (*dentrys)[i]->d_type;
        if (entype[i] == TYPDIR){
            dir_cnt++;
        }
        free((*dentrys)[i]);
    }

    unsigned long** fhshno_arr = (unsigned long**) calloc((n-dir_cnt),sizeof(unsigned long*));
    unsigned long long* haidarr = (unsigned long long*) calloc((n-dir_cnt),sizeof(unsigned long long));
    unsigned long long** dhshno_arr = (unsigned long long**) calloc(dir_cnt, sizeof(unsigned long long*));
    unsigned long long* dhaidarr = (unsigned long long*) calloc(dir_cnt, sizeof(unsigned long long));

    // const char* dirname, unsigned int dnlen
    unsigned long long rootiid;
    unsigned long long roothshno;
    unsigned long rootino = cwd_ino(".");

    mk_one_dir_hashes(&rootiid,&roothshno,rootino);

    mk_hashes(haidarr, fhshno_arr, dhaidarr, dhshno_arr, idarr, n, dir_cnt, entype);

    Fi_Tbl* fitbl = init_fitbl(HTSIZE);
    Dir_Chains* dirchains = init_dchains();
    HashLattice* hashlattice = init_hashlattice();

    FiMap* fimap_ptr;
    Dir_Node* dirNode_ptr;
    unsigned char* hkey = (unsigned char*) malloc(sizeof(unsigned char)*crypto_shorthash_KEYBYTES);

    dirNode_ptr = add_dnode(rootiid,rootdirname,dnlen,1,dirchains);

    mk_little_hash_key(hkey);
    dump_little_hash_key(hkey,rootdirname,dnlen);

    for (i=0;i<n;i++) {
        if (entype[i] == TYPDIR){

            printf("DIR: %s :", farr[i]);
            printf("\n%llu\n",dhaidarr[k]);
            printf("%llu\n\n",((((*dhshno_arr[k])>>DMASKSHFT))^((dhaidarr[k])>>DMASKSHFT)));

            add_dnode(dhaidarr[k],farr[i],nlens[i],1,dirchains);

            sodium_free(dhshno_arr[k]);

            k++;
        }
        else {
            fimap_ptr = mk_fimap(nlens[i],farr[i],haidarr[j],rootino,*fhshno_arr[j]);
            add_entry(fimap_ptr,fitbl);
            make_bridge(fimap_ptr,dirNode_ptr,hashlattice, hkey);
            sodium_free(fhshno_arr[j]);
            j++;

        }

        free(farr[i]);
    }

    for (i = 0; i<fitbl->totsize; i++){
        if (fitbl->entries[i] != NULL){
                        printf("\n%lu: ", expo_fino(fitbl->entries[i]->fhshno,fitbl->entries[i]->fiid));
            printf("%s\n",fitbl->entries[i]->finame);
            printf("fiid: %llu\n",fitbl->entries[i]->fiid);
            printf("fhshno: %lu\n",fitbl->entries[i]->fhshno);
            printf("nmlen: %u\n", expo_finmlen(fitbl->entries[i]->fiid));
            printf("format: %u\n",expo_format(fitbl->entries[i]->fiid));
            printf("Resident dir: %u\n", expo_redir(fitbl->entries[i]->fiid));

            printf("%lu\n",fitbl->entries[i]->fhshno);


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

    }
    printf("\n\n");
    printf("%s\n",dirchains->dir_head->diname);
    printf("%llu\n",dirchains->dir_head->did);


    destryohashlattice(hashlattice);
    destroy_tbl(fitbl);
    destroy_chains(dirchains);
    free(dirchains);
    sodium_free(dentrys);

    free(farr);
    free(idarr);
    free(entype);
    free(nlens);
    free(haidarr);
    free(fhshno_arr);
    free(dhaidarr);
    free(dhshno_arr);
}
