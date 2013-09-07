 
#!/bin/bash

# Script that performs calibration
# note the variables are GLOBAL, in order to force them to be local in functions we should type explicitely "local"

CODEVERSION=/home/miil/MODULE_ANA/ANA_V5/ModuleClass/bin/

# NOTE: if CORES > 4 some extra checks will be needed making sure all data has been processed needed for the next steps. 
CORES=2

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

 echo " JOBS TO WAIT FOR :: " $jobs

 local finished=0;


while [ $finished -lt $jobs ] ; do
  local index=0;
  for ii in ${pids[@]}; do
  echo "job ${ii} index $index array length : ${#pids[@]} . Array: ${pids[*]}" ;
  if [ ! -d /proc/${ii} ]; then 
#           echo "${ii} done ! ( index $index ) " ; 
           (( finished++ )); 
           unset pids[$index];
	   pids=("${pids[@]}")
#	   pids=${pids[@]//${ii}/}
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
 echo " WAITALL :: NEED TO WAIT FOR ${#pids[@]} jobs to finish with IDs :: ${pids[*]}"
 waitsome $1 ${#pids[@]} 
}

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

function mkfolder ()
{
  FOLDER=$1;
  if [ ! -d ${FOLDER} ] ;then  mkdir ${FOLDER}; fi
}

##############################################################################################################

function timing ()
 { START=$1;
   TIME=$(( `date +%s` - $START ));
   echo $TIME;
}

##############################################################################################################

function pedconv ()
 {  ${CODEVERSION}decoder -v -p -f $1 >> $2 ;  
    check ${?} "converting pedfile ${1}"; 
#    sleep 10
  }

##############################################################################################################

