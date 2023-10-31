//

#ifndef TAGFI_LATTICE_CMDS_H
#define TAGFI_LATTICE_CMDS_H

#define ENDBYTES 538968075
#define RSPARRLEN 16
#define INFARRLEN 19
#include "chkmk_didmap.h"

//    SeqMap* seq_map;


/**
 *<h4><code>
 * RequestCMDS
 *<br>

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
    // End sequence / Masking byte
    DONE = 2147483647
}RspFlag;


typedef union LttFlg{
    ReqFlag req;
    RspFlag rsp;
    int flg;
    unsigned int uflg;
}LttFlg;

typedef LttFlg* LttcFlags;



typedef union uniArr{
    unsigned int* iarr;
    unsigned char* carr;
}uniArr;


typedef struct Cmd_Seq Cmd_Seq;

Cmd_Seq* init_cmdseq(Cmd_Seq** cmdSeq, uniArr ** arr, unsigned int type);

Cmd_Seq *reset_cmdseq(Cmd_Seq **cmdSeq, unsigned int type);

//VER F.
//unsigned int init_seqtbl(Seq_Tbl* seq_tbl, unsigned long mx_sz);

typedef struct Cmd_Seq {
    unsigned long seq_id;
    unsigned int lead;
    unsigned int type;
    unsigned int flg_cnt;
    unsigned int arr_len;
    LttcFlags *flags;
    uniArr *arr;
}Cmd_Seq;

typedef struct InfoFrame {
    unsigned int lead;
    unsigned int cat_pfx;
    unsigned int rsp_size;
    unsigned int req_size;
    unsigned int trfidi[3];
    unsigned int sys_op;
    unsigned int qual;
    unsigned int arr_type; //0: none, 1: char, 2: int
    unsigned int arr_len;
    unsigned int flg_cnt;
    LttcFlags *flags;
    uniArr *arr;
} InfoFrame;
InfoFrame *init_info_frm(InfoFrame **info_frm, uniArr **seqArr);


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
 *<h4><code>
 * Responses
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
    MISSPK = 262144
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
 * <li> BUFF = 256 -  Buffers
 * <li> SEQT = 512 -  Stored seq table
 * <li> CMSQ = 1024 -   Cmd sequence (response or request)
 * <li> ICAR = 2048 - Int or Char array
 * <li> VSSL = 4096 - Dirnode vessel/cursor object
 * <li> FIOB = 8192 - Actual file object
 * <li> IDID = 16384 - ID for an object
 * <li> NMNM = 32768 - Name for an object
 * <li> FRLD = 65536 - Cmd frame lead
 * <li> FIDE = 131072 - File desc or socket
 * <li> HSKY = 262144 - Hash key
 * <li> DCHN = 524288 - Dir Chains
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

typedef unsigned char* cmdKey;

typedef ReqFlag* ReqArr;

typedef RspFlag* RspArr;

typedef struct LatticeCommand {
    RspArr rsps;
    ReqArr reqs;
    int n_req;
} latticeCmd;


//}Cmd_Seq;


/** <h4><code>
 * \InformationFrame
 * <br><verbatim>
 *<br> [ rsp size | req size | [trvl_op,  |  fi_op,   | di_op |
 *<br> | sys_op   |  qual    |  arr type  |   arr len | CmdSeq ptr ]
 */


/**
 * \SequenceTable
 *<h4><code>
 * [ max table size | cmd hash key | sequence arr ]
 *</h4>

 *
 * <i>Hash table storing common sequences
 * to compare against or pull from.
 * */


/**
 *
 * \Frame
 *<h4><code>  [ ID | cmd lead | 0: request or 1: response | flags arr | flag count | arr length | arr* ] </h4>
 *
 \Sequence
 *<h4> [CmdLead] -> [ArrayLength] -> [Array] -> [ENDflag] </h4>
 *<br>
 *<li><b>cmd-lead</b><br>      <i> LEAD-flag ORd with cmd flags. Can be extracted by ANDing with END-flag.
 *<li><b>array-length</b><br> <i> number of discrete items (chars or ints) in the arr. 0 if no array in the request.
 *<li><b>array</b><br>         <i> type 0 for no array, 1 for char, 2 for int. Signaled with NARR(int) and GCSQ(char) flags.
 *<li><b>END-Flag</b><br>      <i> will trigger a malformed request error if not in expected position.
 * */


