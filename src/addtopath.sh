
if [ -d ${ANADIR} ]; then
 # ANDIR PREVIOUSLY DEFINED, REMOVE FROM $LD_LIBRARY_PATH and from $PATH
  echo " Removing previous ANADIR=${ANADIR} from path:"
  export PATH=`echo $PATH | sed -e 's#:'"${ANADIR}"'/bin##'`
  export LD_LIBRARY_PATH=`echo $LD_LIBRARY_PATH | sed -e 's#:'"${ANADIR}"'/lib##'`
  echo "PATH ::" $PATH
fi

ANADIR=$1

export ANADIR

echo " Adding $ANADIR to path"

if [[ $PATH != ?(*:)${ANADIR}/bin?(:*) ]]; then
 export PATH=$PATH:$ANADIR/bin
fi


if [[ $LD_LIBRARY_PATH != ?(*:)${ANADIR}/lib?(:*) ]]; then
 export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$ANADIR/lib
fi


