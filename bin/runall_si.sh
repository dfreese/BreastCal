# ls PT_PED*out   > pedfiles
# ls PT_DAQ*out   > daqfiles
# paste daqfiles pedfiles > files 

function timing ()
 { START=$1;
   TIME=$(( `date +%s` - $START ));
   echo $TIME;
}

STARTTIME=`date +%s`

# sh ./runall.sh testje
while read data ped ; do 
#  POS=`echo $data | cut -d u -f 1 | cut -d _ -f 5`
#  echo $POS
  echo "${CODEVERSION}decoder -f $data -pedfile $ped.ped -uv -t -400 ; "
  ${CODEVERSION}decoder -pedfile $ped.ped -f $data -uv -t -400  ; 
  echo -n " decoding done. Time:: "
  timing $STARTTIME 
 done < $1; 
 