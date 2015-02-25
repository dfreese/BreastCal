#include "TROOT.h"
#include "Riostream.h"
#include "TTree.h"
#include "TFile.h"
#include "libInit_avdb.h"
#include "myrootlib.h"
#include "Syspardef.h"
#include "ModuleCal.h"
#include "ModuleDat.h"
#include "PixelCal.h"
#include "TVector.h"
#include "sorting.h"
#include <vector>
#include <sstream>

void usage(void) {
    cout << " Energy Gate calibrated files.\n\n";
    cout << " Usage:  " << endl;
    cout << " ./energy_gate_file [-v] -f [Filename] -el [Low Threshold] -eh [High Threshold]" << endl;
    cout << " -of: Specifies output filename. Default replaces .root\n"
         << "      extension with .egate_[low]_[high].root\n"
         << " -t:  Optionally sets the expected input tree name.\n"
         << "      Default is SCalTree\n"
         << " -ot: Optionally sets the output tree name.\n"
         << "      Default is SCalTree\n"
         << " -el: Set Lower Energy Window - enables energy gating\n"
         << "      Default is 450keV\n"
         << " -eh: Set Upper Energy Window - enables energy gating\n"
         << "      Default is 650keV\n";
}

int main(int argc, char ** argv) {
    if (argc == 1) {
        usage();
        return(0);
    }

    string filename;
    bool filenamespec(false);
    Int_t verbose = 0;
    string input_treename("SCalTree");
    string output_tree_name("SCalTree");
    string output_tree_title("Time Sorted Calibrated data");
    string output_branch_title("Time Sorted Data");
    string output_filename;
    bool output_filename_spec(false);
    float energy_gate_high(650);
    float energy_gate_low(450);

    for(int ix = 1; ix < argc; ix++) {
        if (strncmp(argv[ix], "-h", 2) == 0) {
            usage();
            return(0);
        }
        if (strncmp(argv[ix], "-v", 2) == 0) {
            cout << "Verbose Mode " << endl;
            verbose = 1;
        }
    }

    for(int ix = 1; ix < (argc-1); ix++) {
        if (strncmp(argv[ix], "-f", 2) == 0) {
            filename = string(argv[ix + 1]);
            filenamespec = true;
            ix++;
        }
        if (strncmp(argv[ix], "-of", 3) == 0) {
            output_filename = string(argv[ix + 1]);
            output_filename_spec = true;
            ix++;
        }
        if (strncmp(argv[ix], "-t", 2) == 0) {
            input_treename = string(argv[ix + 1]);
            ix++;
        }
        if (strncmp(argv[ix], "-ot", 3) == 0) {
            output_tree_name = string(argv[ix + 1]);
            ix++;
        }
        if (strncmp(argv[ix], "-eh", 3) == 0) {
            stringstream ss;
            ss << argv[ix + 1];
            ss >> energy_gate_high;
            ix++;
        }
        if (strncmp(argv[ix], "-el", 3) == 0) {
            stringstream ss;
            ss << argv[ix + 1];
            ss >> energy_gate_low;
            ix++;
        }
    }

    if (!filenamespec) {
        cout << " Please provide an input file. " << endl;
        usage();
        return(-2);
    }

    size_t root_file_ext_pos(filename.rfind(".root"));
    if (root_file_ext_pos == string::npos) {
        cerr << "Unable to find .root extension in: \"" << filename << "\"" << endl;
        if (!output_filename_spec) {
            cerr << "...Exiting." << endl;
            return(-4);
        }
    }
    string filebase(filename, 0, root_file_ext_pos);

    if (!output_filename_spec) {
        stringstream ss;
        ss << filebase << ".egate_" << energy_gate_low
           << "_"<< energy_gate_high << ".root";
        output_filename = ss.str();
    }

    if (verbose) {
        cout << " filename    = " << filename << endl;
        cout << " filebase    = " << filebase << endl;
        cout << " output tree = " << output_tree_name << endl;
        cout << " output file = " << output_filename << endl;
        cout << "Energy Window: " << energy_gate_low 
             << " to " << energy_gate_high << endl;
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
    ModuleCal * input_event = 0;
    input_tree->SetBranchAddress("Time Sorted Data",&input_event);

    if (verbose) {
        cout << "Finished opening input file" << endl;
    }

    Long64_t entries = input_tree->GetEntries();

    if (verbose) {
        cout << "Writing out energy gated root file" << endl;
    }

    TFile * output_rootfile = new TFile(output_filename.c_str(),"RECREATE");
    TTree * output_tree = new TTree(output_tree_name.c_str(), output_tree_title.c_str());
    ModuleCal * output_event = new ModuleCal();
    output_tree->Branch(output_branch_title.c_str(), &output_event);

    long dropped_energy_gate(0);
    for (Long64_t ii = 0; ii < entries; ii++) {
        input_tree->GetEntry(ii);
        output_event = input_event;

        float event_energy = input_event->Ecal;

        if ((event_energy > energy_gate_low) && (event_energy < energy_gate_high)) {
            output_tree->Fill();
        } else {
            dropped_energy_gate++;
        }
    }

    if (verbose) {
        cout << "Finished writing out file" << endl;
        if (entries) {
            cout << "Processed events: " << entries << endl;
            cout << "Dropped events:\n"
                << "  (" << 100 * float(dropped_energy_gate) / float(entries) << " %)\n";
        }
    }

    output_tree->Write();
    output_rootfile->Close();

    return(0);
}
