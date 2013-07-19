#makefile:

LIBRARY=~/root/macros/myrootlib.C


SOURCES_DECODE=./src/decoder.C ./src/util.C ./src/ModuleDat.C 
OBJS_DECODE=$(SOURCES_DECODE:.C=.o)

SOURCES_chainpsf=./src/chain_parsed.C
OBJS_chainpsf=$(SOURCES_chainpsf:.C=.o)

SOURCES_GETFLOODS=./src/getfloods.C ./src/ModuleDat.C 
OBJS_GETFLOODS=$(SOURCES_GETFLOODS:.C=.o)

SOURCES_ANAFLOODS=./src/anafloods_psf_v2.C ./src/apd_sort16_v2.C ./src/findnext.C ./src/detCluster.C
OBJS_ANAFLOODS=$(SOURCES_ANAFLOODS:.C=.o)

SOURCES_ENECAL=./src/enecal.C
OBJS_ENECAL=$(SOURCES_ENECAL:.C=.o)

SOURCES_ENEFIT=./src/enefit.C ./src/apd_fit.C ./src/ModuleCal.C ./src/ModuleDat.C ModuleDatDict.C
OBJS_ENEFIT=$(SOURCES_ENEFIT:.C=.o)

#SOURCES_MERGE_4up=./src/merge_4up.C ./src/ModuleCal.C 
SOURCES_MERGE_4up=./src/merge_4up.C 
OBJS_MERGE_4up=$(SOURCES_MERGE_4up:.C=.o)

#SOURCES_MERGE_PANEL=./src/merge_panel.C ./src/ModuleCal.C 
SOURCES_MERGE_PANEL=./src/merge_panel.C 
OBJS_MERGE_PANEL=$(SOURCES_MERGE_PANEL:.C=.o)

#SOURCES_MERGECOINC:=./src/merge_coinc.C ./src/ModuleCal.C ./src/CoincEvent.C ./ModuleDatDict.C
SOURCES_MERGECOINC:=./src/merge_coinc.C ./src/ModuleCal.C ./src/CoincEvent.C 
OBJS_MERGECOINC=$(SOURCES_MERGECOINC:.C=.o)

SOURCES_FREC:=./src/format_recon.C
OBJS_FREC=$(SOURCES_FREC:.C=.o)

SOURCES_MANA:=./src/merge_ana.C
OBJS_MANA=$(SOURCES_MANA:.C=.o)

SOURCES_chainmerge=./src/chain_merged.C
OBJS_chainmerge=$(SOURCES_chainmerge:.C=.o)

SOURCES_FOM=./src/fom_ana.C ./src/apd_peaktovalley.C
OBJS_FOM=$(SOURCES_FOM:.C=.o)

SOURCES_GLFIT=./src/apd_fit.C ./src/glob_fit_results.C
OBJS_GLFIT=$(SOURCES_GLFIT:.C=.o)

#CFLAGS =  -Wall -W -g -DDEBUG -fPIC 
#CFLAGS =  -Wall -Wno-unused-label -W -g -fPIC -I$(HOME)/root/macros -I./include
CFLAGS =  -Wall -W -g -fPIC -I$(HOME)/root/macros -I./include
CXXFLAGS = $(CFLAGS) $(shell root-config --cflags) -O3
MYLIB = -L$(HOME)/root/macros -lmyrootlib -lInit_avdb
DICTLIB = -L./lib -lModuleDatDict
LDFLAGS = $(shell root-config --glibs) $(shell root-config --libs) -lMinuit -lSpectrum -lHistPainter $(MYLIB) $(DICTLIB)
CC =g++

.PHONY:	clean


all: ./lib/libModuleDatDict.so ./bin/Decode ./bin/chain_parsed ./bin/getfloods ./bin/anafloods ./bin/enecal ./bin/enefit ./bin/get_optimal_split ./bin/merge_4up ./bin/merge_panel ./bin/merge_coinc ./bin/chain_merged ./bin/mana ./bin/fom_ana ./bin/glob_fit_results ./bin/format_recon

# 
#UYORUK_toROOT_V1 resizedat resizedat_v2 getfloods anafloods enecal enefit mergecal fom_ana caltime

./ModuleDatDict.C: ./include/ModuleDat.h ./include/ModuleCal.h ./include/CoincEvent.h ./include/LinkDef.h
	rootcint -f $@ -c $(CXXFLAGS) -p $^

#shared library .. can be static I think.

./lib/libModuleDatDict.so: ./ModuleDatDict.C ./src/ModuleDat.C ./src/ModuleCal.C ./src/CoincEvent.C
	g++ -shared -o $@ `root-config --ldflags` $(CXXFLAGS) -I$(ROOTSYS)/include $^


./bin/Decode: $(OBJS_DECODE)
	 $(CC) $(CXXFLAGS) $(LDFLAGS) $(OBJS_DECODE) -o $@

./bin/chain_parsed: $(OBJS_chainpsf)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(OBJS_chainpsf) -o $@

./bin/getfloods: $(OBJS_GETFLOODS)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(OBJS_GETFLOODS) -o $@

./bin/anafloods: $(OBJS_ANAFLOODS)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(OBJS_ANAFLOODS) -o $@
# should get rid of enecal en assign crystal id's in anafloods

./bin/enecal: $(OBJS_ENECAL)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(OBJS_ENECAL) -o $@

./bin/enefit: $(OBJS_ENEFIT)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(OBJS_ENEFIT) -o $@

./bin/get_optimal_split: ./src/get_opt_split.o
	$(CC) $(CXXFLAGS) $(LDFLAGS) ./src/get_opt_split.o  -o $@

./bin/merge_4up: $(OBJS_MERGE_4up)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(OBJS_MERGE_4up) -o $@

./bin/merge_panel: $(OBJS_MERGE_PANEL)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(OBJS_MERGE_PANEL) -o $@

./bin/merge_coinc: $(OBJS_MERGECOINC)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(OBJS_MERGECOINC) -o $@

./bin/chain_merged: $(OBJS_chainmerge)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(OBJS_chainmerge) -o $@

./bin/mana: $(OBJS_MANA)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(OBJS_MANA) -o $@

./bin/fom_ana: $(OBJS_FOM)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(OBJS_FOM) -o $@

./bin/glob_fit_results: $(OBJS_GLFIT)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(OBJS_GLFIT) -o $@

./bin/format_recon: $(OBJS_FREC)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(OBJS_FREC) -o $@


.C.o: $(HDR) 
	$(CC) -c $(CXXFLAGS) $< -o $@

#Parsetomodule: Parsetomodule.o
#	g++

#RENAtoROOT_v1_1: RENAtoROOT_v1_1.o 
#	g++ 

#read_singles_file: read_singles_file.o

clean: 
	rm -f ./src/*.o
	if [[ -e ModuleDatDict.C ]]; then rm ./ModuleDatDict.* ;fi
	rm -f ./lib/*.so
	rm -f ./bin/Decode ./bin/chain_parsed ./bin/getfloods ./bin/anafloods ./bin/enecal ./bin/enefit ./bin/get_optimal_split ./bin/merge_4up ./bin/merge_panel ./bin/merge_coinc ./bin/chain_merged ./bin/mana ./bin/fom_ana ./bin/glob_fit_results