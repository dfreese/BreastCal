#include "TROOT.h"
#include "Riostream.h"
#include "TTree.h"
#include "TFile.h"
#include "libInit_avdb.h"
#include "myrootlib.h"
#include "Syspardef.h"
#include "ModuleCal.h"
#include "sorting.h"
#include <vector>
#include <sstream>

void usage(void)
{
    cout << " Usage:  " << endl;
    cout << " ./sort_file [-v,-eg] -f [Filename]" << endl;
    cout << " -eg: Energy Gate output. Default output file extension"
         << "      becomes \".sort.egate.root\""
         << " -of: Specifies output filename. Default replaces .cal.root"
         << "      extension with .cal.sort.root"
         << " -t:  Optionally sets the expected input tree name."
         << "      Default is CalTree"
         << " -ot: Optionally sets the output tree name."
         << "      Default is SCalTree"
         << " -eh: Set Upper Energy Window - enables energy gating"
         << "      Default is 700keV"
         << " -el: Set Lower Energy Window - enables energy gating"
         << "      Default is 400keV";
    return;
}

int main(int argc, Char_t *argv[])
{
    string filename;
    bool filenamespec(false);
    Int_t verbose = 0;
    ModuleCal * sorted_event = new ModuleCal();
    ModuleCal *unsrt_evt=0;
    TTree* input_tree;
    string input_treename("CalTree");
    string output_treename("SCalTree");
    string output_filename;
    bool output_filename_spec(false);
    float energy_gate_high(650);
    float energy_gate_low(450);
    bool energy_gate_output(false);


    for(int ix = 1; ix < argc; ix++) {
        if(strncmp(argv[ix], "-h", 2) == 0) {
            usage();
            return(0);
        }

        if(strncmp(argv[ix], "-v", 2) == 0) {
            cout << "Verbose Mode " << endl;
            verbose = 1;
        }

        if(strncmp(argv[ix], "-f", 2) == 0) {
            filename = string(argv[ix + 1]);
            filenamespec = true;
            ix++;
        }

        if(strncmp(argv[ix], "-of", 3) == 0) {
            output_filename = string(argv[ix + 1]);
            output_filename_spec = true;
            ix++;
        }

        if(strncmp(argv[ix], "-t", 2) == 0) {
            input_treename = string(argv[ix + 1]);
            ix++;
        }

        if(strncmp(argv[ix], "-ot", 3) == 0) {
            output_treename = string(argv[ix + 1]);
            ix++;
        }
 
        if(strncmp(argv[ix], "-eg", 2) == 0) {
            energy_gate_output = true;
        }       
        if(strncmp(argv[ix], "-eh", 3) == 0) {
            stringstream ss;
            ss << argv[ix + 1];
            ss >> energy_gate_high;
            ix++;
            energy_gate_output = true;
        }
        if(strncmp(argv[ix], "-el", 3) == 0) {
            stringstream ss;
            ss << argv[ix + 1];
            ss >> energy_gate_low;
            ix++;
            energy_gate_output = true;
        }
    }

    if (!filenamespec) {
        cout << " Please provide an input file. " << endl;
        usage();
        return(-2);
    }


    size_t root_file_ext_pos(filename.rfind(".cal.root"));
    if (root_file_ext_pos == string::npos) {
        cerr << "Unable to find .cal.root extension in: \"" << filename << "\"" << endl;
        if (!output_filename_spec) {
            cerr << "...Exiting." << endl;
            return(-5);
        }
    }
    string filebase(filename, 0, root_file_ext_pos);


    if (verbose) {
        cout << " filename = " << filename << endl;
        cout << " filebase = " << filebase << endl;
        cout << " Opening file " << filename << endl;
        if (energy_gate_output) {
            cout << "Energy Window: " << energy_gate_low 
                 << " to " << energy_gate_high << endl;
        }
    }
    TFile * rootfile= new TFile(filename.c_str(),"OPEN");

    if (!rootfile->IsOpen()) {
        cout << "problems opening file " << filename;
        cout << "\n Exiting " << endl;
        return(-6);
    }

    input_tree = (TTree *) rootfile->Get(input_treename.c_str());
    if (!(input_tree)) {
        cout << " Problem reading " << input_treename << " from file. Exiting." << endl;
        return(-7);
    }

    input_tree->SetBranchAddress("Calibrated Event Data",&unsrt_evt);


    if (!output_filename_spec) {
        if (energy_gate_output) {
            output_filename = filebase + ".cal.sort.egate.root";
        } else {
            output_filename = filebase + ".cal.sort.root";
        }
    }
    if (verbose) {
        cout << "Output File: " << output_filename << endl;
    }


    Long64_t entries=input_tree->GetEntries();

    std::vector<Long64_t> key(entries, 0);
    std::vector<Long64_t> timestamp(entries, 0);

    if (verbose) {
        cout << "Gathering Timestamp Data" << endl;
    }
    // Get all timestamps and sort events
    for (Long64_t ii = 0; ii < entries; ii++) {
        key[ii] = ii;
        input_tree->GetEntry(ii);
        timestamp[ii] = unsrt_evt->ct;
    }

    if (verbose) {
        cout << "Sorting Data" << endl;
    }
    insertion_sort_with_key(timestamp, key);
    if (verbose) {
        cout << "Sorting Complete" << endl;
    }

    if (verbose) {
        cout << "Writing out sorted root file" << endl;
    }
    TFile * output_rootfile = new TFile(output_filename.c_str(),"RECREATE");
    TTree * output_tree= new TTree("SCalTree","Time Sorted Calibrated data");
    output_tree->Branch("Time Sorted Data",&sorted_event);
    // Generate Sorted Dataset
    for (Long64_t ii = 0; ii < entries; ii++) {
        input_tree->GetEntry(key[ii]);
        Float_t event_energy = unsrt_evt->Ecal;
        // Write out the event if it's in the energy window or if we're not
        // energy gating the output
        if ((!energy_gate_output) || ((event_energy > energy_gate_low) && (event_energy < energy_gate_high))) {
            sorted_event = unsrt_evt;
            output_tree->Fill();
        }
    }
    if (verbose) {
        cout << "Finished writing out sorted root file" << endl;
    }

    output_tree->Write();
    output_rootfile->Close();

    return(0);
}
