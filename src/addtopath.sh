
ANADIR=$1

export ANADIR

echo " Adding $ANADIR to path"

if [[ $PATH != ?(*:)${ANADIR}/bin?(:*) ]]; then
 export PATH=$PATH:$ANADIR/bin
fi


if [[ $LD_LIBRARY_PATH != ?(*:)${ANADIR}/lib?(:*) ]]; then
 export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$ANADIR/lib
fi


