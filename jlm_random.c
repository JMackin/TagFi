#include <sodium.h>
#include <time.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>

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

int real_hash_keylessly(unsigned char* in, unsigned char* out, size_t inlen) {

    if(crypto_generichash(out, crypto_generichash_BYTES, in, inlen, NULL, 0)  != 0){
        fprintf(stderr, "Bridge hash failed\n");
        return 1;
    } else
    {
        return 0;
    }

    //crypto_generichash_BYTES = 32u
}
void real_hash_keyfully(unsigned char** in, unsigned char** out, size_t inlen, const unsigned char* key, size_t klen) {

    crypto_generichash(*out, crypto_shorthash_KEYBYTES, *in, inlen, key, klen);

    //crypto_generichash_BYTES = 32u
}

void two_32_to_64(unsigned char* kout, unsigned long* ttula, unsigned long* ttulb) {

    *kout = (*ttula << 32 )^(*ttulb);
}

void mk_o64o32_short_hashkey(unsigned long long* kout, unsigned long long* ttula, unsigned long* ttulb){
    kout = (unsigned long long*) sodium_malloc(4);
    sodium_mlock(kout,4);
    *kout = (*ttula &576460752169205760) | *ttulb;
            // 32-bit mask -- 0b1111111111111111111111111111111;

}

void mk_little_hash_key(unsigned char* kout) {
    kout = (unsigned char*) sodium_malloc(crypto_shorthash_KEYBYTES);
    crypto_shorthash_keygen(kout);
}

int dump_little_hash_key(unsigned char* kout) {

    int badflg = 0;

    int hkeyout = openat(AT_FDCWD,"/home/ujlm/CLionProjects/TagFI/keys",O_DIRECTORY);
    int hkfi = 0;
    hkfi = openat(hkeyout,"littlehashkey.lhsk", O_RDWR | O_TRUNC | O_CREAT);
    FILE* hkfi_out = fdopen(hkfi,"w+");

    write(hkfi,kout,crypto_shorthash_KEYBYTES);
    fsync(hkfi);


    fclose(hkfi_out);
    close(hkfi);
    close(hkeyout);


    return badflg;
}

int little_idx_hkey_3264(unsigned char* tobehshed, unsigned char* outp, unsigned long long* numa, unsigned long* numb, unsigned int wlen) {

    unsigned char* hshky = (unsigned char*) sodium_malloc(crypto_shorthash_KEYBYTES);
    crypto_shorthash_keygen(hshky);
    dump_little_hash_key(hshky);

    int res = crypto_shorthash(outp, tobehshed, wlen, (unsigned char *) hshky);
    sodium_free(hshky);
    return res;
}


int little_hash_idx(unsigned char* in, unsigned long* out, unsigned long inlen, const unsigned char *k, int mkkey){
//Key must = 16 bytes


    unsigned char* buf = sodium_allocarray(8, sizeof(unsigned char));


    if (crypto_shorthash(buf, in, inlen, k) != 0) {
        return 1;
    }
    else{
        unsigned long long tonum = eightchartollong(buf,8);
        memcpy(out, &tonum, 8);
        sodium_free(buf);
    }
}

void process_idx(unsigned char* in, unsigned wlen, unsigned long* out, unsigned char* lhk) {
    if (lhk == NULL) {
        mk_little_hash_key(lhk);
        if (lhk != NULL){
            dump_little_hash_key(&lhk);
        }
    }
    little_hash_idx(in, out, wlen, lhk, 0);

}


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
