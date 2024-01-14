//
// Created by ujlm on 12/22/23.
//

#ifndef TAGFI_LATTICE_SESSIONS_H
#define TAGFI_LATTICE_SESSIONS_H
#include "lattice_signals.h"
#include "Consts.h"
#include "Tools.h"
#include <bits/pthreadtypes.h>

#define SPAWNPIECES_CNT 7
#define LATT_AUTH_BYTES 32U
#define LATT_AUTH16_BYTES 16U
#define LATT_AUTHTAG_64_LEN 64U
#define LATT_AUTHTAG_32_LEN 32U
#define LATT_AUTH_MSGBASE_64_LEN 65 // Needs to be >= 2 * bin_in + 1
#define LATT_AUTH_MSGBASE_32_LEN 33
#define SPAWNPIECES_EXT ".spwn"
#define SPAWNPIECES_EXT_LEN 5
#define SPAWNFI_NAME_LEN (LATT_AUTH_BYTES+SPAWNPIECES_EXT_LEN+1)
#define SPAWNTAG_CEIL 0x80000000 // Last tag in sequence to be assigned before rolling over to 1 - (1<<31)
#define THREADCNT_MAX 0x7f  // Threadpool threads capacity - (127)
#define TASKQUEUE_MAX 0x1ff // Threadpool Tasks capacity - (511)
#define ISTAG 0
#define ISID 1

typedef char LatticeID [LATT_AUTH_BYTES]; // Session Persistence and user ID handle
typedef char* LattTagBase; // char string used as the "message" to generate an auth tag.
typedef unsigned char* ULattTagBase; // char string used as the "message" to generate an auth tag.
typedef char* LattTag; // Detached Auth tag derived from signing a TagBase.
typedef unsigned char* ULattTag; // Detached Auth tag derived from signing a TagBase.
typedef char (* LatticeID_PTA)[LATT_AUTH_BYTES]; // const pointer to LatticeID.
typedef unsigned char LatticeSessionTag[LATT_AUTH_BYTES]; // Authentication tag to tie session resources back to a client
typedef unsigned char (* LattSessionTag_PTA)[LATT_AUTH_BYTES]; // Authentication tag to tie session resources back to a client
typedef unsigned char LattAuthTag64[LATT_AUTHTAG_64_LEN]; // Access to resources and previous sessions
typedef unsigned char LattAuthTag32[LATT_AUTHTAG_32_LEN]; // Access to resources and previous sessions
typedef unsigned char (* LattAuthTag32_PTA)[LATT_AUTHTAG_32_LEN]; // const pointer to LattAuthTag64
typedef unsigned char (* LattAuthTag64_PTA)[LATT_AUTHTAG_64_LEN]; // const pointer to LattAuthTag64


typedef pthread_key_t* KitKey;
typedef pthread_key_t** ResourceKeys;
typedef pthread_mutex_t* SpawnLock;
typedef pthread_cond_t* SpawnNotify;

typedef struct SpawnState{
    StsFrame frame;
    unsigned long long cw_dnode;
    Tag tag;
    unsigned long misc;
}SpawnState;
typedef SpawnState* SState;
typedef SpawnState** SState_PTP;

/**
 *
 * <br><br><small><i>
 *  Dereffed Size: 32 + 32 = 64
 */
typedef struct SpawnSecrets{
    LatticeID_PTA Id;
    LattSessionTag_PTA SesKey;
}SpawnSecrets;

/**
 * \AuthTag_Map
 *  This struct is to bundle the "messages" that are generated for each of the major resources
 *  assigned to a client. These messages are generated when resources are allocated and assigned,
 *  and from them an authentication tag is generated using the client's LattAuthTag64. The authentication
 *  tags (declared with type LatticeSessionTag) are then included with the resources so they may be securely tied
 *  back to the client, and can allow for session persistance upon reconnection.
 *
 * <br><br><small><i>
 *  Dereffed Size:
 *  <br>        = 32 + (64?*3)
 *  <br>        = 32 + (3 * LATT_AUTH_MSGBASE_32_LEN)
 *  <br>        = 224
 */
typedef struct SpawnAuthTagMap{
    LattTagBase sessionBase;
    LattTagBase spawnbodyBase;
    LattTagBase kitBase;
    LattTagBase miscBase;
}SpawnAuthTagMap;
typedef SpawnAuthTagMap* AuthTagMap;
typedef SpawnAuthTagMap** AuthTagMap_PTP;


