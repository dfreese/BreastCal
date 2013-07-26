#ifndef ENECAL_H
#define ENECAL_H

#include "TROOT.h"
#include "Riostream.h"
#include "TTree.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TSpectrum.h"
#include "TSpectrum2.h"
#include "TH2F.h"
#include "TVector.h"
#include "TMath.h"
#include "TF1.h"
#include "./decoder.h"
#include "./ModuleDat.h"
Int_t getcrystal(Double_t x, Double_t y, Double_t xpos[64], Double_t ypos[64], Int_t verbose );


Double_t finecalc(Double_t uv, Float_t u_cent, Float_t v_cent);


#endif
