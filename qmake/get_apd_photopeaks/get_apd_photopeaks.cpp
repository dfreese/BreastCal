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
#include <miil/file_utils.h>
#include <TH1F.h>
#include <TSpectrum.h>
#include <TError.h>
#include <TFile.h>

using namespace std;

// getfloods - Fitting the APD photopeaks:
// Give up if the histogram has less than 200 entries
// Otherwise call GetApdPhotopeak with a width of 12, and a window of 700 to
// 2800 for spatials, or 350 to 1200 for commons.

// Peak search using sigma = width (always 12 to start) and threshold = 0.6
// If at least one peak is found within the window, return the furthest right
// else lower the threshold by 0.1 down to and including 0.1 and repeat search.
//
// Peak search using sigma = width and threshold = 0.05
// If at least one peak is found within the window, return the furthest right.
//
// If the width is above 3, lower it by 2 and start over.
//
// Return the lower window value.
float GetApdPhotopeak(
        TH1F *hist,
        float pp_low = 700,
        float pp_up = 2800,
        int width = 12)
{
    if (hist->GetEntries() < 1000) {
        return(0);
    }
    gErrorIgnoreLevel = 5001;
    TSpectrum sp;
    do {
        float thresholds[7] = {0.6, 0.5, 0.4, 0.3, 0.2, 0.1, 0.05};
        // float thresholds[4] = {0.3, 0.2, 0.1, 0.05};
        // float thresholds[4] = {0.2, 0.1, 0.05};
        for (size_t thresh_idx = 0;
             thresh_idx < sizeof(thresholds) / sizeof(float);
             thresh_idx++)
        {
            Int_t npeaks = sp.Search(hist, width, "", thresholds[thresh_idx]);
            if (npeaks < 2 &&
                thresh_idx != (sizeof(thresholds) / sizeof(float) - 1))
            {
                continue;
            }
            // get the peak between E_low and E_up ;
            bool efound = false;
            // Initialize peak to lower bound
            Double_t pp_right = pp_low;
            for (int l = 0; l < npeaks; l++) {
                // look for the peak with the highest x value
                if ((*(sp.GetPositionX() + l) < pp_up ) &&
                    (*(sp.GetPositionX() + l) > pp_right))
                {
                    pp_right = (*(sp.GetPositionX() + l));
                    efound = true;
                }
            }
            if (efound) {
                return(pp_right);
            }
        }
        width -= 2;
    } while (width > 1);
    return(0);
}

void usage() {
    cout << "get_apd_photopeaks [-vh] -c [config] -p [ped file] -f [filename] -f ...\n"
         << "  -o [name] : photopeak output filename\n"
         << "  -l [name] : list file of input filenames\n"
         << "  -ro [name]: optional root output file for energy spectra\n"
         << endl;
}

