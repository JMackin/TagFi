#include <sodium.h>
#include <time.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include "lattice_works.h"
#include "FiDiMasks.h"

#define LTTCKEY_EXT ".lttckey"
#define LTTCKEY_EXT_LEN 8
#define LTTC_MK_NAME "Lattice_MK.lttckey"
#define LTTC_SK_CNTXT_FINAME "Lattice_SK_context"
#define LTTC_SK_CNTXT_IDS_FINAME "Lattice_SK_ctxtid"
#define LTTC_SIGN_KEY_NM "Lattice_Signing_key.lttckey"
#define LTTC_MK_NAME_LEN 18
#define LTTC_SK_CNTXT_BUF_LEN 4
#define LTTC_SK_CNTXT_LEN crypto_kdf_CONTEXTBYTES
#define LTTC_SK_CNTXT_HEXSTR_LEN 10 // 8-byte context string + '\0' + '\n'
#define LTTC_SIGN_KEY_NM_LEN 27


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

//    randombytes_close();
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

unsigned int bytes_tostr(char** strout, const unsigned char* bytes, size_t b_len) {
    if (b_len > 16){
        b_len = 16;
    }else if (b_len < 4){
        return 1;
    }
    const size_t hexmaxlen = b_len * 4;
    *strout = calloc(hexmaxlen+1,UCHAR_SZ);
    if (*strout == NULL){
        perror("Couldnt allocate mem for bytestostr");
        return 1;
    }

    sodium_bin2hex(*strout, hexmaxlen, bytes, b_len);
    return strnlen(*strout,hexmaxlen);
}

void mk_little_hash_key(unsigned char** kout) {
    crypto_shorthash_keygen(*kout);
}

int dump_little_hash_key(unsigned char* kout, unsigned char* name, unsigned int nlen) {

    char* kname = (char*) calloc((nlen+LTTCKEY_EXT_LEN),UCHAR_SZ);


    memcpy(kname,name,(nlen));
    memcpy(kname+nlen,LTTCKEY_EXT,LTTCKEY_EXT_LEN);

    int hkeyout = openat(AT_FDCWD, getenv("DNKEYPATH"), O_DIRECTORY);
    if(hkeyout < 0){
        fprintf(stderr,"<<%s>>\n",name);
        perror("dump_little_hash_key/hkeyout: ");
        free(kname);
        return 1;
    }

    int hkfi = openat(hkeyout, kname, O_CREAT|O_RDWR, S_IRWXU);
    if(hkfi < 0){
        fprintf(stderr,"<<%s>>\n",name);
        perror("dump_little_hash_key/hkfi: ");
        free(kname);
        close(hkeyout);
        return 1;
    }

    write(hkfi,kout,crypto_shorthash_KEYBYTES);

    fsync(hkfi);
    free(kname);
    close(hkfi);
    close(hkeyout);

    return 0;
}
//void recv_little_hash_key(char* pathin, unsigned int nlen, unsigned char* bytesout)
void recv_little_hash_key(int dnkeyfd, unsigned char* dirname, unsigned int knmln, unsigned char* bytesout) {

    char* kname = (char*) calloc((knmln+LTTCKEY_EXT_LEN+1),UCHAR_SZ);

    memcpy(kname,dirname,knmln);
    memcpy(kname+knmln,LTTCKEY_EXT,LTTCKEY_EXT_LEN);

    FILE* lhkstrm;

    int kfi = openat(dnkeyfd, kname, O_RDONLY);

    //if (kfi < '.')
     if (kfi < 0)
    {
        perror("recv_little_hash_key/kfi: ");
        fprintf(stderr,"<<%s>>\n",dirname);
        free(kname);
        return;
    }
    lhkstrm = fdopen(kfi,"r");

    for (int i =0; i < crypto_shorthash_KEYBYTES;i++){
        *(bytesout+i) = fgetc(lhkstrm);
    }
    fclose(lhkstrm);
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

unsigned long latt_hsh_idx(LttcHashKey lattkey, unsigned long fhshno, unsigned char* intbuf){


    unsigned long outidx=0;
    unsigned long outidx2=0;
    unsigned int clk = fhshno&3;
    unsigned int msk = (3817748711);
    unsigned char tbuf[8];
    memcpy(&outidx,&fhshno,8);


    memcpy(tbuf,&outidx,8);

    if (crypto_shorthash(intbuf, tbuf, 8, lattkey) != 0){
        fprintf(stderr, "Something went wrong hashing for an index.\n");
        return 1;
    }
    memcpy(&outidx2,intbuf,8);

    (outidx2 &= 16777215);

    return outidx2;
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
        bigsaltls[n] = (unsigned long long*) malloc(sizeof(unsigned long long));
        genrandsalt(bigsaltls[n]);
    } while(n--);
}

void get_many_little_salts(unsigned long** saltls, uint n)
{
    //*saltls = calloc(n,ULONG_SZ);
    do{
        *(saltls+n) = (unsigned long*) malloc(sizeof(unsigned long));
        rando_sf(*(saltls+n));

    } while(n--);
}

char* gen_socket_name(void* namebase){
    unsigned long* base = malloc(ULONG_SZ);
    unsigned char* base_str = malloc(ULONG_SZ);
    char* nameout = calloc(ARRBUF_LEN,UCHAR_SZ);
    if (namebase != NULL){
        base = (unsigned long *) namebase;
    }else
    {
        rando_sf(base);
    }
    memcpy(base_str,base,ULONG_SZ);
    free(base);
    sodium_bin2hex(nameout,ARRBUF_LEN-10,base_str, ULONG_SZ);
    free(base_str);
    return nameout;
}


void gen_new_latticeid(Lattice lattice, LatticeID_PTA latticeId, AuthTagMap tagMap){
    fflush(stdout);
    randombytes_stir();

    LattTagBase tagBase = sodium_allocarray(LATT_AUTH_MSGBASE_64_LEN, UCHAR_SZ);
    randombytes_buf(tagBase, LATT_AUTH_MSGBASE_64_LEN);
    sodium_bin2hex(*latticeId, LATT_AUTH_MSGBASE_64_LEN, tagBase, LATT_AUTH_MSGBASE_64_LEN);

    sodium_free(tagBase);
}

