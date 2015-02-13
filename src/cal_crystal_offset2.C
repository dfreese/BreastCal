#include "TROOT.h"
#include "TStyle.h"
#include "Riostream.h"
#include "TTree.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TSpectrum.h"
#include "TSpectrum2.h"
#include "TH2F.h"
#include "TVector.h"
#include "TMath.h"
#include "TSpectrum.h"
#include "TF1.h"
#include "decoder.h"
#include "CoincEvent.h"
#include "string.h"
#include "cal_helper_functions.h"

//#define DEBUG2


void usage() {
    cout << "cal_crystal_offset2 [-v -h] -f [Filename]\n"
         << "\n"
         << "Options:\n"
         << "  -n:  do not write out the resulting root file\n"
         << "  -dp:  do not print out any postscript files\n"
         << "  -pto:  only write out the time resolution plot\n"
         << "  -log:  print the time resolution plot using a log scale\n"
         << "  -gf:  use a gaussian fit for the timing calibration\n"
         << "      default: peak search\n"
         << "  -rcal (filename):  read in initial per crystal calibration\n"
         << "  -wcal (filename):  write out per crystal calibration\n"
         << "  -ft (limit ns):  use fine timestamp limit for calibration\n"
         << "      default: use limit +/-100ns\n"
         << "  -redcal (filename): read in an energy dependence file\n";
}


#define MINMODENTRIES 1000

Float_t cryscalfunc(
        TH1F *hist,
        Bool_t gausfit,
        Int_t verbose)
{
    if (gausfit) {
        if (hist->GetEntries() > 70) {
            TF1 * fitfun = fitgaus(hist,0);
            return(fitfun->GetParameter(1));
        } else if (hist->GetEntries() > 50) {
                return hist->GetMean();
        } else {
            return(0.0);
        }
    } else {
        // just peak search 
        TSpectrum *s = new TSpectrum();
        Int_t npeaks = s->Search(hist,3,"",0.2);
        float offset(0);
        if (npeaks) {
            offset = getmax(s,npeaks,verbose);
        }
        delete s;
        return(offset);
    }
}

