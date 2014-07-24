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
    usage
    exit 1
fi

filename=$1
panel=$2
cart=$3


if [ -ne $filename.ped ]; then
    decoder -p -f $filename -cmap ./DAQ_Board_Map.nfo
fi

root -l -q "/products/BreastCal/dev/anascripts/plot_pedestal.C(\"$filename.ped\",$cart)"