/* Generated AuthTag is placed in base_out.
 * Needs to be sodium_free'd
 *
 * pass in 0 to tagsize for 64-bit tag and 1 for 32-bit
 * */
void gen_rand_tag_base(ULattTagBase* base_out, uint tagsize){

    uint base_len = tagsize ? LATT_AUTH_MSGBASE_64_LEN : LATT_AUTH_MSGBASE_32_LEN;
    randombytes_stir();
    uchar_arr base_buf = sodium_malloc(base_len * UCHAR_SZ);
    (*base_out) = sodium_malloc((base_len) * UCHAR_SZ);

    randombytes_buf(base_buf,(base_len>>1)*UCHAR_SZ);
    sodium_bin2hex((char*)(*base_out), base_len, base_buf, (base_len>>1));
    sodium_free(base_buf);
}

/*
 * Generates a 'master key' from which the three 'master-sub-keys' are derived.
 *
 * It's generated with libsodiums's crypto_kdf_keygen function, and then dumped to a file defined by an env variable
 * with a .lttckey extension, and stored in a dedicated directory with minimal permissions (600).
 *
 * This key is intended to be high entropy and rotated less often than its descendents, and should be well isolated and
 * protected.
 *
 * The function aborts on error.
 *
 * https://doc.libsodium.org/key_derivation */
void gen_master_key(void){
    const size_t onebyte = sizeof(uint8_t);
    unsigned char* mkey = sodium_malloc(onebyte*crypto_kdf_KEYBYTES);

    int mkey_dir_fd = openat(AT_FDCWD,getenv("MASTERKEYDIR"),O_DIRECTORY);
    if (mkey_dir_fd == -1){perror("MKDir access failed.");sodium_free(mkey);abort();}

    int mkey_fd = openat(mkey_dir_fd,LTTC_MK_NAME,O_TRUNC|O_CREAT|O_WRONLY|O_CLOEXEC,S_IWUSR|S_IRUSR);
    if (mkey_fd == -1){perror("MKFi access failed.");close(mkey_dir_fd);
        sodium_free(mkey);abort();}

    crypto_kdf_keygen(mkey);

    if(write(mkey_fd,mkey,onebyte*crypto_kdf_KEYBYTES)==-1){
        close(mkey_dir_fd);close(mkey_fd);sodium_free(mkey);abort();
    }
    if(fsync(mkey_fd)==-1){
        perror("Fsync of MK failed.");
    }
    sodium_memzero(mkey,onebyte*crypto_kdf_KEYBYTES);
    close(mkey_fd);
    close(mkey_dir_fd);
    sodium_free(mkey);
}

void free_subkeys(LttcKeySum lts_key){
    const size_t onebyte = sizeof(uint8_t);
    const size_t LKey_sz = onebyte*crypto_kdf_KEYBYTES;

    sodium_memzero((*lts_key)[0],LKey_sz);
    sodium_free((*lts_key)[0]);

    sodium_memzero((*lts_key)[1],LKey_sz);
    sodium_free((*lts_key)[1]);

    sodium_memzero((*lts_key)[2],LKey_sz);
    sodium_free((*lts_key)[2]);

    sodium_free((lts_key));
}

/*
 * Generates three 32-byte "master-sub-keys" from which the functional key's used by Lattice are derived. The master-sub-keys
 * act as seeds for these keys and are not to be used in operation themselves. Each functional
 * key is intended to be re-generated periodically, and each fall under one of three categories
 * represented by the master-subkey from which it's derived.
 *
 * They are:
 *
 * - Encrypt: Keys used for the purpose of encryption and secret-keeping.
 *
 * - Signing: Keys used for the purpose of authentication, identification, and verification.
 *
 * - Hash: Keys used for assorted purposes such as indexing, data-storage,
 *         hashtable functions, random string generation, and so on.
 *         Note these are not used for password hashing. Such a need would
 *         be secured with an 'Encrypt' key.
 *
 *
 * Each master sub-key is derived from a single 'Master key' seed in conjunction with a 'context' and 'id',
 * which are determined at the time of derivation.
 *
 * The context is a randomly generated 8-byte string, originally intended
 * to be used as a descriptive piece of information "similar to a type" which may "mitigate accidental bugs by
 * separating domains", here they serve a purpose more akin to a unique identifier.
 *
 * The id is a long int that is incremented each time new keys are generated. If the subkey-context file is not found,
 * one is created and the id's begin at 1, 2, and 3. They are incremented from here by three at each derivation
 * to avoid overlap.
 *
 * The function uses the crypto_kdf API. See link below for technical details.
 *
 * The function aborts on error.
 *
 * https://doc.libsodium.org/key_derivation */
