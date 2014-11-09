//#define DEBUG 

#include <stdio.h>
#include <stdlib.h>

#include <iostream>

#include "TROOT.h"
#include "TFile.h"
#include "TNetFile.h"
#include "TRandom.h"
#include "TTree.h"
#include "Riostream.h"
#include "TMath.h"
#include "TChain.h"
#include "./decoder.h"


void usage(void) {
    cout << " Program that chains individually sorted and egated files" << endl;
    cout << " Chain_cal -f [filebase] -n [nrfiles] [-v] -start [firstfilenr] -of [outputfile]" <<endl;
    return;
}


int main(int argc, char *argv[]){
    string treename("SCalTree");
    string chain_name("SCalTree");


    string filebase;
    string outfilename;
    Char_t curfilename[MAXFILELENGTH];
    bool outputfilespec(false);

    int nrfiles=0;
    int verbose=0;
    int firstfile=0;

    for (int i=0;i<argc;i++) {
        if (strncmp(argv[i],"-f",2)==0) {
            filebase = string(argv[i + 1]);
            cout << " Using Filebase " << filebase << ". ";
            i++;
        }

        if (strncmp(argv[i],"-of",3)==0) {
            outfilename = string(argv[i + 1]);
            outputfilespec=true;
            i++;
        }


        if (strncmp(argv[i],"-n",2)==0) { 
            nrfiles = atoi(argv[i+1]);
            cout << nrfiles  << " files to merge. " ;
            i++;
        }

        if (strncmp(argv[i],"-v",2)==0) { 
            verbose=1;
            cout << " Verbose mode. ";        
        }

        if (strncmp(argv[i],"-start",6)==0) { 
            firstfile=atoi(argv[i+1]); 
            i++;
        }

        if (strncmp(argv[i],"-t",2)==0) {
            treename = string(argv[i + 1]);
            i++;
        }
    }

    cout << endl;

    if (nrfiles==0){
        usage();
        cout << " Please specify nr of files.\n Exiting." <<endl;
        exit(-4);
    }

    if (verbose){
        cout << " Reading " << nrfiles << " files starting from " << filebase << "_" << firstfile << ".dat.cal.root" << endl;
    }


    if (!outputfilespec) {
        outfilename = filebase + ".root";
    }

    if (verbose) {
        cout << "Outfile: " << outfilename << endl;
    }

    if (verbose) {
        cout << "Creating Root File" << endl;
    }
    TFile *rfile = new TFile(outfilename.c_str(),"RECREATE");

    if (verbose) {
        cout << "Creating TChain" << endl;
    }
    TChain *m = new TChain(treename.c_str(), treename.c_str());

    for (int i = 0; i < nrfiles; i++) {
        sprintf(curfilename,"%s_%d.dat.cal.sort.egate.root",filebase.c_str(),i+firstfile);
        if (verbose) {
            cout << "Adding: " << curfilename << endl;
        }
        m->Add(curfilename);
    }

    rfile->cd();

    m->Write();

    rfile->Close();
}
