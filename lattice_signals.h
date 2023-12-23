//
// Created by ujlm on 11/2/23.
//

#ifndef TAGFI_LATTICE_SIGNALS_H
#define TAGFI_LATTICE_SIGNALS_H


/**
 * <h4><code>
 * \ResponseCMDs
 * <li> FFFF = 0 -  False
 * <li> NRSP = 1 -  Number of responses to follow
 * <li> RARR = 2 -  The following is the ID code for the info that follows thereafter
 * <li> RLEN = 4 -  What follows is the length of an array that follows thereafter
 * <li> STAS = 8 -  Arr that follows is a status frame
 * <li> CODE = 16 - Data type of next response
 * <li> CINT = 32 -  Next response
 * */
typedef enum RspFlag {
    // False
    FFFF = 0,
    // Number of responses to follow
    NRSP = 1,
    // The following is the ID code for the info that follows thereafter
    RARR = 2,
    // What follows is the length of an array that follows thereafter
    RLEN = 4,
    // Arr that follows is a status frame
    STAS = 8,
    // Object type of following info
    CODE = 16,
    // Next response
    CINT = 32,
    // Response arr seg is a char string
    IARR = 64,
    // Response arr seg is an int arr.
    CARR = 128,
    // Response arr seg is a long arr.
    LARR = 256,
    //
    HHHH = 2048,
    // NULL
    ZERO = 512,
    // Error occurred, code will follow
    ERRR = 1024,
    // Vessel traveled
    TRVL = 2048,
    // Carry byte
    STRT = 536870912,
    // End sequence / Masking byte
    DONE = 2147483647
}RspFlag;

/**
 *<h4>
 * RequestCMDS
 *</h4>
 *<br><code>

 \Qualifiers
 * <li> FFF  = 0  - False
 * <li> TTT  = 1  - True
 * <li> DFLT = 2 - Default argument for previous cmd
 * <li> QQQQ = 4 - Empty
\ArrayOps
 * <li> NARR = 8  - Int that follows is the length of an int array that will follow thereafter.
 * <li> GCSQ = 16 - Int that follows is the length of a char array that will follow thereafter
 * <li> RRRR = 32 - Empty
 * <li> AAAA = 64 - Empty
 \TravelOps
 *  <li> ~
 *  <li> GOTO = 128  > Goto dirNode of following id, goto resdir of given file if DFLT
 *  <li> SRCH = 256  > Search for resident dir for given fiid
 *  <li> HEAD = 512  - Goto head node, switch bases if default
 *  <li> VVVV = 1024 - Empty
 \FileOps
 *  <li> ~
 *  <li> FIID = 2048 > Return file filename for id. fid for filename if DFLT
 *  <li> FRES = 4096 > Return resident dir node for given file.
 *  <li> YILD = 8192 > Yield file of following filename , yield resident file table if DFLT
 *  <li> IIII = 16384 - Empty
 \DirNodeOps
 *  <li> ~
 *  <li> DCWD = 32768  > Return ID for given dirnode, current vessel location
 *  <li> JJJJ = 65536  - Empty
 *  <li> LDCH = 131072 > Return list of DirChain node ids. Return list under current base if DFLT.
 *  <li> LIST = 262144 > List dirnode contents of following dID. Current dir if DFLT
 \SystemOps
 *  <li> INFO = 524288  > Produce info assoc. with the following ID code. StatusFrame if defaults
 *  <li> SAVE = 1048576 - Save this sequence
 *  <li> EXIT = 2097152 - Reset/Sleep/Shutdown
 *  <li> UUUU = 4194304 - Empty
 \Structure
 *  <li> LEAD = 536870912 -  Carry byte
 *  <li> END = 2147483647 - End sequence and masking byte
 *
 * <note>NOTE
 * <br>
 *  <note> -Op groups marked with a '~' are mutually exclusive in that only one can be processed per command sequence.
 *  <br>
 *  <note> They can be combined with flags not marked as such however
 *
 * */
