"""
Config File Format:
    -------------------
    -------------------

    Byte Markers:
    --------
    1f001f
        - Lead header
        - prefaces item count

    7f0b7f
        - Lengths-segment head
        - prefaces list of item lengths

    bddbbd
        - Lengths-segment tail
        - iteration terminator when reading in config
        - conjoins lengths and items segments

    711117
        - Items-segment head/tail
        - Encapsulates the items list.

    d1cc1d
        - config string terminator

    00
        - list item deliminator

-------------------

 1f001f
     Num of items
    ------
 7f0b7f
     ItmLenA
     0x00
     ItmLenB
     0x00
     ItmLenZ
     0x00
 bddbbd
    ------
 711117
     ItemA
     0x00
     ItemB
     0x00
     ItemZ
     0x00
 711117
    ------
 d1cc1d

---------------------

    ** Note: Dash marks, new lines, and whitespaces are used only for clarity here.
             When stored in a file the entirety of the config is
             stored as a single byte string, segmented by the byte markers.

             Ex. A config that stores 3 text strings each 5 char's long:

                 1f001f37f0b7f500500500bddbbd711117textA00textB00textC00711117d1cc1d

    ---------------------

"""
import enum
import os
import sys
import re
import count_file_forms

dirnodesconfpath = '/home/ujlm/CLionProjects/TagFI/config/dirnodes'
confdirpath = '/home/ujlm/CLionProjects/TagFI/config'

excl_set = {'asm', 'index', 'h', 'HEAD'}

# fout.write(bytes.fromhex('1f001f'))
# fout.write(int(0).to_bytes())
# fout.write(bytes.fromhex('7f0b7fbddbbd711117'))
# fout.write(bytes.fromhex('00'))
# fout.write(bytes.fromhex('711117'))
# fout.write(bytes.fromhex('d1cc1d'))
#

# 0 "lead", 1 "lens_head", 2 "lens_tail", 3 "items_flank", 4 "terminator", 5 "delim"
marks = ("lead", "lens_head", "lens_tail", "items_flank", "terminator", "delim")


class Marks(enum.Enum):
    lead = 0x1f10081f
    lens_head = 0x7f40207f
    lens_tail = 0x1ff601ff
    items_flank = 0xffef0b03
    terminator = 0x31a81431
    delim = 0x00
    int_zero = 0x00000000

count = 0  # To be re-assigned later to item count
count_pos = 4  # Positon after lead marker
count_end = 7  # Position before lengths head
first_len_pos = 12  # Positon after lengths head
marker_width = 4  # Each marker (except delim) is 4 bytes

end_len_pos = lambda len_pos_start: len_pos_start + 3  # Position before deliminator
subseq_len_pos = lambda last_len_start: last_len_start + 5  # start of next length value

last_len_pos = first_len_pos + ((count - 1) * 5)  # Each length + deliminator is 5 bytes long

# Position following the lengths-segment tail and the first item flank marker.
first_value_pos = subseq_len_pos(last_len_pos) + (2 * marker_width)

# len of item values segment is the sum(len(item)+1 for each item)

# Template
blank_temp_marks_seq = (Marks.lead, Marks.int_zero, Marks.int_zero, Marks.int_zero, Marks.int_zero,  # Item count
                        Marks.lens_head, Marks.delim, Marks.lens_tail,  # Item lengths
                        Marks.items_flank, Marks.delim, Marks.items_flank,  # Item values
                        Marks.terminator)


def ins_bmark(mark: Marks):
    return (lambda m: m.value.to_bytes((4 if mark.name != "delim" else 1)))(mark)


def fo_write(mark: Marks, fout):
    fout.write(ins_bmark(mark))


def add_dirnodes():
    print("Enter filepaths to be added and q when done..\n")
    paths = []
    while True:
        npth = input("Enter filepath > ")
        if re.fullmatch("^q\s*", npth) or len(paths) > 62:
            break
        else:
            if os.path.exists(npth) and os.path.isdir(npth):
                paths.append((npth, len(npth)))
            else:
                print("Invalid path entered\n", file=sys.stderr)
                continue

    eval_res = eval_dirnodesconf(paths)
    if type(eval_res) == tuple:
        update_dirnodesconf(eval_res)
    elif eval_res == -1:
        quit(-1)
    elif eval_res == 0:
        return


def blank_conf_templ(finame, config_dirpath: str = None):
    if config_dirpath is None:
        config_dirpath = confdirpath
        if not os.path.exists(config_dirpath):
            os.mkdir('./config')
            config_dirpath = './config'
    else:
        if not os.path.exists(config_dirpath):
            config_dirpath = confdirpath
            if not os.path.exists(config_dirpath):
                os.mkdir('./config')
                config_dirpath = './config'

    # Write empty config file, variables init'd to 0
    with open(os.path.join(config_dirpath, finame), 'wb') as fout:
        for ea in blank_temp_marks_seq:
            print(f"{ea.value} : {hex(ea.value)}")
            fo_write(ea, fout)