/**
 *  <h4><code>
 *  [status code | err code | action | modifier ]


 * \Status
 * <li> NOTHN = 0
 * <li> LISTN = 1      - listening
 * <li> CNNIN = 2       -  connection received
 * <li> REQST = 4       -   received request (pre validation)
 * <li> RCVCM = 8       -   cmd valid, processing
 * <li> RESPN = 16       -   response pending
 * <li> UPDAT = 32       -   item updated
 * <li> TRVLD = 64     -   Dir Node location changed
 * <li> SHTDN = 128     -   Shutting down
 * <li> RESET = 256     -   System reset
 * <li> STERR = 512     -   Error occured, see error codes
 * <li> SLEEP = 1024        -   sleeping
 *
 * \Error
 * <li> IMFINE = 0,       -   No error status
 * <li> MALREQ = 1,       -   Malformed request
 * <li> MISSNG = 2,       -   Requested object known but not located
 * <li> UNKNWN = 4,       -   Requested object not known
 * <li> NOINFO = 8,       -   Failure to produce/change requested info
 * <li> BADCON = 16,      -   Failure w/ connection socket
 * <li> BADSOK = 32,      -   Failure w/ data socket
 * <li> MISMAP = 64,      -   Error with mmap
 * <li> STFAIL = 128,     -   Error with stat op.
 * <li> FIFAIL = 256,     -   Error with a file descriptor op.
 * <li> MISCLC = 512,     -   Computed value doesn't match expected or provided
 * <li> BADHSH = 1024,    -   Error with hashing op
 * <li> SODIUM = 2048,    -   Error with sodium
 * <li> FULLUP = 4096,    -   Structure or object at capacity
 * <li> ADFAIL = 8192,    -   Failed to update a structure with an object
 * <li> COLISN = 16384,    -   Collision in hash table
 * <li> ILMMOP = 32768,    -   Failed memory operation or seg fault
 * <li> EPOLLE = 65536,     -   EPOLL error
 * <li> BADCNF = 131072    -   Config error
 *
 * \Action
 * <li> ZZZZ = 0,     -   Do nothing
 * <li> RPNG = 1,     -   Respond True
 * <li> RSET = 2,     -   Clear buffers, reset
 * <li> SVSQ = 4,     -   Save recieved sequence
 * <li> SLPP = 8,     -   Enter sleep mode
 * <li> FRSP = 16     - Form response and reply to request
 * <li> GBYE = 128    -   Shutdown exit

<br>
 * \Modifier
 * <note> 0: None
 * */

typedef struct StatFrame{

    LattStts status;
    LattAct act_id;
    LattErr err_code;
    unsigned int modr;
}StatFrame;

//}InfoFrame;
InfoFrame * parse_req(const unsigned char* fullreqbuf,
                      InfoFrame **infofrm,
                      StatFrame** stsfrm,
                      LttcFlags rqflgsbuf,
                      unsigned int** tmparrbuf,
                      unsigned char** carr_buf);

typedef unsigned int** RspMap;

typedef unsigned int (*RspFunc[RSPARRLEN])(StatFrame**, InfoFrame**, DChains*, Lattice*, unsigned char**);

typedef unsigned int (*InfoFunc[INFARRLEN])(unsigned char **buf);


typedef struct Resp_Tbl{
    unsigned int fcnt;
    RspMap* rsp_map; // 3 x 3 x fcnt - 3D array: {LattReply,Mod,actIdx}
    RspFunc* rsp_funcarr;
} Resp_Tbl;

int list_cmds();

void init_lttccmd();

void destroy_lttccmd();

unsigned int build_lead(const unsigned int* cmd_flags, unsigned int flg_cnt);

unsigned long save_seq(Cmd_Seq *cmd_seq, int cnfdir_fd);

//VER F
//unsigned long save_seq(Cmd_Seq *cmd_seq, int cnfdir_fd);

//VER F
//unsigned long save_seq(Cmd_Seq* cmd_seq, Seq_Tbl** seq_tbl, int cnfdir_fd);


void destroy_cmdstructures(unsigned char *buffer, unsigned int *respbuffer, unsigned char *carr, unsigned int *iarr, Resp_Tbl *rsp_tbl);

//VER F
//void destroy_cmdstructures(unsigned char *buffer, unsigned char *respbuffer, unsigned char *carr, unsigned int *iarr,
//                           Resp_Tbl *rsp_tbl, Seq_Tbl *sqTbl);

unsigned int serial_seq(unsigned char* seq_out,
                        Cmd_Seq** cmd_seq);

