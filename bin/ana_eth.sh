#!/bin/bash

##############################################################################################################

NUM_CARTRIDGES_PER_PANEL=3;

##############################################################################################################

##############################################################################################################

function mkfolder ()
{
    FOLDER=$1;
    if [ ! -d ${FOLDER} ] ;then  mkdir ${FOLDER}; fi
}
##############################################################################################################

##############################################################################################################

function timing ()
{
    START=$1;
    TIME=$(( `date +%s` - ${START} ));
    echo $TIME;
}

##############################################################################################################
##############################################################################################################

function pedconv ()
{
    decoder -p -f $1 -cmap "../DAQ_Board_Map.nfo" > $2 ;
    check ${?} "converting pedfile ${1}";
    #    sleep 10
}

##############################################################################################################
##############################################################################################################

function waitsome ()
{
    # waits untill all jobs are done from a list submitted as $1;

    local jobs=${1:-0}

    log " JOBS TO WAIT FOR :: " $jobs

    local finished=0;


    while [ $finished -lt $jobs ] ; do
        local index=0;
        for pid in ${pids[@]}; do
            # echo "job $pid index $index array length : ${#pids[@]} . Array: ${pids[*]}" ;
            kill -0 $pid 2> /dev/null
            if [ $? -eq 1 ]; then
                # echo "$pid done ! ( index $index ) " ;
                (( finished++ ));
                unset pids[$index];
                pids=("${pids[@]}")
                # echo " after unsetting: ${pids[*]}"
                continue
            fi;
            (( index++ ));
        done;
        # echo " Finished jobs :: " $finished " ( pids = " ${pids[@]} " )"
        sleep 2
    done;
}

##############################################################################################################
function waitone ()
{
    while kill -0 $1; do
        sleep 0.5
    done
}

##############################################################################################################

function waitall ()
{
    # waits untill all jobs are done from a list submitted as $1;
    # pids=$1;
    log " WAITALL :: NEED TO WAIT FOR ${#pids[@]} jobs to finish with IDs :: ${pids[*]}"
    waitsome ${#pids[@]}
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

function notify ()
{
    RET=$1;
    if [ ${RET} != 0 ] ; then
        echo " ========================================="
        echo " ............ WARNING ...................."
        echo " Program failed at $2 ";
        echo " ========================================="
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
    echo "ana_eth.sh [ -v -d -cal -c -cc -s -m -t -a -h -C [] ]"
    echo " with: -v: verbose"
    echo "       -d: decode binary data ( Left and Right )"
    echo "       -g: segment data (Left and Right)"
    echo "       -cal: calculate calibration parameters (Left and Right)"
    echo "       -c: calibrate data ( Left and Right )"
    echo "       -cc: calibrate data using individual decoded files"
    echo "       -s: time sort Left and Right data"
    echo "       -m: merge Left and Right"
    echo "       -t: time calibrate"
    echo "       -a: do all of the above"
    echo "       -u: do decode and segment"
    echo "       -l: do -c -s -m -t"
    echo "       -ci: individually calibrate all decoded files"
    echo "       -si: individually sort and energy gate calibrated files"
    echo "       -pi: individually calibrate, sort, and energy gate files"
    echo "       -mi: individually merge and coincidence sort egated files"
    echo "       -chi: chain together individually merged files"
    echo "       -ri: individually coincidence merge/chain/time cal random coinc files"
    echo "       -i: do -pi -mi -chi"
    echo "       -ai: do -d -g -cal -i -t"
    echo "       -h: display this help"
    echo "       -C [] : number of cores"
    echo "       -el [] : set the low energy window, default $energy_low"
    echo "       -eh [] : set the high energy window, default $energy_high"
    echo "       -tt [] : set the trigger threshold, default $trigger_threshold"
    echo "       -dt [] : set the double trigger threshold, default $double_trigger_threshold"
}

##############################################################################################################

CORES=4
energy_low=400
energy_high=700
trigger_threshold=-700
double_trigger_threshold=-700
STARTTIME=`date +%s`
verbose=0
dodecode=0
dosegmentation=0
docalibrate=0
dosort=0
domerge=0
dotimecal=0
doindividualcal=0
doindividualsort=0
doindividualmerge=0
doindividualegate=0
doindividualchain=0
doindividualprocess=0
setupfolders=0
dopedestals=0

while [ $# -gt 0 ];
do
    case "$1" in
	-v) verbose=1;;
	-d) dodecode=1; setupfolders=1; dopedestals=1;;
    -g) dosegmentation=1;;
    -cal) docalccalibrate=1;;
	-c) docalibrate=1;;
    -cc) dodecodedcalibrate=1;;
    -pi) doindividualprocess=1;;
    -ci) doindividualcal=1;;
	-s) dosort=1;;
    -si) doindividualsort=1;;
    -ei) doindividualegate=1;;
	-m) domerge=1;;
    -mi) doindividualmerge=1;;
    -chi) doindividualchain=1;;
    -ri) doindividualrandmerge=1;;
    -i) doindividualprocess=1; doindividualmerge=1; doindividualchain=1;;
    -ai) dodecode=1; setupfolders=1; dopedestals=1; dosegmentation=1; docalccalibrate=1; doindividualprocess=1; doindividualmerge=1; doindividualchain=1; dotimecal=1;;
	-t) dotimecal=1;;
	-a) dodecode=1;dosegmentation=1;docalccalibrate=1;docalibrate=1;dosort=1;domerge=1;dotimecal=1;setupfolders=1;dopedestals=1;;
    -u) docalccalibrate=1;docalibrate=1;dosort=1;;
    -l) docalibrate=1;dosort=1;domerge=1;dotimecal=1;;
	-h) usage ; exit; break;;
	-C) CORES=$2;shift;;
	-el) energy_low=$2;shift;;
	-eh) energy_high=$2;shift;;
	-dgcal) dodecode=1;dosegmentation=1;docalccalibrate=1;setupfolders=1;dopedestals=1;;
	*) usage; break;;
    esac
    shift
