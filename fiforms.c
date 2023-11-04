//
// Created by ujlm on 8/25/23.
//

//
// Created by ujlm on 8/25/23.
//

#include <malloc.h>
#include <string.h>
#include "fiforms.h"
#include <string.h>

unsigned char form_exts[FORMCOUNT][EXTMAXLEN] = {

		"py",
		"pyc",
		"png",
		"md",
		"java",
		"mo",
		"po",
		"js",
		"txt",
		"xz",
		"qml",
		"html",
		"xml",
		"json",
		"go",
		"so",
		"cpp",
		"sh",
		"pyi",
		"cs",
		"jpg",
		"php",
		"kt",
		"rst",
		"svg",
		"c",
		"o",
		"csproj",
		"qm",
		"yaml",
		"sln",
		"rs",
		"mat",
		"class",
		"rb",
		"ts",
		"exe",
		"RECORD",
		"WHEEL",
		"css",
		"sample",
		"Plo",
		"lo",
		"swift",
		"in",
		"typed",
		"yml",
		"qmldir",
		"sav",
		"zst",
		"log",
		"jar",
		"mdl",
		"csv",
		"toml",
		"f90",
		"config",
		"exp",
		"pxd",
		"trs",
		"res",
		"Po",
		"jsp",
		"dat",
		"npy",
		"gz",
		"pak",
		"kts",
		"prefs",
		"pyx",
		"npz",
		"wav",
		"pdf",
		"gpg",
		"props",
		"HEAD",
		"abap",
		"qrc",
		"f",
		"pub",
		"arff",
		"bat",
		"lock",
		"bin",
		"tmpl",
		"zip",
		"mod",
		"ui",
		"bz2",
		"pth",
		"sum",
		"conf",
		"pem",
		"gif",
		"pkl",
		"docx",
		"cmake",
		"cfg",
		"sql",
		"asc",
		"gradle",
		"main",
		"ps1",
		"nu",
		"la",
		"mm",
		"url",
		"ino",
		"m4",
		"tsx",
		"iml",
		"meta",
		"ini",
		"a",
		"mjs",
		"fish",
		"pytpl",
		"master",
		"python",
		"csh",
		"pip",
		"pip3",
		"cor",
		"ico",
		"svgz",
		"hpp",
		"tar",
		"jinja2",
		"lst",
		"hash",
		"am",
		"vec",
		"tsv",
		"model",
		"state",
		"lnk",
		"cshtml",
		"launch",
		"wheel",
		"wheel3",
		"pack",
		"idx",
		"README",
		"woff",
		"drawio",
		"S",
		"save",
		"sls",
		"webp",
		"cmd",
		"setup",
		"mvnw",
		"tld",
		"tag",
		"flask",
		"vocab",
		"nc",
		"jpeg",
		"env",
		"mp3",
		"xhtml",
		"out",
		"MF",
		"war",
		"policy",
		"dotenv",
		"tst",
		"db",
		"key",
		"xsd",
		"tree",
		"spec",
		"APACHE",
		"BSD",
		"name",
		"dKill",
		"dStart",
		"xx",
		"dlogs",
		"pp",
		"bak",
		"eot",
		"woff2",
		"ttf",
		"fzpz",
		"sig",
		"list",
		"NOTICE",
		"jspf",
		"jspx",
		"xsl",
		"ser",
		"xmi",
		"http",
		"kasa",
		"frag",
		"PSF",
		"kml",
		"MIT",
		"pclt",
		"filter",
		"xbel",
		"pot",
		"base",
		"dev",
		"ninja",
		"pc",
		"auth7",
		"auth6",
		"box8",
		"auth2",
		"core5",
		"box7",
		"kdf",
		"core6",
		"kx",
		"core4",
		"hash3",
		"auth5",
		"keygen",
		"stream",
		"core3",
		"auth",
		"box",
		"core1",
		"auth3",
		"sign",
		"box2",
		"core2",
		"codecs",
		"misuse",
		"obj",
		"f2py3",
		"f2py",
		"mallet",
		"low",
		"dict",
		"uci",
		"blei",
		"ani",
		"fits",
		"None",
		"lua",
		"NONE"

};



//enum FiFormId determ_form(unsigned char* fext, int extlen){
//
//    if (extlen < EXTMAXLEN) {
//        unsigned char buf[extlen];
//
//        for (int i = 0; i < extlen; i++){
//            buf[i] = *fext+i;
//        }
//    }
//
//}


unsigned int grab_ffid(unsigned char* fname, unsigned int nlen) {

    enum FiFormId res = NONE;

    unsigned char* buf = (unsigned char*) calloc((size_t) nlen, sizeof(unsigned char));
    unsigned char* ext_buf;
    memcpy(buf, fname, (nlen*sizeof(unsigned char)));

    int n = 0;
    int i;
    int dotpos = 0;
    int cmpres = 0;
    int dotfound = 0;


    // NOTE: Will ned to filter file names with > 1 '.' for this to work, and for titles with dots > half-len.
    while (n < nlen ){

        if (buf[n] == 46) {
            dotpos = n;
        }
        n++;
    }

    if (dotpos == nlen || dotpos == 0){
        free(buf);
        return NONE;
    }
    dotpos++;

    unsigned int extlen = nlen - dotpos;
    ext_buf = (unsigned char*) calloc(extlen,sizeof(unsigned char));
    memcpy(ext_buf, buf+dotpos, sizeof(unsigned char)*(extlen));

    int score;
    for (i = 0; i < FORMCOUNT; i++) {
        score=0;
        for (n = 0; n < extlen; n++){
            if (ext_buf[n] == form_exts[i][n]) {
                score++;
            }
            if (score == extlen) {

                res = i;
                free(buf);
                free(ext_buf);
                return ++res;
            }

            if (ext_buf[n] != form_exts[i][n]) {
                res = NONE;
            }
        }

//        if (memcmp((unsigned char*) ext_buf, (unsigned char*)form_exts[i], (nlen-dotpos)*sizeof(unsigned char)) != 0){
//            res = i;
//
//        }

    }

    free(buf);
    free(ext_buf);

    return res;

}