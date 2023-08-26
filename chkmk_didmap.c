//
// Created by ujlm on 8/6/23.
//
#include "chkmk_didmap.h"
#include "jlm_random.h"
#include "fiforms.h"
#include "fidi_masks.h"
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




void mk_hashes(unsigned long long* haidarr,
               unsigned long** fhshno_arr,
               unsigned long long* dhaidarr,
               unsigned long long** dhshno_arr,
               const unsigned long* idarr,
               int n,int nd,
               const unsigned short* entype) {

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

void traverse_dchains(Dir_Node* dnode) {

    while(dnode->did != (DROOTDID)){
        printf("%s\n", dnode->diname);
        dnode = dnode->left;

    }
}

//mord: 0=docs, 1=media
void add_dnode(unsigned long long did, unsigned char* dname, unsigned short nlen, unsigned int mord, Dir_Chains* dirchains) {

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
}


Fi_Tbl* init_fitbl(unsigned int size){

    Fi_Tbl* fitbl = (Fi_Tbl*) malloc(sizeof(Fi_Tbl*));

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
        *fino = expo_fino(fimap->fhshno,fimap->fiid);
        sodium_mprotect_readonly(fino);


        do {
            (*hshno)++;
        }while (fiTbl->entries[*hshno & HTMASK] != NULL);

        fimap->fhshno = *hshno;
        fimap->fiid = *hshno ^ *fino;


        sodium_free(hshno);
        sodium_free(fino);
    }

    fiTbl->entries[getidx(fimap)] = fimap;
    fiTbl->count++;
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
    free(fitbl->entries);
    free(fitbl);
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
    free(dirChains->vessel);
}



void void_mkmap(const char* dir_path){

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

    n = scandir(dir_path, dentrys, NULL, NULL);

    unsigned char** farr = (unsigned char **) calloc(n, sizeof(unsigned char*));
    unsigned long* idarr = (unsigned long*) calloc(n, sizeof(unsigned long));
    unsigned short* entype = (unsigned short*) calloc(n, sizeof (unsigned short));
    size_t* nlens= (size_t*) calloc(n, sizeof(size_t));

    unsigned long int rootno = (*dentrys)[0]->d_ino;


    for (i=0;i<n;i++){
        nlens[i] = strlen((*dentrys)[i]->d_name);
        farr[i] = (unsigned char*) calloc((nlens[i]),sizeof (unsigned char));
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


    mk_hashes(haidarr, fhshno_arr, dhaidarr, dhshno_arr, idarr, n, dir_cnt, entype);


    Fi_Tbl* fitbl = init_fitbl((int)HTSIZE);
    Dir_Chains* dirchains = init_dchains();


    for (i=0;i<n;i++) {

        if (entype[i] != TYPDIR){
            add_entry(mk_fimap(nlens[i],farr[i],haidarr[j],rootno,*fhshno_arr[j]),fitbl);
            sodium_free(fhshno_arr[j]);
            j++;
        }
        else {

            printf("DIR: %s :", farr[i]);
            printf("\n%llu\n",dhaidarr[k]);
            printf("%llu\n\n",((((*dhshno_arr[k])>>DMASKSHFT))^((dhaidarr[k])>>DMASKSHFT)));
            add_dnode(dhaidarr[k],farr[i],nlens[i],1,dirchains);
            sodium_free(dhshno_arr[k]);
            k++;

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
            printf("%lu\n\n");

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

    printf("\n%llu", ULLONG_MAX);

    printf("\n%lu", ULONG_MAX);


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
