#include "TROOT.h"
#include "Riostream.h"
#include "TTree.h"
#include "TFile.h"
#include "libInit_avdb.h"
#include "myrootlib.h"
#include "Syspardef.h"
#include "CoincEvent.h"
#include <vector>
#include <sstream>
#include <armadillo>
#include <algorithm>
#include <fstream>

void usage(void)
{
    cout << " Usage:  " << endl;
    cout << " ./time_cal [-v] -f [Filename]" << endl;
    cout << " -of: Specifies output filename. Default replaces .cal.root\n"
         << "      extension with .cal.sort.root\n"
         << " -t:  Optionally sets the expected input tree name.\n"
         << "      Default is merged\n"
         << " -ot: Optionally sets the output tree name.\n"
         << "      Default is merged\n"
         << " -a:  Output ascii file of parameters and dtf used for calibration\n";
    return;
}

int main(int argc, Char_t *argv[])
{
    string filename;
    Int_t verbose = 0;
    TTree* input_tree;
    string input_treename("merged");
    string output_treename("merged");
    string output_filename;
    bool output_filename_spec(false);
    bool output_ascii_flag(false);

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
        if(strncmp(argv[ix], "-a", 2) == 0) {
            output_ascii_flag = true;
            ix++;
        }
    }

    if (filename == "") {
        cout << " Please provide an input file. " << endl;
        usage();
        return(-2);
    }

    size_t root_file_ext_pos(filename.rfind(".root"));
    if (root_file_ext_pos == string::npos) {
        cerr << "Unable to find .root extension in: \"" << filename << "\"" << endl;
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
    }
    TFile * input_file= new TFile(filename.c_str(),"OPEN");

    if (!input_file->IsOpen()) {
        cout << "problems opening file " << filename;
        cout << "\n Exiting " << endl;
        return(-6);
    }

    input_tree = (TTree *) input_file->Get(input_treename.c_str());
    if (!(input_tree)) {
        cout << " Problem reading " << input_treename << " from file. Exiting." << endl;
        return(-7);
    }

    CoincEvent * input_event = 0;
    input_tree->SetBranchAddress("Event",&input_event);


    if (!output_filename_spec) {
        output_filename = filebase + "tcal.root";
    }
    if (verbose) {
        cout << "Output File: " << output_filename << endl;
    }


    Long64_t entries=input_tree->GetEntries();

    if (verbose) {
        cout << "Gathering Timestamp Data" << endl;
    }
    // Get all timestamps and sort events
    int nTerms(1024);
    Long64_t length(min(entries, Long64_t(1000000)));
    cout << "Armadillo: Allocating matrix space: " << length << " x " << nTerms << endl;
    arma::mat A_arma(length,nTerms);
    arma::vec y_arma(length);
    ofstream output_text;
    if (output_ascii_flag) {
        output_text.open("system_output.txt");
    }

    for (Long64_t ii = 0; ii < length; ii++) {
        input_tree->GetEntry(ii);
        int apd1 = (((0 * CARTRIDGES_PER_PANEL + input_event->cartridge1) * 
                    FINS_PER_CARTRIDGE + input_event->fin1) * 
                    MODULES_PER_FIN + input_event->m1) *
                    APDS_PER_MODULE + input_event->apd1; 
        int apd2 = (((1 * CARTRIDGES_PER_PANEL + input_event->cartridge2) * 
                    FINS_PER_CARTRIDGE + input_event->fin2) * 
                    MODULES_PER_FIN + input_event->m2) *
                    APDS_PER_MODULE + input_event->apd2; 
        A_arma(ii, apd1) = 1;
        A_arma(ii, apd2) = -1;
        y_arma(ii) = input_event->dtf;
        if (output_ascii_flag) {
            float dtf = input_event->dtf;
            int crystal1 = input_event->crystal1;
            int crystal2 = input_event->crystal2;
            output_text << apd1 << " " << apd2 << " " << crystal1 << " " << crystal2 << " " << dtf << "\n";
        }
    }
    output_text.close();

    if (verbose) {
        cout << "Finished Gathering Timestamp Data" << endl;
        cout << "Solving Linear System" << endl;
    }
    arma::vec x_arma(nTerms);
    bool arma_status(arma::solve(x_arma, A_arma, y_arma));

    if (!arma_status) {
        cerr << "Solve Failed" << endl;
        cerr << "Exiting..." << endl;
        return(-9);
    }
    if (verbose) {
        cout << "Finished Solving Linear System" << endl;
    }

    /*
    if (verbose) {
        cout << "Sorting Data" << endl;
    }
    insertion_sort_with_key(timestamp, key);
    if (verbose) {
        cout << "Sorting Complete" << endl;
    }
    */

    /*
    if (verbose) {
        cout << "Writing out sorted root file" << endl;
    }
    TFile * output_rootfile = new TFile(output_filename.c_str(),"RECREATE");
    TTree * output_tree= new TTree("SCalTree","Time Sorted Calibrated data");
    ModuleCal * sorted_event = new ModuleCal();
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
    */

    return(0);
}
