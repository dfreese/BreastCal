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
#include "sorting.h"
#include <vector>
#include <sstream>
#include "TVector.h"
#include <time.h>

void usage(void)
{
    cout << " Usage:  " << endl;
    cout << " ./egate_file [-v] -f [Filename] -calf [Calibration File]" << endl;
    cout << " -of: Optionally specifies output filename.\n"
         << "      Default replaces .dat.root extension with .dat.cal.root\n"
         << " -t:  Optionally sets the expected input tree name.\n"
         << "      Default is mdata\n"
         << " -ot: Optionally sets the output tree name.\n"
         << "      Default is CalTree\n";
    return;
}

float GetMean(Float_t *array, int array_size = 64) {
    double sum(0);
    for (int ii = 0; ii < array_size; ii++) {
        sum += array[ii];
    }
    return(sum/float(array_size));
}

double FineCalc(double uv, float u_cent, float v_cent) {
    Int_t UV = (Int_t) uv;
    Int_t u = (( UV & 0xFFFF0000 ) >> 16 );
    Int_t v = ( UV & 0xFFFF );
    double tmp = TMath::ATan2(u-u_cent,v-v_cent);
    if (tmp < 0.0) {
        tmp+=2*3.141592;
    }
    return tmp;///(2*3.141592*CIRCLEFREQUENCY);
}


int ReadUVCenters(
        TFile *rfile,
        TVector *uu_c[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE],
        TVector *vv_c[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE])
{
    Char_t tmpstring[40];
    for (Int_t cc=0; cc<CARTRIDGES_PER_PANEL; cc++) {
        for (Int_t f=0; f<FINS_PER_CARTRIDGE; f++) {
            sprintf(tmpstring,"timing/uu_c[%d][%d]",cc,f);
            uu_c[cc][f]= (TVector *) rfile->Get(tmpstring);
            sprintf(tmpstring,"timing/vv_c[%d][%d]",cc,f);
            vv_c[cc][f]= (TVector *) rfile->Get(tmpstring);
        }
    }
    return 0;
}


