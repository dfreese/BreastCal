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
#include <TF1.h>
#include <TSpectrum.h>
#include <TError.h>
#include <TFile.h>

using namespace std;

void usage() {
    cout << "calibrate [-vh] -c [config] -p [ped file] -cal [cal file] -uv [uv file] -f [filename] -f ...\n"
         << "  -o [name] : photopeak output filename\n"
         << "  -l [name] : list file of input filenames\n"
         << "  -ro [name]: optional root output file\n"
         << "  -eg [keV] : enable energy gate (default: disabled)\n"
         << "  -eh [keV] : high energy gate (default: 700keV)\n"
         << "  -el [keV] : low energy gate  (default: 400keV)\n"
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
    string filename_cal;
    string filename_uv;
    string filename_root_output;

    bool energy_gate_flag = false;
    float energy_gate_low  = 400;
    float energy_gate_high = 700;

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
        if (argument == "-eg") {
            energy_gate_flag = true;
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
        if (argument == "-cal") {
            filename_cal = following_argument;
        }
        if (argument == "-uv") {
            filename_uv = following_argument;
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
        if (argument == "-el") {
            if (Util::StringToNumber(following_argument, energy_gate_low) < 0)
            {
                cerr << "Invalid energy_gate_low: "
                     << following_argument << endl;
                return(-3);
            }
            energy_gate_flag = true;
        }
        if (argument == "-eh") {
            if (Util::StringToNumber(following_argument, energy_gate_high) < 0)
            {
                cerr << "Invalid energy_gate_high: "
                     << following_argument << endl;
                return(-3);
            }
            energy_gate_flag = true;
        }
    }

    if (filenames.empty()) {
        cerr << "No filenames specified" << endl;
        return(-4);
    }

    if (filename_output == "") {
        if (filenames.size() == 1) {
            filename_output = filenames[0] + ".cal";
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
        cout << "filename_cal:    " << filename_cal << endl;
        cout << "filename_output: " << filename_output << endl;
        cout << "filename_root_output: " << filename_root_output << endl;
        if (energy_gate_flag) {
            cout << "energy_gate_low:  " << energy_gate_low << endl;
            cout << "energy_gate_high: " << energy_gate_high << endl;
        }
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
        cout << "Loading Pedestals: " << filename_ped << endl;
    }
    int ped_load_status = config.loadPedestals(filename_ped);
    if (ped_load_status < 0) {
        cerr << "SystemConfiguration.loadPedestals() failed with status: "
             << ped_load_status
             << endl;
        return(-3);
    }


    if (verbose) {
        cout << "Loading Calibration: " << filename_cal << endl;
    }
    int cal_load_status = config.loadCalibration(filename_cal);
    if (cal_load_status < 0) {
        cerr << "SystemConfiguration.loadPedestals() failed with status: "
             << cal_load_status
             << endl;
        return(-4);
    }

    if (verbose) {
        cout << "Loading UV Centers: " << filename_uv << endl;
    }
    int uv_load_status = config.loadUVCenters(filename_uv);
    if (uv_load_status < 0) {
        cerr << "SystemConfiguration.loadUVCenters() failed with status: "
             << uv_load_status
             << endl;
        return(-5);
    }

    if (verbose) {
        cout << "Calibrating Data" << endl;
    }
    deque<char> file_data;
    ProcessInfo info;
    vector<EventCal> cal_events;
    for (size_t ii = 0; ii < filenames.size(); ii++) {
        string & filename = filenames[ii];
        int read_status = Util::readFileIntoDeque(filename, file_data);
        if (verbose) {
            cout << filename << " read with status: " << read_status << endl;
        }
        if (read_status < 0) {
            cerr << "Unable to load: " << filename << endl;
            return(-6);
        }
        vector<EventRaw> raw_events;

        ProcessParams::DecodeBuffer(file_data, raw_events, info, &config);
        ProcessParams::ClearProcessedData(file_data, info);
        ProcessParams::CalibrateBuffer(raw_events, cal_events, info, &config);
    }

    if (energy_gate_flag) {
        if (verbose) {
            cout << "Energy Gating from " << energy_gate_low << "keV to "
                 << energy_gate_high << "keV" << endl;
        }
        long events_dropped_egate = 0;
        long events_accepted_egate = 0;
        // Go through and overwrite dropped events
        for (size_t ii = 0; ii < cal_events.size(); ii++) {
            const EventCal & event = cal_events[ii];
            if (event.E < energy_gate_high && event.E > energy_gate_low) {
                cal_events[events_accepted_egate++] = event;
            } else {
                events_dropped_egate++;
            }
        }
        // Now that all of the accepted events have been packed at the start of
        // the vector, chop off the end
        cal_events.resize(events_accepted_egate);
        info.dropped_energy_gate = events_dropped_egate;
        info.accepted_calibrate -= events_dropped_egate;
    }

    if (verbose) {
        cout << "Writing Data: " << filename_output << endl;
    }

    ofstream output(filename_output.c_str(), ios::binary);
    if (!output.good()) {
        cerr << "Unable to open output: " << filename_output << endl;
        return(-4);

    }
    if (!output.write(
                (char*) cal_events.data(),
                sizeof(EventCal) * cal_events.size()))
    {
        cerr << "Failed to write to output: " << filename_output << endl;
        return(-7);
    }


    if (filename_root_output != "") {
        // TODO: write root output
        //if (verbose) {
        //    cout << "Writing histograms to " << filename_root_output << endl;
        //}
        //TFile * output_file =
        //        new TFile(filename_root_output.c_str(), "RECREATE");
        //if (output_file->IsZombie()) {
        //    cerr << "Unable to open root output file: "
        //         << filename_root_output << endl;
        //    return(-6);
        //}
        //output_file->cd();
        //output_file->Close();
    }


    if (verbose) {
        cout << info.getDecodeInfo() << endl;
        cout << info.getCalibrateInfo() << endl;
    }

    return(0);
}
