#!/bin/bash

##############################################################################################################

function mkfolder ()
{
  FOLDER=$1;
  if [ ! -d ${FOLDER} ] ;then  mkdir ${FOLDER}; fi
}
##############################################################################################################

##############################################################################################################

function timing ()
 { START=$1;
   TIME=$(( `date +%s` - ${START} ));
   echo $TIME;
}

##############################################################################################################
##############################################################################################################

function pedconv ()
 {  decoder -p -f $1 >> $2 ;  
    check ${?} "converting pedfile ${1}"; 
#    sleep 10
  }

##############################################################################################################
##############################################################################################################

function waitsome ()
{
 # waits untill all jobs are done from a list submitted as $1;
 
local jobs=${2:-0}
#if [ -z "$2" ] ; then
#  jobs=1;
# else
#  jobs=$2;
# fi

 log " JOBS TO WAIT FOR :: " $jobs

 local finished=0;


while [ $finished -lt $jobs ] ; do
  local index=0;
  for ii in ${pids[@]}; do
#  echo "job ${ii} index $index array length : ${#pids[@]} . Array: ${pids[*]}" ;
  if [ ! -d /proc/${ii} ]; then 
#           echo "${ii} done ! ( index $index ) " ; 
           (( finished++ )); 
           unset pids[$index];
           pids=("${pids[@]}")
#          pids=${pids[@]//${ii}/}
#           echo " after unsetting: ${pids[*]}"
           continue
           fi;
  (( index++ )); 
  done;
#  echo " Finished jobs :: " $finished " ( pids = " ${pids[@]} " )" 
  sleep 2
done;
}

##############################################################################################################

function waitall ()
{
 # waits untill all jobs are done from a list submitted as $1;
# pids=$1;
 log " WAITALL :: NEED TO WAIT FOR ${#pids[@]} jobs to finish with IDs :: ${pids[*]}"
 waitsome $1 ${#pids[@]} 
}

##############################################################################################################
##############################################################################################################

function check ()
{
    RET=$1;
#    echo " RETURN CODE :: ${RET} "
    if [ ${RET} != 0 ] ; then  
      echo " =========================================" 
      echo " ............ WARNING ...................." 
      echo " Program failed at $2 ";
      echo " ========================================="
      exit
   fi
   
}

##############################################################################################################

function log ()
{
 if [[ $verbose -eq 1 ]]; then
     echo $@
 fi;
}

##############################################################################################################

function usage()
{
  echo "ana_eth.sh [ -v -d -c -s -m -t -a -h -C [] ]"
  echo " with: -v: verbose"
  echo "       -d: decode binary data ( Left and Right )"
  echo "       -c: calibrate data ( Left and Right )"
  echo "       -s: time sort Left and Right data"
  echo "       -m: merge Left and Right"
  echo "       -t: time calibrate"
  echo "       -a: do all of the above"
  echo "       -h: display this help"
  echo "       -C [] : number of cores"
}

##############################################################################################################

CORES=2
STARTTIME=`date +%s`
verbose=0
dodecode=0
docalibrate=0
dosort=0
domerge=0
dotimecal=0


while [ $# -gt 0 ];
do
    case "$1" in
	-v) verbose=1;;
	-d) dodecode=1;;
	-c) docalibrate=1;;
	-s) dosort=1;;
	-m) domerge=1;;
	-t) dotimecal=1;;
	-a) dodecode=1;docalibrate=1;dosort=1;domerge=1;dotimecal=1;;
	-h) usage ; exit; break;;
	-C) CORES=$2;shift;;
	*) usage; break;;
    esac
    shift
done;

echo "NUMBER OF CORES:: " $CORES


if [[ $dodecode -eq 1 ]]; then
log "Decoding data"
mkfolder Left
mkfolder Right

for k in Left Right; do
cd $k
# need 'L' and 'R' 
KK=${k:0:1}
if [ -e ../PED*${KK}*dat ]; then mv ../PED*${KK}*dat . ; fi
ls -tr ./PED*dat > pedfiles
#FIXME WILL BE PT***** FOR PT SOURCE DATA
if [ -e ../DAQ*${KK}*dat ]; then mv ../DAQ*${KK}*dat . ; fi
ls -tr ./DAQ*dat > daqfiles

 if [ `wc -l ./pedfiles | awk '{print $1}'` -eq `wc -l ./daqfiles | awk '{print $1}'` ]; then 
  # equal number of pedfiles as datafiles
   paste daqfiles pedfiles  > files
 else 
  # echo "different"; 
  pedcount=0; 
  if [ -e files ] ; then rm files; fi;
  while read data; do 
       echo -n $data" " >> files; 
          if  [ `basename $data .dat | rev | cut -f1 -d'_'` -eq 0   ]; then (( pedcount++ ))  ; fi;  
       PEDFILE=`sed -n "${pedcount}p" ./pedfiles`; 
       echo $PEDFILE  >> files;
  done < ./daqfiles
 
 fi;

 
 # converting binary output to ROOT file format
 for c in `seq 1 100`; do 
 if [ -e pedconv_$c.out ] ; then 
   rm pedconv_$c.out
 fi;
 done;

