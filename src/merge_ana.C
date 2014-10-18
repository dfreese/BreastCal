#include "TROOT.h"
#include "Riostream.h"
#include "TTree.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TSpectrum.h"
#include "TSpectrum2.h"
#include "TH2F.h"
#include "TVector.h"
#include "TMath.h"
#include "TF1.h"
#include "Apd_Fit.h"
#include "./decoder.h"
#include "./CoincEvent.h"

#define ENERGY_CUT_LOW 400
#define ENERGY_CUT_HIGH 700
#define CT_WINDOW 4

int main(int argc, Char_t *argv[]) {
    cout << "Welcome to merge_ana " << endl;
    cout << "Energy gating and crystal range adjustments " << endl;

    Char_t filename[FILENAMELENGTH] = "";
    Int_t verbose = 0;
    bool randoms_flag = false;
    Int_t threshold = -1000;

    for(int ix = 1; ix < argc; ix++) {
        if(strncmp(argv[ix], "-v", 2) == 0) {
            cout << "Verbose Mode " << endl;
            verbose = 1;
        }
        if(strncmp(argv[ix], "-r", 2) == 0) {
            cout << "Randoms Mode " << endl;
            randoms_flag = true;
        }
        if(strncmp(argv[ix], "-t", 2) == 0) {
            threshold = atoi( argv[ix+1]);
            cout << "Threshold =  " << threshold << " ( not implemented yet ) " << endl;
            ix++;
        }
        if(strncmp(argv[ix], "-f", 3) == 0) {
            if(strlen(argv[ix + 1]) < FILENAMELENGTH) {
                sprintf(filename, "%s", argv[ix + 1]);
            } else {
                cout << "Filename " << argv[ix + 1] << " too long !" << endl;
                cout << "Exiting.." << endl;
                return -99;
            }
        }
    }

    rootlogon(verbose);

    Int_t DTC_LOW = -CT_WINDOW;
    Int_t DTC_HI = CT_WINDOW;


    Char_t filebase[FILENAMELENGTH];
    Char_t rootfile[FILENAMELENGTH];

    cout << " Opening file " << filename << endl;
    TFile *file = new TFile(filename,"OPEN");
    TTree *m = (TTree *) file->Get("merged");
    CoincEvent *data = 0;

    m->SetBranchAddress("Event",&data);

    // Open output file - matching required format for ALEX //
    strncpy(filebase,filename,strlen(filename)-5);
    filebase[strlen(filename)-5]='\0';
    sprintf(rootfile,"%s",filebase);
    if (randoms_flag) {
        strcat(rootfile,".randoms.root");
    } else {
        strcat(rootfile,".ana.root");
    }

    cout << " Opening file " << rootfile << " for writing " << endl;
    TFile *orf = new TFile(rootfile,"RECREATE");
    CoincEvent *evt = new CoincEvent();
    TTree *mana = new  TTree("mana","Merged and Calibrated LYSO-PSAPD data ");
    mana->Branch("Event",&evt);

    Long64_t entries_m = m->GetEntries();
    cout << " Processing " <<  entries_m  <<  " Events " << endl;

    Long64_t events_passing_cut = 0;
    Long64_t events_cut_time_window(0);
    Long64_t events_cut_energy_window(0);
    Long64_t events_cut_invalid_index(0);
    for (Long64_t ii = 0; ii<entries_m; ii++) {
        m->GetEntry(ii);
        if (((data->dtc > DTC_LOW) && (data->dtc < DTC_HI)) || randoms_flag) {
            if ((data->E1<ENERGY_CUT_HIGH) && (data->E1 > ENERGY_CUT_LOW)) {
                if ((data->E2 < ENERGY_CUT_HIGH) && (data->E2 > ENERGY_CUT_LOW)) {
                    if ((data->crystal1 > -1) && (data->crystal1 < CRYSTALS_PER_APD)) {
                        if ((data->crystal2 > -1) && (data->crystal2 < CRYSTALS_PER_APD)) {
                            evt = data;
                            mana->Fill();
                            events_passing_cut++;
                        } else {
                            events_cut_invalid_index++;
                        }
                    } else {
                        events_cut_invalid_index++;
                    }
                } else {
                    events_cut_energy_window++;
                }
            } else {
                events_cut_energy_window++;
            }
        } else {
            events_cut_time_window++;
        }
    } // loop over entries
    mana->Write();
    orf->Close();

    cout << " Wrote file " << filename << " with "
         << events_passing_cut << " events." << endl;
    cout << " Dropped Events: " 
         << events_cut_invalid_index + events_cut_energy_window + events_cut_time_window
         << endl;
    cout << "    Time Window (+/-" << CT_WINDOW << "ct counts): " 
         << events_cut_time_window << endl;
    cout << "    Energy Window (" << ENERGY_CUT_LOW << "-" << ENERGY_CUT_HIGH << "keV): "
         << events_cut_energy_window << endl;
    cout << "    Invalid Index Value: "
         << events_cut_invalid_index << endl;

    return(0);
}

