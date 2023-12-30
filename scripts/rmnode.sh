#!/bin/bash
CWS=$(cat /home/ujlm/CLionProjects/TagFI/config/cw_socket)
if [[ -e $CWS ]]; then
  echo "Removing $CWS"
  rm "$CWS"
fi

