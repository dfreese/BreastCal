/* AVDB 
 -- Chaining the split data file together -- 
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
 cout << " Chain_merged -f [filebase] -n [nrfiles] [-v]" <<endl;
  return;}


int main(int argc, char *argv[]){
  Char_t treename[40];
  Char_t filebase[MAXFILELENGTH];
  Char_t outfilename[MAXFILELENGTH];
  Char_t curfilename[MAXFILELENGTH];
  int i;
  int nrfiles=0;
  int verbose=0;

      Bool_t random=0;
      Int_t delay=0;

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

      if (strncmp(argv[i],"-r",2)==0) { 
	random=1;
      cout << " RANDOMS " << endl;        
      }


      if (strncmp(argv[i],"-d",2)==0) { 
        i++;
	delay=atoi(argv[i]);
	//      cout << " RANDOMS " << endl;        
      }


  } // argc
 

  if (random && (delay==0)) { cout << " Please specify delay for randoms.-r -d XX \n Exiting.\n"; return -2; }

  if (nrfiles==0) {
    cout << " Please specify nr of files.\n Exiting." <<endl;
    exit(-4);
  }

  TChain *m;
  Char_t chainname[40];

  if (random) sprintf(outfilename,"%s_all.delaywindow%d.merged.root",filebase,delay);
  else  sprintf(outfilename,"%s_all.merged.root",filebase);

  TFile *rfile = new TFile(outfilename,"RECREATE");

  sprintf(treename,"merged");
  sprintf(chainname,"merged");
  m = new TChain(treename,chainname);


for (i=0;i<=nrfiles;i++){
  if (random)  sprintf(curfilename,"%s_part%d.delaywindow%d.merged.root",filebase,i,delay);
  else  sprintf(curfilename,"%s_part%d.root",filebase,i);
   m->Add(curfilename);
	 }


 rfile->cd();

 m->Write();


rfile->Close();


  return 0;}
