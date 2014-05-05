#define PixelCal_cxx

#include "PixelCal.h"
#include <sys/stat.h>

ClassImp(PixelCal)

/*
PPeaks::PPeaks() {
  PPeaks("myppeaks");
}
*/

PixelCal::~PixelCal() {
  // destructor
}

Int_t PixelCal::ReadCal(const char *filebase){
      TFile *lutfile = 0;
      Char_t peaklocationfilename[FILENAMELENGTH];
      Char_t filename[FILENAMELENGTH];
      ifstream infile;
      Int_t lines;
      Char_t tmpstring[FILENAMELENGTH] = "";
      Float_t aa,bb;
      Int_t k;

      sprintf(filename,"%s.floodmap.root",filebase);

      if (floodlut){ lutfile =  new TFile(filename,"OPEN");	}

      for (Int_t c=0;c<CARTRIDGES_PER_PANEL;c++){
	for (Int_t f=0;f<FINS_PER_CARTRIDGE;f++){
	  for (Int_t i=0;i<MODULES_PER_FIN;i++){
	    for (Int_t j=0;j<APDS_PER_MODULE;j++){
	     validpeaks[c][f][i][j]=0;
             if (floodlut) {
	       sprintf(tmpstring,"floodmap[%d][%d][%d][%d]",c,f,i,j);
	       floodmap[c][f][i][j] = (TH2S *)    lutfile->Get(tmpstring);
	       if ( floodmap[c][f][i][j]->GetEntries() ) validpeaks[c][f][i][j]=1;
             }//floodmap
             else {
	       sprintf(peaklocationfilename,"./CHIPDATA/%s.C%dF%d.module%d_apd%d_peaks",filebase,c,f,i,j);
             strcat(peaklocationfilename,".txt");
             infile.open(peaklocationfilename);
             lines = 0;
              while (1){ 
               if (!infile.good()) break;
               infile >> k >>  aa >> bb;
               if (k < 64) { X[c][f][i][j][k]=aa; Y[c][f][i][j][k]=bb;}
       //       cout << k << ", " << U_x[i][j][k]<< ", " << U_y[i][j][k] << endl;
               lines++;      
                }
	      if (verbose) cout << "Found " << lines-1 << " peaks in  file " << peaklocationfilename << endl;
	      infile.close();
	      if (lines==65){ if (verbose) cout << "Setting Validpeaks " << endl; validpeaks[c][f][i][j]=1; }
	     } // else floodmap
	    }//j
	  }//i
	}//f
      }//c

      return 0;}  //ReadCal


Int_t PixelCal::GetCrystalId(Float_t x, Float_t y, Int_t c, Int_t f, Int_t i, Int_t j){
  //  cout << " Getcrystal :: " << " xpos[21] = " << xpos[21] << " ypos[21] = " << ypos[21] << endl;
  //  cout << " x = " << x << " y = " << y << endl;
  
  Double_t dist,min;
  Int_t histnr;

  histnr=9999;
  min=100000;
  if ((TMath::Abs(x)>1)||(TMath::Abs(y)>1)) return -3;
   for (Int_t k=0;k<PEAKS;k++){
      //      dist=(*(*xpeaks+k)-xdata)*(*(*xpeaks+k)-xdata)+(*(*ypeaks+k)-ydata)*(*(*ypeaks+k)-ydata);

      dist=TMath::Power((Float_t) Y[c][f][i][j][k]-y,2)+TMath::Power((Float_t) X[c][f][i][j][k]-x,2);

     //if (debvar) cout << "k = " << k << "dist = " << dist << endl;

     if (dist<min) {histnr=k;min=dist;

       //   if (debvar) cout << "--------> k = " << k <<" min = " << min <<endl;
       }
     } //for loop 
   //      if (verbose) cout << "FINAL :: histnr = " << histnr <<" min = " << min << endl;
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



Int_t PixelCal::WriteCalTxt(const char *filebase){
  

    TString fDIR="CALPAR";
    
   if ( access( fDIR.Data(), 0 ) != 0 ) {
	cout << "creating dir " << fDIR << endl; 
        if ( mkdir(fDIR, 0777) != 0 ) { 
	    cout << " Error making directory " << fDIR << endl;
	    return -2;
	}
    }

  ofstream ofile;
  Char_t calfilename[120];

      for (Int_t c=0;c<CARTRIDGES_PER_PANEL;c++){
	for (Int_t f=0;f<FINS_PER_CARTRIDGE;f++){
	  for (Int_t i=0;i<MODULES_PER_FIN;i++){
	    for (Int_t j=0;j<APDS_PER_MODULE;j++){
		sprintf(calfilename,"%s/%s.C%dF%dM%dA%d_cal",fDIR.Data(),filebase,c,f,i,j);
	      strcat(calfilename,".txt");
	      if (validpeaks[c][f][i][j]){
		ofile.open(calfilename);
		for (Int_t k=0;k<64;k++){
		  ofile << X[c][f][i][j][k] << " " << Y[c][f][i][j][k] ;
                  ofile <<  " " << GainSpat[c][f][i][j][k] << " " << EresSpat[c][f][i][j][k];
		  ofile << " " << GainCom[c][f][i][j][k]   << " " << EresCom[c][f][i][j][k];
		  ofile << endl;}
		ofile.close();
	      }
	    } // j
	  } //i 
	} //f
      }//c

      return 0;}


