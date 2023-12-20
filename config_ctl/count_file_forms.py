#!/usr/bin/python3

"""

walk through a filesystem and return the counts of each file extension (last text after final '.' char)

** custom version for TagFi **

Optional parameters:
- ret:True      // return value
- maxlength:7   // set minimum length and extension should be
- dump:False    // write results to file
- topres:0      // return top N results, 0 returns all
- excl_forms:set=None           // exclude any extensions within the set 
- excl_digitonlytext:False      // exclude results that contain only digits

"""
import datetime
import os
import shutil
import sys
import re
from collections import namedtuple
import time
import asyncio

ts = time.time().hex().replace('.', '-').replace('+', '-')


async def examine_remove(pathr: str):
    cnt = 0
    try:
        if re.match('^(fiforms0x1-)(.{12,})[ch]$', pathr):
            print(pathr)
            os.remove(os.path.join('..', 'BKUP', pathr))
            cnt = 1
    except TimeoutError as t:
        print(t.with_traceback(t.__traceback__), file=sys.stderr)
        cnt = -1
    except Exception as ee:
        print(ee.with_traceback(ee.__traceback__), file=sys.stderr)
        cnt = -1
    finally:
        return pathr, cnt


async def trim_backups():
    delcount = 0
    if os.path.exists(os.path.join('..', 'BKUP')):
        try:
            async with asyncio.TaskGroup() as tg:
                for f in os.listdir(os.path.join('..', 'BKUP')):
                    pth, cnt = await tg.create_task(examine_remove(f))
                    if cnt > 0:
                        delcount += 1
                        print(f"!! {pth}\n")
            print(f"{delcount} files deleted\n")

        except Exception as ee:
            print(ee)
            exit(1)

try:
    asyncio.run(trim_backups())
except Exception as e:
    print(e.with_traceback(e.__traceback__), file=sys.stderr)



def main(path: list, maxlength: int = 7, dump: bool = False, topres: int = 0, excl_forms: set = None, *args):
    maxlength = int(maxlength)
    topres = int(topres)
    excludedforms = True if excl_forms is not None and type(excl_forms) == set else False
    formats = {}
    itm = namedtuple('itm', 'form cnt')

    for p in path:
        for r, d, f in os.walk(p):
            for ff in f:
                frm = ff.split('.')[-1]
                frm = re.sub("\W",'',frm)
                formats[frm] = 1 if frm not in formats.keys() else formats[frm] + 1

    res = [itm(i, j) for i, j in zip(formats.keys(), formats.values())
           if j > 1 and len(i) < maxlength
           and not re.match('^[0-9]{1,}$', i)]

    if excludedforms:
        res = [r for r in res if r[0] not in excl_forms]

    res = sorted(res, key=lambda x: getattr(x, 'cnt'), reverse=True)
    res = res[:255]
    formcount = len(res) + 1
    cnt = 0
    flip = True
    topr = True if topres > 0 else False

    if len(res) > 1:
        res = [(r[0], i) for r, i in zip(res, range(1, len(res) + 1))]
        last = res[0]
        #last = res[-1]

    else:
        last = 0

    vars_tobe = (formcount, maxlength)

    shutil.copyfile(f'../fiforms.h', f'../BKUP/fiforms{ts}.h')
    shutil.copyfile(f'../fiforms.c', f'../BKUP/fiforms{ts}.c')


    with open('../fiforms.h', 'w') as f:
        with open('FIFORMSDOTHTEMP', 'r') as ff:
            with open('FIFORMSDOTCTEMPL', 'r') as fff:
                with open('../fiforms.c', 'w') as ffff:

                    for j in [ff.readlines(), fff.readlines()]:
                        for l in j:
                            if re.match('^(###!)$', l):
                                cnt = 1
                                (f if flip else ffff).write('\n')
                                continue
                            if cnt == 1:
                                newline = re.split('###%', l)
                                elem = newline[1].split('^')

                                if int(elem[0]) == 0:
                                    (f if flip else ffff).write(f"{newline[0]} {vars_tobe[int(elem[1])]}\n")

                                elif int(elem[0]) == 1:
                                    if int(elem[1]) == 0:
                                        (f if flip else ffff).writelines(f"\t\t{r[0]} = {r[1]},\n" for r in res)
                                        (f if flip else ffff).write(f"\t\tNONE = 0\n")
                                    elif int(elem[1]) == 1:
                                        ffff.writelines(
                                            f"\t\t\"{r[0]}\"" + (",\n" if r[0] != last[0] else f",\n\t\t\"NONE\"\n\n")
                                            for r in res)
                                cnt = 0
                            else:
                                (f if flip else ffff).write(l)

                        flip = False

    print(f"wrote {len(res)} values\n")


# if sys.argv[0]:
#     args = sys.argv[1:]
#
#     print(args)
# else:
#     args = sys.argv
#
# print(f"> {os.getcwd()}")
#
# main(*args)
