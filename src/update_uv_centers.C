#include <stdlib.h>
#include <iostream>
#include "TROOT.h"
#include "TFile.h"
#include "TObject.h"
#include "TTree.h"
#include "Riostream.h"
#include "TMath.h"
#include "TVector.h"
#include "ModuleDat.h"
#include <fstream>
#include <string>
#include "Util.h"
#include "libInit_avdb.h"
#include "myrootlib.h"
#include "Syspardef.h"


using namespace std;

void usage(void) {
    cout << "update_uv_centers [-v] -f [filename]\n";
    cout << "    A program to update the uv centers using the whole dataset\n"
         << "    after chain_parsed is run\n"
         << " options:\n"
         << " -uvt: UV circle centers threshold - default = -700" << endl;
    return;
}

int ReadUVCenters(
        TFile *rfile,
        TVector *uu_c[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE],
        TVector *vv_c[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE])
{
    for (int cc = 0; cc < CARTRIDGES_PER_PANEL; cc++) {
        for (int f = 0; f < FINS_PER_CARTRIDGE; f++) {
            Char_t tmpstring[40];
            sprintf(tmpstring,"timing/uu_c[%d][%d]",cc,f);
            TVector * temp_u = (TVector *) rfile->Get(tmpstring);
            if (!temp_u) {
                return(-1);
            } else {
                uu_c[cc][f] = temp_u;
            }
            sprintf(tmpstring,"timing/vv_c[%d][%d]",cc,f);
            TVector * temp_v = (TVector *) rfile->Get(tmpstring);
            if (!temp_v) {
                return(-1);
            } else {
                vv_c[cc][f] = temp_v;
            }
        }
    }
    return(0);
}

int main(int argc, char ** argv) {
    int uvthreshold(-700);
    int verbose(0);

    string filename;

    bool filename_specified_flag(false);

    // Arguments not requiring input
    for (int ix = 1; ix < argc; ix++) {
        if (strcmp(argv[ix], "-v") == 0) {
            verbose = 1;
            cout << "Verbose mode" << endl;
        }
        if ((strcmp(argv[ix], "-h") == 0) ||
                (strcmp(argv[ix], "--help") == 0))
        {
            usage();
            return(0);
        }
    }
    // Arguments requiring input
    for (int ix = 1; ix < (argc - 1); ix++) {
        if (strcmp(argv[ix], "-uvt") == 0) {
            uvthreshold = atoi(argv[ix + 1]);
        }
        if (strcmp(argv[ix], "-f") == 0) {
            filename = string(argv[ix + 1]);
            filename_specified_flag = true;
        }
    }

    if (!filename_specified_flag) {
        cerr << "Filename not specified" << endl;
        cerr << "Exiting." << endl;
        return(-2);
    }

    if (verbose) {
        cout << "Opening root file: " << filename << endl;
    }
    TFile * input_file = new TFile(filename.c_str(),"UPDATE");
    if (input_file->IsZombie()) {
        cerr << "Error opening file : " << filename << endl;
        cerr << "Exiting..." << endl;
        return(-5);
    }
    if (verbose) {
        cout << "Finished Opening root file: " << filename << endl;
    }

    string input_tree_name("mdata");
    if (verbose) {
        cout << "Reading TTree: " << input_tree_name << endl;
    }
    TTree * mdata = (TTree *) input_file->Get(input_tree_name.c_str());
    if (!mdata) {
        cerr << "Problem reading " << input_tree_name << " from file." << endl;
        cerr << "Exiting." << endl;
        return(-8);
    }
    if (verbose) {
        cout << "Finished reading TTree: " << input_tree_name << endl;
    }

    ModuleDat * event = 0;
    string input_branch_name("eventdata");
    if (verbose) {
        cout << "Reading branch: " << input_branch_name << endl;
    }
    int set_address_status =
            mdata->SetBranchAddress(input_branch_name.c_str(), &event);
    if (set_address_status < 0) {
        cerr << "Problem reading " << input_branch_name
             << " from " << input_tree_name << "." << endl;
        cerr << "Exiting." << endl;
        return(-8);
    }
    if (verbose) {
        cout << "Finished reading branch: " << input_branch_name << endl;
    }


    if (verbose) {
        cout << "Reading uv centers from file" << endl;
    }
    TVector *uu_c[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE];
    TVector *vv_c[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE];
    Long64_t uventries[CARTRIDGES_PER_PANEL]
                      [FINS_PER_CARTRIDGE]
                      [MODULES_PER_FIN]
                      [2] = {{{{0}}}};

    if (ReadUVCenters(input_file, uu_c, vv_c) < 0) {
        cerr << "Problem reading uv circle centers from " << filename << endl;
        cerr << "Exiting...";
        return(-9);
    }
    if (verbose) {
        cout << "Finished reading uv centers from file" << endl;
    }

    long entries = mdata->GetEntries();
    long entires_above_threshold(0);
    if (verbose) {
        cout << "Calculating uv circle centers out of "
             << entries << " events" << endl;
    }
    for (long ii = 0; ii < entries; ii++) {
        mdata->GetEntry(ii);
        if (event->Ech < uvthreshold) {
            entires_above_threshold++;
            long no_entries = ++uventries[event->cartridge]
                                         [event->fin]
                                         [event->module]
                                         [event->apd];
            int ft = event->ft;
            short uh = short((ft & 0xFFFF0000) >> 16);
            short vh = short((ft & 0x0000FFFF) >> 0);
            TVector * u_c = uu_c[event->cartridge][event->fin];
            TVector * v_c = vv_c[event->cartridge][event->fin];
            int index = event->module*2+event->apd;
            (*u_c)[index] += (Float_t)(uh - (*u_c)[index]) / no_entries;
            (*v_c)[index] += (Float_t)(vh - (*v_c)[index]) / no_entries;
        }
    }
    if (verbose) {
        cout << "Finished calculating uv circle centers using "
             << entires_above_threshold << " events above the threshold of "
             << uvthreshold << "." << endl;
    }

    if (verbose) {
        cout << "Overwriting old uv circle centers" << endl;
    }
    // store uvcenters
    for (int cartridge = 0; cartridge < CARTRIDGES_PER_PANEL; cartridge++) {
        for (int fin = 0; fin < FINS_PER_CARTRIDGE; fin++) {
            char tmpstring[30];
            sprintf(tmpstring,"timing/uu_c[%d][%d]", cartridge, fin);
            uu_c[cartridge][fin]->Write(tmpstring, TObject::kOverwrite);
            sprintf(tmpstring,"timing/vv_c[%d][%d]", cartridge, fin);
            vv_c[cartridge][fin]->Write(tmpstring, TObject::kOverwrite);
        }
    }
    if (verbose) {
        cout << "Finished overwriting old uv circle centers" << endl;
    }

    input_file->Close();

    return(0);
}

