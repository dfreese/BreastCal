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
#include "Util.h"
#include "ModuleCal.h"
#include "CoincEvent.h"
#include "cal_helper_functions.h"
#include "Syspardef.h"
#include <cmath>
#include <sorting.h>

#define UV_PERIOD_NS 1020.40816326

using namespace std;

void usage(void) {
    cout << "coinc_sort (-v) -l [left filename] -r [right filename] -o [output]\n"
         << " -t [time window (ns)]: time window in ns, default = 20\n"
         << " -el [low energy window]: default 400keV\n"
         << " -eh [low energy window]: default 700keV\n"
         << " -rcal [filename]: read time calibration file\n"
         << endl;
    return;
}

/*!
 * \brief Calculate the time between two calibrated events
 *
 * Calculate the time difference between two calibrated events.  If ct_only is
 * true, the returned value is simply the difference of the coarse timestamps
 * scaled by the ct_period_ns.  Otherwise, the functions considers both
 * timestamps.  First, if abs(ct1-ct2) < compare_on_ft_ct_window is true, the
 * events are assumed to be within the same period of the uv circle.  Then the
 * difference between the fine timestamps is calculated and wrapped to
 * [-uv_period_ns / 2, uv_period_ns / 2) and returned.
 *
 * \param arg1 The reference event that is subtracted from
 * \param arg2 The event subtracted from the reference
 * \param ct_diff The window on which to assume events are on the same uv circle
 * \param uv_period_ns The period of the uv circle in nanoseconds
 * \param ct_period_ns The period in nanoseconds of each coarse timestamp tick
 * \param ct_only Only compare events on coarse timestamps, not fine.
 *
 * \return The time difference in nanoseconds
 */
float ModuleCalTimeDiff(
        const ModuleCal & arg1,
        const ModuleCal & arg2,
        int ct_diff = 5,
        float uv_period_ns = UV_PERIOD_NS,
        float ct_period_ns = (1.0e3 / 12),
        bool ct_only = false)
{
    // Assume all events that are within ct_only course timestamps should be
    // compared on the basis of fine timestamps rather than by course timestamp.
    // Example: ct_diff = 5 causes a window of 89.8% of the uv_period_ns at
    // 980kHz uv and a 12MHz ct clock.
    if ((std::abs(arg1.ct - arg2.ct) > ct_diff) | ct_only) {
        return((arg1.ct - arg2.ct) * ct_period_ns);
    }
    float difference = arg1.ft - arg2.ft;
    // At this point we assume that the distribution of fine timestamp
    // differences should be from two points within one period of each other
    // however, since we are subtracting, the center of the distribution
    // should be at 0 +/- uv_period_ns.
    while (difference >= uv_period_ns) {
        difference -= uv_period_ns;
    }
    while (difference < -uv_period_ns) {
        difference += uv_period_ns;
    }
    return(difference);
}

bool ModuleCalLessThan(
        const ModuleCal & arg1,
        const ModuleCal & arg2)
{
    float difference = ModuleCalTimeDiff(arg1,arg2);
    return(difference < 0);
}


/*!
 * \brief Calculate if time(event 1) < time(event 2)
 *
 * Effectively returns (EventCalTimeDiff(arg1, arg2) < 0)
 *
 * \param arg1 The reference event that is compared
 * \param arg2 The event compared against the reference
 * \param ct_diff The window on which to assume events are on the same uv circle
 * \param uv_period_ns The period of the uv circle in nanoseconds
 * \param ct_only Only compare events on coarse timestamps, not fine.
 *
 * \return bool indicating if time(event 1) < time(event 2)
 */
bool ModuleCalLessThan(
        const ModuleCal & arg1,
        const ModuleCal & arg2,
        int ct_diff,
        float uv_period_ns,
        bool ct_only)
{
    float difference = ModuleCalTimeDiff(arg1, arg2, ct_diff, uv_period_ns , ct_only);
    return(difference < 0);
}

bool InEnergyWindow(
        const ModuleCal & event,
        float energy_gate_low,
        float energy_gate_high)
{
    if (event.Ecal > energy_gate_high || event.Ecal < energy_gate_low) {
        return(false);
    } else {
        return(true);
    }
}

