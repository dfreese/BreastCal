#makefile:

# AVDB JUL 2013

# make clean :: removes libs, bins and *o
# make .rules :: make dependencies
# make :: will generate hidden files with rules and dependencies ( a bit ugly but it's because we have a variety of dependencies )
# make all :: will compile everything 

# note source files that start with a capital letter are just a bunch of functions and don't have a "main {} "

ROOTMACRODIR=$(HOME)/root/macros
ROOTMACRODIR_LIB=$(ROOTMACRODIR)/lib
ROOTMACRODIR_INC=$(ROOTMACRODIR)/include

#
CFLAGS =  -Wall -Wno-deprecated -W -g -fPIC -I$(ROOTMACRODIR_INC) -I./include -I$(ROOTSYS)/include
CXXFLAGS = $(CFLAGS) $(shell root-config --cflags) -O3
MYLIB = -L$(ROOTMACRODIR_LIB) -lmyrootlib -lInit_avdb
DICTLIB = -L./lib -lModuleAna
LDFLAGS = $(shell root-config --glibs) $(shell root-config --libs) -lMinuit -lSpectrum -lHistPainter -lTreePlayer -lProof $(MYLIB) $(DICTLIB)
CC =g++


SOURCE_DIR  := src
INCLUDE_DIR := include
LIB_DIR     := lib

# note: the make implicit rule to compile C++ programs uses .C or .cc extension only.
CC_EXTENSION := C
CLASS_EXTENSION := cc

