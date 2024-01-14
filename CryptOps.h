#include "lattice_works.h"

//
// Created by ujlm on 8/4/23.
//


void real_hash_keyfully(unsigned char** in, unsigned char** out, unsigned long inlen, const unsigned char** key, unsigned long klen);
void genrandsalt(unsigned long long* expanded_salt);
void rando_sf(unsigned long* salt_ptr);
//void getbigsalt(unsigned long long* big_salt);
void get_many_big_salts(unsigned long long** bigsaltls, int n);
void get_many_little_salts(unsigned long** saltls, int n);
//int real_hash_keyfully(unsigned char* in, unsigned char* out, unsigned long inlen, const unsigned char* key, unsigned long klen);
__attribute__((unused)) int real_hash_keylessly(unsigned char* in, unsigned char* out, unsigned long inlen);
//unsigned char * recv_little_hash_key(char* pathin, unsigned int nlen, unsigned char* bytesout);
void recv_little_hash_key(int dnkeyfd, unsigned char* dirname, unsigned int knmln, unsigned char* bytesout);
void dn_hdid_str(unsigned char** dn_hash, char** strout);
unsigned long latt_hsh_idx(LatticeLittleKey lattkey, unsigned long fhshno, unsigned char intbuf[16]);
unsigned long long eightchartollong(const unsigned char* in, int wlen);
void mk_little_hash_key(unsigned char** kout);
int dump_little_hash_key(unsigned char* kout, unsigned char* name, unsigned int nlen);
unsigned long long little_hsh_llidx(unsigned char* hkey, unsigned char* tobehshed, unsigned int wlen, unsigned long long xno);
unsigned long latt_hsh_idx_2(DNodeArmature* armtr, FiNode *fiNode, char* cbuf[16], unsigned char* obuf[16]);
unsigned int bytes_tostr(char** strout, const unsigned char* bytes, size_t b_len);
char* gen_socket_name(void* namebase);
void gen_master_key(void);
void derive_master_subkeys(void);
void gen_signing_key(void);
void gen_crypt_key(void);
void gen_hash_key(void);
void gen_auth_key();
//uint mk_auth_tag(LattTag authTagOut, LattTagBase* tagbase, unsigned long long int tagbase_len);

uint mk_sign_tag(ULattTag* signTagOut, unsigned long long int** tag_len, ULattTagBase* tagbase, unsigned long long int tagbase_len);
uint mk_auth_tag(ULattTag* authTagOut, unsigned long long int* tag_len, ULattTagBase* tagbase, unsigned long long int tagbase_len);
void gen_rand_tag_base(ULattTagBase* base_out,uint tagsize);
uint verify_auth_tag(ULattTag* authTagIn, ULattTagBase* tagbase, ullint tagbase_len);



