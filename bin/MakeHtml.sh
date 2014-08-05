#!/bin/bash

string=$1;

if [ !  -d $string ]; then 
    mkdir $string; 
    cd $string
else 
    cd $string
    rm *html;
fi;



PNGNAMESUM=$string"*.sum.png"
PNGNAMEFLOOD=$string"*_flood.png"
PNGNAMEGLOB=$string"*_glob_fits.png"

IGALCMD="igal2 -n -r -x -y 150 -w 4"
SEDCOMMAND=sed

for i in 0 1; do 

    if [ -d A$i ]; then
        cd ./A$i
        rm *html
    else
        mkdir ./A$i
        cd ./A$i
    fi

    #    echo "---> FLOODS in `pwd` <--- " 


    if [ -d FOM ]; then
        cd ./FOM
        rm *html
    else
        mkdir FOM
        cd ./FOM
        mv ../../*$string*A${i}*FOM_?.png .
    fi
    #PNGNAME=$1"_FOM_%0d.png"
    #echo $PNGNAME
    #convert -rotate 90 -density 150 -geometry 100% *_FOM.ps $PNGNAME
    echo "---> FLOODS in `pwd` <--- " 
    $IGALCMD
    $SEDCOMMAND -i '/<SPAN [ a-zA-Z0-9]*/ a\   <p class="center"><A href="../../index.html" title="back">Up to module summary</A> </p>  ' index.html
    $SEDCOMMAND -i "s/Index of Pictures/$1 FOM Plots A$i/" index.html
    cd ..

    if [ -d Common ]; then 
        cd ./Common
        rm *html
    else
        mkdir Common
        cd ./Common
        mv ../../*$string*A${i}*E_fits.com_?.png .
    fi
    #PNGNAME=$1"_E_fits.com.%0d.png"
    #convert -rotate 90 -density 150 -geometry 100% *_fits.com.ps $PNGNAME
    $IGALCMD --bigy 750
    $SEDCOMMAND -i '/<SPAN [ a-zA-Z0-9]*/ a\   <p class="center"><A href="../../index.html" title="back">Up to module summary</A> </p>  ' index.html
    $SEDCOMMAND -i "s/Index of Pictures/$1 Common Energy Spectrum Plots A$i/" index.html
    cd ..

    if [ -d Spatial ];  then
        cd ./Spatial
        rm *html
    else
        mkdir Spatial
        cd ./Spatial
        mv ../../*$string*A${i}*E_fits.spat_?.png .
    fi
    PNGNAME=$1"_E_fits.spat.%0d.png"
    #convert -rotate 90 -density 150 -geometry 100% *_fits.spat.ps $PNGNAME
    $IGALCMD --bigy 750
    $SEDCOMMAND -i '/<SPAN [ a-zA-Z0-9]*/ a\   <p class="center"><A href="../../index.html" title="back">Up to module summary</A> </p>  ' index.html
    $SEDCOMMAND -i "s/Index of Pictures/$1 Spatial Energy Spectrum Plots A$i/" index.html

    cd ..

    cd ..

done

# we're going to make the intermediate page her

igal2 -r -w 3 -y 250  --bigy 600
$SEDCOMMAND -i '/<SPAN [ a-zA-Z0-9]*/ a\   <p class="center"><A href="../index.html" title="back">Back to all voltages</A> </p>  ' index.html
$SEDCOMMAND -i '/<P class="small">created with[ a-zA-Z0-9]*/ i\   <p class="center"><A href="./A0/FOM/index.html" title="back">A0 FOM</A> &nbsp; &nbsp;<A href="./A0/Common/index.html" title="back">A0 Common</A> &nbsp; &nbsp; <A href="./A0/Spatial/index.html" title="back">A0 Spatial</A> </p> ' index.html
$SEDCOMMAND -i '/<P class="small">created with[ a-zA-Z0-9]*/ i\   <p class="center"><A href="./A1/FOM/index.html" title="back">A1 FOM</A> &nbsp; &nbsp;<A href="./A1/Common/index.html" title="back">A1 Common</A> &nbsp; &nbsp; <A href="./A1/Spatial/index.html" title="back">A1 Spatial</A> </p> ' index.html
$SEDCOMMAND -i "s/Index of Pictures/$1 Summary/" index.html
