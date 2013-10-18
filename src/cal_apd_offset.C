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




#define MINMODENTRIES 200

Int_t drawmod(TH1F *hi[UNITS][MODULES_PER_FIN][APDS_PER_MODULE], TCanvas *ccc, Char_t filename[MAXFILELENGTH]);
Int_t writ(TH1D *hi[PEAKS], TCanvas *ccc, Char_t filename[MAXFILELENGTH]);
Int_t writ2d(TH2F *hi[PEAKS], TCanvas *ccc, Char_t filename[MAXFILELENGTH]);
TH2F *get2dcrystal(Float_t vals[64], Char_t title[40]) ;
Float_t calfunc(TH1F *hist, TF1 *fitfun,  Bool_t coarsetime, Bool_t gausfit);
Float_t getmax (TSpectrum *, Int_t);

int main(int argc, Char_t *argv[])
{ 
  Int_t MOD1=6;
  Int_t MOD2=1;
  Int_t APD1=0;
  Int_t APD2=0;
  Bool_t coarsetime=1; 
  Int_t crystalmean=0;
  Int_t usegausfit=0;
  Char_t histtitle[40];

 Int_t DTF_low, DTF_hi, FINELIMIT,DTFLIMIT;

 	cout << "Welcome " << endl;

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	Char_t		filenamel[CALFILENAMELENGTH] = "";
	Int_t		verbose = 0;
	Int_t		ix;
	//module UNIT0,UNIT1,UNIT2,UNIT3;
        CoincEvent      *evt = new CoincEvent();
        CoincEvent      *calevt = new CoincEvent();
        Int_t fin1=99,fin2=99;
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



		/* filename '-f' */
		if(strncmp(argv[ix], "-f", 2) == 0) {

		if(strncmp(argv[ix], "-ft", 3) == 0) {
                  coarsetime=0;
                  ix++;
                  if (ix == argc ) { cout << " Please enter finelimit interval: -ft [finelimit]\nExiting. " << endl;
                    return -20;} 
                  FINELIMIT=atoi(argv[ix]);
                  if (FINELIMIT<1) { cout << " Error. FINELIMIT = " << FINELIMIT << " too small. Please specify -ft [finelimit]. " << endl;
		    cout << "Exiting." << endl; return -20;}
             
		  cout << " Fine time interval = "  << FINELIMIT << endl;
		}



                else {
		if(strncmp(argv[ix], "-f1", 3) == 0) {
                  fin1=atoi ( argv[ix+1]); ix++;
		  cout << " Fin 1 :: "  <<fin1<< endl;
		} 
		
                else {
		if(strncmp(argv[ix], "-f2", 3) == 0) {
                  fin2=atoi ( argv[ix+1]) ; ix++;
		  cout << " Fin 2 :: " << fin2 <<endl;
		}

		else {
			if(strlen(argv[ix + 1]) < CALFILENAMELENGTH) {
				sprintf(filenamel, "%s", argv[ix + 1]);
			}
			else {
				cout << "Filename " << argv[ix + 1] << " too long !" << endl;
				cout << "Exiting.." << endl;
				return -99;
			}
		}
		}
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
       
        Char_t filebase[CALFILENAMELENGTH],rootfile[CALFILENAMELENGTH]; 
        Char_t tmpstring[50];
        Int_t i;
        ifstream infile;
 
        TH1F *apdoffset[2][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE];
        TF1 *fit_apdoffset[2][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE];

	Int_t tt, mod, aa,ii;

	if (coarsetime ) {  DTF_low = -300; DTF_hi = 300; FINELIMIT=400; DTFLIMIT=200; cout << " Using Coarse limits: " << FINELIMIT << endl;}
        else { DTF_low = -50; DTF_hi = 50;  DTFLIMIT=5;}


   
	for (ii=0;ii<2;ii++) {
        for (tt=0;tt<FINS_PER_CARTRIDGE;tt++) {
	  for (mod=0;mod<MODULES_PER_FIN;mod++) {
            for (aa=0;aa<APDS_PER_MODULE;aa++) {
              sprintf(histtitle,"apdoffset[%d][%d][%d][%d]",ii,tt,mod,aa);
              if ((ii==0) || (!(coarsetime)))  apdoffset[ii][tt][mod][aa]= new TH1F(histtitle,histtitle,50,DTF_low,DTF_hi);
              else  apdoffset[ii][tt][mod][aa]= new TH1F(histtitle,histtitle,50,DTF_low+150,DTF_hi-150);
	    }
	  }
	}
	}


        cout << " Opening file " << filenamel << endl;
        TFile *rtfile = new TFile(filenamel,"OPEN");
        TTree *mm  = (TTree *) rtfile->Get("merged");

        if (!mm) {
	  cout << " Problem reading Tree merged "   << " from file " << filenamel << ". Trying tree mana." << endl;
          mm  = (TTree *) rtfile->Get("mana"); }
      
        if (!mm) {
        cout << " Problem reading Tree mana "   << " from file " << filenamel << endl;
        cout << " Exiting " << endl;
        return -10;}
   
        mm->SetBranchAddress("Event",&evt);

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
	 if ((evt->crystal1<65)&&((evt->apd1==0)||(evt->apd1==1))&&(evt->m1<MODULES_PER_FIN)) {
	     if ((evt->E1>400)&&(evt->E1<600)) {
	       //	     if ((evt->E2>400)&&(evt->E2<600)) {
               if (TMath::Abs(evt->dtc ) < 6 ) {
	       if (TMath::Abs(evt->dtf ) < FINELIMIT ) {
                 checkevts++;
		 //	    		 crystime[0][evt->m1][evt->apd1][evt->crystal1]->Fill(evt->dtf);
		 apdoffset[0][evt->fin1][evt->m1][evt->apd1]->Fill(evt->dtf);
	       }
               }
	     }
	   }
	 }
          // loop over entries

       cout << " Done looping over entries " << endl;
       cout << " I made " << checkevts << " calls to Fill() " << endl;         

       Float_t mean_apdoffset[2][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE];
       Int_t npeaks;
       TSpectrum *s = new TSpectrum();

       ii=0;

        for (tt=0;tt<FINS_PER_CARTRIDGE;tt++) {
	  for (mod=0;mod<MODULES_PER_FIN;mod++) {
            for (aa=0;aa<APDS_PER_MODULE;aa++) {
              mean_apdoffset[ii][tt][mod][aa] =calfunc(apdoffset[ii][tt][mod][aa],fit_apdoffset[ii][tt][mod][aa],coarsetime,usegausfit);
              sprintf(tmpstring,"fit_apdoffset[%d][%d][%d][%d]",ii,tt,mod,aa);
	      //              fit_apdoffset[ii][tt][mod][aa]->SetName(tmpstring);
            }}}

	c1->Clear();
        apdoffset[0][6][2][0]->Draw();
	//        fit_apdoffset[0][6][2][0]->Draw("same");
        c1->Print("test.ps");

	  Char_t psfile[MAXFILELENGTH];
	  sprintf(psfile,"%s_fin1.ps",rootfile);

       drawmod(apdoffset[0],c1,psfile);
        for (tt=0;tt<FINS_PER_CARTRIDGE;tt++) {
	  for (mod=0;mod<MODULES_PER_FIN;mod++) {
             for (aa=0;aa<APDS_PER_MODULE;aa++) {
	       cout << mean_apdoffset[ii][tt][mod][aa] << " ";}
	     if (mod%2) cout << "| ";}
	  cout << endl;}


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
		 apdoffset[1][evt->fin2][evt->m2][evt->apd2]->Fill(evt->dtf- mean_apdoffset[0][evt->fin1][evt->m1][evt->apd1]);
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



        for (tt=0;tt<FINS_PER_CARTRIDGE;tt++) {
	  for (mod=0;mod<MODULES_PER_FIN;mod++) {
            for (aa=0;aa<APDS_PER_MODULE;aa++) {
              sprintf(tmpstring,"fit_apdoffset[%d][%d][%d][%d]",ii,tt,mod,aa);
              mean_apdoffset[ii][tt][mod][aa] =calfunc(apdoffset[ii][tt][mod][aa],fit_apdoffset[ii][tt][mod][aa],coarsetime,usegausfit);
	    }}}

	  sprintf(psfile,"%s_fin2.ps",rootfile);


        drawmod(apdoffset[1],c1,psfile);
        for (tt=0;tt<FINS_PER_CARTRIDGE;tt++) {
	  for (mod=0;mod<MODULES_PER_FIN;mod++) {
             for (aa=0;aa<APDS_PER_MODULE;aa++) {
	       cout << mean_apdoffset[ii][tt][mod][aa] << " ";}
	     if (mod%2) cout << "| ";}
	  cout << endl;}

  

        strcat(rootfile,".apdoffcal.root");

    TH1F *tres = new TH1F("tres","Time Resolution After Time walk correction",100,-25,25);
       
      cout << " Opening file " << rootfile << " for writing " << endl;
      TFile *calfile = new TFile(rootfile,"RECREATE");
      TTree *merged = new  TTree("merged","Merged and Calibrated LYSO-PSAPD data ");
      //   merged->SetDirectory(0);
      merged->Branch("Event",&calevt);


      //      merged->Branch("event",&calevt->dtc,"dtc/L:dtf/D:E1/D:Ec1/D:Ech1/D:ft1/D:E2/D:Ec2/D:Ech2/D:ft2/D:x1/D:y1/D:x2/D:y2/D:chip1/I:fin1/I:m1/I:apd1/I:crystal1/I:chip2/I:fin2/I:m2/I:apd2/I:crystal2/I:pos/I");

     checkevts=0;
      cout << "filling new Tree :: " << endl;

        for (i=0;i<entries; i++) {
	 mm->GetEntry(i);
	 calevt=evt;
         if (evt->fin1>FINS_PER_CARTRIDGE) continue;
         if (evt->fin2>FINS_PER_CARTRIDGE) continue;
	 if ((evt->crystal1<65)&&((evt->apd1==APD1)||(evt->apd1==1))&&(evt->m1<MODULES_PER_FIN)) {
	 if ((evt->crystal2<65)&&((evt->apd2==APD1)||(evt->apd2==1))&&(evt->m2<MODULES_PER_FIN)) {

           calevt->dtf-= mean_apdoffset[0][evt->fin1][evt->m1][evt->apd1] ;
           calevt->dtf-= mean_apdoffset[1][evt->fin2][evt->m2][evt->apd2] ; 
           if (evt->E1>400&&evt->E1<600&&evt->E2>400&&evt->E2<600) {
	     tres->Fill(calevt->dtf);}
	 }
	 }
         checkevts++;
         merged->Fill();
	}
    

	cout << " New Tree filled with " << checkevts << " events. " << endl;

	merged->Write();
      calfile->Close();

 // crystime[0][MOD1][APD1][0]->Draw(); c1->Print("testje2.ps");
 // cout << crystime[0][MOD1][APD1][0]->GetEntries() << endl;
 // TFile *ff = new TFile("test.root");
 // crystime[0][0][0][0]->Write();
 // ff->Close();


      tres->Fit("gaus","","",-10,10);


      c1->Clear();
      tres->Draw();
      sprintf(psfile,"%s.tres.apdoffset.ps",rootfile);

      c1->Print(psfile);



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
  


Int_t drawmod(TH1F *hi[FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE], TCanvas *ccc, Char_t filename[MAXFILELENGTH])
{
        /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  Int_t   i,k;
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
        ccc->Divide(4, 8);

        for(k = 0; k < FINS_PER_CARTRIDGE ; k++) {
	  for(i = 0 ;i < MODULES_PER_FIN; i++ ) {
             ccc->cd(1+2*(i));
             hi[k][i][0]->Draw("E");
             ccc->cd(2+2*(i));
             hi[k][i][1]->Draw("E");
	  }
           if(k == 0) {
              ccc->Print(filenameo);
                        }
           else {
	     if(k == (FINS_PER_CARTRIDGE-1)) {
              ccc->Print(filenamec);
             }
             else {
              ccc->Print(filename);
                  }
          }
           ccc->Clear();
           ccc->Divide(4, 8);

                        /* } */
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

Float_t calfunc(TH1F *hist,  TF1 *fitfun, Bool_t coarsetime, Bool_t gausfit){
  //   TF1 *fitfun;
   Int_t npeaks;
   TSpectrum *s = new TSpectrum();
   if (gausfit) {
     if ( hist->GetEntries() > MINMODENTRIES ) {
     if (coarsetime){ 
       fitfun = new TF1("fitfun", "gaus+pol0",-75,75);
       fitfun->SetParameter(0,200);
       fitfun->SetParameter(1,0);
       fitfun->SetParameter(2,90);
       fitfun->SetParameter(4,100); 
       hist->Fit(fitfun,"RQ");
       return fitfun->GetParameter(1); 
      }
     else  {
       fitfun=fitgaus_peak(hist,1) ;
       return fitfun->GetParameter(1);}
     }
     // else  // not enough entries to fit 
     //  return 0;
   } // gausfit 
   //else {    // just peak search 
       npeaks = s->Search(hist,3,"",0.2);
       if (npeaks){  return getmax(s,npeaks); }
       else return 0;
   
   

}


