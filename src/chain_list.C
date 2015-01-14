//#define DEBUG 

#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <fstream>

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
    cout << " Program that chains the different coincidence files " << endl;
    cout << " chain_list -f [filelist] [-v] -of [outputfile] -c [chain name: default=merged]" <<endl;
    return;
}


int main(int argc, char *argv[]){
    string chain_name("merged");

    string file_list_name;
    string outfilename;
    bool outputfilespec(false);

    int verbose=0;

    for (int i=0;i<argc;i++) {
        if (strncmp(argv[i],"-f",2)==0) {
            file_list_name = string(argv[i + 1]);
            i++;
        }
        if (strncmp(argv[i],"-of",3)==0) {
            outfilename = string(argv[i + 1]);
            outputfilespec=true;
            i++;
        }
        if (strncmp(argv[i],"-v",2)==0) { 
            verbose=1;
            cout << " Verbose mode. ";        
        }
        if (strcmp(argv[i], "-h") == 0) {
            usage();
            return(0);
        }
        if (strncmp(argv[i],"-c",2)==0) {
            chain_name = string(argv[i + 1]);
            i++;
        }
    }
    cout << endl;

    if (!outputfilespec) {
        cerr << "Chained TFile name not specified.  Exiting" << endl;
        return(-1);
    }

    if (verbose) {
        cout << "Creating Root File" << endl;
    }
    TFile *rfile = new TFile(outfilename.c_str(),"RECREATE");

    if (verbose) {
        cout << "Creating TChain: " << chain_name << endl;
    }
    TChain *m = new TChain(chain_name.c_str(), chain_name.c_str());

    if (verbose) {
        cout << "Opening Filelist" << endl;
    }
    ifstream file_list(file_list_name.c_str());

    if (!file_list) {
        cerr << "Unable to open filelist" << endl;
        return(-2);
    }

    string file_line;
    while (getline(file_list, file_line)) {
        if (verbose) {
            cout << "Adding: " << file_line << endl;
        }
        m->Add(file_line.c_str());
    }

    rfile->cd();
    m->Write();
    rfile->Close();
    return(0);
}
