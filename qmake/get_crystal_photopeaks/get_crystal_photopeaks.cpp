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

// enecal / Sel_GetEhis - fitting the crystal photopeaks within an APD
// For spatial channels:
//     Get spatial photopeak from getfloods
//     For corner crystals:
//         Call GetCrystalPhotopeak with window of photopeak * (0.7 to 1.15)
//         without forcing identification
//         If that fails, force with window of photopeak * (0.6 to 1.5)
//     For non-corner crystals:
//         Call GetCrystalPhotopeak with window of photopeak * (0.85 to 1.15)
//         without forcing identification
//         If that fails, force with window of photopeak * (0.7 to 1.5)
// For common channels:
//     Get common photopeak from getfloods
//     Call GetCrystalPhotopeak with window of photopeak * (0.85 to 1.25)
//     without forcing identification
//     If that fails, force with window of photopeak * (0.6 to 1.25) for corner
//     crystals, or photopeak * (0.7 to 1.5) for non-corner crystals.


// Peak search using sigma = 3 and threshold = 0.9
// If two or more peaks are found, and at least one is within the given window
// return the maximum within the window, else if two or more peaks are found,
// but none are within the window, give up, else, lower the threshold by 0.1
// down to and including 0.1 and repeat peak search
//
// Peak search using sigma = 3 and threshold = 0.05
// If one or more peaks are found within the window, return the largest
//
// If force is not selected, give up
//
// Peak search using sigma = 10 and threshold = 0.4, and "nobackground", which
// disables background removal.
// If one or more peaks are found, and at least one is within the given window
// return the maximum within the window, else, lower sigma by 1 down
// to and including 4 and repeat peak search.
//
// Peak search using sigma = 3 and threshold = 0.4
// If one or more peaks are found within the window, return the largest within
// the window, else return the first peak if it is above the lower limit, else
// lower the threshold by 0.1 down to and including 0.2 and repeat peak search.
//
// Note, given the first peak search method takes care of most cases, this
// could be written as:
//
// Peak search using sigma = 3 and threshold = 0.4
// If a peak is found and it is above the lower limit, return it, else lower the
// threshold by 0.1 down to and including 0.2 and repeat peak search.
//
// Give up