/**
 * \brief State and status of the Hashlattice and spawned threads
 *  <br><br><small><i>
 *  Dereffed Size: 16 + 8 + 8 + 8 = 40
 */


/**
 * \brief Thread Body
 * \Spawn
 *  A spawned thread is encapsulated as a Spawn struct,
 *  which bundles it's handle with sync resources and an ID tag.
 *
 *  Each thread is bundled with a mutex, a condition var,
 *  and an array of thread-specific resource access keys.
 *
 * <br><br><small><i>
 *  Dereffed Size:
 *  <br>             = 8 + 8 + 32 + 8? + 8? + 8?*N
 *  <br>             = 8 + 8 + 32 + sizeof(pthread_mutex_t) + sizeof(pthread_cond_t) + [N * sizeof(pthread_key_t)]
 *  <br>             = 64 + 8N
 *  <br><note>
 *  ( N: # of ResourceKeys )
 * */
typedef struct ThreadSpawn{
    pthread_t thread;
    SpawnLock lock;
    SpawnNotify notify;
    Tag tag;
}ThreadSpawn;
typedef ThreadSpawn* Spawn;

//TODO: Implement sessions.

typedef struct BufferPool{
    buff_arr request_buf;
    buff_arr response_buf;
    buff_arr requestArr_buf;
    buff_arr tempArr_buf;
    Flags_Buffer_PTP flags_buf;
}BufferPool;

/**
 *  Thread-based user session.
 *  \SpawnSession
 *  Each thread, once spawned, is given a session struct containing info and resources unique to the thread.
 *
 *  This bundles a pointer to the Spawn struct which encapsulates the thread handle,
 *  along with a pthread_key that points to the thread's associated Navvy struct,
 *  a value indiciating the last request the thread handled, the data-socket to which
 *  responses are written, and a SessionOps enum which can contain various flags that
 *  indicate the status of the thread session and can be used in conditional handling
 *  and to trigger certain events.
 *
 * <br><br><small><i>
 *  Dereffed Size:
 *  <br>            = (64 + 8N) + 64 + 40 + 8 + 8 + 4 + 8? + 4?
 *  <br>            = [64 + [N * sizeof(pthread_key_t)]] + 64 + 40 +  8 + 8 + 4 + sizeof(pthread_key_t) + sizeof(SessionOps)
 *  <br>            = 200 + 8N
 *  <br><note>( N: # of Spawn::ResourceKeys )
 */
typedef struct SpawnSession{
    int socket;
    SessionOps op;
    Tag tag;
    unsigned long lastreq;
    Spawn spawn;
    KitKey kit_key;
    SpawnSecrets secrets;
    SState state;
    struct BufferPool bufferPool;
}SpawnSession;
typedef SpawnSession * SSession;
typedef SpawnSession ** SSession_PTP;


typedef struct LatticeSingleSession{
    LatticeID id;
    LattSessionTag_PTA sessionTag;
    LSS_DateStamp timestamp;
    AuthTagMap spawn_tagmap;
    ulong misc;
    int comm_pipe;
} LatticeSingleSession;

typedef LatticeSingleSession* LatticeSessionStore;

typedef struct SpawnTask{
    void (*function)(void*);
    void *arg;
    Tag tag;
}SpawnTask;

typedef struct SpawnPool{
    int thread_count;
    int queue_size;
//    int head;
//    int tail;
//    int count;
    uint running_tag;
//    uint stop;
    SpawnPoolState _internal;
    Spawn spawn;
//    SpawnAct *task_queue;
    pthread_mutex_t lock;
    pthread_cond_t notify;
//    LatticeSessionTag sessionTag;
} SpawnPool;

typedef SpawnPool* SPool;
typedef SpawnPool** SPool_PTP;


/**
 *  \StatusFrame
 *<code>
 *  [status code | err code | action | modifier ]
 * */


//__attribute__((unused)) SSession update_session(const SessionOps* op, SSession_PTP lSession, void* newval);
//SSession build_spawn_session(SSession_PTP session, int socket, Spawn spawn);
//void end_session(SSession_PTP session);


Spawn spawn_thread(pthread_t thread_id, SpawnLock lock, uint tag_alpha);
int add_spawn(SPool_PTP spawnpool, pthread_t **thread, SpawnLock lock);
SPool init_spawnpool(uint max_thread_count);

uint init_spawn_fi(LatticeID_PTA id);
uint init_spawn_fi2(LatticeID_PTA id);
SSession init_session(SpawnSecrets secrets);
void init_bufferpool(BufferPool * bufferPool);




#endif //TAGFI_LATTICE_SESSIONS_H
