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

using namespace std;

void usage() {
    cout << "uv_calc [-vh] -c [config] -p [ped file] -f [filename] -f ...\n"
         << "  -o [name] : output filename\n"
         << "  -l [name] : list file of input filenames\n"
         << "  -ad       : assume the input files are decoded\n"
         << endl;
}

int main(int argc, char ** argv) {
    if (argc == 1) {
        usage();
        return(0);
    }

    int uv_threshold = -1000;

    bool verbose = false;
    vector<string> filenames;
    string filename_config;
    string filename_output;
    string filename_ped;
    bool assume_decoded = false;


    // Arguments not requiring input
    for (int ix = 1; ix < argc; ix++) {
        string argument(argv[ix]);
        if (argument == "-v") {
            verbose = true;
            cout << "Running in verbose mode " << endl;
        }
        if (argument == "-ad") {
            assume_decoded = true;
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
            filename_output = filenames[0] + ".uv";
        } else {
            cerr << "Output file not specified" << endl;
            return(-3);
        }
    }

    if (verbose) {
        cout << "filenames:       " << Util::vec2String(filenames) << endl;
        cout << "filename_ped:    " << filename_ped << endl;
        cout << "filename_output: " << filename_output << endl;
        cout << "filename_config: " << filename_config << endl;
        cout << "uv_threshold: " << uv_threshold << endl;
        if (assume_decoded) {
            cout << "assuming decoded files" << endl;
        }
    }

    SystemConfiguration config;
    int config_load_status = config.load(filename_config);
    if (verbose) {
        cout << "Loading Config: " << filename_config << endl;
    }
    if (config_load_status < 0) {
        cerr << "SystemConfiguration.load() failed with status: "
             << config_load_status
             << endl;
        return(-2);
    }

    int ped_load_status = config.loadPedestals(filename_ped);
    if (verbose) {
        cout << "Loading Pedestals: " << filename_ped << endl;
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
    ProcessInfo info;
    for (size_t ii = 0; ii < filenames.size(); ii++) {
        string & filename = filenames[ii];
        vector<EventRaw> raw_events;
        if (assume_decoded) {
            int read_status = Util::readFileIntoVector(filename, raw_events);
            if (verbose) {
                cout << filename << " read with status: " << read_status << endl;
            }
            if (read_status < 0) {
                cerr << "Unable to load: " << filename << endl;
                return(-3);
            }
        } else {
            int read_status = Util::readFileIntoDeque(filename, file_data);
            if (verbose) {
                cout << filename << " read with status: " << read_status << endl;
            }
            if (read_status < 0) {
                cerr << "Unable to load: " << filename << endl;
                return(-3);
            }

            ProcessParams::DecodeBuffer(file_data, raw_events, info, &config);
            ProcessParams::ClearProcessedData(file_data, info);
        }
        for (size_t ii = 0; ii < raw_events.size(); ii++) {
            EventRaw & event = raw_events[ii];
            EventCal cal_event;
            // Check for a double trigger based on configuration
            if (CalculateXYandEnergy(
                        cal_event, event, &config, true, true) < 0)
            {
                continue;
            }
            PedestalCorrectEventRaw(event, &config, false);
            int apd = 0;
            float u = event.u0h;
            float v = event.v0h;
            int comh = event.com0h;
            if (event.com1h < event.com0h) {
                apd = 1;
                u = event.u1h;
                v = event.v1h;
                comh = event.com1h;
            }

            if (comh < uv_threshold) {
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
        }
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

    int write_status = config.writeUVCenters(filename_output);
    if (verbose) {
        cout << "Writing Output: " << filename_output << endl;
    }
    if (write_status < 0) {
        cerr << "SystemConfiguration.writeUVCenters() failed with status: "
             << write_status
             << endl;
        return(-4);
    }

    if (verbose) {
        cout << info.getDecodeInfo() << endl;
    }
    return(0);
}
