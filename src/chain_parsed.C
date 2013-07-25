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
      //      cout << " outputfile = " << outfilename << endl;        
      i++;
      }

   }

 
  cout << " Chaining ROOT files; output file generated will be : " << outfilename << endl;

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

  Int_t  filesread=0;
  TH1F *E[RENACHIPS][MODULES][2];
  TH1F *E_com[RENACHIPS][MODULES][2];
  
  TH1F *ETMP[RENACHIPS][MODULES][2];
  TH1F *ETMP_com[RENACHIPS][MODULES][2];
  TFile *decodedfile;
  Char_t filename[50];
  Char_t tmpstring[50];

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

        filesread++;
        cout << " Files read :: " << filesread << ". Current file : " << curfilename << endl; 

	//  cout << " ListOfKeys :: "  <<  gDirectory->GetListOfKeys()  <<endl;


        mdata->Add(curfilename);

	
       	decodedfile = new TFile(curfilename,"UPDATE");

        if (!decodedfile || decodedfile->IsZombie()) {  
         cout << "problems opening file " << filename << "\n.Exiting" << endl; 
         return -11;}

	//    decodedfile->ls();        
        for (int kk=0;kk<RENACHIPS;kk++){
          for (int j=0;j<2;j++){
            for (i=0;i<4;i++){
	      sprintf(tmpstring,"E[%d][%d][%d]",kk,i,j); //  cout <<  tmpstring  ;
             if (filesread==1) { 
                E[kk][i][j]= (TH1F *) decodedfile->Get(tmpstring); 
                E[kk][i][j]->SetDirectory(0);
                   }
             else { 
	       //   TKey *key2 = (TKey*)gDirectory->GetListOfKeys()->FindObject(E[kk][i][j]->GetName());
	       //     if (key2) { cout << "key found" << endl;
		 	       ETMP[kk][i][j]= (TH1F *) decodedfile->Get(tmpstring); 
	       //	 ETMP[kk][i][j]= (TH1F *) key2->ReadObj();
			       //		       cout << " " << ETMP[kk][i][j]->GetEntries() << " " << E[kk][i][j]->GetEntries() ;
			       E[kk][i][j]->Add(ETMP[kk][i][j],1);
			       //   cout << " " << E[kk][i][j]->GetEntries() <<endl;
	       delete ETMP[kk][i][j]; //}
               }
             sprintf(tmpstring,"E_com[%d][%d][%d]",kk,i,j);
             if (filesread==1) { 
                 E_com[kk][i][j]= (TH1F *) decodedfile->Get(tmpstring); 
                 E_com[kk][i][j]->SetDirectory(0); }
             else { 
	       ETMP_com[kk][i][j]= (TH1F *) decodedfile->Get(tmpstring);  
	       E_com[kk][i][j]->Add(ETMP_com[kk][i][j]);
	        delete ETMP_com[kk][i][j];
               }
     	      }
            } // j
	 }//kk

        decodedfile->Close();

	   //	 }
	   }

  f.close();
 TDirectory *subdir[RENACHIPS];
 rfile->cd();

 //	 for (i=0;i<RENACHIPS;i++){
	   mdata->Write();
	   
            for (int kk=0;kk<RENACHIPS;kk++){
             sprintf(tmpstring,"RENA%d",kk);
             subdir[kk] = rfile->mkdir(tmpstring);
             subdir[kk]->cd();
             for (int j=0;j<2;j++){
               for (int i=0;i<4;i++){
	         E[kk][i][j]->Write();
	         E_com[kk][i][j]->Write();
                 }
	        } // j
          }//kk     
	   
	   //	 }


rfile->Close();


  return 0;}