Float_t GetCrystalPhotopeak(
        TH1F * hist,
        Float_t xlow,
        Float_t xhigh,
        Bool_t force = false)
{
    if (hist->GetEntries() < 100) {
        return(0);
    }
    TSpectrum *ss = new TSpectrum();

    Int_t npeaks = 0;
    for (int i = 0; (i < 10) && (npeaks < 2); i++) {
        if (i < 9)  {
            npeaks = ss->Search(hist, 3, "", 0.9 - (Float_t) i / 10);
        } else {
            npeaks = ss->Search(hist, 3, "", 0.05);
        }
    }

    // Assume that if one peak is found, that it is the correct peak
    // This will only happen at the lowest threshold of 0.05

    // Take the largest peak with x position larger than the lower limit
    Int_t corpeak = 0;
    Float_t yy = 0;
    for (int i = 0; i < npeaks; i++) {
        if ((yy < *(ss->GetPositionY() + i) ) &&
            (*(ss->GetPositionX() + i) > xlow) &&
            (*(ss->GetPositionX() +i) < xhigh))
        {
            corpeak = i;
            yy = *(ss->GetPositionY() + i);
        }
    }

    // If the peak found initially is within the window, return.
    if (((*(ss->GetPositionX() + corpeak)) >= xlow) &&
        ((*(ss->GetPositionX() + corpeak)) <= xhigh))
    {
        float photopeak = *(ss->GetPositionX() + corpeak);
        delete ss;
        return (photopeak);
    }

    if (!force) {
        delete ss;
        return (-1);
    }

    // If the peak is not found in the window, move sigma rather than threshold
    // I find that sigma = 3 is too small, limit it to 4 by demanding i<7
    for (int i = 0; i < 7; i++) {
        npeaks = ss->Search(hist, 10 - i, "nobackground", 0.4);

        // Take the largest peak with x position larger than the lower limit
        corpeak = 0;
        Float_t yy = 0;
        for (int j = 0; j < npeaks; j++) {
            // CHANGE !!
            if ((yy < *(ss->GetPositionY() + j)) &&
                (*(ss->GetPositionX() + j) > xlow) &&
                (*(ss->GetPositionX() +j) < xhigh))
            {
                corpeak = j;
                yy = *(ss->GetPositionY() + j);
            }
        }

        // If a peak is found within the window, return it
        if (((*(ss->GetPositionX() + corpeak)) >= xlow) &&
            ((*(ss->GetPositionX() + corpeak)) <= xhigh))
        {
            float photopeak = *(ss->GetPositionX() + corpeak);
            delete ss;
            return (photopeak);
        }
    }

    // If a peak is not found within the window, go back to the threshold method
    // but just select the largest peak above the lower limit and disregard
    // the upper limit.
    //
    // probably a bug that corpeak is not reset to zero
    //
    // upper limit isn't actually completely discarded, as it is used in
    // setting corpeak.
    float thresholds[3] = {0.4, 0.3, 0.2};
    // note 9-6-13: changed sigma from 2 to 3
    for (int thresh_idx = 0; thresh_idx < 3; thresh_idx++) {
        npeaks = ss->Search(hist, 3, "", thresholds[thresh_idx]);
        Float_t yy = 0;
        for (int i = 0; i < npeaks; i++) {
            // take largest peak with x position larger than lower fit limit
            // CHANGE !!
            if((yy < *(ss->GetPositionY() + i)) &&
               (*(ss->GetPositionX() + i) > xlow) &&
               (*(ss->GetPositionX() +i) < xhigh))
            {
                corpeak = i;
                yy = *(ss->GetPositionY() + i);
            }
        }

        if ((*(ss->GetPositionX() + corpeak)) >= xlow) {
            float photopeak = *(ss->GetPositionX() + corpeak);
            delete ss;
            return (photopeak);
        }
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

bool isCornerCrystal(int id) {
    bool lookup[64] = {true,  false, false, false, false, false, false, true,
                       false, false, false, false, false, false, false, false,
                       false, false, false, false, false, false, false, false,
                       false, false, false, false, false, false, false, false,
                       false, false, false, false, false, false, false, false,
                       false, false, false, false, false, false, false, false,
                       false, false, false, false, false, false, false, false,
                       true,  false, false, false, false, false, false, true};
    if (id < 0 || id >= 64) {
        return(false);
    }
    return(lookup[id]);
}

void usage() {
    cout << "get_crystal_photopeaks [-vh] -c [config] -p [ped file] -x [loc file] -f [filename] -f ...\n"
         << "  -o [name] : photopeak output filename\n"
         << "  -l [name] : list file of input filenames\n"
         << "  -ro [name]: optional root output file for energy spectra\n"
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
    string filename_pp;
    string filename_root_output;


    int Ebins = 160;
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
        if (argument == "-pp") {
            filename_pp = following_argument;
        }
        if (argument == "-o") {
            filename_output = following_argument;
        }
        if (argument == "-x") {
            filename_loc = following_argument;
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
        cout << "filename_pp:     " << filename_pp << endl;
        cout << "filename_loc:    " << filename_loc << endl;
        cout << "filename_output: " << filename_output << endl;
        cout << "filename_root_output: " << filename_root_output << endl;
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

    int loc_load_status = config.loadCrystalLocations(filename_loc);
    if (loc_load_status < 0) {
        cerr << "SystemConfiguration.loadCrystalLocations() failed, status: "
             << loc_load_status
             << endl;
        return(-5);
    }

    int photopeak_count = 0;
    vector<vector<vector<vector<vector<vector<TH1F*> > > > > > spat_hists;
    vector<vector<vector<vector<vector<vector<TH1F*> > > > > > comm_hists;
    config.resizeArrayPCFMAX<TH1F*>(spat_hists, 0);
    config.resizeArrayPCFMAX<TH1F*>(comm_hists, 0);
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
        int read_status = Util::readFileIntoDeque(filename, file_data);
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

    if (verbose) {
        cout << "Finding photopeaks" << endl;
    }
    for (int p = 0; p < config.panels_per_system; p++) {
        for (int c = 0; c < config.cartridges_per_panel; c++) {
            for (int f = 0; f < config.fins_per_cartridge; f++) {
                for (int m = 0; m < config.modules_per_fin; m++) {
                    for (int a = 0; a < config.apds_per_module; a++) {
                        float spat_photopeak = spat_photopeaks[p][c][f][m][a];
                        float comm_photopeak = comm_photopeaks[p][c][f][m][a];
                        for (int x = 0; x < config.crystals_per_apd; x++) {
                            CrystalCalibration & crystal_cal =
                                    config.calibration[p][c][f][m][a][x];

                            // Setup the initial upper and lower bounds for the
                            // spat and common channels.
                            float lower_bound_spat = 0.85 * spat_photopeak;
                            float upper_bound_spat = 1.15 * spat_photopeak;

                            float lower_bound_comm = 0.85 * comm_photopeak;
                            float upper_bound_comm = 1.25 * comm_photopeak;

                            if (isCornerCrystal(x)) {
                                lower_bound_spat = 0.7 * spat_photopeak;
                                upper_bound_spat = 1.15 * spat_photopeak;
                            }

                            crystal_cal.gain_spat = GetCrystalPhotopeak(
                                        spat_hists[p][c][f][m][a][x],
                                        lower_bound_spat, upper_bound_spat,
                                        false);
                            if (crystal_cal.gain_spat == -1) {
                                if (isCornerCrystal(x)) {
                                    lower_bound_spat = 0.6 * spat_photopeak;
                                    upper_bound_spat = 1.5 * spat_photopeak;
                                } else {
                                    lower_bound_spat = 0.7 * spat_photopeak;
                                    upper_bound_spat = 1.5 * spat_photopeak;
                                }
                                crystal_cal.gain_spat = GetCrystalPhotopeak(
                                            spat_hists[p][c][f][m][a][x],
                                            lower_bound_spat,
                                            upper_bound_spat,
                                            true);
                            }
                            // If the peak isn't found again, set to apd
                            // photopeak
                            if (crystal_cal.gain_spat == 0) {
                                crystal_cal.gain_spat = spat_photopeak;
                            }


                            crystal_cal.gain_comm = GetCrystalPhotopeak(
                                        comm_hists[p][c][f][m][a][x],
                                        lower_bound_comm, upper_bound_comm,
                                        false);
                            if (crystal_cal.gain_comm == -1) {
                                if (isCornerCrystal(x)) {
                                    lower_bound_comm = 0.6 * comm_photopeak;
                                    upper_bound_spat = 1.25 * comm_photopeak;
                                } else {
                                    lower_bound_comm = 0.7 * comm_photopeak;
                                    upper_bound_comm = 1.5 * comm_photopeak;
                                }
                                crystal_cal.gain_comm = GetCrystalPhotopeak(
                                            comm_hists[p][c][f][m][a][x],
                                            lower_bound_comm,
                                            upper_bound_comm,
                                            true);
                            }

                            // If the peak isn't found again, set to apd
                            // photopeak
                            if (crystal_cal.gain_comm == 0) {
                                crystal_cal.gain_comm = comm_photopeak;
                            }
                        }
                    }
                }
            }
        }
    }

    // TODO: Fit each peak and place it into the calibration array

    float EFITMIN = 0;
    float EFITMAX = 2400;
    float EFITMIN_COM = 0;
    float EFITMAX_COM = 1200;
    float MINEHISTENTRIES = 1000;
    if (verbose) {
        cout << "Fitting photopeaks" << endl;
    }
    for (int p = 0; p < config.panels_per_system; p++) {
        for (int c = 0; c < config.cartridges_per_panel; c++) {
            for (int f = 0; f < config.fins_per_cartridge; f++) {
                for (int m = 0; m < config.modules_per_fin; m++) {
                    for (int a = 0; a < config.apds_per_module; a++) {
                        for (int x = 0; x < config.crystals_per_apd; x++) {
                            CrystalCalibration & crystal_cal =
                                    config.calibration[p][c][f][m][a][x];
                            TH1F * spat_hist = spat_hists[p][c][f][m][a][x];
                            TH1F * comm_hist = comm_hists[p][c][f][m][a][x];

                            TF1 spat_fit("spat_fit", "gaus", EFITMIN, EFITMAX);
                            TF1 comm_fit("comm_fit", "gaus",
                                         EFITMIN_COM, EFITMAX_COM);

                            if (spat_hist->GetEntries() > MINEHISTENTRIES) {
                                spat_hist->Fit(
                                            &spat_fit,
                                            "Q", "",
                                            crystal_cal.gain_spat * 0.8,
                                            crystal_cal.gain_spat * 1.15);
                            }
                            if (comm_hist->GetEntries() > MINEHISTENTRIES) {
                                comm_hist->Fit(
                                            &comm_fit,
                                            "Q", "",
                                            crystal_cal.gain_comm * 0.8,
                                            crystal_cal.gain_comm * 1.15);
                            }

                            if (spat_fit.GetParameter(1)) {
                               crystal_cal.eres_spat =
                                       235.0 * spat_fit.GetParameter(2) /
                                       spat_fit.GetParameter(1);
                            } else {
                                crystal_cal.eres_spat = -1;
                            }

                            if (comm_fit.GetParameter(1)) {
                               crystal_cal.eres_comm =
                                       235.0 * comm_fit.GetParameter(2) /
                                       comm_fit.GetParameter(1);
                            } else {
                                crystal_cal.eres_comm = -1;
                            }

                        }
                    }
                }
            }
        }
    }

    if (verbose) {
        cout << "Writing out calibration to " << filename_output << endl;
    }
    int config_write_status = config.writeCalibration(filename_output);
    if (config_write_status < 0) {
        cerr << "SystemConfiguration.writeCalibration() failed, status: "
             << config_write_status
             << endl;
        return(-7);
    }

    if (filename_root_output != "") {
        if (verbose) {
            cout << "Writing histograms to " << filename_root_output << endl;
        }
        TFile * output_file =
                new TFile(filename_root_output.c_str(), "RECREATE");
        if (output_file->IsZombie()) {
            cerr << "Unable to open root output file: "
                 << filename_root_output << endl;
            return(-6);
        }
        output_file->cd();
        for (int p = 0; p < config.panels_per_system; p++) {
            for (int c = 0; c < config.cartridges_per_panel; c++) {
                for (int f = 0; f < config.fins_per_cartridge; f++) {
                    for (int m = 0; m < config.modules_per_fin; m++) {
                        for (int a = 0; a < config.apds_per_module; a++) {
                            for (int x = 0; x < config.crystals_per_apd; x++) {
                                // Copy the loaded photopeaks into their vectors
                                spat_hists[p][c][f][m][a][x]->Write();
                                comm_hists[p][c][f][m][a][x]->Write();
                            }
                        }
                    }
                }
            }
        }
        output_file->Close();
    }

    if (verbose) {
        cout << info.getDecodeInfo() << endl;
        cout << info.getCalibrateInfo() << endl;
    }
    return(0);
}
