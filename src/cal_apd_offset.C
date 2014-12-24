#include "TROOT.h"
#include "TStyle.h"
#include "Riostream.h"
#include "TTree.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TSpectrum.h"
#include "TMath.h"
#include "TSpectrum.h"
#include "TF1.h"
#include "libInit_avdb.h"
#include "myrootlib.h"
#include "CoincEvent.h"
#include "string.h"
#include "cal_helper_functions.h"
#include "Syspardef.h"
#include <string>

#define MINMODENTRIES 200

Float_t calfunc(
        TH1F *hist,
        TF1 *fitfun,
        Bool_t coarsetime,
        Bool_t gausfit,
        Int_t verbose)
{
    Int_t npeaks;
    TSpectrum *s = new TSpectrum();
    if (gausfit) {
        if ( hist->GetEntries() > MINMODENTRIES ) {
            if (coarsetime) { 
                fitfun = new TF1("fitfun", "gaus+pol0",-75,75);
                fitfun->SetParameter(0,200);
                fitfun->SetParameter(1,0);
                fitfun->SetParameter(2,90);
                fitfun->SetParameter(4,100); 
                hist->Fit(fitfun,"RQ");
                return(fitfun->GetParameter(1)); 
            } else {
                fitfun = fitgaus_peak(hist,1) ;
                return(fitfun->GetParameter(1));
            }
        }
    } 
    npeaks = s->Search(hist,3,"",0.2);
    if (npeaks) {
        return getmax(s, npeaks, verbose);
    } else {
        return(0);
    }
}

void usage() {
    cout << "cal_apd_offset [-v -h --help] -f (Filename)\n"
         << "\n"
         << "Options:"
         << "  -n:  do not write out the resulting root file\n"
         << "  -dp:  do not print out any postscript files\n"
         << "  -pto:  only write out the time resolution plot\n"
         << "  -gf:  use a gaussian fit in the calibration\n"
         << "  -rcal (filename):  read in initial per crystal calibration\n"
         << "  -wcal (filename):  write out per crystal calibration\n"
         << "  -ft (limit ns):  use fine timestamp limit for calibration\n"
         << "      default: use limit +/-400ns\n";
}

