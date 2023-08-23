//
// Created by ujlm on 8/4/23.
//
void genrandsalt(unsigned long long* expanded_salt);
void rando_sf(unsigned long* salt_ptr);
//void getbigsalt(unsigned long long* big_salt);
void get_many_big_salts(unsigned long long** bigsaltls, int n);
void get_many_little_salts(unsigned long** saltls, int n);