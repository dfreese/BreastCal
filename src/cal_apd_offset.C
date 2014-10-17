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

#define MINMODENTRIES 200

Int_t drawmod(TH1F *hi[UNITS][MODULES_PER_FIN][APDS_PER_MODULE], TCanvas *ccc, Char_t filename[MAXFILELENGTH]);
Float_t calfunc(TH1F *hist, TF1 *fitfun,  Bool_t coarsetime, Bool_t gausfit, Int_t verbose);

int main(int argc, Char_t *argv[])
{ 
    Int_t MOD1=6;
    Int_t MOD2=1;
    Int_t APD1=0;
    Int_t APD2=0;
    Bool_t coarsetime=1; 
    Int_t crystalmean=0;
    Int_t usegausfit=0;
    Char_t histtitle[40];

    Int_t DTF_low;
    Int_t DTF_hi;
    Int_t FINELIMIT(400);
    Int_t DTFLIMIT(5);

    cout << " Welcome to cal_apd_offset" << endl;

    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    Char_t		filenamel[CALFILENAMELENGTH] = "";
    Int_t		verbose = 0;
    //module UNIT0,UNIT1,UNIT2,UNIT3;
    CoincEvent      *evt = new CoincEvent();
    Int_t fin1=99;
    Int_t fin2=99;
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    for(int ix = 1; ix < argc; ix++) {

        /*
         * Verbose '-v'
         */
        if(strncmp(argv[ix], "-v", 2) == 0) {
            cout << "Verbose Mode " << endl;
            verbose = 1;
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

        /* filename '-f' */
        if(strncmp(argv[ix], "-f", 2) == 0) {
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
            } else if(strncmp(argv[ix], "-f1", 3) == 0) {
                fin1=atoi ( argv[ix+1]); ix++;
                cout << " Fin 1 :: "  <<fin1<< endl;
            }  else if(strncmp(argv[ix], "-f2", 3) == 0) {
                fin2=atoi ( argv[ix+1]) ; ix++;
                cout << " Fin 2 :: " << fin2 <<endl;
            } else if(strlen(argv[ix + 1]) < CALFILENAMELENGTH) {
                sprintf(filenamel, "%s", argv[ix + 1]);
            } else {
                cout << "Filename " << argv[ix + 1] << " too long !" << endl;
                cout << "Exiting.." << endl;
                return -99;
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

    Char_t filebase[CALFILENAMELENGTH],rootfile[CALFILENAMELENGTH]; 
    Char_t calparfilename[CALFILENAMELENGTH];
    ifstream infile;

    TH1F * apdoffset[SYSTEM_PANELS][CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE];
    TF1 * fit_apdoffset[SYSTEM_PANELS][CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE];

    if (coarsetime) {
        DTF_low = -300;
        DTF_hi = 300;
        FINELIMIT=400;
        DTFLIMIT=200;
        cout << " Using Coarse limits: " << FINELIMIT << endl;
    } else {
        DTF_low = -50;
        DTF_hi = 50;
        DTFLIMIT=5;
    }



    for (int panel = 0; panel < 2; panel++) {
        for (int cartridge = 0; cartridge < CARTRIDGES_PER_PANEL; cartridge++) {
            for (int fin = 0; fin < FINS_PER_CARTRIDGE; fin++) {
                for (int mod=0; mod<MODULES_PER_FIN; mod++) {
                    for (int apd=0; apd<APDS_PER_MODULE; apd++) {
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


    cout << " Opening file " << filenamel << endl;
    TFile *rtfile = new TFile(filenamel,"OPEN");
    TTree *mm  = (TTree *) rtfile->Get("merged");

    if (!mm) {
        if (verbose) cout << " Problem reading Tree merged "   << " from file " << filenamel << ". Trying tree mana." << endl;
        mm  = (TTree *) rtfile->Get("mana");
    }

    if (!mm) {
        cout << " Problem reading Tree mana "   << " from file " << filenamel << endl;
        cout << " Exiting " << endl;
        return -10;
    }

    mm->SetBranchAddress("Event",&evt);

    Long64_t entries = mm->GetEntries();

    cout << " Total  entries: " << entries << endl; 
    if (verbose) cout << " Filling crystal spectra on the left. " << endl;
    Long64_t checkevts=0;


    strncpy(filebase,filenamel,strlen(filenamel)-5);
    filebase[strlen(filenamel)-5]='\0';
    sprintf(rootfile,"%s",filebase);
    if (verbose) cout << " ROOTFILE = " << rootfile << endl;

    // Fill histograms for the left panel with their associated events
    int panel = 0;
    for (Long64_t ii=0; ii < entries; ii++) {
        mm->GetEntry(ii);
        if (evt->fin1>FINS_PER_CARTRIDGE) {
            continue;
        }
        if ((evt->crystal1<65) && 
            ((evt->apd1==0) || (evt->apd1==1)) &&
            (evt->m1<MODULES_PER_FIN))
        {
            if ((evt->E1>400) && (evt->E1<600)) {
                if (TMath::Abs(evt->dtc ) < 6) {
                    if (TMath::Abs(evt->dtf ) < FINELIMIT ) {
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
    sprintf(calparfilename,"%s_calpar_%d.txt",rootfile, panel);
    ofstream calpars;
    calpars.open(calparfilename);
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
                    calpars <<  std::setw(4) << mean_apdoffset[panel][cartridge][fin][mod][apd] << " ";
                }
            }
            calpars << endl;
        }
    }
    calpars.close();

    Char_t psfile[MAXFILELENGTH];
    sprintf(psfile,"%s_panel0_cartridge0.ps",rootfile);
    drawmod(apdoffset[0][0],c1,psfile);

    if (verbose)  cout << " Filling crystal spectra on the right. " << endl;
    checkevts=0;


    // Using the left side data, generate the calibration parameters for the
    // right side.  Start here by filling histograms based on the panel 1 data
    panel = 1;
    for (Long64_t ii=0; ii < entries; ii++) {
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
                                apdoffset[1][evt->cartridge2][evt->fin2][evt->m2][evt->apd2]->Fill(
                                        evt->dtf - mean_apdoffset[0][evt->cartridge1][evt->fin1][evt->m1][evt->apd1]);
                            }
                        }
                    }
                }
            }
        }
    } // loop over entries

    if (verbose){
        cout << " Done looping over entries " << endl;
        cout << " I made " << checkevts << " calls to Fill() " << endl;         
    }

    sprintf(calparfilename,"%s_calpar_%d.txt",rootfile, panel);
    calpars.open(calparfilename);
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
                    calpars <<  std::setw(4) << mean_apdoffset[panel][cartridge][fin][mod][apd] << " ";
                }
            }
            calpars << endl;
        }
    }
    calpars.close();

    sprintf(psfile,"%s_panel1_cartridge0.ps",rootfile);
    drawmod(apdoffset[1][0],c1,psfile);


    // Now using these parameters, generate the [filename].apdoffcal.root file
    // with the fine timestamp calibrated by subtracting the mean_apdoffset for
    // each apd involved in the interaction
    strcat(rootfile,".apdoffcal.root");

    TH1F *tres = new TH1F("tres","Time Resolution After Time walk correction",100,-25,25);

    if (verbose) cout << " Opening file " << rootfile << " for writing " << endl;
    TFile *calfile = new TFile(rootfile,"RECREATE");
    TTree *merged = new  TTree("merged","Merged and Calibrated LYSO-PSAPD data ");
    CoincEvent *calevt = new CoincEvent();
    merged->Branch("Event",&calevt);

    checkevts=0;
    if (verbose) cout << "filling new Tree :: " << endl;

    for (Long64_t ii=0; ii<entries; ii++) {
        mm->GetEntry(ii);
        calevt=evt;
        if (evt->fin1>FINS_PER_CARTRIDGE) continue;
        if (evt->fin2>FINS_PER_CARTRIDGE) continue;
        if ((evt->crystal1<65)&&((evt->apd1==APD1)||(evt->apd1==1))&&(evt->m1<MODULES_PER_FIN)) {
            if ((evt->crystal2<65)&&((evt->apd2==APD1)||(evt->apd2==1))&&(evt->m2<MODULES_PER_FIN)) {

                calevt->dtf-= mean_apdoffset[0][evt->cartridge1][evt->fin1][evt->m1][evt->apd1] ;
                calevt->dtf-= mean_apdoffset[1][evt->cartridge2][evt->fin2][evt->m2][evt->apd2] ; 
                // Energy gate to put only the photopeaks in the fine time
                // stamp histogram for the time calibration calculation
                if (evt->E1>400&&evt->E1<600&&evt->E2>400&&evt->E2<600) {
                    tres->Fill(calevt->dtf);
                }
            }
        }
        checkevts++;
        merged->Fill();
    }


    cout << " New Tree filled with " << checkevts << " events. " << endl;

    merged->Write();
    calfile->Close();

    // Fit the fine timestamp histogram
    tres->Fit("gaus","","",-10,10);

    // Then output the histogram with it's fit to a postscript file
    c1->Clear();
    tres->Draw();
    sprintf(psfile,"%s.tres.apdoffset.ps",rootfile);
    c1->Print(psfile);

    return(0);
}

