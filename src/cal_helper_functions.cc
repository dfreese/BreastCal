#include "cal_helper_functions.h"
#include "TStyle.h"
#include "Riostream.h"
#include "TMath.h"
#include <sstream>
#include <string>
#include <vector>

Int_t drawmod(
        TH1F *hi[FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE],
        TCanvas *ccc,
        const std::string & filename)
{
    std::string filename_open(filename + "(");
    std::string filename_close(filename + ")");

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
            ccc->Print(filename_open.c_str());
        } else if(kk == (FINS_PER_CARTRIDGE - 1)) {
            // Last Page
            ccc->Print(filename_close.c_str());
        } else {
            ccc->Print(filename.c_str());
        }
        ccc->Clear();
        ccc->Divide(4, 8);
    }

    return(0);
}

Int_t drawmod_crys2(
        TH1F *hi[MODULES_PER_FIN][APDS_PER_MODULE][CRYSTALS_PER_APD],
        TCanvas *ccc,
        const std::string & filename)
{
    std::string filename_open(filename + "(");
    std::string filename_close(filename + ")");

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
                    ccc->Print(filename_open.c_str());
                } else {
                    if((module == (MODULES_PER_FIN-1)) && (apd == 1) && (ii==3)) {
                        ccc->Print(filename_close.c_str());
                    } else {
                        ccc->Print(filename.c_str());
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
        const string & outfile)
{
    ofstream parfile;
    parfile.open(outfile.c_str());
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

int ReadPerCrystalCal(
        std::string filename,
        float crystal_correction[SYSTEM_PANELS]
                                [CARTRIDGES_PER_PANEL]
                                [FINS_PER_CARTRIDGE]
                                [MODULES_PER_FIN]
                                [APDS_PER_MODULE]
                                [CRYSTALS_PER_APD])
{
    std::ifstream input;
    input.open(filename.c_str());
    if (!input.good()) {
        return(-1);
    }

    int entries(SYSTEM_PANELS *
            CARTRIDGES_PER_PANEL *
            FINS_PER_CARTRIDGE *
            MODULES_PER_FIN *
            APDS_PER_MODULE *
            CRYSTALS_PER_APD);

    std::string file_line;
    std::vector<float> file_values(entries, 0);
    int no_entries_found(0);
    while (getline(input, file_line)) {
        std::stringstream ss(file_line);
        if (ss >> file_values[no_entries_found++]) {
            if (no_entries_found >= entries) {
                return(-2);
            }
        } else {
            return(-3);
        }
    }
    int current_entry(0);
    for (int panel = 0; panel < SYSTEM_PANELS; panel++) {
        for (int cartridge = 0; cartridge < CARTRIDGES_PER_PANEL; cartridge++) {
            for (int fin = 0; fin < FINS_PER_CARTRIDGE; fin++) {
                for (int module = 0; module < MODULES_PER_FIN; module++) {
                    for (int apd = 0; apd < APDS_PER_MODULE; apd++) {
                        for (int crystal = 0;
                                crystal < CRYSTALS_PER_APD;
                                crystal++)
                        {
                            crystal_correction[panel]
                                              [cartridge]
                                              [fin]
                                              [module]
                                              [apd]
                                              [crystal] = 
                                              file_values[current_entry++];
                        }
                    }
                }
            }
        }
    }
    return(0);
}


int WritePerCrystalCal(
        std::string filename,
        float crystal_correction[SYSTEM_PANELS]
                                [CARTRIDGES_PER_PANEL]
                                [FINS_PER_CARTRIDGE]
                                [MODULES_PER_FIN]
                                [APDS_PER_MODULE]
                                [CRYSTALS_PER_APD])
{
    std::ofstream output;
    output.open(filename.c_str());
    if (!output.good()) {
        return(-1);
    }

    for (int panel = 0; panel < SYSTEM_PANELS; panel++) {
        for (int cartridge = 0; cartridge < CARTRIDGES_PER_PANEL; cartridge++) {
            for (int fin = 0; fin < FINS_PER_CARTRIDGE; fin++) {
                for (int module = 0; module < MODULES_PER_FIN; module++) {
                    for (int apd = 0; apd < APDS_PER_MODULE; apd++) {
                        for (int crystal = 0;
                                crystal < CRYSTALS_PER_APD;
                                crystal++)
                        {
                            float value(crystal_correction[panel]
                                                          [cartridge]
                                                          [fin]
                                                          [module]
                                                          [apd]
                                                          [crystal]);
                            output << value << "\n";
                        }
                    }
                }
            }
        }
    }
    return(0);
}

int WritePerApdAsPerCrystalCal(
        std::string filename,
        float apd_correction[SYSTEM_PANELS]
                            [CARTRIDGES_PER_PANEL]
                            [FINS_PER_CARTRIDGE]
                            [MODULES_PER_FIN]
                            [APDS_PER_MODULE])
{
    std::ofstream output;
    output.open(filename.c_str());
    if (!output.good()) {
        return(-1);
    }

    for (int panel = 0; panel < SYSTEM_PANELS; panel++) {
        for (int cartridge = 0; cartridge < CARTRIDGES_PER_PANEL; cartridge++) {
            for (int fin = 0; fin < FINS_PER_CARTRIDGE; fin++) {
                for (int module = 0; module < MODULES_PER_FIN; module++) {
                    for (int apd = 0; apd < APDS_PER_MODULE; apd++) {
                        for (int crystal = 0;
                                crystal < CRYSTALS_PER_APD;
                                crystal++)
                        {
                            float value(apd_correction[panel]
                                                      [cartridge]
                                                      [fin]
                                                      [module]
                                                      [apd]);
                            output << value << "\n";
                        }
                    }
                }
            }
        }
    }
    return(0);
}

int AddPerCrystalCal(
        float augend[SYSTEM_PANELS]
                    [CARTRIDGES_PER_PANEL]
                    [FINS_PER_CARTRIDGE]
                    [MODULES_PER_FIN]
                    [APDS_PER_MODULE]
                    [CRYSTALS_PER_APD],
        float addend[SYSTEM_PANELS]
                    [CARTRIDGES_PER_PANEL]
                    [FINS_PER_CARTRIDGE]
                    [MODULES_PER_FIN]
                    [APDS_PER_MODULE]
                    [CRYSTALS_PER_APD],
        float sum[SYSTEM_PANELS]
                 [CARTRIDGES_PER_PANEL]
                 [FINS_PER_CARTRIDGE]
                 [MODULES_PER_FIN]
                 [APDS_PER_MODULE]
                 [CRYSTALS_PER_APD])
{
    for (int panel = 0; panel < SYSTEM_PANELS; panel++) {
        for (int cartridge = 0; cartridge < CARTRIDGES_PER_PANEL; cartridge++) {
            for (int fin = 0; fin < FINS_PER_CARTRIDGE; fin++) {
                for (int module = 0; module < MODULES_PER_FIN; module++) {
                    for (int apd = 0; apd < APDS_PER_MODULE; apd++) {
                        for (int crystal = 0;
                                crystal < CRYSTALS_PER_APD;
                                crystal++)
                        {
                            sum[panel][cartridge][fin][module][apd][crystal] = 
                                augend[panel][cartridge][fin]
                                      [module][apd][crystal] +
                                addend[panel][cartridge][fin]
                                      [module][apd][crystal];
                        }
                    }
                }
            }
        }
    }
    return(0);
}

int AddPerApdAsPerCrystalCal(
        float augend[SYSTEM_PANELS]
                    [CARTRIDGES_PER_PANEL]
                    [FINS_PER_CARTRIDGE]
                    [MODULES_PER_FIN]
                    [APDS_PER_MODULE]
                    [CRYSTALS_PER_APD],
        float addend[SYSTEM_PANELS]
                    [CARTRIDGES_PER_PANEL]
                    [FINS_PER_CARTRIDGE]
                    [MODULES_PER_FIN]
                    [APDS_PER_MODULE],
        float sum[SYSTEM_PANELS]
                 [CARTRIDGES_PER_PANEL]
                 [FINS_PER_CARTRIDGE]
                 [MODULES_PER_FIN]
                 [APDS_PER_MODULE]
                 [CRYSTALS_PER_APD])
{
    for (int panel = 0; panel < SYSTEM_PANELS; panel++) {
        for (int cartridge = 0; cartridge < CARTRIDGES_PER_PANEL; cartridge++) {
            for (int fin = 0; fin < FINS_PER_CARTRIDGE; fin++) {
                for (int module = 0; module < MODULES_PER_FIN; module++) {
                    for (int apd = 0; apd < APDS_PER_MODULE; apd++) {
                        for (int crystal = 0;
                                crystal < CRYSTALS_PER_APD;
                                crystal++)
                        {
                            sum[panel][cartridge][fin][module][apd][crystal] = 
                                augend[panel][cartridge][fin]
                                      [module][apd][crystal] +
                                addend[panel][cartridge][fin]
                                      [module][apd];
                        }
                    }
                }
            }
        }
    }
    return(0);
}

int BoundsCheckEvent(const CoincEvent & event) {
    if ((event.cartridge1 < 0) || (event.cartridge1 >= CARTRIDGES_PER_PANEL)) {
        return(-1);
    } else if ((event.cartridge2 < 0) || (event.cartridge2 >= CARTRIDGES_PER_PANEL)) {
        return(-2);
    } else if ((event.fin1 < 0) || (event.fin1 >= FINS_PER_CARTRIDGE)) {
        return(-3);
    } else if ((event.fin2 < 0) || (event.fin2 >= FINS_PER_CARTRIDGE)) {
        return(-4);
    } else if ((event.m1 < 0) || (event.m1 >= MODULES_PER_FIN)) {
        return(-5);
    } else if ((event.m2 < 0) || (event.m2 >= MODULES_PER_FIN)) {
        return(-6);
    } else if ((event.apd1 < 0) || (event.apd1 >= APDS_PER_MODULE)) {
        return(-7);
    } else if ((event.apd2 < 0) || (event.apd2 >= APDS_PER_MODULE)) {
        return(-8);
    } else if ((event.crystal1 < 0) || (event.crystal1 >= CRYSTALS_PER_APD)) {
        return(-9);
    } else if ((event.crystal2 < 0) || (event.crystal2 >= CRYSTALS_PER_APD)) {
        return(-10);
    } else {
        return(0);
    }
}

int EnergyGateEvent(
        const CoincEvent & event,
        float energy_low,
        float energy_high)
{
    if ((event.E1 < energy_low) || (event.E1 > energy_high)) {
        return(-1);
    } else if ((event.E2 < energy_low) || (event.E2 > energy_high)) {
        return(-2);
    } else {
        return(0);
    }
}

