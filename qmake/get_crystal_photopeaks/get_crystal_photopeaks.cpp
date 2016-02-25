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
#include <TH1F.h>
#include <TSpectrum.h>
#include <TError.h>

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

template<typename T>
int readFileIntoDeque(const string & filename, deque<T> & container) {
    ifstream file(filename.c_str(), ios::binary);
    if (!file.good()) {
        return(-1);
    }

    size_t chunk_size = 1024 * 1024;
    vector<char> chunk(chunk_size);

    while (file.read((char*) chunk.data(), chunk.size()) || file.gcount()) {
        container.insert(
                container.end(), chunk.begin(), chunk.begin() + file.gcount());
    }
    return(0);
}

void usage() {
    cout << "get_crystal_photopeaks [-vh] -c [config] -p [ped file] -x [loc file] -f [filename] -f ...\n"
         << "  -o [name] : photopeak output filename\n"
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
    string filename_loc;


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
        if (argument == "-x") {
            filename_loc = following_argument;
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
        cout << "filename_config: " << filename_config << endl;
        cout << "filename_ped:    " << filename_ped << endl;
        cout << "filename_loc:    " << filename_loc << endl;
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


    int loc_load_status = config.loadCrystalLocations(filename_loc);
    if (loc_load_status < 0) {
        cerr << "SystemConfiguration.loadCrystalLocations() failed, status: "
             << loc_load_status
             << endl;
        return(-4);
    }

    vector<vector<vector<vector<vector<vector<TH1F*> > > > > > spat_hists;
    vector<vector<vector<vector<vector<vector<TH1F*> > > > > > comm_hists;
    config.resizeArrayPCFMAX<TH1F*>(spat_hists, 0);
    config.resizeArrayPCFMAX<TH1F*>(comm_hists, 0);
    for (int p = 0; p < config.panels_per_system; p++) {
        for (int c = 0; c < config.cartridges_per_panel; c++) {
            for (int f = 0; f < config.fins_per_cartridge; f++) {
                for (int m = 0; m < config.modules_per_fin; m++) {
                    for (int a = 0; a < config.apds_per_module; a++) {
                        for (int x = 0; x < config.crystals_per_apd; x++) {
                            char namestring[30];
                            char titlestring[50];
                            sprintf(namestring,
                                    "E[%d][%d][%d][%d][%d][%d]",
                                    p, c, f, m, a, x);
                            sprintf(titlestring,
                                    "E P%dC%dF%dM%dA%dX%d ",
                                    p, c, f, m, a, x);
                            spat_hists[p][c][f][m][a][x] = new TH1F(
                                        namestring, titlestring,
                                        Ebins, E_low, E_up);
                            sprintf(namestring,
                                    "E_com[%d][%d][%d][%d][%d][%d]",
                                    p, c, f, m, a, x);
                            sprintf(titlestring,
                                    "ECOM P%dC%dF%dM%dA%dX%d ",
                                    p, c, f, m, a, x);
                            comm_hists[p][c][f][m][a][x] = new TH1F(
                                        namestring, titlestring,
                                        Ebins_com, E_low_com, E_up_com);
                        }
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
        int read_status = readFileIntoDeque(filename, file_data);
        if (verbose) {
            cout << filename << " read with status: " << read_status << endl;
        }
        if (read_status < 0) {
            cerr << "Unable to load: " << filename << endl;
            return(-3);
        }
        vector<EventRaw> raw_events;
        vector<EventCal> cal_events;

        ProcessParams::DecodeBuffer(file_data, raw_events, info, &config);
        ProcessParams::ClearProcessedData(file_data, info);
        ProcessParams::IDBuffer(raw_events, cal_events, info, &config);

        for (size_t ii = 0; ii < cal_events.size(); ii++) {
            EventCal & event = cal_events[ii];

            TH1F * spat_hist = spat_hists[event.panel][event.cartridge]
                    [event.fin][event.module][event.apd][event.crystal];
            TH1F * comm_hist = comm_hists[event.panel][event.cartridge]
                    [event.fin][event.module][event.apd][event.crystal];

            spat_hist->Fill(event.spat_total);
            comm_hist->Fill(event.E);
        }
    }

    // TODO: add in photopeak fitting function

//    if (verbose) {
//        cout << "Finding photopeaks" << endl;
//    }
//    for (int p = 0; p < config.panels_per_system; p++) {
//        for (int c = 0; c < config.cartridges_per_panel; c++) {
//            for (int f = 0; f < config.fins_per_cartridge; f++) {
//                for (int m = 0; m < config.modules_per_fin; m++) {
//                    for (int a = 0; a < config.apds_per_module; a++) {
//                        for (int x = 0; x < config.crystals_per_apd; x++) {
//                            spat_photopeaks[p][c][f][m][a][x] = GetApdPhotopeak(
//                                        spat_hists[p][c][f][m][a][x],
//                                        700, 2800, 12);

//                            comm_photopeaks[p][c][f][m][a][x] = GetApdPhotopeak(
//                                        comm_hists[p][c][f][m][a][x],
//                                        350, 1200, 12);
//                        }
//                    }
//                }
//            }
//        }
//    }


    // TODO: write function to write calibration out


    if (verbose) {
        cout << info.getDecodeInfo() << endl;
        cout << info.getCalibrateInfo() << endl;
    }
    return(0);
}
