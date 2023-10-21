//
// Created by ujlm on 10/12/23.
//

#ifndef TAGFI_LATTICE_CMDS_H
#define TAGFI_LATTICE_CMDS_H

#define ENDBYTES 536870919


/*
 *  // False
 *  FFF = 0,
 *  // True
 *  TTT = 1,
 *  // default argument for previous cmd
 *  DFLT = 2,
 *  // Int that follows is the length of an int array that will follow thereafter.
 *  NARR = 4,
 *  // Int that follows is the length of a char array that will follow thereafter
 *  GCSQ = 8,
 *  // Give current vessel location when next byte is zero, yield file table otherwise.
 *  VESL = 16,
 *  // Goto dirNode of following name, home if next byte is zero
 *  GOTO = 32,
 *  // List dirnode contents of following name. Current dir if next byte is zero.
 *  LIST = 64,
 *  // Return file id for filename, return did for CWD if next byte is zero.
 *  FIID = 128,
 *  // Yield file of following filename, get filename for given id if FIID is set.
 *  FINM  = 256,
 *  // Produce info assoc. with the following ID code.
 *  INFO = 512,
 *  // Save this sequence
 *  SAVE  = 4096,
 *  // Carry byte
 *  LEAD = 536870912,
 *  // End sequence and masking byte
 *  END = 2147483647
 *
 * */
typedef enum ReqFlag {
    // False
    FFF = 0,
    // True
    TTT = 1,
    // default argument for previous cmd
    DFLT = 2,
    // Int that follows is the length of an int array that will follow thereafter.
    NARR = 4,
    // Int that follows is the length of a char array that will follow thereafter
    GCSQ = 8,
    // Give current vessel location when next byte is zero, yield file table otherwise.
    VESL = 16,
    // Goto dirNode of following name, home if next byte is zero
    GOTO = 32,
    // List dirnode contents of following name. Current dir if next byte is zero.
    LIST = 64,
    // Return file id for filename, return did for CWD if next byte is zero.
    FIID = 128,
    // Yield file of following filename, get filename for given id if FIID is set.
    FINM = 256,
    // Produce info assoc. with the following ID code.
    INFO = 512,
    // Save this sequence
    SAVE = 4096,
    // Carry byte
    LEAD = 536870912,
    // End sequence / Masking byte
    END = 2147483647
} ReqFlag;

/*
 * // False
 * FFFF = 0,
 * // Number of responses to follow
 * NRSP = 1,
 * // The following is the ID code for the info that follows thereafter
 * RARR = 2,
 * // What follows is the length of an array that follows thereafter
 * RLEN = 4,
 * // Arr that follows is a status frame
 * STAS = 8,
 * // Data type of next response
 * CODE = 16,
 * // Next response
 * CINT = 32,
 *
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


typedef ReqFlag* ReqArr;
typedef RspFlag* RspArr;

typedef unsigned char* cmdKey;

typedef struct LatticeCommand {
    RspArr rsps;
    ReqArr reqs;
    int n_req;
} latticeCmd;

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

/*
 *
 *  Frame:
 *
 *      [ ID | cmd lead | 0: request or 1: response | flags arr |  c/i arr | flag count | arr length ]
 *
 *  Sequence:
 *
 *      [cmd-lead] -> [array-length] -> [array] -> [END-flag]
 */
 /*
 * -  cmd-lead      = LEAD-flag ORd with cmd flags. Can be extracted by ANDing with END-flag.
 * -  array-length  = number of discrete items (chars or ints) in the arr. 0 if no array in the request.
 * -  array         = type 0 for no array, 1 for char, 2 for int. Signaled with NARR(int) and GCSQ(char) flags.
 * -  END-Flag      = will trigger a malformed request error if not in expected position.
 *
 * */
typedef struct Cmd_Seq {
    unsigned long seq_id;
    unsigned int lead;
    unsigned int type;
    unsigned int flg_cnt;
    unsigned int arr_len;
    LttcFlags* flags;
    uniArr *arr;
}Cmd_Seq;

typedef struct SeqMap{
    unsigned long seq_id;
    Cmd_Seq** cmd_seq;
}SeqMap;

/*
 *
 * Hash table storing common sequences
 * to compare against or pull from.
 *
 * [ max table size | cmd hash key | sequence arr ]
 * */
typedef struct Seq_Tbl{
    unsigned long mx_sz;
    unsigned int cnt;
    cmdKey cmd_key;
    SeqMap* seq_map;
} Seq_Tbl;


int list_cmds();

void init_lttccmd();

void destroy_lttccmd();

void init_cmdseq(Cmd_Seq** cmdSeq, unsigned int arrsize, unsigned int type);

unsigned int init_seqtbl(Seq_Tbl** seq_tbl, unsigned long mx_sz);

unsigned int build_lead(const unsigned int* cmd_flags, unsigned int flg_cnt);

unsigned long save_seq(Cmd_Seq* cmd_seq, Seq_Tbl** seq_tbl, int cnfdir_fd);

void destroy_cmdstructures(unsigned char* buffer,
                           unsigned char* respbuffer,
                           unsigned char* carr,
                           unsigned int* iarr,
                           Seq_Tbl* sqTbl);

unsigned int serial_seq(unsigned char* seq_out,
                        Cmd_Seq** cmd_seq);

int unmask_cmds(unsigned int** cmds,
                unsigned int** leads,
                unsigned int** lens,
                unsigned char*** seqs,
                unsigned char*** arrs,
                int cmdcnt);

#endif //TAGFI_LATTICE_CMDS_H

/*
 *
 *
1		        0b1
2       		0b10
4		        0b100
8       		0b1000
16		        0b10000
32      		0b100000
64		        0b1000000
128     		0b10000000
255             0b11111111

256		        0b100000000

512     		0b1000000000
1024    		0b10000000000
2048    		0b100000000000
4096    		0b1000000000000
8192    		0b10000000000000
16384   		0b100000000000000
32768   		0b1000000000000000
65536   		0b10000000000000000

131072  		0b100000000000000000
262144  		0b1000000000000000000
524288  		0b10000000000000000000
1048576 		0b100000000000000000000
2097152 		0b1000000000000000000000
4194304 		0b10000000000000000000000
8388608	    	0b100000000000000000000000
16777216		0b1000000000000000000000000
33554432		0b10000000000000000000000000
67108864		0b100000000000000000000000000
134217728		0b1000000000000000000000000000
268435456		0b10000000000000000000000000000
536870912		0b100000000000000000000000000000

1073741824		0b1000000000000000000000000000000
2147483647      0b1111111111111111111111111111111

 *
 *
 *

1: 1
2: 2
3: 3
4: 4
5: 5
6: 6
7: 7
8: 8
9: 9
10: :
11: ;
12: <
13: =
14: >
15: ?
16: @
17: A
18: B
19: C
20: D
21: E
22: F
23: G
24: H
25: I
26: J
27: K
28: L
29: M
30: N
31: O
32: P
33: Q
34: R
35: S
36: T
37: U
38: V
39: W
40: X
41: Y
42: Z
43: [
44: \
45: ]
46: ^
47: _
48: `
49: a
50: b
51: c
52: d
53: e
54: f
55: g
56: h
57: i
58: j
59: k
60: l
61: m
62: n
63: o
64: p
65: q
66: r
67: s
68: t
69: u
70: v
71: w
72: x
73: y
74: z
75: {
76: |
77: }
78: ~

**/