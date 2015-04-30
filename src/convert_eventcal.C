#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include "Riostream.h"
#include "TMath.h"
#include "TVector.h"
#include "Util.h"
#include "ModuleCal.h"
#include "EventCal.h"
#include "daqboardmap.h"
#include "daqpacket.h"
#include "decoderlib.h"
#include "cal_helper_functions.h"
#include "chipevent.h"
#include "sorting.h"
#include "libInit_avdb.h"
#include "myrootlib.h"
#include "Syspardef.h"
#include "EventCoinc.h"

using namespace std;

void usage(void)
{
    cout << "coinc_sort (-v) -f [filename] -o [output]\n"
         << endl;
    return;
}

int main(int argc, char *argv[])
{
    if (argc == 1) {
        usage();
        return(0);
    }

    bool verbose(false);
    string filename;
    string output_filename;

    string output_tree_name("SCalTree");
    string output_tree_title("Time Sorted Calibrated data");
    string output_branch_title("Time Sorted Data");

    float energy_gate_high(650);
    float energy_gate_low(450);
    bool energy_gate_output(false);

    // Arguments not requiring input
    for (int ix = 1; ix < argc; ix++) {
        if (strcmp(argv[ix], "-v") == 0) {
            verbose = true;
        }
    }
    // Arguments requiring input
    for (int ix = 1; ix < (argc - 1); ix++) {
        if (strcmp(argv[ix], "-f") == 0) {
            filename = string(argv[ix + 1]);
        }
        if (strcmp(argv[ix], "-o") == 0) {
            output_filename = string(argv[ix + 1]);
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

    if (filename == "") {
        printf("Please Specify Input Filename !!\n");
        usage();
        return(-1);
    }
    if (output_filename == "") {
        printf("Please Specify Output Filename !!\n");
        usage();
        return(-1);
    }

    if (verbose) {
        cout << "Reading File" << endl;
    }

    ifstream dataFile;
    dataFile.open(filename.c_str(), ios::in | ios::binary);
    if (!dataFile.good()) {
        cerr << "Cannot open file \"" << filename
             << "\" for read operation." << endl;
        cerr << "Exiting." << endl;
        return(-1);
    }

    // Read the full input data file into memory
    dataFile.seekg(0, std::ios::end);
    std::streamsize dataFileSize = dataFile.tellg();
    dataFile.seekg(0, std::ios::beg);

    std::vector<EventCal> input_events(dataFileSize / sizeof(EventCal));
    if (!dataFile.read((char*) input_events.data(), dataFileSize)) {
        cerr << "Left File read failed" << endl;
        return(-2);
    }
    dataFile.close();


    TFile * output_rootfile = new TFile(output_filename.c_str(),"RECREATE");
    TTree * output_tree = new TTree(output_tree_name.c_str(), output_tree_title.c_str());
    ModuleCal * output_event = new ModuleCal();
    output_tree->Branch(output_branch_title.c_str(), &output_event);

    for (size_t ii = 0; ii < input_events.size(); ii++) {
        if (!energy_gate_output ||
                (input_events[ii].E > energy_gate_low &&
                 input_events[ii].E < energy_gate_high))
        {
            output_event->ct = input_events[ii].ct;
//            output_event->ft = input_events[ii].ft / UV_PERIOD_NS * 2 * M_PI;
            output_event->ft = input_events[ii].ft;
            output_event->Ecal = input_events[ii].E;
            output_event->E = input_events[ii].E;
            output_event->Ec = input_events[ii].E;
            output_event->Ech = input_events[ii].E;
            output_event->x = input_events[ii].x;
            output_event->y = input_events[ii].y;
            output_event->chip = 0;
            output_event->cartridge = ((int) (input_events[ii].crystal / 16384)) % 2;
            output_event->fin = ((int) (input_events[ii].crystal / 2048)) % 8;
            output_event->m = ((int) (input_events[ii].crystal / 128)) % 16;
            output_event->apd = ((int) (input_events[ii].crystal / 64)) % 2;
            output_event->id = input_events[ii].crystal % 64;
            output_event->pos = 0;

            output_tree->Fill();
        }
    }


    output_tree->Write();
    output_rootfile->Close();

    return(0);
}

