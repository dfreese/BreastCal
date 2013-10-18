#ifndef MY_COINCEVENT
#define MY_COINCEVENT

#ifndef ROOT_TObjest
#include "TObject.h"
#endif

#define CALFILENAMELENGTH FILENAMELENGTH+50

class CoincEvent : public TObject {
 public:
  Long64_t dtc;
  Double_t dtf;
  Float_t E1;
  Float_t Ec1;
  Float_t Ech1;
  Float_t ft1;
  Float_t E2;
  Float_t Ec2;
  Float_t Ech2;
  Float_t ft2;
  Float_t x1;
  Float_t y1;
  Float_t x2;
  Float_t y2;
  Short_t chip1;
  Short_t fin1;
  Short_t m1;
  Short_t apd1;
  Short_t crystal1;
  Short_t chip2;
  Short_t fin2;
  Short_t m2;
  Short_t apd2;
  Short_t crystal2;
  Int_t pos;
 
  CoincEvent();
  virtual ~CoincEvent();

  ClassDef(CoincEvent,2)

};


#endif
