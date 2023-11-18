//
// Created by ujlm on 8/24/23.
//
#include "fidi_masks.h"

/*
 *  FileNode ops
 * */

unsigned long clip_to_32(unsigned long num) {
    return (num & CLIPHSH);
}

unsigned long long msk_fino(unsigned long ino) {

    //return FIIDTMPL | (ino << FINOSHFT);
    return FIIDTMPL | (ino << 30);
}

unsigned long expo_fino(unsigned long key, unsigned long long fiid) {

    return ((fiid | FIIDTMPL) >> FINOSHFT) ^ key;
}

unsigned long long msk_finmlen(unsigned long fiid, unsigned int fnlen) {
    return fiid | (fnlen<<FNLENSHFT);
}

unsigned int expo_finmlen(unsigned long long fiid) {
    return (FNLENMSK & fiid) >> FNLENSHFT;
}

unsigned long long msk_format(unsigned long long fiid, unsigned int fform) {
    return fiid | (fform << FFRMTSHFT);
}

unsigned int expo_format(unsigned long long fiid) {
    return (fiid & FFRMTMSK) >> FFRMTSHFT;
}

unsigned long long msk_resdir(unsigned long long fiid, unsigned int dirid) {
    return fiid | ((dirid & FBSGPCLIP) << FRDIRSHFT);
}

unsigned int expo_resdir(unsigned long long fiid) {
    return (fiid & FRDIRMSK) >> FRDIRSHFT;
}

unsigned long long msk_dirgrp(unsigned long long fiid, unsigned int dirid) {
    return fiid | (dirid << FDCHNGSHFT);
}

unsigned int expo_dirgrp(unsigned long long fiid) {
    return (fiid & FDCHNGMSK) >> FDCHNGSHFT;
}

/*
 *  DirNode ops
 * */

unsigned int expo_dirnmlen(unsigned long long did) {
    return (did & DNAMEMASK) >> DNAMESHFT;
}

unsigned int msk_dirnmlen(unsigned long long did, unsigned int dirnmln){
    return did | (dirnmln << DNAMESHFT);
}

unsigned int expo_dirbase(unsigned long long did) {
    return (did >> DBASESHFT) & 1;
}

/* *Note: Use MEDACHSE or DOCSCHCE for param 'base'*/
unsigned int msk_dirbase(unsigned long long did, unsigned int base){
    return did | (base ? MEDABASEM : DOCSBASEM);
}
unsigned int expo_dirchnid(unsigned long long did) {
    return (did & DCHNSMASK) ;
}
unsigned int msk_dirchnid(unsigned long long did, unsigned int id){
    return did | ( id & 255);
}

/*
 *  BaseNode ops
 * */

unsigned int expo_basedir_cnt(unsigned long long did){
    return (did & DGCNTMASK) >> DGCNTSHFT;
}
unsigned int msk_basedir_cnt(unsigned long long did, unsigned int cnt){
    return did | (cnt << DNTRYSHFT);
}