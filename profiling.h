//
// Created by ujlm on 11/5/23.
//

#ifndef TAGFI_PROFILING_H
#define TAGFI_PROFILING_H

#include <time.h>


clock_t timr_st();

clock_t timr_hlt();

void upd_avg();

double long durat();

long double time_avg();

void tim_avg_rst();
void timr_rst();

void p_atm(char* title);

void p_sstm(clock_t caa, clock_t cbb, char* title);
void p_stm(char* title, double long dur);
void time_supr();
void time_ssp();

double long coll_rat();

void inc_collA();

void inc_collcntr();

void upd_collrat();

void p_colrat();

void coll_rst();
void coll_upup();




#endif //TAGFI_PROFILING_H
