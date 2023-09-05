//
// Created by ujlm on 8/4/23.
//
void genrandsalt(unsigned long long* expanded_salt);
void rando_sf(unsigned long* salt_ptr);
//void getbigsalt(unsigned long long* big_salt);
void get_many_big_salts(unsigned long long** bigsaltls, int n);
void get_many_little_salts(unsigned long** saltls, int n);
int real_hash_keyfully(unsigned char* in, unsigned char* out, unsigned long inlen, const unsigned char* key, unsigned long klen);
int real_hash_keylessly(unsigned char* in, unsigned char* out, unsigned long inlen);
int little_hash_idx(unsigned char* in, unsigned char* out, unsigned long inlen, const unsigned char *k);
int dump_little_hash_key(unsigned char* kout);
unsigned long long eightchartollong(const unsigned char* in, int wlen);
void process_idx(unsigned char* in, unsigned int wlen, unsigned long* out, unsigned char* k);
int little_idx_hkey_3264(unsigned char* tobehshed, unsigned char* outp, unsigned long long* numa, unsigned long* numb, unsigned int wlen);