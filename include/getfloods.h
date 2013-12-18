#ifndef GETFLOODS_H
#define GETFLOODS_H

#include "TError.h" 
#include "TROOT.h"
#include "Riostream.h"
#include "TTree.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TSpectrum.h"
#include "TH2F.h"
#include "TVector.h"
#include "TMath.h"
#include "./decoder.h"
#include "./ModuleDat.h"


#define FILENAMELENGTH	120
#define MAXFILELENGTH	160

//#define CHIPS 2



Double_t GetPhotopeak_v1(TH1F *hist,Double_t pp_low,Double_t pp_up,Int_t verbose, Int_t width=10);
Double_t GetPhotopeak_v2(TH1F *hist,Double_t pp_low,Double_t pp_up,Int_t verbose);
Int_t	   makeplot(TH1F *hist[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE],Int_t c, Int_t f, Int_t NRFLOODSTODRAW,Char_t suffix[40],Char_t filebase[FILENAMELENGTH]) ;
Int_t	   makeplot(TH2F *hist[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE],Int_t c, Int_t f, Int_t NRFLOODSTODRAW,Char_t suffix[40],Char_t filebase[FILENAMELENGTH]) ;

 
#endif