typedef enum ReqFlag {

// False, has no effect
    FFF = 0,
// True
    TTT = 1,
// default argument for previous cmd
    DFLT = 2,
//
    QQQQ = 4,

// Int that follows is the length of an int array that will follow thereafter.
    NARR = 8,
// Int that follows is the length of a char array that will follow thereafter
    GCSQ = 16,
//
    RRRR = 32,
//
    AAAA = 64,

// Goto dirNode of following id, goto resdir of given file if DFLT
    GOTO = 128,
// Search for resident dir for given fiid
    SRCH = 256,
// Goto headnode, switch bases if DFLT
    HEAD = 512,
//
    VVVV = 1024,

// Return file filename for id. fid for filename if DFLT
    FIID = 2048,
// Return resident dirnode for given file
    FRES = 4096,
// Yield file of following filename , yield resident file table if DFLT
    YILD = 8192,
//
    IIII = 16384,

// Return diid for given Dirnode. CWD if DFLT.
    DCWD = 32768,
//
    JJJJ = 65536,
// Return list of DirChain node ids. Return list under current base if DFLT.
    LDCH = 131072,
// List dirnode contents for given id. Current dir if DFLT
    LIST = 262144,

// Produce info assoc. with the ID code. StatusFrame if defaults
    INFO = 524288,
// Save this sequence
    SAVE  = 1048576,
// Exit/Sleep/Shutdown
    EXIT = 2097152,
//
    UUUU = 4194304,

// Carry byte
    LEAD = 536870912,
// End sequence and masking byte
    END = 2147483647
} ReqFlag;


/**
 *  <h4><code>
 * \Status
 * <li> SLEEP = 0   -  sleeping
 * <li> LISTN = 1   -  listening
 * <li> CNNIN = 2   -  connection received
 * <li> REQST = 4   -  received request (pre validation)
 * <li> RCVCM = 8   -  cmd valid, processing
 * <li> RESPN = 16  -  response pending
 * <li> UPDAT = 32  -  item updated
 * <li> TRVLD = 64  -  Dir Node location changed
 * <li> SHTDN = 128 -  Shutting down
 * <li> RESET = 256 -  System reset
 * <li> STERR = 512 -  Error occured, see error codes
 * */
typedef enum LattStts{
    // No value
    NOTHN = 0,
    // listening,
    LISTN = 1,
    // connection received
    CNNIN = 2,
    // received request (pre validation)
    REQST = 4,
    // cmd valid, processing
    RCVCM = 8,
    // response pending
    RESPN = 16,
    // item updated
    UPDAT = 32,
    // Dir Node location changed
    TRVLD = 64,
    // Shutting down
    SHTDN = 128,
    // System reset
    RESET = 256,
    // Error occured, see error codes
    STERR = 512,
    // sleeping,
    SLEEP = 1024,

    // 256 512 1024 2048 4096 8192
}LattStts;


/**
 * <h4><code>
 * \Errors
 *<li> IMFINE = 0     -  No error status
 *<li> MALREQ = 1     -  Malformed request
 *<li> MISSNG = 2     -  Requested object known but not located
 *<li> UNKNWN = 4     -  Requested object not known
 *<li> NOINFO = 8     -  Failure to produce/change requested info
 *<li> BADCON = 16    -  Failure w/ connection socket
 *<li> BADSOK = 32    -  Failure w/ data socket
 *<li> MISMAP = 64    -  Error with mmap
 *<li> STFAIL = 128   -  Error with stat op.
 *<li> FIFAIL = 256   -  Error with a file descriptor op.
 *<li> MISCLC = 512   -  Computed value doesn't match expected or provided
 *<li> BADHSH = 1024  -  Error with hashing op
 *<li> SODIUM = 2048  -  Error with sodium
 *<li> FULLUP = 4096  -  Structure or object at capacity
 *<li> ADFAIL = 8192  -  Failed to update a structure with an object
 *<li> COLISN = 16384 -  Collision in hash table
 *<li> ILMMOP = 32768 -  Failed memory operation or seg fault
 *<li> EPOLLE = 65536 -  EPOLL error
 * */