int unmask_cmds(unsigned int** cmds,
                unsigned int** leads,
                unsigned int** lens,
                unsigned char*** seqs,
                unsigned char*** arrs,
                int cmdcnt);


Cmd_Seq* destroy_cmdseq(StatFrame** sts_frm, Cmd_Seq** cmdSeq);
Cmd_Seq* copy_cmdseq(unsigned int flip, Cmd_Seq** cmdSeq, Cmd_Seq** copy, StatFrame** sts_frm);

RspFunc* rsp_act(
              RspMap rspMap,
              StatFrame** sts_frm,
              InfoFrame** inf_frm,
              RspFunc* (funarr));


void
init_rsptbl(int cnfg_fd, Resp_Tbl **rsp_tbl, StatFrame **sts_frm, InfoFrame **inf_frm, DChains *dchns, Lattice *hltc);

LattReply dtrm_rsp(StatFrame** sts_frm,
                   InfoFrame** inf_frm);


InfoFrame* respond(Resp_Tbl *rsp_tbl,
                   StatFrame **sts_frm,
                   InfoFrame **inf_frm,
                   DChains *dchns,
                   Lattice *hltc,
                   unsigned char **resp_buf);

typedef union LattTyps{
    LattErr err;
    LattReply rpl;
    LattAct act;
    LattObj obj;
    LattStts sts;
    LttFlg flg;
    int ni;
    unsigned int nui;
    unsigned char nuc;
}LattTyps;



/**<br> >Response string element positions:
*<br>
*<br> bytes start - end
*<br> -----------------
*<br> lead: 0 - ltyp_s
*<br> item: ltyp_s - rspsz_b
*<br> arrsz: rspsz_b - (rspsz_b)+uint_s
*<br> arr: (rspsz_b)+uint_s - (rspsz_b)+uint_s+(arr_len*uchar_s)
*<br> DONE: (rspsz_b)+uint_s+(arr_len*uchar_s) - rspsz_b+ltyp_s+uint_s+(arr_len*uchar_s)
*/


/** Response actions */

unsigned int  rsp_sts(StatFrame** sts_frm, InfoFrame** inf_frm, DChains* dchns, Lattice* hltc, unsigned  char**buf);
unsigned int  rsp_nfo(StatFrame** sts_frm, InfoFrame** inf_frm, DChains* dchns, Lattice* hltc, unsigned  char**buf);
unsigned int  rsp_err(StatFrame** sts_frm, InfoFrame** inf_frm, DChains* dchns, Lattice* hltc, unsigned  char**buf);
unsigned int  rsp_und(StatFrame** sts_frm, InfoFrame **inf_frm, DChains* dchns, Lattice* hltc, unsigned  char**buf);

unsigned int  rsp_gond(StatFrame** sts_frm, InfoFrame** inf_frm, DChains* dchns, Lattice* hltc, unsigned char **buf);
unsigned int  rsp_gohd(StatFrame** sts_frm, InfoFrame** inf_frm, DChains* dchns, Lattice* hltc, unsigned char **buf);
unsigned int  rsp_dsch(StatFrame** sts_frm, InfoFrame** inf_frm, DChains* dchns, Lattice* hltc, unsigned char **buf);
unsigned int  rsp_vvvv(StatFrame** sts_frm, InfoFrame** inf_frm, DChains* dchns, Lattice* hltc, unsigned char **buf);

unsigned int  rsp_diid(StatFrame** sts_frm, InfoFrame** inf_frm, DChains* dchns, Lattice* hltc, unsigned char **buf);
unsigned int  rsp_jjjj(StatFrame** sts_frm, InfoFrame** inf_frm, DChains* dchns, Lattice* hltc, unsigned char **buf);
unsigned int  rsp_dcls(StatFrame** sts_frm, InfoFrame** inf_frm, DChains* dchns, Lattice* hltc, unsigned char **buf);
unsigned int  rsp_dnls(StatFrame** sts_frm, InfoFrame** inf_frm, DChains* dchns, Lattice* hltc, unsigned char **buf);

unsigned int rsp_fiid(StatFrame** sts_frm, InfoFrame** inf_frm, DChains* dchns, Lattice* hltc, unsigned  char**buf);
unsigned int rsp_frdn(StatFrame** sts_frm, InfoFrame** inf_frm, DChains* dchns, Lattice* hltc, unsigned  char**buf);
unsigned int rsp_iiii(StatFrame** sts_frm, InfoFrame** inf_frm, DChains* dchns, Lattice* hltc, unsigned  char**buf);
unsigned int rsp_fyld(StatFrame** sts_frm, InfoFrame** inf_frm, DChains* dchns, Lattice* hltc, unsigned  char**buf);


