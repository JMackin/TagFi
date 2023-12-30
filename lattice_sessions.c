//
// Created by ujlm on 12/22/23.
//

#include <malloc.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utils.h>
#include "lattice_sessions.h"
#include "Consts.h"
#include "lattice_signals.h"
#include "FiOps.h"



/* Create new struct to encapsulate a thread and bundle it with sync resources.*/
Spawn spawn_thread(pthread_t thread_id, SpawnLock lock, SpawnNotify notify, uint tag_alpha){

    Spawn spawn = (Spawn) malloc(sizeof(ThreadSpawn));
    spawn->thread = thread_id;
    spawn->lock = lock;
    spawn->notify = notify;
//    spawn->res_keys = calloc(8,sizeof(pthread_key_t*));

    /*
     * tag vals:
     *  alpha:
     *      0 = NULL
     *      1 = initialized and unassigned
     *    memory status (4-16-28):
     *      16 = struct to-be free'd
     *      24 = res-keys free'd
     */
    spawn->tag[0] = (tag_alpha == 0) ? 1 : tag_alpha; //TODO: enum for representing tag vals
    spawn->tag[1] = 0;
    return spawn;
}

uint destroy_spawn(Spawn spawn){
    if (spawn->tag[0] & 16){
        if(spawn->tag[0] != 24){
//            free(spawn->res_keys);
//            spawn->tag[0] |= 8; // sets bit to inidicate res_keys is free'd
        }
        free(spawn);
        return 0;
    }else{
        return 1;
    }
}

SPool init_spawnpool(int thread_count, int queue_size){
    SPool spawnpool = malloc(sizeof(SpawnPool));
    spawnpool->spawn = NULL;
    spawnpool->running_tag = 0;
    spawnpool->_internal = SPS_INIT;   // 0: Init / all normal
    spawnpool->count = 0;
    spawnpool->head = 0;
    spawnpool->queue_size = 0;
    spawnpool->stop = 0;
    spawnpool->tail = 0;
    spawnpool->task_queue = 0;
    spawnpool->thread_count = 0;
    return spawnpool;
}

// tag_or_id: ISTAG = 0, ISID = 1
Spawn find_spawn(uint tag_or_id, ulong tagid, SPool sPool){

    uint pos = 0;

    if (tag_or_id){
        pthread_t id;
        do{
            if (sPool->spawn+(pos) == NULL){
                return NULL;
            }
            id = ((sPool->spawn+(pos++)))->thread;
        }while (id != tagid && id != 0);
        if (id == 0){
            return NULL;
        }
    }else{
        uint tag;
        do{
            if (sPool->spawn+(pos) == NULL){
                return NULL;
            }
            tag = ((sPool->spawn+(pos++)))->tag[0];
        }while (tag != tagid);
    }

    return (sPool->spawn+(pos++));
}

uint push_spawn_task(SPool_PTP spawnpool, SpawnAct task, uint pos, uint tag){

    if ((*spawnpool)->queue_size - 1 >= TASKQUEUE_MAX){
        (*spawnpool)->_internal = SPS_ATTASKMAX;  // 8: Full task capacity
        return 1;
    }
    Spawn spawn_pos = find_spawn(ISTAG,tag,*spawnpool);
    if (spawn_pos == NULL){
        (*spawnpool)->_internal = SPS_SPAWNNOTFOUND;
        return 1;
    }
    if (spawn_pos->tag[0] != task.tag[0]){
        (*spawnpool)->_internal = SPS_TAGMISMATCH;
        return 1;
    }
    if(spawn_pos->tag[1] > 0){
        ++spawn_pos->tag[1];
        task.tag[1] = spawn_pos->tag[1];
    }
    *((*spawnpool)->task_queue+((*spawnpool)->queue_size++)) = task;

    return 0;
}

int add_spawn(SPool_PTP spawnpool, pthread_t thread, SpawnLock lock, SpawnAct spawnAct) {

    if((*spawnpool)->thread_count-1 >= THREADCNT_MAX){
        (*spawnpool)->_internal = SPS_ATTHREADMAX;  // 2: Full thread capacity
        return 1;
    }if ((*spawnpool)->queue_size - 1 >= TASKQUEUE_MAX){
        (*spawnpool)->_internal = SPS_ATTASKMAX;  // 8: Full task capacity
        return 1;
    }

    (*spawnpool)->thread_count++;
    (*spawnpool)->running_tag++;

    if (((*spawnpool)->running_tag) >= SPAWNTAG_CEIL){
        (*spawnpool)->running_tag = 1;
        (*spawnpool)->_internal = SPS_TAGROLLOVER; // 4: Running_tag rolled over
    }

    uint newtag = ++(*spawnpool)->running_tag;
    *((*spawnpool)->spawn + (*spawnpool)->thread_count) = *spawn_thread(thread, lock, NULL, 0);

    push_spawn_task(spawnpool,spawnAct,0,newtag);

    return 0;

}

int add_SPS_lock(pthread_mutex_t* lock, SPool_PTP spawnpool){
    return 0;
}