void derive_master_subkeys(void){


    int i;
    int sk_fd;
    int sk_cid_exists = 0;
    ulong mk_rin_bytes;
    ulong sk_ctxt_id_bytes_in;
    char sk_names[LTTC_MK_NAME_LEN+2] = {'L','a','t','t','i','c','e','_',0,0,0,'.','l','t','t','c','k','e','y','\0'};
    char sk_types[3][3] = {{'S','i','g'},{'E','n','c'},{'H','s','h'}};

    const size_t onebyte = sizeof(uint8_t);
    const size_t LKey_sz = onebyte*crypto_kdf_BYTES_MAX;

    ulong* sk_ids;
    uchar* sk_ids_buf;
    char newline[1] = {'\n'};

    uchar* mkey = (uchar*) sodium_malloc(onebyte*crypto_kdf_KEYBYTES);
    LttcKeySum lts_tri_sub_key = (LttcKeySum) sodium_malloc(3 * sizeof(LatticeKey));
    (*lts_tri_sub_key)[0]=(LatticeKey) sodium_malloc(LKey_sz);
    (*lts_tri_sub_key)[1]=(LatticeKey) sodium_malloc(LKey_sz);
    (*lts_tri_sub_key)[2]=(LatticeKey) sodium_malloc(LKey_sz);

    uchar* sk_contexts_buf = sodium_malloc((LTTC_SK_CNTXT_LEN)*onebyte);
    char* sk_contexts;


    int skey_dir_fd = openat(AT_FDCWD,getenv("LATTICE_MS_KEYS"),O_DIRECTORY);
    if (skey_dir_fd == -1){perror("LSKDir access failed.");
        sodium_free(sk_contexts_buf);
        free_subkeys(lts_tri_sub_key);
        sodium_memzero(mkey,LKey_sz);
        sodium_free(mkey);
        abort();}

    int sk_ctxt_fd = openat(skey_dir_fd,LTTC_SK_CNTXT_FINAME,O_TRUNC|O_CREAT|O_WRONLY|O_CLOEXEC,S_IWUSR|S_IRUSR);
    if (sk_ctxt_fd == -1){perror("LSK contexts file access failed.");
        close(skey_dir_fd);
        sodium_free(sk_contexts_buf);
        free_subkeys(lts_tri_sub_key);
        sodium_memzero(mkey,LKey_sz);
        sodium_free(mkey);
        abort();}

    if(access(getenv("LATTICE_SK_CTXID_FI"), F_OK) != -1){
        sk_cid_exists = 1;
    }
    int sk_ctxt_id_fd = openat(skey_dir_fd, LTTC_SK_CNTXT_IDS_FINAME, O_CREAT | O_RDWR | O_CLOEXEC, S_IWUSR | S_IRUSR);
    if (sk_ctxt_id_fd == -1){perror("LSK context IDs file access failed.");
        close(skey_dir_fd);
        close(sk_ctxt_fd);
        sodium_free(sk_contexts_buf);
        free_subkeys(lts_tri_sub_key);
        sodium_memzero(mkey,LKey_sz);
        sodium_free(mkey);
        abort();}


    int mkey_dir_fd = openat(AT_FDCWD,getenv("MASTERKEYDIR"),O_DIRECTORY);
    if (mkey_dir_fd == -1){perror("MKDir access failed.");
        free_subkeys(lts_tri_sub_key);
        close(sk_ctxt_fd);
        close(sk_ctxt_id_fd);
        sodium_free(sk_contexts_buf);
        sodium_memzero(mkey,LKey_sz);
        sodium_free(mkey);
        abort();}

    int mkey_fd = openat(mkey_dir_fd,LTTC_MK_NAME,O_RDONLY|O_CLOEXEC);
    if (mkey_fd == -1){perror("MKFi access failed.");
        close(mkey_dir_fd);
        close(sk_ctxt_fd);
        close(sk_ctxt_id_fd);
        free_subkeys(lts_tri_sub_key);
        sodium_free(sk_contexts_buf);
        sodium_memzero(mkey,LKey_sz);
        sodium_free(mkey);
        abort();}

    mk_rin_bytes = read(mkey_fd,mkey,LKey_sz);
    if(mk_rin_bytes == -1 || mk_rin_bytes == 0){
        perror("Failed reading LSK context IDs file");
        close(mkey_dir_fd);
        close(mkey_fd);
        close(skey_dir_fd);
        close(sk_ctxt_fd);
        close(sk_ctxt_id_fd);
        sodium_free(sk_contexts_buf);
        free_subkeys(lts_tri_sub_key);
        sodium_memzero(mkey,LKey_sz);
        sodium_free(mkey);
        abort();
    }
    fsync(mkey_fd);
    close(mkey_dir_fd);
    close(mkey_fd);

    sk_contexts = (char*) sodium_malloc((LTTC_SK_CNTXT_HEXSTR_LEN)*onebyte);
    sodium_munlock(sk_contexts,LTTC_SK_CNTXT_HEXSTR_LEN);
    sodium_memzero(sk_contexts,LTTC_SK_CNTXT_HEXSTR_LEN);
    *(sk_contexts+LTTC_SK_CNTXT_HEXSTR_LEN-1) = '\n';

    sk_ids = (ulong*) malloc(ULONG_SZ);
    sk_ids_buf = (uchar*) malloc((ULONG_SZ+1)*UCHAR_SZ);

    for (i = 0; i < 3; i++){
        sk_names[8] = sk_types[i][0];
        sk_names[9] = sk_types[i][1];
        sk_names[10] = sk_types[i][2];

        randombytes_buf(sk_contexts_buf,LTTC_SK_CNTXT_BUF_LEN);

        sodium_bin2hex(sk_contexts,LTTC_SK_CNTXT_HEXSTR_LEN,sk_contexts_buf,LTTC_SK_CNTXT_BUF_LEN);

        if(write(sk_ctxt_fd,sk_contexts,LTTC_SK_CNTXT_HEXSTR_LEN)==-1){
            perror("Failed writing subkey context.");
            fprintf(stderr,"Failed on: %s\n",sk_names);
            close(mkey_fd);
            close(mkey_dir_fd);
            close(skey_dir_fd);
            close(sk_ctxt_fd);
            sodium_memzero(sk_contexts,LTTC_SK_CNTXT_HEXSTR_LEN);
            sodium_free(sk_contexts);
            sodium_free(sk_contexts_buf);
            free_subkeys(lts_tri_sub_key);
            sodium_memzero(mkey,LKey_sz);
            sodium_free(mkey);
            abort();
        }
        fsync(sk_ctxt_fd);

        if (sk_cid_exists) {
            lseek(sk_ctxt_id_fd, (ULONG_SZ + 1) * i, SEEK_SET);
            sk_ctxt_id_bytes_in = read(sk_ctxt_id_fd, sk_ids_buf, ULONG_SZ + 1);
            if (sk_ctxt_id_bytes_in == -1) {
                perror("Failed reading LSK context IDs file");
                close(skey_dir_fd);
                close(sk_ctxt_fd);
                close(sk_ctxt_id_fd);
                sodium_free(sk_contexts_buf);
                free_subkeys(lts_tri_sub_key);
                sodium_memzero(mkey, LKey_sz);
                sodium_free(mkey);
                abort();
            }
            memcpy(sk_ids,sk_ids_buf,ULONG_SZ);
            *sk_ids += 3;


        }else{
            *sk_ids = i + 1;
        }

        printf(">%ld\n",*sk_ids);

        lseek(sk_ctxt_id_fd,(ULONG_SZ+1)*i,SEEK_SET);
        memcpy(sk_ids_buf,sk_ids,ULONG_SZ);
        if(write(sk_ctxt_id_fd,sk_ids_buf,ULONG_SZ)==-1){
            perror("Failed writing to sk IDs file.");
            fprintf(stderr,"sk ids:\n\t [ %ld ]\n",*sk_ids);
        }
        if(write(sk_ctxt_id_fd,newline,UCHAR_SZ)==-1){
            perror("Failed writing to sk IDs file.");
            fprintf(stderr,"sk ids:\n\t [ %ld ]\n",*sk_ids);
        }

        fsync(sk_ctxt_id_fd);

        crypto_kdf_derive_from_key(*((*lts_tri_sub_key)+i),
                                   crypto_kdf_BYTES_MAX,
                                   *(sk_ids),
                                   sk_contexts,
                                   mkey);

        sk_fd = openat(skey_dir_fd,sk_names,O_TRUNC|O_CREAT|O_WRONLY|O_CLOEXEC,S_IWUSR|S_IRUSR);
        if(sk_fd == -1){
            perror("Failed opening subkey files.");
            fprintf(stderr,"Failed on: %s\n",sk_names);
            close(mkey_fd);
            close(mkey_dir_fd);
            close(skey_dir_fd);
            close(sk_fd);
            close(sk_ctxt_fd);
            sodium_memzero(sk_contexts,LTTC_SK_CNTXT_HEXSTR_LEN);
            sodium_free(sk_contexts);
            sodium_free(sk_contexts_buf);
            free_subkeys(lts_tri_sub_key);
            sodium_memzero(mkey,LKey_sz);
            sodium_free(mkey);
            abort();
        }

        if(write(sk_fd,*((*lts_tri_sub_key)+i),crypto_kdf_BYTES_MAX)==-1){
            perror("Failed writing subkey context.");
            fprintf(stderr,"Failed on: %s\n",sk_names);
            close(mkey_fd);
            close(mkey_dir_fd);
            close(skey_dir_fd);
            close(sk_fd);
            close(sk_ctxt_fd);
            sodium_memzero(sk_contexts,LTTC_SK_CNTXT_HEXSTR_LEN);
            sodium_free(sk_contexts);
            sodium_free(sk_contexts_buf);
            free_subkeys(lts_tri_sub_key);
            sodium_memzero(mkey,LKey_sz);
            sodium_free(mkey);
            abort();
        }
        fsync(sk_fd);
        close(sk_fd);

        sodium_memzero(sk_contexts,LTTC_SK_CNTXT_HEXSTR_LEN-2);
        sodium_memzero(sk_contexts_buf,LTTC_SK_CNTXT_LEN);
        randombytes_stir();
    }

    close(skey_dir_fd);
    close(sk_ctxt_fd);
    close(sk_ctxt_id_fd);
    sodium_free(sk_contexts);
    sodium_memzero(mkey,crypto_kdf_KEYBYTES);
    sodium_free(mkey);
    sodium_free(sk_contexts_buf);
    free_subkeys(lts_tri_sub_key);
    free(sk_ids);
    free(sk_ids_buf);
}

