#include "TROOT.h"
#include "TStyle.h"
#include "Riostream.h"
#include "TTree.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TSpectrum.h"
#include "TSpectrum2.h"
#include "TH2F.h"
#include "TVector.h"
#include "TMath.h"
#include "TSpectrum.h"
#include "TF1.h"
#include "decoder.h"
#include "CoincEvent.h"
#include "string.h"


//#define DEBUG2
#define UNITS 16

#define MINMODENTRIES 1000

Int_t drawmod_crys2(TH1F *hi[MODULES_PER_FIN][APDS_PER_MODULE][64], TCanvas *ccc, Char_t filename[MAXFILELENGTH]);
Int_t writ(TH1D *hi[PEAKS], TCanvas *ccc, Char_t filename[MAXFILELENGTH]);
Int_t writ2d(TH2F *hi[PEAKS], TCanvas *ccc, Char_t filename[MAXFILELENGTH]);
TH2F *get2dcrystal(Float_t vals[64], Char_t title[40]) ;
Float_t getmax (TSpectrum *, Int_t);
Float_t cryscalfunc(TH1F *hist, Bool_t gausfit);

int main(int argc, Char_t *argv[])
{ 
  Int_t MOD1=6;
  Int_t MOD2=1;
  Int_t APD1=0;
  Int_t APD2=0;
  Bool_t usegausfit=0;
  Int_t coarsetime=1; 
  Int_t crystalmean=0;
  Char_t histtitle[40];

 Int_t DTF_low, DTF_hi, FINELIMIT,DTFLIMIT;

 	cout << "Welcome " << endl;

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	Char_t		filenamel[FILENAMELENGTH] = "";
	Int_t		verbose = 0;
	Int_t		ix;
	//module UNIT0,UNIT1,UNIT2,UNIT3;
        CoincEvent           *evt =  new CoincEvent();
        CoincEvent           *calevt = new CoincEvent();

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/



	for(ix = 1; ix < argc; ix++) {

		/*
		 * Verbose '-v'
		 */
		if(strncmp(argv[ix], "-v", 2) == 0) {
			cout << "Verbose Mode " << endl;
			verbose = 1;
		}



		if(strncmp(argv[ix], "-apd1", 5) == 0) {
                  APD1 = atoi( argv[ix+1]);
		  cout << "APD1 =  " << APD1 <<endl;
                  ix++;
		}

		if(strncmp(argv[ix], "-apd2", 5) == 0) {
                  APD2 = atoi( argv[ix+1]);
		  cout << "APD2 =  " << APD2 <<endl;
                  ix++;
		}

		if(strncmp(argv[ix], "-mod1", 5) == 0) {
                  MOD1 = atoi( argv[ix+1]);
		  cout << "MOD1 =  " << MOD1 <<endl;
                  ix++;
		}

		if(strncmp(argv[ix], "-mod2", 5) == 0) {
                  MOD2 = atoi( argv[ix+1]);
		  cout << "MOD2 =  " << MOD2 <<endl;
                  ix++;
		}


		if(strncmp(argv[ix], "-cm", 3) == 0) {
                  crystalmean=1;
		  cout << "Crystal calibration "  <<endl;
		}
 
		if(strncmp(argv[ix], "-gf", 3) == 0) {
                  usegausfit=1;
		  cout << " Using Gauss Fit "  <<endl;
		}
		
		if(strncmp(argv[ix], "-ft", 3) == 0) {
                  coarsetime=0;
		  cout << " Fine time interval "  <<endl;
		}

		


		/* filename '-f' */
		if(strncmp(argv[ix], "-f", 3) == 0) {
			if(strlen(argv[ix + 1]) < FILENAMELENGTH) {
				sprintf(filenamel, "%s", argv[ix + 1]);
			}
			else {
				cout << "Filename " << argv[ix + 1] << " too long !" << endl;
				cout << "Exiting.." << endl;
				return -99;
			}
		}


	}

        rootlogon(verbose);
      gStyle->SetOptStat(kTRUE); 
  //	TStyle::SetOptStat();

   TCanvas *c1;
   c1 = (TCanvas*)gROOT->GetListOfCanvases()->FindObject("c1");
  if (!c1) c1 = new TCanvas("c1","c1",10,10,1000,1000);
   c1->SetCanvasSize(700,700);
       
        Char_t filebase[FILENAMELENGTH],rootfile[FILENAMELENGTH]; 
        Int_t i;
        ifstream infile;
 
        cout << " Opening file " << filenamel << endl;
        TFile *rtfile = new TFile(filenamel,"OPEN");
        TTree *mm  = (TTree *) rtfile->Get("merged");
        mm->SetBranchAddress("Event",&evt);


	//#define UNITS 2

        TH1F *crystaloffset[2][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE][64];
        TF1 *fit_crystaloffset[2][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE][64];

	//        coarsetime=0;

	Int_t tt,  aa,ii,jj,kk;

	if (coarsetime ) {  DTF_low = -100; DTF_hi = 100; FINELIMIT=100; DTFLIMIT=50; cout << " Using Coarse limits: " << FINELIMIT << endl;}
        else { DTF_low = -50; DTF_hi = 50; FINELIMIT=50; DTFLIMIT=5;}


   
	for (ii=0;ii<2;ii++) {
	  for (jj=0;jj<FINS_PER_CARTRIDGE;jj++) {
 	  for (kk=0;kk<MODULES_PER_FIN;kk++) {
          for (aa=0;aa<APDS_PER_MODULE;aa++) {
            for (tt=0;tt<64;tt++){
              sprintf(histtitle,"crystaloffset[%d][%d][%d][%d][%d]",ii,jj,kk,aa,tt);
              crystaloffset[ii][jj][kk][aa][tt]= new TH1F(histtitle,histtitle,50,DTF_low,DTF_hi);
	    }
	  }
	}
	  }
	}

       Long64_t entries = mm->GetEntries();
       cout << " Total  entries: " << entries << endl; 

       
       cout << " Filling crystal spectra on the left. " << endl;

       Long64_t checkevts=0;


        strncpy(filebase,filenamel,strlen(filenamel)-5);
        filebase[strlen(filenamel)-5]='\0';
        sprintf(rootfile,"%s",filebase);

        cout << " ROOTFILE = " << rootfile << endl;

 
       for (i=0;i<entries; i++) {
	 mm->GetEntry(i);
         if (evt->fin1>FINS_PER_CARTRIDGE) continue;
	 if ((evt->crystal1<65)&&((evt->apd1==APD1)||(evt->apd1==1))&&(evt->m1<MODULES_PER_FIN)) {
	     if ((evt->E1>400)&&(evt->E1<600)) {
               if (TMath::Abs(evt->dtc ) < 6 ) {
	       if (TMath::Abs(evt->dtf ) < FINELIMIT ) {
                 checkevts++;
		 //	    		 crystime[0][evt->m1][evt->apd1][evt->crystal1]->Fill(evt->dtf);
		 crystaloffset[0][evt->fin1][evt->m1][evt->apd1][evt->crystal1]->Fill(evt->dtf);
               }
	     }
	   }
	 }
       } // loop over entries
	  
       cout << " Done looping over entries " << endl;
       cout << " I made " << checkevts << " calls to Fill() " << endl;         

       Float_t mean_crystaloffset[2][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE][64]={{{{{0}}}}};


       ii=0;

       for (jj=0;jj<FINS_PER_CARTRIDGE;jj++) {
	 for (kk=0;kk<MODULES_PER_FIN;kk++) { 
          for (aa=0;aa<APDS_PER_MODULE;aa++) {
            for (tt=0;tt<64;tt++) {
           mean_crystaloffset[ii][jj][kk][aa][tt]=cryscalfunc(crystaloffset[ii][jj][kk][aa][tt], usegausfit) ;
            }}}}

	  Char_t psfile[MAXFILELENGTH];
	  sprintf(psfile,"%s_fin1.ps",rootfile);

       drawmod_crys2(crystaloffset[0][1],c1,psfile);
  
       for (jj=0;jj<FINS_PER_CARTRIDGE;jj++){
       for (kk=0;kk<MODULES_PER_FIN;kk++){
        for (aa=0;aa<APDS_PER_MODULE;aa++) {
          for (tt=0;tt<64;tt++) {
	       cout << mean_crystaloffset[ii][jj][kk][aa][tt] << " ";}
	  cout << endl;}
       }
       }
           cout << endl;


       cout << " Filling crystal spectra on the right. " << endl;
	checkevts=0;


        for (i=0;i<entries; i++) {
	 mm->GetEntry(i);
         if (evt->fin1>FINS_PER_CARTRIDGE) continue;
         if (evt->fin2>FINS_PER_CARTRIDGE) continue;
	 if ((evt->crystal1<65)&&((evt->apd1==APD1)||(evt->apd1==1))&&(evt->m1<MODULES_PER_FIN)) {
	 if ((evt->crystal2<65)&&((evt->apd2==APD1)||(evt->apd2==1))&&(evt->m2<MODULES_PER_FIN)) {
           if  ((evt->E2>400)&&(evt->E2<600)) {
           if  ((evt->E1>400)&&(evt->E1<600)) {
               if (TMath::Abs(evt->dtc ) < 6 ) {
	       if (TMath::Abs(evt->dtf ) < FINELIMIT ) {
                 checkevts++;
		 //	    		 crystime[0][evt->m1][evt->apd1][evt->crystal1]->Fill(evt->dtf);
		 crystaloffset[1][evt->fin2][evt->m2][evt->apd2][evt->crystal2]->Fill(evt->dtf- mean_crystaloffset[0][evt->fin1][evt->m1][evt->apd1][evt->crystal1]);
               }
	     }
	   }
	 }
	 }
	 }
       } // loop over entries

       cout << " Done looping over entries " << endl;
       cout << " I made " << checkevts << " calls to Fill() " << endl;         


       ii=1;

       for (jj=0;jj<FINS_PER_CARTRIDGE;jj++) {
	 for (kk=0;kk<MODULES_PER_FIN;kk++) { 
          for (aa=0;aa<APDS_PER_MODULE;aa++) {
            for (tt=0;tt<64;tt++) {
           mean_crystaloffset[ii][jj][kk][aa][tt]=cryscalfunc(crystaloffset[ii][jj][kk][aa][tt], usegausfit) ;
            }}}}



	  sprintf(psfile,"%s_fin2.ps",rootfile);


       drawmod_crys2(crystaloffset[0][1],c1,psfile);
  
       for (jj=0;jj<FINS_PER_CARTRIDGE;jj++){
       for (kk=0;kk<MODULES_PER_FIN;kk++){
        for (aa=0;aa<APDS_PER_MODULE;aa++) {
          for (tt=0;tt<64;tt++) {
	       cout << mean_crystaloffset[ii][jj][kk][aa][tt] << " ";}
	  cout << endl;}
       }
       }
           cout << endl;

    TH1F *tres = new TH1F("tres","Time Resolution After Time walk correction",100,-25,25);

        strcat(rootfile,".crystaloffcal.root");

       
      cout << " Opening file " << rootfile << " for writing " << endl;
      TFile *calfile = new TFile(rootfile,"RECREATE");
      TTree *merged = new  TTree("merged","Merged and Calibrated LYSO-PSAPD data ");
      merged->Branch("Event",&calevt);


      cout << "filling new Tree :: " << endl;

        for (i=0;i<entries; i++) {
	 mm->GetEntry(i);
         if (evt->fin1>FINS_PER_CARTRIDGE) continue;
         if (evt->fin2>FINS_PER_CARTRIDGE) continue;
	 if ((evt->crystal1<65)&&((evt->apd1==APD1)||(evt->apd1==1))&&(evt->m1<MODULES_PER_FIN)) {
	 if ((evt->crystal2<65)&&((evt->apd2==APD1)||(evt->apd2==1))&&(evt->m2<MODULES_PER_FIN)) {
	   calevt=evt;
           calevt->dtf-= mean_crystaloffset[0][evt->fin1][evt->m1][evt->apd1][evt->crystal1] ;
           calevt->dtf-= mean_crystaloffset[1][evt->fin2][evt->m2][evt->apd2][evt->crystal2] ; 
           if (evt->E1>400&&evt->E1<600&&evt->E2>400&&evt->E2<600) {
	     tres->Fill(calevt->dtf);}
	 }
	 }
         merged->Fill();

	}
    

      calfile->Close();


      tres->Fit("gaus","","",-10,10);


      c1->Clear();
      tres->Draw();
      sprintf(psfile,"%s.tres.crysoffset.ps",rootfile);

      c1->Print(psfile);


	     // crystime[0][MOD1][APD1][0]->Draw(); c1->Print("testje2.ps");
	     // cout << crystime[0][MOD1][APD1][0]->GetEntries() << endl;
 // TFile *ff = new TFile("test.root");
 // crystime[0][0][0][0]->Write();
 // ff->Close();


      return 0;}



