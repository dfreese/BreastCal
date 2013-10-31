
#include "TROOT.h"
#include "Riostream.h"
#include "TTree.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TSpectrum.h"
#include "TSpectrum2.h"
#include "TH2F.h"
#include "TVector.h"
#include "TMath.h"
#include "TF1.h"
#include "Apd_Fit.h"
#include "./decoder.h"
#include "./CoincEvent.h"

//void usage(void);

//void usage(void){
// cout << " mergecal -fl [filename] -fr [filename] [-t [threshold] -v ]" <<endl;
//  return;}

int main(int argc, Char_t *argv[])
{
 	cout << "Welcome to merge_ana " << endl;
 	cout << "Energy gating and crystal range adjustments " << endl;

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	Char_t		filename[FILENAMELENGTH] = "";
	Int_t		verbose = 0, threshold=-1000;
	Int_t		ix,ascii;
        Bool_t          RANDOMS=0;
	//module UNIT0,UNIT1,UNIT2,UNIT3;
	//        event           evt;

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

        ascii=0;

	for(ix = 1; ix < argc; ix++) {

		/*
		 * Verbose '-v'
		 */
		if(strncmp(argv[ix], "-v", 2) == 0) {
			cout << "Verbose Mode " << endl;
			verbose = 1;
		}



		if(strncmp(argv[ix], "-r", 2) == 0) {
			cout << "RANDOMS SELECTION " << endl;
			RANDOMS = 1;
		}




		if(strncmp(argv[ix], "-a", 2) == 0) {
			cout << "Ascii output file generated" << endl;
			ascii = 1;
		}


		if(strncmp(argv[ix], "-t", 2) == 0) {
                  threshold = atoi( argv[ix+1]);
		  cout << "Threshold =  " << threshold << " ( not implemented yet ) " << endl;
                  ix++;
		}

		/* filename '-f' */
		if(strncmp(argv[ix], "-f", 3) == 0) {
			if(strlen(argv[ix + 1]) < FILENAMELENGTH) {
				sprintf(filename, "%s", argv[ix + 1]);
			}
			else {
				cout << "Filename " << argv[ix + 1] << " too long !" << endl;
				cout << "Exiting.." << endl;
				return -99;
			}
		}


	} // loop over arguments

        rootlogon(verbose);

        Int_t DTC_LOW = -4;
        Int_t DTC_HI = 4;


        Char_t filebase[FILENAMELENGTH],rootfile[FILENAMELENGTH]; 
        Char_t asciifile[FILENAMELENGTH]; 
	//       Char_t tmpname[20],tmptitle[50];
	//        Int_t i,j,k,m,lines;
	//        Double_t aa, bb;
	//        Int_t augment;
        ifstream infile;
	//        Int_t evts=0;
	//        strncpy(filebase,filename,strlen(filename)-17);
	//        filebase[strlen(filename)-17]='\0';
	//        sprintf(rootfile,"%s",filename);
	//        strcat(rootfile,".root");
              
	//	cout << "Rootfile to open :: " << rootfile << endl;


        cout << " Opening file " << filename << endl;
        TFile *file = new TFile(filename,"OPEN");
        TTree *m = (TTree *) file->Get("merged");
        CoincEvent *data=0;

	m->SetBranchAddress("Event",&data);


	ofstream asciiout;

	  // Open output file - matching required format for ALEX //
	strncpy(filebase,filename,strlen(filename)-5);
         filebase[strlen(filename)-5]='\0';
         sprintf(rootfile,"%s",filebase);
	 if (RANDOMS)  strcat(rootfile,".randoms.root");
	 else strcat(rootfile,".ana.root");
        if (ascii){
	  if (RANDOMS) sprintf(asciifile,"%s.randoms.ascii",filebase);
	  else sprintf(asciifile,"%s.ana.ascii",filebase);
          asciiout.open(asciifile);
	    }
        cout << " Opening file " << rootfile << " for writing " << endl;
	TFile *orf = new TFile(rootfile,"RECREATE");
	// OPEN YOUR OUTPUTFILE HERE ! 
     
  
        CoincEvent *evt = new CoincEvent();

        TTree *mana = new  TTree("mana","Merged and Calibrated LYSO-PSAPD data ");

	mana->Branch("Event",&evt);


       Long64_t entries_m = m->GetEntries();


       cout << " Processing " <<  entries_m  <<  " Events " << endl;

      Long64_t i;
      Long64_t pascut=0; 
      Double_t x1,y1,z1,x2,y2,z2;

      for (i=0;i<entries_m;i++){

	m->GetEntry(i);

	// YOUR CODE HERE -- YOU HAVE ACCESS TO THE STRUCT DATA AND ITS MEMBERS TO DO WHATEVER CONSTRAINTS //
            
        if ( ( (data->dtc > DTC_LOW) && ( data->dtc < DTC_HI ) ) || (RANDOMS) ) {  

	  if ( ( data->E1<700 ) && (data->E1> 400 ) ) {
              if ( ( data->E2<700 ) && (data->E2> 400 ) ) {
		    if ( (data->crystal1>-1 ) && (data->crystal1<64 )) {
           		    if ( (data->crystal2>-1 ) && (data->crystal2<64 )) {

			      evt=data;
                              mana->Fill();
                              pascut++;
#define PANELDISTANCE 60 // mm

                              y1 = PANELDISTANCE/2;
                              y2 = -PANELDISTANCE/2;
                              y1 +=   data->apd1*10;
                              y2 -=   data->apd2*10;
			      y1 +=  (( TMath::Floor(data->crystal1%8)- 4 )*0.5  );
			      y2 +=  (( TMath::Floor(data->crystal1%8)- 4 )*0.5  );                              
			
			      x1 = (data->m1-8)*0.405*25.4;  
                              x2 = (8-data->m2)*0.405*25.4;  
			      x1 +=  (( TMath::Floor(data->crystal1/8)- 4 )*0.5  );
                              x2 +=  (( 4 - TMath::Floor(data->crystal1/8))*0.5  );
                              
                              z1 = data->fin1*0.056*25.4;
          		      z2 = data->fin2*0.056*25.4;
              
			      if (ascii) {
	     if (ascii){
	       asciiout << evt->dtc << " " <<evt->dtf << " ";
               asciiout << evt->chip1 << " " << evt->m1 << " " << evt->apd1 << " " << evt->crystal1 << " ";
               asciiout << evt->E1 << " " << evt->Ec1 << " " << evt->Ech1 << " " << evt->ft1 << " ";
               asciiout << evt->chip2 << " " << evt->m2 << " " << evt->apd2 << " " << evt->crystal2 << " ";
               asciiout << evt->E2 << " " << evt->Ec2 << " " << evt->Ech2 << " " << evt->ft2 << " ";
               asciiout << evt->x1 << " " << evt->y1  << " " << evt->x2 << "  " << evt->y2 << "  " << evt->pos << "  ";
               asciiout << evt->fin1 << "  " << evt->fin2  << endl;

			   // FLAU edited this to print out position.
			   // FLAU edited this to print out fin number (Dec2,2012)
	     }


			      }// if ascii                      
 
             // this is a good data now ..
             // do something+ write to disk


		  }
         	  }
	}
	  }
	} 
           
          
      } // loop over entries
             

#define COARSEDIFF 100       

      mana->Write();

      orf->Close();  
  
      cout << " Wrote file " << filename << " with " << pascut << " events." << endl;

      //       calfile->Close();

      if (ascii){
        asciiout.close();
      }

 
	  return 0;}


