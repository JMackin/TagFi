

#ifndef TAGFI_FIFORMS_H
#define TAGFI_FIFORMS_H

#define FORMCOUNT  12

#define EXTMAXLEN  7


typedef enum FiFormId {



		py = 1,
		pyc = 2,
		txt = 3,
		pem = 4,
		exe = 5,
		xml = 6,
		RECORD = 7,
		WHEEL = 8,
		nu = 9,
		pth = 10,
		tmpl = 11,
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