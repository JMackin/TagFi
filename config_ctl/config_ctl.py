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

import os
import sys
import re
import count_file_forms


dirnodesconfpath = '/home/ujlm/CLionProjects/TagFI/config/dirnodes'
confdirpath = '/home/ujlm/CLionProjects/TagFI/config'

excl_set = {'asm', 'index', 'h', 'HEAD'}

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

    with open(os.path.join(config_dirpath, finame), 'wb') as fout:
        fout.write(bytes.fromhex('1f001f'))
        fout.write(int(0).to_bytes())
        fout.write(bytes.fromhex('7f0b7fbddbbd711117'))
        fout.write(bytes.fromhex('00'))
        fout.write(bytes.fromhex('711117'))
        fout.write(bytes.fromhex('d1cc1d'))


def update_dirnodesconf(pathlist):
    with open(dirnodesconfpath, 'rb+') as fout:
        fout.write(bytes.fromhex('1f001f'))
        fout.write(int(len(pathlist[0])).to_bytes())
        fout.write(bytes.fromhex('7f0b7f'))

        for i in pathlist[1]:
            if i != bytes.fromhex(''):
                fout.write(bytes(i))
                fout.write(bytes.fromhex('00'))

        fout.write(bytes.fromhex('bddbbd'))
        fout.write(bytes.fromhex('711117'))

        for i in pathlist[0]:
            if i != bytes.fromhex(''):
                fout.write(str.encode(i))
                fout.write(bytes.fromhex('00'))

        fout.write(bytes.fromhex('711117'))
        fout.write(bytes.fromhex('d1cc1d'))


def eval_dirnodesconf(dirnodes: list = None):
    # numentries:lenA:lenB ... 3e1a3eFF1f000
    with open(dirnodesconfpath, 'rb+') as fout:
        rin = fout.read()
        
        if rin[0:3] != bytes.fromhex('1f001f') or rin[4:7] != bytes.fromhex('7f0b7f'):
            print(f"Invalid conf formatting: start marks [{rin[0:3]}] {rin[3]} [{rin[4:7]}]\n", file=sys.stderr)
            return -1
        else:
            len_list = []
            fi_list = []
            node_cnt = rin[3]

            if dirnodes is not None:
                if node_cnt + len(dirnodes) > 62:
                    print(f"Node max exceeded. Current count: {len(dirnodes)}\n")
                    return -1

            i = 7
            j = i
            while rin[j].to_bytes() != bytes.fromhex('bd'):
                j += 1
                if j > 10000:
                    print('Invalid conf formatting: length list end mark missedn\n')
                    break

            len_list = rin[i:j].split(bytes.fromhex('00'))
            len_list = [i for i in len_list if i != bytes.fromhex('')]

            if rin[j:j+3] != bytes.fromhex('bddbbd'):
                print(f"Invalid conf formatting: End rin mark [{rin[j:j+3]}]\n", file=sys.stderr)
                return -1

            # head_end = ((8 + node_cnt * 2)-1)
            head_end = j
            if rin[head_end+3:head_end+6] != bytes.fromhex('711117'):
                print(f"Invalid conf formatting: Path-list start mark [{rin[head_end+3:head_end+6]}]\n", file=sys.stderr)
                return -1

            cursr = head_end+6
            path_bldr = ""
            node_enum = 0

        while rin[cursr:cursr+3] != bytes.fromhex('711117'):
                if cursr > 100000:
                    break
                chr_in = rin[cursr]
                if chr_in == 0:
                    fi_list.append(path_bldr)
                    path_bldr = ""
                    cursr += 1
                    continue
                else:
                    path_bldr += chr(chr_in)
                cursr += 1
        print(fi_list)
        fi_list = [i for i in fi_list if i != '']

        if dirnodes is not None:
            for i in dirnodes:
                fi_list.append(i)
                len_list.append(len(i).to_bytes())
            update_dirnodesconf((fi_list,len_list))
            # ./count_file_forms.py "$EXPATH" 7 True 255 "None" True False
            count_file_forms.main(fi_list, 7, True, 255, excl_set)
        else:

            print(fi_list)
            print(len_list)


blank_conf_templ(dirnodesconfpath)
eval_dirnodesconf(['/srv/sandpit/Code', '/srv/sandpit/Vault'])
#
eval_dirnodesconf(['/home/ujlm/Tech'])
#
# eval_dirnodesconf()



