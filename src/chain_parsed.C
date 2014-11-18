#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "TROOT.h"
#include "TKey.h"
#include "TFile.h"
#include "TNetFile.h"
#include "TRandom.h"
#include "TTree.h"
#include "Riostream.h"
#include "TMath.h"
#include "TNtuple.h"
#include "TH2F.h"
#include "TChain.h"
#include "TVector.h"
#include "libInit_avdb.h"
#include "myrootlib.h"
#include "Syspardef.h"

void usage(void) {
    cout << " Chain parsed -f [filelist]  -o [outputfilename] " <<endl;
    return;
}

int main(int argc, char *argv[]) {
    string filelist;
    string outfilename;
    bool verbose(false);
    bool ofilenamespec(false);
    for (int i=0; i<argc; i++) {
        if (strncmp(argv[i],"-v",2)==0) {
            verbose = true;
        }
        if (strncmp(argv[i],"-f",2)==0) {
            filelist = string(argv[i + 1]);
            i++;
        }
        if (strncmp(argv[i],"-o",2)==0) {
            outfilename = string(argv[i + 1]);
            ofilenamespec = true;
            i++;
        }
    }

    cout << " Chaining ROOT files; output file generated will be : " << outfilename << endl;

    // making dummy histogram according to http://root.cern.ch/phpBB3/viewtopic.php?f=3&t=13607
    TH1F* dummy = new TH1F("dummy", "dummy", 10, 0, 1);

    ifstream chainfile;
    chainfile.open(filelist.c_str());
    if (!chainfile.good()) {
        cout << "Filelist could not be opened: " << filelist << endl;
        return(-1);
    }


    if (!ofilenamespec) {
        cout << " Please specify an outputfilename " << endl;
        usage();
        return(-2);
    }
    TFile *rfile = new TFile(outfilename.c_str(),"RECREATE");

    TChain *mdata = new TChain("mdata","mdata");

    int filesread(0);

    TH1F *E[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE];
    TH1F *E_com[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE];
    TH1F *ETMP[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE];
    TH1F *ETMP_com[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE];
    TVector *uu_c[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE];
    TVector *vv_c[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE];

    Char_t tmpstring[50];

    string curfilename;
    while (getline(chainfile,curfilename)) {
        filesread++;
        if (verbose) {
            cout << " Files read :: " << filesread << ". Current file : " << curfilename << endl;
        }

        mdata->Add(curfilename.c_str());

        TFile *decodedfile = new TFile(curfilename.c_str(),"UPDATE");

        if (!decodedfile || decodedfile->IsZombie()) {
            cout << "problems opening file " << curfilename << "\n.Exiting" << endl;
            return(-3);
        }

        // FIXME:: Read circle centers from first file - should have a way to check if there are enough entries to have reliable centers.
        if (verbose) {
            cout << " Reading circle centers " << endl;
        }

        if (filesread == 1) {
            for (int c = 0; c < CARTRIDGES_PER_PANEL; c++) {
                for (int r = 0; r < FINS_PER_CARTRIDGE; r++) {
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
                        sprintf(tmpstring,"C%dF%d/E[%d][%d][%d][%d]",c,f,c,f,m,j);
                        if (filesread==1) {
                            E[c][f][m][j]= (TH1F *) decodedfile->Get(tmpstring);
                            E[c][f][m][j]->SetDirectory(0);
                        } else {
                            ETMP[c][f][m][j]= (TH1F *) decodedfile->Get(tmpstring);
                            E[c][f][m][j]->Add(ETMP[c][f][m][j],1);
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
            cout << " Closing file " << curfilename << endl;
        }

        decodedfile->Close();
    }

    chainfile.close();

    if (verbose) {
        cout << " Writing to root file " << endl;
    }

    rfile->cd();
    mdata->Write();



    TDirectory *subdir[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE];
    for (int c = 0; c < CARTRIDGES_PER_PANEL; c++) {
        for (int f = 0; f < FINS_PER_CARTRIDGE; f++) {
            sprintf(tmpstring,"C%dF%d",c,f);
            subdir[c][f] = rfile->mkdir(tmpstring);
            subdir[c][f]->cd();
            for (int m = 0; m < MODULES_PER_FIN; m++) {
                for (int j = 0; j < APDS_PER_MODULE; j++) {
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

    // need to store uvcenters ::
    for (int c = 0; c < CARTRIDGES_PER_PANEL; c++) {
        for (int r = 0; r < FINS_PER_CARTRIDGE; r++) {
            if(verbose) {
                cout << " C" << c<< " F" <<r << endl;
            }
            sprintf(tmpstring,"uu_c[%d][%d]",c,r);
            uu_c[c][r]->Write(tmpstring);
            sprintf(tmpstring,"vv_c[%d][%d]",c,r);
            vv_c[c][r]->Write(tmpstring);
        }
    }
    rfile->Close();

    return(0);
}
