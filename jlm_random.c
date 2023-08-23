#include <sodium.h>

#include <time.h>
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
