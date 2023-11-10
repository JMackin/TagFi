/** \verbatim
 *  Bit masks and related functions for TagFi filesystem indexing.
 *
 *  fiid:
 *     0b 1 00000000000000000000000000000000  0000000000  00000000  000000  00  0
 *       C| |               a              |  |   b    |  |   c  |  | d  |  |e| |X
 *
 *
 *           C: Control bit, always true. / 1 bit.
 *           a: (Fhshno^Inum) - Random number xor'd with file INum. / 32 bits.
 *           b: Name length - String-length of the filename. / 9 bits.
 *           c: Format - File format code defined by fiforms.h / 8 bits.
 *           d: Dir. Id - Id number of the resident directory,
 *              defined by sequence of addition to DiChains. / 6 bits.
 *           e: Dir. groupbase - The Dir_chains arm to which
 *              the resident directory belongs.
 *             [ (01=media, 10=docs) / 2 bits. ]
 *           X: Unallocated/Misc switch. / 1 bit.
 */
//
// Created by ujlm on 8/24/23.
//

#ifndef TAGFI_FIDI_MASKS_H
#define TAGFI_FIDI_MASKS_H

#include "lattice_rsps.h"

#define TYPFIL 8        // Regular file flag
#define TYPDIR 4        // Directory flag

/* *
 * DirID masks and attributes * */

#define HTMASK 262140                // Hash table mask -> 15 bits
#define HTSIZE 262140                // Max item count
#define DITMMX 511                  // Max directory count
#define LTTCMX 16777215              // Max item count for lattice: Max dir count (255) * Max FileNode count (16383) - 1
#define PARAMX 65535                // Max item count for para bridge lattice
#define FINMMAXL 256                // Max number of chars allocated for a path title.

#define DTOTALMSK 536870911        // 0b11111111111111111111111111111 - DirNode diid Mask
#define DMASKSHFT 29               // Total mask bits

/*
 * DIR NODES:
 */
#define DCHNSMASK 255              // 0b00000000000000000000011111111 - ID under group-base
#define DBASEMASK 768              // 0b00000000000000000001100000000 - Base ID: 01=media, 10=docs - abs:2
#define MEDABASEM 256              // 0b00000000000000000000100000000 - OR Mask: ID for MEDIA base derived from: (did & DBASEMASK)
#define DOCSBASEM 512              // 0b00000000000000000001000000000 - OR Mask: ID for DOCS base derived from: (did & DBASEMASK
#define DBASESHFT 8                // 0b000000000000000000011xxxxxxxx - Trailing bits after group-base flag: baseID = (DBASEMASK & did) >> DBASESHFT
#define DNAMEMASK 130048           // 0b00000000000011111110000000000 - Str len of directory name - abs:7
#define DNAMESHFT 10               // 0b0000000000001111111xxxxxxxxxx - Trailing bits after dir str-len flag - abs:7
#define DENTRYCNT 1073676288       // 0b11111111111100000000000000000 -
#define DNTRYSHFT 16               // 0b111111111111xxxxxxxxxxxxxxxxx -

/*
 * BASE NODES:
 */
#define MEDACHSE 1
#define DOCSCHCE 0
#define DGCNTMASK 4080             // 0b00000000000111111110000 - Total num. of dirs under group-base. For use on MEDA/DOCS nodes.
#define DGCNTSHFT 4                // 0b0000000000011111111xxxx - Number of trailing bits after base - dir count.

#define DGRPTMPL 4611686018427387904 // 1 leading bit and 62 empty bits, template for dirids
#define DROOTDID 4611686018427387905 // Root node did, empty save for 1 set leading bit and 1 trailing - 60 bits
#define DGMEDAID 4611686018427387906 // MEDA group id
#define DGDOCSID 4611686018427387908 // DOCS group id
#define BIGMASK 1152921504606846975 // 62 1s



  /* * * * * * * * * * * * * * * * * * *
  *  FileNode Id masks and attributes  *
 * * * * * * * * * * * * * * * * * **/

#define CLIPHSH 4294967295              // &mask to clip a number and guarantee 32 bit length
#define FBSGPCLIP 255 // &mask to clip down a did for masking with a FiNode object


/** \verbatim
 * FileNode ID Template
 * 1 set leading bit and 62 empty trailing bits
 *
 * 0b100000000000000000000000000000000000000000000000000000000000000
 *  C||                FIIDTMPL - 62bits                           |
 *  */
