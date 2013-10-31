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
#define MINMODENTRIES 1000

Int_t drawmod_crys2(TH1F *hi[MODULES_PER_FIN][APDS_PER_MODULE][64], TCanvas *ccc, Char_t filename[MAXFILELENGTH]);
Int_t writ(TH1D *hi[PEAKS], TCanvas *ccc, Char_t filename[MAXFILELENGTH]);
Int_t writ2d(TH2F *hi[PEAKS], TCanvas *ccc, Char_t filename[MAXFILELENGTH]);
TH2F *get2dcrystal(Float_t vals[64], Char_t title[40]) ;
Float_t getmax (TSpectrum *, Int_t);

int main(int argc, Char_t *argv[])
{ 
  Int_t MOD1=6;
  Int_t MOD2=1;
  Int_t APD1=0;
  Int_t APD2=0;
  Int_t uvcal=0;
  Int_t energycal=0;
  Int_t coarsetime=1; 
  Int_t crystalmean=0;
  Int_t energyspatial=0;

  Int_t DTF_low, DTF_hi, FINELIMIT;

 	cout << "Welcome " << endl;

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	Char_t		filenamel[FILENAMELENGTH] = "";
	Int_t		verbose = 0;
	Int_t		ix,ascii;
	//module UNIT0,UNIT1,UNIT2,UNIT3;
        CoincEvent      *evt = new CoincEvent();     
        CoincEvent      *calevt = new CoincEvent();     
        Int_t fin1=99,fin2=99;
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


		if((strncmp(argv[ix], "-a", 2) == 0)&& (strncmp(argv[ix], "-apd",4) != 0 )) {
			cout << "Ascii output file generated" << endl;
			ascii = 1;
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

		if(strncmp(argv[ix], "-uv", 3) == 0) {
                  uvcal=1;
		  cout << " UV calibration "  <<endl;
		}

		if(strncmp(argv[ix], "-ec", 3) == 0) {
                  energycal=1;
		  cout << " Energy calibration "  <<endl;
		}

		if(strncmp(argv[ix], "-esp", 4) == 0) {
                  energyspatial=1;
		  cout << " Using spatials for energy calibration "  <<endl;
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
		if(strncmp(argv[ix], "-f1", 3) == 0) {
                  fin2=atoi ( argv[ix+1]) ; ix++;
		  cout << " Fin 2 :: " << fin2 <<endl;
		}

                else {
	       

		/* filename '-f' */

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

        TH2F *energydependence[2];



        coarsetime=0;

	Int_t ii;

	if (coarsetime ) {  DTF_low = -300; DTF_hi = 300; FINELIMIT=300;  cout << " Using Coarse limits: " << FINELIMIT << endl;}
        else { DTF_low = -FINELIMIT/2; DTF_hi = FINELIMIT/2;  }

	/*
   
	for (ii=0;ii<2;ii++) {
	  for (jj=0;jj<FINS;jj++) {
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
	*/


	energydependence[0] = new TH2F("energydependence[0]","Edep Panel 0",100,400,600,100,-50,50);
	energydependence[1] = new TH2F("energydependence[1]","Edep Panel 1",100,400,600,100,-50,50);

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
                 energydependence[0]->Fill(evt->E1,evt->dtf);
               }
	     }
	   }
	 }
       } // loop over entries
	  
       cout << " Done looping over entries " << endl;
       cout << " I made " << checkevts << " calls to Fill() " << endl;         


       TH1F *profehist[2];
       TF1 *profehistfit[2];

       profehistfit[0] = new TF1("profehistfit[0]","pol1",400,600);   
       profehistfit[1] = new TF1("profehistfit[1]","pol1",400,600);   

       profehist[0] = (TH1F *) energydependence[0]->ProfileX();
       profehist[0]->SetName("profehist[0]");

       if (verbose ) profehist[0]->Fit("profehistfit[0]");
       else profehist[0]->Fit("profehistfit[0]","Q");

       c1->Clear();
       c1->Divide(1,2);
       c1->cd(1);
       energydependence[0]->Draw("colz");
       c1->cd(2);
       profehist[0]->Draw();

        Char_t psfile[MAXFILELENGTH];
        sprintf(psfile,"%s_edpe_panel1.ps",rootfile);

	c1->Print(psfile);


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
                 energydependence[1]->Fill(evt->E2,evt->dtf-profehistfit[0]->Eval(evt->E1));
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

       profehist[1] = (TH1F *) energydependence[1]->ProfileX();
       profehist[1]->SetName("profehist[1]");

       if (verbose ) profehist[1]->Fit("profehistfit[1]");
       else profehist[1]->Fit("profehistfit[1]","Q");




       c1->Clear();
       c1->Divide(1,2);
       c1->cd(1);
       energydependence[1]->Draw("colz");
       c1->cd(2);
       profehist[1]->Draw();


        sprintf(psfile,"%s_edpe_panel2.ps",rootfile);

	c1->Print(psfile);


	  sprintf(psfile,"%s_fin2.ps",rootfile);




        strcat(rootfile,".edepcal.root");

      TH1F *tres = new TH1F("tres","Time Resolution After Time walk correction",100,-25,25);
       
      cout << " Opening file " << rootfile << " for writing " << endl;
      TFile *calfile = new TFile(rootfile,"RECREATE");
      TTree *merged = new  TTree("merged","Merged and Calibrated LYSO-PSAPD data ");
      //      merged->Branch("event",&calevt.dtc,"dtc/L:dtf/D:E1/D:Ec1/D:Ech1/D:ft1/D:E2/D:Ec2/D:Ech2/D:ft2/D:x1/D:y1/D:x2/D:y2/D:chip1/I:fin1/I:m1/I:apd1/I:crystal1/I:chip2/I:fin2/I:m2/I:apd2/I:crystal2/I:pos/I");
      merged->Branch("Event",&calevt);



      cout << "filling new Tree :: " << endl;

        for (i=0;i<entries; i++) {
	 mm->GetEntry(i);
	   calevt=evt;
         if (evt->fin1>FINS_PER_CARTRIDGE) continue;
         if (evt->fin2>FINS_PER_CARTRIDGE) continue;
	 if ((evt->crystal1<65)&&((evt->apd1==APD1)||(evt->apd1==1))&&(evt->m1<MODULES_PER_FIN)) {
	 if ((evt->crystal2<65)&&((evt->apd2==APD1)||(evt->apd2==1))&&(evt->m2<MODULES_PER_FIN)) {

           calevt->dtf-= profehistfit[0]->Eval(evt->E1);
           calevt->dtf-= profehistfit[1]->Eval(evt->E2);
             if (evt->E1>400&&evt->E1<600&&evt->E2>400&&evt->E2<600) {
	       tres->Fill(calevt->dtf); }
	 }
	 }
         merged->Fill();
	}
    

	merged->Write();

      calfile->Close();


      tres->Fit("gaus","","",-10,10);


      c1->Clear();
      tres->Draw();
      sprintf(psfile,"%s.tres.edepcal.ps",rootfile);

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
