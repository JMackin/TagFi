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