/** Status ops **/

void setSts(StatFrame** sts_frm, LattStts ltcst, unsigned int modr);

void setErr(StatFrame** sts_frm, LattErr ltcerr, unsigned int modr);

void setMdr(StatFrame** sts_frm, unsigned int modr);

void setAct(StatFrame** sts_frm, LattAct lttact, LattStts ltsts, unsigned int modr);

void setRsp(StatFrame** sts_frm, LattReply,unsigned int modr);

void stsReset(StatFrame** sts_frm);

void stsOut(StatFrame** sts_frm);

void serrOut(StatFrame** sts_frm, char* msg);

long stsErno(LattErr ltcerr, StatFrame **sts_frm, int erno, long misc, char *msg, char *function, char *miscdesc);

#endif //TAGFI_LATTICE_CMDS_H

/**
<code>

|1		          |0b1                                  |  2        	|	0b10                              |
*<br>
|4	              |0b100                                |  8        	|   0b1000                            |
*<br>
|16		          |0b10000                              |  32           |   0b100000                          |
*<br>
|64		          |0b1000000                            |  128      	|	0b10000000                        |
*<br>
|256		      |0b100000000                          |  512      	|	0b1000000000                      |
*<br>
|255              |0b11111111                           |  1024         |   0b10000000000                     |
*<br>
|2048    	      |0b100000000000                       |  4096     	|	0b1000000000000                   |
*<br>
|8192    	      |0b10000000000000                     |  16384    	|	0b100000000000000                 |
*<br>
<br>
|32768   	      |0b1000000000000000                   |  65536    	|	0b10000000000000000               |
*<br>
|131072  	      |0b100000000000000000                 |  262144   	|	0b1000000000000000000             |
*<br>
|524288  	      |0b10000000000000000000               |  1048576  	|	0b100000000000000000000           |
*<br>
|2097152 	      |0b1000000000000000000000             |  4194304  	|	0b10000000000000000000000         |
*<br>
|8388608	      |0b100000000000000000000000           |  16777216 	|	0b1000000000000000000000000       |
*<br>
|33554432	      |0b10000000000000000000000000
*<br>
*<br>
|67108864 	      |0b100000000000000000000000000                                                              |
*<br>
|134217728        |0b1000000000000000000000000000                                                             |
*<br>
|268435456	      |0b10000000000000000000000000000                                                           |
*<br>
|536870912        |0b100000000000000000000000000000                                                           |
 *<br>

|1073741824	      |0b1000000000000000000000000000000                                                             |
 *<br>

|2147483647       |0b1111111111111111111111111111111                                                          |
 *
**/


/**
 1: 1 |  2: 2 |  3: 3 |  4: 4 |  5: 5 |  6: 6 |  7: 7 |  8: 8 |  9: 9 |  10: : |  11: ; |  12: < |  13: = |  14: > |
 15: ? |  16: @ |  17: A |  18: B |  19: C |  20: D |  21: E |  22: F |  23: G |  24: H |  25: I |  26: J |  27: K |
 28: L |  29: M |  30: N |  31: O |  32: P |  33: Q |  34: R |  35: S |  36: T |  37: U |  38: V |  39: W |  40: X |
 41: Y |  42: Z |  43: [ |  44: \ |  45: ] |  46: ^ |  47: _ |  48: ` |  49: a |  50: b |  51: c |  52: d |  53: e |
 54: f |  55: g |  56: h |  57: i |  58: j |  59: k |  60: l |  61: m |  62: n |  63: o |  64: p |  65: q |  66: r |
 67: s |  68: t |  69: u |  70: v |  71: w |  72: x |  73: y |  74: z |  75: { |  76: | |  77: } |  78: ~ |
*/

/**
*<verbatim><code>
 * x % 11:
<br>----
<br>2 : 2
<br>4 : 4
<br>8 : 8
<br>16 : 5
<br>32 : 10
<br>64 : 9
<br>128 : 7
<br>256 : 3
<br>512 : 6
<br>1024 : 1
<br>2048 : 2
<br>4096 : 4
<br>8192 : 8
<br>16384 : 5
<br>32768 : 10
<br>65536 : 9

*/