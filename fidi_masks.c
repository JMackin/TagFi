//
// Created by ujlm on 8/24/23.
//
#include "fidi_masks.h"

unsigned long clip_to_32(unsigned long num) {

    return (num & CLIPHSH);
}

unsigned long long msk_fino(unsigned long rndm_no, unsigned long ino) {

    return FMIDTMPL ^ ( (rndm_no ^ ino) << FINOSHFT);
}

unsigned long expo_fino(unsigned long key, unsigned long long fiid) {

    return ((fiid^FMIDTMPL)>>FINOSHFT) ^ key;
}

unsigned long long msk_finmlen(unsigned long fiid, unsigned int fnlen) {
    return fiid ^ (fnlen<<FNLENSHFT);
}

unsigned int expo_finmlen(unsigned long long fiid) {
    return (FNLENMSK & fiid) >> FNLENSHFT;
}

unsigned long long msk_format(unsigned long long fiid, unsigned int fform) {
    return fiid ^ (fform << FFRMTSHFT);
}

unsigned int expo_format(unsigned long long fiid) {
    return (fiid & FFRMTMSK) >> FFRMTSHFT;
}

unsigned long long msk_redir(unsigned long long fiid, unsigned int dirid) {
    return fiid ^ ((dirid & FBSGPCLIP) << FRDIRSHFT);
    //& DBASEMASK) >> DBASESHFT);
}

unsigned int expo_redir(unsigned long long fiid) {
    return (fiid & FRDIRMSK) >> FRDIRSHFT;
}

unsigned long long msk_dirgrp(unsigned long long fiid) {
    return (fiid | (FDCHNGMSK << FDCHNGSHFT));
}

unsigned int expo_dirgrp(unsigned long long fiid, unsigned int digrp) {
    return (fiid & FDCHNGMSK) >> FDCHNGSHFT;
}

/*
 *  Dirnode ops
 * */

unsigned int expo_dirnmlen(unsigned long long did) {
    return (did & DNAMEMASK) >> DNAMESHFT;
}

unsigned int expo_dirbase(unsigned long long did) {
    return (did & DBASEMASK) >> DBASESHFT;
}

unsigned int expo_dirchainid(unsigned long long did) {
    return did & DCHNSMASK;
}