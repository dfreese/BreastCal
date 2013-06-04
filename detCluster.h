#include <TROOT.h>
#include <TH2F.h>
#include <Riostream.h>


struct bin {
  Int_t x;
  Int_t y;
  Float_t val;
  Int_t pixels;
};

bin ClusterSize(TH2F *hist, Int_t xbin, Int_t ybin);
Int_t countcolumn(TH2F *hist, Int_t binx,Int_t biny, Int_t order, bin *max);
