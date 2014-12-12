#ifndef DECODER_H
#define DECODER_H

#include <fstream>
#include <string>
#include "Util.h"
#include "libInit_avdb.h"
#include "myrootlib.h"
#include "Syspardef.h"

// Energy histograms ::
TH1F *E[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE];
TH1F *E_com[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE];

// U - V vectors
#include "TVector.h"
TVector *uu_c[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE];
TVector *vv_c[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE];
Long64_t uventries[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][2]={{{{0}}}};

TDirectory *subdir[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE];

using namespace std;

#endif 