typedef enum LattErr{
    // No error status
    IMFINE = 0,
    // Malformed request
    MALREQ = 1,
    // Requested object known but not located
    MISSNG = 2,
    // Requested object not known
    UNKNWN = 4,
    // Requested info unavailable
    NOINFO = 8,
    // Failure w/ connection socket
    BADCON = 16,
    // Failure w/ data socket
    BADSOK = 32,
    // Error with mmap
    MISMAP = 64,
    // Error with stat op.
    STFAIL = 128,
    // Error with a file descriptor op.
    FIFAIL = 256,
    // Computed value doesn't match expected or provided
    MISCLC = 512,
    // Error with hashing op
    BADHSH = 1024,
    // Error with sodium
    SODIUM = 2048,
    // Structure or object at capacity
    FULLUP = 4096,
    // Failed to update a structure with an object
    ADFAIL = 8192,
    // Collision in hash table
    COLISN = 16384,
    // Failed memory operation or seg fault
    ILMMOP = 32768,
    // EPOLL error
    EPOLLE = 65536,
    // Conf error
    BADCNF = 131072,
    // Response failure
    MISSPK = 262144,
    //SHUTDOWN
    ESHTDN = 524288,
    // Travel Failure
    MISVEN = 1048576
} LattErr;


/**
 * <h4><code>
 *\Actions
 *  <li> ZZZZ = 0 - Do nothing
 *  <li> RPNG = 1 - Respond True
 *  <li> RSET = 2 - Clear buffers, reset
 *  <li> SVSQ = 4 - Save recieved sequence
 *  <li> SLPP = 8 - Enter sleep mode
 *  <li> FRSP = 16 - Form response and reply to request
 *  <li> GBYE = 128 - Shutdown exit
 * */
typedef enum LattAct {
    // Do nothing
    ZZZZ = 0,
    // Respond True
    RPNG = 1,
    // Clear buffers, reset
    RSET = 2,
    // Save recieved sequence
    SVSQ = 4,
    // Enter sleep mode
    SLPP = 8,
    // Form response and reply to request
    FRSP = 16,
    // Shutdown exit
    GBYE = 128
}LattAct;


/**
 * <h4><code>
 * \LatticeObjects
 * <li> NADA = 0 - No or nonexistent object
 * <li> LTTC = 1 -  Hash Lattice
 * <li> BRDG = 2 -  Hash bridge
 * <li> DIRN = 4 -  Dir node
 * <li> FTBL = 8 -  File table
 * <li> FIMP = 16 - File map
 * <li> LFLG = 32 -  Request/response flag
 * <li> SFRM = 64 -  Status frame
 * <li> IFRM = 128 -  Info frame
 * <li> DCHN = 256 - Dir Chains or chainID
 * <li> SEQT = 512 -  Stored seq table
 * <li> CMSQ = 1024 -   Sequence of cmds (response or request)
 * <li> ICAR = 2048 - Int or Char array
 * <li> VSSL = 4096 - Dirnode vessel/cursor object or path traveled
 * <li> FIOB = 8192 - Actual file object
 * <li> IDID = 16384 - ID for an object
 * <li> NMNM = 32768 - Name for an object
 * <li> FIDE = 65536 - File desc or socket
 * <li> HSKY = 262144 - Hash key
 * <li> FRLD = 524288 - Cmd frame lead
 * <li> BUFF = 131072 -  Buffers
 * <li> SHMM = 2097150 - Shm obj
 * <li> MEMA = 4194300 - Memory address.

 */

