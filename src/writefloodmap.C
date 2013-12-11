
#include "TROOT.h"
#include "TStyle.h" 
#include "Riostream.h"
#include "TTree.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TH2S.h"
#include "TVector.h"
#include "THistPainter.h"
#include "TPaletteAxis.h"
#include "TMath.h"


#include "decoder.h"
#include "writefloodmap.h"

Int_t getcrystal(Double_t x, Double_t y, Double_t xpos[64], Double_t ypos[64], Int_t verbose);

Int_t main(Int_t argc, Char_t** argv){

 cout << " Welcome to writecal. Program generates flood LUTs.  " ; 

 Int_t ix,m,i,j; 
 Char_t          filename[FILENAMELENGTH] = "";
 Char_t          filebase[FILENAMELENGTH] ="";
 Char_t          peaklocationfilename[FILENAMELENGTH] ="";
 Char_t          tmpstring[120],titlestring[120];
 Int_t           verbose = 0;
 ifstream        infile;

   for(ix = 1; ix < argc; ix++) {
           if(strncmp(argv[ix], "-v", 2) == 0) {
              cout << "Verbose Mode " << endl;
              verbose = 1;
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
                 ix++;}
   } // ix loop

        cout << " Inputfile : " << filename << "." << endl; 
        if (!verbose) {
        gErrorIgnoreLevel = kBreak;
                }
        strncpy(filebase,filename,strlen(filename)-5);
        filebase[strlen(filename)-5]='\0';
        if (verbose) cout << " filebase = " << filebase << endl;
      
 
      

        rootlogon(verbose);



        sprintf(tmpstring,"%s.floodmap.root",filebase);
        TFile *rfile = new TFile(tmpstring,"RECREATE");
         if (!rfile || rfile->IsZombie()) {
            cout << "problems opening file " << tmpstring << ". Exiting." << endl;
            return -11;}


				      Double_t XX[64],YY[64];
				      Int_t k=0,lines=0;
				      Int_t crystal;
				      Int_t ii,jj;
				      Double_t xx,yy;

				      Double_t levels[64];
				      for (i=0;i<64;i++) {levels[i] = i ; }


 

         for (m=0;m<RENACHIPS;m++){ 
  	 for (i=0;i<MODULES;i++) {
 	  for (j=0;j<APDS_PER_MODULE;j++){
	       //         validpeaks[m][j][i]=0;
	       sprintf(tmpstring,"floodmap[%d][%d][%d]",m,i,j);
	       sprintf(titlestring,"FLOOD LUT RENA %d UNIT %d APD %d",m,i,j);
	       if (verbose) { cout << titlestring << endl;} 
         floodmap[m][i][j] = new TH2S(tmpstring,titlestring,FLOODBINS,FLOODMIN,FLOODMAX,FLOODBINS,FLOODMIN,FLOODMAX);
         sprintf(peaklocationfilename,"./CHIPDATA/%s.RENA%d.unit%d_apd%d_peaks",filebase,m,i,j);
         strcat(peaklocationfilename,".txt");
         infile.open(peaklocationfilename);
	 k=0;lines=0;
	 while (  infile >> k ) { 
  	   infile >>  XX[k] >> YY[k] ;
               lines++;
	       //              cout << k << " " << XX[k] << " " << YY[k] << endl;      
                }
              if (verbose) cout << "Found " << lines << " peaks in  file " << peaklocationfilename << endl;
       infile.close();
       if (lines==64){ if (verbose) cout << "Setting Validpeaks " << endl; 
	 //validpeaks[m][j][i]=1;  
  
       	 for (ii=0;ii<floodmap[m][i][j]->GetNbinsX();ii++) { 
	   xx = floodmap[m][i][j]->GetXaxis()->GetBinCenter(ii); 
	   for (jj=0;jj<floodmap[m][i][j]->GetNbinsY();jj++){  
	     yy= floodmap[m][i][j]->GetYaxis()->GetBinCenter(jj); 
	     crystal = getcrystal(xx,yy,XX,YY,verbose );
	     floodmap[m][i][j]->SetBinContent(ii,jj,crystal);
	   } //loop jj 
  	 } //loop ii

	 floodmap[m][i][j]->SetContour((sizeof(levels)/sizeof(Double_t)), levels); 
	 floodmap[m][i][j]->SetMinimum(-0.001);
 
       } // if 64 crystals read
       	     
              floodmap[m][i][j]->Write();
	 	     } // loop j (APDs)
       	   } // loop i  (MODULES) 
       } // loop m ( RENA )


	 rfile->Close();


  return 0;
}


Int_t getcrystal(Double_t x, Double_t y, Double_t xpos[PEAKS], Double_t ypos[PEAKS], Int_t verbose){
  
  Double_t dist,min;
  Int_t histnr;

  histnr=9999;
  min=100000;
  if ((TMath::Abs(x)>1)||(TMath::Abs(y)>1)) return -3;
   for (Int_t k=0;k<PEAKS;k++){
      dist=TMath::Power((Float_t) ypos[k]-y,2)+TMath::Power((Float_t) xpos[k]-x,2);
      if (dist<min) {histnr=k;min=dist;
       }
     } //for loop 

    if (histnr!=9999) { 
       return histnr;
        }
     
     else { if (verbose) 
         {cout << "No associated histogram found !" << endl;
           cout << " Entry :  x = " << x << " y = " << y <<endl;}
     }
    min=10000;
    histnr=9999;
    
  return -2;}
