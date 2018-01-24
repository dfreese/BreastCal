#!/bin/bash

# https://stackoverflow.com/a/246128/2465202
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

ANA_BIN="${ANA_DIR}/bin"
if [ -n "${ANA_BIN}" ]; then
    if [ -d ${ANA_BIN} ]; then
        # remove the previously defined ANADIR/bin and it's leading ':'
        PATH=`echo $PATH | sed -e 's#:'"${ANA_BIN}"'##g'`
        # remove the previously defined ANADIR/bin without a leading ':'
        # couldn't get a \? escape on the : to work for some reason.
        PATH=`echo $PATH | sed -e 's#'"${ANA_BIN}"'##g'`
    fi
fi

ANADIR=$DIR
ANA_BIN="${ANA_DIR}/bin"

if [ -n "$PATH" ]; then
    PATH=$PATH:$ANA_BIN
else
    PATH=$ANA_BIN
fi

export ANADIR
