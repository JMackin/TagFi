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
#define LTTC_SK_CNTXT_LEN 16
#define LTTC_SK_CNTXT_HEXSTR_LEN 35
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

    LattTagBase tagBase = sodium_allocarray(LATT_AUTH_MSGBASE_LEN,UCHAR_SZ);
    randombytes_buf(tagBase,LATT_AUTH_MSGBASE_LEN);
    sodium_bin2hex(*latticeId,LATT_AUTH_MSGBASE_LEN,tagBase,LATT_AUTH_MSGBASE_LEN);

    sodium_free(tagBase);
}

void gen_crypt_keys(){}

void gen_master_key(void){
    const size_t onebyte = sizeof(uint8_t);
    if(sodium_init() != 0){ perror("sodium failed to init in gen_MK.");abort();}

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

void free_subkeys(LttcTriSubKey lts_key){
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

void derive_master_subkeys(void){

    if(sodium_init() == 1){ perror("sodium failed to init in gen_MK.");abort();}

    int i;
    int sk_fd;
    int sk_cid_exists = 0;
    ulong mk_rin_bytes;
    ulong sk_ctxt_id_bytes_in;
    char sk_names[LTTC_MK_NAME_LEN+2] = {'L','a','t','t','i','c','e','_',0,0,0,'.','l','t','t','c','k','e','y','\0'};
    char sk_types[3][3] = {{'S','i','g'},{'E','n','c'},{'H','s','h'}};

    const size_t onebyte = sizeof(uint8_t);
    const size_t LKey_sz = onebyte*crypto_kdf_KEYBYTES;

    ulong* sk_ids;
    uchar* sk_ids_buf;
    char newline[1] = {'\n'};

    uchar* mkey = (uchar*) sodium_malloc(onebyte*crypto_kdf_KEYBYTES);
    LttcTriSubKey lts_tri_sub_key = (LttcTriSubKey) sodium_malloc(3*sizeof(LatticeKey));
    (*lts_tri_sub_key)[0]=(LatticeKey) sodium_malloc(LKey_sz);
    (*lts_tri_sub_key)[1]=(LatticeKey) sodium_malloc(LKey_sz);
    (*lts_tri_sub_key)[2]=(LatticeKey) sodium_malloc(LKey_sz);

    uchar* sk_contexts_buf = sodium_malloc((LTTC_SK_CNTXT_LEN)*onebyte);
    char* sk_contexts;


    int skey_dir_fd = openat(AT_FDCWD,getenv("LATTICKEYDIR"),O_DIRECTORY);
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
    close(mkey_dir_fd);
    close(mkey_fd);
    fsync(mkey_fd);

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

        randombytes_buf(sk_contexts_buf,LTTC_SK_CNTXT_LEN);
        sodium_bin2hex(sk_contexts,LTTC_SK_CNTXT_HEXSTR_LEN,sk_contexts_buf,LTTC_SK_CNTXT_LEN);

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
                                   LKey_sz,
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

        if(write(sk_fd,*((*lts_tri_sub_key)+i),LTTC_SK_CNTXT_HEXSTR_LEN)==-1){
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
    sodium_memzero(mkey,LKey_sz);
    sodium_free(mkey);
    sodium_free(sk_contexts_buf);
    free_subkeys(lts_tri_sub_key);
    free(sk_ids);
    free(sk_ids_buf);
}

void gen_signing_key(void){
    int smkey_dir_fd = openat(AT_FDCWD,getenv("LATTICKEYDIR"),O_DIRECTORY);

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
        free(pub_sk);
        free(sec_sk);
        free(ms_sk);
        abort();
    }

    if(read(smkey_dir_fd,ms_sk,crypto_sign_SEEDBYTES)!=0){
        perror("Failed retrieving lattice MS-signing key.");
        close(smkey_dir_fd);
        close(smkey_fd);
        free(pub_sk);
        free(sec_sk);
        free(ms_sk);
        abort();
    }


    int pubsigkey_fd = openat(smkey_dir_fd,getenv("LATTICE_PUB_SIGNING_KEY"),O_TRUNC|O_CREAT|O_WRONLY|O_CLOEXEC,S_IWUSR|S_IRUSR);
    if(pubsigkey_fd==-1){
        perror("Failed opening lattice pub signing key.");
        close(smkey_dir_fd);
        close(pubsigkey_fd);
        abort();
    }

    crypto_sign_seed_keypair(pub_sk,sec_sk,)


    int secsigkey_fd = openat(smkey_dir_fd,getenv("LATTICE_SEC_SIGNING_KEY"),O_TRUNC|O_CREAT|O_WRONLY|O_CLOEXEC,S_IWUSR|S_IRUSR);
    if(secsigkey_fd==-1){
        perror("Failed opening lattice pub signing key.");
        close(smkey_dir_fd);
        close(smkey_fd);
        close(pubsigkey_fd);
        abort();
    }

    close(smkey_dir_fd);
    close(smkey_fd);
    close(pubsigkey_fd);
    close(secsigkey_fd);


}


//unsigned char* gen_latttag_base(LatticeID (*Id)){
//
//    Id
//}

//LatticeSessionTag gen_lattice_tag(void){
//    return 0;
