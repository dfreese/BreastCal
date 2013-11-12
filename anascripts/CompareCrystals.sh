 DIR1=$1
 DIR2=$2

 if [ -e alllocdiff ]; then rm alllocdiff; fi;
 if [ -e allEdiff ]; then rm allEdiff; fi;
  for i in L ; do 
#    for j in `seq 0 3`; do 
    for j in `seq 0 1`; do 
      for r in `seq 0 7`; do 
        for m in `seq 0 3`; do 
          for k in 0 1; do 
            if [ -e ./${i}${j}/CHIPDATA/PT_DAQ_${DIR1}_${i}${j}.RENA${r}.unit${m}_apd${k}_cal.txt ]; then 
                paste ./${i}${j}/CHIPDATA/PT_DAQ_${DIR1}_${i}${j}.RENA${r}.unit${m}_apd${k}_cal.txt ../${DIR2}/${i}${j}/CHIPDATA/PT_DAQ_${DIR2}_${i}${j}.RENA${r}.unit${m}_apd${k}_cal.txt  | awk '{print sqrt( ($1-$7)*($1-$7) + ($2-$8)*($2-$8) )}' >> alllocdiff ;  
                paste ./${i}${j}/CHIPDATA/PT_DAQ_${DIR1}_${i}${j}.RENA${r}.unit${m}_apd${k}_cal.txt ../${DIR2}/${i}${j}/CHIPDATA/PT_DAQ_${DIR2}_${i}${j}.RENA${r}.unit${m}_apd${k}_cal.txt  | awk '{print  $3/$9" "$5/$11}' >> allEdiff ; 
	    fi ; 
          done; 
        done; 
      done; 
    done;
 done;
