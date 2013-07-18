SPLITS=0;

DATE=`pwd | cut -d/ -f4`
for s in L R; do 
#for s in L ; do
j=0;
for i in 0 1 2 3; do
#for i in 0; do
echo "Estimating split level"
~/MODULE_ANA/PSF_V4/DEV/get_optimal_split -f ./${s}${i}/${MODE}_DAQ_${DATE}_${s}${i}.cal.root --${s} -rb ${j} > events${s}${i}.txt
check ${?} "get_optimal_split panel ${s}${i}";
#fi
THISSPLITS=`grep FINDME events${s}${i}.txt | awk '{print $5}'`
THISTIME=`grep FINDME events${s}${i}.txt | awk '{print $3}'`
if [ "${THISSPLITS}" -gt "${SPLITS}" ] ; then
SPLITS=${THISSPLITS};
TIME=${THISTIME};
fi;
done;
done;

echo " SPLITS : ${SPLITS}" 
for s in L R; do
j=0;
for i in 0 1 2 3; do
# merge all entries in a 4-up board
cd ${s}${i}
~/MODULE_ANA/PSF_V4/DEV/merge_4up -f ${MODE}_DAQ_${DATE}_${s}${i}.cal.root -rb ${j} --${s} -nc ${SPLITS} -ts ${TIME};   
check $? "merge_4up panel ${s}${j}"
((j++)) ; 
cd ..;

#ln -s ./${s}${i}/${MODE}_DAQ_${DATE}_${s}${i}.4up?.root .
done;
done;

((SPLITS--))
for k in `seq 1 ${SPLITS}`; do
~/MODULE_ANA/PSF_V4/DEV/merge_panel -f ${MODE}_DAQ_${DATE}_R0.4up0_part${k}.root -nb 4
check ${?} "merge panel R part ${k}"
~/MODULE_ANA/PSF_V4/DEV/merge_panel -f ${MODE}_DAQ_${DATE}_L0.4up0_part${k}.root -nb 4
check ${?} "mergep panel L part ${k}"
#combining both sides:
~/MODULE_ANA/PSF_V4/DEV/mergecal_v3 -a -fl ${MODE}_DAQ_${DATE}_part${k}_L.panel.root -fr ${MODE}_DAQ_${DATE}_part${k}_R.panel.root
check $? "mergecal part ${k}"
done;
fi;


 ~/MODULE_ANA/PSF_V4/DEV/chain_merged -f ${MODE}_DAQ_${DATE} -v -n $SPLITS
check ${?} "chain_merged"

~/MODULE_ANA/PSF_V4/DEV/mana -f ${MODE}_DAQ_${DATE}_all.merged.root
check ${?} "mana"-bash-4.1$  head -n 80 ~/bin/cart_ana_v2.sh
 