Float_t getmax(TSpectrum *s,Int_t npeaks){
  Int_t maxpeakheight=-1000000;
  Float_t maxpos = 0;
  if (npeaks>1 ) {
    for (Int_t i=0;i<npeaks;i++) { 
      cout << " Peak " << i << " :  " << *(s->GetPositionX()+i) << " " << *(s->GetPositionY()+i) << endl;
    }
  }

  for (Int_t i=0;i<npeaks;i++) {
    if ( (*(s->GetPositionY()+i)) > maxpeakheight ){
      maxpos= *(s->GetPositionX()+i);
      maxpeakheight = *(s->GetPositionY()+i);
    }
  }
  return maxpos;}
  


Int_t drawmod_crys2(TH1F *hi[MODULES_PER_FIN][APDS_PER_MODULE][64], TCanvas *ccc, Char_t filename[MAXFILELENGTH])
{
        /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  Int_t   i,k,kk,j;
        Char_t  filenameo[MAXFILELENGTH+1], filenamec[MAXFILELENGTH+1];
        /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

         // cout << "filename = " << filename << endl;

        strcpy(filenameo, filename);
        strcpy(filenamec, filename);
        strcat(filenameo, "(");
        strcat(filenamec, ")");

        /*
          cout << "in : " << filenameo<<endl;
          * TCanvas *ccc = new TCanvas("ccc","Energy Spectra",10,10,1000,900);
         */
        ccc->Clear();
        ccc->Divide(4, 4);

	for (kk =0;kk<MODULES_PER_FIN ;kk++){
        for(k = 0; k < APDS_PER_MODULE ; k++) {
	  for(i = 0 ;i < 4; i++ ) {
	    for (j=0;j<16;j++){
	      ccc->cd(j+1);
             hi[kk][k][i*16+j]->Draw("E");
	  }
	    if((kk == 0)&&(k==0)&&(i==0)) {
              ccc->Print(filenameo);
                        }
           else {
	     if((kk == (MODULES_PER_FIN-1))&&(k==1)&&(i==3)) {
              ccc->Print(filenamec);
             }
             else {
              ccc->Print(filename);
                  }
          }
           ccc->Clear();
           ccc->Divide(4, 4);
	  }
                        /* } */
                }
        }

        return 0;
}

