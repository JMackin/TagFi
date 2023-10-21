//
// Created by ujlm on 10/19/23.
//

#ifndef TAGFI_LATTICECOMM_H
#define TAGFI_LATTICECOMM_H

#define SELFRESET 2147483647

#include "lattice_cmds.h"

/*
 *   SLEEP = 0,      // sleeping,
 *   LISTN = 1,      // listening,
 *   CNNIN = 2,      // connection received
 *   REQST = 4,      // received request (pre validation)
 *   RCVCM = 8,      // cmd valid, processing
 *   RESPN = 16,     // response pending
 *   UPDAT = 32,     // item updated
 *   TRVLD = 64,     // Dir Node location changed
 *   SHTDN = 128,    // Shutting down
 *   RESET = 256,    // System reset
 *   STERR = 512,    // Error occured, see error codes
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


/*
 *    SILNT = 0,      // No response
 *    STATS = 1,      // Current status frame
 *    CWDIR = 2,      // ID of current working dirnode
 *    FILID = 4,      // ID of a given file
 *    DIRID = 8,      // ID of a given dir node
 *    DNLST = 16,     // Array of contents for a given dirnode
 *    DCHNS = 32,     // Nodes currently present in the dirchains
 *    OBYLD = 64,     // Yield object
 *    ERRCD = 127,    // Error code
 *    DNODE = 128,    // Resident dirnode for a given file
 *    OBJNM = 256,    // Name of object for a given ID
 *    OINFO = 512     // Info string for a given object
 * */
typedef enum LattReply{
    // No response
    SILNT = 0,
    // Current status frame
    STATS = 1,
    // ID of current working dirnode
    CWDIR = 2,
    // ID of a given file
    FILID = 4,
    // ID of a given dir node
    DIRID = 8,
    // Array of contents for a given dirnode
    DNLST = 16,
    // Nodes currently present in the dirchains
    DCHNS = 32,
    // Yield object
    OBYLD = 64,
    // Error code
    ERRCD = 127,
    // Resident dirnode for a given file
    DNODE = 128,
    // Name of object for a given ID
    OBJNM = 256,
    // Info string for a given object
    OINFO = 512
    // 1024 2048 4096 8192
} LattReply;


/*
 * IMFINE = 0,     // No error status
 * MALREQ = 1,     // Malformed request
 * MISSNG = 2,     // Requested object known but not located
 * UNKNWN = 4,     // Requested object not known
 * NOINFO = 8,     // Failure to produce/change requested info
 * BADCON = 16,    // Failure w/ connection socket
 * BADSOK = 32,    // Failure w/ data socket
 * MISMAP = 64,    // Error with mmap
 * STFAIL = 128,   // Error with stat op.
 * FIFAIL = 256,   // Error with a file descriptor op.
 * MISCLC = 512,   // Computed value doesn't match expected or provided
 * BADHSH = 1024,  // Error with hashing op
 * SODIUM = 2048,  // Error with sodium
 * FULLUP = 4096,  // Structure or object at capacity
 * ADFAIL = 8192,  // Failed to update a structure with an object
 * COLISN = 16384,  // Collision in hash table
 * ILMMOP = 32768,  // Failed memory operation or seg fault
 * EPOLLE = 65536   // EPOLL error
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
    BADCNF = 131072
} LattErr;


/*
 *   ZZZZ = 0,   // Do nothing
 *   RPNG = 1,   // Respond True
 *   RSET = 2,   // Clear buffers, reset
 *   SVSQ = 4,   // Save recieved sequence
 *   SLPP = 8,   // Enter sleep mode
 *   GBYE = 128  // Shutdown exit
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
    // Shutdown exit
    GBYE = 128
}LattAct;


/*
 *  [status code | response ID | err code | action | modifier ]
 */