# Takes a 2-position tuple containing 2 lists:
# the item values in [0] and the item lengths in [1]
def update_dirnodesconf(pathlist):
    with open(dirnodesconfpath, 'rb+') as fout:

        # Start / Item count
        fo_write(Marks.lead, fout)
        fout.write(int(len(pathlist[0])).to_bytes(4))
        print(f"COUNT: {int(len(pathlist[0])).to_bytes(4)}")

        # Item Lengths
        fo_write(Marks.lens_head, fout)
        for i in pathlist[1]:
            if i != bytes.fromhex(''):
                fout.write(i)
                fo_write(Marks.delim, fout)
                print(f"LEN: {i}")
        fo_write(Marks.lens_tail, fout)

        # Item Values
        fo_write(Marks.items_flank, fout)
        for i in pathlist[0]:
            if i != bytes.fromhex(''):
                fout.write(str.encode(i))
                fo_write(Marks.delim, fout)
                print(f"VAL: {i}")
        fo_write(Marks.items_flank, fout)

        # End
        fo_write(Marks.terminator, fout)


def eval_dirnodesconf(dirnodes: list = None):
    with open(dirnodesconfpath, 'rb+') as fout:
        rin = fout.read()
        #  Check lead and lengths-segment-head byte marks
        #    [lead - 0:4 , count - 4:8, len_head - 8:12]
        #
        if int.from_bytes(rin[0:4]) != Marks.lead.value or int.from_bytes(rin[8:12]) != Marks.lens_head.value:
            print(f"Invalid conf formatting:\n\t"
                  f" lead : count : len_head \n\t"
                  f" [{rin[0:4]}] : {rin[count_pos:count_end + 1]} : [{rin[8:12]}]\n\n",
                  f" [{int.from_bytes(rin[0:4])}] : {int.from_bytes(rin[count_pos:count_end + 1])} : [{int.from_bytes(rin[8:12])}]\n\n",
                  f" [{ Marks.lead.value}] : {int.from_bytes(rin[count_pos:count_end + 1])} : [{ Marks.lens_head.value}]\n\n",
                  file=sys.stderr)
            return -1
        else:
            len_list = []  # List of item lengths
            fi_list = []  # List of item values

            # Count stored as 4-byte int in positions 4-7
            node_cnt = int.from_bytes(rin[count_pos:count_end + 1])
            print(f"COUNT: {node_cnt}")

            if dirnodes is not None:
                # TODO: Adjust node max
                if node_cnt + len(dirnodes) > 62:
                    print(f"Node max exceeded. Current count: {len(dirnodes)}\n")
                    return -1

            # Begin iterating from first value in the 'lengths' segment (immed. after the lens_head marker)
            i = first_len_pos
            j = i
            # Stop when the len-segment tail is hit.
            while int.from_bytes(rin[j: (j + marker_width)]) != Marks.lens_tail.value:
                j += 1
                if j > 10000:
                    print('Invalid conf formatting: length list end mark missedn\n')
                    break
                    # TODO: Improve this check

            # Parse list of items lengths by splitting on the delim
            len_list = rin[i:j].split(ins_bmark(Marks.delim))
            # Filter out null values
            len_list = [i for i in len_list if i != bytes.fromhex('')]

            if int.from_bytes(rin[j:j + marker_width]) != Marks.lens_tail.value:

                print(f"Invalid conf formatting: End rin mark [{rin[j:j + marker_width]}]\n", file=sys.stderr)
                return -1

            # Check for beginning of values segment by testing for item-flank
            head_end = j
            if int.from_bytes(rin[head_end + marker_width:head_end + (2 * marker_width)]) != Marks.items_flank.value:
                print(f"Invalid conf formatting: Item-value start:"
                      f" [{rin[head_end + marker_width:head_end + (2 * marker_width)]}]\n"
                      f" [{Marks.items_flank.value}]\n",
                      file=sys.stderr)
                return -1

            cursr = head_end + (2 * marker_width)
            path_bldr = ""  # Buffer to hold each item value

        #
        # END ELSE-BRANCH

        # Begin evaluating item values segment
        # Iterate until the second item flank is hit
        while int.from_bytes(rin[cursr:cursr + marker_width]) != Marks.items_flank.value:
            if cursr > 10000:
                print("LIMIT HIT")
                # TODO: Improve this check
                break

            chr_in = rin[cursr]
            if chr_in == ins_bmark(Marks.delim) and len(path_bldr) > 1:
                # If a null byte is hit (a delimiter) it's assumed the end of the value has been reached,
                # and it's appended to the results list
                fi_list.append(path_bldr)
                path_bldr = ""
                cursr += 1
                continue
            else:
                # Each item value is constructed a character at a time until reaching the delimiter
                print(chr(chr_in))
                path_bldr += chr(chr_in)
                cursr += 1

        # Filter out Null item values
        fi_list = [i for i in fi_list if i != '']


    # If a list of dirnodes is passed into this function they are added to the existing list and
        # the config file is updated.
        if dirnodes is not None:
            for i in dirnodes:
                fi_list.append(i)
                len_list.append(len(i).to_bytes())

            update_dirnodesconf((fi_list, len_list))

            count_file_forms.main(fi_list, 7, True, 255, excl_set)
        else:

            print(fi_list)
            print(len_list)


blank_conf_templ(dirnodesconfpath)
#
eval_dirnodesconf(['/srv/sandpit/Code', '/srv/sandpit/Vault','/srv/sandpit/Tech', '/srv/sandpit/Art'])
eval_dirnodesconf()
