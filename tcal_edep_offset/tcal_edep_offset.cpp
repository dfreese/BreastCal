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
#include <TH1F.h>
#include <TF1.h>
#include <TSpectrum.h>
#include <TError.h>
#include <TFile.h>

using namespace std;

void usage() {
    cout << "tcal_edep_offset [-vh] -c [config] -f [filename] -f ...\n"
         << "  -o [name] : output edep cal output filename\n"
         << "  -i [name] : input edep cal output filename\n"
         << "  -l [name] : list file of input filenames\n"
         << "  -a [val]  : optional alpha relaxation parameter (default: 0.3)\n"
         << "  -t [val]  : optional window on time calibration (default: 200ns)\n"
         << "  -m [val]  : optional minimum number of entries for fit (default: 50)\n"
         << "  -b [val]  : optional number of time bins (default: 50)\n"
         << "  -be [val] : optional number of energy bins (default: 100)\n"
         << "  -n [val]  : optional number iterations (default: 10)\n"
         << "  -ro [name]: optional root output file for timing spectra\n"
         << "  -eo [name]: optional output file of edep offsets\n"
         << "  -eh [keV] : high energy gate (default: 600keV)\n"
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
    string filename_input;
    string filename_output;
    string filename_root_output;
    string filename_edep_output;

    bool use_gaussian_fit_flag = false;

    float alpha = 0.3;
    float time_window = 200.0;
    int no_time_bins = 50;
    int min_no_entries = 50;
    int no_iterations = 10;
    int no_energy_bins = 100;
    float energy_gate_low = 400.0;
    float energy_gate_high = 600.0;

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
        if (argument == "-i") {
            filename_input = following_argument;
        }
        if (argument == "-o") {
            filename_output = following_argument;
        }
        if (argument == "-ro") {
            filename_root_output = following_argument;
        }
        if (argument == "-eo") {
            filename_edep_output = following_argument;
        }
        if (argument == "-l") {
            if (Util::loadFilelistFile(following_argument, filenames) < 0) {
                cerr << "Unable to load filelist: "
                     << following_argument << endl;
                return(-2);
            }
        }
        if (argument == "-t") {
            if (Util::StringToNumber(following_argument, time_window) < 0) {
                cerr << "Invalid time_window value: "
                     << following_argument << endl;
                return(-2);
            }
        }
        if (argument == "-a") {
            if (Util::StringToNumber(following_argument, alpha) < 0) {
                cerr << "Invalid alpha value: "
                     << following_argument << endl;
                return(-2);
            }
            if (alpha <= 0 || alpha > 1) {
                cerr << "Invalid alpha value: "
                     << alpha << endl;
                return(-2);
            }
        }
        if (argument == "-m") {
            if (Util::StringToNumber(following_argument, min_no_entries) < 0) {
                cerr << "Invalid min_no_entries value: "
                     << following_argument << endl;
                return(-2);
            }
        }
        if (argument == "-b") {
            if (Util::StringToNumber(following_argument, no_time_bins) < 0) {
                cerr << "Invalid no_time_bins value: "
                     << following_argument << endl;
                return(-2);
            }
        }
        if (argument == "-be") {
            if (Util::StringToNumber(following_argument, no_energy_bins) < 0) {
                cerr << "Invalid no_energy_bins value: "
                     << following_argument << endl;
                return(-2);
            }
        }
        if (argument == "-el") {
            if (Util::StringToNumber(following_argument, energy_gate_low) < 0) {
                cerr << "Invalid energy_gate_low value: "
                     << following_argument << endl;
                return(-2);
            }
        }
        if (argument == "-eh") {
            if (Util::StringToNumber(following_argument, energy_gate_high) < 0) {
                cerr << "Invalid energy_gate_high value: "
                     << following_argument << endl;
                return(-2);
            }
        }
        if (argument == "-n") {
            if (Util::StringToNumber(following_argument, no_iterations) < 0) {
                cerr << "Invalid no_iterations value: "
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
            filename_output = filenames[0] + ".tcal";
        }
    }

    // Bail if neither of the output files have been specified
    if (filename_output == "") {
        cerr << "No output filename specified" << endl;
        return(-5);
    }

    if (verbose) {
        cout << "filenames:       " << Util::vec2String(filenames) << endl;
        cout << "filename_input: " << filename_input << endl;
        cout << "filename_output: " << filename_output << endl;
        cout << "filename_config: " << filename_config << endl;
        cout << "filename_root_output: " << filename_root_output << endl;
        cout << "filename_crystal_offset_output: "
             << filename_edep_output << endl;

        cout << "alpha             : " << alpha << endl;
        cout << "time_window       : " << time_window << endl;
        cout << "no_time_bins      : " << no_time_bins << endl;
        cout << "no_energy_bins    : " << no_energy_bins << endl;
        cout << "energy_gate_low   : " << energy_gate_low << endl;
        cout << "energy_gate_high  : " << energy_gate_high << endl;
        cout << "min_no_entries    : " << min_no_entries << endl;
    }

    if (verbose && use_gaussian_fit_flag) {
        cout << "Using Gaussian fit of timing spectra" << endl;
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

    if (filename_input != "") {
        int tcal_load_status = config.loadTimeCalWithEdep(filename_input);
        if (verbose) {
            cout << "tcal_load_status: " << tcal_load_status << endl;
        }
        if (tcal_load_status < 0) {
            cerr << "SystemConfiguration.loadTimeCalWithEdep() "
                 << "failed with status: " << tcal_load_status
                 << endl;
            return(-3);
        }
    }

    vector<vector<float> > fixed_offset;
    vector<vector<float> > edep_offset;
    vector<vector<TH2F*> > edep_hist;
    vector<vector<TF1*> > edep_profile_fit;
    vector<vector<TH1F*> > edep_profile_hist;
    config.resizeArrayPC<TH2F*>(edep_hist, 0);
    config.resizeArrayPC<TF1*>(edep_profile_fit, 0);
    config.resizeArrayPC<TH1F*>(edep_profile_hist, 0);
    config.resizeArrayPC<float>(fixed_offset, 0);
    config.resizeArrayPC<float>(edep_offset, 0);
    for (int p = 0; p < config.panels_per_system; p++) {
        for (int c = 0; c < config.cartridges_per_panel; c++) {
            char namestring[512];
            char titlestring[512];
            sprintf(namestring, "edep[%d][%d]", p, c);
            sprintf(titlestring,
                    "Energy Depdendence P%dC%d",
                    p, c);
            edep_hist[p][c] = new TH2F(
                        namestring, titlestring,
                        no_energy_bins,
                        energy_gate_low - 511.0, energy_gate_high - 511.0,
                        no_time_bins,
                        -time_window, time_window);

            sprintf(namestring, "edep_fit[%d][%d]", p, c);
            edep_profile_fit[p][c] = new TF1(
                        namestring, "pol1",
                        energy_gate_low - 511.0,
                        energy_gate_high - 511.0);
        }
    }

    // With coincidence events, we should be able to read all of them into
    // memory, (1GB is 14e6 events)
    if (verbose) {
        cout << "Loading Files" << endl;
    }
    deque<EventCoinc> coinc_data;
    for (size_t ii = 0; ii < filenames.size(); ii++) {
        string & filename = filenames[ii];
        int read_status = Util::readFileIntoDeque(filename, coinc_data);
        if (verbose) {
            cout << filename << " read with status: " << read_status << endl;
        }
        if (read_status < 0) {
            cerr << "Unable to load: " << filename << endl;
            return(-3);
        }
    }

    for (int iter_no = 0; iter_no < no_iterations; iter_no++) {

        // Clear the histograms prior to every iteration
        for (int p = 0; p < config.panels_per_system; p++) {
            for (int c = 0; c < config.cartridges_per_panel; c++) {
                TH2F * h = edep_hist[p][c];
                h->Reset();
            }
        }


        if (verbose) {
            cout << "Filling Time Histograms with "
                 << coinc_data.size()
                 << " events" << endl;
        }
        for (size_t ii = 0; ii < coinc_data.size(); ii++) {
            // Don't copy by reference so we don't modify the initial distribution
            EventCoinc event = coinc_data[ii];
            // That way we can always call TimeCalCoincEvent which will apply the
            // full correction
            TimeCalCoincEvent(event, &config);

            TH2F * cart0_hist = edep_hist[0][event.cartridge0];
            TH2F * cart1_hist = edep_hist[1][event.cartridge1];

            cart0_hist->Fill(event.E0 - 511.0, event.dtf);
            cart1_hist->Fill(event.E1 - 511.0, event.dtf);
        }

        if (verbose) {
            cout << "Finding spectrum peaks" << endl;
        }
        for (int p = 0; p < config.panels_per_system; p++) {
            for (int c = 0; c < config.cartridges_per_panel; c++) {
                edep_profile_hist[p][c] = (TH1F *) edep_hist[p][c]->ProfileX();

                stringstream fit_name_stream;
                fit_name_stream << "edep_profile[" << p << "][" << c << "]";
                edep_profile_hist[p][c]->SetName(
                            fit_name_stream.str().c_str());
                edep_profile_hist[p][c]->Fit(edep_profile_fit[p][c], "Q");

                fixed_offset[p][c] = alpha *
                        edep_profile_fit[p][c]->GetParameter(0);

                edep_offset[p][c] = alpha *
                        edep_profile_fit[p][c]->GetParameter(1);


                // The right panel is always subtracted from the left,
                // so we flip the sign.  This was we always subtract
                // from the fine timestamp of the singles events.
                if (p == 1) {
                    fixed_offset[p][c] *= -1;
                    edep_offset[p][c] *= -1;
                }
            }
        }

        if (verbose) {
            cout << "Applying offsets to calibration" << endl;
        }
        for (int p = 0; p < config.panels_per_system; p++) {
            for (int c = 0; c < config.cartridges_per_panel; c++) {
                for (int f = 0; f < config.fins_per_cartridge; f++) {
                    for (int m = 0; m < config.modules_per_fin; m++) {
                        for (int a = 0; a < config.apds_per_module; a++) {
                            for (int x = 0; x < config.crystals_per_apd; x++) {
                                CrystalCalibration & x_cal =
                                            config.calibration[p][c][f][m][a][x];
                                x_cal.time_offset += fixed_offset[p][c];
                                x_cal.time_offset_edep += edep_offset[p][c];
                            }
                        }
                    }
                }
            }
        }
    }

    if (verbose) {
        cout << "Writing out calibration: " << filename_output << endl;
    }
    int write_status = config.writeTimeCalWithEdep(filename_output);
    if (write_status < 0) {
        cerr << "config.writeTimeCalWithEdep() failed with status: "
             << write_status << endl;
        return(-7);
    }


    if (filename_edep_output != "") {
        if (verbose) {
            cout << "Writing offsets: " << filename_edep_output << endl;
        }
        // Write out to the photopeak file
        ofstream edep_output(filename_edep_output.c_str());
        for (int p = 0; p < config.panels_per_system; p++) {
            for (int c = 0; c < config.cartridges_per_panel; c++) {
                edep_output << std::fixed << std::setprecision(1)
                            << fixed_offset[p][c] << " "
                            << std::fixed << std::setprecision(3)
                            << fixed_offset[p][c] << "\n";
            }
        }
        edep_output.close();
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
                edep_hist[p][c]->Write();
                edep_profile_fit[p][c]->Write();
                edep_profile_hist[p][c]->Write();
            }
        }
        output_file->Close();
    }

    return(0);
}