echo -n " Converting pedfiles @ "
 timing $STARTTIME
 RUNNINGJOBS=0;
 j=0;
 for i in `cat pedfiles`; do 
    if [ $RUNNINGJOBS -lt $CORES ]; then 
    (( j++ ));
 #   echo " SUBMITTING JOB "
    pedconv $i pedconv_$j.out &
    pids+=($!);
    (( RUNNINGJOBS++ ));
    else
#    echo " RUNNINGJOBS : $RUNNINGJOBS"
    waitsome $pids 1
    RUNNINGJOBS=${#pids[@]}
#    echo " PEDCONV LOOP RUNNINGJOBS after waitsome : $RUNNINGJOBS"
    (( j++ )) ;
    pedconv $i pedconv_$j.out &
    pids+=($!);
    (( RUNNINGJOBS++ ));
    fi;
    done;

# pedestal correction, parsing and fine time calculation

    waitall $pids

echo -n " pedestal decoding done @ "
timing $STARTTIME

RUNNINGJOBS=0
c=0;

NOHIT=-50;

  while read data ped ; do 
   PTCHAR=${data:0:1}
   if [ "$PTCHAR" == "PT" ] ; then 
      pos=`echo $data | cut -f1 -d 'u' |  sed 's/.*[^0-9]\([0-9]\+\)[^0-9]*$/\1/'`
   else
      pos=0;
   fi;
 
   if [ $RUNNINGJOBS -lt $CORES ]; then 
    (( c++ ));
    echo -n " SUBMITTING JOB "
    echo "${CODEVERSION}decoder -pedfile $ped.ped -f $data -uv -t -400 -pos $pos -n $NOHIT; "
    ${CODEVERSION}decoder -pedfile $ped.ped -f $data -uv -t -400 -pos $pos -n  $NOHIT > $data.conv.out &
#    pedconv $i pedconv_$j.out &
    pids+=($!);
    (( RUNNINGJOBS++ ));
    else
#    echo " RUNNINGJOBS : $RUNNINGJOBS"
    waitsome $pids 1
    echo -n " SUBMITTING JOB "
    echo "${CODEVERSION}decoder -pedfile $ped.ped -f $data -uv -t -400 -pos $pos -n  $NOHIT; "
    ${CODEVERSION}decoder -pedfile $ped.ped -f $data -uv -t -400 -pos $pos -n  $NOHIT > $data.conv.out &
    pids+=($!);
    (( RUNNINGJOBS++ ));
    RUNNINGJOBS=${#pids[@]}
  #    echo " RUNNINGJOBS after waitsome : $RUNNINGJOBS"
    fi;
 done < files;

 waitall $pids

 echo -n " decoding done. Time:: "
 timing $STARTTIME 
 log " PIDS = ${pids[@]} "


 TIMECHECK=0
 RUNNINGJOBS=0;
 c=0;

#echo " PIDS before calibration : ${#pids[@]} :: ${pids[@]} "

cd ..

done;

fi;

#############################################################################################################

if [[ $docalibrate -eq 1 ]]; then

for k in Left Right; do
cd $k
KK=${k:0:1}

BASE=`ls DAQ*${KK}0*root | head -n 1 | cut -d ${KK} -f 1`${KK}
ls DAQ*${KK}0*root | cut -d _ -f 5 | sort -n | awk -v FBASE=$BASE '{print FBASE"0_"$1}' > filelist
chain_parsed -f filelist -o ${BASE}.root
check ${?} "chain_parsed ${BASE}.root"; 
getfloods -f ${BASE}.root
check ${?} "getfloods -f  ${BASE}.root"; 
#for i in `seq 0 7`; do (anafloods_psf_v2 -f ${BASE}.root -c 0 -l $i &); sleep 1; done;

RUNNINGJOBS=0;
j=0;
 for i in `seq 0 7`; do 
    if [ $RUNNINGJOBS -lt $CORES ]; then 
    (( j++ ));
 #   echo " SUBMITTING JOB "
    anafloods_psf_v2 -f ${BASE}.root -c 0 -l $i &
    pids+=($!);
    (( RUNNINGJOBS++ ));
    else
#    echo " RUNNINGJOBS : $RUNNINGJOBS"
    waitsome $pids 1
    RUNNINGJOBS=${#pids[@]}
#    echo " ANAFLOODS LOOP RUNNINGJOBS after waitsome : $RUNNINGJOBS"
    (( j++ )) ;
     anafloods_psf_v2 -f ${BASE}.root -c 0 -l $i &
    pids+=($!);
    (( RUNNINGJOBS++ ));
    fi;
    done;

mkdir CHIPDATA
mv *peaks.txt ./CHIPDATA
enecal -f ${BASE}.root
check ${?} "enecal -f  ${BASE}.root"; 
calibrate -f ${BASE}.root
check ${?} "calibrate -f  ${BASE}.root"; 
cd ..
done;

fi