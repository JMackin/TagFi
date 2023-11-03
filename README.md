Request Sequence Structure:
----------------------------------
    * A  [cmd-lead]      
    * B  [array-length]
    * C  [array]
    * D  [END-Flag]

-  #### _(uint)_       [ cmd-lead ]
   > LEAD-flag ORd with cmd flags. Can be extracted by ANDing with END-flag.
- #### _(uint)_       [ array-length ]
   > number of discrete items (chars or ints) in the arr. 0 if no array in the request.
- #### _(uint/uchar)_ [ array ]
   > type 0 for no array, 1 for char, 2 for int. Signaled with NARR(int) and GCSQ(char) flags.
- #### _(uint)_       [ END-flag ]
   > will trigger a malformed request error if not in expected position.

--------------------------------- 


--------------------------------- 

Request sequences:
----------------------------------

#### Shutdown
>   _reply_: status code (4*uint)

    * A - [LEAD | TTT | NARR]
    * B - [1]
    * C - [END]
    * D - [END]


#### Ping 
>  _reply_: true (1*uchar)

    * A - [LEAD]
    * B - [0]
    * C - [0]
    * D - [END]


#### Status Request:
>   _reply_: status code (4*uint)


    * A - [LEAD | INFO | DFLT | NARR]
    * B - [1]
    * C - [INFO]
    * D - [END]


--------------------------------- 


--------------------------------- 

Exchange Flow:
--------------------------------- 

    Request receipt 
    
    >> Request is divided into it component parts; a lead and an array.
    >> The lead is parsed into an array of flags via recursive bitmasking
    >> Two structs are generated from the process, 
        one (ExchFrame) pointing to the other (Cmd_Seq)
    >> ExchFrame holds info on the request for response determination
        Cmd_Seq points to the request array and the array of parsed flags,
        and holds the unparsed lead plus some metadata.

    Output:

          [ExchFrame] -> *[CMD_Seq struct]
               /             \
    - Request Meta    - MetaData (flag-count, array length, id)
    - Sequence size   - Request Lead (OR'd flags as single int)
    - Operations      - Parsed flags (LttcFlags*)
    - Cmd qualifiers  - Request array buffer (uniArr*)
    


    Response determination

    Input:
        ExchFrame -> Mapped operation triggers, an array of 4 integers: 

        { Travel Op, File op, Dir op, System ops, Qualifiers } 

    >> Triggers are mapped to groups of LattReplies, with categories
        corresponding to their triggers. 
        These LattReplies are enums corresponding to indexes in an array of functions
        and represent the item of response.
    >> Qualifiers are values paired with other units of reply to change the nature of their behaviour
    >> Notably a Dir op with a DNODE flag makes it a Travel op, which has similiar output but different action.
        

        Trigger -> LattReplies -> Qualified action

                
        Travel           |  File                | Dir             | Sys
        |                |  |                   | |               | |
        Dir/Mod          |  File                | Dir             | Info/Mod 
        -------------     -------------          ---------------    -----------------
        DIRIDQ      |   FILID              | DIRID           | ERRCD                          
        = Change dir     |   = Return fiID      | = ID for given  | = LattErr frame
        DCHNSQ       |   DNODE              | DSRCH           | OINFO    
        = Return to Head |   = Resident DirNode | = Search for RD | = Info for given              
        DCHNS+DNODE       |   OBYLD              | DCHNS           | STATS         
        = switch Base    |   = Yield object     | = DirNode IDs   | = Status frame
        DNLST+DNODE       |   IIIII              | DNLST           | *+MODIF/SCCSS
        = cd to given    |   = X  | = List content  | = Meta info/etc.   
         file's loc.     |                      | for given       |              
    
 
    - first bit set True = D/T
        -- 2nd bit -> True = T else = D
    - first bit set False = F/S
        -- 3rd & 4th bits set True = I else S

    D: 1,5,9,13           D: [ DIRID:01:'0b0001' | JJJJJ:05:'0b0101' | DCHNS:09:'0b1001' | DNLST:13:'0b1101']
    T: 3,7,11,15          T: [*DIRID:03:'0b0011' |*DSRCH:07:'0b0111' |*DCHNS:11:'0b1011' |*VVVVV:15:'0b1111']
    F: 0,2,4,8            F: [ FILID:00:'0b0000' | DNODE:02:'0b0010' | OBYLD:04:'0b0100' | IIIII:08:'0b1000']
    S/I: 6,10,12,14       S: ['UNDEF:06:'0b0110' | ERRCD:10:'0b1010' | OINFO:12:'0b1100' | STATS:14:'0b1110']


    Corresponding Request Flags and returned bits.  

    Travel: 3 
     GOTO = 128  - 1   : DIRIDQ - 3   : 0b0011
     SRCH = 256  - 2   : DSRCHQ - 7   : 0b0111
     HEAD = 512  - 4   : DCHNSQ - 11  : 0b1011
     VVVV = 1024 - 8   : VVVVVV - 15  : 0b1111 (Empty)
    
    File: 0 
     FIID = 2048  - 1   : FILID - 0   : 0b0000
     FRES = 4096  - 2   : DNODE - 2   : 0b0010
     YILD = 8192  - 4   : OBYLD - 4   : 0b0100
     IIII = 16384 - 8   : IIIII - 8   : 0b1000 (Empty) 
  
    Dir: 1
     DCWD = 32768  - 1   : DIRID - 1  : 0b0001
     JJJJ = 65536  - 2   : JJJJJ - 5  : 0b0101 (Empty)
     LDCH = 131072 - 4   : DCHNS - 9  : 0b1001
     LIST = 262144 - 8   : DNLST - 13 : 0b1101

    Sys/Info: 2 (not reflected in mask)
     INFO = 524288  - 1  : OINFO - 12 / STATS - 14 : 0b1100 / 0b1110
     SAVE = 1048576 - 2  : UNDEF - 6 : 0b0110
     EXIT = 2097152 - 4  : UNDEF - 6 : 0b0110
     UUUU = 4194304 - 8  : UNDEF - 6 : 0b0110 (Empty)
