//
// Created by ujlm on 8/6/23.
//

#ifndef TAGFI_CHKMK_DIDMAP_H
#define TAGFI_CHKMK_DIDMAP_H

#include "lattice_works.h"


/**
 *
 *<br>    Dir nodeno begin at 8(0b1000) and are masked to id their parent base
 * <br>  (media: 0b100 doc: 0b010)
 * <br>
 *<br>   MEDIA dirs will be chained LEFT of the head
 *<br>   DOC dirs will be chained RIGHT of the head
 * <br>
 *<br>   nodes are doubly-linked
 *<br>   each node points to their respective fitbl
 *<br>   each node also points to a hash bridge structure, from which
 *<br>   file indexes can be extracted given the directory and file name
 *<br>   if the index is not already known
 *<br>
 *<br>   In this way a file table can be access from a directory node
 *<br>   but not vice-versa. However the filemap IDs are masked
 *<br>   with a number to ID their resident directory.
 *<br>
 *<br>   This goes the same for the hashlattices
 *<br>   which are meant to provide an easy translation
 *<br>   from a file name to an index, as well as well as natural crossing from
 *<br>   directory to file node, especially if accessing from several levels
 *<br>   away or when the files hashno and index are unknown.
 *<br>
 *<br>   Dir nodes are accessed with "vessel" a dir node pointer
 *<br>   that walks up and down the chains. The vessel can switch
 *<br>   between the two chains by crossing over the head node.
 *<br>   The tails nodes do not link anywhere and are pointed to
 *<br>   by the most recently added directory on a given chain
 *<br>   and serve to provide an unambiguous end point.
 **/

/**
 * \verbatim
 *

                   (vessel)
                 R/   *    \L
                 /  (head)  \
                    /  \
     {ftbl}   (media)  (docs)  {ftbl}
      ^    \     /   |      \   /  ^
      |     (dir)  -(*)-  (dir)    |
      |     /   \         /   \    |
      [hltc]  (...)    (...)   [hltc]
                |        |
             (tail)   (tail)

**/

#endif //TAGFI_CHKMK_DIDMAP_H
