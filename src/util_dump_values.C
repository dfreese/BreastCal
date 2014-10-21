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

    if (verbose) {
        cout << " Opening file " << filename << endl;
    }
    TFile *rtfile = new TFile(filename.c_str(),"OPEN");
    TTree *mm  = (TTree *) rtfile->Get("mana");
    CoincEvent *evt =  new CoincEvent();
    mm->SetBranchAddress("Event",&evt);

    string output_textfile(filebase + ".dat");

    Long64_t entries = mm->GetEntries();
    if (use_all_flag) {
        max_length = entries;
    }
    Long64_t length(max_length < entries ? max_length : entries);
    if (verbose) cout << "Number of events: " << length << endl;

    if (verbose) cout << "Writing Entries" << endl;
    ofstream outfile(output_textfile.c_str());
    for (Long64_t ii = 0; ii < length; ii++) {
        mm->GetEntry(ii);
        outfile << evt->cartridge1 << " "
                << evt->fin1 << " "
                << evt->m1 << " "
                << evt->apd1 << " "
                << evt->crystal1 << " "
                << evt->cartridge2 << " "
                << evt->fin2 << " "
                << evt->m2 << " "
                << evt->apd2 << " "
                << evt->crystal2 << " "
                << evt->dtf << "\n";
    }
    outfile.close();

    return(0);
}

