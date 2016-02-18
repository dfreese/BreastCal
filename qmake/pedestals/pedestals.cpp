#include <iostream>
#include <sstream>
#include <string>
#include <deque>
#include <vector>
#include <fstream>
#include <cmath>
#include <miil/SystemConfiguration.h>
#include <miil/process/processing.h>
#include <miil/process/ProcessParams.h>
#include <miil/process/ProcessInfo.h>
#include <miil/util.h>

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
    cout << "pedestals [-vh] -c [config] -f [filename] -f ...\n"
         << "  -o [name] : output filename\n"
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
        if (argument == "-o") {
            filename_output = following_argument;
        }
    }

    if (filenames.empty()) {
        cerr << "No filenames specified" << endl;
        return(-4);
    }

    if (filename_output == "") {
        if (filenames.size() == 1) {
            filename_output = filenames[0] + ".ped";
        } else {
            cerr << "Output file not specified" << endl;
            return(-3);
        }
    }

    if (verbose) {
        cout << "filenames:       " << Util::vec2String(filenames) << endl;
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

            ModulePedestals & pedestals = config.pedestals[event.panel]
                    [event.cartridge][event.daq][event.rena][event.module];

            const ModulePedestals ped_copy = pedestals;

            pedestals.events++;
            pedestals.a += (event.a - pedestals.a) / pedestals.events;
            pedestals.b += (event.b - pedestals.b) / pedestals.events;
            pedestals.c += (event.c - pedestals.c) / pedestals.events;
            pedestals.d += (event.d - pedestals.d) / pedestals.events;
            pedestals.com0 +=
                    (event.com0 - pedestals.com0) / pedestals.events;
            pedestals.com1 +=
                    (event.com1 - pedestals.com1) / pedestals.events;
            pedestals.com0h +=
                    (event.com0h - pedestals.com0h) / pedestals.events;
            pedestals.com1h +=
                    (event.com1h - pedestals.com1h) / pedestals.events;

            pedestals.a_std +=
                    (event.a - pedestals.a) * (event.a - ped_copy.a);
            pedestals.b_std +=
                    (event.b - pedestals.b) * (event.b - ped_copy.b);
            pedestals.c_std +=
                    (event.c - pedestals.c) * (event.c - ped_copy.c);
            pedestals.d_std +=
                    (event.d - pedestals.d) * (event.d - ped_copy.d);
            pedestals.com0_std += (event.com0 - pedestals.com0) *
                    (event.com0 - ped_copy.com0);
            pedestals.com1_std += (event.com1 - pedestals.com1) *
                    (event.com1 - ped_copy.com1);
            pedestals.com0h_std += (event.com0h - pedestals.com0h) *
                    (event.com0h - ped_copy.com0h);
            pedestals.com1h_std += (event.com1h - pedestals.com1h) *
                    (event.com1h - ped_copy.com1h);
        }
    }


    for (int p = 0; p < config.panels_per_system; p++) {
        for (int c = 0; c < config.cartridges_per_panel; c++) {
            for (int d = 0; d < config.daqs_per_cartridge; d++) {
                for (int r = 0; r < config.renas_per_daq; r++) {
                    for (int m = 0; m < config.modules_per_rena; m++) {
                        ModulePedestals & pedestals =
                                config.pedestals[p][c][d][r][m];
                        if (pedestals.events) {
                            pedestals.a_std = sqrt(pedestals.a_std /
                                                   (double) pedestals.events);
                            pedestals.b_std = sqrt(pedestals.b_std /
                                                   (double) pedestals.events);
                            pedestals.c_std = sqrt(pedestals.c_std /
                                                   (double) pedestals.events);
                            pedestals.d_std = sqrt(pedestals.d_std /
                                                   (double) pedestals.events);
                            pedestals.com0_std = sqrt(
                                    pedestals.com0_std /
                                    (double) pedestals.events);
                            pedestals.com1_std = sqrt(
                                    pedestals.com1_std /
                                    (double) pedestals.events);
                            pedestals.com0h_std = sqrt(
                                    pedestals.com0h_std /
                                    (double) pedestals.events);
                            pedestals.com1h_std = sqrt(
                                    pedestals.com1h_std /
                                    (double) pedestals.events);
                        }
                    }
                }
            }
        }
    }

    int write_status = config.writePedestals(filename_output);
    if (verbose) {
        cout << "write_status: " << write_status << endl;
    }
    if (write_status < 0) {
        cerr << "SystemConfiguration.writePedestals() failed with status: "
             << write_status
             << endl;
        return(-4);
    }

    if (verbose) {
        cout << info << endl;
    }
    return(0);
}