Int_t writ(TH1D *hi[PEAKS], TCanvas *ccc, Char_t filename[MAXFILELENGTH])
{
        /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
        Int_t   k;
        Char_t  filenameo[MAXFILELENGTH+1], filenamec[MAXFILELENGTH+1];
        /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	cout << " Welcome to TH1D writ " << endl;
         // cout << "filename = " << filename << endl;

        strcpy(filenameo, filename);
        strcpy(filenamec, filename);
        strcat(filenameo, "(");
        strcat(filenamec, ")");

        /*
          cout << "in : " << filenameo<<endl;
          * TCanvas *ccc = new TCanvas("ccc","Energy Spectra",10,10,1000,900);
         */
        ccc->Clear();
        ccc->Divide(2, 4);

        for(k = 1; k < PEAKS + 1; k++) {
                if(k % 8)
                        ccc->cd(k % 8);
                else
                        ccc->cd(8);

                
		//      cout << k << " " << k%8 <<endl;
                 
                hi[k - 1]->Draw("E");

                /*
                 * cout << ff[k-1]->GetName() <<endl;
                 */
		//                if(drawfunc) ff[k - 1]->Draw("same");
                if(!(k % 8)) {

                        /*
                         * cout << k << endl;
                         */
                        if(k == 8) {
                                ccc->Print(filenameo);
                        }
                        else {
                                if(k == PEAKS)
                                        ccc->Print(filenamec);
                                else {
                                        ccc->Print(filename);
                                }
                        }

                        /*
                         * if (k!=PEAKS){ ;
                         * cout << "Clearing and Dividing " <<endl;
      */
                        ccc->Clear();
                        ccc->Divide(2, 4);

                        /* } */
                }
        }

        return 0;
}

