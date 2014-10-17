#include "cal_helper_functions.h"
#include "TStyle.h"
#include "Riostream.h"
#include "TMath.h"

Int_t drawmod_crys2(TH1F *hi[MODULES_PER_FIN][APDS_PER_MODULE][CRYSTALS_PER_APD], TCanvas *ccc, Char_t filename[MAXFILELENGTH])
{
    Char_t  filenameo[MAXFILELENGTH+1];
    Char_t  filenamec[MAXFILELENGTH+1];

    strcpy(filenameo, filename);
    strcpy(filenamec, filename);
    strcat(filenameo, "(");
    strcat(filenamec, ")");

    ccc->Clear();
    ccc->Divide(4, 4);

    for (int module = 0; module < MODULES_PER_FIN; module++) {
        for (int apd = 0; apd < APDS_PER_MODULE; apd++) {
            for (int ii = 0; ii < 4; ii++) {
                for (int jj = 0; jj < 16; jj++) {
                    ccc->cd(jj+1);
                    hi[module][apd][ii*16+jj]->Draw("E");
                }
                if ((module == 0) && (apd == 0) && (ii == 0)) {
                    ccc->Print(filenameo);
                } else {
                    if((module == (MODULES_PER_FIN-1)) && (apd == 1) && (ii==3)) {
                        ccc->Print(filenamec);
                    } else {
                        ccc->Print(filename);
                    }
                }
                ccc->Clear();
                ccc->Divide(4, 4);
            }
        }
    }
    return(0);
}

Int_t writ(
        TH1D *hi[PEAKS],
        TCanvas *ccc,
        Char_t filename[MAXFILELENGTH])
{
    Char_t  filenameo[MAXFILELENGTH+1];
    Char_t  filenamec[MAXFILELENGTH+1];

    strcpy(filenameo, filename);
    strcpy(filenamec, filename);
    strcat(filenameo, "(");
    strcat(filenamec, ")");

    ccc->Clear();
    ccc->Divide(2, 4);

    for(Int_t k = 1; k < PEAKS + 1; k++) {
        if(k % 8) {
            ccc->cd(k % 8);
        } else {
            ccc->cd(8);
        }

        hi[k - 1]->Draw("E");

        if(!(k % 8)) {
            if(k == 8) {
                ccc->Print(filenameo);
            } else {
                if(k == PEAKS) {
                    ccc->Print(filenamec);
                } else {
                    ccc->Print(filename);
                }
            }
            ccc->Clear();
            ccc->Divide(2, 4);
        }
    }
    return(0);
}

Int_t writ2d(
        TH2F *hi[PEAKS],
        TCanvas *ccc,
        Char_t filename[MAXFILELENGTH])
{
    Char_t  filenameo[MAXFILELENGTH+1];
    Char_t  filenamec[MAXFILELENGTH+1];

    strcpy(filenameo, filename);
    strcpy(filenamec, filename);
    strcat(filenameo, "(");
    strcat(filenamec, ")");

    ccc->Clear();
    ccc->Divide(2, 4);

    for (Int_t k = 1; k < PEAKS + 1; k++) {
        if(k % 8) {
            ccc->cd(k % 8);
        } else {
            ccc->cd(8);
        }

        hi[k - 1]->Draw("colz");

        if(!(k % 8)) {
            if (k == 8) {
                ccc->Print(filenameo);
            } else {
                if (k == PEAKS) {
                    ccc->Print(filenamec);
                } else {
                    ccc->Print(filename);
                }
            }
            ccc->Clear();
            ccc->Divide(2, 4);
        }
    }
    return(0);
}

TH2F * get2dcrystal(
        Float_t vals[CRYSTALS_PER_APD],
        Char_t title[40]="area")
{
    TH2F *thispar = new TH2F("thispar",title,8,0,8,8,0,8);
    for (int ii = 0; ii < 8; ii++){
        for (int jj = 0; jj < 8; jj++) {
            thispar->SetBinContent(ii + 1, jj + 1, vals[ii * 8 + jj]);
        }
    }
    return(thispar);
}

Float_t getmax(TSpectrum *s,Int_t npeaks, Int_t verbose) {
    Int_t maxpeakheight = -1000000;
    Float_t maxpos = 0;
    if (npeaks > 1) {
        for (Int_t i = 0; i < npeaks; i++) { 
            if (verbose) {
                cout << " Peak " << i << " :  " << *(s->GetPositionX()+i) 
                     << " " << *(s->GetPositionY()+i) << endl;
            }
        }
    }

    for (Int_t i = 0; i<npeaks; i++) {
        if ( (*(s->GetPositionY()+i)) > maxpeakheight ) {
            maxpos= *(s->GetPositionX()+i);
            maxpeakheight = *(s->GetPositionY()+i);
        }
    }
    return(maxpos);
}
