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
        Char_t filename[MAXFILELENGTH])
{
    // I'm assuming that the parentheses are something to do with how
    // root handles multi-page postscript files.
    Char_t  filename_open[MAXFILELENGTH+1];
    Char_t  filename_close[MAXFILELENGTH+1];
    strcpy(filename_open, filename);
    strcpy(filename_close, filename);
    strcat(filename_open, "(");
    strcat(filename_close, ")");

    ccc->Clear();
    ccc->Divide(4, 4);

    for (int ii = 0; ii < 4; ii++) {
        for (int jj = 0; jj < 16; jj++) {
            ccc->cd(jj+1);
            hi[ii*16+jj]->Draw("E");
        }
        if (ii == 0) {
            ccc->Print(filename_open);
        } else if (ii == 3) {
            ccc->Print(filename_close);
        } else {
            ccc->Print(filename);
        }
        ccc->Clear();
        ccc->Divide(4, 4);
    }
    return(0);
}

Float_t cryscalfunc(
        TH1F *hist,
        Bool_t gausfit,
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
    } else {    // just peak search 
        TSpectrum *s = new TSpectrum();
        Int_t npeaks = s->Search(hist,3,"",0.2);
        if (npeaks) {
            return getmax(s,npeaks,verbose);
        } else {
            return(0.0);
        }
    } // ! gausfit 
}

/*!
 * A function for writing out the global per crystal postion calibration
 * parameters.
 */
Int_t write_crystal_cal_val(
        Float_t mean_crystaloffset[CRYSTALS_PER_APD],
        Char_t outfile[MAXFILELENGTH])
{
    ofstream parfile;
    parfile.open(outfile);
    if (parfile.good()) {
        for (int crystal=0; crystal < CRYSTALS_PER_APD; crystal++) {
            parfile << setw(6) << setprecision(3) 
                    << mean_crystaloffset[crystal] << " ";
            if ((crystal % 8) == 7) {
                parfile << endl;
            }
        }
        parfile << endl;
    }
    parfile.close();
    return(0);
}

