#!/bin/bash

#delay is coarse time steps
DELAY=${3}
FILES=$2
BASE=$1


for i in `seq 0 ${FILES}`; do 
merge_coinc -fl ./${BASE}_part${i}_L.panel.root -fr ./${BASE}_part${i}_R.panel.root -d ${DELAY};
done;

chain_merged -r -d ${DELAY} -n ${FILES} -f ./${BASE}
merge_ana -r -f ./${BASE}_all.delaywindow${DELAY}.merged.root

format_recon -r -f ./${BASE}_all.delaywindow${DELAY}.merged.randoms.root -p 61