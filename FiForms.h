

#ifndef TAGFI_FIFORMS_H
#define TAGFI_FIFORMS_H

#define FORMCOUNT  3

#define EXTMAXLEN  7


typedef enum FiFormId {
		png = 1,
		webp = 2,
		NONE = 0
} fiFormId;


typedef struct FiForms {
    fiFormId id;
    int cnt;
    int fextlen;
} FiForms;

unsigned int grab_ffid(unsigned char*, unsigned int extlen);
FiForms get_formext(int);



#endif //TAGFI_FIFORMS_H