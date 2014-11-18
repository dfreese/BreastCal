#!/bin/bash

usage ()
{
    echo ""
    echo "plot_pedestals.sh"
    echo "    usage: plot_pedestals <filename> <panel> <cartridge>"
    echo ""
    echo "    filename  - the name of the pedestal \".dat\" file"
    echo "    panel     - 0 or 1 for left or right panel"
    echo "    cartridge - the cartridge number to display from the file"
    echo ""
    echo "note: This program assumes it is run from the Tech folder that"
    echo "      contains the relevant DAQ_Board_Map.nfo"
}


if [ $# -ne 3 ]; then
    if [ $# -ne 1 ]; then
        usage
        exit 1
    else
        if [ $1 == "-a" ]; then
            doall=1
        fi
    fi
fi


if [ $doall -eq 1 ]; then
    left_pedfiles=$(ls PED*_L0_0.dat)
    for file in $left_pedfiles; do
        echo $file
        decoder -p -f $file -cmap ./DAQ_Board_Map.nfo
        root -l -q "${ANADIR}/anascripts/plot_pedestal.C(\"$file.ped\",0,0)"
        root -l -q "${ANADIR}/anascripts/plot_pedestal.C(\"$file.ped\",1,0)"
    done

    right_pedfiles=$(ls PED*R0_0.dat)
    for file in $right_pedfiles; do
        echo $file
        decoder -p -f $file -cmap ./DAQ_Board_Map.nfo
        root -l -q "${ANADIR}/anascripts/plot_pedestal.C(\"$file.ped\",0,1)"
        root -l -q "${ANADIR}/anascripts/plot_pedestal.C(\"$file.ped\",1,1)"
    done
else 
    filename=$1
    panel=$2
    cart=$3
    if [ ! -e $filename.ped ]; then
        decoder -p -f $filename -cmap ./DAQ_Board_Map.nfo
    fi
    root -l -q "${ANADIR}/anascripts/plot_pedestal.C(\"$filename.ped\",$cart)"
fi
