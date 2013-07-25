#ifndef APD_FIT_INC
#define APD_FIT_INC
#define FITDEBUG
#define PEAKS 64
#define SATURATION 2500

#include "TROOT.h"
#include "TMath.h"
#include "TF1.h"
#include "TH1F.h"
#include "TSpectrum2.h"
#include "TSpectrum.h"
#include "Riostream.h"
#include "TCanvas.h"
#include "TGraphErrors.h"
//#include </products/ROOT/root_v5.16.00/include/TSpectrum2.h>

#ifndef H_GLOBALS_FIT
#define H_GLOBALS_FIT
extern Float_t escape[PEAKS][4];
extern TH1F *mean,*fwhm,*chndf,*fwhmE;
extern TH1F *mean3,*fwhm3,*chndf3,*fwhmE3;
extern TGraphErrors *mean2,*fwhm2;
extern TGraph *chndf2;
extern Double_t means[PEAKS],fwhms[PEAKS],chndfs[PEAKS];
extern Double_t meansE[PEAKS],fwhmsE[PEAKS],x[PEAKS],xe[PEAKS];
extern Int_t validEflag[PEAKS];                                   
//extern TF1 *ffunc[PEAKS];
#endif

#endif


Double_t Background(Double_t*, Double_t*);
Double_t Signal(Double_t*, Double_t*);
Double_t Doublegaus(Double_t*, Double_t*);
Double_t Total(Double_t*, Double_t*);
Double_t Total2(Double_t*, Double_t*);
Double_t Total3(Double_t*, Double_t*);
Double_t Total4(Double_t*, Double_t*);
//TF1* fitapdspec(TH1F*, Float_t ,Float_t,Int_t, Int_t);
TF1* fitapdspec(TH1F *hist, Float_t xlow=0,Float_t xhigh=25,Int_t com=0,Int_t verbose=0);
//Int_t fitall(TH1F**,Double_t*,Double_t[64][9] ,Float_t , Float_t , TCanvas*,Int_t);
Int_t fitall(TH1F *hi[PEAKS],TF1 *ffunc[PEAKS],Double_t *values,Double_t pixvals[PEAKS][9],Float_t lowfit, Float_t hifit, TCanvas *c1,Int_t verbose=0);
//Int_t writ(TH1F**, TCanvas*, Char_t[40]  );
Int_t writ(TH1F *hi[PEAKS] , TF1 *ff[PEAKS],TCanvas *ccc, Char_t filename[40] = "outputfile.ps" ,Int_t drawfunc=0);

