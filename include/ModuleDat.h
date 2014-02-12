#ifndef MODULEDAT_H
#define MODULEDAT_H

#ifndef ROOT_TObject
#include "TObject.h"
#endif

class ModuleDat : public TObject {
  //class ModuleDat  {
 public:
  Long64_t ct;
  Short_t cartridge;
  Short_t fin;
  Short_t chip;
  Short_t module;
  Short_t apd;
  Float_t Ec;
  Float_t Ech;
  Float_t x;
  Float_t y;
  Float_t E;
  // this is double because I store u and v in it during decoding
  Double_t ft;
  Int_t pos;
  Short_t a;
  Short_t b;
  Short_t c;
  Short_t d;
  Short_t id;
 
  ModuleDat(); 
  //  ~ModuleDat(); 
  virtual ~ModuleDat();

  ClassDef(ModuleDat,4) 
};



#endif
