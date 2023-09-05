

#ifndef TAGFI_FIFORMS_H
#define TAGFI_FIFORMS_H

#define FORMCOUNT  60

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
		yml = 17,
		ino = 18,
		yaml = 19,
		toml = 20,
		pdf = 22,
		conf = 23,
		asc = 24,
		sls = 25,
		webp = 26,
		tar = 27,
		HEAD = 28,
		gif = 29,
		csv = 30,
		go = 31,
		master = 32,
		rb = 33,
		setup = 34,
		jpeg = 35,
		h = 36,
		ini = 37,
		db = 38,
		zip = 39,
		save = 40,
		pem = 41,
		config = 43,
		pack = 44,
		idx = 45,
		mod = 46,
		sum = 47,
		pp = 48,
		README = 49,
		bak = 50,
		eot = 51,
		woff2 = 52,
		woff = 53,
		ttf = 54,
		log = 55,
		fzpz = 56,
		cpp = 57,
		sig = 58,
		list = 59,
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