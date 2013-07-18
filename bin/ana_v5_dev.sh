 
#!/bin/bash

BINDIR=~/MODULE_ANA/ANA_V5/DEV


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


 MODE=$1;
 ARG=$2;

 # specifiy SHORT on the command line to only do analysis 
 # for example: sh ~/MODULE_ANA/PSF_V4/cart_ana.sh PT SHORT

 if [ "$MODE" != 'PT' ]; then
  if [ "$MODE" != 'LN' ] ; then
    echo "Please specify mode: \"cart_ana PT SHORT\" or \"cart_ana LN SHORT\" "
    exit -1;
  fi;
fi; 

if [ "$ARG" != 'SHORT' ]; then

#change to DATA directory if needing to rerun
if [ -d DATA ]; then
  cd DATA;
fi;

if [ "$MODE" == "LN" ] ; then
 ls PED_BinaryData_*out > pedfiles;
else
 ls ./Tech/${MODE}_PED*dat   > pedfiles;
fi;
 ls ./Tech/${MODE}_DAQ*dat   > daqfiles
 paste daqfiles pedfiles  > files

fi; # Arg != short

stop=0;

 if [ $stop -eq 0 ]; then

 if [ "$ARG" != 'SHORT' ]; then

# converting binary output to ROOT file format
 if [ -e pedconv.out ] ; then 
   rm pedconv.out
 fi;
 for i in `cat pedfiles`; do ${BINDIR}/Decode -p -f $i >> pedconv.out ;  check ${?} "converting pedfile ${i}"; done;
# for i in `cat daqfiles`; do ~/MODULE_ANA/PSF_V4/UYORUK_toROOT_V3 -f $i;  check ${?} "converting daqfile ${i}";done;

# pedestal correction, parsing and fine time calculation

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
sh ${BINDIR}/runall.sh files 
fi; # MODE=LN
 

if [ ! -d DATA ] ;then 
mkdir DATA
mv *out ./DATA
#mv *out.root ./DATA
#mv *out.parse.root ./DATA
mv *ped.RENA? ./DATA
else
 cd ..
fi;

fi;  # ARG = SHORT

### TODO:  MAKE SURE THE CLEANUP WORKS !!
TIMECHECK=0
DATE=`pwd | cut -d/ -f4`
for s in L R; do
#for s in L ; do
j=0;
for i in 0 1 2 3; do
#for i in 0; do
THISDIR=${s}${i};
mkdir ${THISDIR};

cd ./${THISDIR};
#if [  "$ARG" != 'SHORT' ]; then
ls ../Tech/*.root | grep DAQ | grep ${s}${i} > parselist_${s}${i};
#chain data together
if [ "$MODE" == "LN" ] ; then
 data=`ls -1 ../DATA/${MODE}_DAQ_BinaryData*${s}${i}*out.parse.root | head -n 1`
 ln -s $data ./${MODE}_DAQ_${DATE}_${s}${i}.root;
else
${BINDIR}/chain_parsed -f parselist_${s}${i} -o ${MODE}_DAQ_${DATE}_${s}${i}.root;
     check ${?} "chain_psf panel ${s}${i}";
fi;
#fi;
# get floods
# mv ../DATA/${MODE}_DAQ_${DATE}_${s}${i}.root .
${BINDIR}/getfloods -f ${MODE}_DAQ_${DATE}_${s}${i}.root;
check ${?} "getfloods_psf panel ${s}${i}";
mkdir CHIPDATA;
mv ${MODE}_DAQ_${DATE}_${s}${i}.RENA?.floodsapd?.png ./CHIPDATA/
mv ${MODE}_DAQ_${DATE}_${s}${i}.RENA?.Eapd?.png ./CHIPDATA/
# crystal segmentation
${BINDIR}/anafloods -f ${MODE}_DAQ_${DATE}_${s}${i}.root; 
check ${?} "anafloods_psf panel ${s}${i}";
 mkdir ./FLOODS;
 mv ${MODE}_DAQ*${s}${i}*flood.png ./FLOODS
 mkdir ./QUADRANTS;
 mv ${MODE}_DAQ*${s}${i}*quadrants.png ./QUADRANTS
# determine energy calibration parameters
${BINDIR}/enecal -f ${MODE}_DAQ_${DATE}_${s}${i}.root;
check ${?} "enecal_psf panel ${s}${i}";
# rm ${MODE}_DAQ_${DATE}_${s}${i}.root;
# perform energy calibration
${BINDIR}/enefit -f ${MODE}_DAQ_${DATE}_${s}${i}.enecal.root;
check ${?} "enefit_psf panel ${s}${i}";
 mkdir ./ERES
 mv ${MODE}_DAQ_${DATE}_${s}${i}*glob.png ./ERES/
 rm ${MODE}_DAQ_${DATE}_${s}${i}.enecal.root;
 mv ../${MODE}_DAQ_${DATE}_${s}${i}.RENA* ./CHIPDATA
#  rm *PED_BinaryData_*_${s}${i}*out.ped.RENA?; 
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
echo "Estimating split level"
${BINDIR}/get_optimal_split -f ./${s}${i}/${MODE}_DAQ_${DATE}_${s}${i}.cal.root --${s} -rb ${j} > events${s}${i}.txt
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
${BINDIR}/merge_4up -f ${MODE}_DAQ_${DATE}_${s}${i}.cal.root -rb ${j} --${s} -nc ${SPLITS} -ts ${TIME} -lt ${SPLITTIME};   
check $? "merge_4up panel ${s}${j}"
((j++)) ; 
cd ..;

#ln -s ./${s}${i}/${MODE}_DAQ_${DATE}_${s}${i}.4up?.root .
done;
done;

#rm parselist*

#combining 4-up boards into cartridge:
((SPLITS--))
for k in `seq 0 ${SPLITS}`; do
${BINDIR}/merge_panel -f ${MODE}_DAQ_${DATE}_R0.4up0_part${k}.root -nb 4
check ${?} "merge panel R part ${k}"
${BINDIR}/merge_panel -f ${MODE}_DAQ_${DATE}_L0.4up0_part${k}.root -nb 4
check ${?} "mergep panel L part ${k}"
#combining both sides:
${BINDIR}/merge_coinc -a -fl ${MODE}_DAQ_${DATE}_part${k}_L.panel.root -fr ${MODE}_DAQ_${DATE}_part${k}_R.panel.root
check $? "mergecal part ${k}"
done;



 ${BINDIR}/chain_merged -f ${MODE}_DAQ_${DATE}  -n $SPLITS
check ${?} "chain_merged"

${BINDIR}/mana -f ${MODE}_DAQ_${DATE}_all.merged.root
check ${?} "mana"

fi; # if stop