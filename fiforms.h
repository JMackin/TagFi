/*
 *
 */

#ifndef TAGFI_FIFORMS_H
#define TAGFI_FIFORMS_H

#define FORMCOUNT  11

#define EXTMAXLEN  7

typedef char* FiFormExtArr[FORMCOUNT];

typedef enum FiFormId {

		py = 1,
		pyc = 2,
		txt = 3,
		pem = 4,
		exe = 5,
		RECORD = 6,
		WHEEL = 7,
		nu = 8,
		pth = 9,
		tmpl = 10,
		NONETYPE = 0


} fiFormId;

FiFormExtArr fiFormExtArr = {

		"py",
		"pyc",
		"txt",
		"pem",
		"exe",
		"RECORD",
		"WHEEL",
		"nu",
		"pth",
		"tmpl",
		"NONETYPE"

};

typedef FiFormExtArr* fiFormExt;

typedef struct FiForms {
    fiFormId id;
    int cnt;
    fiFormExt ext;
    int fextlen;
} FiForms;

FiForms get_formid(unsigned char*, int extlen);
FiForms get_formext(int);

#endif //TAGFI_FIFORMS_H