#ifndef FORMAT_RECON_H
#define FORMAT_RECON_H

#include "TROOT.h"
#include "decoder.h"
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
#include "Apd_Fit.h"
#include "CoincEvent.h"


struct buf{
    float x1;
    float y1;
    float z1;
    float cdt;
    float ri;
    float x2;
    float y2;
    float z2;
    float si;
    float Si;
};


#endif
