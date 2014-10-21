#include "TROOT.h"
#include "TStyle.h"
#include "Riostream.h"
#include "TTree.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TSpectrum.h"
#include "TMath.h"
#include "TSpectrum.h"
#include "TF1.h"
#include "libInit_avdb.h"
#include "myrootlib.h"
#include "CoincEvent.h"
#include "string.h"
#include "cal_helper_functions.h"
#include <armadillo>
#include <time.h>
#include <string>

int main(int argc, Char_t *argv[])
{ 

    string filename;
    string filebase;
    Int_t verbose = 0;
    bool use_all_flag(false);
    const Long64_t default_max_length(400000);
    Long64_t max_length(default_max_length);

    for(int ix = 1; ix < argc; ix++) {
        if (strncmp(argv[ix], "-v", 2) == 0) {
            cout << "Verbose Mode " << endl;
            verbose = 1;
        }
        if (strncmp(argv[ix], "-f", 3) == 0) {
            filename = string(argv[ix + 1]);
        }
        if (strncmp(argv[ix], "-a", 3) == 0) {
            use_all_flag = true;
        }
        if (strncmp(argv[ix], "--all", 6) == 0) {
            use_all_flag = true;
        }
        if (strncmp(argv[ix], "-n", 3) == 0) {
            max_length = atol(argv[ix+1]);
            if (max_length == 0) {
                max_length = default_max_length;
            }
        }
    }

    if (verbose) {
        cout << "Welcome to cal_ls_apd_offset" << endl;
        cout << "max_length: " << max_length << endl;
    }

    size_t root_ext_pos(filename.find(".root"));
    if (root_ext_pos == string::npos) {
        cerr << "Error: .root extension not found in \"" << filename << "\"\n"
             << "...Exiting" << endl;
        return(-1);
    } else {
        filebase = string(filename, 0, root_ext_pos);
        if (verbose) {
            cout << "filebase: " << filebase << endl;
        }
    }

    if (!verbose) {
        gErrorIgnoreLevel = kError;
    }

    rootlogon(verbose);
    gStyle->SetOptStat(kTRUE); 

    TCanvas *c1;
    c1 = (TCanvas*)gROOT->GetListOfCanvases()->FindObject("c1");
    if (!c1) c1 = new TCanvas("c1","c1",10,10,1000,1000);
    c1->SetCanvasSize(700,700);


    string output_rootfile(filebase + ".ft_offcal.root");

    if (verbose) {
        cout << " Opening file " << filename << endl;
    }
    TFile *rtfile = new TFile(filename.c_str(),"OPEN");
    TTree *mm  = (TTree *) rtfile->Get("mana");
    CoincEvent *evt =  new CoincEvent();
    mm->SetBranchAddress("Event",&evt);

    if (verbose) cout << "Armadillo: Start" << endl;
    clock_t armadillo_time(clock());
    int nTerms(512);
    Long64_t entries = mm->GetEntries();

    if (use_all_flag) {
        max_length = entries;
    }
    Long64_t length(max_length < entries ? max_length : entries);
    if (verbose) cout << "Number of events: " << length << endl;
    if (verbose) cout << "Armadillo: Allocating matrix space" << endl;
    arma::mat A(length, nTerms);
    arma::vec y(length);

    if (verbose) cout << "Armadillo: Filling Matrix Entries" << endl;
    for (Long64_t ii = 0; ii < length; ii++) {
        mm->GetEntry(ii);
        int fin1(evt->fin1 + 8*(evt->cartridge1 + 2*(0)));
        int fin2(evt->fin2 + 8*(evt->cartridge2 + 2*(1)));
        int apd1(evt->apd1 + 2*(evt->m1 + 16*(fin1)));
        int apd2(evt->apd2 + 2*(evt->m2 + 16*(fin2)));
        A(ii, apd1) = 1;
        //A(ii, apd2) = -1;
        y(ii) = evt->dtf;
    }
    //if (verbose) cout << "Armadillo: Calculating Pinv" << endl;
    if (verbose) cout << "Armadillo: Calculating Fit" << endl;

    arma::vec per_crystal_correction(nTerms);
    bool status(arma::solve(per_crystal_correction, A, y));
    
    //arma::mat pinv_A(nTerms, length);
    //bool status(arma::pinv(pinv_A, A, 1e-1));

    if (!status) {
        //cerr << "Pinv for the solution failed" << endl;
        cerr << "Fit for the solution failed" << endl;
        return(-1);
    } else {
        //if (verbose) cout << "Armadillo: Calculating Fit" << endl;
        //arma::vec per_crystal_correction(nTerms);
        //per_crystal_correction = pinv_A * y;
        armadillo_time = clock() - armadillo_time;
        if (verbose) {
            cout << "Armadillo timing ______________" << endl;
            cout << "     ticks: " << armadillo_time << endl;
            cout << "  time (s): " << ((float) armadillo_time) / CLOCKS_PER_SEC << endl;;
        }

        cout << per_crystal_correction << endl;
    }
    return(0);
}