/*
 * Stand-alone function to generate 'Lattice Signing Key' key-pair.
 *
 * A 32-byte public and 64-byte private key are derived from the 'Sign-master-subkey' and
 * dumped to a location defined by env variables with '.lttckey' extensions.
 *
 * Aborts on error.
 *
 * https://doc.libsodium.org/public-key_cryptography/public-key_signatures*/
void gen_signing_key(void){

    int smkey_dir_fd = openat(AT_FDCWD,getenv("LATTICE_MS_KEYS"),O_DIRECTORY);
    int latt_keys_dir;

    unsigned char* pub_sk = (unsigned char*) sodium_malloc(crypto_sign_PUBLICKEYBYTES);
    unsigned char* sec_sk = (unsigned char*) sodium_malloc(crypto_sign_SECRETKEYBYTES);
    unsigned char* ms_sk = (unsigned char*) sodium_malloc(crypto_sign_PUBLICKEYBYTES);


    if(smkey_dir_fd == -1){
        perror("Failed opening lattice key dir.");
        abort();
    }
    int smkey_fd = openat(smkey_dir_fd,getenv("LATTICE_M_SIGSUB_KEY"),O_RDONLY|O_CLOEXEC);
    if(smkey_fd==-1){
        perror("Failed retrieving lattice MS-signing key.");
        close(smkey_dir_fd);
        sodium_free(ms_sk);
        sodium_free(pub_sk);
        sodium_free(sec_sk);
        abort();
    }

    if(read(smkey_fd,ms_sk,crypto_sign_SEEDBYTES)==-1){
        perror("Failed retrieving lattice MS-signing key.");
        sodium_memzero(ms_sk,crypto_sign_SEEDBYTES);
        sodium_free(ms_sk);
        close(smkey_dir_fd);
        close(smkey_fd);
        sodium_free(pub_sk);
        sodium_free(sec_sk);
        abort();
    }
    close(smkey_fd);
    close(smkey_dir_fd);

    crypto_sign_seed_keypair(pub_sk,sec_sk,ms_sk);

    sodium_memzero(ms_sk,crypto_sign_SEEDBYTES);
    sodium_free(ms_sk);

    latt_keys_dir = openat(AT_FDCWD,getenv("LATTICKEYDIR"),O_DIRECTORY);


    int secsigkey_fd = openat(latt_keys_dir,getenv("LATTICE_SEC_SIGNING_KEY"),O_TRUNC|O_CREAT|O_WRONLY|O_CLOEXEC,S_IWUSR|S_IRUSR);
    if(secsigkey_fd==-1){
        perror("Failed opening lattice sec signing key.");
        sodium_memzero(sec_sk,crypto_sign_SECRETKEYBYTES);
        sodium_free(sec_sk);
        close(smkey_dir_fd);
        sodium_memzero(sec_sk,crypto_sign_PUBLICKEYBYTES);
        sodium_free(pub_sk);
        abort();
    }

    if (write(secsigkey_fd,sec_sk,crypto_sign_SECRETKEYBYTES)==-1){
        perror("Failed writing out sec signing key.");
        sodium_memzero(sec_sk,crypto_sign_SECRETKEYBYTES);
        sodium_free(sec_sk);
        close(latt_keys_dir);
        close(secsigkey_fd);
        sodium_memzero(sec_sk,crypto_sign_PUBLICKEYBYTES);
        sodium_free(pub_sk);
        abort();
    }
    fsync(secsigkey_fd);
    close(secsigkey_fd);

    sodium_memzero(sec_sk,crypto_sign_SECRETKEYBYTES);
    sodium_free(sec_sk);

    int pubsigkey_fd = openat(latt_keys_dir,getenv("LATTICE_PUB_SIGNING_KEY"),O_TRUNC|O_CREAT|O_WRONLY|O_CLOEXEC,S_IWUSR|S_IRUSR);
    if(pubsigkey_fd==-1){
        perror("Failed opening lattice pub signing key.");
        sodium_memzero(pub_sk,crypto_sign_PUBLICKEYBYTES);
        sodium_free(pub_sk);
        close(latt_keys_dir);
        abort();
    }
    if (write(pubsigkey_fd,pub_sk,crypto_sign_PUBLICKEYBYTES) == -1){
        perror("Writing out pub-sign key failed.");
        sodium_memzero(pub_sk,crypto_sign_PUBLICKEYBYTES);
        sodium_free(pub_sk);
        close(pubsigkey_fd);
        abort();
    }
    fsync(pubsigkey_fd);
    close(pubsigkey_fd);

    sodium_memzero(pub_sk,crypto_sign_PUBLICKEYBYTES);
    sodium_free(pub_sk);

    close(latt_keys_dir);
}

