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

using namespace std;

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
    cout << "fill_energy [-vh] -c [config] -p [ped file] -f [filename] -f ...\n"
         << "  -o [name]  : output filename\n"
         << "  -po [name] : photopeak output filename\n"
         << endl;
}

int main(int argc, char ** argv) {
    if (argc < 1) {
        usage();
        return(0);
    }

    bool verbose = false;
    vector<string> filenames;
    string filename_config;
    string filename_output;
    string filename_ped;
    string filename_photopeak_output;


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
        if (argument == "-po") {
            filename_photopeak_output = following_argument;
        }
    }

    if (filenames.empty()) {
        cerr << "No filenames specified" << endl;
        return(-4);
    }

    if (filename_output == "") {
        if (filenames.size() == 1) {
            filename_output = filenames[0] + ".root";
        } else {
            cerr << "Output file not specified" << endl;
            return(-3);
        }
    }

    if (filename_photopeak_output == "") {
         filename_photopeak_output = filename_output + ".pp";
    }

    if (verbose) {
        cout << "filenames:       " << Util::vec2String(filenames) << endl;
        cout << "filename_ped:    " << filename_ped << endl;
        cout << "filename_output: " << filename_output << endl;
        cout << "filename_config: " << filename_config << endl;
        cout << "filename_photopeak_output: " << filename_photopeak_output << endl;
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
        return(-2);
    }

    
    vector<vector<vector<vector<vector<vector<TH1F*> > > > > > spat_hists;
    vector<vector<vector<vector<vector<vector<TH1F*> > > > > > comm_hists;
    config.resizeArrayPCFMA(spat_hist, 0);
    config.resizeArrayPCFMA(comm_hist, 0);
    for (int p = 0; p < config.panels_per_system; p++) {
        for (int c = 0; c < config.cartridges_per_panel; c++) {
            for (int f = 0; f < config.fins_per_cartridge; f++) {
                for (int m = 0; m < config.modules_per_fin; m++) {
                    for (int a = 0; a < config.apds_per_module; a++) {
                        char tmpstring[30];
                        char titlestring[50];
                        sprintf(tmpstring, "E[%d][%d][%d][%d][%d]", p, c, f, m, a);
                        sprintf(titlestring,"E P%dC%dF%dM%dA%d ", p, c, f, m, a);
                        spat_hists[p][c][f][i][j] = new TH1F(tmpstring, titlestring, Ebins, E_low, E_up);
                        sprintf(tmpstring, "E_com[%d][%d][%d][%d][%d]", p, c, f, m, a);
                        sprintf(titlestring,"ECOM P%dC%dF%dM%dA%d ", p, c, f, m, a);
                        comm_hists[p][c][f][i][j] = new TH1F(tmpstring, titlestring, Ebins_com, E_low_com, E_up_com);
                    }
                }
            }
        }
    }


    deque<char> file_data;
    ProcessInfo info;
    for (size_t ii = 0; ii < filenames.size(); ii++) {
        string & filename = filenames[ii];
        int read_status = readFileIntoDeque(filename, file_data);
        if (verbose) {
            cout << "read_status: " << read_status << endl;
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
            int apd;
            float x;
            float y;
            float energy;
            CalculateXYandEnergy(event, &config, x, y, energy, apd);
        }
    }


    for (int p = 0; p < config.panels_per_system; p++) {
        for (int c = 0; c < config.cartridges_per_panel; c++) {
            for (int f = 0; f < config.fins_per_cartridge; f++) {
                for (int m = 0; m < config.modules_per_fin; m++) {
                    for (int a = 0; a < config.apds_per_module; a++) {
                    }
                }
            }
        }
    }

    if (verbose) {
        cout << info << endl;
    }
    return(0);
}
