//
// Created by ujlm on 12/5/23.
//

#ifndef TAGFI_OPTIONS_H
#define TAGFI_OPTIONS_H
#define OP_FLG_CNT 5


extern unsigned int opsflags[OP_FLG_CNT];
void init_ops_flags(void);
unsigned int flip_flags(const unsigned int flag_choice[OP_FLG_CNT]);
unsigned long save_ops_settings(__attribute__((unused)) unsigned int);
unsigned long read_ops_settings(void);



#endif //TAGFI_OPTIONS_H
