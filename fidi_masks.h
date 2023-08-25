/*
 *  Bit masks and related functions for TagFi filesystem indexing.
 *
 *  fiid:
 *         0b 1 00000000000000000000000000000000  0000000000  00000000  000000  00  0
 *           C| |               a              |  |   b    |  |   c  |  | d  |  |e| |X
 *
 *           C: Control bit, always true. / 1 bit.
 *           a: (Fhshno^Inum) - Random number xor'd with file INum. / 32 bits.
 *           b: Name length - String-length of the filename. / 9 bits.
 *           c: Format - File format code defined by fiforms.h / 8 bits.
 *           d: Dir. Id - Id number of the resident directory, defined by sequence of addition to Dir_Chains. / 6 bits.
 *           e: Dir. groupbase - The Dir_chains arm to which the resident directory belongs. (01=media, 10=docs) / 2 bits.
 *           X: Unallocated/Misc switch. / 1 bit.
 */
//
// Created by ujlm on 8/24/23.
//

#ifndef TAGFI_FIDI_MASKS_H
#define TAGFI_FIDI_MASKS_H
#define TYPFIL 8        // Regular file flag
#define TYPDIR 4        // Directory flag

/* Directory ID masks and attributes*/

#define HTMASK 8191                // Hash table mask -> 13 bits
#define HTSIZE 8192                // Max item count
#define DCHNSSHIFT 6               // Number of bits in Dir ID no.
#define DCHNSMASK 63               // 0b00000000000000111111 - ID under group-base - abs:63
#define DBASEMASK 192              // 0b00000000000011000000 - Base ID: 01=media, 10=docs - abs:2
#define DUNUSED 1792               // 0b00000000011100000000 - Unused bits - abs:7
#define DNAMEMASK 1046528          // 0b11111111100000000000 - Str len of directory name/path - abs:511
#define MEDABASEM 64               // 0b00000000000001000000 - ID for MEDIA base derived from: (did & DBASEMASK)
#define DOCSBASEM 128              // 0b00000000000010000000 - ID for DOCS base derived from: (did & DBASEMASK)
#define DMASKSHFT 20               // Total mask bits
#define DNAMESHFT 11               // Trailing bits after dir str-len flag:  nlen = (DNAMEMASK & did) >> DNAMESHFT
#define DBASESHFT 6                // Trailing bits after group-base flag: baseID = (DBASEMASK & did) >> DBASESHFT
#define DGCNTMASK 4032             // 0b111111000000 - Total num. of dirs under group-base. For use on MEDA/DOCS nodes.
#define DGCNTSHFT 6                // Number of bits in base ID no.
#define DROOTDID 576460752303423489 // Root node did, empty save for 1 set leading bit and 1 trailing - 60 bits
#define DGRPTMPL 576460752303423488 // 1 leading bit and 59 empty bits, template for base-group did


/* FileMap Id masks and attributes */

#define FMIDTMPL 576460752303423488     /* FileMap ID empty template
 * 1 set leading bit and 59 empty trailing bits
 *
 * 0b100000000000000000000000000000000000000000000000000000000000
 *  C||                FMIDTMPL - 59bits                        |  */

#define FMIDSHFT 59                     // Num trailing bits after the leading control bit in the fiid

#define CLIPHSH 4294967295              // &mask to clip the random number and guarantee 32 bit length


#define FINOMASK 1152921504472629248    /* File XOR'd INo. &mask
 * Gives the number produced by XORing a random u_long and the file index num.
 *
 * 0b111111111111111111111111111111111000000000000000000000000000
 *  C||      FINOMASK - 32bits       ||       FiMap attr.       |  */

#define FINOSHFT 27  // Num of trailing bits after FINOMASK


#define FNLENMSK 133955584              /* File name length &mask
 * Gives string length of the file name.
 *
 * 0b100000000000000000000000000000000111111111000000000000000000
 *  C||         fhshno               ||   *   ||    add.attr.   |
 *
 *  * FNLENMSK - 9 bits */

#define FNLENSHFT 18 // Num of trailing bits after FNLENMSK


#define FFRMTMSK 130560                   /* File format &mask
 * Gives the fileformat, defined by fiForms.
 *
 *  0b10000000000000000000000000000000000000000011111111000000000
 *   C||         fhshno               ||add.atr||   *  ||add.atr|
 *
 *  * FFRMTMSK - 8 bits */

#define FFRMTSHFT 9 // Num of trailing bits after FFRMTMSK


#define FRDIRMSK 63                     /* File resident directory &mask
 * Gives dir chain ID of the files resident ID
 *
 *  0b10000000000000000000000000000000000000000000000000111111000
 *   C||         fhshno               ||   add.attr.   || *  || |
 *
 *  * FRDIRMSK - 6 bits */

#define FRDIRSHFT 7  // Num of trailing bits after FRDIRMSK


#define FDCHNGMSK 7                      /* Fancy ETC. &mask
 * Give the ID for the dir chain group, 01=media, 10=docs
 *
 *  0b10000000000000000000000000000000000000000000000000000000110
 *   C||         fhshno               ||     add.attr.        ||*
 *
 *  * FDCHNGMSK - 2 bits */

#define FDCHNGSHFT 1 // Num of trailing bits after FDCHNGMSK



unsigned long clip_to_32(unsigned long num);
unsigned long expo_fino(unsigned long key, unsigned long long fiid);
unsigned long long msk_fino(unsigned long rndm_no, unsigned long ino);
unsigned long expo_fino(unsigned long key, unsigned long long fiid);
unsigned long long msk_finmlen(unsigned long fiid, unsigned int fnlen);
unsigned int expo_finmlen(unsigned long long fiid);
unsigned long long msk_format(unsigned long long fiid, unsigned int fform);
unsigned int expo_format(unsigned long long fiid);
unsigned long long msk_redir(unsigned long long fiid, unsigned int dirid);
unsigned int expo_redir(unsigned long long fiid);
unsigned long long msk_dirgrp(unsigned long long fiid);
unsigned int expo_dirgrp(unsigned long long fiid, unsigned int digrp);
;
;
;
;
;


#endif //TAGFI_FIDI_MASKS_H
