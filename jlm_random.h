//
// Created by ujlm on 8/4/23.
//
void genrandsalt(unsigned long long* expanded_salt);
void rando_sf(unsigned long* salt_ptr);
//void getbigsalt(unsigned long long* big_salt);
void get_many_big_salts(unsigned long long** bigsaltls, int n);
void get_many_little_salts(unsigned long** saltls, int n);
//int real_hash_keyfully(unsigned char* in, unsigned char* out, unsigned long inlen, const unsigned char* key, unsigned long klen);
int real_hash_keylessly(unsigned char* in, unsigned char* out, unsigned long inlen);
unsigned char * recv_little_hash_key(char* pathin, unsigned int nlen, unsigned char* bytesout);
unsigned long long eightchartollong(const unsigned char* in, int wlen);
void mk_little_hash_key(unsigned char* kout);
int dump_little_hash_key(unsigned char* kout, unsigned char* name, unsigned int nlen);
unsigned long long little_hsh_llidx(unsigned char* hkey, unsigned char* tobehshed, unsigned int wlen, unsigned long long xno);