/*
 * Stand-alone function to generate 'Lattice Encrypt Key' key-pair.
 *
 * A 32-byte public and private key are derived from the 'Encrypt-master-subkey' and
 * dumped to a location defined by env variables with '.lttckey' extensions.
 *
 * Aborts on error.
 *
 * https://doc.libsodium.org/public-key_cryptography/authenticated_encryption */
void gen_crypt_key(void){
    int smkey_dir_fd = openat(AT_FDCWD,getenv("LATTICE_MS_KEYS"),O_DIRECTORY);

    unsigned char* pub_ek = (unsigned char*) sodium_malloc(crypto_box_PUBLICKEYBYTES);
    unsigned char* sec_ek = (unsigned char*) sodium_malloc(crypto_sign_SECRETKEYBYTES);
    unsigned char* ms_ek = (unsigned char*) sodium_malloc(crypto_box_PUBLICKEYBYTES);


    if(smkey_dir_fd == -1){
        perror("Failed opening lattice key dir.");
        abort();
    }
    int emkey_fd = openat(smkey_dir_fd,getenv("LATTICE_M_ENCSUB_KEY"),O_RDONLY|O_CLOEXEC);
    if(emkey_fd==-1){
        perror("Failed retrieving lattice MS-signing key.");
        close(smkey_dir_fd);
        sodium_free(ms_ek);
        sodium_free(pub_ek);
        sodium_free(sec_ek);
        abort();
    }

    if(read(emkey_fd,ms_ek,crypto_sign_SEEDBYTES)==-1){
        perror("Failed retrieving lattice MS-signing key.");
        sodium_memzero(ms_ek,crypto_sign_SEEDBYTES);
        sodium_free(ms_ek);
        close(smkey_dir_fd);
        close(emkey_fd);
        sodium_free(pub_ek);
        sodium_free(sec_ek);
        abort();
    }
    close(emkey_fd);
    close(smkey_dir_fd);

    crypto_box_seed_keypair(pub_ek,sec_ek,ms_ek);

    sodium_memzero(ms_ek,crypto_sign_SEEDBYTES);
    sodium_free(ms_ek);

    int lattkey_dir_fd = openat(AT_FDCWD,getenv("LATTICKEYDIR"),O_DIRECTORY);

    int secenckey_fd = openat(lattkey_dir_fd,getenv("LATTICE_SEC_ENCRYPT_KEY"),O_TRUNC|O_CREAT|O_WRONLY|O_CLOEXEC,S_IWUSR|S_IRUSR);
    if(secenckey_fd==-1){
        perror("Failed opening lattice sec signing key.");
        sodium_memzero(sec_ek,crypto_box_SECRETKEYBYTES);
        sodium_free(sec_ek);
        close(lattkey_dir_fd);
        sodium_memzero(sec_ek,crypto_box_PUBLICKEYBYTES);
        sodium_free(pub_ek);
        abort();
    }
    if (write(secenckey_fd,sec_ek,crypto_box_SECRETKEYBYTES)==-1){
        perror("Failed writing out sec signing key.");
        sodium_memzero(sec_ek,crypto_box_SECRETKEYBYTES);
        sodium_free(sec_ek);
        close(lattkey_dir_fd);
        close(secenckey_fd);
        sodium_memzero(sec_ek,crypto_box_PUBLICKEYBYTES);
        sodium_free(pub_ek);
        abort();
    }
    fsync(secenckey_fd);
    close(secenckey_fd);

    sodium_memzero(sec_ek,crypto_box_SECRETKEYBYTES);
    sodium_free(sec_ek);

    int pubenckey_fd = openat(lattkey_dir_fd, getenv("LATTICE_PUB_ENCRYPT_KEY"), O_TRUNC | O_CREAT | O_WRONLY | O_CLOEXEC, S_IWUSR | S_IRUSR);
    if(pubenckey_fd == -1){
        perror("Failed opening lattice pub signing key.");
        sodium_memzero(pub_ek,crypto_box_PUBLICKEYBYTES);
        sodium_free(pub_ek);
        close(lattkey_dir_fd);
        abort();
    }

    if (write(pubenckey_fd, pub_ek, crypto_box_PUBLICKEYBYTES) == -1){
        perror("Writing out pub-sign key failed.");
        sodium_memzero(pub_ek,crypto_box_PUBLICKEYBYTES);
        sodium_free(pub_ek);
        close(lattkey_dir_fd);
        close(pubenckey_fd);
    }
    fsync(pubenckey_fd);
    close(pubenckey_fd);

    sodium_memzero(pub_ek,crypto_box_PUBLICKEYBYTES);
    sodium_free(pub_ek);

    close(lattkey_dir_fd);
}


