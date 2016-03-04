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
#include <miil/sorting.h>
#include <miil/EventCoinc.h>
#include <miil/util.h>
#include <miil/file_utils.h>
#include <TH1F.h>
#include <TF1.h>
#include <TSpectrum.h>
#include <TError.h>
#include <TFile.h>

using namespace std;

void usage() {
    cout << "sort_coinc [-vh] -c [config] -p [ped file] -cal [cal file] -uv [uv file] -fl [filename] -fr ...\n"
         << "  -o [name] : coincidence output filename\n"
         << "  -ll [name]: list file of left input filenames\n"
         << "  -lr [name]: list file of right input filenames\n"
         << "  -ro [name]: optional root output file\n"
         << "  -t [name] : optional time calibration file\n"
         << "  -te [name]: optional time calibration with edep file\n"
         << "  -w [ns]   : set time window (default: 25ns)\n"
         << "  -d [ns]   : set delay window(s) length (default: 0ns)\n"
         << "  -eg [keV] : enable energy gate (default: disabled)\n"
         << "  -eh [keV] : high energy gate (default: 700keV)\n"
         << "  -el [keV] : low energy gate  (default: 400keV)\n"
         << "  -ac       : assume calibrated files\n"
         << endl;
}

int main(int argc, char ** argv) {
    if (argc == 1) {
        usage();
        return(0);
    }

    bool verbose = false;
    vector<string> filenames_left;
    vector<string> filenames_right;
    string filename_config;
    string filename_output;
    string filename_ped;
    string filename_cal;
    string filename_uv;
    string filename_tcal;
    string filename_tcal_edep;
    string filename_root_output;

    bool energy_gate_flag = false;
    float energy_gate_low  = 400;
    float energy_gate_high = 700;

    bool assume_calibrated_flag = false;

    float time_window = 25;
    std::vector<float> delayed_windows;

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
        if (argument == "-ac") {
            assume_calibrated_flag = true;
        }
    }
    // Arguments requiring input
    for (int ix = 1; ix < (argc - 1); ix++) {
        string argument(argv[ix]);
        string following_argument(argv[ix + 1]);
        if (argument == "-fl") {
            filenames_left.push_back(following_argument);
        }
        if (argument == "-fr") {
            filenames_right.push_back(following_argument);
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
        if (strcmp(argv[ix], "-d") == 0) {
            float window_val = 0;
            if (Util::StringToNumber(following_argument, window_val) < 0) {
                cerr << "Invalid delayed window value: "
                     << following_argument << endl;
                return(-2);
            }
            delayed_windows.push_back(window_val);
        }
        if (argument == "-t") {
            filename_tcal = following_argument;
        }
        if (argument == "-te") {
            filename_tcal_edep = following_argument;
        }
        if (argument == "-ro") {
            filename_root_output = following_argument;
        }
        if (argument == "-ll") {
            if (Util::loadFilelistFile(following_argument, filenames_left) < 0)
            {
                cerr << "Unable to load left filelist: "
                     << following_argument << endl;
                return(-2);
            }
        }
        if (argument == "-lr") {
            if (Util::loadFilelistFile(following_argument, filenames_right) < 0)
            {
                cerr << "Unable to load right filelist: "
                     << following_argument << endl;
                return(-2);
            }
        }
        if (argument == "-w") {
            if (Util::StringToNumber(following_argument, time_window) < 0)
            {
                cerr << "Invalid time_window: "
                     << following_argument << endl;
                return(-3);
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

    if (filenames_left.empty()) {
        cerr << "No left filenames specified" << endl;
        return(-4);
    }

    if (filenames_right.empty()) {
        cerr << "No right filenames specified" << endl;
        return(-4);
    }

    if (filename_output == "") {
        cerr << "No output filename specified" << endl;
        return(-5);
    }

    if (delayed_windows.empty()) {
        delayed_windows.push_back(0);
    }

    if (verbose) {
        cout << "filenames (left):   " << Util::vec2String(filenames_left) << endl;
        cout << "filenames (right):  " << Util::vec2String(filenames_right) << endl;
        cout << "filename_config:    " << filename_config << endl;
        cout << "filename_ped:       " << filename_ped << endl;
        cout << "filename_cal:       " << filename_cal << endl;
        cout << "filename_output:    " << filename_output << endl;
        cout << "filename_tcal:      " << filename_tcal << endl;
        cout << "filename_tcal_edep: " << filename_tcal_edep << endl;
        cout << "filename_root_output: " << filename_root_output << endl;
        cout << "time_window: " << time_window << endl;
        cout << "delayed_windows: " << Util::vec2String(delayed_windows) << endl;
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
             << ped_load_status
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

    if (filename_tcal != "") {
        if (verbose) {
            cout << "Loading Time Calibration: " << filename_tcal << endl;
        }
        int tcal_load_status = config.loadTimeCalibration(filename_tcal);
        if (tcal_load_status < 0) {
            cerr << "SystemConfiguration.loadTimeCalibration()"
                 << " failed with status: "
                 << tcal_load_status
                 << endl;
            return(-5);
        }
    }

    if (filename_tcal_edep != "") {
        if (verbose) {
            cout << "Loading Energy Dependent Time Calibration: "
                 << filename_tcal_edep << endl;
        }
        int edep_load_status = config.loadTimeCalWithEdep(filename_tcal_edep);
        if (edep_load_status < 0) {
            cerr << "SystemConfiguration.loadTimeCalWithEdep()"
                 << " failed with status: "
                 << edep_load_status
                 << endl;
            return(-6);
        }
    }


    if (verbose) {
        cout << "Calibrating Data" << endl;
    }
    ProcessInfo info;
    vector<EventCal> cal_events_left;
    vector<EventCal> cal_events_right;

    for (size_t ii = 0; ii < filenames_left.size(); ii++) {
        string & filename = filenames_left[ii];
        if (assume_calibrated_flag) {
            int read_status = Util::readFileIntoVector(
                        filename, cal_events_left);
            if (verbose) {
                cout << filename << " read with status: "
                     << read_status << endl;
            }
            if (read_status < 0) {
                cerr << "Unable to load: " << filename << endl;
                return(-3);
            }
        } else {
            deque<char> file_data;
            int read_status = Util::readFileIntoDeque(filename, file_data);
            if (verbose) {
                cout << filename << " read with status: "
                     << read_status << endl;
            }
            if (read_status < 0) {
                cerr << "Unable to load: " << filename << endl;
                return(-3);
            }
            vector<EventRaw> raw_events;
            ProcessParams::DecodeBuffer(file_data, raw_events, info, &config);
            ProcessParams::ClearProcessedData(file_data, info);
            ProcessParams::CalibrateBuffer(
                        raw_events, cal_events_left, info, &config);
        }
    }

    for (size_t ii = 0; ii < filenames_right.size(); ii++) {
        string & filename = filenames_right[ii];
        if (assume_calibrated_flag) {
            int read_status = Util::readFileIntoVector(
                        filename, cal_events_right);
            if (verbose) {
                cout << filename << " read with status: "
                     << read_status << endl;
            }
            if (read_status < 0) {
                cerr << "Unable to load: " << filename << endl;
                return(-3);
            }
        } else {
            deque<char> file_data;
            int read_status = Util::readFileIntoDeque(filename, file_data);
            if (verbose) {
                cout << filename << " read with status: "
                     << read_status << endl;
            }
            if (read_status < 0) {
                cerr << "Unable to load: " << filename << endl;
                return(-3);
            }
            vector<EventRaw> raw_events;
            ProcessParams::DecodeBuffer(file_data, raw_events, info, &config);
            ProcessParams::ClearProcessedData(file_data, info);
            ProcessParams::CalibrateBuffer(
                        raw_events, cal_events_right, info, &config);
        }
    }


    if (energy_gate_flag) {
        if (verbose) {
            cout << "Energy Gating from " << energy_gate_low << "keV to "
                 << energy_gate_high << "keV" << endl;
        }
        long events_dropped_egate = 0;
        long events_accepted_egate = 0;
        // Go through and overwrite dropped events
        for (size_t ii = 0; ii < cal_events_left.size(); ii++) {
            const EventCal & event = cal_events_left[ii];
            if (event.E < energy_gate_high && event.E > energy_gate_low) {
                cal_events_left[events_accepted_egate++] = event;
            } else {
                events_dropped_egate++;
            }
        }
        // Now that all of the accepted events have been packed at the start of
        // the vector, chop off the end
        cal_events_left.resize(events_accepted_egate);
        info.dropped_energy_gate = events_dropped_egate;
        info.accepted_calibrate -= events_dropped_egate;

        // Now do the right side
        events_dropped_egate = 0;
        events_accepted_egate = 0;
        // Go through and overwrite dropped events
        for (size_t ii = 0; ii < cal_events_right.size(); ii++) {
            const EventCal & event = cal_events_right[ii];
            if (event.E < energy_gate_high && event.E > energy_gate_low) {
                cal_events_right[events_accepted_egate++] = event;
            } else {
                events_dropped_egate++;
            }
        }
        // Now that all of the accepted events have been packed at the start of
        // the vector, chop off the end
        cal_events_right.resize(events_accepted_egate);
        info.dropped_energy_gate += events_dropped_egate;
        info.accepted_calibrate -= events_dropped_egate;
    }


    if (verbose) {
        cout << "Sorting Events" << endl;
    }

    insertion_sort(cal_events_left, EventCalLessThan,
                   (float) config.uv_period_ns, (float) config.ct_period_ns);
    insertion_sort(cal_events_right, EventCalLessThan,
                   (float) config.uv_period_ns, (float) config.ct_period_ns);

    std::vector<EventCoinc> output_events;
    output_events.reserve((int) (0.5 * 0.5 * (cal_events_left.size())));


    cout << "Time Window: " << time_window << "ns\n";

    long dropped_single(0);
    long dropped_multiple(0);

    if (verbose) {
        cout << "Sorting Coincidences" << endl;
    }

    for (size_t delay_idx = 0; delay_idx < delayed_windows.size(); delay_idx++)
    {
        float & delayed_window = delayed_windows[delay_idx];

        if (verbose) {
            if (delayed_window) {
                cout << "Running with delayed window (ns): "
                     << delayed_window << endl;
            }
        }

        std::vector<EventCal>::iterator iterator_left =
                cal_events_left.begin();
        std::vector<EventCal>::iterator iterator_right =
                cal_events_right.begin();

        while(iterator_left != cal_events_left.end() &&
              iterator_right != cal_events_right.end())
        {
            // 0. If one list is at the end, drop the rest as singles.
            if (iterator_left == cal_events_left.end() ||
                    iterator_right == cal_events_right.end())
            {
                // Drop the rest as singles.
                dropped_single += cal_events_left.end() - iterator_left
                                + cal_events_right.end() - iterator_right;
                break;
            }

            std::vector<EventCal>::iterator current_event;
            std::vector<EventCal>::iterator pair_event;
            bool current_on_left;
            float left_window_end;
            float right_window_end;
            // 1. Check to see if the left or right side is first
            if (EventCalTimeDiff(*iterator_left, *iterator_right,
                                 config.uv_period_ns, config.ct_period_ns) <
                    (-delayed_window))
            {
                current_event = iterator_left;
                pair_event = iterator_right;
                current_on_left = true;
                left_window_end = time_window;
                right_window_end = time_window + delayed_window;
            } else {
                current_event = iterator_right;
                pair_event = iterator_left;
                current_on_left = false;
                left_window_end = time_window - delayed_window;
                right_window_end = time_window;
            }

            // 2. Find all other events within time window
            int events_in_time_window_left(0);
            while (EventCalTimeDiff(*iterator_left, *current_event,
                                    config.uv_period_ns, config.ct_period_ns) <
                   left_window_end)
            {
                events_in_time_window_left++;
                iterator_left++;
                if (iterator_left == cal_events_left.end()) {
                    break;
                }
            }
            int events_in_time_window_right(0);
            while (EventCalTimeDiff(*iterator_right, *current_event,
                                    config.uv_period_ns, config.ct_period_ns) <
                   right_window_end)
            {
                events_in_time_window_right++;
                iterator_right++;
                if (iterator_right == cal_events_right.end()) {
                    break;
                }
            }

            // 3. Pair events if there is only one event on other side
            if ((events_in_time_window_left == 1) &&
                (events_in_time_window_right == 1))
            {
                EventCoinc event;
                if (current_on_left) {
                    event = MakeCoinc(*current_event, *pair_event,
                                      config.uv_period_ns, config.ct_period_ns);
                } else {
                    event = MakeCoinc(*pair_event, *current_event,
                                      config.uv_period_ns, config.ct_period_ns);
                }
                // Since the window on the right side is ahead of the left, and
                // dtf is calculated from ModuleCalTimeDiff(left, right), we add
                // back in the delayed window to recenter the window around
                // zero.  For non-randoms, delayed_window defaults to 0, so this
                // has no effect.
                event.dtf += delayed_window;
                output_events.push_back(event);
            } else if ((events_in_time_window_left == 0) ||
                       (events_in_time_window_right == 0))
            {
                dropped_single += events_in_time_window_left +
                                  events_in_time_window_right;
            } else {
                // 4. If there is more than one event, drop all events, go to 1.
                dropped_multiple += events_in_time_window_left +
                                    events_in_time_window_right;
            }
        }
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
                (char*) output_events.data(),
                sizeof(EventCoinc) * output_events.size()))
    {
        cerr << "Failed to write to output: " << filename_output << endl;
        return(-5);
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


    if (verbose && !assume_calibrated_flag) {
        cout << info.getDecodeInfo() << endl;
        cout << info.getCalibrateInfo() << endl;
    }
    if (verbose) {
        cout << "Events Dropped:\n"
             << "  Single - " << dropped_single << "\n"
             << "  Multiple - " << dropped_multiple << "\n"
             << "Coincidences: " << output_events.size() << "\n";

    }

    return(0);
}