Int_t writ2d(TH2F *hi[PEAKS], TCanvas *ccc, Char_t filename[MAXFILELENGTH])
{
        /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
        Int_t   k;
        Char_t  filenameo[MAXFILELENGTH+1], filenamec[MAXFILELENGTH+1];
        /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

         // cout << "filename = " << filename << endl;

        strcpy(filenameo, filename);
        strcpy(filenamec, filename);
        strcat(filenameo, "(");
        strcat(filenamec, ")");

        /*
          cout << "in : " << filenameo<<endl;
          * TCanvas *ccc = new TCanvas("ccc","Energy Spectra",10,10,1000,900);
         */
        ccc->Clear();
        ccc->Divide(2, 4);

        for(k = 1; k < PEAKS + 1; k++) {
                if(k % 8)
                        ccc->cd(k % 8);
                else
                        ccc->cd(8);

                /*
                 * cout << k << " " << k%8 <<endl;
                 */
                hi[k - 1]->Draw("colz");

                /*
                 * cout << ff[k-1]->GetName() <<endl;
                 */
		//                if(drawfunc) ff[k - 1]->Draw("same");
                if(!(k % 8)) {

                        /*
                         * cout << k << endl;
                         */
                        if(k == 8) {
                                ccc->Print(filenameo);
                        }
                        else {
                                if(k == PEAKS)
                                        ccc->Print(filenamec);
                                else {
                                        ccc->Print(filename);
                                }
                        }

                        /*
                         * if (k!=PEAKS){ ;
                         * cout << "Clearing and Dividing " <<endl;
      */
                        ccc->Clear();
                        ccc->Divide(2, 4);

                        /* } */
                }
        }

        return 0;
}


