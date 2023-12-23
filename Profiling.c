//
// Created by ujlm on 11/5/23.
//

#include "Profiling.h"
#include <stdio.h>


// ----------------------------
float TiCNT = 0;
float CiCNT = 0;
double long AVGTIME = 0;
double long CUMTIME = 0;
clock_t cb = 0;
clock_t ca = 0;
double long collRatioA = 0.0;
float collcntA = 0;

void timr_rst(){
    ca = 0;
    cb = 0;
}

clock_t timr_st(){
    timr_rst();
    ca = clock();
    return ca;
}

clock_t timr_hlt(){
    cb = clock();
    upd_avg();
    return cb;
}

double long durat(){return cb-ca;}


void tim_avg_rst()
{
    TiCNT = 0;
    CUMTIME = 0;
    AVGTIME = 0;
}

void upd_avg(){
    ++TiCNT;
    CUMTIME += durat();
    AVGTIME = CUMTIME/TiCNT;
}

void p_atm(char* title){
  if (title != NULL){
        printf("<<%s time: %.3Lf>>\n", title, AVGTIME);
    }else {
        printf("<<time: %.3Lf>>\n", AVGTIME);
    }
}
void p_sstm(clock_t caa, clock_t cbb, char* title){

    if (title != NULL){
        printf("<<%s time: %.3Lf>>\n", title, durat());
    }else {
        printf("<<time: %.3Lf>>\n", durat());
    }
}
void p_stm(char* title, double long dur){

    if (title != NULL){
        printf("<<%s time: %.3Lf>>\n", title, dur);
    }else {
        printf("<<time: %.3Lf>>\n", dur);
    }
}

long double time_avg(){return AVGTIME;}

void time_ssp(){
    p_stm(NULL,durat());
}

void time_supr(){
    timr_hlt();
    upd_avg();
    p_atm("AVG");
    tim_avg_rst();
}

void inc_collcntr(){
    CiCNT++;
}
void inc_collA(){
    collcntA++;
}
void upd_collrat(){
    collRatioA = collcntA/CiCNT;
}

double long coll_rat()
{
    return collRatioA;
}

void coll_rst(){
    CiCNT = 0;
    collRatioA = 0;
    collcntA = 0;
}

void p_colrat(){
    upd_collrat();
    printf("[[ collRatio: %.3Lf%% ]]\n",collRatioA*100);
}

void coll_upup(){
    inc_collA();
    upd_collrat();
}
