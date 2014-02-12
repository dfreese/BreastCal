#ifndef MODULECAL_H
#define MODULECAL_H

#ifndef ROOT_TObjest
#include "TObject.h"
#endif

class ModuleCal : public TObject {
 public:
  Long64_t ct;
  Double_t ft;
  Float_t Ecal;
  Float_t E;
  Float_t Ec;
  Float_t Ech;
  Float_t x;
  Float_t y;
  Short_t chip;
  Short_t cartridge;
  Short_t fin;
  Short_t m;
  Short_t apd;
  Short_t id;
  Int_t pos;

  ModuleCal();
  virtual ~ModuleCal();
  
  ClassDef(ModuleCal,1)
 };

#endif
