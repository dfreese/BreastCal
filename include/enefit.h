
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
#include "./decoder.h"
#include "ModuleDat.h"
#include "ModuleCal.h"
#include "TError.h"


Float_t getpeak(TH1F *hist, Float_t xlow, Float_t xhigh, Int_t verbose);
Int_t getcrystal(Double_t x, Double_t y, Double_t xpos[64], Double_t ypos[64], Int_t verbose );
