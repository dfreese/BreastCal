/* AVDB 4-25-11 
   Conversion Program, parsing RENA data to some kind of module data, and pedestal subtraction 
   Function takes tree input arguments:
   -v  :: verbosity switch
   -f  :: inputfilename
   -o  :: outputfilename
   -p  :: pedestal file list
   only the -f switch is necessary ! 

  AVDB 8-8-12
  We are now reading an entire 4-up board
 
  AVDB 8-17-12
  Adjusting the code for PSF. 
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

//#include "./convertconfig.h"
void usage(void);


void usage(void){
 cout << " Chain parsed -f [filelist] [ -o [outputfilename] ]" <<endl;
  return;}


int main(int argc, char *argv[]){
  Char_t filelist[MAXFILELENGTH];
  Char_t outfilename[MAXFILELENGTH];
  Char_t curfilename[MAXFILELENGTH];
  Char_t treename[40];
  int i;
  for (i=0;i<argc;i++) {


    if (strncmp(argv[i],"-f",2)==0) {
      sprintf(filelist,"%s",argv[i+1]);
      i++;
    }
      if (strncmp(argv[i],"-o",2)==0) { 
      sprintf(outfilename,"%s",argv[i+1]);
      cout << " outputfile = " << outfilename << endl;        
      i++;
      }

   }

 
  ifstream f,rootfile;
  f.open(filelist);

  //  TChain *b[RENACHIPS];
  TChain *mdata;
  Char_t chainname[40];



  TFile *rfile = new TFile(outfilename,"RECREATE");

  //  for (i=0;i<RENACHIPS;i++){
  sprintf(treename,"mdata");
  sprintf(chainname,"mdata");
  mdata = new TChain(treename,chainname);
  //}


  while (f >> curfilename) {   // FLAU edited, see note below

    // -------------------------------------------------------	  
	// FLAU edited to fix bug, and commented the below out
	// if use f.good() to check if end of file, sometimes reads last line twice 
	// See: http://stackoverflow.com/questions/4324441/testing-stream-good-or-stream-eof-reads-last-line-twice
	// which says: "All of the stream state functions – fail, bad, eof, and good – tell you the 
	// current state of the stream rather than predicting the success of a future operation. Check the 
	// stream itself (which is equivalent to an inverted fail check) after the desired operation
	// Also, it's best to avoid infinite loops and break whenever possible
      
	//while (1){
	  //if  (!(f.good())) break;
	  //f >> curfilename;  // FLAU commented out
    // -------------------------------------------------------	  

	 //	 rootfile.open(curfilename) ;
	 //         if !(rootfile.valid()) {
	 //	     cout << curfilename << " invalid file name ! " << endl;
	 //             continuel;
	 //	   }
    //	 for (i=0;i<RENACHIPS;i++){
	   mdata->Add(curfilename);
	   //	 }
 }

f.close();

 rfile->cd();

 //	 for (i=0;i<RENACHIPS;i++){
	   mdata->Write();
	   //	 }


rfile->Close();


  return 0;}
