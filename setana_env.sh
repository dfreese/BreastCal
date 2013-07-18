
CURDIR=`pwd`

if [[ $PATH != ?(*:)$CURDIR/bin?(:*) ]]; then
 export PATH=$PATH:$CURDIR/bin
fi


if [[ $LD_LIBRARY_PATH != ?(*:)$CURDIR/lib?(:*) ]]
 export LD_LIBRARY_PATH=$LD_LIBRARY_PATH/lib
fi