done;

echo "NUMBER OF CORES:: " $CORES


################################################################################
if [[ $setupfolders -eq 1 ]]; then
    echo "Setting up folders"
    if [ ! -d Left ]; then
        mkdir Left
    fi

    files=$(ls -1 ./PED*L*dat 2> /dev/null)
    for file in $files; do
        mv $file ./Left/
    done

    files=$(ls -1 ./DAQ*L*dat 2> /dev/null)
    for file in $files; do
        mv $file ./Left/
    done

    if [ ! -d Right ]; then
        mkdir Right
    fi

    files=$(ls -1 ./PED*R*dat 2> /dev/null)
    for file in $files; do
        mv $file ./Right/
    done

    files=$(ls -1 ./DAQ*R*dat 2> /dev/null)
    for file in $files; do
        mv $file ./Right/
    done
fi

################################################################################
# Right now, only take the first pedestal file, and decode it to use with the
# decoding the data files.  This will need to change if something more
# complicated needs to be done.

if [[ $dopedestals -eq 1 ]]; then
    log "Decoding pedestals"
    for k in Left Right; do
        cd $k

        panel_id=0
        if [ $k == "Right" ]; then
            panel_id=1;
        fi

        ls -tr ./PED*dat > pedfiles
        ped_data_file=$(head -n 1 pedfiles)
        cmd="decoder -p -f $ped_data_file -cmap ../DAQ_Board_Map.nfo -pid $panel_id &> $ped_data_file.decoder.out"
        echo $cmd
        eval $cmd
        check $? $cmd
        cd ..
    done
fi

#############################################################################################################

