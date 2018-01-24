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
    cout << "decode [-vh] -c [config] -p [ped file] -f [filename] -f ...\n"
         << "  -o [name] : decoded output filename\n"
         << "  -l [name] : list file of input filenames\n"
         << "  -ro [name]: optional root output file\n"
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
    string filename_root_output;

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
        if (argument == "-o") {
            filename_output = following_argument;
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
            filename_output = filenames[0] + ".dec";
        }
    }

    if (filename_output == "") {
        cerr << "No output filename specified" << endl;
        return(-5);
    }

    if (verbose) {
        cout << "filenames:       " << Util::vec2String(filenames) << endl;
        cout << "filename_config: " << filename_config << endl;
        cout << "filename_ped:    " << filename_ped << endl;
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

    if (filename_ped != "") {
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
    }

    if (verbose) {
        cout << "Decoding Data" << endl;
    }
    deque<char> file_data;
    ProcessInfo info;
    vector<EventRaw> raw_events;

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
        ProcessParams::DecodeBuffer(file_data, raw_events, info, &config);
        ProcessParams::ClearProcessedData(file_data, info);;
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
                (char*) raw_events.data(),
                sizeof(EventRaw) * raw_events.size()))
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


    if (verbose) {
        cout << info.getDecodeInfo() << endl;
    }

    return(0);
}
