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
         << "  -pn [num] : specify panel number\n"
         << "  -cn [num] : specify cartridge number\n"
         << "  -fn [num] : specify fin number\n"
         << "  -mn [num] : specify module number\n"
         << "  -an [num] : specify apd number\n"
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

    int specific_panel = -1;
    int specific_cartridge = -1;
    int specific_fin = -1;
    int specific_module = -1;
    int specific_apd = -1;

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
        if (argument == "-pn") {
            if (Util::StringToNumber(following_argument, specific_panel) < 0) {
                cerr << "specific_panel number invalid" << endl;
                return(-4);
            }
        }
        if (argument == "-cn") {
            if (Util::StringToNumber(
                        following_argument,specific_cartridge) < 0)
            {
                cerr << "specific_cartridge number invalid" << endl;
                return(-4);
            }
        }
        if (argument == "-fn") {
            if (Util::StringToNumber(following_argument, specific_fin) < 0) {
                cerr << "specific_fin number invalid" << endl;
                return(-4);
            }
        }
        if (argument == "-mn") {
            if (Util::StringToNumber(following_argument, specific_module) < 0) {
                cerr << "specific_module number invalid" << endl;
                return(-4);
            }
        }
        if (argument == "-an") {
            if (Util::StringToNumber(following_argument, specific_apd) < 0) {
                cerr << "specific_apd number invalid" << endl;
                return(-4);
            }
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

    int first_panel = 0;
    int last_panel = config.panels_per_system;
    int first_cart = 0;
    int last_cart = config.cartridges_per_panel;
    int first_fin = 0;
    int last_fin = config.fins_per_cartridge;
    int first_module = 0;
    int last_module = config.modules_per_fin;
    int first_apd = 0;
    int last_apd = config.apds_per_module;

    if (specific_panel >=0) {
        if (specific_panel < config.panels_per_system) {
            first_panel = specific_panel;
            last_panel = first_panel + 1;
        } else {
            cerr << "Invalid specific_panel" << endl;
            return(-5);
        }
    }

    if (specific_cartridge >=0) {
        if (specific_cartridge < config.cartridges_per_panel) {
            first_cart = specific_cartridge;
            last_cart = first_cart + 1;
        } else {
            cerr << "Invalid specific_cartridge" << endl;
            return(-5);
        }
    }

    if (specific_fin >=0) {
        if (specific_fin < config.fins_per_cartridge) {
            first_fin = specific_fin;
            last_fin = first_fin + 1;
        } else {
            cerr << "Invalid specific_fin" << endl;
            return(-5);
        }
    }

    if (specific_module >=0) {
        if (specific_module < config.modules_per_fin) {
            first_module = specific_module;
            last_module = first_module + 1;
        } else {
            cerr << "Invalid specific_module" << endl;
            return(-5);
        }
    }

    if (specific_apd >=0) {
        if (specific_apd < config.apds_per_module) {
            first_apd = specific_apd;
            last_apd = first_apd + 1;
        } else {
            cerr << "Invalid specific_apd" << endl;
            return(-5);
        }
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

    if (verbose) {
        cout << "Getting the floods" << endl;
    }
    for (int p = first_panel; p < last_panel; p++) {
        for (int c = first_cart; c < last_cart; c++) {
            for (int f = first_fin; f < last_fin; f++) {
                for (int m = first_module; m < last_module; m++) {
                    for (int a = first_apd; a < last_apd; a++) {

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
                        peaks[p][c][f][m][a] =
                                new TGraph(config.crystals_per_apd);
                        peaks[p][c][f][m][a]->SetName(name_string);
                    }
                }
            }
        }
    }


    vector<vector<vector<vector<vector<bool> > > > > valid_fit;
    config.resizeArrayPCFMA<bool>(valid_fit, false);

    if (verbose) {
        cout << "Fitting the floods" << endl;
    }
    for (int p = first_panel; p < last_panel; p++) {
        for (int c = first_cart; c < last_cart; c++) {
            for (int f = first_fin; f < last_fin; f++) {
                for (int m = first_module; m < last_module; m++) {
                    for (int a = first_apd; a < last_apd; a++) {
                        if (verbose) {
                            cout << "fitting "
                                 << floods[p][c][f][m][a]->GetName()
                                 << endl;
                        }
                        Int_t validflag = 0;
                        Float_t cost = -1;
                        PeakSearch(
                                peaks[p][c][f][m][a],
                                floods[p][c][f][m][a],
                                0, validflag, cost, a);

                        if (validflag == 15) {
                            valid_fit[p][c][f][m][a] = true;
                        }
                    }
                }
            }
        }
    }
    input_file->Close();

    if (verbose) {
        cout << "Writing out results" << endl;
    }

    ofstream output(filename_output.c_str());
    if (!output.good()) {
        cerr << "Unable to open output file: " << filename_output << endl;
        return(-3);
    }
    for (int p = first_panel; p < last_panel; p++) {
        for (int c = first_cart; c < last_cart; c++) {
            for (int f = first_fin; f < last_fin; f++) {
                for (int m = first_module; m < last_module; m++) {
                    for (int a = first_apd; a < last_apd; a++) {
                        if (valid_fit[p][c][f][m][a]) {
                            TGraph * peak_graph = peaks[p][c][f][m][a];
                            Double_t * x_values = peak_graph->GetX();
                            Double_t * y_values = peak_graph->GetY();
                            for (int ii = 0; ii < config.crystals_per_apd; ii++)
                            {
                                float x = x_values[ii];
                                float y = y_values[ii];
                                output << 1 << " "  << x << " " << y << "\n";
                            }
                        } else {
                            for (int ii = 0; ii < config.crystals_per_apd; ii++)
                            {
                                output << 0 << " " << 0 << " " << 0 << "\n";
                            }
                        }
                    }
                }
            }
        }
    }
    output.close();

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
