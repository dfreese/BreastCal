#include "cal_helper_functions.h"
#include "TStyle.h"
#include "Riostream.h"
#include "TMath.h"

Int_t drawmod(TH1F *hi[FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE], TCanvas *ccc, Char_t filename[MAXFILELENGTH])
{
    Char_t filenameo[MAXFILELENGTH+1];
    Char_t filenamec[MAXFILELENGTH+1];
    strcpy(filenameo, filename);
    strcpy(filenamec, filename);
    strcat(filenameo, "(");
    strcat(filenamec, ")");

    ccc->Clear();
    ccc->Divide(4, 8);

    for(int kk = 0; kk < FINS_PER_CARTRIDGE; kk++) {
        for(int ii = 0; ii < MODULES_PER_FIN; ii++) {
            ccc->cd(1+2*(ii));
            hi[kk][ii][0]->Draw("E");
            ccc->cd(2+2*(ii));
            hi[kk][ii][1]->Draw("E");
        }
        if(kk == 0) {
            // First Page
            ccc->Print(filenameo);
        } else if(kk == (FINS_PER_CARTRIDGE - 1)) {
            // Last Page
            ccc->Print(filenamec);
        } else {
            ccc->Print(filename);
        }
        ccc->Clear();
        ccc->Divide(4, 8);
    }

    return(0);
}

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

Int_t writval(
        Float_t mean_crystaloffset[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE][CRYSTALS_PER_APD],
        Char_t outfile[MAXFILELENGTH])
{
    ofstream parfile;
    parfile.open(outfile);
    if (parfile.good()) {
        for (int cartridge = 0; cartridge < CARTRIDGES_PER_PANEL; cartridge++) {
            for (int fin = 0; fin < FINS_PER_CARTRIDGE; fin++) {
                for (int module = 0; module < MODULES_PER_FIN; module++) {
                    for (int apd = 0; apd < APDS_PER_MODULE; apd++) {
                        for (int crystal=0; crystal < CRYSTALS_PER_APD; crystal++) {
                            if ((crystal % 8) == 0) {
                                parfile << "C" << cartridge << "F" << fin 
                                    << "M" << module << "A" << apd
                                    << "R" << TMath::Floor(crystal/8) << " ";
                            }
                            parfile << setw(6) << setprecision(3) 
                                << mean_crystaloffset[cartridge][fin][module][apd][crystal] << " ";
                            if ((crystal % 8) == 7) {
                                parfile << endl;
                            }
                        }
                        parfile << endl;
                    }
                }
            }
        }
        parfile << endl;
    }
    parfile.close();
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
