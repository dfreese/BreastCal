#include "TROOT.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TSpectrum.h"
#include "TSpectrum2.h"
#include "TH2F.h"
#include "TSpectrum.h"
#include "Syspardef.h"

#define CRYSTALS_PER_APD 64

Int_t drawmod_crys2(TH1F *hi[MODULES_PER_FIN][APDS_PER_MODULE][CRYSTALS_PER_APD], TCanvas *ccc, Char_t filename[MAXFILELENGTH]);
Int_t writ(TH1D *hi[PEAKS], TCanvas *ccc, Char_t filename[MAXFILELENGTH]);
Int_t writ2d(TH2F *hi[PEAKS], TCanvas *ccc, Char_t filename[MAXFILELENGTH]);
TH2F *get2dcrystal(Float_t vals[CRYSTALS_PER_APD], Char_t title[40]);
Float_t getmax (TSpectrum *s, Int_t npeaks, Int_t verbose);
