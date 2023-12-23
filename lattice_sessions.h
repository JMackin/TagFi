//
// Created by ujlm on 12/22/23.
//

#ifndef TAGFI_LATTICE_SESSIONS_H
#define TAGFI_LATTICE_SESSIONS_H
#include "lattice_signals.h"

#define SESH_UV_CMD_CEIL 32

typedef struct StatusFrame{
    LattSts status;
    LattAct act_id;
    LattErr err_code;
    unsigned int modr;
}StatusFrame;
typedef StatusFrame* StsFrame;
typedef StatusFrame** StsFrame_PTP;

typedef union uniArr{
    unsigned int* iarr;
    unsigned char* carr;
}uniArr;

typedef struct LatticeState{
    StsFrame frame;
    unsigned long long int cwdnode;
    unsigned int tag;
    unsigned int misc;
}LatticeState;
typedef LatticeState* LState;
typedef LatticeState** LState_PTP;

typedef struct LatticeSession{
    LState state;
    unsigned long* thread;
    unsigned long lastreq;
    int socket;
    unsigned int tag;
    SessionOps op;
}LatticeSession;

typedef LatticeSession * LSession;
typedef LatticeSession ** LSession_PTP;


/**
 *<h4>
 *  \StatusFrame
 *</h4><code>
 *  [status code | err code | action | modifier ]

 * */

typedef struct ErrorBundle{
    LState ltcstate;
    LattErr ltcerr;
    unsigned long relvval;
    char func[32];
    char relvval_desc[64];
    char note[128];
    char msg[256];
    int erno;
    unsigned int raised;
    /**
     *<li> 0: Normal
     *<li> 2: Dummy ltcState in place, alloc'd
     *<li> 3: Dummy ltcState free'd
     *<li> 7: dummy sttsframe in use during raise_err
     */
    unsigned int _internal;
}ErrorBundle;
typedef ErrorBundle* ErrBundle;



LState init_latticestate(void);
LSession init_session(void);
LSession update_session(const SessionOps* op, LSession_PTP lSession, void* newval);
LSession tailor_session(LSession_PTP session, unsigned long* thread_id, int socket, unsigned int tag);
void end_session(LSession_PTP session);

/** Status ops **/

void setSts(StatusFrame** sts_frm, LattSts ltcst, unsigned int modr);

void setErr(StatusFrame** sts_frm, LattErr ltcerr, unsigned int modr);

void setMdr(StatusFrame** sts_frm, unsigned int modr);

void setAct(StatusFrame** sts_frm, LattAct lttact, LattSts ltsts, unsigned int modr);

void setRsp(StatusFrame** sts_frm, LattReply, unsigned int modr);

void stsReset(StatusFrame** sts_frm);

void stsOut(StatusFrame** sts_frm);

void serrOut(StatusFrame** sts_frm, char* msg);

long
stsErno(LattErr ltcerr, StatusFrame **sts_frm, char *msg, unsigned long misc, char *miscdesc, char *function, char *note,
        int erno);

ErrorBundle raiseErr(ErrorBundle bundle);
ErrorBundle init_errorbundle();
ErrorBundle * bundle_add(ErrBundle* bundle, unsigned int attr, void* val);
ErrorBundle bundle_addglob(ErrorBundle bundle, LattErr ltcerr, LState ltcstate, char *msg, unsigned long relvval,
                           const char *relvval_desc, char *func, const char *note, int erno);
ErrorBundle bundle_and_raise(ErrorBundle bundle, LattErr ltcerr, LState ltcstate, char *msg, unsigned long relvval,
                             const char *relvval_desc, char *func, const char *note, int erno);

#endif //TAGFI_LATTICE_SESSIONS_H
