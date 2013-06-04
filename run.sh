./Decode -p -f PT_PED_Data_1211261223_058400u_000100u_L0_0.dat  -v
./Decode -pedfile PT_PED_Data_1211261223_058400u_000100u_L0_0.dat.ped -f PT_DAQ_Data_1211261223_058400u_000100u_L0_0.dat -uv -t -800  -v
 # need to chain here 

 # root file size :: 6.1 MB
./getfloods  -f ./PT_DAQ_Data_1211261223_058400u_000100u_L0_0.dat.root -v
 # root file size :: 6.8 MB
 ./anafloods -f PT_DAQ_Data_1211261223_058400u_000100u_L0_0.dat.root -v 

./enecal -f PT_DAQ_Data_1211261223_058400u_000100u_L0_0.dat.root
./enefit -f PT_DAQ_Data_1211261223_058400u_000100u_L0_0.dat.enecal.root

~/MODULE_ANA/ANA_V5/get_optimal_split -f ./tmp.R1.cal.root --R -rb 1
~/MODULE_ANA/ANA_V5/get_optimal_split -f ./tmp.L0.cal.root --L -rb 0

~/MODULE_ANA/ANA_V5/merge_4up  -f ./tmp.R1.cal.root --R -rb 1 -nc 6 -ts 111378285746
~/MODULE_ANA/ANA_V5/merge_4up  -f ./tmp.L0.cal.root --L -rb 0 -nc 6 -ts 111378285746