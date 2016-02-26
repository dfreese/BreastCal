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
#include <TH2F.h>
#include <TSpectrum.h>
#include <TFile.h>

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

int loadPhotopeaks(
        const string & filename,
        vector<float> & spat_photopeaks,
        vector<float> & comm_photopeaks,
        size_t expected_lines)
{
    vector<float> spat_photopeaks_loaded(expected_lines, 0);
    vector<float> comm_photopeaks_loaded(expected_lines, 0);

    ifstream input(filename.c_str());
    if (!input.good()) {
        return(-1);
    }

    size_t lines_read = 0;
    string line;
    while (getline(input, line)) {
        if (lines_read >= expected_lines) {
            return(-4);
        }
        stringstream ss;
        ss << line;
        string spat_pp_str;
        string comm_pp_str;
        ss >> spat_pp_str;
        ss >> comm_pp_str;
        if (Util::StringToNumber(
                    spat_pp_str, spat_photopeaks_loaded[lines_read]) < 0)
        {
            return(-3);
        }
        if (Util::StringToNumber(
                    comm_pp_str, comm_photopeaks_loaded[lines_read]) < 0)
        {
            return(-3);
        }
        lines_read++;
    }

    spat_photopeaks = spat_photopeaks_loaded;
    comm_photopeaks = comm_photopeaks_loaded;
    return(0);
}



void usage() {
    cout << "fill_floods [-vh] -c [config] -p [ped file] -pp [pp file] -f [filename] -f ...\n"
         << "  -o [name]    : flood output filename\n"
         << "  -l [name]    : list file of input filenames\n"
         << "  -el [energy] : low energy limit  (default: 357.7keV)\n"
         << "  -eh [energy] : high energy limit (default: 664.3keV)\n"
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
    string filename_pp;

    // Flood photopeak windows taken from get_floods.C (pp * 0.7 or 1.3)
    float energy_high = 664.3;
    float energy_low  = 357.7;

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
        if (argument == "-l") {
            if (Util::loadFilelistFile(following_argument, filenames) < 0) {
                cerr << "Unable to load filelist: "
                     << following_argument << endl;
                return(-2);
            }
        }
        if (argument == "-c") {
            filename_config = following_argument;
        }
        if (argument == "-p") {
            filename_ped = following_argument;
        }
        if (argument == "-pp") {
            filename_pp = following_argument;
        }
        if (argument == "-o") {
            filename_output = following_argument;
        }
        if (argument == "-eh") {
            if (Util::StringToNumber(following_argument, energy_high) < 0) {
                cerr << "Invalid energy_high value" << endl;
                return(-5);
            }
        }
        if (argument == "-el") {
            if (Util::StringToNumber(following_argument, energy_low) < 0) {
                cerr << "Invalid energy_low value" << endl;
                return(-6);
            }
        }
    }

    if (filenames.empty()) {
        cerr << "No filenames specified" << endl;
        return(-4);
    }

    if (filename_output == "") {
        if (filenames.size() == 1) {
            filename_output = filenames[0] + ".floods.root";
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
        cout << "filename_pp:     " << filename_pp << endl;
        cout << "filename_output: " << filename_output << endl;
        cout << "filename_config: " << filename_config << endl;
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


    if (verbose) {
        cout << "loading photopeaks: " << filename_pp << endl;
    }
    vector<vector<vector<vector<vector<float> > > > > spat_photopeaks;
    vector<vector<vector<vector<vector<float> > > > > comm_photopeaks;
    config.resizeArrayPCFMA<float>(spat_photopeaks, 0);
    config.resizeArrayPCFMA<float>(comm_photopeaks, 0);

    vector<float> spat_pp_load;
    vector<float> comm_pp_load;

    int pp_load_status = loadPhotopeaks(
                filename_pp,
                spat_pp_load, comm_pp_load,
                config.apds_per_system);

    if (pp_load_status < 0) {
        cerr << "loadPhotopeaks() failed with status: "
             << pp_load_status
             << endl;
        return(-4);
    }

    // These store upper and lower bounds on energy for if an event is added to
    // the flood histogram
    vector<vector<vector<vector<vector<float> > > > > egate_los;
    vector<vector<vector<vector<vector<float> > > > > egate_his;
    config.resizeArrayPCFMA<float>(egate_los, 0);
    config.resizeArrayPCFMA<float>(egate_his, 0);


    int photopeak_count = 0;
    vector<vector<vector<vector<vector<TH2F*> > > > > floods;
    config.resizeArrayPCFMA<TH2F*>(floods, 0);

    for (int p = 0; p < config.panels_per_system; p++) {
        for (int c = 0; c < config.cartridges_per_panel; c++) {
            for (int f = 0; f < config.fins_per_cartridge; f++) {
                for (int m = 0; m < config.modules_per_fin; m++) {
                    for (int a = 0; a < config.apds_per_module; a++) {
                        // Copy the loaded photopeaks into their vectors
                        spat_photopeaks[p][c][f][m][a] =
                                spat_pp_load[photopeak_count];
                        comm_photopeaks[p][c][f][m][a] =
                                comm_pp_load[photopeak_count];
                        photopeak_count++;

                        // Calculate the energy gates
                        egate_los[p][c][f][m][a] = (energy_low / 511.0) *
                                spat_photopeaks[p][c][f][m][a];
                        egate_his[p][c][f][m][a] = (energy_high / 511.0) *
                                spat_photopeaks[p][c][f][m][a];


                        // Create all of the 2D flood histograms with the
                        // appropriate names.
                        char namestring[30];
                        char titlestring[50];
                        sprintf(namestring,
                                "floods[%d][%d][%d][%d][%d]",
                                p, c, f, m, a);
                        sprintf(titlestring,
                                "P%dC%dF%dM%dA%d Flood Histogram",
                                p, c, f, m, a);
                        floods[p][c][f][m][a] = new TH2F(
                                    namestring, titlestring,
                                    256, -1, 1,
                                    256, -1, 1);
                    }
                }
            }
        }
    }

    long events_filled = 0;
    long events_egated = 0;
    if (verbose) {
        cout << "Filling Flood Histograms" << endl;
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

            TH2F * flood =
                    floods[event.panel][event.cartridge][fin][module][apd];

            float egate_lo =
                    egate_los[event.panel][event.cartridge][fin][module][apd];
            float egate_hi =
                    egate_his[event.panel][event.cartridge][fin][module][apd];

            if ((energy < egate_hi) && (energy > egate_lo)) {
                flood->Fill(x, y);
                events_filled++;
            } else {
                events_egated++;
            }
        }
    }

    if (verbose) {
        cout << info.getDecodeInfo();
    }

    if (verbose) {
        cout << "Events Used in Floods  : " << events_filled << endl;
        cout << "Events Rejected (egate): " << events_egated << endl;
    }

    if (verbose) {
        cout << "Writing out floods: " << filename_output << endl;
    }

    TFile * output_file = new TFile(filename_output.c_str(), "RECREATE");
    output_file->cd();
    for (int p = 0; p < config.panels_per_system; p++) {
        for (int c = 0; c < config.cartridges_per_panel; c++) {
            for (int f = 0; f < config.fins_per_cartridge; f++) {
                for (int m = 0; m < config.modules_per_fin; m++) {
                    for (int a = 0; a < config.apds_per_module; a++) {
                        TH2F * flood = floods[p][c][f][m][a];
                        flood->Write();
                    }
                }
            }
        }
    }
    output_file->Close();
    return(0);
}