PRG_SUFFIX_FLAG := 0
SRCS := $(wildcard $(SOURCE_DIR)/*.$(CC_EXTENSION))
OBJS := $(SRCS:%.C=%.o) $(CLASSES_C:%.cc=%.o)
HDRS  = $(wildcard  $(INCLUDE_DIR)/*.h)
PRGS := $(patsubst $(SOURCE_DIR)/%.C,%,$(SRCS))
PRG_SUFFIX=
BINNS := $(patsubst %,./bin/%$(PRG_SUFFIX),$(PRGS))
BINS := $(shell  echo $(BINNS) |  sed  's/\(\.\/bin\/[A-Z]\+[A-Za-z_0-9]\+\)\b//g') #| grep -v "[A-Z]*" )
HDRONLY := Syspardef.h

CLASSES_C := $(wildcard $(SOURCE_DIR)/*.$(CLASS_EXTENSION))
CLASSES_H := $(patsubst $(SOURCE_DIR)/%.cc,$(INCLUDE_DIR)/%.h,$(CLASSES_C))
CLASSES_H := $(CLASSES_H) $(INCLUDE_DIR)/LinkDef.h 

SOLIBS := ./lib/libModuleDat.so 
#./lib/Sel_GetFloods.so
#SOLIBS_DICT := $(patsubst ./lib/lib%.so,%Dict.C,$(SOLIBS))

#depend: .depend .rules
#	@echo "Done with dependencies"

#default: ./setana_env.sh $(SOLIBS_DICT) $(SOLIBS)  $(BINS)
default: ./setana_env.sh ./ModuleAnaDict.C ./lib/libModuleAna.so .rules $(BINS) 
	./PAR/MakePAR.sh
	@echo "Done making ANA CODE"

all:	 default 

check:
	@echo $(CLASSES_H)
	@echo $(CLASSES_C)
	@echo $(OBJS)

.PHONY:	clean print 

.depend: $(SRCS) $(CLASSES_C)
	rm -f ./.depend
	@echo $(SRCS)
	$(CC) $(CFLAGS) -MM $^  >> ./.depend;
	sed -i 's/\(^[[:alnum:]_]\+\.o\)/src\/&/g' ./.depend

.rules: .depend
	@echo "Making rules for SW"
	$(shell cat .depend |  tr ' ' '\n' | grep -v $(ROOTSYS) | sed '/\\/d' | grep -v $(HDRONLY) | grep -v myrootlib | grep -v libInit_avdb  | grep "[[:alnum:]]" | tr '\n' ' '  | sed  's/src\/[[:alnum:]_]*.o:/\n&/g'  | sed '/^$$/d' > .buildrules; echo "" >> .buildrules)
	@cat .buildrules | sed 's/^src/bin/g' | sed 's/include/src/g' | sed 's/\.o//' | sed 's/\.[Ch]/\.o/g' | sed 's/\.\///g'  |  awk '{ while(++i<=NF) printf (!a[$$(i)]++) ? $$(i) FS : ""; i=split("",a); print "" }'  |  sed '/^bin\/decoder/!s/[a-zA-Z/]*decoder\.o//'   | awk '{print $$(0)"\n\t $$(CC) $$(CXXFLAGS) $$(LDFLAGS) $$^ -o $$@"}' > .binrules

-include .binrules
-include .depend

#.cxx.o:
#	$(CC) -c $(CXXFLAGS) $< -o $@

#%.o: .cxx
#	$(CC) -c $(CXXFLAGS) $< -o $@

print: 
	 @echo $(SRCS)
	 @echo $(OBJS)
	@echo $(HDRS)
	@echo -n "BINNS:  "
	@echo $(BINNS)
	@echo -n "BINS:  "
	@echo $(BINS)

./ModuleAnaDict.C: $(CLASSES_H) 
	rootcint -f $@ -c $(CXXFLAGS) -p $^

#./Sel_GetFloodsDict.C: $(CLASSES_H) 
#	rootcint -f $@ -c $(CXXFLAGS) -p $^

#./PPeaks.C: $(CLASSES_H)
#	rootcint -f $@ -c $(CXXFLAGS) -p $^

#shared library .. can be static I think.

./lib/libModuleAna.so: ./ModuleAnaDict.C $(CLASSES_C)
	g++ -shared -o $@ `root-config --ldflags --glibs` $(CXXFLAGS) -I$(ROOTSYS)/include $^

#./lib/Sel_GetFloods.so: ./Sel_GetFloodsDict.C $(CLASSES_C)
#	g++ -shared -o $@ `root-config --ldflags` $(CXXFLAGS) -I$(ROOTSYS)/include $^


#./setana_env.sh:
#	echo "CURDIR=`pwd`" > ./setana_env.sh
#	echo "source `pwd`/src/addtopath.sh `pwd`" > ./setana_env.sh

# GENERATED WITH ::
# 1. change whitespace to endline
# 2. remove all lines with ROOT in them
# 3. remove empty lines
# 4. remove all lines with home
# 5. turn \n into whitesace
# 6. split line wherever there's an object

# cat  .depend |  tr ' ' '\n' | sed '/ROOT/d' | sed '/^$/d' | sed '/\\/d' | grep -v home | tr '\n' ' ' | sed  's/ [[:alnum:]_]*.o:/\n&/g'  > buildrules
# echo  "" >> buildrules

# 1. Replace include with src
# 2. Get rid of the .o at start of lines  ( that would result in binaries )
# 3. Replace .C and .h with .o files ( that would be the objects )
# 4. Replace ./ with ""
# 5. Use awk to get rid of duplicates
# 6. Remove decoder.o from all lines except the decoder rule
# 7. Add ./bin to sentence start
# 8. Add Compile line 

#cat buildrules | sed 's/include/src/g' | sed 's/\.o//' | sed 's/\.[Ch]/\.o/g' | sed 's/\.\///g'  |  awk '{ while(++i<=NF) printf (!a[$i]++) ? $i FS : ""; i=split("",a); print "" }'|  sed '/^decoder/!s/[a-zA-Z/]*decoder\.o//'    | sed 's/^/bin\//'  | awk '{print $0"\n\t $(CC) $(CXXFLAGS) $(LDFLAGS) $^ -o $@"}' 


clean: 
	rm .depend
	rm .buildrules
	rm .binrules
	rm -f ./src/*.o
	if [[ -e ModuleDatDict.C ]]; then rm ./ModuleDatDict.* ;fi
	rm -f ./lib/*.so
	rm -f ./bin/decoder ./bin/chain_parsed ./bin/getfloods ./bin/anafloods ./bin/enecal ./bin/enefit ./bin/get_optimal_split ./bin/merge_4up ./bin/merge_panel ./bin/merge_coinc ./bin/chain_merged ./bin/mana ./bin/fom_ana ./bin/glob_fit_results