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
#include "TKey.h"
#include "TFile.h"
#include "TNetFile.h"
#include "TRandom.h"
#include "TTree.h"
#include "Riostream.h"
#include "TMath.h"
#include "TNtuple.h"
#include "TH2F.h"
#include "TChain.h"
#include "TVector.h"
#include "./decoder.h"

//#include "./convertconfig.h"
void usage(void);


void usage(void){
 cout << " Chain parsed -f [filelist]  -o [outputfilename] " <<endl;
  return;}


int main(int argc, char *argv[]){
  Char_t filelist[MAXFILELENGTH];
  Char_t outfilename[MAXFILELENGTH];
  Char_t curfilename[MAXFILELENGTH];
  Char_t treename[40];
  int i;
  Bool_t verbose=0;
  Bool_t ofilenamespec=kFALSE;
  for (i=0;i<argc;i++) {

    if (strncmp(argv[i],"-v",2)==0) {
      verbose=kTRUE;
      
    }

    if (strncmp(argv[i],"-f",2)==0) {
      sprintf(filelist,"%s",argv[i+1]);
      i++;
    }
      if (strncmp(argv[i],"-o",2)==0) { 
      sprintf(outfilename,"%s",argv[i+1]);
      ofilenamespec = kTRUE;
      //      cout << " outputfile = " << outfilename << endl;        
      i++;
      }

   }

  cout << " Chaining ROOT files; output file generated will be : " << outfilename << endl;

  // making dummy histogram according to http://root.cern.ch/phpBB3/viewtopic.php?f=3&t=13607
  TH1F* dummy = new TH1F("dummy", "dummy", 10, 0, 1);

  ifstream chainfile,rootfile;
  chainfile.open(filelist);

  TChain *mdata;
  Char_t chainname[40];


  if (!ofilenamespec) {
      cout << " Please specify an outputfilename " << endl;
      usage();
      return -99;
    }
  TFile *rfile = new TFile(outfilename,"RECREATE");

  //  for (i=0;i<RENACHIPS;i++){
  sprintf(treename,"mdata");
  sprintf(chainname,"mdata");
  mdata = new TChain(treename,chainname);
  //}

  Int_t  filesread=0;
  
  TH1F *ETMP[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE];
  TH1F *ETMP_com[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE];

  TFile *decodedfile;
  Char_t filename[50];
  Char_t tmpstring[50];


  while (chainfile >> curfilename) {   // FLAU edited, see note below

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

        filesread++;
        if (verbose) cout << " Files read :: " << filesread << ". Current file : " << curfilename << endl; 

	//  cout << " ListOfKeys :: "  <<  gDirectory->GetListOfKeys()  <<endl;


        mdata->Add(curfilename);

	
       	decodedfile = new TFile(curfilename,"UPDATE");

        if (!decodedfile || decodedfile->IsZombie()) {  
         cout << "problems opening file " << filename << "\n.Exiting" << endl; 
         return -11;}
 
	// FIXME:: Read circle centers from first file - should have a way to check if there are enough entries to have reliable centers. 

        if (verbose) cout << " Reading circle centers " << endl;
	
	if ( filesread == 1 ) {
	  for (Int_t c=0;c<CARTRIDGES_PER_PANEL;c++){ 
	    for (Int_t r=0;r<FINS_PER_CARTRIDGE;r++){
              sprintf(tmpstring,"timing/uu_c[%d][%d]",c,r);
	      uu_c[c][r] = (TVector *) decodedfile->Get(tmpstring);
              sprintf(tmpstring,"timing/vv_c[%d][%d]",c,r);
	      vv_c[c][r] = (TVector *) decodedfile->Get(tmpstring);
	    } //r
	  } //c
	}// filesread

        if (verbose) cout << " Reading histograms " << endl;
	
	//    decodedfile->ls();        
	  for (Int_t c=0;c<CARTRIDGES_PER_PANEL;c++){ 
	    for (Int_t f=0;f<FINS_PER_CARTRIDGE;f++){
	      for (Int_t m=0;m<MODULES_PER_FIN;m++){
		for (Int_t j=0;j<APDS_PER_MODULE;j++){
		  sprintf(tmpstring,"C%dF%d/E[%d][%d][%d][%d]",c,f,c,f,m,j);  
		  //		  if (verbose) cout <<  tmpstring  << endl;
		  if (filesread==1) { 
		    E[c][f][m][j]= (TH1F *) decodedfile->Get(tmpstring); 
		    E[c][f][m][j]->SetDirectory(0);
                   }
		  else { 
		    ETMP[c][f][m][j]= (TH1F *) decodedfile->Get(tmpstring); 
	            if (ETMP[c][f][m][j]) E[c][f][m][j]->Add(ETMP[c][f][m][j],1);
		    delete ETMP[c][f][m][j]; //}
		  }
		  sprintf(tmpstring,"C%dF%d/E_com[%d][%d][%d][%d]",c,f,c,f,m,j);
		  //		  if (verbose) cout <<  tmpstring << endl ;
		  if (filesread==1) { 
		    E_com[c][f][m][j]= (TH1F *) decodedfile->Get(tmpstring); 
		    E_com[c][f][m][j]->SetDirectory(0); }
		  else { 
		    ETMP_com[c][f][m][j]= (TH1F *) decodedfile->Get(tmpstring);  
		    if (ETMP_com[c][f][m][j]) E_com[c][f][m][j]->Add(ETMP_com[c][f][m][j]);
		    delete ETMP_com[c][f][m][j];
		  }
		} //j
	      } // m
	    } // f
	 }//c
	
	  if (verbose) cout << " Closing file " << curfilename << endl;

	 decodedfile->Close();

    //	 }
  } // while .. 

  chainfile.close();
 
  if (verbose) cout << " Writing to root file " << endl;

  rfile->cd();

 //	 for (i=0;i<RENACHIPS;i++){


	   mdata->Write();
	   
	   

	   for (Int_t c=0;c<CARTRIDGES_PER_PANEL;c++){
	     for (Int_t f=0;f<FINS_PER_CARTRIDGE;f++) {
	       sprintf(tmpstring,"C%dF%d",c,f);
	       subdir[c][f] = rfile->mkdir(tmpstring);
	       subdir[c][f]->cd();
	       for (Int_t m=0;m<MODULES_PER_FIN;m++){
		 for (Int_t  j=0;j<APDS_PER_MODULE;j++){
		   E[c][f][m][j]->Write();
		   E_com[c][f][m][j]->Write();
		 } //j
	       } //m
	     } //f
	   } // c

	   
	   rfile->cd();

	   if (verbose) cout << " Writing U,V centers to roofile " << endl;
	   TDirectory *timing =  rfile->mkdir("timing");

    
	   timing->cd();
	   
   // need to store uvcenters ::
	   for (Int_t c=0;c<CARTRIDGES_PER_PANEL;c++){
	     for (Int_t r=0;r<FINS_PER_CARTRIDGE;r++){
		 if(verbose)	 cout << " C" << c<< " F" <<r << endl;
	       sprintf(tmpstring,"uu_c[%d][%d]",c,r);
	       uu_c[c][r]->Write(tmpstring);
	       sprintf(tmpstring,"vv_c[%d][%d]",c,r);
	       vv_c[c][r]->Write(tmpstring);
	     } //r
	   } //c
	   

	   
rfile->Close();


  return 0;}
