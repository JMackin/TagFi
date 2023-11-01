#include <sodium.h>
#include <time.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>

#define DNKEYPATH "/home/ujlm/CLionProjects/TagFI/keys"


unsigned long long eightchartollong(const unsigned char* in, int wlen) {
    int i;
    int z = 0;

    if (wlen < 1 || wlen > 8) {
        wlen = 8;
    }

    for (i = 0; i< wlen; i++) {
        z = z ^ (((unsigned char) in[i]) << (7*i));
    }

    return z;
}

void rando_sf(unsigned long* salt_ptr) {


    const unsigned long clck = (clock() << 16) + (clock () << 2) ;

    fflush(stdout);

    *salt_ptr = (randombytes_random() << 1) << 16;

    *salt_ptr = (*salt_ptr ^ clck);

    randombytes_close();
    fflush(stdout);

}

void genrandsalt(unsigned long long* expanded_salt) {


    unsigned long* salt_ptr = (unsigned long*) sodium_malloc(sizeof(unsigned long));

    unsigned long* salt_ptrb = (unsigned long*) sodium_malloc(sizeof(unsigned long));


    rando_sf(salt_ptr);
    rando_sf(salt_ptrb);

    //unsigned long* two_ints[2] = {salt_ptr, salt_ptrb};

    *expanded_salt = (*salt_ptrb)<<16 ^ (*salt_ptr);

    sodium_free(salt_ptr);
    sodium_free(salt_ptrb);

}

//void getbigsalt(unsigned long long* big_salt)
//{
//
//
//    rando_sf(salt_ptr);
//    rando_sf(salt_ptrb);
//
//    unsigned long* two_ints[2] = {salt_ptr, salt_ptrb};
//
//    *big_salt = (*two_ints[0])<<16 ^ (*two_ints[1]);
//
//    sodium_free(salt_ptr);
//    sodium_free(salt_ptrb);
//}
__attribute__((unused)) int real_hash_keylessly(unsigned char* in, unsigned char* out, size_t inlen) {

    if(crypto_generichash(out, crypto_generichash_BYTES, in, inlen, NULL, 0)  != 0){
        fprintf(stderr, "Bridge hash failed\n");
        return 1;
    } else
    {
        return 0;
    }

    //crypto_generichash_BYTES = 32u
}
void real_hash_keyfully(unsigned char** in, unsigned char** out, size_t inlen, const unsigned char** key, size_t klen) {

    crypto_generichash(*out, crypto_generichash_BYTES, *in, inlen, *key, klen);

    //crypto_generichash_BYTES = 32u
}

void dn_hdid_str(unsigned char** dn_hash, char** strout) {
    sodium_bin2hex(*strout, (crypto_generichash_BYTES*2+1), *dn_hash, crypto_generichash_BYTES);
}


void mk_little_hash_key(unsigned char** kout) {
    crypto_shorthash_keygen(*kout);
    printf(">");
}

int dump_little_hash_key(unsigned char* kout, unsigned char* name, unsigned int nlen) {

    int badflg = 0;
    char* kname = malloc((nlen+6)*sizeof(char));

    memcpy(kname,name,nlen);
    memcpy(kname+nlen,".lhsk",6);

    int hkeyout = openat(AT_FDCWD, DNKEYPATH, O_DIRECTORY, O_RDWR);

    if(hkeyout < 0){
        fprintf(stderr,"<<%s>>\n",name);

        perror("dump_little_hash_key/hkeyout: ");
    }

    int hkfi = openat(hkeyout, kname, O_CREAT|O_RDWR, S_IRWXU);

    if(hkfi < 0){

        fprintf(stderr,"<<%s>>\n",name);
        perror("dump_little_hash_key/hkfi: ");
    }

    //FILE* hkfi_out = fdopen(hkfi,"w+");

    write(hkfi,kout,crypto_shorthash_KEYBYTES);

    //fflush(hkfi_out);
    fsync(hkfi);

    if (hkeyout < 0 || hkfi < 0)
    {
        badflg = 1;
    }

    //fclose(hkfi_out);
    free(kname);
    close(hkfi);
    close(hkeyout);

    return badflg;
}
//void recv_little_hash_key(char* pathin, unsigned int nlen, unsigned char* bytesout)
void recv_little_hash_key(int dnkeyfd, unsigned char* dirname, unsigned int knmln, unsigned char* bytesout) {


    char* kname = (char*) calloc(knmln+6, sizeof(char));
    memcpy(kname,dirname,knmln);
    memcpy(kname+knmln,".lhsk",6);


    int kfi = openat(dnkeyfd, kname, O_RDONLY);

    //if (kfi < '.')
     if (kfi < 0)
    {
        perror("recv_little_hash_key/kfi: ");
        fprintf(stderr,"<<%s>>\n",dirname);
        return;
    }
    FILE* lhkstrm = fdopen(kfi,"r");

    for (int i =0; i < crypto_shorthash_KEYBYTES;i++){
        *(bytesout+i) = fgetc(lhkstrm);
    }
    close(kfi);
    free(kname);
}


unsigned long long little_hsh_llidx(unsigned char* hkey, unsigned char* tobehshed, unsigned int tbh_len, unsigned long long xno) {

    unsigned char* outp = (unsigned char*) sodium_malloc(sizeof (unsigned long));

    if (crypto_shorthash(outp, tobehshed, tbh_len, hkey) != 0){
        fprintf(stderr, "Something went wrong hashing for an index.\n");
        sodium_free(outp);
        return 1;
    }

    unsigned long outidx = eightchartollong(outp,crypto_shorthash_KEYBYTES) ^ xno;
    sodium_free(outp);
    return outidx;
}

//unsigned long long noky_lhsh_lidx(unsigned char* tobehshed, unsigned int wlen, unsigned long xno) {
//
//    unsigned char* outp = (unsigned char*) sodium_malloc(sizeof (unsigned long));
//
//    if (crypto_shorthash(outp, tobehshed, wlen, NULL) != 0){
//        fprintf(stderr, "Something went wrong hashing for an index.\n");
//        sodium_free(outp);
//        return 1;
//    }
//
//    unsigned long outidx = eightchartollong(outp,crypto_shorthash_KEYBYTES) ^ xno;
//    sodium_free(outp);
//    return outidx;
//}

void get_many_big_salts(unsigned long long** bigsaltls, int n){
    do{
        bigsaltls[n] = (unsigned long long*) sodium_malloc(sizeof(unsigned long long));
        genrandsalt(bigsaltls[n]);
    } while(n--);
}

void get_many_little_salts(unsigned long** saltls, int n)
{
    do{
        saltls[n] = (unsigned long*) sodium_malloc(sizeof(unsigned long));
        rando_sf(saltls[n]);

    } while(n--);
}
