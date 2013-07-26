# ls PT_PED*out   > pedfiles
# ls PT_DAQ*out   > daqfiles
# paste daqfiles pedfiles > files 

# sh ./runall.sh testje
while read data ped ; do 
  POS=`echo $data | cut -d u -f 1 | cut -d _ -f 5`
  echo $POS
  echo "/home/miil/MODULE_ANA/ANA_V5/DEV/decode -f $data.dat -pedfile $ped.ped -uv -t -800 -pos $POS; "
  /home/miil/MODULE_ANA/ANA_V5/DEV/decode -pedfile $ped.ped -f $data -uv -t -800 -pos $POS ; 
 done < $1; 
