/* AVDB 
 -- In case individual decoded data is calibrated, this program will merge everything back together -- 
*/

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
#include "TNtuple.h"
#include "TH2F.h"
#include "TChain.h"
#include "./decoder.h"


void usage(void);


void usage(void){
  cout << " Program that chains the different coincidence files " << endl;
 cout << " Chain_cal -f [filebase] -n [nrfiles] [-v] -start [firstfilenr]" <<endl;
  return;}


int main(int argc, char *argv[]){
  Char_t treename[40];
  Char_t filebase[MAXFILELENGTH];
  Char_t outfilename[MAXFILELENGTH];
  Char_t curfilename[MAXFILELENGTH];
  int i;
  int nrfiles=0;
  int verbose=0;
  int firstfile=0;

  for (i=0;i<argc;i++) {
    if (strncmp(argv[i],"-f",2)==0) {
      sprintf(filebase,"%s",argv[i+1]);
      i++;
    }
      if (strncmp(argv[i],"-n",2)==0) { 
	nrfiles = atoi(argv[i+1]);
	cout << nrfiles+1  << " files to merge " << endl;
      i++;
      }

      if (strncmp(argv[i],"-v",2)==0) { 
	verbose=1;
      cout << " verbose mode " << endl;        
      }

      if (strncmp(argv[i],"-start",6)==0) { 
        firstfile=atoi(argv[i+1]); 
        i++;
      }


  }  // argc
 


  if (nrfiles==0){
    usage();
    cout << " Please specify nr of files.\n Exiting." <<endl;
    exit(-4);
  }

  if (verbose){
    cout << " Reading " << nrfiles << " files starting from " << filebase << "_" << firstfile << ".dat.cal.root" << endl;
  }

  TChain *m;
  Char_t chainname[40];

  sprintf(outfilename,"%s.cal.root",filebase);

  TFile *rfile = new TFile(outfilename,"RECREATE");

  sprintf(treename,"CalTree");
  sprintf(chainname,"CalTree");
  m = new TChain(treename,chainname);


for (i=0;i<=nrfiles;i++){

  sprintf(curfilename,"%s_%d.dat.cal.root",filebase,i+firstfile);
   m->Add(curfilename);
	 }


 rfile->cd();

 m->Write();


rfile->Close();


  return 0;}
