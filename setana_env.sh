#!/bin/bash

# https://stackoverflow.com/a/246128/2465202
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

ANABIN="${ANADIR}/bin"
if [ -n "${ANADIR}" ]; then
    if [ -d ${ANABIN} ]; then
        # remove the previously defined ANADIR/bin and it's leading ':'
        PATH=`echo $PATH | sed -e 's#:'"${ANABIN}"'##g'`
        # remove the previously defined ANADIR/bin without a leading ':'
        # couldn't get a \? escape on the : to work for some reason.
        PATH=`echo $PATH | sed -e 's#'"${ANABIN}"'##g'`
    fi
fi

ANADIR=$DIR
ANABIN="${ANADIR}/bin"

if [ -n "$PATH" ]; then
    PATH=$PATH:$ANABIN
else
    PATH=$ANABIN
fi

export ANADIR
export PATH