/*
 * Stand-alone function to generate 'Lattice Hash Key'.
 *
 * Key is dumped to a location defined by an env variable with '.lttckey' extension.
 * Aborts on error.
 *
 * Files are also updated or created if not found for storing the 'context' and 'id' that were used with
 * the Hash-master-subkey to derive the hash key.
 *
 * The context is a 32-byte string randomly generated at the time of the key's creation.
 *
 * The id is a long int used as a nonce, and is incremented each time a new key is derived. If no id file is found,
 * one is created and the id begins again at 1.
 *
 * https://doc.libsodium.org/hashing/generic_hashing */
void gen_hash_key(void){
    // LATTICE_M_HSHSUB_KEY
    // LATTICE_HASH_KEY
    //LATTICE_HASHKEY_CONTEXT_FI=Lattice_HashKey_context;LATTICE_HASHKEY_ID_FI=Lattice_HashKey_context_id
    char* hk_context_fi = getenv("LATTICE_HASHKEY_CONTEXT_FI");
    char* hk_ctxt_id_fi = getenv("LATTICE_HASHKEY_ID_FI");

    int i;
    int sk_cid_exists;
    ulong sk_ctxt_id_bytes_in;

    unsigned char* sec_hk = (unsigned char*) sodium_malloc(crypto_kdf_KEYBYTES);
    unsigned char* ms_hk = (unsigned char*) sodium_malloc(crypto_kdf_KEYBYTES);
    uchar* sk_contexts_buf = sodium_malloc((LTTC_SK_CNTXT_LEN));
    char* sk_contexts;
    ulong* sk_ids;
    uchar* sk_ids_buf;
    char newline[1] = {'\n'};

    int smkey_dir_fd = openat(AT_FDCWD,getenv("LATTICKEYDIR"),O_DIRECTORY);
    if(smkey_dir_fd == -1){
        perror("Failed opening lattice key dir.");
        sodium_free(sec_hk);
        sodium_free(ms_hk);
        abort();
    }

    int sk_ctxt_fd = openat(smkey_dir_fd,hk_context_fi,O_TRUNC|O_CREAT|O_WRONLY|O_CLOEXEC,S_IWUSR|S_IRUSR);
    if (sk_ctxt_fd == -1){perror("LSK contexts file access failed.");
        close(smkey_dir_fd);
        sodium_free(sk_contexts_buf);
        abort();}


    sk_contexts = (char*) sodium_malloc((LTTC_SK_CNTXT_HEXSTR_LEN));
    sodium_munlock(sk_contexts,LTTC_SK_CNTXT_HEXSTR_LEN);
    sodium_memzero(sk_contexts,LTTC_SK_CNTXT_HEXSTR_LEN);
    *(sk_contexts+LTTC_SK_CNTXT_HEXSTR_LEN-1) = '\n';


    randombytes_buf(sk_contexts_buf,LTTC_SK_CNTXT_BUF_LEN);
    sodium_bin2hex(sk_contexts,LTTC_SK_CNTXT_HEXSTR_LEN,sk_contexts_buf,LTTC_SK_CNTXT_BUF_LEN);

    if(write(sk_ctxt_fd,sk_contexts,LTTC_SK_CNTXT_HEXSTR_LEN)==-1){
        perror("Failed writing hashkey context.");

        close(smkey_dir_fd);
        close(sk_ctxt_fd);
        sodium_memzero(sk_contexts,LTTC_SK_CNTXT_HEXSTR_LEN);
        sodium_free(sk_contexts);
        sodium_free(sk_contexts_buf);
        abort();
    }
    fsync(sk_ctxt_fd);

    sk_ids = (ulong*) malloc(ULONG_SZ);
    sk_ids_buf = (uchar*) calloc((ULONG_SZ),UCHAR_SZ);

    if(access(getenv("LATTICE_HK_CTXID_FI"), F_OK) != -1){
        sk_cid_exists = 1;
    }else{
        sk_cid_exists = 0;
    }
    int sk_ctxt_id_fd = openat(smkey_dir_fd, hk_ctxt_id_fi, O_CREAT | O_RDWR | O_CLOEXEC, S_IWUSR | S_IRUSR);
    if (sk_ctxt_id_fd == -1){perror("LSK context IDs file access failed.");
        close(smkey_dir_fd);
        close(sk_ctxt_fd);
        sodium_free(sk_contexts_buf);
        abort();}

    if (sk_cid_exists) {
        sk_ctxt_id_bytes_in = read(sk_ctxt_id_fd, sk_ids_buf, ULONG_SZ);
        if (sk_ctxt_id_bytes_in == -1) {
            perror("Failed reading LSK context IDs file");
            close(smkey_dir_fd);
            close(sk_ctxt_fd);
            close(sk_ctxt_id_fd);
            sodium_free(sk_contexts_buf);
            abort();
        }
        memcpy(sk_ids,sk_ids_buf,ULONG_SZ);
        *sk_ids += 1;
    }else{
        *sk_ids = 1;
    }

    printf(">%ld\n",*sk_ids);

    lseek(sk_ctxt_id_fd,0,SEEK_SET);
    memcpy(sk_ids_buf,sk_ids,ULONG_SZ);
    if(write(sk_ctxt_id_fd,sk_ids_buf,ULONG_SZ)==-1){
        perror("Failed writing to sk IDs file.");
        fprintf(stderr,"sk ids:\n\t [ %ld ]\n",*sk_ids);
    }
    if(write(sk_ctxt_id_fd,newline,UCHAR_SZ)==-1){
        perror("Failed writing to sk IDs file.");
        fprintf(stderr,"sk ids:\n\t [ %ld ]\n",*sk_ids);
    }

    fsync(sk_ctxt_id_fd);

    int mhkey_dir_fd = openat(AT_FDCWD,getenv("LATTICE_MS_KEYS"),O_DIRECTORY);
    if(mhkey_dir_fd == -1){
        perror("Failed opening lattice key dir.");
        sodium_free(sec_hk);
        sodium_free(ms_hk);
        abort();
    }

    int hmkey_fd = openat(mhkey_dir_fd,getenv("LATTICE_M_HSHSUB_KEY"),O_RDONLY|O_CLOEXEC);
    if(hmkey_fd==-1){
        perror("Failed retrieving lattice MS-signing key.");
        close(smkey_dir_fd);
        close(mhkey_dir_fd);
        sodium_free(sec_hk);
        sodium_free(ms_hk);
        abort();
    }

    if(read(hmkey_fd,ms_hk,crypto_kdf_KEYBYTES)==-1){
        perror("Failed retrieving lattice MS-signing key.");
        sodium_memzero(ms_hk,crypto_kdf_KEYBYTES);
        sodium_free(ms_hk);
        close(smkey_dir_fd);
        close(hmkey_fd);
        sodium_free(sec_hk);
        abort();
    }
    close(hmkey_fd);
    close(mhkey_dir_fd);

    crypto_kdf_derive_from_key(sec_hk,crypto_kdf_KEYBYTES,*sk_ids,sk_contexts,ms_hk);

    sodium_memzero(ms_hk,crypto_kdf_KEYBYTES);
    sodium_free(ms_hk);


    int secenckey_fd = openat(smkey_dir_fd,getenv("LATTICE_HASH_KEY"),O_TRUNC|O_CREAT|O_WRONLY|O_CLOEXEC,S_IWUSR|S_IRUSR);
    if(secenckey_fd==-1){
        perror("Failed opening lattice sec hash key.");
        sodium_memzero(sec_hk,crypto_kdf_KEYBYTES);
        sodium_free(sec_hk);
        close(smkey_dir_fd);
        sodium_memzero(sec_hk,crypto_kdf_KEYBYTES);
        abort();
    }
    if (write(secenckey_fd,sec_hk,crypto_kdf_KEYBYTES)==-1){
        perror("Failed writing out sec hash key.");
        sodium_memzero(sec_hk,crypto_kdf_KEYBYTES);
        sodium_free(sec_hk);
        close(smkey_dir_fd);
        close(secenckey_fd);
        sodium_memzero(sec_hk,crypto_kdf_KEYBYTES);
        abort();
    }
    fsync(secenckey_fd);
    close(secenckey_fd);

    sodium_memzero(sec_hk,crypto_kdf_KEYBYTES);
    sodium_free(sec_hk);

    close(smkey_dir_fd);
}

