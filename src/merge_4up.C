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
//#include "Apd_Fit.h"
//#include "Apd_Peaktovalley.h"
//#include "/Users/arne/root/macros/myrootlib.h"
//##include "/home/miil/root/libInit_avdb.h"
#include "./decoder.h"
#include "time.h"
#include "ModuleCal.h"

//#define DEBUG

void usage(void)
{
    cout << " Usage:  " << endl;
    cout << " ./merge_4up -f [Filename] [-v] -nc [splits] -ts [timesplit] -lt [lasttime]" << endl;
    return;
}
int main(int argc, Char_t *argv[])
{
    cout << "Welcome to Merge_4up. Sorting data from each 4up. " << endl;

    bool ct_bug_hack_enabled(false);
    string filename;
    bool filenamespec(false);
    Int_t verbose = 0;
    Int_t ncuts = 0;
    Long64_t timeinterval=0;
    ModuleCal *event = new ModuleCal();
    ModuleCal *evt = new ModuleCal();
    ModuleCal *unsrt_evt=0;
    TTree* cal;
    Long64_t lasttime=-1;


    for(int ix = 1; ix < argc; ix++) {
        if(strncmp(argv[ix], "-h", 2) == 0) {
            usage();
            return(0);
        }

        if(strncmp(argv[ix], "-v", 2) == 0) {
            cout << "Verbose Mode " << endl;
            verbose = 1;
        }

        if(strncmp(argv[ix], "-nc", 3) == 0) {
            ncuts = atoi(argv[ix+1]);
            if (verbose) {
                cout << "Number of splits : " << ncuts <<endl;
            }
            ix++;
        }

        if(strncmp(argv[ix], "-ts", 3) == 0) {
            timeinterval = (Long64_t) atol(argv[ix+1]);
            if (verbose) {
                cout << "Split at time : " << timeinterval <<endl;
            }
            ix++;
        }

        if(strncmp(argv[ix], "-lt", 3) == 0) {
            lasttime = (Long64_t) atol(argv[ix+1]);
            if (verbose) {
                cout << "Last time : " << lasttime <<endl;
            }
            ix++;
        }

        if(strncmp(argv[ix], "-f", 2) == 0) {
            filename = string(argv[ix + 1]);
            filenamespec = true;
            ix++;
        }

        if(strncmp(argv[ix], "--hack", 6) == 0) {
            ct_bug_hack_enabled = true;
            if (verbose) {
                cout << "Course Timestamp hack enabled" << endl;
            }
        }
    }

    if (!filenamespec) {
        cout << " Please provide an input file. " << endl;
        usage();
        return(-2);
    }

    if (lasttime < 0) {
        cout << "Please specify last time as calculated by get_opt_split :: -lt [value]" << endl;
        return(-3);
    }

    if ((ncuts > 1) && (timeinterval == 0)) {
        cout << " Please specify the time at which to perform the splits:  -ts [timestamp] " << endl ;
        cout << " Exiting " << endl;
        return(-4);
    }

    TCanvas *c1;
    c1 = (TCanvas*)gROOT->GetListOfCanvases()->FindObject("c1");
    if (!c1) {
        c1 = new TCanvas("c1","c1",10,10,1000,1000);
    }


    rootlogon(verbose);


    size_t root_file_ext_pos(filename.rfind(".cal.root"));
    if (root_file_ext_pos == string::npos) {
        cerr << "Unable to find .cal.root extension in: \"" << filename << "\"" << endl;
        cerr << "...Exiting." << endl;
        return(-5);
    }
    string filebase(filename, 0, root_file_ext_pos);


    if (verbose) {
        cout << " filename = " << filename << endl;
        cout << " filebase = " << filebase << endl;
        cout << " Opening file " << filename << endl;
    }
    TFile * rootfile= new TFile(filename.c_str(),"OPEN");

    if (!rootfile->IsOpen()) {
        cout << "problems opening file " << filename;
        cout << "\n Exiting " << endl;
        return(-6);
    }

    if (verbose) {
        cout << " File Content : " << endl;
        rootfile->ls();
    }

    string treename("CalTree");

    cal = (TTree *) rootfile->Get(treename.c_str());
    if (!(cal))    {
        cout << " Problem reading " << treename << " from file. Exiting." << endl;
        return(-7);
    }

    cal->SetBranchAddress("Calibrated Event Data",&unsrt_evt);


    // Create Tree //
    Long64_t entries=cal->GetEntries();

    cal->GetEntry(0);
    Long64_t firsttime=unsrt_evt->ct;

    if (verbose) {
        cout << " Last timestamp = " << lasttime << endl;
    }



    if ( ncuts > MAXCUTS ) {
        cout << " ERROR ! THERE ARE OVER " << MAXCUTS*MAXHITS << " EVENTS TO PROCESS. " << endl;
        cout << " This is too many !\n Not even trying.\n Bye. " << endl;
        return -999;
    }

    if (verbose) {
        cout << " Anticipating " << ncuts << " iterations. " << endl;
    }

    Long64_t timecut[MAXCUTS];
    Long64_t partentries[MAXCUTS]= {0};

    Long64_t curtime=0;


    for (int k=0; k<ncuts; k++) {
        timecut[k]=firsttime+timeinterval*(k+1);
    }

    if (ncuts>0) {
        // make sure that timecut[ncuts-1] > last possible time; used to be:
        timecut[ncuts-1]=lasttime;
    } else {
        timecut[0]=lasttime;
    }

    if (verbose) {
        for (int k=0; k<ncuts; k++) {
            cout << " Time cut " << k << ": " << timecut[k]  << endl;
        }
    }

    // do 2 criterion   and while loop  :: while  !((criteria1) and (criteria2))
    // criteria1 set to false if  (firsttime >  timecut ),
    // criteria2 is set to false if criteria1 is false :
    // here we'll look for a number of events past the event criterion 1 was set
    // if the time of the event is smaller than the cutofftime, we fill the histogram,
    // IMPORTANT: on next iteration we need to go back to event where criterion 1
    // happened ( so it seems we need a dual check before filling if  time > prevcutoff && time > prevcutoff)
    int cut_number(0);
    int nrrollovers(0);
    Long64_t curevent(0);
    while ((curtime<lasttime)) {
        if (verbose) {
            cout << "\n Split number :: " << cut_number << ", timecut = " << timecut[cut_number] << endl;
        }
        time_t time_before_filling = clock();

        // Open Calfile //
        Char_t rootfile[FILENAMELENGTH];
        sprintf(rootfile,"%s_part%d.root",filebase.c_str(),cut_number);

        if (verbose) {
            cout << " Opening file " << rootfile << " for writing " << endl;
        }
        TFile *calfile = new TFile(rootfile,"RECREATE");
        TTree *panel = new TTree("panel","Sorted Panel Data") ;
        Long64_t lasteventtime=0;
        panel->SetDirectory(0);

        panel->Branch("Sorted data",&event);

        // this could be simple: we just split the file according to the timestamp
        // check whether timestamp[unsrt_evt.chip]

        Long64_t l(curevent);
        curtime=0;
        int stop=0;
        int extra(0);
        // setting cutime to 0 here ensures we don't skip the next while loop
        while (!(stop)) {
            cal->GetEntry(l);
            curtime=unsrt_evt->ct ;
            // if we get a negative number, skip
#ifdef DEBUG
            cout << " L : " << l << " curtime : " << curtime << " (= " << curtime/COARSECLOCKFREQUENCY << " s), lasteventtime : " << lasteventtime << " timecut[ " << cut_number << "] : " << timecut[cut_number] << endl;
#endif
            // note that 60e6 at 12 MHz is 5 seconds, unlikely to have more tha 5 s between events
            // note that 11e9 at 12 MHz is 5 seconds, unlikely to have more than 15 min between events
            // FIXME :: HARDCODED VALUE DEALING WITH CORRUPTED DATA FROM LEFT AVDB 2014/05/12
            if ((  TMath::Abs(curtime-lasteventtime)   > 600e6) && ( lasteventtime > 0 ) && (l>100) &&
                    ( (curtime -lasteventtime)<10e9) && (ct_bug_hack_enabled))
            {
                if (verbose) {
                    cout << " CT BUG CORRUPTED DATA:: Event " << l << ": curtime = " << curtime << ", lasteventtime = " << lasteventtime << endl;
                }
                l++;
                if (l>entries) {
                    stop=1;
                }
                continue;
            }

            if (((curtime - lasteventtime) > 10e9) && (lasteventtime > 0) && (ct_bug_hack_enabled)) {
                if (verbose) {
                    cout << " CT BUG :: Event " << l << ": curtime = " << curtime << ", lasteventtime = " << lasteventtime << endl;
                }
                l++;
                if (l > entries) {
                    stop=1;
                }
                continue;
            }

            if (( curtime+1e12 ) < lasteventtime ) {
                // rollover occured !!
                // FIXME :: only one rollover supported as of now.
                cout << " ROLLOVER ! curtime: " << curtime << " lasttime: " << lasteventtime << " l= " << l << endl;
                nrrollovers=1;
            }
            if (nrrollovers) {
                curtime+=nrrollovers*ROLLOVERTIME;
            }
#ifdef DEBUG
            cout << " thischiptime = " << curtime ; //<< endl;
#endif
            event->ct=curtime;
            event->Ecal=unsrt_evt->Ecal;
            event->ft=unsrt_evt->ft;
            event->Ec=unsrt_evt->Ec;
            event->E=unsrt_evt->E;
            event->Ech=unsrt_evt->Ech;
            event->x=unsrt_evt->x;
            event->y=unsrt_evt->y;
            event->fin=unsrt_evt->fin;
            event->m=unsrt_evt->m;
            event->cartridge=unsrt_evt->cartridge;
            event->apd=unsrt_evt->apd;
            event->id=unsrt_evt->id;
            event->pos=unsrt_evt->pos;

            Long64_t mintime(0);
            if (cut_number > 0) {
                mintime=timecut[cut_number-1];
            }
            if ((curtime <= timecut[cut_number]) && curtime > mintime) {
                panel->Fill();
            } else {
                // we're going to need to go back a number of steps
                extra++;
                if (( curtime-timecut[cut_number])>10000 ) {
                    if (verbose) {
                        cout << "STOPPING LOOP @ EVENT " << l << ": curtime - "
                             << "timecut[" << cut_number << "] : "
                             << curtime-timecut[cut_number] << endl;
                    }
                    stop=1;
                }
            }
            // we stop the while loop and close the file if the timedifference is larger than 10
#ifdef DEBUG
            cout << " filling chip " << unsrt_evt->chip << " MODULE " << event->m ;
            cout << " (l="<<l<<")"<< endl;
#endif
            l++;
            if (l >= entries) {
                stop=1;
            }
            if (l < 0) {
                l = 0; // safety
            }
            if ((verbose)&&(l>0)&&((l%5000000)==0)) {
                cout << " Processed " << l/1e6 << " million events " << endl;
            }
            lasteventtime = curtime;
        }

        if (verbose) {
            cout << " Extra entries :: " << extra << "; l = " << l << endl;
        }

        // the extra here is to prevent roll-off between splits
        l -= extra;
        curevent = l;
        if (verbose) {
            cout << " curevent = " << curevent << "; curtime = " << curtime << endl;
        }

        partentries[cut_number] = panel->GetEntries();
        cut_number++;

        time_t time_before_sorting=clock();
        // sorting

        Long64_t N = panel->GetEntries();

        Long64_t *table = new Long64_t[N];
        Long64_t *sorted = new Long64_t[N];

        for (Long64_t entry = 0; entry < N; entry++) {
            panel->GetEntry(entry);
            table[entry] = event->ct;
        }

        if (verbose) {
            cout << "Sorting Run numbers :: " ;    //<<std::endl;
        }

        TMath::Sort(N,table,sorted,kFALSE);

        time_t time_after_sorting=clock();

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
        
        time_t thistime=clock();

        for (Long64_t entry = 0; entry < N; entry++) {
            if ((((entry)%5000000)==0)&&(entry>0)&&(verbose))  {
                cout << entry << " entries processed; " ;
                cout << " time spent = " <<(double)  (clock()-thistime)/CLOCKS_PER_SEC ;
                cout << "; since start: " <<(double) (clock()-time_after_sorting)/CLOCKS_PER_SEC <<endl;
                thistime = clock();
            }

            panel->GetEntry(sorted[entry]);
            evt=event;
            fourup->Fill();
        }

        delete[] table;
        delete[] sorted;

        time_t time_after_filling=clock();

        if (verbose) {
            printf("Time to clone the Tree (sec): %f\n",(double)(time_after_filling-time_after_sorting)/(CLOCKS_PER_SEC));
        }


        fourup->Write();
        calfile->Close();

        delete panel;
        delete calfile;

        if (cut_number >= ncuts) {
            break;
        }
    }


    Long64_t allpartentries=0;
    cout << " Total entries ::" << entries << endl;
    for (int k = 0; k < ncuts; k++) {
        cout << " Entries part " << k << " : " << partentries[k] << endl;
        allpartentries+=partentries[k];
    }
    cout << " Total entries each part: " << allpartentries << "( = " << 100*allpartentries/entries  << " %)"<<endl;
    return(0);
} 
