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
#define UNITS 16

#define MINMODENTRIES 1000

Float_t cryscalfunc(
        TH1F *hist,
        Bool_t gausfit,
        Int_t verbose)
{
    if (gausfit) {
        if ( hist->GetEntries() > 70 ) {     
            TF1 * fitfun = fitgaus(hist,0);
            return(fitfun->GetParameter(1));
        } else if ( hist->GetEntries() > 50 ) {
                return hist->GetMean();
        } else {
            return(0.0);
        }
    } else {
        // just peak search 
        TSpectrum *s = new TSpectrum();
        Int_t npeaks = s->Search(hist,3,"",0.2);
        if (npeaks) {
            return getmax(s,npeaks,verbose);
        } else {
            return(0.0);
        }
    }
}

int main(int argc, Char_t *argv[])
{ 
    Int_t MOD1=6; // Has a flag in the input part, but is not used
    Int_t MOD2=1; // Has a flag in the input part, but is not used
    // Seems to be a flag used within the logic for checking for a valid apd or
    // crystal id value.  If this is zero, aka not specified by the user, then
    // the logic checks for a 0 or 1 from the apd value.  If it's specified,
    // then the logic checks for 1 or the user specified value.  I don't know
    // why this useful. -Freese
    Int_t APD1=0;
    Int_t APD2=0; // Has a flag in the input part, but is not used
    // A flag that is used by the crystal calibration function to determine
    // wether a fit algorithm or a peak searching algorithm should be used
    // in determining the calibration parameter for that crystal
    Bool_t usegausfit=0;
    // A flag for whether course timestamp limits should be used. They are
    // only used if a fine timestamp limit is not specified
    Int_t coarsetime=1; 
    Int_t crystalmean=0; // Has a flag in the input part, but is not used
    Char_t histtitle[40];

    // DTF high and low set the upper and lower limits of the histograms for
    // each of the crystals.
    Int_t DTF_low;
    Int_t DTF_hi;
    // The fine timestamp limit does not eliminate any events from the root
    // file that is generated as a result of this program.  This sets the limit
    // for the fine timestamp difference that can be use
    Int_t FINELIMIT;
    Int_t DTFLIMIT; // Unused

    cout << " Welcome to cal_crystal_ofset2 " << endl;

    Char_t filenamel[FILENAMELENGTH] = "";
    Int_t verbose = 0;
    // Parameter for determining if the fine time stamp histogram generated
    // after calibration should be plotted in a log scale or not.  Default
    // is to not use a log scale
    Int_t logscale = 0; 



    for(int ix = 1; ix < argc; ix++) {

        /*
         * Verbose '-v'
         */
        if(strncmp(argv[ix], "-v", 2) == 0) {
            cout << "Verbose Mode " << endl;
            verbose = 1;
        }

        // Plot in log scale
        if(strncmp(argv[ix], "-log", 4) == 0) {
            cout << "Output Graph in Log Scale " << endl;
            logscale = 1;
        }

        if(strncmp(argv[ix], "-apd1", 5) == 0) {
            APD1 = atoi( argv[ix+1]);
            cout << "APD1 =  " << APD1 <<endl;
            ix++;
        }

        if(strncmp(argv[ix], "-apd2", 5) == 0) {
            APD2 = atoi( argv[ix+1]);
            cout << "APD2 =  " << APD2 <<endl;
            ix++;
        }

        if(strncmp(argv[ix], "-mod1", 5) == 0) {
            MOD1 = atoi( argv[ix+1]);
            cout << "MOD1 =  " << MOD1 <<endl;
            ix++;
        }

        if(strncmp(argv[ix], "-mod2", 5) == 0) {
            MOD2 = atoi( argv[ix+1]);
            cout << "MOD2 =  " << MOD2 <<endl;
            ix++;
        }

        if(strncmp(argv[ix], "-cm", 3) == 0) {
            crystalmean=1;
            cout << "Crystal calibration "  <<endl;
        }

        if(strncmp(argv[ix], "-gf", 3) == 0) {
            usegausfit=1;
            cout << " Using Gauss Fit "  <<endl;
        }

        if(strncmp(argv[ix], "-ft", 3) == 0) {
            coarsetime=0;
            ix++;
            if (ix == argc) {
                cout << " Please enter finelimit interval: -ft [finelimit]\nExiting. " << endl;
                return(-20);
            }
            FINELIMIT = atoi(argv[ix]);
            if (FINELIMIT < 1) {
                cout << " Error. FINELIMIT = " << FINELIMIT << " too small. Please specify -ft [finelimit]. " << endl;
                cout << "Exiting." << endl; return -20;
            }
            cout << " Fine time interval = "  << FINELIMIT << endl;
        }




        /* filename '-f' */
        if(strncmp(argv[ix], "-f", 3) == 0) {
            if(strlen(argv[ix + 1]) < FILENAMELENGTH) {
                sprintf(filenamel, "%s", argv[ix + 1]);
            } else {
                cout << "Filename " << argv[ix + 1] << " too long !" << endl;
                cout << "Exiting.." << endl;
                return -99;
            }
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

    Char_t filebase[FILENAMELENGTH],rootfile[FILENAMELENGTH]; 
    ifstream infile;

    cout << " Opening file " << filenamel << endl;
    TFile *rtfile = new TFile(filenamel,"OPEN");
    TTree *mm  = (TTree *) rtfile->Get("merged");
    CoincEvent *evt =  new CoincEvent();
    mm->SetBranchAddress("Event",&evt);


    TH1F *crystaloffset[SYSTEM_PANELS][CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE][CRYSTALS_PER_APD];

    if (coarsetime) {
        DTF_low = -100;
        DTF_hi = 100;
        FINELIMIT=100;
        DTFLIMIT=50;
        cout << " Using Coarse limits: " << FINELIMIT << endl;
    } else {
        DTF_low = -FINELIMIT;
        DTF_hi = FINELIMIT;
        DTFLIMIT=5;
    }



    for (int panel = 0; panel < SYSTEM_PANELS; panel++) {
        for (int cartridge = 0; cartridge < CARTRIDGES_PER_PANEL; cartridge++) {
            for (int fin = 0; fin < FINS_PER_CARTRIDGE; fin++) {
                for (int module = 0; module < MODULES_PER_FIN; module++) {
                    for (int apd = 0; apd < APDS_PER_MODULE; apd++) {
                        for (int crystal = 0; crystal < CRYSTALS_PER_APD; crystal++) {
                            sprintf(histtitle,"crystaloffset[%d][%d][%d][%d][%d][%d]",
                                    panel, cartridge, fin, module, apd, crystal);
                            crystaloffset[panel][cartridge][fin][module][apd][crystal] = 
                                new TH1F(histtitle,histtitle,50,DTF_low,DTF_hi);
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


    strncpy(filebase,filenamel,strlen(filenamel)-5);
    filebase[strlen(filenamel)-5]='\0';
    sprintf(rootfile,"%s",filebase);
    cout << " ROOTFILE = " << rootfile << endl;


    for (Long64_t ii = 0; ii<entries; ii++) {
        mm->GetEntry(ii);
        if (evt->fin1>FINS_PER_CARTRIDGE) continue;
        if ((evt->crystal1<65) && 
                ((evt->apd1==APD1) || (evt->apd1==1)) && 
                (evt->m1<MODULES_PER_FIN))
        {
            if ((evt->E1>400) && (evt->E1<600)) {
                if (TMath::Abs(evt->dtc ) < 6 ) {
                    if (TMath::Abs(evt->dtf ) < FINELIMIT ) {
                        checkevts++;
                        crystaloffset[0][evt->cartridge1][evt->fin1][evt->m1][evt->apd1][evt->crystal1]->Fill(evt->dtf);
                    }
                }
            }
        }
    } // loop over entries

    if (verbose){
        cout << " Done looping over entries " << endl;
        cout << " I made " << checkevts << " calls to Fill() " << endl;         }

    Float_t mean_crystaloffset[SYSTEM_PANELS][CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE][CRYSTALS_PER_APD]={{{{{{0}}}}}};

    for (int cartridge = 0; cartridge < CARTRIDGES_PER_PANEL; cartridge++) {
        for (int fin = 0; fin < FINS_PER_CARTRIDGE; fin++) {
            for (int module = 0; module < MODULES_PER_FIN; module++) {
                for (int apd = 0; apd < APDS_PER_MODULE; apd++) {
                    for (int crystal = 0; crystal < CRYSTALS_PER_APD; crystal++) {
                        mean_crystaloffset[0][cartridge][fin][module][apd][crystal] = 
                            cryscalfunc(crystaloffset[0][cartridge][fin][module][apd][crystal], usegausfit,verbose);
                    }
                }
            }
        }
    }


    Char_t psfile[MAXFILELENGTH];
    for (int fin = 0; fin < FINS_PER_CARTRIDGE; fin++) {
        sprintf(psfile,"%s_panel0_cartridge0_fin%d.ps",rootfile, fin);
        drawmod_crys2(crystaloffset[0][0][fin],c1,psfile);
    }

    Char_t calparfilename[MAXFILELENGTH];
    sprintf(calparfilename,"%s_calpar_0.txt",rootfile);
    writval(mean_crystaloffset[0],calparfilename);

    if (verbose) cout << " Filling crystal spectra on the right. " << endl;
    checkevts=0;


    for (Long64_t ii = 0; ii < entries; ii++) {
        mm->GetEntry(ii);
        if (evt->fin1>FINS_PER_CARTRIDGE) continue;
        if (evt->fin2>FINS_PER_CARTRIDGE) continue;
        if ((evt->crystal1<65)&&((evt->apd1==APD1)||(evt->apd1==1))&&(evt->m1<MODULES_PER_FIN)) {
            if ((evt->crystal2<65)&&((evt->apd2==APD1)||(evt->apd2==1))&&(evt->m2<MODULES_PER_FIN)) {
                if  ((evt->E2>400)&&(evt->E2<600)) {
                    if  ((evt->E1>400)&&(evt->E1<600)) {
                        if (TMath::Abs(evt->dtc ) < 6 ) {
                            if (TMath::Abs(evt->dtf ) < FINELIMIT ) {
                                checkevts++;
                                crystaloffset[1][evt->cartridge2][evt->fin2][evt->m2][evt->apd2][evt->crystal2]->Fill(
                                        evt->dtf - mean_crystaloffset[0][evt->cartridge1][evt->fin1][evt->m1][evt->apd1][evt->crystal1]);
                            }
                        }
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

    sprintf(calparfilename,"%s_calpar_1.txt",rootfile);
    writval(mean_crystaloffset[1],calparfilename);

    TH1F *tres = new TH1F("tres","Time Resolution After Time walk correction",400,-100,100);

    strcat(rootfile,".crystaloffcal.root");


    if (verbose) cout << " Opening file " << rootfile << " for writing " << endl;
    TFile *calfile = new TFile(rootfile,"RECREATE");
    TTree *merged = new  TTree("merged","Merged and Calibrated LYSO-PSAPD data ");
    CoincEvent *calevt = new CoincEvent();
    merged->Branch("Event",&calevt);


    if (verbose) cout << "filling new Tree :: " << endl;

    for (Long64_t ii=0; ii<entries; ii++) {
        mm->GetEntry(ii);
        calevt = evt;
        if (evt->fin1>FINS_PER_CARTRIDGE) continue;
        if (evt->fin2>FINS_PER_CARTRIDGE) continue;
        if ((evt->crystal1<65)&&((evt->apd1==APD1)||(evt->apd1==1))&&(evt->m1<MODULES_PER_FIN)) {
            if ((evt->crystal2<65)&&((evt->apd2==APD1)||(evt->apd2==1))&&(evt->m2<MODULES_PER_FIN)) {
                calevt->dtf -= mean_crystaloffset[0][evt->cartridge1][evt->fin1][evt->m1][evt->apd1][evt->crystal1];
                calevt->dtf -= mean_crystaloffset[1][evt->cartridge2][evt->fin2][evt->m2][evt->apd2][evt->crystal2]; 
                if (evt->E1>400&&evt->E1<600&&evt->E2>400&&evt->E2<600) {
                    tres->Fill(calevt->dtf);}
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
    sprintf(psfile,"%s.tres.crysoffset.ps",rootfile);

    c1->Print(psfile);

    // below section added by David
    if (logscale == 1) {
        c1->SetLogy();
        sprintf(psfile,"%s.tres.crysoffset.log.ps",rootfile);
        c1->Print(psfile);
    }
    return(0);
}

