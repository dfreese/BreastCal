/*

This program fills the energy histograms for every crystal, needed to do energy calibration. If we don't have a valid segmnetation, the crystal id remains -1.

 A new file is created, this is a little redundant. Crystal id's are assigned.

 */


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
#include "/home/miil/root/macros/myrootlib.h"
#include "/home/miil/root/libInit_avdb.h"
#include "./convertconfig.h"

#define FILENAMELENGTH	120
#define MAXFILELENGTH	160
#define E_up 2500
#define E_low -100
#define Ebins 260
#define PEAKS 64
#define EMIN  0 
#define EMAX  2400
#define EBINS 150
//#define CHIPS 2



//void usage(void);

//void usage(void){
// cout << " Parsetomodule -f [filename] [-p [pedfile] -v -o [outputfilename]] -n [nrfiles in loop] -t [threshold]" <<endl;
//  return;}

Int_t getcrystal(Double_t x, Double_t y, Double_t xpos[64], Double_t ypos[64], Int_t verbose );

int main(int argc, Char_t *argv[])
{
 	cout << "Welcome " << endl;

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	Char_t		filename[FILENAMELENGTH] = "";
	Int_t		verbose = 0, threshold=-1000;
	Int_t		ix;
        module UNIT0,UNIT1,UNIT2,UNIT3;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	for(ix = 1; ix < argc; ix++) {

		/*
		 * Verbose '-v'
		 */
		if(strncmp(argv[ix], "-v", 2) == 0) {
			cout << "Verbose Mode " << endl;
			verbose = 1;
		}

		if(strncmp(argv[ix], "-t", 2) == 0) {
                  threshold = atoi( argv[ix+1]);
		  cout << "Threshold =  " << threshold <<endl;
                  ix++;
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



	TFile *rfile = new TFile(filename,"OPEN");
       
        Char_t filebase[FILENAMELENGTH],peaklocationfilename[FILENAMELENGTH],newrootfile[FILENAMELENGTH]; 
        Char_t tmpname[50],tmptitle[50];
        Int_t i,j,k,m,lines;
        Int_t validpeaks[RENACHIPS][4][2];
        Double_t U_x[RENACHIPS][4][2][64];
        Double_t U_y[RENACHIPS][4][2][64];
        Double_t aa, bb;
        ifstream infile;
        TH1F *Ehist[RENACHIPS][4][2][64];
	//        TF1 *Efits[4][2][64];
        TVector* ppVals = (TVector *) rfile->Get("TVectorT<float>");
        TCanvas *c1 =new TCanvas();
        Char_t treename[40];

        strncpy(filebase,filename,strlen(filename)-5);
        filebase[strlen(filename)-5]='\0';
        cout << " filebase = " << filebase << endl;

	for (m=0;m<RENACHIPS;m++){
        for (j=0;j<4;j++){
	  for (i=0;i<2;i++){
	     validpeaks[m][j][i]=0;
	     sprintf(peaklocationfilename,"%s.RENA%d.unit%d_apd%d_peaks",filebase,m,j,i);
             strcat(peaklocationfilename,".txt");
             infile.open(peaklocationfilename);
             lines = 0;
              while (1){
               if (!infile.good()) break;
               infile >> k >>  aa >> bb;
               if (k < 64) { U_y[m][j][i][k]=aa; U_x[m][j][i][k]=bb;}
       //       cout << k << ", " << U_x[i][j][k]<< ", " << U_y[i][j][k] << endl;
               lines++;      
                }
       cout << "Found " << lines-1 << " peaks in  file " << peaklocationfilename << endl;
       infile.close();
       if (lines==65){ cout << "Setting Validpeaks " << endl; validpeaks[m][j][i]=1; }
	}
	}
	}


	if (verbose){
	  for (m=0;m<RENACHIPS;m++){
        for (j=0;j<4;j++){
	  for (i=0;i<2;i++){
            cout << " m = " << m << " i = " << i << " j = " << j << " : " <<validpeaks[m][j][i] <<endl;
	  }}
	}
	}




	//	cout << " U_x[0][0][21] ="<< U_x[0][0][21] << " U_y[0][0][21] = " << U_y[0][0][21]<<endl;

	

       TFile *f;
       sprintf(newrootfile,"%s.enecal",filebase);
       strcat(newrootfile,".root");
       if (verbose) {
	 cout << "Creating New Root file : " << newrootfile << endl;}
       f = new TFile(newrootfile,"RECREATE");

	TTree *block;
	/* loop over the chips !! */

	for (m=0;m<RENACHIPS;m++){

       /* Creating histograms */
	  for (j=0;j<4;j++){
            for (i=0;i<2;i++){
	      for (k=0;k<64;k++){
                sprintf(tmpname,"Ehist[%d][%d][%d][%d]",m,j,i,k);
                sprintf(tmptitle,"RENA %d Unit %d Module %d Pixel %d",m,j,i,k);
                Ehist[m][j][i][k] = new TH1F(tmpname,tmptitle,EBINS,EMIN,EMAX);
		//                sprintf(tmpname,"Efits[%d][%d][%d]",i,j,k);
		//                sprintf(tmptitle,"Unit %d Module %d Pixel %d",i,j,k);
		//                Efits[i][j][k] = new TF1(tmpname,"gaus",EBINS,EMIN,EMAX);
	      }
            }
          }


	  /*
	  if (m==0)  block = (TTree *) rfile->Get("block1");
          else  block = (TTree *) rfile->Get("block2");
	  */
	  sprintf(treename,"block[%d]",m);
	  block = (TTree *) rfile->Get(treename);
  
        cout << " Entries : " << block->GetEntries() << endl;
	

        block->SetBranchAddress("UNIT0",&UNIT0);
        block->SetBranchAddress("UNIT1",&UNIT1);
        block->SetBranchAddress("UNIT2",&UNIT2);
        block->SetBranchAddress("UNIT3",&UNIT3);


         strncpy(filebase,filename,strlen(filename)-5);
         filebase[strlen(filename)-5]='\0';
  
	 if (verbose) cout << " Cloning Tree " << endl;
          TTree *calblock = block->CloneTree(0);
	  sprintf(treename,"calblock[%d]",m);
	  /*
         if (m==0) calblock->SetName("calblock1");
         else calblock->SetName("calblock2");
	  */
          calblock->SetName(treename);

	  for (i=0;i<block->GetEntries();i++){
	    if ((i%100000)==0) fprintf(stdout,"%d Events Processed\r",i);
	   //      	  for (i=0;i<1e5;i++){
	    block->GetEntry(i);
             if (validpeaks[m][0][0]&&UNIT0.com1h<threshold)
	       {UNIT0.id=getcrystal(UNIT0.x,UNIT0.y,U_x[m][0][0],U_y[m][0][0],verbose);
		if ((UNIT0.id)>=0) Ehist[m][0][0][UNIT0.id]->Fill(UNIT0.E); }
	     else { 
             if (validpeaks[m][0][1]&&UNIT0.com2h<threshold) 
	       {UNIT0.id=getcrystal(UNIT0.x,UNIT0.y,U_x[m][0][1],U_y[m][0][1],verbose);
                if ((UNIT0.id)>=0) Ehist[m][0][1][UNIT0.id]->Fill(UNIT0.E);}
	     }
             if (validpeaks[m][1][0]&&UNIT1.com1h<threshold)
	       {UNIT1.id=getcrystal(UNIT1.x,UNIT1.y,U_x[m][1][0],U_y[m][1][0],verbose);
                if ((UNIT1.id)>=0)
       		 Ehist[m][1][0][UNIT1.id]->Fill(UNIT1.E);}
	     else { 
             if (validpeaks[m][1][1]&&UNIT1.com2h<threshold) 
	       {UNIT1.id=getcrystal(UNIT1.x,UNIT1.y,U_x[m][1][1],U_y[m][1][1],verbose);
		if ((UNIT1.id)>=0) Ehist[m][1][1][UNIT1.id]->Fill(UNIT1.E);}
	     }
             if (validpeaks[m][2][0]&&UNIT2.com1h<threshold)
	       {UNIT2.id=getcrystal(UNIT2.x,UNIT2.y,U_x[m][2][0],U_y[m][2][0],verbose);
		 if ((UNIT2.id)>=0) Ehist[m][2][0][UNIT2.id]->Fill(UNIT2.E);}
	     else { 
             if (validpeaks[m][2][1]&&UNIT2.com2h<threshold) 
	       {UNIT2.id=getcrystal(UNIT2.x,UNIT2.y,U_x[m][2][1],U_y[m][2][1],verbose);
		 if ((UNIT2.id)>=0) Ehist[m][2][1][UNIT2.id]->Fill(UNIT2.E);}
	     }
             if (validpeaks[m][3][0]&&UNIT3.com1h<threshold)
	       {UNIT3.id=getcrystal(UNIT3.x,UNIT3.y,U_x[m][3][0],U_y[m][3][0],verbose);
		 if ((UNIT3.id)>=0)  Ehist[m][3][0][UNIT3.id]->Fill(UNIT3.E);}
	     else { 
             if (validpeaks[m][3][1]&&UNIT3.com2h<threshold) 
	       {UNIT3.id=getcrystal(UNIT3.x,UNIT3.y,U_x[m][3][1],U_y[m][3][1],verbose);
		 if ((UNIT3.id)>=0) Ehist[m][3][1][UNIT3.id]->Fill(UNIT3.E);}
	     }


             calblock->Fill();
             
	     //	     cout << validpeaks[0][0] << " "<<UNIT0.E << " " << UNIT0.id<< endl;

	  }


	  cout << " Done looping over the events " <<endl;



		 ppVals->Write();

          calblock->AutoSave();
	  //          f->Write();     
	  for (j=0;j<2;j++){
            for (i=0;i<4;i++){
	      for (k=0;k<64;k++){
                Ehist[m][i][j][k]->Write();
	      }
            }
          }
	} // loop over m

	//	Ehist[1][0][0][34]->Draw(); c1->Print("test.png");
 
          f->Close();
         
	rfile->Close();


	return 0;}


Int_t getcrystal(Double_t x, Double_t y, Double_t xpos[64], Double_t ypos[64], Int_t verbose){
  //  cout << " Getcrystal :: " << " xpos[21] = " << xpos[21] << " ypos[21] = " << ypos[21] << endl;
  //  cout << " x = " << x << " y = " << y << endl;
  
  Double_t dist,min;
  Int_t histnr;

  histnr=9999;
  min=100000;
  if ((TMath::Abs(x)>1)||(TMath::Abs(y)>1)) return -3;
   for (Int_t k=0;k<PEAKS;k++){
      //      dist=(*(*xpeaks+k)-xdata)*(*(*xpeaks+k)-xdata)+(*(*ypeaks+k)-ydata)*(*(*ypeaks+k)-ydata);

      dist=TMath::Power((Float_t) ypos[k]-y,2)+TMath::Power((Float_t) xpos[k]-x,2);

     //if (debvar) cout << "k = " << k << "dist = " << dist << endl;

     if (dist<min) {histnr=k;min=dist;

       //   if (debvar) cout << "--------> k = " << k <<" min = " << min <<endl;
       }
     } //for loop 
   //      if (verbose) cout << "FINAL :: histnr = " << histnr <<" min = " << min << endl;
     if (histnr!=9999) { 
       return histnr;
	}
     
  
     else { cout << "No associated histogram found !" << endl;
      cout << " Entry :  x = " << x << " y = " << y <<endl;
     }
    min=10000;
    histnr=9999;
    
  return -2;}