typedef enum LattObj{
    // No or nonexistent object
    NADA = 0,
    // Hash Lattice
    LTTC = 1,
    // Hash bridge
    BRDG = 2,
    // Dir node
    DIRN = 4,
    // File table
    FTBL = 8,
    // File map
    FIMP = 16,
    // Request/response flag
    LFLG = 32,
    // Status frame
    SFRM = 64,
    // Info frame
    IFRM = 128,
    // Dir Chains or chainID
    DCHN = 256,
    // Stored seq table
    SEQT = 512,
    // Cmd sequence (response or request)
    CMSQ = 1024,
    // Int or Char array
    ICAR = 2048,
    // Dirnode vessel/cursor object or path traveled
    VSSL = 4096,
    // Actual file object
    FIOB = 8192,
    // ID for an object
    IDID = 16384,
    // Name for an object
    NMNM = 32768,
    // File desc or socket
    FIDE = 65536,
    // Buffers
    BUFF = 131072,
    // Hash key
    HSKY = 262144,
    // Cmd frame lead
    FRLD = 524288,
    // Ping reply.
    HERE = 1048575,
    // Shm obj.
    SHMM = 2097150,
    // Memory address
    MEMA = 4194300
} LattObj;

/**
 *<h4><code>
 * Replys
 *<br>

 \InfoOp
 *<li> UNDEF  = 6   - Undefined
 *<li> ERRCD = 10   - An errorcode
 *<li> OINFO = 12   - Info string for a given object
 *<li> STATS = 14   - Current status frame
 \TravelOp
 *<li> DIRIDQ = 3   -  Go to: Change dir to given
 *<li> DSRCHQ = 7   -  Search for given files resident dir
 *<li> DCHNSQ = 11  -  Go to: head node.
 *<li> VVVVVV = 15  -  Empty
 *\DirOp
 *<li> DIRID = 1   - ID of a given dirnode
 *<li> JJJJJ = 5   - Empty
 *<li> DCHNS = 9   - Nodes currently present in the dirchains
 *<li> DNLST = 13  - Array of contents for a given dirnode
 *\FileOp
 *<li> FILID = 0   - ID of a given file
 *<li> DNODE = 2   - Resident dirnode for a given file
 *<li> OBYLD = 4   - Yield file object
 *<li> IIIII = 8   - Empty
 *
 * */

typedef enum LattReply{
/* *
 * INFO
 **/
    //-  Undefined
    UNDEF = 6,
    //-  Info string for a given object
    ERRCD = 10,
    //-  Error code
    OINFO = 12,
    //- Current status frame
    STATS = 14,
/* *
 * Travel
 **/
    //-  Go to: Change dir to given, or to resdir of given file
    DIRIDQ = 3,
    //-  Search for DirNode
    DSRCHQ = 7,
    //-  Go to: head node or opposite base.
    DCHNSQ = 11,
    //-
    VVVVVV = 15,
/* *
 * DIR
 **/
    //-  ID of a given dir node
    DIRID = 1,
    //-
    JJJJJ = 5,
    //-  Nodes currently present in the dirchains
    DCHNS = 9,
    //-  Array of contents for a given dirnode
    DNLST = 13,
/* *
 * FILE
 **/
    //-  ID of a given file
    FILID = 0,
    //-  Resident dirnode for a given file
    DNODE = 2,
    //-  Yield file
    OBYLD = 4,
    //-  Empty
    IIIII = 8,
} LattReply;

typedef union LattFlag{
    ReqFlag req;
    RspFlag rsp;
    int int_flg;
    unsigned int uflg_flg;
}LattFlag;
//SPLITTO works

typedef LattFlag* LttFlgs;
typedef LttFlgs* Flags_Buffer_PTP; // Flags buffer, pointer to pointer.


/**
 *<h4>
 *  \StatusFrame
 *</h4><code>
 *  [status code | err code | action | modifier ]

 * */
typedef struct StatFrame{
    LattStts status;
    LattAct act_id;
    LattErr err_code;
    unsigned int modr;
}StatFrame;
typedef StatFrame* SttsFrame;
typedef StatFrame** Status_Frame_PTP;

typedef union uniArr{
    unsigned int* iarr;
    unsigned char* carr;
}uniArr;

typedef struct LatticeState{
    SttsFrame frame;
    unsigned long long int cwdnode;
    unsigned int tag;
    unsigned int misc;
}LatticeState;

typedef LatticeState* LttcState;
typedef LatticeState** LttcState_PTP;

