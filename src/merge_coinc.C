#include "TROOT.h"
#include "Riostream.h"
#include "TTree.h"
#include "TFile.h"
#include "TMath.h"
#include "Util.h"
#include "Syspardef.h"
#include "ModuleCal.h"
#include "CoincEvent.h"
#include <string>

void usage() {
    cout << "merge_coinc [-v -h] -fl [left filename] -fr [right filename]\n"
         << "\n"
         << "Options:\n"
         << "  -a : write out coincidence event to an ascii file as well\n"
         << "  -c : specify course window in coarse timestamp units\n"
         << "  -d : specify the delay for a delayed window coincidence\n"
         << "  -of: specify the output filename instead of default\n"
         << endl;
}

int main(int argc, char ** argv)
{
    if (argc == 1) {
        usage();
        return(0);
    }
    string filename_left;
    string filename_right;
    string filename_output;
    Int_t verbose = 0;
    Int_t ascii= 0;
    Int_t DELAY = 0;
    Int_t COARSEDIFF = 10;
    CoincEvent * evt = new CoincEvent();
    Bool_t outfilespec = kFALSE;

    for(int ix = 1; ix < argc; ix++) {
        if(strcmp(argv[ix], "-h") == 0) {
            usage();
            return(0);
        }
        if(strcmp(argv[ix], "-v") == 0) {
            cout << "Verbose Mode " << endl;
            verbose = 1;
        }
        if(strcmp(argv[ix], "-a") == 0) {
            cout << "Ascii output file generated" << endl;
            ascii = 1;
        }
        if(strcmp(argv[ix], "-c") == 0) {
            COARSEDIFF = atoi(argv[ix+1]);
            cout << "Coarse timestamp window =  " << COARSEDIFF << endl;
            ix++;
        }
        if(strcmp(argv[ix], "-d") == 0) {
            DELAY = atoi( argv[ix+1]);
            cout << "Delayed window =  " << DELAY << ". " << endl;
            ix++;
        }
        if(strcmp(argv[ix], "-fl") == 0) {
            filename_left = string(argv[ix + 1]);
        }
        if(strcmp(argv[ix], "-fr") == 0) {
            filename_right = string(argv[ix + 1]);
        }
        if(strcmp(argv[ix], "-of") == 0) {
            filename_output = string(argv[ix + 1]);
            outfilespec = kTRUE;
        }
    }

    if (verbose) {
        cout << "Opening file " << filename_left << endl;
    }
    TFile *file_left = new TFile(filename_left.c_str(),"OPEN");
    if (!file_left || file_left->IsZombie()) {
        cerr << "problems opening file \"" << filename_left << "\"\n"
             << "Exiting..." << endl;
        return(-11);
    }
    TTree *car_l = (TTree *) file_left->Get("SCalTree");
    ModuleCal *EL=0;
    ModuleCal *ER=0;
    car_l->SetBranchAddress("Time Sorted Data",&EL);

    if (verbose) {
        cout << "Opening file " << filename_right <<endl;
    }
    TFile *file_right = new TFile(filename_right.c_str(),"OPEN");
    if (!file_right || file_right->IsZombie()) {
        cerr << "problems opening file \"" << filename_right << "\"\n"
             << "Exiting..." << endl;
        return(-12);
    }
    TTree *car_r = (TTree *) file_right->Get("SCalTree");
    ofstream asciiout;

    car_r->SetBranchAddress("Time Sorted Data",&ER);

    // Open Calfile //
    Char_t asciifile[FILENAMELENGTH];
    Char_t rootfile[FILENAMELENGTH];
    string filebase(filename_left, 0, filename_left.size() - 13);
    if (outfilespec) {
        sprintf(rootfile,"%s",filename_output.c_str());
    } else {
        sprintf(rootfile,"%s",filebase.c_str());
        if (DELAY) {
            sprintf(rootfile,"%s.delaywindow%d",filebase.c_str(),DELAY);
        }
        strcat(rootfile,".merged.root");
    }
    if (ascii) {
        sprintf(asciifile,"%s.merged.ascii",filebase.c_str());
        asciiout.open(asciifile);
    }

    cout << "Opening file " << rootfile << " for writing " << endl;
    TFile *calfile = new TFile(rootfile,"RECREATE");

    TTree *merged = new  TTree("merged","Merged and Calibrated LYSO-PSAPD data ");
    merged->Branch("Event",&evt);

    Long64_t entries_car_r = car_r->GetEntries();
    Long64_t entries_car_l = car_l->GetEntries();

    if (verbose) {
        cout << "Right entries: " << entries_car_r  << endl;
        cout << "Left  entries: " << entries_car_l  << endl;
    }

    Int_t car_ri = 0;
    Int_t car_li = 0;

    Long64_t lefttime = 0;
    Long64_t righttime = 0;

    long dropped_single(0);

    Int_t coincidence_events=0;

    while ((car_ri < entries_car_r) && (car_li < entries_car_l)) {
        car_r->GetEntry(car_ri);
        car_l->GetEntry(car_li);

        lefttime=EL->ct;
        righttime=ER->ct;

        if (TMath::Abs(lefttime+DELAY-righttime) < COARSEDIFF) {
            evt->ct=lefttime;
            evt->dtc= lefttime-righttime;
            evt->dtf= EL->ft-ER->ft;
            evt->chip1=EL->chip;
            evt->fin1=EL->fin;
            evt->m1=EL->m;
            evt->apd1=EL->apd;
            evt->crystal1=EL->id;
            evt->E1=EL->Ecal;
            evt->Ec1=EL->Ec;
            evt->Ech1=EL->Ech;
            evt->ft1=EL->ft;
            evt->x1=EL->x;
            evt->y1=EL->y;
            evt->chip2=ER->chip;
            evt->fin2=ER->fin;
            evt->m2=ER->m;
            evt->apd2=ER->apd;
            evt->crystal2=ER->id;
            evt->E2=ER->Ecal;
            evt->Ec2=ER->Ec;
            evt->Ech2=ER->Ech;
            evt->ft2=ER->ft;
            evt->x2=ER->x;
            evt->y2=ER->y;
            evt->cartridge1=EL->cartridge;
            evt->cartridge2=ER->cartridge;
            evt->pos = ER->pos;

            if (evt->dtf < -TMath::Pi() ) {
                evt->dtf+=2*TMath::Pi();
            }
            if (evt->dtf >  TMath::Pi() ) {
                evt->dtf-=2*TMath::Pi();
            }
            evt->dtf*=1e9;
            evt->dtf/=(2*TMath::Pi()*UVFREQUENCY);
            if (((EL->ft > -1) && (ER->ft > -1)) && (ER->pos == EL->pos)) {
                coincidence_events++;
                merged->Fill();
                if (ascii) {
                    asciiout << evt->dtc << " "
                             << evt->dtf << " "
                             << evt->chip1 << " "
                             << evt->m1 << " "
                             << evt->apd1 << " "
                             << evt->crystal1 << " "
                             << evt->E1 << " "
                             << evt->Ec1 << " "
                             << evt->Ech1 << " "
                             << evt->ft1 << " "
                             << evt->chip2 << " "
                             << evt->m2 << " "
                             << evt->apd2 << " "
                             << evt->crystal2 << " "
                             << evt->E2 << " "
                             << evt->Ec2 << " "
                             << evt->Ech2 << " "
                             << evt->ft2 << " "
                             << evt->x1 << " "
                             << evt->y1  << " "
                             << evt->x2 << "  "
                             << evt->y2 << "  "
                             << evt->pos << "  "
                             << evt->fin1 << "  "
                             << evt->fin2
                             << endl;
                }
            }
            car_li++;
            car_ri++;
        } else {
            dropped_single++;
            if (lefttime < righttime) {
                car_li++;
            } else {
                car_ri++;
            }
        }
    }

    cout << "Filled tree with " << coincidence_events << " events"
         << "  (car_li = " << car_li << "; car_ri = " << car_ri << ")" << endl;
    cout << "Dropped:\n" << "  Single - " << dropped_single << endl;

    merged->Write();
    calfile->Close();

    return(0);
}
