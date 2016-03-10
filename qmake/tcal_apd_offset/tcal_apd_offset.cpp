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


TF1* fitgaus_peak(TH1F * hist) {

   // Initial lower and upperlimits + fit
   TF1 *inifit = new TF1("inifit","gaus");
   inifit->SetLineColor(2);
   double low = hist->GetMean()-hist->GetRMS();
   double up  = hist->GetMean()+hist->GetRMS();
   inifit->SetParameter(1, hist->GetMean());
   inifit->SetParameter(2, hist->GetRMS());
   inifit->SetParLimits(2, 0, 1e99);

   hist->Fit(inifit, "Q", "", low, up);

   // Fitting 2nd time
   low = inifit->GetParameter(1) - 1.5 * (inifit->GetParameter(2));
   up  = inifit->GetParameter(1) + 1.5 * (inifit->GetParameter(2));
   hist->Fit(inifit, "Q", "", low, up);

   // Fitting 3rd time
   low = inifit->GetParameter(1) - 2.5 * (inifit->GetParameter(2));
   up  = inifit->GetParameter(1) + 2.5 * (inifit->GetParameter(2));
   hist->Fit(inifit, "Q", "", low, up);

   // Adjusting the range of the fitted function, so we can see the fit Range
   inifit->SetRange(low, up);
   return inifit;
}

float getmax(const TSpectrum & s, int npeaks) {
    int maxpeakheight = 0;
    float maxpos = 0;
    for (int i = 0; i < npeaks; i++) {
        if (i == 0) {
            maxpos = *(s.GetPositionX() + i);
            maxpeakheight = *(s.GetPositionY() + i);
        }
        if ((*(s.GetPositionY() + i)) > maxpeakheight) {
            maxpos = *(s.GetPositionX() + i);
            maxpeakheight = *(s.GetPositionY() + i);
        }
    }
    return(maxpos);
}

float calcApdOffset(
        TH1F * hist,
        bool use_gaussian_fit,
        int min_entries)
{
    if (hist->GetEntries() < min_entries) {
        return(0);
    }
    if (use_gaussian_fit) {
        TF1 * fitfun = fitgaus_peak(hist);
        float value = fitfun->GetParameter(1);
        delete fitfun;
        return(value);
    }
    TSpectrum s;
    int npeaks = s.Search(hist, 3, "", 0.2);
    float offset(0);
    if (npeaks) {
        offset = getmax(s, npeaks);
    }
    return(offset);
}


