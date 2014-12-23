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
    cout << " -eg: Energy Gate output. Default output file extension\n"
         << "      becomes \".sort.egate.root\"\n"
         << " -s:  Sort the Output\n"
         << " -of: Specifies output filename. Default replaces .dat.root\n"
         << "      extension with .cal.root\n"
         << " -t:  Optionally sets the expected input tree name.\n"
         << "      Default is mdata\n"
         << " -ot: Optionally sets the output tree name.\n"
         << "      Default is CalTree, SCalTree with sorting enabled\n"
         << " -eh: Set Upper Energy Window - enables energy gating\n"
         << "      Default is 650keV\n"
         << " -el: Set Lower Energy Window - enables energy gating\n"
         << "      Default is 450keV\n";
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
    clock_t start_time(clock());
    string filename;
    bool filenamespec(false);
    Int_t verbose = 0;
    string calibration_filename;
    string input_treename("mdata");
    string output_tree_name("CalTree");
    string output_tree_title("Calibrated Event Data");
    string output_branch_title("Calibrated Event Data");
    string output_filename;
    bool output_filename_spec(false);
    bool output_tree_name_spec(false);
    float energy_gate_high(650);
    float energy_gate_low(450);
    bool energy_gate_output(false);
    bool sort_output(false);


    for(int ix = 1; ix < argc; ix++) {
        if(strncmp(argv[ix], "-h", 2) == 0) {
            usage();
            return(0);
        }

        if(strncmp(argv[ix], "-v", 2) == 0) {
            cout << "Verbose Mode " << endl;
            verbose = 1;
        }
        if(strncmp(argv[ix], "-s", 2) == 0) {
            sort_output = true;
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
        if(strncmp(argv[ix], "-t", 2) == 0) {
            input_treename = string(argv[ix + 1]);
            ix++;
        }
        if(strncmp(argv[ix], "-ot", 3) == 0) {
            output_tree_name = string(argv[ix + 1]);
            ix++;
            output_tree_name_spec = true;
        }
        if(strncmp(argv[ix], "-calf", 5) == 0) {
            calibration_filename = string(argv[ix + 1]);
            ix++;
        }
        if(strncmp(argv[ix], "-eg", 2) == 0) {
            energy_gate_output = true;
        }       
        if(strncmp(argv[ix], "-eh", 3) == 0) {
            stringstream ss;
            ss << argv[ix + 1];
            ss >> energy_gate_high;
            ix++;
            energy_gate_output = true;
        }
        if(strncmp(argv[ix], "-el", 3) == 0) {
            stringstream ss;
            ss << argv[ix + 1];
            ss >> energy_gate_low;
            ix++;
            energy_gate_output = true;
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
        cerr << "Unable to find .cal.root extension in: \"" << filename << "\"" << endl;
        if (!output_filename_spec) {
            cerr << "...Exiting." << endl;
            return(-4);
        }
    }
    string filebase(filename, 0, root_file_ext_pos);

    if (sort_output) {
        output_tree_title = "Time Sorted Calibrated data";
        output_branch_title = "Time Sorted Data";
        if (!output_tree_name_spec) {
            output_tree_name = "SCalTree";
        }
    }

    if (!output_filename_spec) {
        if (energy_gate_output) {
            if (sort_output) {
                output_filename = filebase + ".cal.sort.egate.root";
            } else {
                output_filename = filebase + ".cal.egate.root";
            }
        } else {
            if (sort_output) {
                output_filename = filebase + ".cal.sort.root";
            } else {
                output_filename = filebase + ".cal.root";
            }
        }
    }

    if (verbose) {
        cout << " filename    = " << filename << endl;
        cout << " filebase    = " << filebase << endl;
        cout << " output tree = " << output_tree_name << endl;
        cout << " output file = " << output_filename << endl;
        if (energy_gate_output) {
            cout << "Energy Window: " << energy_gate_low 
                 << " to " << energy_gate_high << endl;
        }
        if (sort_output) {
            cout << "Sorting Enabled" << endl;
        }
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
        cerr << "Problem opening file " << filename;
        cerr << "\n Exiting..." << endl;
        return(-7);
    }

    TTree* input_tree = (TTree *) input_file->Get(input_treename.c_str());
    if (!(input_tree)) {
        cout << " Problem reading " << input_treename << " from file. Exiting." << endl;
        return(-8);
    }
    ModuleDat * input_event = 0;
    input_tree->SetBranchAddress("eventdata",&input_event);

    if (verbose) {
        cout << "Reading UV Centers from file" << endl;
    }
    TVector *uu_c[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE];
    TVector *vv_c[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE];
    if (ReadUVCenters(input_file, uu_c, vv_c) < 0) {
        cerr << "Problem reading uv circle centers from " << filename << endl;
        cerr << "Exiting...";
        return(-9);
    }
    if (verbose) {
        cout << "Finished opening input file" << endl;
    }




    Long64_t entries = input_tree->GetEntries();

    std::vector<Long64_t> key;
    std::vector<Long64_t> timestamp;

    if (sort_output) {
        key.resize(entries,0);
        timestamp.resize(entries,0);
        if (verbose) {
            cout << "Gathering Timestamp Data" << endl;
        }
        // Get all timestamps and sort events
        for (Long64_t ii = 0; ii < entries; ii++) {
            key[ii] = ii;
            input_tree->GetEntry(ii);
            timestamp[ii] = input_event->ct;
        }

        if (verbose) {
            cout << "Sorting Data" << endl;
        }
        insertion_sort_with_key(timestamp, key);
        if (verbose) {
            cout << "Sorting Complete" << endl;
        }
    }

    if (verbose) {
        cout << "Writing out calibrated root file" << endl;
    }




    TFile * output_rootfile = new TFile(output_filename.c_str(),"RECREATE");
    TTree * output_tree = new TTree(output_tree_name.c_str(), output_tree_title.c_str());
    ModuleCal * output_event = new ModuleCal();
    output_tree->Branch(output_branch_title.c_str(), &output_event);
    // Generate Sorted Dataset
    long dropped_no_valid_peaks(0);
    long dropped_get_crystal_id(0);
    long dropped_energy_gate(0);
    for (Long64_t ii = 0; ii < entries; ii++) {
        if (sort_output) {
            input_tree->GetEntry(key[ii]);
        } else {
            input_tree->GetEntry(ii);
        }

        int cartridge = input_event->cartridge;
        int fin = input_event->fin;
        int chip = input_event->chip;
        int module = input_event->module;
        int apd = input_event->apd;

        if (!CrystalCal->validpeaks[cartridge][fin][module][apd]) {
            dropped_no_valid_peaks++;
        } else {
            float x = input_event->x;
            float y = input_event->y;
            output_event->x = x;
            output_event->y = y;

            int crystal = CrystalCal->GetCrystalId(x,y,cartridge,fin,module,apd);
            output_event->id = crystal;
            if (crystal < 0) {
                dropped_get_crystal_id++;
            } else {
                output_event->cartridge = cartridge;
                output_event->fin = fin;
                output_event->chip = chip;
                output_event->m = module;
                output_event->apd = apd;

                float E = input_event->E;
                float Ec = input_event->Ec;

                output_event->E = E * GetMean(CrystalCal->GainSpat[cartridge][fin][module][apd]) / 
                    CrystalCal->GainSpat[cartridge][fin][module][apd][crystal];
                output_event->Ecal = E * 511 / CrystalCal->GainSpat[cartridge][fin][module][apd][crystal];
                output_event->Ec = -Ec * 511 / CrystalCal->GainCom[cartridge][fin][module][apd][crystal];

                Float_t event_energy = output_event->Ecal;
                // Write out the event if it's in the energy window or if we're not
                // energy gating the output
                if ((!energy_gate_output) || ((event_energy > energy_gate_low) && (event_energy < energy_gate_high))) {
                    double ft = input_event->ft;
                    output_event->ft = FineCalc(ft,
                            (*uu_c[cartridge][fin])[module*2+apd],
                            (*vv_c[cartridge][fin])[module*2+apd]);

                    Long64_t ct = input_event->ct;
                    output_event->ct = ct;
                    Int_t pos = input_event->pos;
                    output_event->pos = pos;

                    output_tree->Fill();
                } else {
                    dropped_energy_gate++;
                }
            }
        }

    }
    if (verbose) {
        cout << "Finished calibrating and writing out file" << endl;
        if (entries) {
            cout << "Processed events: " << entries << endl;
            cout << "Dropped events:\n"
                << "    No Valid Peaks: " << dropped_no_valid_peaks
                << "  (" << 100 * float(dropped_no_valid_peaks) / float(entries) << " %)\n"
                << "    Bad Crystal Id: " << dropped_get_crystal_id
                << "  (" << 100 * float(dropped_get_crystal_id) / float(entries) << " %)\n"
                << "    Energy Gated:   " << dropped_energy_gate
                << "  (" << 100 * float(dropped_energy_gate) / float(entries) << " %)\n";
        }
    }

    output_tree->Write();
    output_rootfile->Close();

    if (verbose) {
        clock_t end_time(clock());
        cout << "Ticks Required: " << end_time - start_time << endl;
        cout << "Time Required:  " << float(end_time - start_time) / float(CLOCKS_PER_SEC) << endl;  
    }
    return(0);
}
