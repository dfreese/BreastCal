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

/*!
 * This function generates a postscript file containing all 64 histograms
 * for each crystal position with their fits.
 *
 */
Int_t draw_crystal_histograms(
        TH1F *hi[CRYSTALS_PER_APD],
        TCanvas *ccc,
        string filename)
{
    // I'm assuming that the parentheses are something to do with how
    // root handles multi-page postscript files.
    string filename_open(filename + "(");
    string filename_close(filename + ")");

    ccc->Clear();
    ccc->Divide(4, 4);

    for (int ii = 0; ii < 4; ii++) {
        for (int jj = 0; jj < 16; jj++) {
            ccc->cd(jj+1);
            hi[ii*16+jj]->Draw("E");
        }
        if (ii == 0) {
            ccc->Print(filename_open.c_str());
        } else if (ii == 3) {
            ccc->Print(filename_close.c_str());
        } else {
            ccc->Print(filename.c_str());
        }
        ccc->Clear();
        ccc->Divide(4, 4);
    }
    return(0);
}

Float_t cryscalfunc(
        TH1F *hist,
        bool gausfit,
        Int_t verbose)
{
    if (gausfit) {
        if (hist->GetEntries() > 70) {     
            TF1 * fitfun = fitgaus(hist,0) ;
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

/*!
 * A function for writing out the global per crystal postion calibration
 * parameters.
 */
int write_crystal_cal_val(
        float mean_crystaloffset[CRYSTALS_PER_APD],
        std::string outfile)
{
    ofstream parfile;
    parfile.open(outfile.c_str());
    if (parfile.good()) {
        for (int crystal = 0; crystal < CRYSTALS_PER_APD; crystal++) {
            parfile << setw(6) << setprecision(3) 
                    << mean_crystaloffset[crystal] << " ";
            if ((crystal % 8) == 7) {
                parfile << endl;
            }
        }
        parfile << endl;
    } else {
        return(-1);
    }
    parfile.close();
    return(0);
}

int main(int argc, Char_t *argv[])
{ 
    bool use_gaussian_fit_flag(false);

    double dtf_difference_limit(100);

    float energy_gate_low(400);
    float energy_gate_high(600);

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


    string filename;
    Int_t verbose = 0;
    CoincEvent *evt =  new CoincEvent();
    // Parameter for determining if the fine time stamp histogram generated
    // after calibration should be plotted in a log scale or not.  Default
    // is to not use a log scale
    bool write_out_log_time_res_plot_flag(false);

    // Flags not requiring input
    for(int ix = 1; ix < argc; ix++) {
        if(strcmp(argv[ix], "-v") == 0) {
            cout << "Verbose Mode " << endl;
            verbose = 1;
        }
        if(strcmp(argv[ix], "-log") == 0) {
            cout << "Output Graph in Log Scale " << endl;
            write_out_log_time_res_plot_flag = true;
        }
        if(strcmp(argv[ix], "-gf") == 0) {
            use_gaussian_fit_flag = true;
            cout << " Using Gauss Fit "  <<endl;
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
    }

    // Flags requiring input
    for(int ix = 1; ix < (argc - 1); ix++) {
        if(strcmp(argv[ix], "-v") == 0) {
            cout << "Verbose Mode " << endl;
            verbose = 1;
        }
        if(strcmp(argv[ix], "-log") == 0) {
            cout << "Output Graph in Log Scale " << endl;
            write_out_log_time_res_plot_flag = true;
        }
        if(strcmp(argv[ix], "-gf") == 0) {
            use_gaussian_fit_flag=1;
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
        if(strcmp(argv[ix], "-f") == 0) {
            filename = string(argv[ix + 1]);
        }
        if(strcmp(argv[ix], "-ft") == 0) {
            dtf_difference_limit = atoi(argv[ix + 1]);
            if (dtf_difference_limit<1) {
                cerr << " Error. FINELIMIT = " << dtf_difference_limit
                     << " too small. Please specify -ft [finelimit]. " << endl;
                cerr << "Exiting." << endl;
                return(-2);
            }
            cout << " Fine time interval = "  << dtf_difference_limit << endl;
        }
    }

    if (!verbose) {
        gErrorIgnoreLevel = kError;
    }

    rootlogon(verbose);
    gStyle->SetOptStat(kTRUE); 

    size_t root_file_ext_pos(filename.rfind(".root"));
    if (root_file_ext_pos == string::npos) {
        cerr << "Unable to find .root extension in: \"" << filename << "\"" << endl;
        cerr << "...Exiting." << endl;
        return(-1);
    }
    string filebase(filename, 0, root_file_ext_pos);
    if (verbose) cout << "filebase: " << filebase << endl;


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

    TCanvas *c1;
    c1 = (TCanvas*)gROOT->GetListOfCanvases()->FindObject("c1");
    if (!c1) c1 = new TCanvas("c1","c1",10,10,1000,1000);
    c1->SetCanvasSize(700,700);

    cout << " Opening file " << filename << endl;
    TFile *rtfile = new TFile(filename.c_str(),"OPEN");
    if (rtfile->IsZombie()) {
        cerr << "Could not open file" << endl;
        cerr << "Exiting..." << endl;
        return(-1);
    }
    TTree *mm  = 0;
    mm  = (TTree *) rtfile->Get("merged");
    if (!mm) {
        cerr << "TTree not found in file" << endl;
        cerr << "Exiting..." << endl;
        return(-2);
    }
    mm->SetBranchAddress("Event", &evt);


    TH1F *crystaloffset[CRYSTALS_PER_APD];

    for (int crystal = 0; crystal < CRYSTALS_PER_APD; crystal++) {
        Char_t histtitle[40];
        sprintf(histtitle,"crystaloffset[%d]", crystal);
        crystaloffset[crystal] = new TH1F(histtitle,
                                          histtitle,
                                          50, -dtf_difference_limit, dtf_difference_limit);
    }

    Long64_t entries = mm->GetEntries();
    cout << " Total entries: " << entries << endl; 

    if (verbose) {
        cout << " Filling crystal spectra from both panels. " << endl;
    }

    Long64_t checkevts(0);
    for (Long64_t ii = 0; ii<entries; ii++) {
        mm->GetEntry(ii);
        if (BoundsCheckEvent(*evt) == 0) {
            if (EnergyGateEvent(*evt, energy_gate_low, energy_gate_high) == 0) {
                if (TMath::Abs(evt->dtc ) < 6 ) {
                    if (TMath::Abs(evt->dtf ) < dtf_difference_limit ) {
                        checkevts++;
                        crystaloffset[evt->crystal1]->Fill(evt->dtf);
                        crystaloffset[evt->crystal2]->Fill(0 - evt->dtf);
                    }
                }
            }
        }
    }

    if (verbose) {
        cout << " Done looping over entries " << endl;
        cout << " Calls to Fill(): " << checkevts << endl;
    }

    float mean_crystaloffset[CRYSTALS_PER_APD] = {0};

    for (int crystal = 0; crystal < CRYSTALS_PER_APD; crystal++) {
        mean_crystaloffset[crystal] = cryscalfunc(crystaloffset[crystal],
                                                  use_gaussian_fit_flag,
                                                  verbose);
    }

    string calpar_filename(filebase + "_calpar.txt");
    write_crystal_cal_val(mean_crystaloffset, calpar_filename);



    if (verbose) {
        cout << "Writing out new per crystal correction file\n";
    }
    AddPerCrystalLocationAsPerCrystalCal(crystal_cal,
                                         mean_crystaloffset,
                                         crystal_cal);
    if (write_per_crystal_correction) {
        int write_status(WritePerCrystalCal(output_crystal_cal_filename,
                                            crystal_cal));
        if (write_status < 0) {
            cerr << "Write out of crystal calibration failed." << endl;
        }
    }
    if (verbose) {
        cout << "Finished writing out new per crystal correction file\n";
    }



    TH1F *tres(0);
    if (write_out_time_res_plot_flag) {
        tres = new TH1F("tres",
                        "Time Resolution After Time walk correction",
                        400, -100, 100);
    }
    string rootfile(filebase + ".crystaloffcal.root");

    if (verbose) {
        cout << " Opening file " << rootfile << " for writing " << endl;
    }
    
    CoincEvent *calevt = new CoincEvent();
    TFile *calfile(0);
    TTree *merged(0);
    if (write_out_root_file_flag) {
        calfile = new TFile(rootfile.c_str(),"RECREATE");
        merged = new  TTree("merged","Merged and Calibrated LYSO-PSAPD data ");
        merged->Branch("Event", &calevt);
    }

    if (verbose) {
        cout << "filling new Tree :: " << endl;
    }
    for (Long64_t ii=0; ii<entries; ii++) {
        mm->GetEntry(ii);
        calevt = evt;
        if (BoundsCheckEvent(*evt) == 0) {
            calevt->dtf -= mean_crystaloffset[evt->crystal1];
            calevt->dtf += mean_crystaloffset[evt->crystal2];
            if (EnergyGateEvent(*evt, energy_gate_low, energy_gate_high) == 0) {
                tres->Fill(calevt->dtf);
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

    if (write_out_postscript_flag) {
        string postscript_filename(filename + ".crysoffset.ps");
        draw_crystal_histograms(crystaloffset, c1, postscript_filename);
    }

    if (write_out_time_res_plot_flag) {
        // Fit the fine timestamp histogram and print it out with the fit
        tres->Fit("gaus","","",-10,10);
        c1->Clear();
        tres->Draw();
        string time_res_fit_filename(filename + ".tres.crysoffset.ps");

        c1->Print(time_res_fit_filename.c_str());

        if (write_out_log_time_res_plot_flag) {
            c1->SetLogy();
            string time_res_fit_log_filename(filename +
                                             ".tres.crysoffset.log.ps");
            c1->Print(time_res_fit_log_filename.c_str());
        }
    }
    return(0);
}

