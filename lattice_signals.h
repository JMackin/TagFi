//
// Created by ujlm on 11/2/23.
//

#ifndef TAGFI_LATTICE_SIGNALS_H
#define TAGFI_LATTICE_SIGNALS_H


/**
 * <h4><code>
 * \ResponseCMDs
 * <l_ulong> FFFF = 0 -  False
 * <l_ulong> NRSP = 1 -  Number of responses to follow
 * <l_ulong> RARR = 2 -  The following is the ID code for the info that follows thereafter
 * <l_ulong> RLEN = 4 -  What follows is the length of an array that follows thereafter
 * <l_ulong> STAS = 8 -  Arr that follows is a status frame
 * <l_ulong> CODE = 16 - Data type of next response
 * <l_ulong> CINT = 32 -  Next response
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
    //
    DDDD = 64,
    //
    EEEE = 128,
    //
    GGGG = 255,
    //
    HHHH = 2048,
    // NULL
    ZERO = 512,
    // Error occurred, code will follow
    ERRR = 1024,
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
 * <l_ulong> FFF  = 0  - False
 * <l_ulong> TTT  = 1  - True
 * <l_ulong> DFLT = 2 - Default argument for previous cmd
 * <l_ulong> QQQQ = 4 - Empty
\ArrayOps
 * <l_ulong> NARR = 8  - Int that follows is the length of an int array that will follow thereafter.
 * <l_ulong> GCSQ = 16 - Int that follows is the length of a char array that will follow thereafter
 * <l_ulong> RRRR = 32 - Empty
 * <l_ulong> AAAA = 64 - Empty
 \TravelOps
 *  <l_ulong> ~
 *  <l_ulong> GOTO = 128  > Goto dirNode of following id, goto resdir of given file if DFLT
 *  <l_ulong> SRCH = 256  > Search for resident dir for given fiid
 *  <l_ulong> HEAD = 512  - Goto head node, switch bases if default
 *  <l_ulong> VVVV = 1024 - Empty
 \FileOps
 *  <l_ulong> ~
 *  <l_ulong> FIID = 2048 > Return file filename for id. fid for filename if DFLT
 *  <l_ulong> FRES = 4096 > Return resident dir node for given file.
 *  <l_ulong> YILD = 8192 > Yield file of following filename , yield resident file table if DFLT
 *  <l_ulong> IIII = 16384 - Empty
 \DirNodeOps
 *  <l_ulong> ~
 *  <l_ulong> DCWD = 32768  > Return ID for given dirnode, current vessel location
 *  <l_ulong> JJJJ = 65536  - Empty
 *  <l_ulong> LDCH = 131072 > Return list of DirChain node ids. Return list under current base if DFLT.
 *  <l_ulong> LIST = 262144 > List dirnode contents of following dID. Current dir if DFLT
 \SystemOps
 *  <l_ulong> INFO = 524288  > Produce info assoc. with the following ID code. StatusFrame if defaults
 *  <l_ulong> SAVE = 1048576 - Save this sequence
 *  <l_ulong> EXIT = 2097152 - Reset/Sleep/Shutdown
 *  <l_ulong> UUUU = 4194304 - Empty
 \Structure
 *  <l_ulong> LEAD = 536870912 -  Carry byte
 *  <l_ulong> END = 2147483647 - End sequence and masking byte
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
 * <l_ulong> SLEEP = 0   -  sleeping
 * <l_ulong> LISTN = 1   -  listening
 * <l_ulong> CNNIN = 2   -  connection received
 * <l_ulong> REQST = 4   -  received request (pre validation)
 * <l_ulong> RCVCM = 8   -  cmd valid, processing
 * <l_ulong> RESPN = 16  -  response pending
 * <l_ulong> UPDAT = 32  -  item updated
 * <l_ulong> TRVLD = 64  -  Dir Node location changed
 * <l_ulong> SHTDN = 128 -  Shutting down
 * <l_ulong> RESET = 256 -  System reset
 * <l_ulong> STERR = 512 -  Error occured, see error codes
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
 *<l_ulong> IMFINE = 0     -  No error status
 *<l_ulong> MALREQ = 1     -  Malformed request
 *<l_ulong> MISSNG = 2     -  Requested object known but not located
 *<l_ulong> UNKNWN = 4     -  Requested object not known
 *<l_ulong> NOINFO = 8     -  Failure to produce/change requested info
 *<l_ulong> BADCON = 16    -  Failure w/ connection socket
 *<l_ulong> BADSOK = 32    -  Failure w/ data socket
 *<l_ulong> MISMAP = 64    -  Error with mmap
 *<l_ulong> STFAIL = 128   -  Error with stat op.
 *<l_ulong> FIFAIL = 256   -  Error with a file descriptor op.
 *<l_ulong> MISCLC = 512   -  Computed value doesn't match expected or provided
 *<l_ulong> BADHSH = 1024  -  Error with hashing op
 *<l_ulong> SODIUM = 2048  -  Error with sodium
 *<l_ulong> FULLUP = 4096  -  Structure or object at capacity
 *<l_ulong> ADFAIL = 8192  -  Failed to update a structure with an object
 *<l_ulong> COLISN = 16384 -  Collision in hash table
 *<l_ulong> ILMMOP = 32768 -  Failed memory operation or seg fault
 *<l_ulong> EPOLLE = 65536 -  EPOLL error
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
    ESHTDN = 524288
} LattErr;


/**
 * <h4><code>
 *\Actions
 *  <l_ulong> ZZZZ = 0 - Do nothing
 *  <l_ulong> RPNG = 1 - Respond True
 *  <l_ulong> RSET = 2 - Clear buffers, reset
 *  <l_ulong> SVSQ = 4 - Save recieved sequence
 *  <l_ulong> SLPP = 8 - Enter sleep mode
 *  <l_ulong> FRSP = 16 - Form response and reply to request
 *  <l_ulong> GBYE = 128 - Shutdown exit
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
 * <l_ulong> NADA = 0 - No or nonexistent object
 * <l_ulong> LTTC = 1 -  Hash Lattice
 * <l_ulong> BRDG = 2 -  Hash bridge
 * <l_ulong> DIRN = 4 -  Dir node
 * <l_ulong> FTBL = 8 -  File table
 * <l_ulong> FIMP = 16 - File map
 * <l_ulong> LFLG = 32 -  Request/response flag
 * <l_ulong> SFRM = 64 -  Status frame
 * <l_ulong> IFRM = 128 -  Info frame
 * <l_ulong> DCHN = 256 - Dir Chains
 * <l_ulong> SEQT = 512 -  Stored seq table
 * <l_ulong> CMSQ = 1024 -   Cmd sequence (response or request)
 * <l_ulong> ICAR = 2048 - Int or Char array
 * <l_ulong> VSSL = 4096 - Dirnode vessel/cursor object
 * <l_ulong> FIOB = 8192 - Actual file object
 * <l_ulong> IDID = 16384 - ID for an object
 * <l_ulong> NMNM = 32768 - Name for an object
 * <l_ulong> FIDE = 65536 - File desc or socket
 * <l_ulong> HSKY = 262144 - Hash key
 * <l_ulong> FRLD = 524288 - Cmd frame lead
 * <l_ulong> BUFF = 131072 -  Buffers
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
    // Dir Chains
    DCHN = 256,
    // Stored seq table
    SEQT = 512,
    // Cmd sequence (response or request)
    CMSQ = 1024,
    // Int or Char array
    ICAR = 2048,
    // Dirnode vessel/cursor object
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
    HERE = 1048575

} LattObj;

/**
 *<h4><code>
 * Replys
 *<br>

 \InfoOp
 *<l_ulong> UNDEF  = 6   - Undefined
 *<l_ulong> ERRCD = 10   - An errorcode
 *<l_ulong> OINFO = 12   - Info string for a given object
 *<l_ulong> STATS = 14   - Current status frame
 \TravelOp
 *<l_ulong> DIRIDQ = 3   -  Go to: Change dir to given
 *<l_ulong> DSRCHQ = 7   -  Search for given files resident dir
 *<l_ulong> DCHNSQ = 11  -  Go to: head node.
 *<l_ulong> VVVVVV = 15  -  Empty
 *\DirOp
 *<l_ulong> DIRID = 1   - ID of a given dirnode
 *<l_ulong> JJJJJ = 5   - Empty
 *<l_ulong> DCHNS = 9   - Nodes currently present in the dirchains
 *<l_ulong> DNLST = 13  - Array of contents for a given dirnode
 *\FileOp
 *<l_ulong> FILID = 0   - ID of a given file
 *<l_ulong> DNODE = 2   - Resident dirnode for a given file
 *<l_ulong> OBYLD = 4   - Yield file object
 *<l_ulong> IIIII = 8   - Empty
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

typedef StatFrame* SttsFrm;

typedef union uniArr{
    unsigned int* iarr;
    unsigned char* carr;
}uniArr;

/** Status ops **/

void setSts(StatFrame** sts_frm, LattStts ltcst, unsigned int modr);

void setErr(StatFrame** sts_frm, LattErr ltcerr, unsigned int modr);

void setMdr(StatFrame** sts_frm, unsigned int modr);

void setAct(StatFrame** sts_frm, LattAct lttact, LattStts ltsts, unsigned int modr);

void setRsp(StatFrame** sts_frm, LattReply,unsigned int modr);

void stsReset(StatFrame** sts_frm);

void stsOut(StatFrame** sts_frm);

void serrOut(StatFrame** sts_frm, char* msg);

long stsErno(LattErr ltcerr, StatFrame **sts_frm, int erno, unsigned long misc, char *msg, char *function, char *miscdesc);



#endif //TAGFI_LATTICE_SIGNALS_H
