
#FILENAME=1401171034_L0
FILENAME=$1

#Run decoder 

#decoder -f PED_Data_1401171034_L0_0.dat -p
#decoder -f DAQ_Data_1401171034_L0_0.dat -pedfile PED_Data_1401171034_L0_0.dat.ped -uv

#Generate filelist
#ls DAQ_Data_1401171034_L0_0.dat.root > filelist

#warning if filelist or modlist.txt do not exist
if [ ! -e filelist ]; then 
    echo  " No filelist exists ! " ; 
    echo  " Please create a filelist.\n Exiting. " ; 
    exit -1;
fi

if [ ! -e modlist.txt ]; then 
    echo  " No modlist.txt exists ! " ; 
    echo  " Please create a modlist.\n Exiting. " ; 
    exit -1;
fi

#Chain all files together
chain_parsed -f filelist -o $FILENAME.root

#GetFloods
getfloods -f $FILENAME.root

#AnaFloods
anafloods_psf_v2 -f $FILENAME.root -l 6 
if [ ! -d CHIPDATA ]; then mkdir CHIPDATA; fi;
cp $FILENAME*peaks*txt ./CHIPDATA/

#Enecal - determine gain per pixel
enecal -f $FILENAME.root

#CALIBRATE
calibrate -f $FILENAME.root 

#Fom Ana
fom_ana -f $FILENAME.cal.root -c 0

# generate summary files
while read line ; do 
    name=($line); 
    echo ${name[1]} ; 
    module=$(( 8 + ${name[0]} ))
    for i in 0 1; do
	echo "modana -f $FILENAME.root -C 0 -F 6 -M $module -A ${i} -t ${name[1]};"
	modana -f $FILENAME.root -C 0 -F 6 -M $module -A ${i} -t ${name[1]};
	done;
done  < modlist.txt

