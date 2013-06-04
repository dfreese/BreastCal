//#define INFO

#include "TH1F.h"
#include "TF1.h"
#include "TNtuple.h"
#include "TCanvas.h"
#include "TPaveText.h"
//#include "/Users/arne/root/macros/myrootlib.h"
//#include "/home/miil/root/macros/myrootlib.h" 
//#include "libInit_avdb.h"
//#include "myrootlib.h"


#define PI 3.141592
#define NRPOSBINS 500
#define POSMIN -1.2
#define POSMAX 1.2
#define ANGER 1
#define CHIN 2

/*
#ifndef H_GLOBALS_PTV
#define H_GLOBALS_PTV
extern TF1 *posgausfits[PEAKS];
extern TH1F *xsums[8];         
extern TH1F *tacs2[64];
#endif
*/


Int_t peaktovalley_refill(Double_t values[PEAKS][9], Double_t com_values[PEAKS][9], TNtuple *, Double_t *, Double_t *, TH1F *xops[64], TH1F *ypos[64],Int_t,Float_t,Float_t,TH1F *tacs[64],TH1F *spat_glob, TH1F *com_glob,Int_t,Int_t   );
Int_t peaktovalley(TH1F **,TCanvas *);

Double_t *ptv_ana(TH1F *hist, TF1 *posgausfits[],TCanvas *c1, Int_t nr=3, Int_t x=1,Int_t verbose=0);
Int_t ptv_ana_old(TH1F *, TCanvas *);
Double_t TotalPTV(Double_t *, Double_t *);
Int_t fitpos(TH1F **,TF1 *posgausfits[], Int_t);
