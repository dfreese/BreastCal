#makefile:

# AVDB JUL 2013

# make clean :: removes libs, bins and *o
# make :: will generate hidden files with rules and dependencies ( a bit ugly but it's because we have a variety of dependencies )
# make all :: will compile everything 

# note source files that start with a capital letter are just a bunch of functions and don't have a "main {} "

LIBRARY=~/root/macros/myrootlib.C

#
CFLAGS =  -Wall -Wno-deprecated -W -g -fPIC -I$(HOME)/root/macros -I./include -I$(ROOTSYS)/include
CXXFLAGS = $(CFLAGS) $(shell root-config --cflags) -O3
MYLIB = -L$(HOME)/root/macros -lmyrootlib -lInit_avdb
DICTLIB = -L./lib -lModuleDatDict
LDFLAGS = $(shell root-config --glibs) $(shell root-config --libs) -lMinuit -lSpectrum -lHistPainter $(MYLIB) $(DICTLIB)
CC =g++


SOURCE_DIR  := src
INCLUDE_DIR := include
LIB_DIR     := lib

CC_EXTENSION := C
PRG_SUFFIX_FLAG := 0
SRCS := $(wildcard $(SOURCE_DIR)/*.$(CC_EXTENSION))
OBJS := $(SRCS:%.C=%.o)
HDRS  = $(wildcard  $(INCLUDE_DIR)/*.h)
PRGS := $(patsubst $(SOURCE_DIR)/%.C,%,$(SRCS))
PRG_SUFFIX=
BINNS := $(patsubst %,./bin/%$(PRG_SUFFIX),$(PRGS))
BINS := $(shell  echo $(BINNS) |  sed  's/\(\.\/bin\/[A-Z]\+[A-Za-z_0-9]\+\)\b//g') #| grep -v "[A-Z]*" )

SOLIBS := ./lib/libModuleDatDict.so


#depend: .depend .rules
#	@echo "Done with dependencies"

default: ./ModuleDatDict.C ./lib/libModuleDatDict.so  $(BINS)
	@echo "Done making ANA CODE"

all:	default

.PHONY:	clean print 

.depend: $(SRCS)
	rm -f ./.depend
	$(CC) $(CFLAGS) -MM $^  >> ./.depend;
	sed -i 's/\(^[[:alnum:]_]\+\.o\)/src\/&/g' ./.depend

.rules: .depend
	$(shell cat .depend |  tr ' ' '\n' | sed '/ROOT/d' | sed '/\\/d' | grep -v home | grep "[[:alnum:]]" | tr '\n' ' ' | sed  's/src\/[[:alnum:]_]*.o:/\n&/g'  | sed '/^$$/d' > .buildrules; echo "" >> .buildrules)
	@cat .buildrules | sed 's/^src/bin/g' | sed 's/include/src/g' | sed 's/\.o//' | sed 's/\.[Ch]/\.o/g' | sed 's/\.\///g'  |  awk '{ while(++i<=NF) printf (!a[$$(i)]++) ? $$(i) FS : ""; i=split("",a); print "" }'  |  sed '/^bin\/decoder/!s/[a-zA-Z/]*decoder\.o//'   | awk '{print $$(0)"\n\t $$(CC) $$(CXXFLAGS) $$(LDFLAGS) $$^ -o $$@"}' > .binrules

-include .binrules
-include .depend

#.C.o:
#	$(CC) -c $(CXXFLAGS) $< -o $@

#%.o: .c
#	$(CC) -c $(CXXFLAGS) $< -o $@

print: 
	 @echo $(SRCS)
	 @echo $(OBJS)
	@echo $(BINS)
	@echo $(HDRS)


./ModuleDatDict.C: ./include/ModuleDat.h ./include/ModuleCal.h ./include/CoincEvent.h ./include/LinkDef.h
	rootcint -f $@ -c $(CXXFLAGS) -p $^

#shared library .. can be static I think.

./lib/libModuleDatDict.so: ./ModuleDatDict.C ./src/ModuleDat.C ./src/ModuleCal.C ./src/CoincEvent.C
	g++ -shared -o $@ `root-config --ldflags` $(CXXFLAGS) -I$(ROOTSYS)/include $^



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