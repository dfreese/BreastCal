/*************

 ************/
#include "TROOT.h"
#include "Riostream.h"
#include "TTree.h"
#include "TTreeIndex.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TSpectrum.h"
#include "TSpectrum2.h"
#include "TH2F.h"
#include "TVector.h"
#include "TMath.h"
#include "TF1.h"
#include "./decoder.h"
#include "time.h"
#include "ModuleCal.h"

//#define DEBUG

void usage(void) {
    cout << " Usage:  " << endl;
    cout << " ./merge_4up -f [Filename] [-v] -nc [splits] -ts [timesplit] -lt [lasttime]" << endl;
    return;
}

int main(int argc, Char_t *argv[]) {
    cout << "Welcome to Merge_4up. Sorting data from each 4up. " << endl;

    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    Char_t		filenamel[FILENAMELENGTH] = "";
    Int_t verbose(0);
    Int_t ncuts(0);
    Long64_t timeinterval=(0);
    Long64_t mintime;
    ModuleCal *event = new ModuleCal();
    ModuleCal *evt = new ModuleCal(); 
    ModuleCal *unsrt_evt=0;
    TTree* cal;
    Long64_t lasttime(-1);
    Bool_t filenamespec(kFALSE);
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    for(int ix = 1; ix < argc; ix++) {
        if(strncmp(argv[ix], "-h", 2) == 0) {
            usage();
            // cout << " ./merge_panel -f [Filename] -rb [renaboard] --L/--R [-v] " << endl;
            //   cout << " Renaboard is either 0,1,2 or 3" << endl;
            //    cout << " Specify which panel: --L for left; --R for right " << endl;
            return -1;
        }

        /*
         * Verbose '-v'
         */
        if(strncmp(argv[ix], "-v", 2) == 0) {
            cout << "Verbose Mode " << endl;
            verbose = 1;
        }

        if(strncmp(argv[ix], "-nc", 3) == 0) {
            ncuts = atoi(argv[ix+1]);
            if (verbose) cout << "Number of splits : " << ncuts <<endl;
            ix++;
        }

        if(strncmp(argv[ix], "-ts", 3) == 0) {
            timeinterval = (Long64_t) atol(argv[ix+1]);
            if (verbose) cout << "Split at time : " << timeinterval <<endl;
            ix++;
        }

        if(strncmp(argv[ix], "-lt", 3) == 0) {
            lasttime = (Long64_t) atol(argv[ix+1]);
            if (verbose) cout << "Last time : " << lasttime <<endl;
            ix++;
        }



        /* filename '-f' */
        if(strncmp(argv[ix], "-f", 2) == 0) {
            if(strlen(argv[ix + 1]) < FILENAMELENGTH) {
                sprintf(filenamel, "%s", argv[ix + 1]);
                filenamespec = kTRUE;
            }
            else {
                cout << "Filename " << argv[ix + 1] << " too long !" << endl;
                cout << "Exiting.." << endl;
                return -99;
            }
            ix++;
        }
    } // loop over input arguments

    if (!filenamespec) {
        cout << " Please provide an input file. " << endl;
        usage();
        return -99;
    }

    if (lasttime < 0) {
        cout << "Please specify last time as calculated by get_opt_split :: -lt [value]" << endl;
        return -1;}

    if ((ncuts > 1) && (timeinterval == 0)) {
        cout << " Please specify the time at which to perform the splits:  -ts [timestamp] " << endl ;
        cout << " Exiting " << endl;
        return -1;
    }

    TCanvas *c1;
    c1 = (TCanvas*)gROOT->GetListOfCanvases()->FindObject("c1");
    if (!c1) {
        c1 = new TCanvas("c1","c1",10,10,1000,1000);
    }


    rootlogon(verbose);


    Char_t filebase[FILENAMELENGTH],rootfile[FILENAMELENGTH]; 
    // Variable used only once uninitialized...
    Int_t m;
    strncpy(filebase,filenamel,strlen(filenamel)-9);
    filebase[strlen(filenamel)-9]='\0';


    if (verbose){
        cout << " filename = " << filenamel << endl;
        cout << " filebase = " << filebase << endl; 
        cout << " Opening file " << filenamel << endl;
    }
    TFile *file_left = new TFile(filenamel,"OPEN");

    if (!file_left->IsOpen()) {
        cout << "problems opening file " << filenamel;
        cout << "\n Exiting " << endl; 
        return -11;
    }


    if (verbose ) {
        cout << " File Content : " << endl; 
        file_left->ls();
    }




    Char_t treename[40];

    sprintf(treename,"CalTree");
    cal = (TTree *) file_left->Get(treename);
    if (!(cal)) {
        cout << " Problem reading " << treename << " from file. Exiting." << endl;
        return -11;
    }

    cal->SetBranchAddress("Calibrated Event Data",&unsrt_evt);


    // Create Tree //
    Long64_t entries(cal->GetEntries());

    cal->GetEntry(0);
    Long64_t firsttime(unsrt_evt->ct);

    if (verbose) {
        cout << " Last timestamp = " << lasttime << endl;
    }

    Long64_t l;
    Long64_t curevent(0);


    if (ncuts > MAXCUTS) {
        cout << " ERROR ! THERE ARE OVER " << MAXCUTS*MAXHITS << " EVENTS TO PROCESS. " << endl;
        cout << " This is too many !\n Not even trying.\n Bye. " << endl;
        return -999;
    }

    if (verbose) {
        cout << " Anticipating " << ncuts << " iterations. " << endl;
    }

    Int_t nrrollovers=0;
    Long64_t timecut[MAXCUTS];
    Long64_t partentries[MAXCUTS]={0};

    time_t time_before_filling, time_before_sorting, time_after_sorting, time_after_filling,thistime; 
    Long64_t curtime(0);


    for (int k=0; k<ncuts; k++) {
        timecut[k] = firsttime + timeinterval*(k+1);
    }

    if (ncuts>0) {
        // make sure that timecut[ncuts-1] > last possible time; used to be:
        timecut[ncuts-1] = lasttime;
    } else {
        timecut[0] = lasttime;
    }

    if (verbose) {
        for (Int_t k=0;k<ncuts;k++) {
            cout << " Time cut " << k << ": " << timecut[k]  << endl;
        }
    }
    Int_t kk=0;

    // do 2 criterion   and while loop  :: while  !((criteria1) and (criteria2))
    // criteria1 set to false if  (firsttime >  timecut ),
    // criteria2 is set to false if criteria1 is false :
    // here we'll look for a number of events past the event criterion 1 was set
    // if the time of the event is smaller than the cutofftime, we fill the histogram,
    // IMPORTANT: on next iteration we need to go back to event where criterion 1 happened 
    // ( so it seems we need a dual check before filling if  time > prevcutoff && time > prevcutoff

    while ((curtime < lasttime)) {
        if (verbose) {
            cout << "\n Split number :: " << kk << ", timecut = " << timecut[kk] << endl;
        }
        time_before_filling = clock();

        sprintf(rootfile,"%s_part%d.root",filebase,kk);

        if (verbose) {
            cout << " Opening file " << rootfile << " for writing " << endl;
        }
        TFile *calfile = new TFile(rootfile,"RECREATE");
        TTree *panel = new TTree("panel","Sorted Panel Data") ;
        Long64_t lasteventtime(0);
        panel->SetDirectory(0); 

        panel->Branch("Sorted data",&event);

        // this could be simple: we just split the file according to the timestamp 
        // check whether timestamp[unsrt_evt.chip]  

        l=curevent;
        curtime=0;
        Int_t stop(0);
        Int_t extra(0);
        // setting cutime to 0 here ensures we don't skip the next while loop 
        while (!stop) {
            cal->GetEntry(l);
            curtime=unsrt_evt->ct;
#ifdef DEBUG
            cout << " L : " << l << " curtime : " << curtime << " (= " 
                 << curtime/COARSECLOCKFREQUENCY << " s), lasteventtime : " 
                 << lasteventtime << " timecut[ " << kk << "] : " 
                 << timecut[kk] << endl;
#endif
            // note that 60e6 at 12 MHz is 5 seconds, unlikely to have more tha 5 s between events
            // note that 11e9 at 12 MHz is 5 seconds, unlikely to have more than 15 min between events
            // FIXME :: HARDCODED VALUE DEALING WITH CORRUPTED DATA FROM LEFT AVDB 2014/05/12
            if ((TMath::Abs(curtime-lasteventtime) > 600e6) &&
                    (lasteventtime > 0) && (l > 100) &&
                    ((curtime -lasteventtime) < 10e9))
            {
                l++;
                if (l>entries) {
                    stop=1;
                } 
                continue;
            }
            if (((curtime - lasteventtime) > 10e9) && (lasteventtime > 0)) {
                if (verbose) {
                    cout << " CT BUG :: Event " << l << ": curtime = " 
                         << curtime << ", lasteventtime = " << lasteventtime 
                         << endl;
                }
                l++;
                if ( l> entries ) {
                    stop=1;
                }
                continue;
            }
            if ((curtime + 1e12) < lasteventtime) { 
                // rollover occured !! 
                // FIXME :: only one rollover supported as of now. 
                cout << " ROLLOVER ! curtime: " << curtime << " lasttime: " 
                     << lasteventtime << " l= " << l << endl;
                nrrollovers=1;
            }
            if (nrrollovers) { 
                curtime+=nrrollovers*ROLLOVERTIME;
            }

            event->ct=curtime;
            event->Ecal=unsrt_evt->Ecal;
            event->ft=unsrt_evt->ft;
            event->Ec=unsrt_evt->Ec;
            event->E=unsrt_evt->E;
            event->Ech=unsrt_evt->Ech;
            event->x=unsrt_evt->x;
            event->y=unsrt_evt->y;
            /* assigning chip number = from 0 - > 32 for each cartridge so 0->7; 8->15; 16->23; 24->32; 
               the chip number will be more for debugging */
            event->fin=unsrt_evt->fin;
            event->m=unsrt_evt->m;

            //HSU ADDED THIS
            event->cartridge=unsrt_evt->cartridge;

            event->apd=unsrt_evt->apd;
            event->id=unsrt_evt->id;
            event->pos=unsrt_evt->pos;
            if (kk > 0) {
                mintime=timecut[kk-1];
            } else {
                mintime=0;
            }
            if ((curtime <= timecut[kk]) && curtime>mintime) {
                panel->Fill();
            } else {
                // we're going to need to go back a number of steps
                extra++;
                if ((curtime - timecut[kk]) > 10000) { 
                    if (verbose) { 
                        cout << "STOPPING LOOP @ EVENT " << l << ": curtime -timecut[" << kk << "] : " << curtime-timecut[kk] << endl;
                    }
                    stop=1;
                }
            }   
            // we stop the while loop and close the file if the timedifference is larger than 10
#ifdef DEBUG
            cout << " filling chip " << unsrt_evt->chip << " MODULE " << event->m;
            cout << " (l="<<l<<")"<< endl;
#endif
            l++;
            if (l>=entries) {
                stop=1;
            }
            if (l<0 ) {
                l=0; // safety
            }
            if ((verbose) && (l>0)&&((l%5000000)==0)) {
                cout << " Processed " << l/1e6 << " million events " << endl;
            }
            lasteventtime=curtime;  
        }  // while !stop

        if (verbose) {
            cout << " Extra entries :: " << extra << "; l = " << l << endl;
        }

        // the extra here is to prevent roll-off between splits
        l-=extra;

        curevent=l;
        if (verbose) {
            cout << " Curevent[" << m << "] = " << curevent << "; curtime = " << curtime << endl;
        }

        partentries[kk]=panel->GetEntries();
        kk++;
        // Fill tree with first 10% of events, then sort. 

        time_before_sorting=clock();
        // sorting

        Long64_t N = panel->GetEntries();

        Long64_t *table = new Long64_t[N];
        Long64_t *sorted = new Long64_t[N];

        for (l=0; l<N; l++) {
            panel->GetEntry(l);
            table[l] = event->ct;
        } 

        if (verbose) {
            cout<<"Sorting Run numbers :: " ; //<<std::endl;
        }

        TMath::Sort(N,table,sorted,kFALSE);

        time_after_sorting=clock();

        if (verbose) {
            printf("Time to merge chips (sec): %f",(double)(time_before_sorting-time_before_filling)/(CLOCKS_PER_SEC));
            printf(" Time to sort the merged chips (sec): %f\n",(double)(time_after_sorting-time_before_sorting)/(CLOCKS_PER_SEC));
        }

        if (verbose) {
            cout << N << " entries in tree; cloning tree :: " << endl;
        }

        TTree *fourup = new TTree("SCalTree","Time Sorted Calibrated data");
        fourup->Branch("Time Sorted Data",&evt);

        if (verbose) {
            cout << " Loading Baskets ... " << endl;
        }
        panel->LoadBaskets(2328673567232LL);
        thistime=clock();  

        for (l=0;l<N;l++){
            if ((((l) % 5000000) == 0) && (l > 0) && (verbose)) {
                cout << l << " entries processed; " ;
                cout << " time spent = " <<(double)  (clock()-thistime)/CLOCKS_PER_SEC;
                cout << "; since start: " <<(double) (clock()-time_after_sorting)/CLOCKS_PER_SEC <<endl; 
                thistime = clock();
            }

            panel->GetEntry(sorted[l]);
            evt=event;
            fourup->Fill();
        }

        time_after_filling=clock();

        if (verbose) {
            printf("Time to clone the Tree (sec): %f\n",(double)(time_after_filling-time_after_sorting)/(CLOCKS_PER_SEC));
        }

        fourup->Write();
        calfile->Close();

        delete panel;
        delete calfile;

        if (kk >= ncuts) {
            break;
        }
    } // while loop 


    Long64_t allpartentries=0;
    cout << " Total entries ::" << entries << endl;
    for (int k=0;k < ncuts; k++) {
        cout << " Entries part " << k << " : " << partentries[k] << endl;
        allpartentries+=partentries[k];
    }
    cout << " Total entries each part: " << allpartentries << "( = " 
         << 100*allpartentries/entries  << " %)"<<endl;
    return 0;
}

