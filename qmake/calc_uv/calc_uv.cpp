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

}

int main(int argc, char ** argv) {
    if (argc < 1) {
        usage();
        return(0);
    }

    int uv_threshold = -1000;

    bool verbose = false;
    string filename;
    string filename_config;
    string filename_output;
    string filename_ped;


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
        if (argument == "-t") {
            if (Util::StringToNumber(following_argument, uv_threshold) < 0) {
                cerr << "invalid threshold value of: " << endl;
                return(-1);
            }
        }
        if (argument == "-f") {
            filename = following_argument;
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
    }

    if (verbose) {
        cout << "filename:        " << filename << endl;
        cout << "filename_ped:    " << filename_ped << endl;
        cout << "filename_output: " << filename_output << endl;
        cout << "filename_config: " << filename_config << endl;
        cout << "uv_threshold: " << uv_threshold << endl;
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

    vector<vector<vector<vector<vector<vector<float> > > > > > u_centers;
    vector<vector<vector<vector<vector<vector<float> > > > > > v_centers;
    vector<vector<vector<vector<vector<vector<int> > > > > > uv_entries;

    config.resizeArrayPCDRMA<float>(u_centers, 0);
    config.resizeArrayPCDRMA<float>(v_centers, 0);
    config.resizeArrayPCDRMA<int>(uv_entries, 0);

    deque<char> file_data;
    int read_status = readFileIntoDeque(filename, file_data);
    if (verbose) {
        cout << "read_status: " << read_status << endl;
    }
    if (read_status < 0) {
        cerr << "Unable to load: " << filename << endl;
        return(-3);
    }
    vector<EventRaw> raw_events;
    ProcessInfo info;

    ProcessParams::DecodeBuffer(file_data, raw_events, info, &config);
    ProcessParams::ClearProcessedData(file_data, info);
    for (size_t ii = 0; ii < raw_events.size(); ii++) {
        EventRaw & event = raw_events[ii];
        int apd = 0;
        float u = event.u0h;
        float v = event.v0h;
        if (event.com1h > event.com0h) {
            apd = 1;
            u = event.u1h;
            v = event.v1h;
        }

        float & u_center = u_centers[event.panel][event.cartridge]
                [event.daq][event.rena][event.module][apd];
        float & v_center = v_centers[event.panel][event.cartridge]
                [event.daq][event.rena][event.module][apd];
        int & entries = uv_entries[event.panel][event.cartridge]
                [event.daq][event.rena][event.module][apd];

        entries++;
        u_center += (u - u_center) / entries;
        v_center += (v - v_center) / entries;
    }

    for (int p = 0; p < config.panels_per_system; p++) {
        for (int c = 0; c < config.cartridges_per_panel; c++) {
            for (int d = 0; d < config.daqs_per_cartridge; d++) {
                for (int r = 0; r < config.renas_per_daq; r++) {
                    for (int m = 0; m < config.modules_per_rena; m++) {
                        ModulePedestals & pedestals = config.pedestals[p][c][d][r][m];
                        pedestals.u0h = u_centers[p][c][d][r][m][0];
                        pedestals.u1h = u_centers[p][c][d][r][m][1];
                        pedestals.v0h = v_centers[p][c][d][r][m][0];
                        pedestals.v1h = v_centers[p][c][d][r][m][1];
                    }
                }
            }
        }
    }

    config.writeUVCenters(filename_output);

    if (verbose) {
        cout << info << endl;
    }
    return(0);
}