int main(int argc, Char_t *argv[])
{
    if (argc == 1) {
        usage();
        return(0);
    }
    // A flag that is used by the crystal calibration function to determine
    // wether a fit algorithm or a peak searching algorithm should be used
    // in determining the calibration parameter for that crystal
    Bool_t usegausfit=0;

    // The fine timestamp limit does not eliminate any events from the root
    // file that is generated as a result of this program.  This sets the limit
    // for the fine timestamp difference that can be use
    Int_t FINELIMIT(100);

    string filename;

    bool read_per_crystal_correction(false);
    bool write_per_crystal_correction(false);
    string input_crystal_cal_filename;
    string output_crystal_cal_filename;

    bool read_per_crystal_energy_correction(false);
    string input_per_crystal_energy_cal_filename;

    Int_t verbose = 0;
    // Parameter for determining if the fine time stamp histogram generated
    // after calibration should be plotted in a log scale or not.  Default
    // is to not use a log scale
    bool write_out_log_time_res_plot_flag(false);

    // A flag that is true unless a -n option is found in which case only the
    // calibration parameters and the associated plots are generated, but the
    // calibrated root file is not created.
    bool write_out_root_file_flag(true);
    // A flag that disables print outs of postscript files.  Default is on.
    // Flag is disabled by -dp.
    bool write_out_postscript_flag(true);
    bool write_out_time_res_plot_flag(true);

    float energy_gate_low(400);
    float energy_gate_high(600);

    for(int ix = 1; ix < argc; ix++) {
        if(strcmp(argv[ix], "-v") == 0) {
            cout << "Verbose Mode " << endl;
            verbose = 1;
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
        if(strcmp(argv[ix], "-log") == 0) {
            cout << "Output Graph in Log Scale " << endl;
            write_out_log_time_res_plot_flag = true;
        }
        if(strcmp(argv[ix], "-gf") == 0) {
            usegausfit=1;
            cout << " Using Gauss Fit "  <<endl;
        }
    }

    for(int ix = 1; ix < (argc - 1); ix++) {
        if(strcmp(argv[ix], "-rcal") == 0) {
            read_per_crystal_correction = true;
            input_crystal_cal_filename = string(argv[ix + 1]);
        }
        if(strcmp(argv[ix], "-wcal") == 0) {
            write_per_crystal_correction = true;
            output_crystal_cal_filename = string(argv[ix + 1]);
        }
        if(strcmp(argv[ix], "-redcal") == 0) {
            read_per_crystal_energy_correction = true;
            input_per_crystal_energy_cal_filename = string(argv[ix + 1]);
        }
        if(strcmp(argv[ix], "-ft") == 0) {
            FINELIMIT = atoi(argv[ix + 1]);
            if (FINELIMIT < 1) {
                cout << " Error. FINELIMIT = " << FINELIMIT
                     << " too small. Please specify -ft [finelimit]. " << endl;
                cout << "Exiting." << endl;
                return(-20);
            }
            cout << " Fine time interval = "  << FINELIMIT << endl;
        }
        if(strcmp(argv[ix], "-f") == 0) {
            filename = string(argv[ix + 1]);
        }
    }

    if (!verbose) {
        gErrorIgnoreLevel = kError;
    }

    rootlogon(verbose);
    gStyle->SetOptStat(kTRUE); 

    TCanvas *c1;
    c1 = (TCanvas*)gROOT->GetListOfCanvases()->FindObject("c1");
    if (!c1) c1 = new TCanvas("c1","c1",10,10,1000,1000);
    c1->SetCanvasSize(700,700);


    size_t root_file_ext_pos(filename.rfind(".root"));
    if (root_file_ext_pos == string::npos) {
        cerr << "Unable to find .root extension in: \""
             << filename << "\"" << endl;
        cerr << "...Exiting." << endl;
        return(-1);
    }
    string filebase(filename, 0, root_file_ext_pos);
    if (verbose) cout << "filebase: " << filebase << endl;
    string rootfile(filebase + ".crystaloffcal.root");
    if (verbose) cout << "ROOTFILE = " << rootfile << endl;



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
                input_crystal_cal_filename, crystal_cal));
        if (cal_read_status < 0) {
            cerr << "Error in reading input energy calibration file: "
                 << cal_read_status << endl;
            cerr << "Exiting.." << endl;
            return(-3);
        }
    }


    if (verbose) {
        cout << "Reading Input crystal energy calibration file\n";
    }
    float crystal_edep_cal[SYSTEM_PANELS]
            [CARTRIDGES_PER_PANEL]
            [FINS_PER_CARTRIDGE]
            [MODULES_PER_FIN]
            [APDS_PER_MODULE]
            [CRYSTALS_PER_APD]
            [2] = {{{{{{{0}}}}}}};

    if (read_per_crystal_energy_correction) {
        int cal_read_status(ReadPerCrystalEnergyCal(
                input_per_crystal_energy_cal_filename, crystal_edep_cal));
        if (cal_read_status < 0) {
            cerr << "Error in reading input calibration file: "
                 << cal_read_status << endl;
            cerr << "Exiting.." << endl;
            return(-4);
        }
    }


    cout << " Opening file " << filename << endl;
    TFile *rtfile = new TFile(filename.c_str(),"OPEN");
    TTree *mm  = (TTree *) rtfile->Get("merged");
    CoincEvent *evt =  new CoincEvent();
    mm->SetBranchAddress("Event",&evt);


    TH1F *crystaloffset[SYSTEM_PANELS]
            [CARTRIDGES_PER_PANEL]
            [FINS_PER_CARTRIDGE]
            [MODULES_PER_FIN]
            [APDS_PER_MODULE]
            [CRYSTALS_PER_APD];


    for (int panel = 0; panel < SYSTEM_PANELS; panel++) {
        for (int cartridge = 0; cartridge < CARTRIDGES_PER_PANEL; cartridge++) {
            for (int fin = 0; fin < FINS_PER_CARTRIDGE; fin++) {
                for (int module = 0; module < MODULES_PER_FIN; module++) {
                    for (int apd = 0; apd < APDS_PER_MODULE; apd++) {
                        for (int crystal = 0; crystal < CRYSTALS_PER_APD; crystal++) {
                            Char_t histtitle[40];
                            sprintf(histtitle,"crystaloffset[%d][%d][%d][%d][%d][%d]",
                                    panel, cartridge, fin, module, apd, crystal);
                            crystaloffset[panel][cartridge][fin][module][apd][crystal] = 
                                new TH1F(histtitle,histtitle,50, -FINELIMIT, FINELIMIT);
                        }
                    }
                }
            }
        }
    }

    Long64_t entries = mm->GetEntries();
    cout << " Total entries: " << entries << endl; 


    if (verbose) {
        cout << " Filling crystal spectra on the left. " << endl;
    }

    Long64_t checkevts(0);


    for (Long64_t ii = 0; ii<entries; ii++) {
        mm->GetEntry(ii);
        if (BoundsCheckEvent(*evt) == 0) {
            if (EnergyGateEvent(*evt, energy_gate_low, energy_gate_high) == 0) {
                if (TMath::Abs(evt->dtc ) < 6) {
                    if (TMath::Abs(evt->dtf ) < FINELIMIT) {
                        checkevts++;
                        crystaloffset[0][evt->cartridge1][evt->fin1][evt->m1]
                                [evt->apd1][evt->crystal1]->Fill(
                                    evt->dtf
                                    - GetEventOffset(*evt, crystal_cal)
                                    - GetEventOffsetEdep(*evt,
                                                         crystal_edep_cal));
                    }
                }
            }
        }
    }

    if (verbose){
        cout << " Done looping over entries " << endl;
        cout << " I made " << checkevts << " calls to Fill() " << endl;
    }

    float mean_crystaloffset[SYSTEM_PANELS]
            [CARTRIDGES_PER_PANEL]
            [FINS_PER_CARTRIDGE]
            [MODULES_PER_FIN]
            [APDS_PER_MODULE]
            [CRYSTALS_PER_APD]={{{{{{0}}}}}};

    for (int cartridge = 0; cartridge < CARTRIDGES_PER_PANEL; cartridge++) {
        for (int fin = 0; fin < FINS_PER_CARTRIDGE; fin++) {
            for (int module = 0; module < MODULES_PER_FIN; module++) {
                for (int apd = 0; apd < APDS_PER_MODULE; apd++) {
                    for (int crystal = 0; crystal < CRYSTALS_PER_APD; crystal++) {
                        mean_crystaloffset[0][cartridge][fin][module][apd][crystal] =
                            cryscalfunc(crystaloffset[0][cartridge][fin][module]
                                        [apd][crystal], usegausfit,verbose);
                    }
                }
            }
        }
    }


    if (write_out_postscript_flag) {
        for (int fin = 0; fin < FINS_PER_CARTRIDGE; fin++) {
            stringstream ps_fin_filename;
            ps_fin_filename << filebase << ".panel0_cartridge0_fin" << fin << ".ps";
            if (verbose) {
                cout << "Writing fits to: " << ps_fin_filename.str() << endl;
            }
            drawmod_crys2(crystaloffset[0][0][fin],c1,ps_fin_filename.str());
        }
    }

    string calpar_left_filename(filebase + ".calpar_0.txt");
    if (verbose) {
        cout << "Writing left cal params to: " << calpar_left_filename << endl;
    }
    writval(mean_crystaloffset[0],calpar_left_filename.c_str());

    if (verbose) cout << " Filling crystal spectra on the right. " << endl;
    checkevts=0;


    for (Long64_t ii = 0; ii < entries; ii++) {
        mm->GetEntry(ii);
        if (BoundsCheckEvent(*evt) == 0) {
            if (EnergyGateEvent(*evt, energy_gate_low, energy_gate_high) == 0) {
                if (TMath::Abs(evt->dtc ) < 6) {
                    if (TMath::Abs(evt->dtf) < FINELIMIT) {
                        checkevts++;
                        crystaloffset[1][evt->cartridge2][evt->fin2][evt->m2][evt->apd2][evt->crystal2]->Fill(
                                evt->dtf - GetEventOffset(*evt, crystal_cal)
                                - mean_crystaloffset[0][evt->cartridge1]
                                                    [evt->fin1][evt->m1]
                                                    [evt->apd1][evt->crystal1]
                                - GetEventOffsetEdep(*evt,
                                                     crystal_edep_cal));
                    }
                }
            }
        }
    }

    if (verbose) {
        cout << " Done looping over entries " << endl;
        cout << " I made " << checkevts << " calls to Fill() " << endl;
    }

    for (int cartridge = 0; cartridge < CARTRIDGES_PER_PANEL; cartridge++) {
        for (int fin = 0; fin < FINS_PER_CARTRIDGE; fin++) {
            for (int module = 0; module < MODULES_PER_FIN; module++) {
                for (int apd = 0; apd < APDS_PER_MODULE; apd++) {
                    for (int crystal = 0; crystal < CRYSTALS_PER_APD; crystal++) {
                        mean_crystaloffset[1][cartridge][fin][module][apd][crystal] = 
                            cryscalfunc(crystaloffset[1][cartridge][fin][module][apd][crystal], usegausfit,verbose);
                    }
                }
            }
        }
    }

    string calpar_right_filename(filebase + ".calpar_1.txt");
    if (verbose) {
        cout << "Writing right cal params to: " << calpar_right_filename << endl;
    }
    writval(mean_crystaloffset[1], calpar_right_filename);


    AddPerCrystalCal(crystal_cal, mean_crystaloffset, crystal_cal);
    if (write_per_crystal_correction) {
        int write_status(WritePerCrystalCal(output_crystal_cal_filename,
                                            crystal_cal));
        if (write_status < 0) {
            cerr << "Write out of crystal calibration failed." << endl;
        }
    }


    TH1F *tres(0);
    if (write_out_time_res_plot_flag) {
        tres = new TH1F("tres",
                        "Time Resolution After Time walk correction",
                        400, -100, 100);
    }

    TFile *calfile(0);
    TTree *merged(0);
    CoincEvent *calevt = new CoincEvent();
    if (write_out_root_file_flag) {
        if (verbose) cout << " Opening file " << rootfile << " for writing " << endl;
        calfile = new TFile(rootfile.c_str(),"RECREATE");
        merged = new  TTree("merged","Merged and Calibrated LYSO-PSAPD data ");
        merged->Branch("Event",&calevt);
    }

    if (verbose) cout << "filling new Tree :: " << endl;

    for (Long64_t ii=0; ii<entries; ii++) {
        mm->GetEntry(ii);
        calevt = evt;
        if (BoundsCheckEvent(*evt) == 0) {
            calevt->dtf -= mean_crystaloffset[0][evt->cartridge1][evt->fin1][evt->m1][evt->apd1][evt->crystal1];
            calevt->dtf -= mean_crystaloffset[1][evt->cartridge2][evt->fin2][evt->m2][evt->apd2][evt->crystal2];
            calevt->dtf -= GetEventOffset(*evt, crystal_cal);
            calevt->dtf -= GetEventOffsetEdep(*evt, crystal_edep_cal);
            if (EnergyGateEvent(*evt, energy_gate_low, energy_gate_high) == 0) {
                if (write_out_time_res_plot_flag) {
                    tres->Fill(calevt->dtf);
                }
            }
        }
        if (write_out_root_file_flag) {
            merged->Fill();
        }
    }
    if (write_out_root_file_flag) {
        merged->Write();
        calfile->Close();
    }


    if (write_out_time_res_plot_flag) {
        tres->Fit("gaus","","",-10,10);
        c1->Clear();
        tres->Draw();
        string ps_tres_filename(filebase + ".tres.crysoffset.ps");
        if (verbose) {
            cout << "Writing tres histogram to: " << ps_tres_filename << endl;
        }
        c1->Print(ps_tres_filename.c_str());

        if (write_out_log_time_res_plot_flag) {
            c1->SetLogy();
            string ps_tres_log_filename(filebase + ".tres_log.crysoffset.ps");
            if (verbose) {
                cout << "Writing log tres histogram to: " << ps_tres_log_filename << endl;
            }
            c1->Print(ps_tres_log_filename.c_str());
        }
    }
    return(0);
}

