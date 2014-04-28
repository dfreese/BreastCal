/*************
  
   12-12 :: AVDB :: going to estimate lasttime for each 2-chip section,
   Events are not time ordered up to now.

        Perhaps I should try a Sort nonetheless  

  Events are only generally time ordered, but it should be enough to estimate the split level

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
#include "Apd_Fit.h"
//#include "Apd_Peaktovalley.h"
#include "./decoder.h"
#include "./ModuleCal.h"
#include "time.h"


int main(int argc, Char_t *argv[])
{
  	cout << "Welcome " << endl;

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	Char_t		filenamel[FILENAMELENGTH] = "";
	Int_t		verbose = 0;
	Int_t		ix;

	//  Int_t    left=-9999;
	//        modulecal       UL0,UL1,UL2,UL3;

        TTree*           cal;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

     

	for(ix = 1; ix < argc; ix++) {

		if(strncmp(argv[ix], "-h", 2) == 0) {
			cout << " Usage:  " << endl;
//                        cout << " ./merge_panel -f [Filename] --L/--R [-v] " << endl;
                   cout << " ./merge_panel -f [Filename] [-v] " << endl;
//                        cout << " Specify which panel: --L for left; --R for right " << endl;
			return -1;
		}


		/*
		 * Verbose '-v'
		 */
		if(strncmp(argv[ix], "-v", 2) == 0) {
			cout << "Verbose Mode " << endl;
			verbose = 1;
		}


		/* filename '-f' */
		if(strncmp(argv[ix], "-f", 2) == 0) {
			if(strlen(argv[ix + 1]) < FILENAMELENGTH) {
				sprintf(filenamel, "%s", argv[ix + 1]);
			}
			else {
				cout << "Filename " << argv[ix + 1] << " too long !" << endl;
				cout << "Exiting.." << endl;
				return -99;
			}
			ix++;
                    }
		
/*
                if (strncmp(argv[ix],"--L",3) ==0 ){
		  cout << " Left panel used " << endl;
                  left=1;
		}

                if (strncmp(argv[ix],"--R",3) ==0 ){
                  if (left==1) { cout << "Please specify --l OR --r not both !\n Exiting.\n"; return -99;}
		  cout << " Right panel used " << endl;
                  left=0;

		  }*/
	}