Int_t drawmod(TH1F *hi[FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE], TCanvas *ccc, Char_t filename[MAXFILELENGTH])
{
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    Int_t   i,k;
    Char_t  filenameo[MAXFILELENGTH+1], filenamec[MAXFILELENGTH+1];
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    // cout << "filename = " << filename << endl;

    strcpy(filenameo, filename);
    strcpy(filenamec, filename);
    strcat(filenameo, "(");
    strcat(filenamec, ")");

    ccc->Clear();
    ccc->Divide(4, 8);

    for(k = 0; k < FINS_PER_CARTRIDGE ; k++) {
        for(i = 0 ;i < MODULES_PER_FIN; i++ ) {
            ccc->cd(1+2*(i));
            hi[k][i][0]->Draw("E");
            ccc->cd(2+2*(i));
            hi[k][i][1]->Draw("E");
        }
        if(k == 0) {
            ccc->Print(filenameo);
        } else {
            if(k == (FINS_PER_CARTRIDGE-1)) {
                ccc->Print(filenamec);
            } else {
                ccc->Print(filename);
            }
        }
        ccc->Clear();
        ccc->Divide(4, 8);
    }


    return 0;
}

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
            }
            else  {
                fitfun = fitgaus_peak(hist,1) ;
                return(fitfun->GetParameter(1));
            }
        }
    } // gausfit 
    npeaks = s->Search(hist,3,"",0.2);
    if (npeaks) {
        return getmax(s, npeaks, verbose);
    } else {
        return(0);
    }
}

