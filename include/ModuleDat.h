#ifndef MY_MODULEDAT
#define MY_MODULEDAT

#ifndef ROOT_TObject
#include "TObject.h"
#endif

class ModuleDat : public TObject {
  //class ModuleDat  {
 public:
  Long64_t ct;
  Short_t chip;
  Short_t module;
  Short_t apd;
  Float_t Ec;
  Float_t Ech;
  Float_t x;
  Float_t y;
  Float_t E;
  Float_t ft;
  Int_t pos;
  Short_t a;
  Short_t b;
  Short_t c;
  Short_t d;
  Short_t id;
 
  ModuleDat(); 
  //  ~ModuleDat(); 
  virtual ~ModuleDat();

  ClassDef(ModuleDat,1) 
};



#endif
