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

  } // argc
 

  if (nrfiles==0) {
    cout << " Please specify nr of files.\n Exiting." <<endl;
    exit(-4);
  }

  TChain *m;
  Char_t chainname[40];

  sprintf(outfilename,"%s_all.merged.root",filebase);

  TFile *rfile = new TFile(outfilename,"RECREATE");

  sprintf(treename,"merged");
  sprintf(chainname,"merged");
  m = new TChain(treename,chainname);


for (i=0;i<=nrfiles;i++){
  sprintf(curfilename,"%s_part%d.merged.root",filebase,i);
   m->Add(curfilename);
	 }


 rfile->cd();

 m->Write();


rfile->Close();


  return 0;}