typedef struct ErrorBundle{
    LttcState ltcstate;
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
//(char*) calloc(64,UCHAR_SZ);
//calloc(32,UCHAR_SZ);
//calloc(128,UCHAR_SZ);
//alloc(256,UCHAR_SZ);

/**
 *<h4><code>
 * \SpinOffArgOptions
 *<li> SOA_HASHLATTICE = 1,
 *<li> SOA_REQBUF = 2,
 *<li> SOA_RESPBUF = 4,
 *<li> SOA_REQARRBUF = 8,
 *<li> SOA_TEMPARRBUF = 16,
 *<li> SOA_FLAGSBUF = 32,
 *<li> SOA_INFOFRAME = 64,
 *<li> SOA_RESPTBL = 128,
 *<li> SOA_EPEVENT = 256,
 *<li> SOA_THREADID = 512,
 *<li> SOA_EPOLLFD = 1024,
 *<li> SOA_DATASOCKET = 2048,
 *<li> SOA_BUFLEN = 4096,
 *<li> SOA_TAG = 8192,
 *<li> SOA_MUTEX = 16384
 */
typedef enum SOA_OPTS{
    SOA_HASHLATTICE = 1,
    SOA_REQBUF = 2,
    SOA_RESPBUF = 4,
    SOA_REQARRBUF = 8,
    SOA_TEMPARRBUF = 16,
    SOA_FLAGSBUF = 32,
    SOA_INFOFRAME = 64,
    SOA_RESPTBL = 128,
    SOA_EPEVENT = 256,
    SOA_THREADID = 512,
    SOA_EPOLLFD = 1024,
    SOA_DATASOCKET = 2048,
    SOA_BUFLEN = 4096,
    SOA_TAG = 8192,
    SOA_MUTEX = 16384,
    SOA_INTERNAL = 32768
}SOA_OPTS;

typedef struct SOA_internal{
    unsigned int shtdn : 1;
    unsigned int : 0;
}SOAINTernal;

/**
 *<h4><code>
 * \SpawnPoolState
 * <li> SPS_INIT = 0,
 * <li> SPS_ATTHREADMAX = 2,
 * <li> SPS_TAGROLLOVER = 4,
 * <li> SPS_ATTASKMAX = 8,
 */
typedef enum SpawnPoolState{
    SPS_INIT = 0,
    SPS_ATTHREADMAX = 2,
    SPS_TAGROLLOVER = 4,
    SPS_ATTASKMAX = 8,
    SPS_SPAWNNOTFOUND = 16,
    SPS_TAGMISMATCH = 32
}SpawnPoolState;

/** Status ops **/

void setSts(StatFrame** sts_frm, LattStts ltcst, unsigned int modr);

void setErr(StatFrame** sts_frm, LattErr ltcerr, unsigned int modr);

void setMdr(StatFrame** sts_frm, unsigned int modr);

void setAct(StatFrame** sts_frm, LattAct lttact, LattStts ltsts, unsigned int modr);

void setRsp(StatFrame** sts_frm, LattReply,unsigned int modr);

void stsReset(StatFrame** sts_frm);

void stsOut(StatFrame** sts_frm);

void serrOut(StatFrame** sts_frm, char* msg);

long
stsErno(LattErr ltcerr, StatFrame **sts_frm, char *msg, unsigned long misc, char *miscdesc, char *function, char *note,
        int erno);

ErrorBundle raiseErr(ErrorBundle bundle);
ErrorBundle init_errorbundle();
ErrorBundle * bundle_add(ErrBundle* bundle, unsigned int attr, void* val);
ErrorBundle bundle_addglob(ErrorBundle bundle, LattErr ltcerr, LttcState ltcstate, char *msg, unsigned long relvval,
               const char *relvval_desc, char *func, const char *note, int erno);
ErrorBundle bundle_and_raise(ErrorBundle bundle, LattErr ltcerr, LttcState ltcstate, char *msg, unsigned long relvval,
                             const char *relvval_desc, char *func, const char *note, int erno);

#endif //TAGFI_LATTICE_SIGNALS_H