int main(int argc, Char_t *argv[])
{ 
    Bool_t usegausfit = 0;
    Int_t coarsetime = 1; 
    Char_t histtitle[40];

    Int_t DTF_low;
    Int_t DTF_hi;
    Int_t FINELIMIT;

    cout << " Welcome to cal_crystal_offset1" << endl;

    string filename;
    Int_t verbose = 0;
    CoincEvent *evt =  new CoincEvent();
    // Parameter for determining if the fine time stamp histogram generated
    // after calibration should be plotted in a log scale or not.  Default
    // is to not use a log scale
    Int_t logscale = 0; 

    for(int ix = 1; ix < argc; ix++) {
        if(strncmp(argv[ix], "-v", 2) == 0) {
            cout << "Verbose Mode " << endl;
            verbose = 1;
        }

        if(strncmp(argv[ix], "-log", 4) == 0) {
            cout << "Output Graph in Log Scale " << endl;
            logscale = 1;
        }

        if(strncmp(argv[ix], "-gf", 3) == 0) {
            usegausfit=1;
            cout << " Using Gauss Fit "  <<endl;
        }



        if(strncmp(argv[ix], "-f", 3) == 0) {
            if(strncmp(argv[ix], "-ft", 3) == 0) {
                coarsetime=0;
                ix++;
                if (ix == argc) {
                    cout << " Please enter finelimit interval: -ft [finelimit]\nExiting. " << endl;
                    return -20;
                }
                FINELIMIT=atoi(argv[ix]);
                if (FINELIMIT<1) {
                    cout << " Error. FINELIMIT = " << FINELIMIT << " too small. Please specify -ft [finelimit]. " << endl;
                    cout << "Exiting." << endl; return -20;
                }
                cout << " Fine time interval = "  << FINELIMIT << endl;
            } else {
                filename = string(argv[ix + 1]);
            }
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
    mm->SetBranchAddress("Event",&evt);


    TH1F *crystaloffset[CRYSTALS_PER_APD];

    if (coarsetime) {
        DTF_low = -100;
        DTF_hi = 100;
        FINELIMIT=100;
        cout << " Using Coarse limits: " << FINELIMIT << endl;
    } else {
        DTF_low = -FINELIMIT;
        DTF_hi = FINELIMIT;
    }

    for (int crystal = 0; crystal < CRYSTALS_PER_APD; crystal++) {
        sprintf(histtitle,"crystaloffset[%d]", crystal);
        crystaloffset[crystal] = new TH1F(histtitle,histtitle,50,DTF_low,DTF_hi);
    }

    Long64_t entries = mm->GetEntries();
    cout << " Total entries: " << entries << endl; 

    if (verbose) {
        cout << " Filling crystal spectra from both panels. " << endl;
    }

    Long64_t checkevts(0);




    for (Long64_t ii = 0; ii<entries; ii++) {
        mm->GetEntry(ii);
        if (evt->fin1>FINS_PER_CARTRIDGE) continue;
        if ((evt->crystal1<65) && 
                ((evt->apd1==0) || (evt->apd1==1)) && 
                (evt->m1<MODULES_PER_FIN))
        {
            if ((evt->E1>400) && (evt->E1<600)) {
                if (TMath::Abs(evt->dtc ) < 6 ) {
                    if (TMath::Abs(evt->dtf ) < FINELIMIT ) {
                        checkevts++;
                        crystaloffset[evt->crystal1]->Fill(evt->dtf);
                        // NEW
                        crystaloffset[evt->crystal2]->Fill(0-evt->dtf);
                    }
                }
            }
        }
    } // loop over entries

    if (verbose) {
        cout << " Done looping over entries " << endl;
        cout << " Calls to Fill(): " << checkevts << endl;
    }

    Float_t mean_crystaloffset[CRYSTALS_PER_APD] = {0};

    for (int crystal = 0; crystal < CRYSTALS_PER_APD; crystal++) {
        mean_crystaloffset[crystal] = cryscalfunc(crystaloffset[crystal], usegausfit, verbose);
    }

    Char_t calparfilename[MAXFILELENGTH];
    sprintf(calparfilename, "%s_calpar.txt", filebase.c_str());
    write_crystal_cal_val(mean_crystaloffset, calparfilename);

    TH1F *tres = new TH1F("tres","Time Resolution After Time walk correction",400,-100,100);

    string rootfile = filebase + ".crystaloffcal.root";

    if (verbose) {
        cout << " Opening file " << rootfile << " for writing " << endl;
    }
    
    TFile *calfile = new TFile(rootfile.c_str(),"RECREATE");
    TTree *merged = new  TTree("merged","Merged and Calibrated LYSO-PSAPD data ");
    CoincEvent *calevt = new CoincEvent();
    merged->Branch("Event",&calevt);

    if (verbose) {
        cout << "filling new Tree :: " << endl;
    }
    for (Long64_t ii=0; ii<entries; ii++) {
        mm->GetEntry(ii);
        calevt = evt;
        if (evt->fin1>FINS_PER_CARTRIDGE) continue;
        if (evt->fin2>FINS_PER_CARTRIDGE) continue;
        if ((evt->crystal1<65)&&((evt->apd1==0)||(evt->apd1==1))&&(evt->m1<MODULES_PER_FIN)) {
            if ((evt->crystal2<65)&&((evt->apd2==0)||(evt->apd2==1))&&(evt->m2<MODULES_PER_FIN)) {
                calevt->dtf -= mean_crystaloffset[evt->crystal1];
                calevt->dtf += mean_crystaloffset[evt->crystal2];
                if (evt->E1>400&&evt->E1<600&&evt->E2>400&&evt->E2<600) {
                    tres->Fill(calevt->dtf);
                }
            }
        }
        merged->Fill();
    }
    merged->Write();
    calfile->Close();

    // Fit the fine timestamp histogram and print it out with the fit
    tres->Fit("gaus","","",-10,10);
    c1->Clear();
    tres->Draw();
    Char_t psfile[MAXFILELENGTH];
    sprintf(psfile,"%s.tres.crysoffset.ps",rootfile.c_str());

    c1->Print(psfile);

    sprintf(psfile,"%s.crysoffset.ps",rootfile.c_str());
    draw_crystal_histograms(crystaloffset, c1, psfile);

    // below section added by David
    if (logscale == 1) {
        c1->SetLogy();
        sprintf(psfile,"%s.tres.crysoffset.log.ps",rootfile.c_str());
        c1->Print(psfile);
    }
    return(0);
}