TH2F *get2dcrystal(Float_t vals[64], Char_t title[40]="area") {
  TH2F *thispar = new TH2F("thispar",title,8,0,8,8,0,8);
  Int_t i,j;
  for (i=0;i<8;i++){
     for (j=0;j<8;j++){
       thispar->SetBinContent(i+1,j+1,vals[i*8+j]);  }
  }
  

  return thispar;}


Float_t cryscalfunc(TH1F *hist,   Bool_t gausfit){
   TF1 *fitfun;
   Int_t npeaks;
   TSpectrum *s = new TSpectrum();

  if (gausfit) {
     if ( hist->GetEntries() > 70 ) {     
       fitfun=fitgaus(hist,1) ;
       return fitfun->GetParameter(1);}
     }
   else { 
     if ( hist->GetEntries() > 50 ) {
       return hist->GetMean();
     } 
     else return 0;} 
     // else  // not enough entries to fit 
     //  return 0;
    // gausfit 
   //else {    // just peak search 
 
      npeaks = s->Search(hist,3,"",0.2);
       if (npeaks){  return getmax(s,npeaks); }
       else return 0;
   
   

}


	      //              sprintf(tmpstring,"fit_crystaloffset[%d][%d][%d]",ii,aa,tt);
	      //	      fit_crystaloffset[ii][aa][tt] = new TF1(tmpstring,"gaus+pol0",DTF_low,-DTF_hi);
	      //                fit_crystaloffset[ii][aa][tt]->SetParameter(0,200);
	      // fit_crystaloffset[ii][aa][tt]->SetParameter(1,0);
	      // fit_crystaloffset[ii][aa][tt]->SetParameter(2,90);
              //  fit_crystaloffset[ii][aa][tt]->SetParameter(4,100);

	      /*                npeaks = s->Search(crystaloffset[ii][jj][kk][aa][tt],2,"",0.2);
                if (npeaks) {
		  mean_crystaloffset[ii][jj][kk][aa][tt] = getmax(s,npeaks) ; }
		  else*/ 