function calibrate () {

#skip=1
# if [ $skip -eq 0 ]; then


THISDIR=$1;
mkfolder ${THISDIR}
cd ./${THISDIR};
ls ../Tech/*.root | grep DAQ | grep $1 > parselist_$1;
#chain data together
if [ "$MODE" == "LN" ] ; then
 data=`ls -1 ../DATA/${MODE}_DAQ_BinaryData*${s}${i}*out.parse.root | head -n 1`
 ln -s $data ./${MODE}_DAQ_${DATE}_${s}${i}.root;
else
${CODEVERSION}chain_parsed -f parselist_${1} -o ${MODE}_DAQ_${DATE}_${1}.root;
     check ${?} "chain_psf panel ${1}";
fi;

# get floods

${CODEVERSION}getfloods -f ${MODE}_DAQ_${DATE}_${1}.root;
check ${?} "getfloods panel ${1}";
mkfolder CHIPDATA;
mv ${MODE}_DAQ_${DATE}_${1}.RENA?.floodsapd?.png ./CHIPDATA/
mv ${MODE}_DAQ_${DATE}_${1}.RENA?.Eapd?.png ./CHIPDATA/

# crystal segmentation
${CODEVERSION}anafloods_psf_v2 -f ${MODE}_DAQ_${DATE}_${1}.root; 
check ${?} "anafloods_psf panel ${1}";
mkfolder ./FLOODS;
mv ${MODE}_DAQ*${1}*flood.png ./FLOODS
mkfolder ./QUADRANTS;
mv ${MODE}_DAQ*${1}*quadrants.png ./QUADRANTS

# determine energy calibration parameters
${CODEVERSION}enecal -f ${MODE}_DAQ_${DATE}_${1}.root;
check ${?} "enecal_psf panel ${1}";
## rm ${MODE}_DAQ_${DATE}_${s}${i}.root;
# perform energy calibration
${CODEVERSION}enefit -f ${MODE}_DAQ_${DATE}_${1}.enecal.root;
check ${?} "enefit_psf panel ${1}";
 mkfolder ./ERES;
 mv ${MODE}_DAQ_${DATE}_${1}*glob.png ./ERES/
 rm ${MODE}_DAQ_${DATE}_${1}.enecal.root;
 mv ${MODE}_DAQ_${DATE}_${1}.RENA* ./CHIPDATA
##  rm *PED_BinaryData_*_${s}${i}*out.ped.RENA?; 
${CODEVERSION}fom_ana -f  ${MODE}_DAQ_${DATE}_${1}.cal.root;
check ${?} "fom_ana panel ${1}"

 cd ..;

#fi; #skip

echo "Estimating split level ${1}"
VAR=$1;
LR=`echo "${VAR:0:1}"`;
echo "${CODEVERSION}get_opt_split -f ./${1}/${MODE}_DAQ_${DATE}_${1}.cal.root --${LR}  > events${1}.txt"
${CODEVERSION}get_opt_split -f ./${1}/${MODE}_DAQ_${DATE}_${1}.cal.root --${LR}  > events${1}.txt
check ${?} "get_opt_split panel ${s}${i}";


}

##############################################################################################################

function merging () {
echo "input args :: $0 $1 $2 $3 $4"
VAR=$1
cd ${VAR}
LR=${VAR:0:1}
RB=${VAR:1:1}
echo "${CODEVERSION}merge_4up -f ${MODE}_DAQ_${DATE}_${VAR}.cal.root -rb ${RB} --${LR} -nc ${2} -ts ${3} -lt ${4};   "
${CODEVERSION}merge_4up -f ${MODE}_DAQ_${DATE}_${VAR}.cal.root -rb ${RB} --${LR} -nc ${2} -ts ${3} -lt ${4};   
check $? "merge_4up panel ${LR}${RB}"
cd ..;
}

##############################################################################################################

mergepanel () {
${CODEVERSION}merge_panel -f ${MODE}_DAQ_${DATE}_R0.4up0_part${1}.root -nb 4
check ${?} "merge panel R part ${1}"
${CODEVERSION}merge_panel -f ${MODE}_DAQ_${DATE}_L0.4up0_part${1}.root -nb 4
check ${?} "merge panel L part ${1}"
#combining both sides:
${CODEVERSION}merge_coinc -a -fl ${MODE}_DAQ_${DATE}_part${1}_L.panel.root -fr ${MODE}_DAQ_${DATE}_part${1}_R.panel.root
check $? "mergecal part ${1}"
}

##############################################################################################################


 MODE=$1;
 ARG=$2;
 STARTTIME=`date +%s`

 

 # specifiy SHORT on the command line to only do analysis 
 # for example: sh ~/MODULE_ANA/PSF_V4/cart_ana.sh PT SHORT

 if [ "$MODE" != 'PT' ]; then
  if [ "$MODE" != 'LN' ] ; then
    if [ "$MODE" != 'SI' ]; then
    echo "Please specify mode: \"cart_ana PT SHORT\", \"cart_ana LN SHORT\" or \"cart_ana SI SHORT\" [single mode] "
    exit -1;
    fi;
  fi; 
fi;

if [ "$ARG" != 'SHORT' ]; then

#change to DATA directory if needing to rerun
#if [ -d DATA ]; then
#  cd DATA;
#fi;

if  [ "$MODE" == "LN" ] ; then
 ls PED_BinaryData_*out > pedfiles;
else
 if [ "$MODE" == "SI" ] ; then
 # first count how many files were collected :: 
 if [ -e files ] ; then 
   rm files
 fi;
 for k in L R; do
  for j in 0 1 2 3; do
 pedfile=`ls ./Tech/PED*_${k}${j}_*dat`
 NR=`ls -1 ./Tech/DAQ*${k}${j}_*.dat | wc -l`; (( NR-- )); for i in `seq 0 ${NR}`; do echo `ls ./Tech/DAQ*${k}${j}_${i}.dat` $pedfile >> files; done;
 done;
 done;
 ls ./Tech/PED_*dat > pedfiles;
 ls ./Tech/DAQ_*dat > daqfiles;
else  
 ls ./Tech/${MODE}_PED*dat   > pedfiles;
 ls ./Tech/${MODE}_DAQ*dat   > daqfiles
 paste daqfiles pedfiles  > files
fi; # mode != SI
fi; # Mode != LN


fi; # Arg != short

stop=0;

 if [ $stop -eq 0 ]; then

 if [ "$ARG" != 'SHORT' ]; then

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
 #   echo " RUNNINGJOBS : $RUNNINGJOBS"
    waitsome $pids 1
    RUNNINGJOBS=${#pids[@]}
    echo " PEDCONV LOOP RUNNINGJOBS after waitsome : $RUNNINGJOBS"
    fi;
    done;

# pedestal correction, parsing and fine time calculation

    waitall $pids

echo -n " pedestal decoding done @ "
timing $STARTTIME

RUNNINGJOBS=0
c=0;

if [ "$MODE" == "LN" ] ; then
  for i in L R; do
   for j in 0 1 2 3; do
    NRFILES=`grep ${i}${j} daqfiles  | wc -l`;
    data=`grep ${i}${j} daqfiles | head -n 1`;
    ped=`grep ${i}${j} pedfiles  | head -n 1`;
 #    ~/MODULE_ANA/PSF_V4/resizedat_psf -t -700 -f $data.root -p $ped.ped -n $NRFILES -pos 0;
     echo " LN MODE NOT SUPPORTED YET ! PLEASE CONTACT AVDB IF NEEDED "
     check ${?} "resizedat_psf ${i}${j}";
   done;
  done;
else
 if [ "$MODE" == "PT" ] ; then
  sh ${CODEVERSION}runall.sh files 
 else   
  while read data ped ; do 
   if [ $RUNNINGJOBS -lt $CORES ]; then 
    (( c++ ));
    echo -n " SUBMITTING JOB "
    echo "${CODEVERSION}decoder -f $data -pedfile $ped.ped -uv -t -400 ; "
    ${CODEVERSION}decoder -pedfile $ped.ped -f $data -uv -t -400 > $data.conv.out &
#    pedconv $i pedconv_$j.out &
    pids+=($!);
    (( RUNNINGJOBS++ ));
    else
#    echo " RUNNINGJOBS : $RUNNINGJOBS"
    waitsome $pids 1
    RUNNINGJOBS=${#pids[@]}
#    echo " RUNNINGJOBS after waitsome : $RUNNINGJOBS"
    fi;
 done < files; 

#  sh ${CODEVERSION}runall_si.sh files
 fi; #MODE =PT
fi; # MODE=LN


waitall $pids

 echo -n " decoding done. Time:: "
 timing $STARTTIME 
 echo " PIDS = ${pids[@]} "


 
 
#if [ ! -d DATA ] ;then 
#mkdir DATA
#mv *out ./DATA
#mv *out.root ./DATA
#mv *out.parse.root ./DATA
#mv *ped.RENA? ./DATA
#else
# cd ..
#fi;

# FIXME JULY29 :: THERE IS AN INCONSISTENCY HERE 

fi;  # ARG != SHORT

echo " CHECK ME :: PWD :: `pwd` "



### TODO:  MAKE SURE THE CLEANUP WORKS !!
TIMECHECK=0
 RUNNINGJOBS=0;
 c=0;

echo " PIDS before calibration : ${#pids[@]} :: ${pids[@]} "


#fixme .. fix code !! doesn't run enecal ! --- investigate.

DATE=`pwd | cut -d/ -f4`
for s in L R; do
#for s in R ; do
j=0;
i=0;
while [ $i -le 3 ] ; do 
#for i in 0 1 2 3; do
#for i in 1; do
 if [ $RUNNINGJOBS -lt $CORES ]; then 
    (( c++ ));
 #   echo " SUBMITTING JOB "
    calibrate ${s}${i} & 
    pids+=($!);
    (( RUNNINGJOBS++ ));
    (( i++ )); 
   else
 #   echo " RUNNINGJOBS : $RUNNINGJOBS"
    waitsome $pids 1
    RUNNINGJOBS=${#pids[@]}
    echo " Calibration loop ... RUNNINGJOBS after waitsome : $RUNNINGJOBS"
    fi;
    done;
done;

#  waiting for all calibrations to finish 

waitall $pids



echo -n " calibration done @ "
timing $STARTTIME

check -1 "WIP"


SPLITS=0;

DATE=`pwd | cut -d/ -f4`
for s in L R; do 
#for s in L ; do
j=0;
for i in 0 1 2 3; do
#for i in 0; do
#fi
THISSPLITS=`grep FINDME events${s}${i}.txt | awk '{print $5}'`
THISTIME=`grep FINDME events${s}${i}.txt | awk '{print $3}'`
THISSPLITTIME=`grep FINDME events${s}${i}.txt | awk '{print $7}'`
if [ "${THISSPLITS}" -gt "${SPLITS}" ] ; then
SPLITS=${THISSPLITS};
TOTIME=${THISTIME};
SPLITTIME=${THISSPLITTIME};
fi;
done;
done;


echo " SPLITS : ${SPLITS} TIME: ${TOTIME} SPLITTIME: ${SPLITTIME}" 

echo -n " split estimate done @ "
timing $STARTTIME

#RUNNINGJOBS=0
echo " RUNNINGJOBS after estimating split level @merging : $RUNNINGJOBS  ( pids :: ${pids[@]} )"
# RUNNINGJOBS=${#pids[@]}

for s in L R; do
i=0;
while [ $i -le 3 ] ; do
# merge all entries in a 4-up board
echo "PANEL ${s}${i}"
if [ $RUNNINGJOBS -lt $CORES ]; then 
    echo " TIME  :: ${TOTIME}, submitting ${s}${i} "
    merging ${s}${i} ${SPLITS} ${TOTIME} ${SPLITTIME} &
    pids+=($!);
    (( RUNNINGJOBS++ ));
    (( i++ ));
else
    echo " RUNNINGJOBS before waitsome @merging : $RUNNINGJOBS  ( pids :: ${pids[@]} )"
    waitsome $pids 1
    RUNNINGJOBS=${#pids[@]}
    echo " RUNNINGJOBS after waitsome @merging : $RUNNINGJOBS ( pids :: ${pids[@]} )"
    echo " i = $i " 
fi;

#ln -s ./${s}${i}/${MODE}_DAQ_${DATE}_${s}${i}.4up?.root .
done;
done;

echo "before  waitall combining :: RUNNINGJOBS = $RUNNINGJOBS : ${pids[@]}" 
waitall $pids
echo "after  waitall combining :: RUNNINGJOBS = $RUNNINGJOBS : ${pids[@]}" 
RUNNINGJOBS=${#pids[@]};


echo -n " merging done @ "
timing $STARTTIME



#rm parselist*

#combining 4-up boards into cartridge:
((SPLITS--))
#for k in `seq 1 ${SPLITS}`; do
k=1;
while [ $k -le ${SPLITS} ] ; do
if [ $RUNNINGJOBS -lt $CORES ]; then 
    mergepanel ${k} &
    pids+=($!);
    (( RUNNINGJOBS++ ));
    (( k++ ));
else
    waitsome $pids 1
    RUNNINGJOBS=${#pids[@]}
    echo " RUNNINGJOBS after waitsome : $RUNNINGJOBS :: ${pids[@]}"
fi;
done;

echo "After combining :: RUNNINGJOBS = $RUNNINGJOBS : ${pids[@]}" 
waitall $pids
RUNNINGJOBS=${#pids[@]};

${CODEVERSION}chain_merged -f ${MODE}_DAQ_${DATE}  -n $SPLITS
check ${?} "chain_merged"

${CODEVERSION}merge_ana -f ${MODE}_DAQ_${DATE}_all.merged.root
check ${?} "merge_mana"


echo -n " All done @ "
timing $STARTTIME


fi; # if stop