/*
	if (left==-9999) {
          cout << "Please specify which panel we're using: Add --L or --R to command line"  << endl;  return -1;}         
*/

             TCanvas *c1;
             c1 = (TCanvas*)gROOT->GetListOfCanvases()->FindObject("c1");
             if (!c1) c1 = new TCanvas("c1","c1",10,10,1000,1000);


        rootlogon(verbose);


        Char_t filebase[FILENAMELENGTH];
	//        Int_t m;
        ifstream infile;
        strncpy(filebase,filenamel,strlen(filenamel)-9);
        filebase[strlen(filenamel)-9]='\0';
        
	cout << " filename = " << filenamel << endl;
	cout << " filebase = " << filebase << endl;


        cout << " Opening file " << filenamel << endl;
        TFile *file_left = new TFile(filenamel,"OPEN");

        if (!file_left->IsOpen()) 
	  { cout << "problems opening file " << filenamel ;
            cout << "\n Exiting " << endl; 
            return -11;}


	if (verbose){
	cout << " File Content : " << endl;
      	file_left->ls();
        }
        

        ModuleCal *calevent=0;
        Long64_t entries;
        Long64_t lasttime=0;
        Long64_t maxentries=0;
        Long64_t totentries=0;
        Int_t maxchip=0;
        Char_t treename[40];
	//        for (m=0;m<RENACHIPS;m++){


	  sprintf(treename,"CalTree");
          cal = (TTree *) file_left->Get(treename);

          if (verbose) { cal->Print();}
          if (!(cal))    {
	    cout << " Problem reading " << treename << " from file." << endl;
	    //          continue;
	  }

	  if (verbose)	  cal->GetListOfBranches()->Print(); 

       cal->SetBranchAddress("Calibrated Event Data",&calevent);

       // cal->SetBranchAddress("ct",&calevent.ct);
       //       cal->SetBranchAddress("chip",&calevent.chip);
	  /*
        cal[m]->SetBranchAddress("U0",&UL[m*4+0]);
        cal[m]->SetBranchAddress("U1",&UL[m*4+1]);
        cal[m]->SetBranchAddress("U2",&UL[m*4+2]);
        cal[m]->SetBranchAddress("U3",&UL[m*4+3]);
	  */
        // Create Tree //
        entries=cal->GetEntries();
        cout << " Entries " << treename << ": " << entries << endl; 
        totentries+=entries;
        if (entries>maxentries) {maxentries=entries;maxchip=calevent->chip;}

	// 	} // loop m

	// Long64_t lasttimes;
 bool rollover=false;
 Int_t skipchip;
 // Long64_t l;
 Long64_t curevent;

 //        for (m=0;m<RENACHIPS;m++){
        if(entries) {
	  cal->GetEntry(entries-1);
          cout << " Last Timestamp Chip "  << calevent->chip  << ": " << calevent->ct << endl;
          
          maxchip=calevent->chip;
          lasttime=calevent->ct;
	  skipchip=0;
          curevent=0;
         }
        else skipchip=1;
	//}  loop over m


	Long64_t maxtime;
	maxtime=cal->GetMaximum("ct");
        cout << " Maxtime :: " << maxtime << endl;
 
	totentries=entries;

        cout << " Last timestamp = " << lasttime << endl;
        cout << " Total Entries : " << totentries << endl;
        cout << " Maximum entries : " << maxentries << " (chip " << maxchip << ")." << endl;	

       if ( totentries==0 ) {
          cout << " Timestep = " << 0 << endl;
          cout << " FINDME::: Timestep: " << 0 << " Splits: " << 0 << " Last: " << 0 << endl;
	  cout << " No entries in TFile " <<  filenamel ;
          cout << " .\n Exiting." << endl;
          return 0;
         }

       // note it could be that the last time stamp is slightly smaller than a previous one, because the events aren't sorted.
       if ( maxtime > ( lasttime + 100000 )) {
	 cout << " Rollover occured ! " << endl; 
       rollover=true; }
 


        Int_t cuts = TMath::Ceil((Double_t) totentries/MAXHITS);

        if ( cuts > MAXCUTS ) {
          cout << " ERROR ! THERE ARE OVER " << MAXCUTS*MAXHITS << " EVENTS TO PROCESS. " << endl;
          cout << " This is too many !\n Not even trying.\n Bye. " << endl;
          return -999;}


        if (rollover) {
	}

        cout << " Anticipating " << cuts << " iterations. " << endl;
        
        Int_t k;
        Long64_t timecut[MAXCUTS];
	for (k=0;k<cuts;k++) {
          cal->GetEntry(TMath::Min((Long64_t) (k+1)*MAXHITS,entries-1));
          timecut[k]=calevent->ct;
	  cout << " Time cut " << k << ": " << timecut[k]  << endl;}


      
       
	//	Long64_t timestep=48e6;

 	//        lasttime=1e10;
	//	timestep=TMath::Floor(lasttime/10); //1e10;
	//	            timestep=TMath::Floor(lasttime/1000); //1e10;
	//FIXME :: only supports one rollovertime currently, which as of 12/12/12 is 2 * 101.8 hrs ::
        // TMath::Power(2/42)/ ( 12e6*3600)  -- 42 bit timestamp and 12 MHz clock ( 48 MHz but we omit
        // the 2 LSB  
	if (rollover) { lasttime+=ROLLOVERTIME;}
          cout << " Total time : " << lasttime << endl;
	  cout << " Timestep   : " << lasttime/cuts << endl;
          cout << " FINDME::: Timestep: " << lasttime/cuts << " Splits: " << cuts << " Last: ";
          cout << lasttime << endl;
  

        return 0; }
