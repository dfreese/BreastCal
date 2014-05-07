
if [ ! -e $1 ]; then
 echo " $1 does not exist"
 exit -1
fi

cat $1  | grep C0 | awk -v SPATTRESH=5 -v HIGAINC=8 -v LOWGAIN=3 '{if (($4<SPATTRESH)||($6<SPATTRESH)||($8<SPATTRESH)||($10<SPATTRESH)||($12<LOWGAINC)||($14<HIGAINC)||($16<LOWGAIN)) {print $0}}' 


# note to print nicely pipe this to ::  a2ps  --font-size=9pts  --columns 1