int main(int argc, Char_t *argv[])
{
    clock_t start_time(clock());
    string filename;
    bool filenamespec(false);
    Int_t verbose = 0;
    string input_treename("mdata");
    string output_treename("CalTree");
    string output_filename;
    string calibration_filename;
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
        if(strncmp(argv[ix], "-of", 2) == 0) {
            output_filename = string(argv[ix + 1]);
            output_filename_spec = true;
            ix++;
        }
        if(strncmp(argv[ix], "-calf", 5) == 0) {
            calibration_filename = string(argv[ix + 1]);
            ix++;
        }
        if(strncmp(argv[ix], "-t", 2) == 0) {
            input_treename = string(argv[ix + 1]);
            ix++;
        }
        if(strncmp(argv[ix], "-ot", 3) == 0) {
            output_treename = string(argv[ix + 1]);
            ix++;
        }
    }

    if (!filenamespec) {
        cout << " Please provide an input file. " << endl;
        usage();
        return(-2);
    }

    if (calibration_filename == "") {
        cerr << "Calibration File not specified.  Exiting" << endl;
        usage();
        return(-3);
    }

    size_t root_file_ext_pos(filename.rfind(".dat.root"));
    if (root_file_ext_pos == string::npos) {
        cerr << "Unable to find .cal.sort.root extension in: \"" << filename << "\"" << endl;
        if (!output_filename_spec) {
            cerr << "Please specify an output filename "
                 << "or pick a correctly named file." << endl;
            cerr << "...Exiting." << endl;
            return(-4);
        }
    }
    string filebase(filename, 0, root_file_ext_pos);

    if (verbose) {
        cout << " filename = " << filename << endl;
        cout << " filebase = " << filebase << endl;
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

    if (verbose) {
        cout << "Opening input file" << endl;
    }
    TFile * input_file = new TFile(filename.c_str(),"OPEN");

    if (!input_file->IsOpen()) {
        cout << "problems opening file " << filename;
        cout << "\n Exiting " << endl;
        return(-8);
    }

    TTree * input_tree = (TTree *) input_file->Get(input_treename.c_str());
    if (!input_tree) {
        cout << " Problem reading " << input_treename << " from file. Exiting." << endl;
        return(-9);
    }
    ModuleDat * uncalibrated_event = 0;
    input_tree->SetBranchAddress("eventdata",&uncalibrated_event);


    TVector *uu_c[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE];
    TVector *vv_c[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE];
    ReadUVCenters(input_file, uu_c, vv_c);


    if (!output_filename_spec) {
        output_filename = filebase + ".dat.cal.root";
    }
    if (verbose) {
        cout << "Output File: " << output_filename << endl;
    }

    if (verbose) {
        cout << "Calibrating and writing out file" << endl;
    }
    Long64_t entries=input_tree->GetEntries();
    TFile * output_rootfile = new TFile(output_filename.c_str(),"RECREATE");
    TTree * output_tree = new TTree(output_treename.c_str(),"Time Sorted Calibrated data");
    ModuleCal * calibrated_event = new ModuleCal();
    output_tree->Branch("Calibrated Event Data",&calibrated_event);

    long dropped_no_valid_peaks(0);
    long dropped_get_crystal_id(0);
    for (Long64_t ii = 0; ii < entries; ii++) {
        input_tree->GetEntry(ii);

        int cartridge = uncalibrated_event->cartridge;
        int fin = uncalibrated_event->fin;
        int chip = uncalibrated_event->chip;
        int module = uncalibrated_event->module;
        int apd = uncalibrated_event->apd;

        if (!CrystalCal->validpeaks[cartridge][fin][module][apd]) {
            dropped_no_valid_peaks++;
        } else {
            float x = uncalibrated_event->x;
            float y = uncalibrated_event->y;
            calibrated_event->x = x;
            calibrated_event->y = y;

            int crystal = CrystalCal->GetCrystalId(x,y,cartridge,fin,module,apd);
            calibrated_event->id = crystal;
            if (crystal < 0) {
                dropped_get_crystal_id++;
            } else {
                calibrated_event->cartridge = cartridge;
                calibrated_event->fin = fin;
                calibrated_event->chip = chip;
                calibrated_event->m = module;
                calibrated_event->apd = apd;

                Long64_t ct = uncalibrated_event->ct;
                calibrated_event->ct = ct;
                Int_t pos = uncalibrated_event->pos;
                calibrated_event->pos = pos;

                float E = uncalibrated_event->E;
                float Ec = uncalibrated_event->Ec;

                calibrated_event->E = E * GetMean(CrystalCal->GainSpat[cartridge][fin][module][apd]) / 
                    CrystalCal->GainSpat[cartridge][fin][module][apd][crystal];
                calibrated_event->Ecal = E * 511 / CrystalCal->GainSpat[cartridge][fin][module][apd][crystal];
                calibrated_event->Ec = -Ec * 511 / CrystalCal->GainCom[cartridge][fin][module][apd][crystal];

                double ft = uncalibrated_event->ft;
                calibrated_event->ft = FineCalc(ft,
                        (*uu_c[cartridge][fin])[module*2+apd],
                        (*vv_c[cartridge][fin])[module*2+apd]);

                output_tree->Fill();
            }
        }
    }
    output_tree->Write();
    output_rootfile->Close();
    if (verbose) {
        cout << "Finished calibrating and writing out file" << endl;
        cout << "Dropped events:\n"
             << "    No Valid Peaks: " << dropped_no_valid_peaks << "\n"
             << "    Bad Crystal Id: " << dropped_get_crystal_id << endl;
    }

    clock_t end_time(clock());
    cout << "Ticks Required: " << end_time - start_time << endl;
    cout << "Time Required:  " << (end_time - start_time) / CLOCKS_PER_SEC << endl;  
    return(0);
}
