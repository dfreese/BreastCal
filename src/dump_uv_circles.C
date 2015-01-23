#include "TROOT.h"
#include "Riostream.h"
#include "TTree.h"
#include "TFile.h"
#include "libInit_avdb.h"
#include "myrootlib.h"
#include "Syspardef.h"
#include "ModuleCal.h"
#include "ModuleDat.h"
#include "PixelCal.h"
#include "TVector.h"
#include "sorting.h"
#include <vector>
#include <sstream>

void usage(void)
{
    cout << " Calibrate, Sort, and Energy Gate decoded files.\n\n";
    cout << " Usage:  " << endl;
    cout << " ./process_file [-v,-eg, -s] -f [Filename] -calf [Calibration File]" << endl;
    cout << " -of: Specifies output filename.\n"
         << "      Default adds .uv.dat\n";
    return;
}

int WriteOutUvCircles(
        TVector *uu_c[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE],
        TVector *vv_c[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE],
        const std::string & output_filename)
{
    std::ofstream output_stream;
    output_stream.open(output_filename.c_str());
    if (!output_stream.good()) {
        return(-1);
    }

    for (int cartridge = 0; cartridge < CARTRIDGES_PER_PANEL; cartridge++) {
        for (int fin = 0; fin < FINS_PER_CARTRIDGE; fin++) {
            for (int module = 0; module < MODULES_PER_FIN; module++) {
                for (int apd = 0; apd < APDS_PER_MODULE; apd++) {
                    output_stream << (*uu_c[cartridge][fin])[module * 2 + apd] << " "
                                  << (*vv_c[cartridge][fin])[module * 2 + apd] << "\n";
                }
            }
        }
    }
    return(0);
}

int ReadUVCenters(
        TFile *rfile,
        TVector *uu_c[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE],
        TVector *vv_c[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE])
{
    for (int cc = 0; cc < CARTRIDGES_PER_PANEL; cc++) {
        for (int f = 0; f < FINS_PER_CARTRIDGE; f++) {
            Char_t tmpstring[40];
            sprintf(tmpstring,"timing/uu_c[%d][%d]",cc,f);
            TVector * temp_u = (TVector *) rfile->Get(tmpstring);
            if (!temp_u) {
                return(-1);
            } else {
                uu_c[cc][f] = temp_u;
            }
            sprintf(tmpstring,"timing/vv_c[%d][%d]",cc,f);
            TVector * temp_v = (TVector *) rfile->Get(tmpstring);
            if (!temp_v) {
                return(-1);
            } else {
                vv_c[cc][f] = temp_v;
            }
        }
    }
    return(0);
}

int main(int argc, Char_t *argv[])
{
    string filename;
    bool filenamespec(false);
    Int_t verbose = 0;
    string output_filename;
    bool output_filename_spec(false);

    for(int ix = 1; ix < argc; ix++) {
        if(strncmp(argv[ix], "-h", 2) == 0) {
            usage();
            return(0);
        }
        if(strncmp(argv[ix], "-v", 2) == 0) {
            cout << "Verbose Mode " << endl;
            verbose = 1;
        }
        if(strncmp(argv[ix], "-f", 2) == 0) {
            filename = string(argv[ix + 1]);
            filenamespec = true;
            ix++;
        }
        if(strncmp(argv[ix], "-of", 3) == 0) {
            output_filename = string(argv[ix + 1]);
            output_filename_spec = true;
            ix++;
        }
    }

    if (!filenamespec) {
        cout << " Please provide an input file. " << endl;
        usage();
        return(-2);
    }

    if (!output_filename_spec) {
        output_filename = filename + ".uv.dat";
    }


    if (verbose) {
        cout << "Opening input file" << endl;
    }
    TFile * input_file = new TFile(filename.c_str(),"OPEN");

    if (!input_file->IsOpen()) {
        cerr << "Problem opening file " << filename;
        cerr << "\n Exiting..." << endl;
        return(-7);
    }

    if (verbose) {
        cout << "Reading UV Centers from file" << endl;
    }

    TVector *uu_c[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE];
    TVector *vv_c[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE];
    int read_uv_status(0);
    read_uv_status = ReadUVCenters(input_file, uu_c, vv_c);

    if (read_uv_status < 0) {
        cerr << "Problem reading uv circle centers from "
             << filename << endl;
        cerr << "Exiting..." << endl;
        return(-9);
    }
    if (verbose) {
        cout << "Finished opening input file" << endl;
    }

    if (verbose) {
        cout << "Writing out UV Centers to file" << endl;
    }
    int write_out_status(WriteOutUvCircles(uu_c, vv_c, output_filename));

    if (write_out_status < 0) {
        cerr << "Problem writing out uv circle centers to "
             << output_filename << endl;
        cerr << "Exiting..." << endl;
        return(-10);
    }

    if (verbose) {
        cout << "Finished writing out UV Centers to file" << endl;
    }

    return(0);
}