#define FIIDTMPL 4611686018427387904

#define FIIDSHFT 62 // Num of trailing bits after the leading control bit in the fiid


/** \verbatim

 * File INo. &mask
 *
 * Gives the file index num.
 *
 * 0b111111111111111111111111111111111000000000000000000000000000000
 *  C||      FINOMASK - 32bits       ||        FiNode attr.        |  */
#define FINOMASK 9223372035781033984
#define FINOSHFT 30  // Num of trailing bits after FINOMASK

/**
 *\verbatim
 * File name-length &mask
 *
 * Gives string length of the file name.
 *
 *  0b100000000000000000000000000000000111111111000000000000000000000
 *   C||          fiino               ||   *   ||      add.attr.    |
 *
 *  * FNLENMSK - 9 bits
 *
 *  */
#define FNLENMSK 1071644672
#define FNLENSHFT 21 // Num of trailing bits after FNLENMSK


/** \verbatim
 * File format &mask
 *
 * Gives the fileformat, defined by fiForms.
 *
 *  0b100000000000000000000000000000000000000000111111110000000000000
 *   C||          fiino               ||add.atr||   *  ||  add.atr  |
 *
 *  * FFRMTMSK - 8 bits */
#define FFRMTMSK 2088960
#define FFRMTSHFT 13 // Num of trailing bits after FFRMTMSK


/** \verbatim
 * File resident directory &mask
 *
 * Gives dir chain ID of the files resident dir ID, respective of it resident base
 *
 *  0b100000000000000000000000000000000000000000000000001111111100000
 *   C||          fiino               ||   add.attr.   ||  *   ||   |
 *
 *  * FRDIRMSK - 8 bits */
#define FRDIRMSK 8160
#define FRDIRSHFT 5  // Num of trailing bits after FRDIRMSK


/**
 * \verbatim
 *
 * Base group &mask
 *
 * Give the ID for the dir chain group, 1=media, 0=docs, defaults to zero.
 *
 *  0b100000000000000000000000000000000000000000000000000000000010000
 *   C||          fiino               ||      add.attr.        |^*  |
 *
 *  * FDCHNGMSK - 1 bit */
#define FDCHNGMSK 16
#define FDCHNGSHFT 4 // Num of trailing bits after FDCHNGMSK




/* *
 *  OP flag masks
 * */

#define QUALIFIR 7        //Cmds tags that modifiy the interpretation of others
#define ARRAYOPS 120      //Cmds pertaining to the arr portion of a cmd sequence
#define TRAVLOPS 1920     //Cmds that deal with moving and searching along DirChains
#define FIOBJOPS 30720    //Cmds dealing with FileNodes and fileTables
#define DIRNDOPS 491520   //Cmds that operate in the scope of the directory nodes
#define SYSTMOPS 7864320  //Cmds for managing the system.

/* *
 * FiNode masking
 */

unsigned long clip_to_32(unsigned long num);
unsigned long expo_fino(unsigned long key, unsigned long long fiid);
unsigned long long msk_fino(unsigned long ino);
unsigned long long msk_finmlen(unsigned long fiid, unsigned int fnlen);
unsigned int expo_finmlen(unsigned long long fiid);
unsigned long long msk_format(unsigned long long fiid, unsigned int fform);
unsigned int expo_format(unsigned long long fiid);
unsigned long long msk_resdir(unsigned long long fiid, unsigned int dirid);
unsigned int expo_resdir(unsigned long long fiid);
unsigned long long msk_dirgrp(unsigned long long fiid);
unsigned int expo_dirgrp(unsigned long long fiid);

/* *
 *  DirNode masking
 * */

unsigned int expo_dirnmlen(unsigned long long did);
unsigned int msk_dirnmlen(unsigned long long did, unsigned int dirnmln);
unsigned int expo_dirbase(unsigned long long did);
unsigned int msk_dirbase(unsigned long long did, unsigned int base);
unsigned int expo_dirchnid(unsigned long long did);
unsigned int msk_dirchnid(unsigned long long did, unsigned int id);

/* *
 * BaseNode masking
 * */

unsigned int expo_basedir_cnt(unsigned long long did);
unsigned int msk_basedir_cnt(unsigned long long did, unsigned int cnt);


#endif //TAGFI_FIDI_MASKS_H