int main(int argc, char ** argv) {
    if (argc == 1) {
        usage();
        return(0);
    }

    bool verbose = false;
    vector<string> filenames;
    string filename_config;
    string filename_output;
    string filename_ped;
    string filename_root_output;


    int Ebins = 320;
    int E_up = 3000;
    int E_low = -200;

    int Ebins_com = 160;
    int E_up_com = 1400;
    int E_low_com = -200;


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
            filenames.push_back(following_argument);
        }
        if (argument == "-c") {
            filename_config = following_argument;
        }
        if (argument == "-p") {
            filename_ped = following_argument;
        }
        if (argument == "-o") {
            filename_output = following_argument;
        }
        if (argument == "-ro") {
            filename_root_output = following_argument;
        }
        if (argument == "-l") {
            if (Util::loadFilelistFile(following_argument, filenames) < 0) {
                cerr << "Unable to load filelist: "
                     << following_argument << endl;
                return(-2);
            }
        }
    }

    if (filenames.empty()) {
        cerr << "No filenames specified" << endl;
        return(-4);
    }

    if (filename_output == "") {
        if (filenames.size() == 1) {
            filename_output = filenames[0] + ".pp";
        }
    }

    // Bail if neither of the output files have been specified
    if (filename_output == "") {
        cerr << "No output filename specified" << endl;
        return(-5);
    }

    if (verbose) {
        cout << "filenames:       " << Util::vec2String(filenames) << endl;
        cout << "filename_ped:    " << filename_ped << endl;
        cout << "filename_output: " << filename_output << endl;
        cout << "filename_config: " << filename_config << endl;
        cout << "filename_root_output: " << filename_root_output << endl;
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

    int ped_load_status = config.loadPedestals(filename_ped);
    if (verbose) {
        cout << "ped_load_status: " << ped_load_status << endl;
    }
    if (ped_load_status < 0) {
        cerr << "SystemConfiguration.loadPedestals() failed with status: "
             << ped_load_status
             << endl;
        return(-3);
    }


    vector<vector<vector<vector<vector<float> > > > > spat_photopeaks;
    vector<vector<vector<vector<vector<float> > > > > comm_photopeaks;
    vector<vector<vector<vector<vector<TH1F*> > > > > spat_hists;
    vector<vector<vector<vector<vector<TH1F*> > > > > comm_hists;
    config.resizeArrayPCFMA<TH1F*>(spat_hists, 0);
    config.resizeArrayPCFMA<TH1F*>(comm_hists, 0);
    config.resizeArrayPCFMA<float>(spat_photopeaks, 0);
    config.resizeArrayPCFMA<float>(comm_photopeaks, 0);
    for (int p = 0; p < config.panels_per_system; p++) {
        for (int c = 0; c < config.cartridges_per_panel; c++) {
            for (int f = 0; f < config.fins_per_cartridge; f++) {
                for (int m = 0; m < config.modules_per_fin; m++) {
                    for (int a = 0; a < config.apds_per_module; a++) {
                        char namestring[30];
                        char titlestring[50];
                        sprintf(namestring,
                                "E[%d][%d][%d][%d][%d]",
                                p, c, f, m, a);
                        sprintf(titlestring,
                                "E P%dC%dF%dM%dA%d ",
                                p, c, f, m, a);
                        spat_hists[p][c][f][m][a] = new TH1F(
                                    namestring, titlestring,
                                    Ebins, E_low, E_up);
                        sprintf(namestring,
                                "E_com[%d][%d][%d][%d][%d]",
                                p, c, f, m, a);
                        sprintf(titlestring,
                                "ECOM P%dC%dF%dM%dA%d ",
                                p, c, f, m, a);
                        comm_hists[p][c][f][m][a] = new TH1F(
                                    namestring, titlestring,
                                    Ebins_com, E_low_com, E_up_com);
                    }
                }
            }
        }
    }

    if (verbose) {
        cout << "Filling Energy Histograms" << endl;
    }
    deque<char> file_data;
    ProcessInfo info;
    for (size_t ii = 0; ii < filenames.size(); ii++) {
        string & filename = filenames[ii];
        int read_status = Util::readFileIntoDeque(filename, file_data);
        if (verbose) {
            cout << filename << " read with status: " << read_status << endl;
        }
        if (read_status < 0) {
            cerr << "Unable to load: " << filename << endl;
            return(-3);
        }
        vector<EventRaw> raw_events;

        ProcessParams::DecodeBuffer(file_data, raw_events, info, &config);
        ProcessParams::ClearProcessedData(file_data, info);
        for (size_t ii = 0; ii < raw_events.size(); ii++) {
            EventRaw & event = raw_events[ii];
            int apd, module, fin;
            float x, y, energy;
            int calc_status = CalculateXYandEnergy(
                        event, &config, x, y, energy, apd, module, fin);
            if (calc_status < 0) {
                continue;
            }

            TH1F * spat_hist =
                    spat_hists[event.panel][event.cartridge][fin][module][apd];
            TH1F * comm_hist =
                    comm_hists[event.panel][event.cartridge][fin][module][apd];

            spat_hist->Fill(energy);
            if (apd == 0) {
                comm_hist->Fill(event.com0);
            } else if (apd == 1) {
                comm_hist->Fill(event.com1);
            }
        }
    }

    if (verbose) {
        cout << "Finding photopeaks" << endl;
    }
    for (int p = 0; p < config.panels_per_system; p++) {
        for (int c = 0; c < config.cartridges_per_panel; c++) {
            for (int f = 0; f < config.fins_per_cartridge; f++) {
                for (int m = 0; m < config.modules_per_fin; m++) {
                    for (int a = 0; a < config.apds_per_module; a++) {
                        spat_photopeaks[p][c][f][m][a] = GetApdPhotopeak(
                                    spat_hists[p][c][f][m][a], 700, 2800, 12);

                        comm_photopeaks[p][c][f][m][a] = GetApdPhotopeak(
                                    comm_hists[p][c][f][m][a], 350, 1200, 12);
                    }
                }
            }
        }
    }

    if (verbose) {
        cout << "Writing photopeaks: " << filename_output << endl;
    }
    // Write out to the photopeak file
    ofstream pp_output(filename_output.c_str());
    for (int p = 0; p < config.panels_per_system; p++) {
        for (int c = 0; c < config.cartridges_per_panel; c++) {
            for (int f = 0; f < config.fins_per_cartridge; f++) {
                for (int m = 0; m < config.modules_per_fin; m++) {
                    for (int a = 0; a < config.apds_per_module; a++) {
                        pp_output << std::fixed << std::setprecision(1)
                                  << spat_photopeaks[p][c][f][m][a] << " "
                                  << comm_photopeaks[p][c][f][m][a] << "\n";
                    }
                }
            }
        }
    }
    pp_output.close();


    if (filename_root_output != "") {
        if (verbose) {
            cout << "Writing histograms to " << filename_root_output << endl;
        }
        TFile * output_file =
                new TFile(filename_root_output.c_str(), "RECREATE");
        if (output_file->IsZombie()) {
            cerr << "Unable to open root output file: "
                 << filename_root_output << endl;
            return(-6);
        }
        output_file->cd();
        for (int p = 0; p < config.panels_per_system; p++) {
            for (int c = 0; c < config.cartridges_per_panel; c++) {
                for (int f = 0; f < config.fins_per_cartridge; f++) {
                    for (int m = 0; m < config.modules_per_fin; m++) {
                        for (int a = 0; a < config.apds_per_module; a++) {
                            // Copy the loaded photopeaks into their vectors
                            spat_hists[p][c][f][m][a]->Write();
                            comm_hists[p][c][f][m][a]->Write();
                        }
                    }
                }
            }
        }
        output_file->Close();
    }



    if (verbose) {
        cout << info.getDecodeInfo() << endl;
    }
    return(0);
}
