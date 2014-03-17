
#!/bin/bash

# check dir:
THISDIR=`pwd | sed 's/^.*\///'`
GOBACK=0

if [ $THISDIR != "PAR" ]; then
 GOBACK=1
 if [ -d PAR ] ; then
 cd PAR
 else
 echo "MakePAR.sh launched from wrong directory. Exiting."
 exit -1
 fi
fi

for i in `ls -1`; do 
    if [ -d  $i ]; then 
	rm $i.par; 
	tar zcfh $i.par $i ; 
    fi  ; 
done

if [ $GOBACK -eq 1 ]; then
 cd ..
fi  