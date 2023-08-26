

#ifndef TAGFI_FIFORMS_H
#define TAGFI_FIFORMS_H

#define FORMCOUNT  28

#define EXTMAXLEN  7


typedef enum FiFormId {



		py = 1,
		txt = 2,
		pyc = 3,
		json = 4,
		c = 5,
		h = 6,
		pem = 7,
		exe = 8,
		sample = 9,
		xml = 10,
		cmake = 11,
		ninja = 12,
		o = 13,
		RECORD = 14,
		WHEEL = 15,
		gcno = 16,
		gcov = 17,
		main = 18,
		yaml = 19,
		bin = 20,
		out = 21,
		log = 22,
		nu = 23,
		pth = 24,
		tmpl = 25,
		iml = 26,
		HEAD = 27,
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