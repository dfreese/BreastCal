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

void usage(void)
{
    cout << " Usage:  " << endl;
    cout << " ./sort_file -f [Filename] [-v] -of [output filename] -t [input tree name] -ot [output tree name]" << endl;
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

        if(strncmp(argv[ix], "-of", 2) == 0) {
            output_filename = string(argv[ix + 1]);
            output_filename_spec = true;
            ix++;
        }

        if(strncmp(argv[ix], "-t", 2) == 0) {
            input_treename = string(argv[ix + 1]);
            ix++;
        }

        if(strncmp(argv[ix], "-ot", 2) == 0) {
            output_treename = string(argv[ix + 1]);
            ix++;
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
        cerr << "...Exiting." << endl;
        return(-5);
    }
    string filebase(filename, 0, root_file_ext_pos);


    if (verbose) {
        cout << " filename = " << filename << endl;
        cout << " filebase = " << filebase << endl;
        cout << " Opening file " << filename << endl;
    }
    TFile * rootfile= new TFile(filename.c_str(),"OPEN");

    if (!rootfile->IsOpen()) {
        cout << "problems opening file " << filename;
        cout << "\n Exiting " << endl;
        return(-6);
    }

    input_tree = (TTree *) rootfile->Get(input_treename.c_str());
    if (!(input_tree))    {
        cout << " Problem reading " << input_treename << " from file. Exiting." << endl;
        return(-7);
    }

    input_tree->SetBranchAddress("Calibrated Event Data",&unsrt_evt);


    if (!output_filename_spec) {
        output_filename = filebase + ".cal.sort.root";
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
        sorted_event = unsrt_evt;
        output_tree->Fill();
    }
    if (verbose) {
        cout << "Finished writing out sorted root file" << endl;
    }

    output_tree->Write();
    output_rootfile->Close();

    return(0);
}