/*
 * Stand-alone function to generate 'Lattice Auth Key'.
 *
 * Key is dumped to a location defined by env variable with '.lttckey' extension.
 * aborts on error.
 * */
void gen_auth_key(){
    LatticeKey authKey = (LatticeKey) sodium_malloc(crypto_auth_KEYBYTES);

    int lttckey_dir = openat(AT_FDCWD,getenv("LATTICKEYDIR"),O_DIRECTORY);
    if(lttckey_dir == -1) {
        perror("Failed to open latticekey dir @gen_auth_key.");
        sodium_free(authKey);
        abort();
    }

    int authkey_fi = openat(lttckey_dir, getenv("LATTICE_AUTH_KEY"),O_TRUNC|O_CREAT|O_WRONLY|O_CLOEXEC,S_IWUSR|S_IRUSR);
    if (authkey_fi == -1){
        perror("Failed to open auth key file @genauthkey.");
        sodium_free(authKey);
        close(lttckey_dir);
        abort();
    }

    crypto_auth_keygen(authKey);

    if(write(authkey_fi,authKey,crypto_auth_KEYBYTES) == -1){
        perror("Failed to write out auth key to file. @gen_auth_key");
        sodium_free(authKey);
        close(authkey_fi);
        close(lttckey_dir);
        abort();
    }
    fsync(authkey_fi);
    sodium_memzero(authKey,crypto_auth_KEYBYTES);
    close(authkey_fi);
    close(lttckey_dir);
    sodium_free(authKey);
}

char* key_names(LttcKeyType lk_type, char (*kn_buf)[MAJORKEYCNT]){
    KeyNames keyNames = {{'S','I','G'},{'E','N','C'},{'H','S','H'},{'P','U','B'},{'A','T','H'}};

    (*kn_buf)[0] = keyNames[lk_type][0];
    (*kn_buf)[1] = keyNames[lk_type][1];
    (*kn_buf)[2] = keyNames[lk_type][2];
    (*kn_buf)[3] = '\0';

    return *kn_buf;
}
/*
 * Pass a LttcKeyType enum to choose key to load.
 *
 *   LK_SIG = 0 - Signing key. (64 Bytes)
 *   LK_ENC = 1 - Encryption key. (32 Bytes)
 *   LK_HSH = 2 - Keyed-hash key. (32 Bytes)
 *   LK_PUB = 3 - Public key for client use. (32 Bytes)
 *   LK_ATH = 4 - Generating auth/session tags. (32 Bytes)
 *
 *   returns 0 on success and 1 on error.
 * */
