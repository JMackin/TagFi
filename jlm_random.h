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
unsigned long latt_hsh_idx(Armature* armtr, FiNode *fiNode, unsigned char intbuf[16], unsigned int clk);
unsigned long long eightchartollong(const unsigned char* in, int wlen);
void mk_little_hash_key(unsigned char** kout);
int dump_little_hash_key(unsigned char* kout, unsigned char* name, unsigned int nlen);
unsigned long long little_hsh_llidx(unsigned char* hkey, unsigned char* tobehshed, unsigned int wlen, unsigned long long xno);
unsigned long latt_hsh_idx_2(Armature* armtr, FiNode *fiNode, char* cbuf[16], unsigned char* obuf[16]);
