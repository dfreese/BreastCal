#include "TROOT.h"
#include "TStyle.h"
#include "Riostream.h"
#include "TTree.h"
#include "TFile.h"
#include "libInit_avdb.h"
#include "myrootlib.h"
#include "Syspardef.h"
#include "CoincEvent.h"
#include "PixelCal.h"
#include "TVector.h"
#include "sorting.h"
#include <vector>
#include <sstream>
#include "cal_helper_functions.h"

void usage(void) {
    cout << " Write out time calibrated files.\n";
    cout << " Usage:  " << endl;
    cout << " ./write_with_timing_cal [-v] -f [Filename]" << endl;
    cout << " -of: Specifies output filename. Default replaces .root\n"
         << "      extension with .tcal.root\n"
         << " -t:  Optionally sets the expected input tree name.\n"
         << "      Default is merged\n"
         << " -ot: Optionally sets the output tree name.\n"
         << "      Default is merged\n"
         << " -rcal (filename):  read in initial per crystal calibration\n"
         << " -redcal (filename): read in an energy dependence file\n"
         << " -n: do not write out the resulting root file\n";
}

int main(int argc, char ** argv) {
    if (argc == 1) {
        usage();
        return(0);
    }

    string filename;
    bool filenamespec(false);
    Int_t verbose = 0;
    string input_treename("merged");
    string input_branch_title("Event");
    string output_tree_name("merged");
    string output_tree_title("Merged and Calibrated LYSO-PSAPD data ");
    string output_branch_title("Event");
    string output_filename;
    bool output_filename_spec(false);

    bool read_per_crystal_correction(false);
    string input_crystal_cal_filename;

    bool read_per_crystal_energy_correction(false);
    string input_per_crystal_energy_cal_filename;

    // A flag that is true unless a -n option is found in which case only the
    // calibration parameters and the associated plots are generated, but the
    // calibrated root file is not created.
    bool write_out_root_file_flag(true);


    for(int ix = 1; ix < argc; ix++) {
        if (strcmp(argv[ix], "-h") == 0) {
            usage();
            return(0);
        }
        if (strcmp(argv[ix], "-v") == 0) {
            cout << "Verbose Mode " << endl;
            verbose = 1;
        }
        if(strcmp(argv[ix], "-n") == 0) {
            cout << "Calibrated Root File will not be created." << endl;
            write_out_root_file_flag = false;
        }
    }

    for(int ix = 1; ix < (argc-1); ix++) {
        if (strcmp(argv[ix], "-f") == 0) {
            filename = string(argv[ix + 1]);
            filenamespec = true;
            ix++;
        }
        if (strcmp(argv[ix], "-of") == 0) {
            output_filename = string(argv[ix + 1]);
            output_filename_spec = true;
            ix++;
        }
        if (strcmp(argv[ix], "-t") == 0) {
            input_treename = string(argv[ix + 1]);
            ix++;
        }
        if (strcmp(argv[ix], "-ot") == 0) {
            output_tree_name = string(argv[ix + 1]);
            ix++;
        }
        if(strcmp(argv[ix], "-redcal") == 0) {
            read_per_crystal_energy_correction = true;
            input_per_crystal_energy_cal_filename = string(argv[ix + 1]);
        }
        if(strcmp(argv[ix], "-rcal") == 0) {
            read_per_crystal_correction = true;
            input_crystal_cal_filename = string(argv[ix + 1]);
        }
    }

    if (!filenamespec) {
        cout << " Please provide an input file. " << endl;
        usage();
        return(-2);
    }

    rootlogon(verbose);
    gStyle->SetOptStat(kTRUE);

    TCanvas *c1;
    c1 = (TCanvas*)gROOT->GetListOfCanvases()->FindObject("c1");
    if (!c1) c1 = new TCanvas("c1","c1",10,10,1000,1000);
    c1->SetCanvasSize(700,700);

    size_t root_file_ext_pos(filename.rfind(".root"));
    if (root_file_ext_pos == string::npos) {
        cerr << "Unable to find .root extension in: \"" << filename << "\"" << endl;
        if (!output_filename_spec) {
            cerr << "...Exiting." << endl;
            return(-4);
        }
    }
    string filebase(filename, 0, root_file_ext_pos);



    float crystal_cal[SYSTEM_PANELS]
            [CARTRIDGES_PER_PANEL]
            [FINS_PER_CARTRIDGE]
            [MODULES_PER_FIN]
            [APDS_PER_MODULE]
            [CRYSTALS_PER_APD] = {{{{{{0}}}}}};

    if (read_per_crystal_correction) {
        int cal_read_status(ReadPerCrystalCal(
                input_crystal_cal_filename,crystal_cal));
        if (cal_read_status < 0) {
            cerr << "Error in reading input calibration file: "
                 << cal_read_status << endl;
            cerr << "Exiting.." << endl;
            return(-3);
        }
    }


    if (verbose) {
        cout << "Reading Input crystal energy calibration file\n";
    }
    float crystal_edep_cal[SYSTEM_PANELS]
            [CARTRIDGES_PER_PANEL]
            [FINS_PER_CARTRIDGE]
            [MODULES_PER_FIN]
            [APDS_PER_MODULE]
            [CRYSTALS_PER_APD]
            [2] = {{{{{{{0}}}}}}};

    if (read_per_crystal_energy_correction) {
        int cal_read_status(ReadPerCrystalEnergyCal(
                input_per_crystal_energy_cal_filename, crystal_edep_cal));
        if (cal_read_status < 0) {
            cerr << "Error in reading input calibration file: "
                 << cal_read_status << endl;
            cerr << "Exiting.." << endl;
            return(-4);
        }
    }


    if (!output_filename_spec) {
        output_filename = filebase + ".tcal.root";
    }

    if (verbose) {
        cout << " filename    = " << filename << endl;
        cout << " filebase    = " << filebase << endl;
        cout << " output tree = " << output_tree_name << endl;
        cout << " output file = " << output_filename << endl;
    }

    if (verbose) {
        cout << "Opening input file" << endl;
    }
    TFile * input_file = new TFile(filename.c_str(),"OPEN");

    if (!input_file->IsOpen()) {
        cerr << "Problem opening file " << filename;
        cerr << "\n Exiting..." << endl;
        return(-7);
    }

    TTree* input_tree = (TTree *) input_file->Get(input_treename.c_str());
    if (!(input_tree)) {
        cout << " Problem reading " << input_treename << " from file. Exiting." << endl;
        return(-8);
    }
    CoincEvent * input_event = 0;
    input_tree->SetBranchAddress(input_branch_title.c_str(),&input_event);

    if (verbose) {
        cout << "Finished opening input file" << endl;
    }

    Long64_t entries = input_tree->GetEntries();

    if (verbose) {
        cout << "Writing out time calibrated root file" << endl;
    }

    TH1F * tres = new TH1F("tres",
                           "Time Resolution After Time walk correction",
                           400, -100, 100);

    TFile * output_rootfile = 0;
    TTree * output_tree = 0;
    CoincEvent * output_event = new CoincEvent();
    if (write_out_root_file_flag) {
        output_rootfile = new TFile(output_filename.c_str(),"RECREATE");
        output_tree = new TTree(output_tree_name.c_str(), output_tree_title.c_str());
        output_tree->Branch(output_branch_title.c_str(), &output_event);
    }

    for (Long64_t ii = 0; ii < entries; ii++) {
        input_tree->GetEntry(ii);
        output_event = input_event;
        output_event->dtf -= GetEventOffset(*output_event, crystal_cal);
        output_event->dtf -= GetEventOffsetEdep(*output_event, crystal_edep_cal);
        tres->Fill(output_event->dtf);
        if (write_out_root_file_flag) {
            output_tree->Fill();
        }
    }

    if (write_out_root_file_flag) {
        output_tree->Write();
        output_rootfile->Close();
    }

    if (verbose) {
        cout << "Finished writing out file" << endl;
        cout << "Processed events: " << entries << endl;
    }
    if (verbose) {
        cout << "Fitting Time Resolution" << endl;
    }

    tres->Fit("gaus","","",-10,10);
    c1->Clear();
    tres->Draw();
    string ps_tres_filename(filebase + ".tres.ps");
    if (verbose) {
        cout << "Writing tres histogram to: " << ps_tres_filename << endl;
    }
    c1->Print(ps_tres_filename.c_str());

    return(0);
}
