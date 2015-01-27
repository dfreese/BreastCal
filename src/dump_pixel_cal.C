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
#include <fstream>

void usage(void)
{
    cout << " Dump the Pixel Calibration file into a text file.\n\n";
    cout << " Usage:  " << endl;
    cout << " ./process_file [-v] -f [Calibration File]" << endl;
    cout << " -of: Specifies output filename.\n"
         << "      Default is [filename].par.root.dat\n";
    return;
}

int WriteOutPixelCal(
        PixelCal * CrystalCal,
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
                    for (int crystal = 0; crystal < CRYSTALS_PER_APD; crystal++) {
                        if (CrystalCal->validpeaks[cartridge][fin][module][apd]) {
                            output_stream << "1 "
                                          << CrystalCal->X[cartridge][fin][module][apd][crystal] << " "
                                          << CrystalCal->Y[cartridge][fin][module][apd][crystal] << " "
                                          << CrystalCal->GainSpat[cartridge][fin][module][apd][crystal] << " "
                                          << CrystalCal->GainCom[cartridge][fin][module][apd][crystal] << " "
                                          << CrystalCal->EresSpat[cartridge][fin][module][apd][crystal] << " "
                                          << CrystalCal->EresCom[cartridge][fin][module][apd][crystal] << "\n";
                        } else {
                            output_stream << "0 0 0 0 0 0 0\n";
                        }
                    }
                }
            }
        }
    }
    return(0);
}

int main(int argc, Char_t *argv[])
{
    Int_t verbose = 0;
    string calibration_filename;
    string output_filename;
    bool output_filename_spec(false);
    bool output_tree_name_spec(false);
    float energy_gate_high(650);
    float energy_gate_low(450);
    bool energy_gate_output(false);
    bool sort_output(false);
    bool read_uv_from_calibration_file(false);


    for(int ix = 1; ix < argc; ix++) {
        if(strncmp(argv[ix], "-h", 2) == 0) {
            usage();
            return(0);
        }
        if(strncmp(argv[ix], "-v", 2) == 0) {
            cout << "Verbose Mode " << endl;
            verbose = 1;
        }
        if(strncmp(argv[ix], "-of", 3) == 0) {
            output_filename = string(argv[ix + 1]);
            output_filename_spec = true;
            ix++;
        }
        if(strncmp(argv[ix], "-f", 5) == 0) {
            calibration_filename = string(argv[ix + 1]);
            ix++;
        }
    }

    if (calibration_filename == "") {
        cerr << "Calibration File not specified.  Exiting" << endl;
        usage();
        return(-3);
    }

    if (!output_filename_spec) {
        output_filename = calibration_filename + ".dat";
    }

    if (verbose) {
        cout << "Opening calibration file" << endl;
    }
    TFile *calibration_file = new TFile(calibration_filename.c_str());

    if (calibration_file->IsZombie()) {
        cerr << "Error opening file : " << calibration_filename << endl;
        cerr << "Exiting..." << endl;
        return(-5);
    }

    PixelCal *CrystalCal;
    CrystalCal = (PixelCal *) calibration_file->Get("CrysCalPar");
    if (!CrystalCal) {
        cerr << "Problem reading CrystalCalPar from " << calibration_filename << endl;
        cerr << "Exiting." << endl;
        return(-6);
    }
    if (verbose) {
        cout << "Finished opening calibration file" << endl;
    }

    WriteOutPixelCal(CrystalCal, output_filename);

    return(0);
}
