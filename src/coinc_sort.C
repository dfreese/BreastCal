#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include "Riostream.h"
#include "TMath.h"
#include "TVector.h"
#include "Util.h"
#include "ModuleDat.h"
#include "EventCal.h"
#include "daqboardmap.h"
#include "daqpacket.h"
#include "decoderlib.h"
#include "cal_helper_functions.h"
#include "chipevent.h"
#include "sorting.h"
#include "libInit_avdb.h"
#include "myrootlib.h"
#include "Syspardef.h"
#include "EventCoinc.h"

using namespace std;

void usage(void)
{
    cout << "coinc_sort (-v) -l [left filename] -r [right filename] -o [output]\n"
         << " -t [time window (ns)]: time window in ns, default = 20\n"
         << " -el [low energy window]: default 400keV\n"
         << " -eh [low energy window]: default 600keV\n"
         << endl;
    return;
}

EventCoinc MakeCoinc(
        const EventCal & event_left,
        const EventCal & event_right)
{
    EventCoinc event;
    event.dtc = event_left.ct - event_right.ct;
    event.dtf = event_left.ft - event_right.ft;
    if (event.dtf < UV_PERIOD_NS / -2.0) {
        event.dtf += UV_PERIOD_NS;
    }
    if (event.dtf > UV_PERIOD_NS / 2.0) {
        event.dtf -= UV_PERIOD_NS;
    }
    event.E1 = event_left.E;
    event.E2 = event_right.E;
    event.crystal1 = event_left.crystal;
    event.crystal2 = event_right.crystal;
    return(event);
}

