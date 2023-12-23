//
// Created by ujlm on 12/22/23.
//

#include <malloc.h>
#include "lattice_sessions.h"
#include "Consts.h"

LState init_latticestate(void){
    StsFrame status_frm = (StsFrame) malloc(sizeof(StatusFrame));
    LState lattstate = (LState) malloc(sizeof(LatticeState));
    (status_frm)->status=RESET;
    (status_frm)->err_code=IMFINE;
    (status_frm)->modr=0;
    lattstate->frame = status_frm;
    lattstate->cwdnode = 1;
    lattstate->misc = 0;

    return lattstate;
}

LSession init_session(void){
    LSession lSession = (LSession) malloc(sizeof(LatticeSession));
    lSession->state = init_latticestate();
    lSession->thread = NULL;
    lSession->tag = 0;
    lSession->lastreq = 0;
    lSession->socket = 0;

    return lSession;
}

LSession update_session(const SessionOps* op_ptr, LSession_PTP lSession, void* newval){
    SessionOps op = *op_ptr;
    uint i = 0;
    uint itr;

    if (op == SESH_SERIAL_UPDATE) {
        newval = (multsz_int*) newval;
        itr = (*((multsz_int*) newval)).reg_ui;
        while ((i++) < itr) {
            op = *(op_ptr+i);
            multsz_int* nv = (newval+i);
            if (op <= SESH_UV_CMD_CEIL) {
                switch (op) {
                    case SESH_UV_THREAD:
                        (*lSession)->thread = nv->long_ui_ptr;
                        break;
                    case SESH_UV_SOCKET:
                        (*lSession)->socket = nv->reg_i;
                        break;
                    case SESH_UV_LASTREQ:
                        (*lSession)->lastreq = nv->long_ui;
                        break;
                    case SESH_UV_TAG:
                        (*lSession)->tag = nv->long_ui;
                        break;
                    default:
                        break;
                }
            }
        }
    }else {
        void* nv = (newval+i);
        if (op <= SESH_UV_CMD_CEIL) {
            switch (op) {
                case SESH_UV_STATE:
                    (*lSession)->state = (LState) nv;
                    break;
                case SESH_UV_THREAD:
                    (*lSession)->thread = ((unsigned long *) nv);
                    break;
                case SESH_UV_SOCKET:
                    (*lSession)->socket = *((int *) nv);
                    break;
                case SESH_UV_LASTREQ:
                    (*lSession)->lastreq = *((unsigned long *) nv);
                    break;
                case SESH_UV_TAG:
                    (*lSession)->tag = *((unsigned long *) nv);
                    break;
                case SESH_UV_OP:
                    (*lSession)->op = *((SessionOps *) nv);
                    break;
                default:
                    break;
            }
        }

    }

    return *lSession;
}

LSession tailor_session(LSession_PTP session, ulong* thread_id, int socket, uint tag){

    if(!tag){tag=1;}

    (*session)->thread = thread_id;
    (*session)->socket = socket;
    (*session)->tag = tag;

    return *session;
}

void end_session(LSession_PTP session){
    if ((*session) != NULL) {
        if ((*session)->state != NULL) {
            if ((*session)->state->frame != NULL) {
                free((*session)->state->frame);
                (*session)->state->frame = NULL;
            }
            free((*session)->state);
            (*session)->state = NULL;
        }
        free(*session);
        *session = NULL;
    }
}