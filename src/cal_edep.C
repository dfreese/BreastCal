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
#include <sstream>

void usage() {
    cout << "cal_edep [-v -h] -f [Filename]\n"
         << "\n"
         << "Options:\n"
         << "  -n:  do not write out the resulting root file\n"
         << "  -dp:  do not print out any postscript files\n"
         << "  -pto:  only write out the time resolution plot\n"
         << "  -c:  use common channel energy for calibration\n"
         << "  -rcal (filename):  read in initial per crystal calibration\n"
         << "  -ft (limit ns):  use fine timestamp limit for calibration\n"
         << "      default: use limit +/-40ns\n"
         << "  -wedcal (filename): write out the energy dependence to a file\n";
}

int main(int argc, Char_t *argv[]) {
    if (argc == 1) {
        usage();
        return(0);
    }

    Int_t verbose = 0;

    // The limit that is put on events before they are placed into histograms.
    // This does not effect what events are passed to the resulting root file.
    int dtf_difference_limit(40);
    // A flag to indicate that the common channel energy spectrum should be
    // used for the energy dependence calibration.  The default is to use the
    // spatial energy spectrum
    bool use_common_channel_energy_flag(false);

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


    bool read_per_crystal_correction(false);
    string input_crystal_cal_filename;

    bool write_per_crystal_energy_correction(false);
    string output_per_crystal_energy_cal_filename;

    string filename;

    // Flags not requiring input
    for (int ix = 1; ix < argc; ix++) {
        if(strncmp(argv[ix], "-h", 2) == 0) {
            usage();
            return(0);
        }
        if(strncmp(argv[ix], "--help", 2) == 0) {
            usage();
            return(0);
        }
        if (strncmp(argv[ix], "-v", 2) == 0) {
            cout << "Verbose Mode " << endl;
            verbose = 1;
        }
        if (strncmp(argv[ix], "-n", 2) == 0) {
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
        if(strcmp(argv[ix], "-c") ==0 ) {
            cout << " Using common for energy dependence " << endl ;
            use_common_channel_energy_flag = true;
        }
    }

    // Flags requiring input
    for (int ix = 1; ix < (argc - 1); ix++) {
        if(strcmp(argv[ix], "-rcal") == 0) {
            read_per_crystal_correction = true;
            input_crystal_cal_filename = string(argv[ix + 1]);
        }
        if(strcmp(argv[ix], "-f") == 0) {
            filename = string(argv[ix + 1]);
        }
        if(strcmp(argv[ix], "-ft") == 0) {
            dtf_difference_limit = atoi(argv[ix + 1]);
            if (dtf_difference_limit < 1) {
                cout << " Error. FINELIMIT = " << dtf_difference_limit
                     << " too small. Please specify -ft [finelimit]. " << endl;
                cout << "Exiting." << endl;
                return(-20);
            }
            cout << " Fine time interval = "  << dtf_difference_limit << endl;
        }
        if(strcmp(argv[ix], "-wedcal") == 0) {
            write_per_crystal_energy_correction = true;
            output_per_crystal_energy_cal_filename = string(argv[ix + 1]);
        }
    }
    rootlogon(verbose);
    gStyle->SetOptStat(kTRUE);

    TCanvas *c1;
    c1 = (TCanvas*)gROOT->GetListOfCanvases()->FindObject("c1");
    if (!c1) {
        c1 = new TCanvas("c1","c1",10,10,1000,1000);
    }
    c1->SetCanvasSize(700,700);


    size_t root_file_ext_pos(filename.rfind(".root"));
    if (root_file_ext_pos == string::npos) {
        cerr << "Unable to find .root extension in: \"" << filename << "\"" << endl;
        cerr << "...Exiting." << endl;
        return(-1);
    }
    string filebase(filename, 0, root_file_ext_pos);
    if (verbose) cout << "filebase: " << filebase << endl;
    string rootfile(filebase + ".edepcal.root");
    if (verbose) cout << "ROOTFILE = " << rootfile << endl;


    if (verbose) {
        cout << "Reading input crystal calibration file\n";
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
            return(-3);
        }
    }
    if (verbose) {
        cout << "Finished reading input crystal calibration file\n";
    }


    cout << " Opening file " << filename << endl;
    TFile *rtfile = new TFile(filename.c_str());
    if (rtfile->IsZombie()) {
        cerr << "Failed to open file: " << filename << endl;
        cerr << "Exiting" << endl;
        return(-4);
    }

    TTree *mm  = (TTree *) rtfile->Get("merged");
    if (!mm) {
        cerr << "Failed to open tree \"merged\" in: \""
             << filename << "\""
             << endl;
        cerr << "Exiting" << endl;
        return(-4);
    }
    CoincEvent * evt = new CoincEvent();
    mm->SetBranchAddress("Event",&evt);

    TH2F *energydependence[SYSTEM_PANELS][CARTRIDGES_PER_PANEL];
    for (int p = 0; p < SYSTEM_PANELS; p++) {
        for (int c = 0; c < CARTRIDGES_PER_PANEL; c++) {
            stringstream name_stream;
            name_stream << "energydependence[" << p << "][" << c << "]";
            stringstream title_stream;
            title_stream << "Energy Dependence P" << p << "C" << c;
            energydependence[p][c] = new TH2F(
                    name_stream.str().c_str(), title_stream.str().c_str(),
                    100, energy_gate_low, energy_gate_high, 100, -50, 50);
        }
    }

    Long64_t entries = mm->GetEntries();
    cout << " Total  entries: " << entries << endl;


    if (verbose) {
        cout << " Filling crystal spectra on the left. " << endl;
    }

    Long64_t checkevts = 0;
    for (Long64_t ii = 0; ii < entries; ii++) {
        mm->GetEntry(ii);
        if (BoundsCheckEvent(*evt) == 0) {
            if (EnergyGateEvent(*evt, energy_gate_low, energy_gate_high) == 0) {
                if (TMath::Abs(evt->dtc) < 6) {
                    if (TMath::Abs(evt->dtf ) < dtf_difference_limit) {
                        checkevts++;
                        float energy(evt->E1);
                        if (use_common_channel_energy_flag) {
                            energy = evt->Ec1;
                        }
                        energydependence[0][evt->cartridge1]->Fill(
                                    energy,
                                    evt->dtf - GetEventOffset(*evt, crystal_cal));
                    }
                }
            }
        }
    }

    if (verbose) {
        cout << " Done looping over entries " << endl;
        cout << " I made " << checkevts << " calls to Fill() " << endl;
    }

    TH1F *profehist[SYSTEM_PANELS][CARTRIDGES_PER_PANEL];
    TF1 *profehistfit[SYSTEM_PANELS][CARTRIDGES_PER_PANEL];

    for (int p = 0; p < SYSTEM_PANELS; p++) {
        for (int c = 0; c < CARTRIDGES_PER_PANEL; c++) {
            stringstream hist_name_stream;
            hist_name_stream << "profehistfit[" << p << "][" << c << "]";

            profehistfit[p][c] = new TF1(
                    hist_name_stream.str().c_str(),"pol1",400,600);
        }
    }

    for (int p = 0; p < 1; p++) {
        for (int c = 0; c < CARTRIDGES_PER_PANEL; c++) {
            stringstream fit_name_stream;
            fit_name_stream << "profehist[" << p << "][" << c << "]";
            profehist[p][c] = (TH1F *) energydependence[p][c]->ProfileX();
            profehist[p][c]->SetName(fit_name_stream.str().c_str());

            if (verbose) {
                profehist[p][c]->Fit(profehistfit[p][c]);
            } else {
                profehist[p][c]->Fit(profehistfit[p][c], "Q");
            }
        }
    }

    if (verbose) {
        cout << " Filling crystal spectra on the right. " << endl;
    }
    checkevts = 0;
    for (Long64_t ii = 0; ii < entries; ii++) {
        mm->GetEntry(ii);
        if (BoundsCheckEvent(*evt) == 0) {
            if (EnergyGateEvent(*evt, energy_gate_low, energy_gate_high) == 0) {
                if (TMath::Abs(evt->dtc) < 6) {
                    if (TMath::Abs(evt->dtf) < dtf_difference_limit) {
                        checkevts++;
                        float energy(evt->E2);
                        if (use_common_channel_energy_flag) {
                            energy = evt->Ec2;
                        }
                        energydependence[1][evt->cartridge2]->Fill(
                                    evt->Ec2,
                                    evt->dtf
                                    - profehistfit[0][evt->cartridge1]->Eval(energy)
                                    - GetEventOffset(*evt, crystal_cal));
                    }
                }
            }
        }
    }

    if (verbose) {
        cout << " Done looping over entries " << endl;
        cout << " I made " << checkevts << " calls to Fill() " << endl;
    }

    for (int p = 1; p < 2; p++) {
        for (int c = 0; c < CARTRIDGES_PER_PANEL; c++) {
            stringstream fit_name_stream;
            fit_name_stream << "profehist[" << p << "][" << c << "]";
            profehist[p][c] = (TH1F *) energydependence[p][c]->ProfileX();
            profehist[p][c]->SetName(fit_name_stream.str().c_str());

            if (verbose) {
                profehist[p][c]->Fit(profehistfit[p][c]);
            } else {
                profehist[p][c]->Fit(profehistfit[p][c], "Q");
            }
        }
    }

    if (write_out_postscript_flag) {
        for (int p = 0; p < SYSTEM_PANELS; p++) {
            for (int c = 0; c < CARTRIDGES_PER_PANEL; c++) {
                c1->Clear();
                c1->Divide(1,2);
                c1->cd(1);
                energydependence[p][c]->Draw("colz");
                c1->cd(2);
                profehist[p][c]->Draw();
                stringstream ps_name_stream;
                ps_name_stream << "edep_P" << p << "C" << c << ".ps";
                c1->Print(ps_name_stream.str().c_str());
            }
        }
    }


    TH1F * tres(0);
    if (write_out_time_res_plot_flag) {
        tres = new TH1F("tres","Time Resolution After Time walk correction",100,-25,25);
    }
    if (verbose) {
        cout << " Opening file " << rootfile << " for writing " << endl;
    }
    CoincEvent * calevt = new CoincEvent();
    TFile *calfile(0);
    TTree *merged(0);
    if (write_out_root_file_flag) {
        calfile = new TFile(rootfile.c_str(),"RECREATE");
        merged = new  TTree("merged","Merged and Calibrated LYSO-PSAPD data ");
        merged->Branch("Event",&calevt);
    }

    if (verbose) {
        cout << "filling new Tree :: " << endl;
    }
    checkevts = 0;
    for (Long64_t ii = 0; ii < entries; ii++) {
        mm->GetEntry(ii);
        if (BoundsCheckEvent(*evt) == 0) {
            calevt=evt;
            float energy_panel1(evt->E1);
            float energy_panel2(evt->E2);
            if (use_common_channel_energy_flag) {
                energy_panel1 = evt->Ec1;
                energy_panel2 = evt->Ec2;
            }
            calevt->dtf -= profehistfit[0][evt->cartridge1]->Eval(energy_panel1);
            calevt->dtf -= profehistfit[1][evt->cartridge2]->Eval(energy_panel2);
            calevt->dtf -= GetEventOffset(*calevt, crystal_cal);

            if (write_out_time_res_plot_flag) {
                if (EnergyGateEvent(*evt, energy_gate_low, energy_gate_high) == 0) {
                    tres->Fill(calevt->dtf);
                }
            }
        }
        checkevts++;
        if (write_out_root_file_flag) {
            merged->Fill();
        }
    }
    if (write_out_root_file_flag) {
        merged->Write();
        calfile->Close();
    }

    if (verbose) {
        cout << " Done looping over entries " << endl;
        cout << " I made " << checkevts << " calls to Fill() " << endl;
    }

    if (write_per_crystal_energy_correction) {
        float crystal_energy_correction[SYSTEM_PANELS]
                                [CARTRIDGES_PER_PANEL]
                                [2] = {{{0}}};
        for (int panel = 0; panel < SYSTEM_PANELS; panel++) {
            for (int cart = 0; cart < CARTRIDGES_PER_PANEL; cart++) {
                crystal_energy_correction[panel][cart][0] =
                        profehistfit[panel][cart]->GetParameter(0);
                crystal_energy_correction[panel][cart][1] =
                        profehistfit[panel][cart]->GetParameter(1);
            }
        }
        int write_status(WritePerCartridgeAsPerCrystalEnergyCal(
                             output_per_crystal_energy_cal_filename,
                             crystal_energy_correction));
        if (write_status < 0) {
            cerr << "Error: Writing out per crystal energy calibration"
                 << " failed\n";
        }
    }

    if (write_out_time_res_plot_flag) {
        tres->Fit("gaus","","",-10,10);
        c1->Clear();
        tres->Draw();
        string ps_tres_filename(filename + ".edepcal.tres.ps");
        c1->Print(ps_tres_filename.c_str());

        if (verbose) {
            cout << tres->GetEntries() << " Entries in tres." << endl;
        }
    }

    return(0);
}