CoincEvent MakeCoinc(
        const ModuleCal & event_left,
        const ModuleCal & event_right)
{
    CoincEvent event;
    event.ct = event_left.ct;
    event.dtc = event_left.ct - event_right.ct;
    event.dtf = ModuleCalTimeDiff(event_left, event_right);
    event.E1 = event_left.Ecal;
    event.Ec1 = event_left.Ec;
    event.Ech1 = event_left.Ech;
    event.ft1 = event_left.ft;
    event.E2 = event_right.Ecal;
    event.Ec2 = event_right.Ec;
    event.Ech2 = event_right.Ech;
    event.ft2 = event_right.ft;
    event.x1 = event_left.x;
    event.y1 = event_left.y;
    event.x2 = event_right.x;
    event.y2 = event_right.y;
    event.chip1 = event_left.chip;
    event.fin1 = event_left.fin;
    event.m1 = event_left.m;
    event.apd1 = event_left.apd;
    event.crystal1 = event_left.id;
    event.chip2 = event_right.chip;
    event.fin2 = event_right.fin;
    event.m2 = event_right.m;
    event.apd2 = event_right.apd;
    event.crystal2 = event_right.id;
    event.cartridge1 = event_left.cartridge;
    event.cartridge2 = event_right.cartridge;
    event.pos = event_left.pos;

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
    string tree_name = "SCalTree";
    string branch_name = "Time Sorted Data";
    float time_window(20);

    float energy_gate_high(700);
    float energy_gate_low(400);

    bool read_per_crystal_correction(false);
    string input_crystal_cal_filename;

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
        if(strcmp(argv[ix], "-rcal") == 0) {
            read_per_crystal_correction = true;
            input_crystal_cal_filename = string(argv[ix + 1]);
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

    float crystal_cal[SYSTEM_PANELS]
            [CARTRIDGES_PER_PANEL]
            [FINS_PER_CARTRIDGE]
            [MODULES_PER_FIN]
            [APDS_PER_MODULE]
            [CRYSTALS_PER_APD] = {{{{{{0}}}}}};

    if (read_per_crystal_correction) {
        if (verbose) {
            cout << "Reading Input crystal calibration file\n";
        }
        int cal_read_status =
                ReadPerCrystalCal(input_crystal_cal_filename, crystal_cal);
        if (cal_read_status < 0) {
            cerr << "Error in reading input calibration file: "
                 << cal_read_status << endl;
            cerr << "Exiting.." << endl;
            return(-3);
        }
    }

    if (verbose) {
        cout << "Reading Left File" << endl;
    }

    if (verbose) {
        cout << " Opening file " << filename_left << endl;
    }
    TFile *file_left = new TFile(filename_left.c_str());
    if (!file_left || file_left->IsZombie()) {
        cerr << "problems opening file " << filename_left
             << "\n.Exiting" << endl;
        return(-1);
    }

    TTree * tree_left = (TTree *) file_left->Get(tree_name.c_str());
    if (!tree_left) {
        cerr << "problems opening tree " << tree_name << "\n.Exiting" << endl;
        return(-2);
    }
    ModuleCal * event_left = 0;

    if (tree_left->SetBranchAddress(branch_name.c_str(), &event_left) < 0) {
        cerr << "problems opening branch " << tree_name << "\n.Exiting" << endl;
        return(-3);
    }

    long entries = tree_left->GetEntries();

    // Read the full input data file into memory
    std::vector<ModuleCal> events_left(entries);
    for (long ii = 0; ii < entries; ii++) {
        tree_left->GetEntry(ii);
        events_left[ii] = *event_left;
        events_left[ii].ft *= (UV_PERIOD_NS / (2 * M_PI));
        events_left[ii].ft -= crystal_cal[0][event_left->cartridge]
                                         [event_left->fin][event_left->m]
                                         [event_left->apd][event_left->id];
        while (events_left[ii].ft < 0) {
            events_left[ii].ft += UV_PERIOD_NS;
        }
        while (events_left[ii].ft >= UV_PERIOD_NS) {
            events_left[ii].ft -= UV_PERIOD_NS;
        }
    }


    if (verbose) {
        cout << "Reading Right File" << endl;
    }

    if (verbose) {
        cout << " Opening file " << filename_right << endl;
    }
    TFile * file_right = new TFile(filename_right.c_str());
    if (!file_right || file_right->IsZombie()) {
        cerr << "problems opening file " << filename_right
             << "\n.Exiting" << endl;
        return(-1);
    }

    TTree * tree_right = (TTree *) file_right->Get(tree_name.c_str());
    if (!tree_right) {
        cerr << "problems opening tree " << tree_name << "\n.Exiting" << endl;
        return(-2);
    }
    ModuleCal * event_right = 0;

    if (tree_right->SetBranchAddress(branch_name.c_str(), &event_right) < 0) {
        cerr << "problems opening branch " << tree_name << "\n.Exiting" << endl;
        return(-3);
    }

    entries = tree_right->GetEntries();

    // Read the full input data file into memory
    std::vector<ModuleCal> events_right(entries);
    for (long ii = 0; ii < entries; ii++) {
        tree_right->GetEntry(ii);
        events_right[ii] = *event_right;
        events_right[ii].ft *= (UV_PERIOD_NS / (2 * M_PI));
        events_right[ii].ft -= crystal_cal[1][event_right->cartridge]
                                          [event_right->fin][event_right->m]
                                          [event_right->apd][event_right->id];
        while (events_right[ii].ft < 0) {
            events_right[ii].ft += UV_PERIOD_NS;
        }
        while (events_right[ii].ft >= UV_PERIOD_NS) {
            events_right[ii].ft -= UV_PERIOD_NS;
        }
    }


    if (verbose) {
        cout << "Sorting Events" << endl;
    }
    insertion_sort(events_left, ModuleCalLessThan);
    insertion_sort(events_right, ModuleCalLessThan);


    std::vector<ModuleCal>::iterator iterator_left(events_left.begin());
    std::vector<ModuleCal>::iterator iterator_right(events_right.begin());
    std::vector<ModuleCal>::iterator current_event;
    std::vector<ModuleCal>::iterator pair_event;
    bool current_on_left;


    std::vector<CoincEvent> output_events;
    output_events.reserve((int) (0.5 * 0.5 * (events_left.size() +
                                              events_right.size())));


    cout << "Time Window: " << time_window << "ns\n"
         << "Energy Window: " << energy_gate_low << "keV to "
         << energy_gate_high << "keV\n";

    long dropped_single(0);
    long dropped_multiple(0);
    long dropped_energy_window(0);

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
                dropped_energy_window++;
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
                dropped_energy_window++;
            } else {
                break;
            }
        }
        if (iterator_left == events_left.end() ||
                iterator_right == events_right.end())
        {
            // Drop the rest as singles.
            dropped_single += events_left.end() - iterator_left
                            + events_right.end() - iterator_right;
            break;
        }

        // Check to see if the left or right side is first
        if (ModuleCalLessThan(*iterator_left, *iterator_right)) {
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
        while(ModuleCalTimeDiff(*iterator_left, *current_event) < time_window) {
            if (InEnergyWindow(*(iterator_left),
                               energy_gate_low,
                               energy_gate_high))
            {
                events_in_time_window_left++;
            } else {
                dropped_energy_window++;
            }
            iterator_left++;
            if (iterator_left == events_left.end()) {
                break;
            }
        }
        int events_in_time_window_right(0);
        while(ModuleCalTimeDiff(*iterator_right, *current_event) < time_window) {
            if (InEnergyWindow(*(iterator_right),
                               energy_gate_low,
                               energy_gate_high))
            {
                events_in_time_window_right++;
            } else {
                dropped_energy_window++;
            }
            iterator_right++;
            if (iterator_right == events_right.end()) {
                break;
            }
        }

        // 3. Pair events if there is only one event within energy window on other side, pair
        if ((events_in_time_window_left == 1) &&
            (events_in_time_window_right == 1))
        {
            CoincEvent event;
            if (current_on_left) {
                event = MakeCoinc(*current_event, *pair_event);
            } else {
                event = MakeCoinc(*pair_event, *current_event);
            }
            output_events.push_back(event);
        } else if ((events_in_time_window_left == 0) ||
                   (events_in_time_window_right == 0))
        {
            dropped_single += events_in_time_window_left +
                              events_in_time_window_right;
        } else {
            dropped_multiple += events_in_time_window_left +
                                events_in_time_window_right;
        }
        // 4. If there is more than one event, drop all events, go to 1.
    }

    if (verbose) {
        cout << "Writing Out Coincidences" << endl;
    }
    TFile * output_file = new TFile(filename_output.c_str(),"RECREATE");
    if (output_file->IsZombie()) {
        cerr << "Opening output file failed" << endl;
        cerr << "Exiting" << endl;
        return(-5);
    }
    TTree * output_tree =
            new TTree("merged","Merged and Calibrated LYSO-PSAPD data ");

    if (!output_tree) {
        cerr << "Opening output tree failed" << endl;
        cerr << "Exiting" << endl;
        return(-5);
    }

    CoincEvent * output_event = 0;
    output_tree->Branch("Event", &output_event);
    for (size_t ii = 0; ii < output_events.size(); ii++) {
        output_event = &output_events[ii];
        output_tree->Fill();
    }
    output_tree->Write();
    output_file->Close();

    cout << "Events Processed:\n"
         << "  Left - " << events_left.size() << "\n"
         << "  Right - " << events_right.size() << "\n"
         << "Events Dropped:\n"
         << "  Energy - " << dropped_energy_window << "\n"
         << "  Single - " << dropped_single << "\n"
         << "  Multiple - " << dropped_multiple << "\n"
         << "Coincidences: " << output_events.size() << "\n";

    return(0);
}
