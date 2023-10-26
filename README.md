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
        one (InfoFrame) pointing to the other (Cmd_Seq)
    >> InfoFrame holds info on the request for response determination
        Cmd_Seq points to the request array and the array of parsed flags,
        and holds the unparsed lead plus some metadata.

    Output:

          [InfoFrame]->*[CMD_Seq struct]
               /           \
    - Request Meta    - MetaData (flag-count, array length, id)
    - Sequence size   - Request Lead (OR'd flags as single int)
    - Operations      - Parsed flags (LttcFlags*)
    - Cmd qualifiers  - Request array buffer (uniArr*)
    


    Response determination

    Input:
        InfoFrame -> Mapped operation triggers, an array of 4 integers: 

        { Travel Op, File op, Dir op, System ops, Qualifiers } 


    >> Triggers are mapped to groups of LattReplies, with categories
        corresponding to their triggers. 
        These LattReplies are enums corresponding to indexes in an array of functions
        and represent the item of response.
    >> Qualifiers are values paired with the units of reply to change the nature of their behaviour
    >> Notably a Dir op with a MODIF flag makes it a Travel op, which has similiar output but different action.
        

        Trigger -> LattReplies -> Qualified action


                
        Travel           |  File                |Dir              | Sys
        |                |  |                   ||                | |
        Dir/Mod          |  File                |Dir              | Logic/Info/Mod 
        -------------       -------------       ---------------     -----------------
        DIRID+MODIF      |   FILID              | DIRID           | ERRCD                          
        = Change dir     |   = Return fiID      | = ID for given  | = LattErr frame
        CWDIR+MODF       |   DNODE              | CWDIR           | OINFO    
        = Return to Head |   = Resident DirNode | = ID for CWD    | = Info for given              
        DCHNS+MODF       |   OBYLD              | DCHNS           | STATS         
        = switch Base    |   = Yield object     | = DirNode IDs   | = Status frame
        DNLST+MODF       |   OBJNM              | DNLST           | *+MODIF/SCCSS
        = cd to given    |   = Return filename  | = List content  | = Meta info/etc.   
         file's loc.     |                      | for given       |              
            
        ----- ----- ----- ----- ----- ----- ----- ----- -----        

         DIRID+MODIF   |  1    ||  DCHNS+MODF    |  9  
         FILID         |  2    ||  OBYLD         |  10 
         DIRID         |  3    ||  DCHNS         |  11 
         ERRCD         |  4    ||  STATS         |  12 
         CWDIR+MODF    |  5    ||  DNLST+MODF    |  13 
         DNODE         |  6    ||  OBJNM         |  14 
         CWDIR         |  7    ||  DNLST         |  15
         OINFO         |  8    ||    
       
       