int main(int argc, Char_t *argv[])
{ 
    Bool_t coarsetime=1; 
    Int_t usegausfit=0;
    Int_t verbose = 0;

    Int_t DTF_low;
    Int_t DTF_hi;
    Int_t FINELIMIT(400);

    string filename;

    float energy_gate_low(400);
    float energy_gate_high(600);

    // A hard limit placed on all of the data used for calibration
    const float course_time_limit(6);

    bool read_per_crystal_correction(false);
    bool write_per_crystal_correction(false);
    string input_crystal_cal_filename;
    string output_crystal_cal_filename;

    // A flag that is true unless a -n option is found in which case only the
    // calibration parameters and the associated plots are generated, but the
    // calibrated root file is not created.
    bool write_out_root_file_flag(true);
    // A flag that disables print outs of postscript files.  Default is on.
    // Flag is disabled by -dp.
    bool write_out_postscript_flag(true);
    bool write_out_time_res_plot_flag(true);

    cout << " Welcome to cal_apd_offset" << endl;


    for(int ix = 1; ix < argc; ix++) {
        if(strncmp(argv[ix], "-v", 2) == 0) {
            cout << "Verbose Mode " << endl;
            verbose = 1;
        }
        if(strncmp(argv[ix], "-h", 2) == 0) {
            usage();
            return(0);
        }
        if(strncmp(argv[ix], "--help", 2) == 0) {
            usage();
            return(0);
        }
        if(strncmp(argv[ix], "-n", 2) == 0) {
            cout << "Calibrated Root File will not be created." << endl;
            write_out_root_file_flag = false;
        }
        if(strcmp(argv[ix], "-dp") == 0) {
            cout << "Postscript Files will not be created." << endl;
            write_out_postscript_flag = false;
            write_out_time_res_plot_flag = false;
        }
        if(strcmp(argv[ix], "-pto") == 0) {
            cout << "Only time resolution plot will be created." << endl;
            write_out_postscript_flag = false;
            write_out_time_res_plot_flag = true;
        }
        if(strncmp(argv[ix], "-gf", 3) == 0) {
            usegausfit=1;
            cout << " Using Gauss Fit "  <<endl;
        }
        if(strcmp(argv[ix], "-rcal") == 0) {
            read_per_crystal_correction = true;
            input_crystal_cal_filename = string(argv[ix + 1]);
        }
        if(strcmp(argv[ix], "-wcal") == 0) {
            write_per_crystal_correction = true;
            output_crystal_cal_filename = string(argv[ix + 1]);
        }
        if(strncmp(argv[ix], "-f", 2) == 0) {
            if(strncmp(argv[ix], "-ft", 3) == 0) {
                coarsetime=0;
                ix++;
                if (ix == argc) {
                    cout << " Please enter finelimit interval: -ft [finelimit]\nExiting. " << endl;
                    return(-1);
                } 
                FINELIMIT = atoi(argv[ix]);
                if (FINELIMIT < 1) {
                    cout << " Error. FINELIMIT = " << FINELIMIT << " too small. Please specify -ft [finelimit]. " << endl;
                    cout << "Exiting." << endl;
                    return(-2);
                }
                cout << " Fine time interval = "  << FINELIMIT << endl;
            } else {
                /* filename '-f' */
                filename = string(argv[ix + 1]);
            }
        }
    }

    rootlogon(verbose);
    gStyle->SetOptStat(kTRUE); 
    if (!(verbose)) {
        gErrorIgnoreLevel=kError;
    }



    TCanvas *c1(0);
    c1 = (TCanvas*)gROOT->GetListOfCanvases()->FindObject("c1");
    if (!c1) c1 = new TCanvas("c1","c1",10,10,1000,1000);
    c1->SetCanvasSize(700,700);


    TH1F * apdoffset[SYSTEM_PANELS][CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE];
    TF1 * fit_apdoffset[SYSTEM_PANELS][CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE];

    if (coarsetime) {
        DTF_low = -300;
        DTF_hi = 300;
        cout << " Using Coarse limits: " << FINELIMIT << endl;
    } else {
        DTF_low = -50;
        DTF_hi = 50;
    }

    for (int panel = 0; panel < SYSTEM_PANELS; panel++) {
        for (int cartridge = 0; cartridge < CARTRIDGES_PER_PANEL; cartridge++) {
            for (int fin = 0; fin < FINS_PER_CARTRIDGE; fin++) {
                for (int mod=0; mod<MODULES_PER_FIN; mod++) {
                    for (int apd=0; apd<APDS_PER_MODULE; apd++) {
                        Char_t histtitle[40];
                        sprintf(histtitle,"apdoffset[%d][%d][%d][%d][%d]",panel, cartridge, fin, mod, apd);
                        if ((panel == 0) || (!coarsetime))  {
                            apdoffset[panel][cartridge][fin][mod][apd] = new TH1F(histtitle,histtitle,50,DTF_low,DTF_hi);
                        } else {
                            apdoffset[panel][cartridge][fin][mod][apd] = new TH1F(histtitle,histtitle,50,DTF_low+150,DTF_hi-150);
                        }
                    }
                }
            }
        }
    }

    if (filename == "") {
        cerr << "Filename not specified" << endl;
        cerr << "Exiting..." << endl;
        usage();
        return(-2);
    }
    size_t root_file_ext_pos(filename.rfind(".root"));
    if (root_file_ext_pos == string::npos) {
        cerr << "Unable to find .root extension in: \"" << filename << "\"" << endl;
        cerr << "...Exiting." << endl;
        return(-1);
    }
    string filebase(filename, 0, root_file_ext_pos);
    if (verbose) cout << "filebase: " << filebase << endl;
    string rootfile(filebase + ".apdoffcal.root");
    if (verbose) cout << " ROOTFILE = " << rootfile << endl;


    if (verbose) {
        cout << "Reading Input crystal calibration file\n";
    }
    float crystal_cal[SYSTEM_PANELS]
            [CARTRIDGES_PER_PANEL]
            [FINS_PER_CARTRIDGE]
            [MODULES_PER_FIN]
            [APDS_PER_MODULE]
            [CRYSTALS_PER_APD] = {{{{{{0}}}}}};

    if (read_per_crystal_correction) {
        int cal_read_status(ReadPerCrystalCal(
                input_crystal_cal_filename,crystal_cal));
        if (cal_read_status < 0) {
            cerr << "Error in reading input calibration file: "
                 << cal_read_status << endl;
            cerr << "Exiting.." << endl;
            return(-2);
        }
    }

    cout << " Opening file " << filename << endl;
    TFile *rtfile = new TFile(filename.c_str(),"OPEN");
    TTree *mm  = (TTree *) rtfile->Get("merged");

    if (!mm) {
        if (verbose) cout << " Problem reading Tree merged "   << " from file " << filename << ". Trying tree mana." << endl;
        mm  = (TTree *) rtfile->Get("mana");
    }

    if (!mm) {
        cout << " Problem reading Tree mana "   << " from file " << filename << endl;
        cout << " Exiting " << endl;
        return -10;
    }

    CoincEvent *evt = new CoincEvent();
    mm->SetBranchAddress("Event",&evt);

    Long64_t entries = mm->GetEntries();

    cout << " Total  entries: " << entries << endl; 
    if (verbose) cout << " Filling crystal spectra on the left. " << endl;
    Long64_t checkevts=0;


    // Fill histograms for the left panel with their associated events
    int panel = 0;
    for (Long64_t ii = 0; ii < entries; ii++) {
        mm->GetEntry(ii);
        if (BoundsCheckEvent(*evt) == 0) {
            if (EnergyGateEvent(*evt, energy_gate_low, energy_gate_high) == 0) {
                if (TMath::Abs(evt->dtc) < course_time_limit) {
                    if (TMath::Abs(evt->dtf) < FINELIMIT) {
                        checkevts++;
                        apdoffset[panel][evt->cartridge1][evt->fin1][evt->m1][evt->apd1]->Fill(evt->dtf);
                    }
                }
            }
        }
    }
    // loop over entries

    if (verbose) {
        cout << " Done looping over entries " << endl;
        cout << " I made " << checkevts << " calls to Fill() " << endl;
    }

    Float_t mean_apdoffset[SYSTEM_PANELS][CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE];

    // Calculate and print the calibration parameters for the left side
    string calpar_filename_left(filebase + ".calpar_0.txt");
    if (verbose) {
        cout << "Writing Calibration Parameters to: "
             << calpar_filename_left << endl;
    }
    ofstream calpars_left;
    calpars_left.open(calpar_filename_left.c_str());
    for (int cartridge = 0; cartridge < CARTRIDGES_PER_PANEL; cartridge++) {
        for (int fin=0; fin < FINS_PER_CARTRIDGE; fin++) {
            for (int mod=0; mod<MODULES_PER_FIN; mod++) {
                for (int apd=0; apd<APDS_PER_MODULE; apd++) {
                    mean_apdoffset[panel][cartridge][fin][mod][apd] = calfunc(
                            apdoffset[panel][cartridge][fin][mod][apd],
                            fit_apdoffset[panel][cartridge][fin][mod][apd],
                            coarsetime,
                            usegausfit,
                            verbose);
                    calpars_left <<  std::setw(4) << mean_apdoffset[panel][cartridge][fin][mod][apd] << " ";
                }
            }
            calpars_left << endl;
        }
    }
    calpars_left.close();
    if (write_out_postscript_flag) {
        string ps_filename_left_cartridge(filebase + ".panel0_cartridge0.ps");
        drawmod(apdoffset[0][0], c1, ps_filename_left_cartridge.c_str());
    }

    if (verbose)  cout << " Filling crystal spectra on the right. " << endl;
    checkevts=0;


    // Using the left side data, generate the calibration parameters for the
    // right side.  Start here by filling histograms based on the panel 1 data
    panel = 1;
    for (Long64_t ii=0; ii < entries; ii++) {
        mm->GetEntry(ii);
        if (BoundsCheckEvent(*evt) == 0) {
            if (EnergyGateEvent(*evt, energy_gate_low, energy_gate_high) == 0) {
                if (TMath::Abs(evt->dtc) < course_time_limit) {
                    if (TMath::Abs(evt->dtf) < FINELIMIT) {
                        checkevts++;
                        apdoffset[1][evt->cartridge2][evt->fin2][evt->m2][evt->apd2]->Fill(
                                evt->dtf - mean_apdoffset[0][evt->cartridge1][evt->fin1][evt->m1][evt->apd1]);
                    }
                }
            }
        }
    }

    if (verbose){
        cout << " Done looping over entries " << endl;
        cout << " I made " << checkevts << " calls to Fill() " << endl;         
    }

    string calpar_filename_right(filebase + ".calpar_1.txt");
    if (verbose) {
        cout << "Writing Calibration Parameters to: "
             << calpar_filename_right << endl;
    }
    ofstream calpars_right;
    calpars_right.open(calpar_filename_right.c_str());
    for (int cartridge = 0; cartridge < CARTRIDGES_PER_PANEL; cartridge++) {
        for (int fin=0; fin < FINS_PER_CARTRIDGE; fin++) {
            for (int mod = 0; mod < MODULES_PER_FIN; mod++) {
                for (int apd = 0; apd < APDS_PER_MODULE; apd++) {
                    mean_apdoffset[panel][cartridge][fin][mod][apd] = calfunc(
                            apdoffset[panel][cartridge][fin][mod][apd],
                            fit_apdoffset[panel][cartridge][fin][mod][apd],
                            coarsetime,
                            usegausfit,
                            verbose);
                    calpars_right <<  std::setw(4) << mean_apdoffset[panel][cartridge][fin][mod][apd] << " ";
                }
            }
            calpars_right << endl;
        }
    }
    calpars_right.close();

    if (write_out_postscript_flag) {
        string ps_filename_right_cartridge(filebase + ".panel1_cartridge0.ps");
        drawmod(apdoffset[1][0],c1,ps_filename_right_cartridge.c_str());
    }


    AddPerApdAsPerCrystalCal(crystal_cal, mean_apdoffset, crystal_cal);
    if (write_per_crystal_correction) {
        int write_status(WritePerCrystalCal(output_crystal_cal_filename,
                                            crystal_cal));
        if (write_status < 0) {
            cerr << "Write out of crystal calibration failed." << endl;
        }
    }


    // Now using these parameters, generate the [filename].apdoffcal.root file
    // with the fine timestamp calibrated by subtracting the mean_apdoffset for
    // each apd involved in the interaction
    TH1F *tres = new TH1F("tres","Time Resolution After Time walk correction",100,-25,25);

    CoincEvent *calevt = new CoincEvent();
    TFile * calfile(0);
    TTree * merged(0);
    if (write_out_root_file_flag) {
        if (verbose) cout << " Opening file " << rootfile << " for writing " << endl;
        calfile = new TFile(rootfile.c_str(),"RECREATE");
        merged = new  TTree("merged","Merged and Calibrated LYSO-PSAPD data ");
        merged->Branch("Event", &calevt);
    }

    checkevts=0;
    if (verbose) cout << "filling new Tree :: " << endl;

    for (Long64_t ii=0; ii<entries; ii++) {
        mm->GetEntry(ii);
        if (BoundsCheckEvent(*evt) == 0) {
            calevt = evt;
            calevt->dtf -= mean_apdoffset[0][evt->cartridge1][evt->fin1][evt->m1][evt->apd1];
            calevt->dtf -= mean_apdoffset[1][evt->cartridge2][evt->fin2][evt->m2][evt->apd2]; 
            // Energy gate to put only the photopeaks in the fine time
            // stamp histogram for the time calibration calculation
            if (EnergyGateEvent(*evt, energy_gate_low, energy_gate_high) == 0) {
                tres->Fill(calevt->dtf);
            }
            if (write_out_root_file_flag) {
                merged->Fill();
                checkevts++;
            }
        }
    }
    cout << " New Tree filled with " << checkevts << " events. " << endl;

    if (write_out_root_file_flag) {
        merged->Write();
        calfile->Close();
    }

    // Fit the fine timestamp histogram
    tres->Fit("gaus","","",-10,10);
    if (write_out_time_res_plot_flag) {
        // Then output the histogram with it's fit to a postscript file
        c1->Clear();
        tres->Draw();
        string ps_tres_filename(filebase + ".tres.apdoffset.ps");
        c1->Print(ps_tres_filename.c_str());
    }
    return(0);
}


