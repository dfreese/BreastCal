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
#define MINMODENTRIES 1000

int main(int argc, Char_t *argv[])
{ 
    Int_t MOD1 = 6; // Unused
    Int_t MOD2 = 1; // Unused
    Int_t APD1 = 0;
    Int_t APD2 = 0; // Unused
    Int_t uvcal = 0; // Unused
    Int_t energycal = 0; // Unused
    Int_t coarsetime = 1;  // Not really used
    Int_t crystalmean = 0; // Unused
    Int_t energyspatial = 0; // Unused
    Int_t verbose = 0;
    Int_t ascii = 0; // Unused

    Int_t DTF_low; // Not really used
    Int_t DTF_hi; // Not really used
    Int_t FINELIMIT;
    Bool_t common = 0;
    Int_t fin1 = 99; // Unused
    Int_t fin2 = 99; // Unused

    cout << " Welcome to cal_edep " << endl;

    Char_t filenamel[FILENAMELENGTH] = "";
    CoincEvent * evt = new CoincEvent();     

    for(int ix = 1; ix < argc; ix++) {
        if(strncmp(argv[ix], "-v", 2) == 0) {
            cout << "Verbose Mode " << endl;
            verbose = 1;
        }

        if((strncmp(argv[ix], "-a", 2) == 0)&& (strncmp(argv[ix], "-apd",4) != 0 )) {
            cout << "Ascii output file generated" << endl;
            ascii = 1;
        }

        if(strncmp(argv[ix],"-c",2) ==0 ) { 
            cout << " Using common for energy dependence " << endl ;
            common = 1;
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
            crystalmean = 1;
            cout << "Crystal calibration "  <<endl;
        }

        if(strncmp(argv[ix], "-uv", 3) == 0) {
            uvcal = 1;
            cout << " UV calibration "  <<endl;
        }

        if(strncmp(argv[ix], "-ec", 3) == 0) {
            energycal=1;
            cout << " Energy calibration "  <<endl;
        }

        if(strncmp(argv[ix], "-esp", 4) == 0) {
            energyspatial = 1;
            cout << " Using spatials for energy calibration "  <<endl;
        }

        if(strncmp(argv[ix], "-f", 2) == 0) {
            if(strncmp(argv[ix], "-ft", 3) == 0) {
                coarsetime = 0;
                ix++;
                if (ix == argc) {
                    cout << " Please enter finelimit interval: -ft [finelimit]\nExiting. " << endl;
                    return(-20);
                }
                FINELIMIT = atoi(argv[ix]);
                if (FINELIMIT < 1) {
                    cout << " Error. FINELIMIT = " << FINELIMIT << " too small. Please specify -ft [finelimit]. " << endl;
                    cout << "Exiting." << endl;
                    return(-20);
                }
                cout << " Fine time interval = "  << FINELIMIT << endl;
            } else if(strncmp(argv[ix], "-f1", 3) == 0) {
                fin1=atoi ( argv[ix+1]); ix++;
                cout << " Fin 1 :: "  <<fin1<< endl;
            } else if(strncmp(argv[ix], "-f1", 3) == 0) {
                fin2=atoi ( argv[ix+1]) ; ix++;
                cout << " Fin 2 :: " << fin2 <<endl;
            } else if(strlen(argv[ix + 1]) < FILENAMELENGTH) {
                sprintf(filenamel, "%s", argv[ix + 1]);
            } else {
                cout << "Filename " << argv[ix + 1] << " too long !" << endl;
                cout << "Exiting.." << endl;
                return(-99);
            }
        }
    }
    rootlogon(verbose);
    gStyle->SetOptStat(kTRUE); 

    TCanvas *c1((TCanvas*)gROOT->GetListOfCanvases()->FindObject("c1"));
    if (!c1) {
        c1 = new TCanvas("c1","c1",10,10,1000,1000);
    }
    c1->SetCanvasSize(700,700);

    Char_t filebase[FILENAMELENGTH],rootfile[FILENAMELENGTH]; 

    cout << " Opening file " << filenamel << endl;
    TFile *rtfile = new TFile(filenamel,"OPEN");
    TTree *mm  = (TTree *) rtfile->Get("merged");
    mm->SetBranchAddress("Event",&evt);

    coarsetime = 0;
    if (coarsetime) {
        DTF_low = -300;
        DTF_hi = 300;
        FINELIMIT = 300;
        cout << " Using Coarse limits: " << FINELIMIT << endl;
    } else {
        DTF_low = -FINELIMIT/2;
        DTF_hi = FINELIMIT/2;
    }

    TH2F *energydependence[2];
    energydependence[0] = new TH2F("energydependence[0]","Edep Panel 0",100,400,600,100,-50,50);
    energydependence[1] = new TH2F("energydependence[1]","Edep Panel 1",100,400,600,100,-50,50);

    Long64_t entries = mm->GetEntries();
    cout << " Total  entries: " << entries << endl; 


    if (verbose) {
        cout << " Filling crystal spectra on the left. " << endl;
    }

    strncpy(filebase,filenamel,strlen(filenamel)-5);
    filebase[strlen(filenamel)-5]='\0';
    sprintf(rootfile,"%s",filebase);

    cout << " ROOTFILE = " << rootfile << endl;

    Long64_t checkevts = 0;
    for (int ii = 0; ii < entries; ii++) {
        mm->GetEntry(ii);
        if (evt->cartridge1 > CARTRIDGES_PER_PANEL) continue;
        if (evt->fin1 > FINS_PER_CARTRIDGE) continue;
        if ((evt->crystal1 < 65) &&
                ((evt->apd1 == APD1) || (evt->apd1 == 1)) &&
                (evt->m1 < MODULES_PER_FIN))
        {
            if ((evt->E1 > 400) && (evt->E1 < 600)) {
                if (TMath::Abs(evt->dtc) < 6) {
                    if (TMath::Abs(evt->dtf ) < FINELIMIT ) {
                        checkevts++;
                        if (common) {
                            energydependence[0]->Fill(evt->Ec1,evt->dtf);
                        } else {
                            energydependence[0]->Fill(evt->E1,evt->dtf);
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

    TH1F *profehist[2];
    TF1 *profehistfit[2];

    profehistfit[0] = new TF1("profehistfit[0]","pol1",400,600);   
    profehistfit[1] = new TF1("profehistfit[1]","pol1",400,600);   

    profehist[0] = (TH1F *) energydependence[0]->ProfileX();
    profehist[0]->SetName("profehist[0]");

    if (verbose) {
        profehist[0]->Fit("profehistfit[0]");
    } else {
        profehist[0]->Fit("profehistfit[0]","Q");
    }

    c1->Clear();
    c1->Divide(1,2);
    c1->cd(1);
    energydependence[0]->Draw("colz");
    c1->cd(2);
    profehist[0]->Draw();

    Char_t psfile[MAXFILELENGTH];
    sprintf(psfile,"%s_edpe_panel1.ps",rootfile);

    c1->Print(psfile);


    if (verbose) {
        cout << " Filling crystal spectra on the right. " << endl;
    }
    checkevts = 0;
    for (int ii = 0; ii < entries; ii++) {
        mm->GetEntry(ii);
        if (evt->cartridge1 > CARTRIDGES_PER_PANEL) continue;
        if (evt->cartridge2 > CARTRIDGES_PER_PANEL) continue;
        if (evt->fin1 > FINS_PER_CARTRIDGE) continue;
        if (evt->fin2 > FINS_PER_CARTRIDGE) continue;
        if ((evt->crystal1 < 65) &&
                ((evt->apd1 == APD1) || (evt->apd1 == 1)) &&
                (evt->m1 < MODULES_PER_FIN))
        {
            if ((evt->crystal2 < 65) &&
                    ((evt->apd2 == APD1) || (evt->apd2 == 1)) &&
                    (evt->m2 < MODULES_PER_FIN))
            {
                if  ((evt->E2 > 400) && (evt->E2 < 600)) {
                    if  ((evt->E1 > 400) && (evt->E1 < 600)) {
                        if (TMath::Abs(evt->dtc ) < 6) {
                            if (TMath::Abs(evt->dtf) < FINELIMIT) {
                                checkevts++;
                                energydependence[1]->Fill(evt->Ec2,evt->dtf-profehistfit[0]->Eval(evt->Ec1));
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

    profehist[1] = (TH1F *) energydependence[1]->ProfileX();
    profehist[1]->SetName("profehist[1]");

    if (verbose) {
        profehist[1]->Fit("profehistfit[1]");
    } else {
        profehist[1]->Fit("profehistfit[1]","Q");
    }

    c1->Clear();
    c1->Divide(1,2);
    c1->cd(1);
    energydependence[1]->Draw("colz");
    c1->cd(2);
    profehist[1]->Draw();


    sprintf(psfile,"%s_edpe_panel2.ps",rootfile);
    c1->Print(psfile);
    sprintf(psfile,"%s_fin2.ps",rootfile);


    strcat(rootfile,".edepcal.root");

    TH1F *tres = new TH1F("tres","Time Resolution After Time walk correction",100,-25,25);

    if (verbose) {
        cout << " Opening file " << rootfile << " for writing " << endl;
    }
    TFile *calfile = new TFile(rootfile,"RECREATE");
    TTree *merged = new  TTree("merged","Merged and Calibrated LYSO-PSAPD data ");

    CoincEvent * calevt = new CoincEvent();     
    merged->Branch("Event",&calevt);

    if (verbose) {
        cout << "filling new Tree :: " << endl;
    }
    checkevts = 0;
    for (int ii = 0; ii < entries; ii++) {
        mm->GetEntry(ii);
        calevt=evt;
        if (evt->cartridge1 > CARTRIDGES_PER_PANEL) continue;
        if (evt->cartridge2 > CARTRIDGES_PER_PANEL) continue;
        if (evt->fin1 > FINS_PER_CARTRIDGE) continue;
        if (evt->fin2 > FINS_PER_CARTRIDGE) continue;
        if ((evt->crystal1<65) && 
                ((evt->apd1 == APD1) || (evt->apd1 == 1)) && 
                (evt->m1<MODULES_PER_FIN))
        {
            if ((evt->crystal2<65) &&
                    ((evt->apd2 == APD1) || (evt->apd2 == 1)) &&
                    (evt->m2 < MODULES_PER_FIN))
            {
                if (common) {
                    calevt->dtf-= profehistfit[0]->Eval(evt->Ec1);
                    calevt->dtf -= profehistfit[1]->Eval(evt->Ec2);
                } else {
                    calevt->dtf-= profehistfit[0]->Eval(evt->E1);
                    calevt->dtf -= profehistfit[1]->Eval(evt->E2);
                }
                if ((evt->E1 > 400) &&
                       (evt->E1 < 600) &&
                       (evt->E2 > 400) &&
                       (evt->E2 < 600))
                {
                    tres->Fill(calevt->dtf); 
                }
            }
        }
        checkevts++;
        merged->Fill();
    }
    merged->Write();
    calfile->Close();

    if (verbose) {
        cout << " Done looping over entries " << endl;
        cout << " I made " << checkevts << " calls to Fill() " << endl;
    }
    tres->Fit("gaus","","",-10,10);

    c1->Clear();
    tres->Draw();
    sprintf(psfile,"%s.tres.edepcal.ps",rootfile);
    c1->Print(psfile);

    if (verbose) {
        cout << tres->GetEntries() << " Entries in tres." << endl;
    }

    return(0);
}
