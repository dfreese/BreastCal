
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
#include "TPaveText.h"
#include "apd_fit.h"
#include "./decoder.h"


int main(int argc, Char_t *argv[])
{
 	cout << "Welcome to glob fit results " << endl;

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	Char_t		filename[FILENAMELENGTH] = "";
	Int_t		verbose = 0;//, threshold=-800;
	Int_t		ix;
        Int_t       dorefit=0;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	//see eg http://root.cern.ch/phpBB3/viewtopic.php?f=14&t=3498
	TROOT:gErrorIgnoreLevel=1001;
	for(ix = 1; ix < argc; ix++) {

		/*
		 * Verbose '-v'
		 */
		if(strncmp(argv[ix], "-v", 2) == 0) {
			cout << "Verbose Mode " << endl;
			gErrorIgnoreLevel=-1;
			verbose = 1;
		}

		if(strncmp(argv[ix], "-r", 2) == 0) {
			cout << "refit " << endl;
			dorefit=1;
		      
		}

		/* filename '-f' */
		if(strncmp(argv[ix], "-f", 2) == 0) {
			if(strlen(argv[ix + 1]) < FILENAMELENGTH) {
				sprintf(filename, "%s", argv[ix + 1]);
			}
			else {
				cout << "Filename " << argv[ix + 1] << " too long !" << endl;
				cout << "Exiting.." << endl;
				return -99;
			}
		}
	}

        rootlogon(verbose);



        Char_t filebase[FILENAMELENGTH],peaklocationfilename[FILENAMELENGTH],rootfile[FILENAMELENGTH]; 
        Char_t tmpname[20];
        Int_t i,j,k,m;
        Double_t globeres[RENACHIPS][4][2];
        Double_t globeres_e[RENACHIPS][4][2];
        ifstream infile;
        TH1F *globhist[RENACHIPS][4][2];
        TF1 *globfits[RENACHIPS][4][2];

	//        strncpy(filebase,filename,strlen(filename)-17);
	//        filebase[strlen(filename)-17]='\0';
        sprintf(rootfile,"%s",filename);
	//        strcat(rootfile,".root");
              
	cout << "Rootfile to open :: " << rootfile << endl;

	TFile *enefile = new TFile(rootfile,"OPEN");       
         if (!enefile || enefile->IsZombie()) {  
         cout << "problems opening file " << rootfile << "\n.Exiting" << endl; 
         return -11;}



        TVector* ppVals = (TVector *) enefile->Get("TVectorT<float>");

		enefile->ls() ;

        strncpy(filebase,filename,strlen(filename)-12);
        filebase[strlen(filename)-12]='\0';

	/* Getting Glob hists */
      Char_t tmpstring[30];
      for (m=0;m<RENACHIPS;m++){
           for (i=0;i<4;i++){
           for (j=0;j<2;j++){ 
	     sprintf(tmpstring,"RENA%d/globhist[%d][%d][%d]",m,m,i,j);
             globhist[m][i][j]=(TH1F *) enefile->Get(tmpstring); 
             if (verbose) {cout << " Entries:: " << globhist[m][i][j]->GetEntries() << endl;}
	   }//j 
	   } //i  
      }  // m


	
      Float_t Emean,peak,Emeans[8*RENACHIPS];
	     /* fitting energy spectra */


	  for (m=0;m<RENACHIPS;m++){
	    if (verbose) cout << " Getting mean energy :: " ;
           for (i=0;i<4;i++){
          for (j=0;j<2;j++){ 
	       Emean = (Float_t )  (*ppVals)(m*8+j*4+i);
	       Emeans[m*8+j*4+i] = (Float_t )  (*ppVals)(m*8+j*4+i);
	       if (verbose) cout  << Emean  << " "; 
	   } // i 
	  } //j
	   if (verbose) cout << endl;
	  } // m
	
 Double_t eres,d_eres;


	if (dorefit){
	

 Int_t hibin=0;
 Int_t max=0;
 Float_t xlow,xhigh;
 // Double_t params[6];

 TPaveText *labeltxt = new TPaveText(.12,.8,.5,.88,"NDC");
 labeltxt->SetFillColor(kWhite);
   TCanvas *c1;
   c1 = (TCanvas*)gROOT->GetListOfCanvases()->FindObject("c1");
  if (!c1) c1 = new TCanvas("c1","c1",10,10,1000,1000);
   c1->SetCanvasSize(700,700);


   if (verbose)   cout << "Fitting global histogram: " <<endl;

   for (m=0;m<RENACHIPS;m++){
    for (Int_t i=0;i<4;i++){
     for (Int_t j=0;j<2;j++){
	    hibin=0;
            max=0;
	    xlow = Emeans[m*8+j*4+i]*0.85;
	    xhigh = Emeans[m*8+j*4+i]*1.15;
            c1->Clear();
            if (globhist[m][i][j]->GetEntries() > MINHISTENTRIES){
	    if (verbose){
	      cout << "Fitting global energy histogram RENA " << m << " UNIT " << i << " MODULE " << j << endl;}
      	       globfits[m][i][j]=fitapdspec(globhist[m][i][j],xlow,xhigh,1,verbose);
	       sprintf(tmpname,"globfits[%d][%d][%d]",m,i,j);
               globfits[m][i][j]->SetName(tmpname);
               globhist[m][i][j]->Draw();
               eres=2.35*globfits[m][i][j]->GetParameter(5)/globfits[m][i][j]->GetParameter(4);
               d_eres=2.35*errorprop_divide(globfits[m][i][j]->GetParameter(5),globfits[m][i][j]->GetParError(5),
                                       globfits[m][i][j]->GetParameter(4),globfits[m][i][j]->GetParError(4));
	    } // if enough entries 
	    else{
	      // in case not enough entries we still want to store the globhist and draw it,
              globhist[m][i][j]->Draw();
	      globfits[m][i][j]= new TF1();
	      sprintf(tmpname,"globfits[%d][%d][%d]",m,i,j);
              globfits[m][i][j]->SetName(tmpname);

	      eres=0.99;d_eres=0.99;
                }
               sprintf(tmpstring,"Eres = %.2f #pm %.2f FWHM",100*eres,100*d_eres);
               labeltxt->Clear();
               labeltxt->AddText(tmpstring);
               sprintf(peaklocationfilename,"%s.RENA%d.unit%d_apd%d_glob",filebase,m,i,j);
               strcat(peaklocationfilename,".png");
               labeltxt->Draw();
	       c1->Print(peaklocationfilename);
	      //            peak=getpeak
              
  		    // FIXME should create a directory on this file

	       globeres[m][i][j]=eres;
 	       globeres_e[m][i][j]=d_eres;

	  }//i
	}//j
   }//m

	} // dorefit

 else {
   if (verbose) { cout << "getting previous fit values :: " << endl;}
   for (m=0;m<RENACHIPS;m++){
    for (Int_t i=0;i<4;i++){
     for (Int_t j=0;j<2;j++){
       if (verbose) { cout << "R" << m << "U" << i << "A" << j << ":: " << globhist[m][i][j]->GetEntries() << " "; }
	  globfits[m][i][j]=(TF1 *)globhist[m][i][j]->GetListOfFunctions()->FindObject("fitfunc");
	  sprintf(tmpname,"globfits[%d][%d][%d]",m,i,j);
          if ( globfits[m][i][j] ) globfits[m][i][j]->SetName(tmpname);
          if (( globfits[m][i][j]) && (globfits[m][i][j]->GetParameter(4))) {
          eres=2.35*globfits[m][i][j]->GetParameter(5)/globfits[m][i][j]->GetParameter(4);
          d_eres=2.35*errorprop_divide(globfits[m][i][j]->GetParameter(5),globfits[m][i][j]->GetParError(5),
				       globfits[m][i][j]->GetParameter(4),globfits[m][i][j]->GetParError(4));
	    }  else {
	      eres = 0.99;d_eres=0.99;}
           globeres[m][i][j]=eres;
          globeres_e[m][i][j]=d_eres;

	  }//j
     if (verbose) cout << endl;
	}//i
   }//m

	} // if not dorefit 


	if (verbose) cout << " Writing parameters to file " << endl;

        // Open Textfile //
        strncpy(filebase,filename,strlen(filename)-12);
        filebase[strlen(filename)-12]='\0';
	//        sprintf(rootfile,"%s",filebase);
	//        strcat(rootfile,".cal.root");
 
         
      
	  ofstream ofile;

       sprintf(peaklocationfilename,"%s.Glob_fit.txt",filebase);
       //       strcat(peaklocationfilename,".txt");
       ofile.open(peaklocationfilename);


        for (m=0;m<RENACHIPS;m++){
       
       for (i=0;i<4;i++){
        for (j=0;j<2;j++){ 
	  ofile << "R" << m << "U" << i << "M" << j <<":: ";
	  
	    for (k=0;k<6;k++){
	    if (k==2) continue; 
	   	  if (globfits[m][i][j]){  ofile << globfits[m][i][j]->GetParameter(k) << " " ; }
         	  else  ofile << "0" << " " ;
		  }
            ofile << globeres[m][i][j] << " " << globeres_e[m][i][j] ;
         ofile << endl;
     
     } // j
     } //i 
          } //m
        ofile.close();
return 0;}