if [[ $dodecode -eq 1 ]]; then
    log "Decoding data"
    for k in Left Right; do
        cd $k
        # get 'L' or 'R' from k
        KK=${k:0:1}
		ls -tr ./DAQ*dat > daqfiles
        panel_id=0
        if [ $k == "Right" ]; then
            panel_id=1;
        fi

        pedfile=$(ls -1r ./*.ped | head -n 1)

        files=$(cat daqfiles)
        for file in $files; do
            while [ ${#pids[@]} -ge $CORES ]; do
                waitsome 1
            done
            cmd="decoder -pedfile $pedfile -f $file -uv -t $trigger_threshold -n $double_trigger_threshold -cmap ../DAQ_Board_Map.nfo -pid $panel_id &> $file.decode.out &"
            echo $cmd
            eval $cmd
            notify $? $cmd
            pids+=($!);
        done
        waitall
        cd ../
    done
fi


#############################################################################################################

if [[ $dosegmentation -eq 1 ]]; then
    for k in Left Right; do
        cd $k
        KK=${k:0:1}
        BASE=`ls DAQ*${KK}0*root | head -n 1 | cut -d ${KK} -f 1`${KK}
        ls DAQ*${KK}0*dat.root | cut -d _ -f 5 | sort -n | awk -v FBASE=$BASE '{print FBASE"0_"$1}' > filelist

        cmd="chain_parsed -f filelist -o ${BASE}.root &> ${BASE}.root.chain_parsed.out"
        echo $cmd
        eval $cmd
        check ${?} $cmd

        cmd="getfloods -f ${BASE}.root &> ${BASE}.root.getfloods.out"
        echo $cmd
        eval $cmd
        check ${?} $cmd

        for cartridge in `seq 0 $((NUM_CARTRIDGES_PER_PANEL-1))`; do
            for fin in `seq 0 7`; do
                while [ ${#pids[@]} -ge $CORES ]; do
                    waitsome 1
                done
                cmd="anafloods_psf_v2 -f ${BASE}.root -c $cartridge -l $fin &> ${BASE}.root.anafloods_c${cartridge}f${fin}.out &"
                echo $cmd
                eval $cmd
                notify $? $cmd
                pids+=($!);
            done;
        done;
        waitall
        cd ..
    done;
fi


#############################################################################################################

if [[ $docalccalibrate -eq 1 ]]; then
    for k in Left Right; do
        cd $k
        KK=${k:0:1}
        BASE=`ls DAQ*${KK}0*root | head -n 1 | cut -d ${KK} -f 1`${KK}
        mkdir CHIPDATA
        mv *peaks.txt ./CHIPDATA
	    mv *peaks.failed.txt ./CHIPDATA
        cmd="enecal -f ${BASE}.root &> ${BASE}.root.enecal.out"
        echo $cmd
        eval $cmd
        check ${?} $cmd
        cd ..
    done;
fi


#############################################################################################################


if [[ $docalibrate -eq 1 ]]; then
    for k in Left Right; do
        cd $k
        KK=${k:0:1}
        BASE=`ls DAQ*${KK}0*root | head -n 1 | cut -d ${KK} -f 1`${KK}
        cmd="calibrate -f ${BASE}.root &> ${BASE}.root.calibrate.out"
        echo $cmd
        eval $cmd
        check ${?} $cmd
        cd ..
    done;
fi


#############################################################################################################

if [[ $dodecodedcalibrate -eq 1 ]]; then
    for k in Left Right; do
        cd $k
        KK=${k:0:1}
        BASE=`ls DAQ*${KK}0*root | head -n 1 | cut -d ${KK} -f 1`${KK}
        ls -tr ./DAQ*${KK}*dat.root > daqfiles
        for daq_file in `cat daqfiles`; do
            while [ ${#pids[@]} -ge $CORES ]; do
                waitsome 1
            done
            cmd="calibrate -f ${daq_file} -c ${BASE}.par.root -noplots -q &> ${daq_file}.calibrate.out &"
            echo $cmd
            eval $cmd
            pids+=($!);
            echo $daq_file
        done;

        # calibration

        waitall

        chain_cal -f ${BASE}0 -n `wc -l daqfiles | awk '{print $1}'` -of ${BASE}.cal.root

        cd ..
    done;
fi



#############################################################################################################

# This processes each of the decoded files by calibrating, sorting, and energy
# gating each of the files. Takes files with extension of .dat.root and outputs
# a .dat.cal.sort.egate.root file.  This does everything that doindividualcal,
# doindividualsort, and do individualegate does.  This produces just one output
# file as opposed one for each step.
if [[ $doindividualprocess -eq 1 ]]; then
    for k in Left Right; do
        cd $k
        KK=${k:0:1}
        BASE=`ls DAQ*${KK}0*root | head -n 1 | cut -d ${KK} -f 1`${KK}
        ls -tr ./DAQ*${KK}*dat.root > daqfiles

        for daq_file in `cat daqfiles`; do
            while [ ${#pids[@]} -ge $CORES ]; do
                waitsome 1
            done
            cmd="process_file -f ${daq_file} -calf ${BASE}.par.root -s -eg -el $energy_low -eh $energy_high &> ${daq_file}.process_file.out &"
            echo $cmd
            eval $cmd
            notify $? $cmd
            pids+=($!);
        done;
        waitall
        cd ../
    done;
fi


#############################################################################################################

if [[ $doindividualcal -eq 1 ]]; then
    for k in Left Right; do
        cd $k
        KK=${k:0:1}
        BASE=`ls DAQ*${KK}0*root | head -n 1 | cut -d ${KK} -f 1`${KK}
        ls -tr ./DAQ*${KK}*dat.root > daqfiles
        for daq_file in `cat daqfiles`; do
            while [ ${#pids[@]} -ge $CORES ]; do
                waitsome 1
            done
            cmd="calibrate -f ${daq_file} -c ${BASE}.par.root -noplots -q &> ${daq_file}.calibrate.out &"
            echo $cmd
            eval $cmd
            pids+=($!);
            echo $daq_file
        done;
        waitall

        cd ..
    done;
fi




#############################################################################################################

if [[ $doindividualsort -eq 1 ]]; then
    for k in Left Right; do
        cd $k
        KK=${k:0:1}
        BASE=`ls DAQ*${KK}0*root | head -n 1 | cut -d ${KK} -f 1`${KK}
        ls -tr ./DAQ*${KK}*dat.root > daqfiles

        ls -tr ./DAQ*${KK}*dat.cal.root > calfiles
        for daq_file in `cat calfiles`; do
            while [ ${#pids[@]} -ge $CORES ]; do
                waitsome 1
            done
            cmd="sort_file -f ${daq_file} -eg -v &> ${daq_file}.sort.out &"
            echo $cmd
            eval $cmd
            pids+=($!);
            echo $daq_file
        done;

        waitall
        cd ..
    done;
fi


#############################################################################################################

# Useful as a secondary step if you use sort_file to generate a time sorted
# list of singles events that are not energy gated.  This will energy gate them
# and produce the necessary .cal.sort.egate file that can then be merged
if [[ $doindividualegate -eq 1 ]]; then
    for k in Left Right; do
        cd $k
        KK=${k:0:1}

        ls -tr ./DAQ*${KK}*dat.cal.sort.root > sortedfiles
        for daq_file in `cat sortedfiles`; do
            while [ ${#pids[@]} -ge $CORES ]; do
                waitsome 1
            done

            echo $daq_file
            cmd="egate_file -f ${daq_file} -el $energy_low -eh $energy_high -v &> ${daq_file}.egate.out &"
            echo $cmd
            eval $cmd
            pids+=($!);
        done;

        waitall
        cd ..
    done;
fi


#############################################################################################################

if [[ $doindividualmerge -eq 1 ]]; then
    for k in Left; do
        cd Left
        KK=${k:0:1}
        BASE=`ls DAQ*${KK}0*root | head -n 1 | cut -d ${KK} -f 1`${KK}
        ls -tr DAQ*${KK}*cal.sort.egate.root > ../calfiles
        cd ..

        for daq_file in `cat calfiles`; do
            while [ ${#pids[@]} -ge $CORES ]; do
                waitsome 1
            done
            right_file=./Right/`echo $daq_file | sed 's/L0/R0/g'`
            if [ -e $right_file ]; then
                output_file=`echo $daq_file | sed 's/_L0//g'`

                cmd="merge_coinc -fl ./Left/$daq_file -fr $right_file -of $output_file &> $output_file.merge_coinc.out &"
                echo $cmd
                eval $cmd
                notify $? $cmd
                pids+=($!);
            fi
        done
    done
fi

#############################################################################################################
# This part does random coincidences

if [[ $doindividualrandmerge -eq 1 ]]; then

    mkdir RANDOMS;

    for k in Left; do
        cd Left
        KK=${k:0:1}
        BASE=`ls DAQ*${KK}0*root | head -n 1 | cut -d ${KK} -f 1`${KK}
        ls -tr DAQ*${KK}*cal.sort.egate.root > ../RANDOMS/calfiles
        cd ..

        for daq_file in `cat ./RANDOMS/calfiles`; do
            while [ ${#pids[@]} -ge $CORES ]; do
                waitsome 1
            done
            right_file=./Right/`echo $daq_file | sed 's/L0/R0/g'`
            if [ -e $right_file ]; then
                output_file=./RANDOMS/`echo $daq_file | sed 's/_L0//g'`

		# Added delay of 300 to merge_coinc so it's doing randoms coinc
                cmd="merge_coinc -fl ./Left/$daq_file -fr $right_file -of $output_file -d 300 &> $output_file.merge_coinc.out &"
                echo $cmd
                eval $cmd
                pids+=($!);
            fi
        done
    done

    # Individualchainportion for randoms
    cd Left
    BASE1=`ls DAQ*L0*root | head -n 1 | cut -d L -f 1`
    cd ../RANDOMS
    ls -tr DAQ_Data_*.cal.sort.egate.root > mergedfiles

    cmd="chain_list -v -f mergedfiles -of ${BASE1}all.merged.root -c merged &> ${BASE1}all.merged.root.chain_list.out"
    echo $cmd
    eval $cmd

    # Timecal portion for randoms

    BASE2=`ls *all.merged.root | cut -d _ -f 1-3`

    input_file="${BASE2}_all.merged.root"
    cmd="merge_ana -r -f $input_file &> $input_file.merge_ana.out"
    echo $cmd
    eval $cmd

    input_file="${BASE2}_all.merged.randoms.root"
    cmd="cal_apd_offset -f $input_file &> $input_file.cal_apd_offset.out"
    echo $cmd
    eval $cmd

    input_file="${BASE2}_all.merged.randoms.apdoffcal.root"
    cmd="cal_crystal_offset2 -f $input_file -ft 100000 -dp &> $input_file.cal_crystal_offset2.out"
    echo $cmd
    eval $cmd

    input_file="${BASE2}_all.merged.randoms.apdoffcal.crystaloffcal.root"
    cmd="cal_edep -f $input_file -ft 100000 &> $input_file.cal_edep.out"
    echo $cmd
    eval $cmd

    input_file="${BASE2}_all.merged.randoms.apdoffcal.crystaloffcal.edepcal.root"
    cmd="cal_crystal_offset2  -f $input_file -ft 100000 -dp &> $input_file.cal_crystal_offset2.out"
    echo $cmd
    eval $cmd

    cd ..
fi

#############################################################################################################

if [[ $doindividualchain -eq 1 ]]; then
    cd Left
    BASE=`ls DAQ*L0*root | head -n 1 | cut -d L -f 1`
    cd ../
    ls -tr DAQ_Data_*.cal.sort.egate.root > mergedfiles

    cmd="chain_list -v -f mergedfiles -of ${BASE}all.merged.root -c merged &> ${BASE}all.merged.root.chain_list.out"
    echo $cmd
    eval $cmd
    check $? $cmd
fi

#############################################################################################################

if [[ $dosort -eq 1 ]]; then
    SPLITS=0
    for k in Left Right; do
        cd $k
        KK=${k:0:1}

        BASE=`ls DAQ*${KK}0*root | head -n 1 | cut -d ${KK} -f 1`${KK}
        log "Estimating split level ${1}"
        VAR=$1;
        LR=`echo "${VAR:0:1}"`;
        log "get_opt_split -f ./${BASE}.cal.root   > opt_split_${KK}.txt"
        get_opt_split -f ./${BASE}.cal.root   > opt_split_${KK}.txt
        check ${?} "get_opt_split ${k}";
        THISSPLITS=`grep FINDME opt_split_${KK}.txt | awk '{print $5}'`
        THISTIME=`grep FINDME opt_split_${KK}.txt | awk '{print $7}'`
        THISSPLITTIME=`grep FINDME opt_split_${KK}.txt | awk '{print $3}'`
        if [ "${THISSPLITS}" -gt "${SPLITS}" ] ; then
            SPLITS=${THISSPLITS};
            TOTIME=${THISTIME};
            SPLITTIME=${THISSPLITTIME};
        fi;
        cd ..
    done;

    #echo $SPLITS
    #echo $TOTIME
    #echo $SPLITTIME

    for k in Left Right; do
        cd $k
        KK=${k:0:1}
        i=0;
        # note:: in the future, we may have up to 3 L's : L0 L1 and L3
        while [ $i -le 0 ] ; do
            BASE=`ls ./DAQ*${KK}${i}*root | head -n 1 | cut -d ${KK} -f 1`${KK}
            while [ ${#pids[@]} -ge $CORES ]; do
                waitsome 1
            done
            cmd="merge_4up -f ./${BASE}.cal.root -nc ${SPLITS} -ts ${SPLITTIME} -lt ${TOTIME} &> merge_4up.${KK}${i}.out   &"
            echo $cmd
            eval $cmd
            check $? $cmd
            pids+=($!);
            (( i++ ));
        done;
        cd ..
    done;

    waitall

fi


#############################################################################################################

if [[ $domerge -eq 1 ]]; then
    # estimate number of splits
    LEFTSPLITS=`ls -ltr ./Left/*part*root  | wc -l`
    RIGHTSPLITS=`ls -ltr ./Right/*part*root  | wc -l`

    if [ $LEFTSPLITS -ne $RIGHTSPLITS ]; then
        echo "Error: Counted $LEFTSPLITS files on the left, and $RIGHTSPLITS files on the right, these should be equal"
        exit -99;
    fi;

    BASE=` ls -1 ./Left/DAQ*part*root | head -n 1 | cut -d / -f 3 | cut -d _ -f 1-3`

    # counting from zero
    (( LEFTSPLITS-- )) ;

    for i in `seq 0 $LEFTSPLITS` ; do
        while [ ${#pids[@]} -ge $CORES ]; do
            waitsome 1
        done
        cmd="merge_coinc -fl ./Left/${BASE}_L.cal_part${i}.root -fr ./Right/${BASE}_R.cal_part${i}.root -of  ./${BASE}_part${i}.root &> ./${BASE}_part${i}.root.merge_coinc.out &"
        echo $cmd
        eval $cmd
        pids+=($!);
    done;

    waitall

    # This is confusing, but the argument to "-n" gets augmented by one in chain_merged.C

    cmd="chain_merged -f ${BASE} -n ${LEFTSPLITS} &> ${BASE}.chain_merged.out"
    echo $cmd
    eval $cmd
fi;


#############################################################################################################

if [[ $dotimecal -eq 1 ]]; then
    BASE=`ls *all.merged.root | cut -d _ -f 1-3`

    input_file="${BASE}_all.merged.root"
    cmd="merge_ana -f $input_file &> $input_file.merge_ana.out"
    echo $cmd
    eval $cmd
    check $? $cmd

    input_file="${BASE}_all.merged.ana.root"
    cmd="cal_apd_offset -f $input_file &> $input_file.cal_apd_offset.out"
    echo $cmd
    eval $cmd
    check $? $cmd

    input_file="${BASE}_all.merged.ana.apdoffcal.root"
    cmd="cal_crystal_offset2 -f $input_file -ft 60 -dp &> $input_file.cal_crystal_offset2.out"
    echo $cmd
    eval $cmd
    check $? $cmd

    input_file="${BASE}_all.merged.ana.apdoffcal.crystaloffcal.root"
    cmd="cal_edep -f $input_file -ft 40 &> $input_file.cal_edep.out"
    echo $cmd
    eval $cmd
    check $? $cmd

    input_file="${BASE}_all.merged.ana.apdoffcal.crystaloffcal.edepcal.root"
    cmd="cal_crystal_offset2  -f $input_file -ft 30 -dp &> $input_file.cal_crystal_offset2.out"
    echo $cmd
    eval $cmd
    check $? $cmd
fi
#############################################################################################################
## ONLY STEP TO DO:  format_recon -f ${BASE}_all.merged.ana.apdoffcal.crystaloffcal.edepcal.crystaloffcal.root -p 64.262 -t 20
## -p :: paneldistance in mm
## -t :: fine time window, e.g: -t 2- ::   -20 < ft < 20
############################################################################################################
