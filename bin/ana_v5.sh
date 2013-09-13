 
#!/bin/bash

CODEVERSION=/home/miil/MODULE_ANA/ANA_V5/ModuleClass/bin/
CORES=4

function waitsome ()
{
 # waits untill all jobs are done from a list submitted as $1;
 if [ -z "$2" ] ; then
  jobs=0;
 else
  jobs=$2;
 fi

 echo " JOBS TO WAIT FOR :: " $jobs
 pids=$1;
 finished=0;

while [ $finished -lt $jobs ] ; do
  index=0;
  for i in ${pids[@]}; do
#  echo "job $i index $index array length : ${#pids[@]} . Array: ${pids[*]}" ;
  if [ ! -d /proc/$i ]; then 
#           echo "$i done ! ( index $index ) " ; 
           (( finished++ )); 
           unset pids[$index];
	   pids=("${pids[@]}")
#	   pids=${pids[@]//$i/}
#           echo " after unsetting: ${pids[*]}"
	   continue
           fi;
  (( index++ )); 
  done;
#  echo " Finished jobs :: " $finished " ( pids = " ${pids[@]} " )" 
  sleep .5
done;
}

function waitall ()
{
 # waits untill all jobs are done from a list submitted as $1;
 pids=$1;
 waitsome $1 ${#pids[@]} 
}



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

function mkfolder ()
{
  FOLDER=$1;
  if [ ! -d ${FOLDER} ] ;then  mkdir ${FOLDER}; fi
}


function timing ()
 { START=$1;
   TIME=$(( `date +%s` - $START ));
   echo $TIME;
}

function pedconv ()
 {  ${CODEVERSION}decoder -v -p -f $1 >> $2 ;  
    check ${?} "converting pedfile ${1}"; 
#    sleep 10
  }


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
    echo " RUNNINGJOBS after waitsome : $RUNNINGJOBS"
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
    ${CODEVERSION}decoder -pedfile $ped.ped -f $data -uv -t -400  &
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

echo -n " decoding done @ "
timing $STARTTIME
 

if [ ! -d DATA ] ;then 
mkdir DATA
mv *out ./DATA
#mv *out.root ./DATA
#mv *out.parse.root ./DATA
mv *ped.RENA? ./DATA
else
 cd ..
fi;

fi;  # ARG != SHORT

check -1 "WIP ( ${#pids[2]} entries in pids :: ${pids[@]} )"

### TODO:  MAKE SURE THE CLEANUP WORKS !!
TIMECHECK=0
DATE=`pwd | cut -d/ -f4`
for s in L R; do
#for s in R ; do
j=0;
for i in 0 1 2 3; do
#for i in 1; do
THISDIR=${s}${i};
mkfolder ${THISDIR}
cd ./${THISDIR};
#if [  "$ARG" != 'SHORT' ]; then
ls ../Tech/*.root | grep DAQ | grep ${s}${i} > parselist_${s}${i};
#chain data together
if [ "$MODE" == "LN" ] ; then
 data=`ls -1 ../DATA/${MODE}_DAQ_BinaryData*${s}${i}*out.parse.root | head -n 1`
 ln -s $data ./${MODE}_DAQ_${DATE}_${s}${i}.root;
else
${CODEVERSION}chain_parsed -f parselist_${s}${i} -o ${MODE}_DAQ_${DATE}_${s}${i}.root;
     check ${?} "chain_psf panel ${s}${i}";
fi;


#fi;
# get floods
# mv ../DATA/${MODE}_DAQ_${DATE}_${s}${i}.root .
${CODEVERSION}getfloods -f ${MODE}_DAQ_${DATE}_${s}${i}.root;
check ${?} "getfloods panel ${s}${i}";
mkfolder CHIPDATA;
mv ${MODE}_DAQ_${DATE}_${s}${i}.RENA?.floodsapd?.png ./CHIPDATA/
mv ${MODE}_DAQ_${DATE}_${s}${i}.RENA?.Eapd?.png ./CHIPDATA/

# crystal segmentation
${CODEVERSION}anafloods_psf_v2 -f ${MODE}_DAQ_${DATE}_${s}${i}.root; 
check ${?} "anafloods_psf panel ${s}${i}";
mkfolder ./FLOODS;
mv ${MODE}_DAQ*${s}${i}*flood.png ./FLOODS
mkfolder ./QUADRANTS;
mv ${MODE}_DAQ*${s}${i}*quadrants.png ./QUADRANTS

# determine energy calibration parameters
${CODEVERSION}enecal -f ${MODE}_DAQ_${DATE}_${s}${i}.root;
check ${?} "enecal_psf panel ${s}${i}";
## rm ${MODE}_DAQ_${DATE}_${s}${i}.root;
# perform energy calibration
${CODEVERSION}enefit -f ${MODE}_DAQ_${DATE}_${s}${i}.enecal.root;
check ${?} "enefit_psf panel ${s}${i}";
 mkfolder ./ERES;
 mv ${MODE}_DAQ_${DATE}_${s}${i}*glob.png ./ERES/
 rm ${MODE}_DAQ_${DATE}_${s}${i}.enecal.root;
 mv ${MODE}_DAQ_${DATE}_${s}${i}.RENA* ./CHIPDATA
##  rm *PED_BinaryData_*_${s}${i}*out.ped.RENA?; 
 cd ..;
done;
done;



SPLITS=0;

DATE=`pwd | cut -d/ -f4`
for s in L R; do 
#for s in L ; do
j=0;
for i in 0 1 2 3; do
#for i in 0; do
echo "Estimating split level ${s}${i}"
${CODEVERSION}get_optimal_split -f ./${s}${i}/${MODE}_DAQ_${DATE}_${s}${i}.cal.root --${s} -rb ${j} > events${s}${i}.txt
check ${?} "get_optimal_split panel ${s}${i}";
#fi
THISSPLITS=`grep FINDME events${s}${i}.txt | awk '{print $5}'`
THISTIME=`grep FINDME events${s}${i}.txt | awk '{print $3}'`
THISSPLITTIME=`grep FINDME events${s}${i}.txt | awk '{print $7}'`
if [ "${THISSPLITS}" -gt "${SPLITS}" ] ; then
SPLITS=${THISSPLITS};
TIME=${THISTIME};
SPLITTIME=${THISSPLITTIME};
fi;
done;
done;


echo " SPLITS : ${SPLITS}" 
for s in L R; do
j=0;
for i in 0 1 2 3; do
# merge all entries in a 4-up board
cd ${s}${i}
${CODEVERSION}merge_4up -f ${MODE}_DAQ_${DATE}_${s}${i}.cal.root -rb ${j} --${s} -nc ${SPLITS} -ts ${TIME} -lt ${SPLITTIME};   
check $? "merge_4up panel ${s}${j}"
((j++)) ; 
cd ..;

#ln -s ./${s}${i}/${MODE}_DAQ_${DATE}_${s}${i}.4up?.root .
done;
done;

#rm parselist*

#combining 4-up boards into cartridge:
((SPLITS--))
for k in `seq 1 ${SPLITS}`; do
${CODEVERSION}merge_panel -f ${MODE}_DAQ_${DATE}_R0.4up0_part${k}.root -nb 4
check ${?} "merge panel R part ${k}"
${CODEVERSION}merge_panel -f ${MODE}_DAQ_${DATE}_L0.4up0_part${k}.root -nb 4
check ${?} "merge panel L part ${k}"
#combining both sides:
${CODEVERSION}merge_coinc -a -fl ${MODE}_DAQ_${DATE}_part${k}_L.panel.root -fr ${MODE}_DAQ_${DATE}_part${k}_R.panel.root
check $? "mergecal part ${k}"
done;



${CODEVERSION}chain_merged -f ${MODE}_DAQ_${DATE}  -n $SPLITS
check ${?} "chain_merged"

${CODEVERSION}mana -f ${MODE}_DAQ_${DATE}_all.merged.root
check ${?} "mana"

fi; # if stop