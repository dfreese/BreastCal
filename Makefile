#makefile:

LIBRARY=~/root/macros/myrootlib.C

SOURCES_DECODE=decoder.cpp config.cpp util.cpp
OBJS_DECODE=$(SOURCES_DECODE:.cpp=.o)

SOURCES_chainpsf=chain_parsed.C
OBJS_chainpsf=$(SOURCES_chainpsf:.C=.o)

SOURCES_GETFLOODS=getfloods.C 
OBJS_GETFLOODS=$(SOURCES_GETFLOODS:.C=.o)

SOURCES_ANAFLOODS=anafloods_psf_v2.C apd_sort16_v2.C findnext.C detCluster.C
OBJS_ANAFLOODS=$(SOURCES_ANAFLOODS:.C=.o)

SOURCES_ENECAL=enecal.C
OBJS_ENECAL=$(SOURCES_ENECAL:.C=.o)

SOURCES_ENEFIT=enefit.C apd_fit.C
OBJS_ENEFIT=$(SOURCES_ENEFIT:.C=.o)

SOURCES_MERGE_4up=merge_4up.C
OBJS_MERGE_4up=$(SOURCES_MERGE_4up:.C=.o)

SOURCES_MERGE_PANEL=merge_panel.C
OBJS_MERGE_PANEL=$(SOURCES_MERGE_PANEL:.C=.o)

SOURCES_MERGECOINC:=merge_coinc.C
OBJS_MERGECOINC=$(SOURCES_MERGECOINC:.C=.o)

SOURCES_MERGERAW:=mergeraw.C
OBJS_MERGERAW=$(SOURCES_MERGERAW:.C=.o)

SOURCES_FREC:=format_recon.C
OBJS_FREC=$(SOURCES_FREC:.C=.o)

SOURCES_MANA:=merge_ana.C
OBJS_MANA=$(SOURCES_MANA:.C=.o)

SOURCES_chainmerge=chain_merged.C
OBJS_chainmerge=$(SOURCES_chainmerge:.C=.o)

SOURCES_FOM=fom_ana.C apd_peaktovalley.C
OBJS_FOM=$(SOURCES_FOM:.C=.o)

SOURCES_GLFIT=apd_fit.C glob_fit_results.C
OBJS_GLFIT=$(SOURCES_GLFIT:.C=.o)

#CFLAGS =  -Wall -W -g -DDEBUG -fPIC 
CFLAGS =  -Wall -W -g -fPIC -I$(HOME)/root/macros
CXXFLAGS = $(CFLAGS) $(shell root-config --cflags) -O3
MYLIB = -L$(HOME)/root/macros -lmyrootlib -lInit_avdb
LDFLAGS = $(shell root-config --glibs) $(shell root-config --libs) -lMinuit -lSpectrum -lHistPainter $(MYLIB)
CC =g++

.PHONY:	clean


all: Decode chain_parsed getfloods anafloods enecal enefit get_optimal_split merge_4up merge_panel merge_coinc mergeraw chain_merged mana fom_ana glob_fit_results
# 
#UYORUK_toROOT_V1 resizedat resizedat_v2 getfloods anafloods enecal enefit mergecal fom_ana caltime

Decode: $(OBJS_DECODE)
	 $(CC) $(CXXFLAGS) $(LDFLAGS) $(OBJS_DECODE) -o $@

chain_parsed: $(OBJS_chainpsf)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(OBJS_chainpsf) -o $@

getfloods: $(OBJS_GETFLOODS)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(OBJS_GETFLOODS) -o $@

anafloods: $(OBJS_ANAFLOODS)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(OBJS_ANAFLOODS) -o $@
# should get rid of enecal en assign crystal id's in anafloods
enecal: $(OBJS_ENECAL)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(OBJS_ENECAL) -o $@

enefit: $(OBJS_ENEFIT)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(OBJS_ENEFIT) -o $@

get_optimal_split: get_opt_split.o
	$(CC) $(CXXFLAGS) $(LDFLAGS) get_opt_split.o  -o $@

merge_4up: $(OBJS_MERGE_4up)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(OBJS_MERGE_4up) -o $@

merge_panel: $(OBJS_MERGE_PANEL)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(OBJS_MERGE_PANEL) -o $@

merge_coinc: $(OBJS_MERGECOINC)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(OBJS_MERGECOINC) -o $@

mergeraw: $(OBJS_MERGERAW)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(OBJS_MERGERAW) -o $@
	
chain_merged: $(OBJS_chainmerge)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(OBJS_chainmerge) -o $@

mana: $(OBJS_MANA)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(OBJS_MANA) -o $@

fom_ana: $(OBJS_FOM)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(OBJS_FOM) -o $@

glob_fit_results: $(OBJS_GLFIT)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(OBJS_GLFIT) -o $@

.C.o: $(HDR) 
	$(CC) -c $(CXXFLAGS) $< -o $@

#Parsetomodule: Parsetomodule.o
#	g++

#RENAtoROOT_v1_1: RENAtoROOT_v1_1.o 
#	g++ 

#read_singles_file: read_singles_file.o

clean: 
	-rm *.o
