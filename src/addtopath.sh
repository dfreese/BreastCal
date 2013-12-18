
CURDIR=$1

echo " Adding $CURDIR to path"

if [[ $PATH != ?(*:)${CURDIR}/bin?(:*) ]]; then
 export PATH=$PATH:$CURDIR/bin
fi


if [[ $LD_LIBRARY_PATH != ?(*:)${CURDIR}/lib?(:*) ]]; then
 export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$CURDIR/lib
fi