uint loadKey(LttcKeyType lkey_type, LatticeKey* key_out){

    size_t key_sz = lkey_type == LK_SIG ? crypto_sign_SECRETKEYBYTES : crypto_kdf_KEYBYTES;
    int key_fd;
    int lattkeys_dir = openat(AT_FDCWD,getenv("LATTICKEYDIR"),O_DIRECTORY);
    if (lattkeys_dir == -1){
        perror("Couldn't open lattice-key dir @loadkey.");
        return 1;
    }
    *key_out = (LatticeKey) sodium_malloc(key_sz);
//    LATTICE_HASH_KEY=Lattice_HashKey;LATTICE_PUB_SIGNING_KEY=Lattice_Signing_Pub_Key.lttckey ;LATTICE_SEC_ENCRYPT_KEY=Lattice_Encrypt_Sec_Key.lttckey;LATTICE_SEC_SIGNING_KEY=Lattice_Signing_Sec_Key.lttckey

    switch (lkey_type) {
        case LK_SIG:
            key_fd = openat(lattkeys_dir, getenv("LATTICE_SEC_SIGNING_KEY"), O_RDONLY|O_CLOEXEC);
            break;
        case LK_ENC:
          //  key_out = sodium_malloc(key_sz);
            key_fd = openat(lattkeys_dir, getenv("LATTICE_SEC_ENCRYPT_KEY"), O_RDONLY|O_CLOEXEC);
            break;
        case LK_HSH:
          //  key_out = sodium_malloc(key_sz);
            key_fd = openat(lattkeys_dir, getenv("LATTICE_HASH_KEY"), O_RDONLY|O_CLOEXEC);
            break;
        case LK_PUB:
         //   key_out = sodium_malloc(key_sz);
            key_fd = openat(lattkeys_dir, getenv("LATTICE_PUB_SIGNING_KEY"), O_RDONLY|O_CLOEXEC);
            break;
        case LK_ATH:
            key_fd = openat(lattkeys_dir, getenv("LATTICE_AUTH_KEY"), O_RDONLY|O_CLOEXEC);
            break;
        default:
            fprintf(stderr,"Invalid key option: %d\n @loadkey\n", lkey_type);
            close(lattkeys_dir);
            return 1;

    }
    if (key_fd == -1){
        perror("Failed to open lattice key file.");
        sodium_memzero((*key_out) ,key_sz);
        sodium_free((*key_out) );
        char kname[MAJORKEYCNT] = {0};
        fprintf(stderr,"Failed key: %s\n @loadkey\n", key_names(lkey_type,&kname));
        close(lattkeys_dir);
        return 1;
    }
    if (read(key_fd,*key_out,key_sz)==-1){
        perror("Failed to read lattice-key.");
        sodium_memzero((*key_out) ,key_sz);
        sodium_free((*key_out) );
        char kname[MAJORKEYCNT] = {0};
        fprintf(stderr,"Failed key: %s\n @loadkey\n", key_names(lkey_type,&kname));
        close(lattkeys_dir);
        return 1;
    }
    close(key_fd);
    close(lattkeys_dir);

    return 0;
}

/*
 * Tag is placed in signTagOut, and needs to be sodium_free'd.
 *
 * If 0 is passed for tagbase len, a random string is generated for a base and the tagbase parameter value is ignored,
 * essesntially creating a random tag value, which is placed in the tagbase variable.
 *
 * returns 0 on success and 1 on error.
 * */
uint mk_sign_tag(ULattTag* signTagOut, ullint** tag_len, ULattTagBase* tagbase, ullint tagbase_len){
    if (tagbase_len == 0){
        tagbase_len = LATT_AUTH_MSGBASE_64_LEN;
        *tagbase = sodium_malloc(LATT_AUTH_MSGBASE_64_LEN);
        gen_rand_tag_base(tagbase,0);
    }

    LatticeKey sigKey = NULL;
    *signTagOut = (ULattTag) sodium_malloc(crypto_sign_BYTES);

    if (loadKey(LK_SIG,&sigKey) != 0){
        fprintf(stderr,"Failed to load sig key.\n @mk_sign_tag\n");

        sodium_free(*signTagOut);
        sodium_free(tagbase);
        return 1;
    }

    if (crypto_sign_detached(*signTagOut, *tag_len, (uchar*)*tagbase, tagbase_len, sigKey) != 0){
        fprintf(stderr,"Failed to load sig key.\n @mk_sign_tag\n");

        sodium_memzero(sigKey,crypto_sign_SECRETKEYBYTES);
        sodium_free(sigKey);
        sodium_free(signTagOut);
        sodium_free(*tagbase);
        return 1;
    }
    sodium_memzero(sigKey,crypto_sign_SECRETKEYBYTES);
    sodium_free(sigKey);

    return 0;
}

/*
 * Tag is placed in authTagOut, and needs to be sodium_free'd.
 *
 * If 0 is passed for tagbase len, a random string is generated for a base and the tagbase parameter value is ignored,
 * essesntially creating a random tag value, which is placed in the tagbase variable.
 *
 * returns 0 on success and 1 on error.
 * */
uint mk_auth_tag(ULattTag* authTagOut, ullint* tag_len, ULattTagBase* tagbase, ullint tagbase_len){
    if (tagbase_len == 0){
        tagbase_len = LATT_AUTH_MSGBASE_32_LEN;
        *tagbase = sodium_malloc(LATT_AUTH_MSGBASE_32_LEN);
        gen_rand_tag_base(tagbase,1);
    }

    LatticeKey athKey = NULL;
    *authTagOut = (ULattTag) sodium_malloc(LATT_AUTHTAG_32_LEN);
    if (loadKey(LK_ATH,&athKey) != 0){
        fprintf(stderr,"Failed to load auth key.\n @mk_auth_tag\n");
        sodium_free(*authTagOut);
        sodium_free(tagbase);
        return 1;
    }

    if (crypto_auth(*authTagOut, *tagbase, tagbase_len, athKey) != 0){
        fprintf(stderr,"Failed to load sig key.\n @mk_auth_tag\n");

        sodium_memzero(athKey, crypto_auth_KEYBYTES);
        sodium_free(athKey);
        sodium_free(*authTagOut);
        sodium_free(*tagbase);
        return 1;
    }
    sodium_memzero(athKey, crypto_auth_KEYBYTES);
    sodium_free(athKey);
    *tag_len = crypto_auth_BYTES;
    return 0;
}

/*
 * Verifies an AuthTag generated w/ mk_auth_tag().
 *
 * If 0 is provided to parameter tagbase_len, the value is assumed to be LATT_AUTH_MSGBASE_32_LEN (32 bytes),
 * That is, it's assumed to have been randomly generated by passing 0 to param tagbase_len of mk_auth_tag.
 *
 * Returns 0 if verified, and 1 if not.
 * */
uint verify_auth_tag(ULattTag* authTagIn, ULattTagBase* tagbase, ullint tagbase_len){
    LatticeKey athKey = NULL;
    uint verified;

    if(tagbase_len==0){
        tagbase_len = LATT_AUTH_MSGBASE_32_LEN;
    }

    if (loadKey(LK_ATH,&athKey) != 0){
        fprintf(stderr,"Failed to load auth key.\n @mk_auth_tag\n");
        return 1;
    }

    verified = crypto_auth_verify(*authTagIn,*tagbase,tagbase_len,athKey) == 0 ? 0 : 1;

    return verified;

}





//unsigned char* gen_latttag_base(LatticeID (*Id)){
//
//    Id
//}

//LatticeSessionTag gen_lattice_tag(void){
//    return 0;