int main(int argc, char *argv[])
{
    if (argc == 1) {
        usage();
        return(0);
    }

    bool verbose(false);
    string filename_left;
    string filename_right;
    string filename_output;
    float time_window(20);

    float energy_gate_high(600);
    float energy_gate_low(400);

    // Arguments not requiring input
    for (int ix = 1; ix < argc; ix++) {
        if (strcmp(argv[ix], "-v") == 0) {
            verbose = true;
        }
    }
    // Arguments requiring input
    for (int ix = 1; ix < (argc - 1); ix++) {
        if (strcmp(argv[ix], "-t") == 0) {
            time_window = atoi(argv[ix + 1]);
        }
        if (strcmp(argv[ix], "-l") == 0) {
            filename_left = string(argv[ix + 1]);
        }
        if (strcmp(argv[ix], "-r") == 0) {
            filename_right = string(argv[ix + 1]);
        }
        if (strcmp(argv[ix], "-o") == 0) {
            filename_output = string(argv[ix + 1]);
        }
        if(strcmp(argv[ix], "-eh") == 0) {
            stringstream ss;
            ss << argv[ix + 1];
            ss >> energy_gate_high;
        }
        if(strcmp(argv[ix], "-el") == 0) {
            stringstream ss;
            ss << argv[ix + 1];
            ss >> energy_gate_low;
        }
    }

    if (filename_left == "") {
        printf("Please Specify Left Filename !!\n");
        usage();
        return(-1);
    }
    if (filename_right == "") {
        printf("Please Specify Right Filename !!\n");
        usage();
        return(-1);
    }
    if (filename_output == "") {
        printf("Please Specify Output Filename !!\n");
        usage();
        return(-1);
    }

    if (verbose) {
        cout << "Reading Left File" << endl;
    }

    ifstream dataFile;
    dataFile.open(filename_left.c_str(), ios::in | ios::binary);
    if (!dataFile.good()) {
        cerr << "Cannot open file \"" << filename_left
             << "\" for read operation." << endl;
        cerr << "Exiting." << endl;
        return(-1);
    }

    // Read the full input data file into memory
    dataFile.seekg(0, std::ios::end);
    std::streamsize dataFileSize = dataFile.tellg();
    dataFile.seekg(0, std::ios::beg);

    std::vector<EventCal> events_left(dataFileSize / sizeof(EventCal));
    if (!dataFile.read((char*) events_left.data(), dataFileSize)) {
        cerr << "Left File read failed" << endl;
        return(-2);
    }
    dataFile.close();


    if (verbose) {
        cout << "Reading Right File" << endl;
    }

    dataFile.open(filename_right.c_str(), ios::in | ios::binary);
    if (!dataFile.good()) {
        cerr << "Cannot open file \"" << filename_right
             << "\" for read operation." << endl;
        cerr << "Exiting." << endl;
        return(-1);
    }

    // Read the full input data file into memory
    dataFile.seekg(0, std::ios::end);
    dataFileSize = dataFile.tellg();
    dataFile.seekg(0, std::ios::beg);

    std::vector<EventCal> events_right(dataFileSize / sizeof(EventCal));
    if (!dataFile.read((char*) events_right.data(), dataFileSize)) {
        cerr << "Right File read failed" << endl;
        return(-3);
    }
    dataFile.close();


    std::vector<EventCal>::iterator iterator_left(events_left.begin());
    std::vector<EventCal>::iterator iterator_right(events_right.begin());
    std::vector<EventCal>::iterator current_event;
    std::vector<EventCal>::iterator pair_event;
    bool current_on_left;


    std::vector<EventCoinc> output_events;
    output_events.reserve((int) (0.5 * 0.5 * (events_left.size() +
                                              events_right.size())));


    cout << "Time Window: " << time_window << "ns\n"
         << "Energy Window: " << energy_gate_low << "keV to "
         << energy_gate_high << "keV\n";


    if (verbose) {
        cout << "Sorting Coincidences" << endl;
    }

    while(iterator_left != events_left.end() &&
          iterator_right != events_right.end())
    {
        // 1. Find first event within energy window
        // Initialize each iterator to an event within the energy window first
        while (iterator_left != events_left.end()) {
            if (!InEnergyWindow(*iterator_left,
                                energy_gate_low,
                                energy_gate_high))
            {
                iterator_left++;
            } else {
                break;
            }
        }
        while (iterator_right != events_right.end()) {
            if (!InEnergyWindow(*iterator_right,
                                energy_gate_low,
                                energy_gate_high))
            {
                iterator_right++;
            } else {
                break;
            }
        }
        if (iterator_left == events_left.end() ||
                iterator_right == events_right.end())
        {
            break;
        }

        // Check to see if the left or right side is first
        if (EventCalLessThan(*iterator_left, *iterator_right)) {
            current_event = iterator_left;
            pair_event = iterator_right;
            current_on_left = true;
        } else {
            current_event = iterator_right;
            pair_event = iterator_left;
            current_on_left = false;
        }

        // 2. Find all other events within time window
        int events_in_time_window_left(0);
        while(EventCalTimeDiff(*iterator_left, *current_event) < time_window) {
            if (InEnergyWindow(*(iterator_left++),
                               energy_gate_low,
                               energy_gate_high))
            {
                events_in_time_window_left++;
            }
            if (iterator_left == events_left.end()) {
                break;
            }
        }
        int events_in_time_window_right(0);
        while(EventCalTimeDiff(*iterator_right, *current_event) < time_window) {
            if (InEnergyWindow(*(iterator_right++),
                               energy_gate_low,
                               energy_gate_high))
            {
                events_in_time_window_right++;
            }
            if (iterator_right == events_right.end()) {
                break;
            }
        }

        // 3. Pair events if there is only one event within energy window on other side, pair
        if ((events_in_time_window_left == 1) &&
            (events_in_time_window_right == 1))
        {
            EventCoinc event;
            if (current_on_left) {
                event = MakeCoinc(*current_event, *pair_event);
            } else {
                event = MakeCoinc(*pair_event, *current_event);
            }
            output_events.push_back(event);
        }
        // 4. If there is more than one event, drop all events, go to 1.
    }

    if (verbose) {
        cout << "Writing Out Coincidences" << endl;
    }

    std::ofstream output_stream;
    output_stream.open(filename_output.c_str(), std::ios::binary);

    output_stream.write((char *)&output_events[0],
                        sizeof(EventCoinc) * output_events.size());
    output_stream.close();

    cout << "Events Processed - Left: " << events_left.size()
         << "  Right: " << events_right.size() << "\n"
         << "Coincidences: " << output_events.size() << "\n";

    return(0);
}
