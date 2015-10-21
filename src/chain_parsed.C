#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include "TROOT.h"
#include "TFile.h"
#include "TRandom.h"
#include "TTree.h"
#include "Riostream.h"
#include "TH2F.h"
#include "TChain.h"
#include "TVector.h"
#include "decoder.h"

void usage() {
    cout << "chain_parsed -f [filelist]  -o [outputfilename] " << endl;
    cout << "  -t [treename] : specify treename (default: \"mdata\")" << endl;
    return;
}

int main(int argc, char ** argv) {
    if (argc == 1) {
        usage();
        exit(0);
    }

    string filelist;
    string outfilename;
    string treename = "mdata";
    bool verbose = false;

    for (int i=0; i<argc; i++) {
        if (strcmp(argv[i],"-v") == 0) {
            verbose = true;
        }
        if (strcmp(argv[i],"-f") == 0) {
            filelist = string(argv[++i]);
        }
        if (strcmp(argv[i],"-o") == 0) {
            outfilename = string(argv[++i]);
        }
        if (strcmp(argv[i],"-t") == 0) {
            treename = string(argv[++i]);
        }
    }

    if (outfilename == "") {
        cout << " Please specify an outputfilename " << endl;
        usage();
        exit(-1);
    }

    cout << " Chaining ROOT files; output file generated will be : " << outfilename << endl;

    // making dummy histogram according to
    // http://root.cern.ch/phpBB3/viewtopic.php?f=3&t=13607
    TH1F* dummy = new TH1F("dummy", "dummy", 10, 0, 1);

    ifstream chainfile;
    chainfile.open(filelist.c_str());
    if (!chainfile.good()) {
        cout << "Filelist could not be opened: " << filelist << endl;
        return(-1);
    }

    TFile * rfile = new TFile(outfilename.c_str(),"RECREATE");
    TChain * mdata = new TChain(treename.c_str(), treename.c_str());

    Int_t  filesread=0;

    TH1F *ETMP[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE];
    TH1F *ETMP_com[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE];

    TFile *decodedfile;

    string current_filename;
    while (getline(chainfile, current_filename)) {
        filesread++;
        if (verbose) {
            cout << " Files read :: " << filesread
                 << ". Current file : " << current_filename << endl;
        }
        mdata->Add(current_filename.c_str());

        decodedfile = new TFile(current_filename.c_str(),"UPDATE");

        if (!decodedfile || decodedfile->IsZombie()) {
            cout << "problems opening file " << current_filename << "\n.Exiting" << endl;
            return -11;
        }

        // FIXME:: We currently read circle centers from first file, but
        // should have a way to check if there are enough entries to have
        // reliable centers.

        if (verbose) {
            cout << " Reading circle centers " << endl;
        }

        if (filesread == 1) {
            for (int c = 0; c < CARTRIDGES_PER_PANEL; c++) {
                for (int r=0; r<FINS_PER_CARTRIDGE; r++) {
                    Char_t tmpstring[50];
                    sprintf(tmpstring,"timing/uu_c[%d][%d]",c,r);
                    uu_c[c][r] = (TVector *) decodedfile->Get(tmpstring);
                    sprintf(tmpstring,"timing/vv_c[%d][%d]",c,r);
                    vv_c[c][r] = (TVector *) decodedfile->Get(tmpstring);
                }
            }
        }

        if (verbose) {
            cout << " Reading histograms " << endl;
        }

        for (int c = 0; c < CARTRIDGES_PER_PANEL; c++) {
            for (int f = 0; f < FINS_PER_CARTRIDGE; f++) {
                for (int m = 0; m < MODULES_PER_FIN; m++) {
                    for (int j = 0; j < APDS_PER_MODULE; j++) {
                        Char_t tmpstring[50];
                        sprintf(tmpstring,"C%dF%d/E[%d][%d][%d][%d]",c,f,c,f,m,j);
                        if (filesread == 1) {
                            E[c][f][m][j]= (TH1F *) decodedfile->Get(tmpstring);
                            E[c][f][m][j]->SetDirectory(0);
                        } else {
                            ETMP[c][f][m][j] = (TH1F *) decodedfile->Get(tmpstring);
                            E[c][f][m][j]->Add(ETMP[c][f][m][j], 1);
                            delete ETMP[c][f][m][j];
                        }
                        sprintf(tmpstring,"C%dF%d/E_com[%d][%d][%d][%d]",c,f,c,f,m,j);
                        if (filesread==1) {
                            E_com[c][f][m][j]= (TH1F *) decodedfile->Get(tmpstring);
                            E_com[c][f][m][j]->SetDirectory(0);
                        } else {
                            ETMP_com[c][f][m][j]= (TH1F *) decodedfile->Get(tmpstring);
                            E_com[c][f][m][j]->Add(ETMP_com[c][f][m][j]);
                            delete ETMP_com[c][f][m][j];
                        }
                    }
                }
            }
        }
        if (verbose) {
            cout << " Closing file " << current_filename << endl;
        }
        decodedfile->Close();
    }

    chainfile.close();

    if (verbose) {
        cout << " Writing to root file " << endl;
    }

    rfile->cd();
    mdata->Write();

    for (int c = 0; c < CARTRIDGES_PER_PANEL; c++) {
        for (int f = 0; f < FINS_PER_CARTRIDGE; f++) {
            Char_t tmpstring[50];
            sprintf(tmpstring,"C%dF%d",c,f);
            subdir[c][f] = rfile->mkdir(tmpstring);
            subdir[c][f]->cd();
            for (Int_t m=0; m<MODULES_PER_FIN; m++) {
                for (Int_t  j=0; j<APDS_PER_MODULE; j++) {
                    E[c][f][m][j]->Write();
                    E_com[c][f][m][j]->Write();
                }
            }
        }
    }

    rfile->cd();

    if (verbose) {
        cout << " Writing U,V centers to roofile " << endl;
    }
    TDirectory *timing =  rfile->mkdir("timing");
    timing->cd();
    for (int c = 0; c < CARTRIDGES_PER_PANEL; c++) {
        for (int f = 0; f < FINS_PER_CARTRIDGE; f++) {
            Char_t tmpstring[50];
            sprintf(tmpstring,"uu_c[%d][%d]", c, f);
            uu_c[c][f]->Write(tmpstring);
            sprintf(tmpstring,"vv_c[%d][%d]", c, f);
            vv_c[c][f]->Write(tmpstring);
        }
    }
    rfile->Close();
    return(0);
}
