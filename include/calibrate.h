
#include "TROOT.h"
#include "TH1F.h"
#include "Riostream.h"
#include "TTree.h"
#include "TFile.h"
#include "TVector.h"
#include "TCanvas.h"
#include "TPaveText.h"
#include "Apd_Fit.h"
#include "TMath.h"
#include "TSpectrum.h"
#include "ModuleDat.h"
//#include "/home/miil/MODULE_ANA/ANA_V5/SpeedUp/include/ModuleCal.h"
#include "ModuleCal.h"
#include "TError.h"
#include "TChain.h"

#define MINPIXENTRIES 1000
#define NOVALIDPEAKFOUND -8989898

Float_t getpeak(TH1F *hist, Float_t xlow, Float_t xhigh, Bool_t force, Int_t verbose);
Int_t getcrystal(Double_t x, Double_t y, Double_t xpos[64], Double_t ypos[64], Int_t verbose );
Float_t getmean(Float_t *array);