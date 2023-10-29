//
// Created by ujlm on 10/12/23.
//

#ifndef TAGFI_LATTICE_CMDS_H
#define TAGFI_LATTICE_CMDS_H

#include "consts.h"

typedef reqFlag reqArr;
typedef rspFlag rspArr;

typedef union LatticeStatement {
    rspArr* lttcrsps;
    reqArr* lttcreqs;
} latticeStmnt;


unsigned long exitmsk(unsigned long cmd);
//int parse_req(const unsigned char* req, unsigned int* parse_buf, unsigned char* carr_buf, unsigned int* iarr_buf);
int parse_req(const unsigned char* req, unsigned char* carr_buf, unsigned int* iarr_buf);
int list_cmds();
void init_lttccmd();
void destroy_lttccmd();
unsigned long exitmsk(unsigned long cmd);
void shtdwn_cmd(unsigned int** outp);
void destroy_cmdstructures(unsigned char* buffer,
                           unsigned char* respbuffer,
                           unsigned char* carr,
                           unsigned int* iarr);

int unmask_cmds(unsigned int** cmds,
                unsigned int** leads,
                unsigned int** lens,
                unsigned char*** seqs,
                unsigned char*** arrs,
                int cmdcnt);



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

#endif //TAGFI_LATTICE_CMDS_H
