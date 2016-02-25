#include <iostream>
#include <sstream>
#include <string>
#include <deque>
#include <vector>
#include <fstream>
#include <miil/SystemConfiguration.h>
#include <miil/process/processing.h>
#include <miil/process/ProcessParams.h>
#include <miil/process/ProcessInfo.h>
#include <miil/util.h>
#include <TH2F.h>
#include <TGraph.h>
#include <TFile.h>
#include <segmentation.h>

using namespace std;

void usage() {
    cout << "fit_floods [-vh] -c [config] -f [filename]\n"
         << "  -o [name] : crystal location output filename\n"
         << endl;
}

int main(int argc, char ** argv) {
    if (argc == 1) {
        usage();
        return(0);
    }

    bool verbose = false;
    string filename;
    string filename_config;
    string filename_output;

    // Arguments not requiring input
    for (int ix = 1; ix < argc; ix++) {
        string argument(argv[ix]);
        if (argument == "-v") {
            verbose = true;
            cout << "Running in verbose mode " << endl;
        }
        if (argument == "-h" || argument == "--help") {
            usage();
            return(0);
        }
    }
    // Arguments requiring input
    for (int ix = 1; ix < (argc - 1); ix++) {
        string argument(argv[ix]);
        string following_argument(argv[ix + 1]);
        if (argument == "-f") {
            filename = following_argument;
        }
        if (argument == "-c") {
            filename_config = following_argument;
        }
        if (argument == "-o") {
            filename_output = following_argument;
        }
    }

    if (filename == "") {
        cerr << "No input filename specified" << endl;
        return(-4);
    }

    if (filename_output == "") {
        filename_output = filename + ".loc";
    }

    if (verbose) {
        cout << "filename       : " << filename << endl;
        cout << "filename_config: " << filename_config << endl;
        cout << "filename_output: " << filename_output << endl;
    }

    SystemConfiguration config;
    int config_load_status = config.load(filename_config);
    if (verbose) {
        cout << "config_load_status: " << config_load_status << endl;
    }
    if (config_load_status < 0) {
        cerr << "SystemConfiguration.load() failed with status: "
             << config_load_status
             << endl;
        return(-2);
    }


    if (verbose) {
        cout << "loading floods: " << filename << endl;
    }
    vector<vector<vector<vector<vector<TH2F*> > > > > floods;
    vector<vector<vector<vector<vector<TGraph*> > > > > peaks;
    config.resizeArrayPCFMA<TH2F*>(floods, 0);
    config.resizeArrayPCFMA<TGraph*>(peaks, 0);


    TFile * input_file = new TFile(filename.c_str());
    if (input_file->IsZombie()) {
        cerr << "Unable to open root file: " << filename << endl;
        return(-3);
    }

    for (int p = 0; p < config.panels_per_system; p++) {
        for (int c = 0; c < config.cartridges_per_panel; c++) {
            for (int f = 0; f < config.fins_per_cartridge; f++) {
                for (int m = 0; m < config.modules_per_fin; m++) {
                    for (int a = 0; a < config.apds_per_module; a++) {

                        // Create all of the 2D flood histograms with the
                        // appropriate names.
                        char name_string[30];
                        sprintf(name_string,
                                "floods[%d][%d][%d][%d][%d]",
                                p, c, f, m, a);
                        floods[p][c][f][m][a] =
                                (TH2F*) input_file->Get(name_string);
                        if (!floods[p][c][f][m][a]) {
                            cerr << "Unable to get flood: " << name_string << endl;
                        }


                        sprintf(name_string,
                                "peaks[%d][%d][%d][%d][%d]",
                                p, c, f, m, a);
                        peaks[p][c][f][m][a] = new TGraph(64);
                        peaks[p][c][f][m][a]->SetName(name_string);
                    }
                }
            }
        }
    }


    for (int p = 0; p < config.panels_per_system; p++) {
        for (int c = 0; c < config.cartridges_per_panel; c++) {
            for (int f = 0; f < config.fins_per_cartridge; f++) {
                for (int m = 0; m < config.modules_per_fin; m++) {
                    for (int a = 0; a < config.apds_per_module; a++) {
                        if (verbose) {
                            cout << "fitting " << floods[p][c][f][m][a]->GetName() << endl;
                        }
                        Int_t validflag;
                        Float_t cost;
                        PeakSearch(
                                peaks[p][c][f][m][a],
                                floods[p][c][f][m][a],
                                0, validflag, cost, a);
                    }
                }
            }
        }
    }

//    TFile * output_file = new TFile("test.root", "RECREATE");
//    Int_t validflag;
//    Float_t cost;
//    PeakSearch(peaks[0][1][1][1][0],
//               floods[0][1][1][1][0],
//               0, validflag, cost, 0);

//    output_file->cd();
//    peaks[0][1][1][1][0]->Write();

//    input_file->Close();


//    if (verbose) {
//        cout << "Writing out floods: " << filename_output << endl;
//    }

//    TFile * output_file = new TFile(filename_output.c_str(), "RECREATE");
//    output_file->cd();
//    for (int p = 0; p < config.panels_per_system; p++) {
//        for (int c = 0; c < config.cartridges_per_panel; c++) {
//            for (int f = 0; f < config.fins_per_cartridge; f++) {
//                for (int m = 0; m < config.modules_per_fin; m++) {
//                    for (int a = 0; a < config.apds_per_module; a++) {
//                        TH2F * flood = floods[p][c][f][m][a];
//                        flood->Write();
//                    }
//                }
//            }
//        }
//    }
//    output_file->Close();
    return(0);
}
