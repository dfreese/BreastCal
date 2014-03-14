#ifndef PPeaks_h
#define PPeaks_h
#include "TROOT.h"
#include "TNamed.h"
#include "TFile.h"
#ifndef ROOT_TObject
#include "TObject.h"
#endif

#include "Syspardef.h" 
//#include "/home/miil/MODULE_ANA/ANA_V5/SpeedUp/include/Syspardef.h"

class PPeaks : public TNamed {
 
 public:
  PPeaks(const char *s = "mypeaks") : TNamed(s,""){}
  //  PPeaks() ;
  Double_t spat[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE];
  Double_t com[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE];
 
  virtual ~PPeaks();


 ClassDef(PPeaks,1)
};


 

#endif
