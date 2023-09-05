#!/usr/bin/bash
#- ret:True      // return value
#- minlength:7   // set minimum length and extension should be
#- dump:False    // write results to file
#- topres:0      // return top N results, 0 returns all
#- excl_forms:set=None           // exclude any extensions within the set
#- excl_digitonlytext:False      // exclude results that contain only digits

if [[ -z "$1" ]]; then
  EXPATH="/home/ujlm/Tech/"
else
  EXPATH=$1
fi

./count_file_forms.py "$EXPATH" 7 True 255 "None" True False