/*
 * Status Code:
 *   NOTHN = 0
 *   LISTN = 1,      // listening,
 *   CNNIN = 2,      // connection received
 *   REQST = 4,      // received request (pre validation)
 *   RCVCM = 8,      // cmd valid, processing
 *   RESPN = 16,     // response pending
 *   UPDAT = 32,     // item updated
 *   TRVLD = 64,     // Dir Node location changed
 *   SHTDN = 128,    // Shutting down
 *   RESET = 256,    // System reset
 *   STERR = 512,    // Error occured, see error codes
 *   SLEEP = 1024,      // sleeping,
 *
 *
 * Error Code:
 *   IMFINE = 0,     // No error status
 *   MALREQ = 1,     // Malformed request
 *   MISSNG = 2,     // Requested object known but not located
 *   UNKNWN = 4,     // Requested object not known
 *   NOINFO = 8,     // Failure to produce/change requested info
 *   BADCON = 16,    // Failure w/ connection socket
 *   BADSOK = 32,    // Failure w/ data socket
 *   MISMAP = 64,    // Error with mmap
 *   STFAIL = 128,   // Error with stat op.
 *   FIFAIL = 256,   // Error with a file descriptor op.
 *   MISCLC = 512,   // Computed value doesn't match expected or provided
 *   BADHSH = 1024,  // Error with hashing op
 *   SODIUM = 2048,  // Error with sodium
 *   FULLUP = 4096,  // Structure or object at capacity
 *   ADFAIL = 8192,  // Failed to update a structure with an object
 *   COLISN = 16384,  // Collision in hash table
 *   ILMMOP = 32768,  // Failed memory operation or seg fault
 *   EPOLLE = 65536   // EPOLL error
 *   BADCNF = 131072  // Config error
 *
 * Response ID:
 *   SILNT = 0,      // No response
 *   STATS = 1,      // Current status frame
 *   CWDIR = 2,      // ID of current working dirnode
 *   FILID = 4,      // ID of a given file
 *   DIRID = 8,      // ID of a given dir node
 *   DNLST = 16,     // Array of contents for a given dirnode
 *   DCHNS = 32,     // Nodes currently present in the dirchains
 *   OBYLD = 64,     // Yield object
 *   ERRCD = 127,    // Error code
 *   DNODE = 128,    // Resident dirnode for a given file
 *   OBJNM = 256,    // Name of object for a given ID
 *   OINFO = 512     // Info string for a given object
 *
 * Action code:
 *   ZZZZ = 0,   // Do nothing
 *   RPNG = 1,   // Respond True
 *   RSET = 2,   // Clear buffers, reset
 *   SVSQ = 4,   // Save recieved sequence
 *   SLPP = 8,   // Enter sleep mode
 *   GBYE = 128  // Shutdown exit
 *
 * Modifier:
 *        0: None
 * */
typedef struct StatFrame{

    LattStts status;
    LattReply resp_id;
    LattAct act_id;
    LattErr err_code;
    unsigned int modr;
}StatFrame;

/*
 *  // No or nonexistent object
 * NADA = 0,
 * // Hash Lattice
 * LTTC = 1,
 * // Hash bridge
 * BRDG = 2,
 * // Dir node
 * DIRN = 4,
 * // File table
 * FTBL = 8,
 * // File map
 * FIMP = 16,
 * // Request/response flag
 * LFLG = 32,
 * // Status frame
 * SFRM = 64,
 * // Info frame
 * IFRM = 128,
 * // Buffers
 * BUFF = 256,
 * // Stored seq table
 * SEQT = 512,
 * // Cmd sequence (response or request)
 * CMSQ = 1024,
 * // Int or Char array
 * ICAR = 2048,
 * // Dirnode vessel/cursor object
 * VSSL = 4096,
 * // Actual file object
 * FIOB = 8192,
 * // ID for an object
 * IDID = 16384,
 * // Name for an object
 * NMNM = 32768,
 * // Cmd frame lead
 * FRLD = 65536,
 * // File desc or socket
 * FIDE = 131072,
 * // Hash key
 * HSKY = 262144,
 * // Dir Chains
 * DCHN = 524288
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
    // Buffers
    BUFF = 256,
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
    // Cmd frame lead
    FRLD = 65536,
    // File desc or socket
    FIDE = 131072,
    // Hash key
    HSKY = 262144,
    // Dir Chains
    DCHN = 524288
} LattObj;


/*
 *  [ req size | arr type | arr len | CmdSeq struct ptr ]
 */
typedef struct InfoFrame{
    unsigned int rsp_size;
    unsigned int req_size;
    unsigned int arr_type; //0: none, 1: char, 2: int
    unsigned int arr_len;
    Cmd_Seq* cmdSeq;
}InfoFrame;


InfoFrame* init_info_frm(InfoFrame** info_frm);

InfoFrame * parse_req(const unsigned char* req,
                     Cmd_Seq** cmd_seq,
                     InfoFrame* rinfo,
                     StatFrame** sts_frm);

void setSts(StatFrame** sts_frm, LattStts ltcst, unsigned int modr);

void setErr(StatFrame** sts_frm, LattErr ltcerr, unsigned int modr);

void setMdr(StatFrame** sts_frm, unsigned int modr);

void setAct(StatFrame** sts_frm, LattAct lttact, LattStts ltsts, unsigned int modr);

void stsReset(StatFrame** sts_frm);

void stsOut(StatFrame** sts_frm);

void serrOut(StatFrame** sts_frm);



#endif //TAGFI_LATTICECOMM_H
