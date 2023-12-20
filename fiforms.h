

#ifndef TAGFI_FIFORMS_H
#define TAGFI_FIFORMS_H

#define FORMCOUNT  58

#define EXTMAXLEN  7


typedef enum FiFormId {

		png = 1,
		md = 2,
		jpg = 3,
		svg = 4,
		sh = 5,
		html = 6,
		js = 7,
		ts = 8,
		txt = 9,
		css = 10,
		py = 11,
		sample = 12,
		gz = 13,
		json = 14,
		pub = 15,
		gpg = 16,
		webp = 17,
		el = 18,
		yml = 19,
		ino = 20,
		yaml = 21,
		toml = 22,
		pdf = 23,
		conf = 24,
		asc = 25,
		tar = 26,
		sls = 27,
		gif = 29,
		csv = 30,
		go = 31,
		master = 32,
		rb = 33,
		setup = 34,
		jpeg = 35,
		zip = 36,
		ini = 37,
		db = 38,
		save = 39,
		pem = 40,
		config = 41,
		pack = 42,
		idx = 43,
		mod = 44,
		sum = 45,
		pp = 46,
		README = 47,
		bak = 48,
		eot = 49,
		woff2 = 50,
		woff = 51,
		ttf = 52,
		log = 53,
		fzpz = 54,
		cpp = 55,
		sig = 56,
		list = 57,
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