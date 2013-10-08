#makefile:
LIBRARY=/home/miil/root/macros/myrootlib.C



SOURCES_calc_average=calc_average.C
OBJS_calc_average=$(SOURCES_calc_average:.C=.o)

CFLAGS =  -Wall -W -g -DDEBUG -fPIC 
CXXFLAGS = $(CFLAGS) $(shell root-config --cflags) -O3
MYLIB = -L/home/miil/root/macros -lmyrootlib -lInit_avdb
LDFLAGS = $(shell root-config --glibs) $(shell root-config --libs) -lMinuit -lSpectrum -lHistPainter $(MYLIB)
CC =g++

.PHONY:	clean

all: calc_average

calc_average: $(OBJS_calc_average)
	 $(CC) $(CXXFLAGS) $(LDFLAGS) $(OBJS_calc_average) -o $@


clean: 
	-rm *.o