void usage() {
    cout << "tcal_apd_offset [-vh] -c [config] -f [filename] -f ...\n"
         << "  -o [name] : output edep cal output filename\n"
         << "  -i [name] : input edep cal output filename\n"
         << "  -l [name] : list file of input filenames\n"
         << "  -a [val]  : optional alpha relaxation parameter (default: 1.0)\n"
         << "  -t [val]  : optional window on time calibration (default: 200ns)\n"
         << "  -m [val]  : optional minimum number of entries for fit (default: 200)\n"
         << "  -n [val]  : optional number of bins in histograms (default: 50)\n"
         << "  -ro [name]: optional root output file for timing spectra\n"
         << "  -ao [name]: optional output file of apd offsets\n"
         << "  -gf       : use gaussian fit of the timing specturm\n"
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
    string filename_apd_offset_output;

    bool use_gaussian_fit_flag = false;

    float alpha = 1.0;
    float time_window = 200.0;
    int no_time_bins = 50;
    int min_no_entries = 200;

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
        if (argument == "-gf") {
            use_gaussian_fit_flag = true;
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
        if (argument == "-ao") {
            filename_apd_offset_output = following_argument;
        }
        if (argument == "-l") {
            if (Util::loadFilelistFile(following_argument, filenames) < 0) {
                cerr << "Unable to load filelist: "
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
        }
        if (argument == "-m") {
            if (Util::StringToNumber(following_argument, min_no_entries) < 0) {
                cerr << "Invalid min_no_entries value: "
                     << following_argument << endl;
                return(-2);
            }
        }
        if (argument == "-n") {
            if (Util::StringToNumber(following_argument, no_time_bins) < 0) {
                cerr << "Invalid no_time_bins value: "
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

        cout << "alpha         : " << alpha << endl;
        cout << "time_window   : " << time_window << endl;
        cout << "no_time_bins  : " << no_time_bins << endl;
        cout << "min_no_entries: " << min_no_entries << endl;
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

    vector<vector<vector<vector<vector<float> > > > > apd_time_offset;
    vector<vector<vector<vector<vector<TH1F*> > > > > apd_dtf;
    config.resizeArrayPCFMA<TH1F*>(apd_dtf, 0);
    config.resizeArrayPCFMA<float>(apd_time_offset, 0);
    for (int p = 0; p < config.panels_per_system; p++) {
        for (int c = 0; c < config.cartridges_per_panel; c++) {
            for (int f = 0; f < config.fins_per_cartridge; f++) {
                for (int m = 0; m < config.modules_per_fin; m++) {
                    for (int a = 0; a < config.apds_per_module; a++) {
                        char namestring[30];
                        char titlestring[50];
                        sprintf(namestring,
                                "dtf[%d][%d][%d][%d][%d]",
                                p, c, f, m, a);
                        sprintf(titlestring,
                                "P%dC%dF%dM%dA%d dtf",
                                p, c, f, m, a);
                        apd_dtf[p][c][f][m][a] = new TH1F(
                                    namestring, titlestring,
                                    no_time_bins, -time_window, time_window);
                    }
                }
            }
        }
    }

    if (verbose) {
        cout << "Filling Time Histograms" << endl;
    }
    for (size_t ii = 0; ii < filenames.size(); ii++) {
        string & filename = filenames[ii];
        vector<EventCoinc> coinc_data;
        int read_status = Util::readFileIntoVector(filename, coinc_data);
        if (verbose) {
            cout << filename << " read with status: " << read_status << endl;
        }
        if (read_status < 0) {
            cerr << "Unable to load: " << filename << endl;
            return(-3);
        }

        for (size_t ii = 0; ii < coinc_data.size(); ii++) {
            EventCoinc & event = coinc_data[ii];

            TH1F * apd0_hist =
                    apd_dtf[0][event.cartridge0][event.fin0][event.module0][event.apd0];
            TH1F * apd1_hist =
                    apd_dtf[1][event.cartridge1][event.fin1][event.module1][event.apd1];

            apd0_hist->Fill(event.dtf);
            apd1_hist->Fill(event.dtf);
        }
    }

    if (verbose) {
        cout << "Finding spectrum peaks" << endl;
    }
    for (int p = 0; p < config.panels_per_system; p++) {
        for (int c = 0; c < config.cartridges_per_panel; c++) {
            for (int f = 0; f < config.fins_per_cartridge; f++) {
                for (int m = 0; m < config.modules_per_fin; m++) {
                    for (int a = 0; a < config.apds_per_module; a++) {
                        apd_time_offset[p][c][f][m][a] = alpha * calcApdOffset(
                                    apd_dtf[p][c][f][m][a],
                                    use_gaussian_fit_flag,
                                    min_no_entries);
                        // The right panel is always subtracted from the left,
                        // so we flip the sign.  This was we always subtract
                        // from the fine timestamp of the singles events.
                        if (p == 1) {
                            apd_time_offset[p][c][f][m][a] *= -1;
                        }
                    }
                }
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
                            CrystalCalibration & x_cal = config.calibration[p][c][f][m][a][x];
                            x_cal.time_offset += apd_time_offset[p][c][f][m][a];
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


    if (filename_apd_offset_output != "") {
        if (verbose) {
            cout << "Writing offsets: " << filename_apd_offset_output << endl;
        }
        // Write out to the photopeak file
        ofstream apd_output(filename_apd_offset_output.c_str());
        for (int p = 0; p < config.panels_per_system; p++) {
            for (int c = 0; c < config.cartridges_per_panel; c++) {
                for (int f = 0; f < config.fins_per_cartridge; f++) {
                    for (int m = 0; m < config.modules_per_fin; m++) {
                        for (int a = 0; a < config.apds_per_module; a++) {
                            apd_output << std::fixed << std::setprecision(1)
                                      << apd_time_offset[p][c][f][m][a] << "\n";
                        }
                    }
                }
            }
        }
        apd_output.close();
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
                            apd_dtf[p][c][f][m][a]->Write();
                        }
                    }
                }
            }
        }
        output_file->Close();
    }

    return(0);
}
