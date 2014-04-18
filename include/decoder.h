#ifndef DECODER_H
#define DECODER_H

#include <fstream>
#include <string>


//#include "config.h"
#include "Util.h"
//#define _HOMEDIR=/Users/arne
//FIXME :: need something more robust
//#include "/Users/arne/root/libInit_avdb.h"
//#include "/Users/arne/root/macros/myrootlib.h"
#include "libInit_avdb.h"
#include "myrootlib.h"

#include "Syspardef.h"


ifstream dataFile;
streampos lastPos;
streampos endPos;
//vector<char> currentPack;
vector<char> packBuffer;
unsigned long long int byteCounter;



// Energy histograms ::
  TH1F *E[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE];
  TH1F *E_com[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE];

   // U - V vectors
#include "TVector.h"
TVector *uu_c[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE];
TVector *vv_c[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE];
Long64_t uventries[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][2]={{{{0}}}};

TDirectory *subdir[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE];

struct pcnumber
{ Short_t panel;
  Short_t cartridge;
};


struct chipevent
{ Long64_t ct;
  Short_t chip;
  Short_t cartridge;
  Short_t module;
  Short_t com1;
  Short_t com2;
  Short_t com1h;
  Short_t com2h;
  Short_t u1;
  Short_t v1;
  Short_t u2;
  Short_t v2;
  Short_t u1h;
  Short_t v1h;
  Short_t u2h;
  Short_t v2h;
  Short_t a;
  Short_t b;
  Short_t c;
  Short_t d;
  Int_t pos;
};




using namespace std;


#endif 