SSession init_session(SpawnSecrets secrets){
    /*
* tag vals:
*  alpha:
*      0 = NULL
*      1 = initialized and unassigned
*      2 = tailored and assigned.
*    memory status (4-32-60):
*      32 = struct to-be free'd
*      48 = statusFrame free'd
*      56 = latticestate free'd
*/
    SSession lSession = (SSession) malloc(sizeof(SpawnSession));
    lSession->tag[0] = 1;
    lSession->tag[1] = 0;
    lSession->secrets = secrets;
    lSession->lastreq = 0;
    lSession->socket = 0;
    lSession->kit_key = NULL;
    lSession->op = SESH_INIT;
    lSession->spawn = NULL;

    return lSession;
}
//
//__attribute__((unused)) SSession update_session(const SessionOps* op_ptr, SSession_PTP lSession, void* newval){
//    SessionOps op = *op_ptr;
//    uint i = 0;
//    uint itr;
//
//    if (op == SESH_SERIAL_UPDATE) {
//        newval = (multsz_int*) newval;
//        itr = (*((multsz_int*) newval)).reg_ui;
//        while ((i++) < itr) {
//            op = *(op_ptr+i);
//            multsz_int* nv = (newval+i);
//            if (op <= SESH_UV_CMD_CEIL) {
//                switch (op) {
////                    case SESH_UV_THREAD:
////                        (*lSession)->thread = nv->long_ui_ptr;
////                        break;
//                    case SESH_UV_SOCKET:
//                        (*lSession)->socket = nv->reg_i;
//                        break;
//                    case SESH_UV_LASTREQ:
//                        (*lSession)->lastreq = nv->long_ui;
//                        break;
//                    case SESH_UV_TAG:
//                        (*lSession)->tag[1] = nv->long_ui;
//                        break;
//                    default:
//                        break;
//                }
//            }
//        }
//    }else {
//        void* nv = (newval+i);
//        if (op <= SESH_UV_CMD_CEIL) {
//            switch (op) {
//                case SESH_UV_STATE:
//                    (*lSession)->state = (LState) nv;
//                    break;
////                case SESH_UV_THREAD:
////                    (*lSession)->thread = ((unsigned long *) nv);
//                    break;
//                case SESH_UV_SOCKET:
//                    (*lSession)->socket = *((int *) nv);
//                    break;
//                case SESH_UV_LASTREQ:
//                    (*lSession)->lastreq = *((unsigned long *) nv);
//                    break;
//                case SESH_UV_TAG:
//                    (*lSession)->tag[1] = *((unsigned long *) nv);
//                    break;
//                case SESH_UV_OP:
//                    (*lSession)->op = *((SessionOps *) nv);
//                    break;
//                default:
//                    break;
//            }
//        }
//    }
//    return *lSession;
//}

SSession tailor_session(SSession_PTP    session, int socket, Spawn spawn, KitKey key) {

    if ((*session)->tag[0] != 1){
        fprintf(stderr,"Session must be newly initialized with init_session()\n");
        return NULL;
    }
    if(spawn->tag[0] != 1) {
        fprintf(stderr,"ThreadSpawn must be newly initialized with spawn_thread()\n");
        return NULL;
    }
//    int socket;
//    SessionOps op;
//    Tag tag;
//    unsigned long lastreq;
//    LState state;
//    Spawn spawn;
//    KitKey kit_key;
//    SpawnSecrets secrets;

    (*session)->spawn = spawn;
    (*session)->socket = socket;
    (*session)->kit_key = key;

    return *session;

}

void end_session(SSession_PTP session) {
    uint tag = (*session)->tag[0];

    if ((*session) != NULL) {
        if (tag & 32) {
            if ((*session)->state != NULL) {
                if ((*session)->state->frame != NULL) {
                    free((*session)->state->frame);
                    (*session)->tag[0] |= 16; //Sets the bit indicating status frame has been free'd
                    (*session)->state->frame = NULL;
                }
                free((*session)->state);
                (*session)->tag[0] |= 8; // Sets the bit indicating LatticeState has been free'd
                (*session)->state = NULL;
            }
            free(*session);
            *session = NULL;
        }
    }
}


//uint dump_spawn_pieces(LatticeID_PTA id, SpawnPieces item, void* item_val){
//
//    char* outdir = (char*) calloc(LATT_AUTH_BYTES+SPAWNPIECES_EXT_LEN,UCHAR_SZ);
//    uint res;
//
//    int spawndir_fd = openat(AT_FDCWD, getenv("SPAWN_DIR"), O_DIRECTORY, O_RDONLY);
//    if (spawndir_fd == -1){
//        perror("Couldn't open spawn dir (@dump_spawn_pieces).");
//        free(outdir);
//        return 1;
//    }
//
//    switch (item) {
//        case PIECE_NONE:
//        case PIECE_BODY:
//            res = 1;
//            break;
//        case PIECE_SESSION:
//            printf("Session\n");
//            SSession session = (SSession) item_val;
//            res = 0;
//            break;
//        case PIECE_STATE:
//            printf("State\n");
//            LState state = (LState) item_val;
//            res = 0;
//            break;
//        case PIECE_AUTHMAP:
//            printf("AuthMap\n");
//            AuthTagMap tagMap = (AuthTagMap) item_val;
//            res = 0;
//            break;
//        case PIECE_KIT:
//            printf("Kit\n");
//            res = 2;
//            break;
//        case PIECE_ID:
//            printf("ID\n");
//            LatticeID_PTA latticeid = (LatticeID_PTA) item_val;
//            res = 0;
//            break;
//        case PIECE_KEY:
//            printf("Key\n");
//            SesKey spawnKey = (SesKey) item_val;
//            res = 0;
//            break;
//        default:
//            fprintf(stderr,"Invalid Spawn Piece value: %d\n", item);
//            res = 1;
//            break;
//    }
//
//    if (res == 0){
//
//    }
//
//    return res;
//}