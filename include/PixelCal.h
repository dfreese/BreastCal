#ifndef PixelCal_h
#define PixelCal_h
#include "TROOT.h"
#include "TNamed.h"
#ifndef ROOT_TObject
#include "TObject.h"
#endif
#include "TFile.h"
#include "Riostream.h"
#include "TH2S.h"
#include "TMath.h"
//#include "/home/miil/MODULE_ANA/ANA_V5/SpeedUp/include/Syspardef.h"
#include "Syspardef.h"

class PixelCal : public TNamed {
 
 public:
  PixelCal(const char *s = "mypeaks") : TNamed(s,""){}
  //  PixelCal() ;
  Float_t X[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE][XTALS_PER_APD];
  Float_t Y[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE][XTALS_PER_APD];
  Float_t GainSpat[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE][XTALS_PER_APD];
  Float_t GainCom[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE][XTALS_PER_APD];
  Float_t EresSpat[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE][XTALS_PER_APD];
  Float_t EresCom[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE][XTALS_PER_APD];
  Bool_t validpeaks[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE];
  TH2S *floodmap[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE];
  Bool_t floodlut;


  //FIXME perhaps make floodlut private to the class
  Int_t ReadCal(const char *filebase);
  Int_t GetCrystalId(Float_t x, Float_t y, Int_t c, Int_t f, Int_t i, Int_t j);
  virtual ~PixelCal();
  void SetVerbose(Bool_t val){ verbose=val;}
  Int_t WriteCalTxt(const char *filebase);
 private:
  Bool_t verbose;


 ClassDef(PixelCal,1)
};


 

#endif
