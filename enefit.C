
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
#include "ModuleDat.h"
#include "ModuleCal.h"
//#include "/home/miil/root/macros/myrootlib.h"
//#include "/home/miil/root/libInit_avdb.h"
//#include "./convertconfig.h"

//#define FILENAMELENGTH	120
//#define MAXFILELENGTH	160
//#define E_up 2500
//#define E_low -100
//#define Ebins 260
//#define PEAKS 64
//#define EBINS 150
//#define CHIPS 2

//void usage(void);
//void usage(void){
// cout << " Parsetomodule -f [filename] [-p [pedfile] -v -o [outputfilename]] -n [nrfiles in loop] -t [threshold]" <<endl;
//  return;}
Float_t getpeak(TH1F *hist, Float_t xlow, Float_t xhigh, Int_t verbose);
Int_t getcrystal(Double_t x, Double_t y, Double_t xpos[64], Double_t ypos[64], Int_t verbose );

int main(int argc, Char_t *argv[])
{
 	cout << "Welcome to enefit " << endl;

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	Char_t		filename[FILENAMELENGTH] = "";
	Int_t		verbose = 0;//, threshold=-800;
	Int_t		ix;
        ModuleDat *event = 0;
        ModuleCal   *calevent =  new ModuleCal();

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
        Char_t tmpname[20],tmptitle[50];
        Int_t i,j,k,m,lines;
        Int_t validpeaks[RENACHIPS][4][2];
        Double_t U_x[RENACHIPS][4][2][64];
        Double_t U_y[RENACHIPS][4][2][64];
        Double_t aa, bb;
        ifstream infile;
        TH1F *Ehist[RENACHIPS][4][2][64];
        TH1F *Ehist_com[RENACHIPS][4][2][64];
        TH1F *globhist[RENACHIPS][4][2];
        TF1 *globfits[RENACHIPS][4][2];
        TH1F *globhist_com[RENACHIPS][4][2];
        TF1 *globfits_com[RENACHIPS][4][2];
        TF1 *Efits[RENACHIPS][4][2][64];
        TF1 *Efits_com[RENACHIPS][4][2][64];
        TDirectory *subdir[RENACHIPS];

	//        strncpy(filebase,filename,strlen(filename)-17);
	//        filebase[strlen(filename)-17]='\0';
        sprintf(rootfile,"%s",filename);
	//        strcat(rootfile,".root");
              
	cout << "Rootfile to open :: " << rootfile << endl;

	TFile *enefile = new TFile(rootfile,"OPEN");       
         if (!enefile || enefile->IsZombie()) {  
         cout << "problems opening file " << rootfile << "\n.Exiting" << endl; 
         return -11;}



        TVector* ppVals = (TVector *) enefile->Get("pp_spat");
        TVector* ppVals_com = (TVector *) enefile->Get("pp_com");

	//	enefile->ls() ;

        strncpy(filebase,filename,strlen(filename)-12);
        filebase[strlen(filename)-12]='\0';

        for (m=0;m<RENACHIPS;m++){
        for (i=0;i<4;i++)
        for (j=0;j<2;j++){
	  { validpeaks[m][i][j]=0;
	    sprintf(peaklocationfilename,"%s.RENA%d.unit%d_apd%d_peaks",filebase,m,i,j);
       strcat(peaklocationfilename,".txt");
       infile.open(peaklocationfilename);
       lines = 0;
       while (1){
       if (!infile.good()) break;
       infile >> k >>  aa >> bb;
       if (k < 64) { U_x[m][i][j][k]=aa; U_y[m][i][j][k]=bb;}
       //       cout << k << ", " << U_x[i][j][k]<< ", " << U_y[i][j][k] << endl;
       lines++;      
       }
       if (verbose) cout << "Found " << lines-1 << " peaks in  file " << peaklocationfilename << endl;
       infile.close();
       if ((lines==65)){ if (verbose) cout << "Setting Validpeaks " << endl; validpeaks[m][i][j]=1; }
	  }//j
	} //i 
	} //m

	if (verbose){
	  for (m=0;m<RENACHIPS;m++){
      	  for (i=0;i<4;i++){
        for (j=0;j<2;j++){      
	  cout << " m = " << m << " i = " << i << " j = " << j << " : " <<validpeaks[m][i][j] <<endl;
	} //j 
	  }//i
	  }//m
 
	  cout <<  " File content :: " << endl;
          enefile->ls();
	} //verbose
	

 

	/* Getting E hists */
      Char_t tmpstring[30];
      for (m=0;m<RENACHIPS;m++){
	//        sprintf(tmpstring,"RENA%d",m);
	//      	enefile->cd(tmpstring);
	//        subdir[m]->cd();
           for (i=0;i<4;i++){
           for (j=0;j<2;j++){ 
	   for (k=0;k<64;k++){              
	     sprintf(tmpstring,"RENA%d/Ehist[%d][%d][%d][%d];1",m,m,i,j,k);
              Ehist[m][i][j][k]=(TH1F *) enefile->Get(tmpstring); 
              sprintf(tmpstring,"Efits[%d][%d][%d][%d];1",m,i,j,k);
              Efits[m][i][j][k]= new TF1(tmpstring,"gaus",EFITMIN,EFITMAX);
              sprintf(tmpstring,"Efits_com[%d][%d][%d][%d];1",m,i,j,k);
              Efits_com[m][i][j][k]= new TF1(tmpstring,"gaus",EFITMIN_COM,EFITMAX_COM);
    	     sprintf(tmpstring,"RENA%d/Ehist_com[%d][%d][%d][%d];1",m,m,i,j,k);
              Ehist_com[m][i][j][k]=(TH1F *) enefile->Get(tmpstring); 

	   }//k
	   }//j 
	   } //i  
      }  // m


	
      //	  TF1 *ftmp[64];
          Float_t CRYSTALPP[RENACHIPS][4][2][64];
          Float_t CRYSTALPP_COM[RENACHIPS][4][2][64];
	/* Fitting E spectra */
	  Float_t Emean,Emean_com,peak,Emeans[8*RENACHIPS],Emeans_com[8*RENACHIPS];
	     /* fitting energy spectra */



	  for (m=0;m<RENACHIPS;m++){
           for (i=0;i<4;i++){
          for (j=0;j<2;j++){ 
	     if (validpeaks[m][i][j]==1) {
	       if (verbose) cout << " Getting mean energy :: " << endl;
	       Emean = (Float_t )  (*ppVals)(m*8+j*4+i);
	       Emeans[m*8+j*4+i] = (Float_t )  (*ppVals)(m*8+j*4+i);
	       Emean_com = (Float_t )  (*ppVals_com)(m*8+j*4+i);
	       Emeans_com[m*8+j*4+i] = (Float_t )  (*ppVals_com)(m*8+j*4+i);
	       if (verbose) cout  << Emean   << " " << Ehist[m][i][j][0]->GetEntries()   << endl;
               for (k=0;k<64;k++){
	      //	      fitall(Ehist[i][j], ftmp, &vals[0], pixvals, E_low, E_up, c1, verbose);
		 sprintf(tmpstring,"Efits[%d][%d][%d][%d]",m,i,j,k);
              if (verbose) {
                cout << "===================================================================" << endl;
                cout << "= Histogram["<<m<<"]["<<i<<"]["<<j<<"]["<<k<<"]"                             << endl;
                cout << "===================================================================" << endl;
                cout << " ----------- Spatials ---------------- " <<endl;
                cout << " Hist entries : " << Ehist[m][i][j][k]->GetEntries() << " Efits mean :" ;
		cout << Efits[m][i][j][k]->GetParameter(1) << endl;}
              peak=getpeak( Ehist[m][i][j][k], Emean*0.85,Emean*1.15,verbose);
              Efits[m][i][j][k]->SetParameter(1,peak);
              Efits[m][i][j][k]->SetParameter(2,0.04*peak);
              if (verbose) {
		cout << " Peak @ " << peak <<", fitting between : " << peak*0.85 << " and " << peak*1.15 << endl;}
              if ( (Ehist[m][i][j][k]->GetEntries()) >  MINEHISTENTRIES ){
		Ehist[m][i][j][k]->Fit(Efits[m][i][j][k],"Q","",peak*0.85,peak*1.15);}
              Efits[m][i][j][k]->SetLineColor(kRed);
	      CRYSTALPP[m][i][j][k]=Efits[m][i][j][k]->GetParameter(1);
              if (TMath::Abs(CRYSTALPP[m][i][j][k]/Emean -1 ) > 0.3 )          {
		if (verbose) cout << " BAD FIT to histogram ["<<m<<"]["<<i<<"]["<<j<<"]["<<k<<"]" << endl;
                CRYSTALPP[m][i][j][k]=Emean;}
	      if (verbose) {
                cout << " ------------ Common ----------------- " <<endl;
                cout << " Hist entries : " << Ehist_com[m][i][j][k]->GetEntries() << " Efits mean :" ;
		cout << Efits_com[m][i][j][k]->GetParameter(1) << endl;}
              peak=getpeak( Ehist_com[m][i][j][k], Emean_com*0.85,Emean_com*1.15,verbose);
              Efits_com[m][i][j][k]->SetParameter(1,peak);
              Efits_com[m][i][j][k]->SetParameter(2,0.04*peak);
              if (verbose) {
		cout << " Peak @ " << peak <<", fitting between : " << peak*0.85 << " and " << peak*1.15 << endl;}
              if ( (Ehist_com[m][i][j][k]->GetEntries()) >  MINEHISTENTRIES ){
		Ehist_com[m][i][j][k]->Fit(Efits_com[m][i][j][k],"Q","",peak*0.85,peak*1.15);}
              Efits_com[m][i][j][k]->SetLineColor(kBlue);
	      CRYSTALPP_COM[m][i][j][k]=Efits_com[m][i][j][k]->GetParameter(1);
              if (TMath::Abs(CRYSTALPP_COM[m][i][j][k]/Emean_com -1 ) > 0.3 )          {
		if (verbose) cout << " BAD FIT to histogram ["<<m<<"]["<<i<<"]["<<j<<"]["<<k<<"]" << endl;
                CRYSTALPP_COM[m][i][j][k]=Emean_com;}

	       } // loop over k
	     } // validpeaks
	   } // i 
	  } //j
	  } // m
	


	  if (verbose) {
	    cout << "Energy Calibration parameters :: " << endl;
	    for (m=0;m<RENACHIPS;m++){
	      cout << "Chip " << m << endl;
	      for (k=0;k<64;k++){
              for ( i=0;i<4;i++){
                for (j=0;j<2;j++){
                  cout << CRYSTALPP[m][i][j][k]<<" " << CRYSTALPP_COM[m][i][j][k]; }}
              cout << endl;}
	      cout <<endl; }
	  }



	  // Open Calfile //
        strncpy(filebase,filename,strlen(filename)-12);
        filebase[strlen(filename)-12]='\0';
        sprintf(rootfile,"%s",filebase);
        strcat(rootfile,".cal.root");

        TTree *calblock;

        TFile *calfile = new TFile(rootfile,"RECREATE");
	cout << " Creating file " << rootfile << endl;
             
          // Create Tree //

        TTree *cal;

        Char_t treename[40];
        Char_t treetitle[50];
	//   for (m=0;m<RENACHIPS;m++){
            sprintf(treename,"cal");
            sprintf(treetitle,"Energy calibrated LYSO-PSAPD module data ");
	    cal = new TTree(treename,treetitle);
            cal->Branch("Calibrated Event Data",&calevent);
	    /*
	    cal->Branch("ct",&calevent.ct,"ct/L");
	    cal->Branch("chip",&calevent.chip,"chip/S");
	    cal->Branch("module",&calevent.module,"module/S");
	    cal->Branch("apd",&calevent.apd,"apd/S");
	    cal->Branch("Ecal",&calevent.Ecal,"Ecal/F");
	    cal->Branch("E",&calevent.E,"E/F");
	    cal->Branch("Ec",&calevent.Ec,"Ec/F");
	    cal->Branch("Ech",&calevent.Ech,"Ech/F");
	    cal->Branch("x",&calevent.x,"x/F");
	    cal->Branch("y",&calevent.y,"y/F");
	    cal->Branch("ft",&calevent.ft,"ft/F");
	    cal->Branch("id",&calevent.id,"id/S");
	    cal->Branch("pos",&calevent.pos,"pos/I");
	    */
	    /*
            cal[m]->Branch("U0",&U0.ct,"ct/L:Ecal/D:E/D:Ec/D:Ech/D:x/D:y/D:ft1/D:ft2/D:id/I:mod/I:pos/I");
            cal[m]->Branch("U1",&U1.ct,"ct/L:Ecal/D:E/D:Ec/D:Ech/D:x/D:y/D:ft1/D:ft2/D:id/I:mod/I:pos/I");
            cal[m]->Branch("U2",&U2.ct,"ct/L:Ecal/D:E/D:Ec/D:Ech/D:x/D:y/D:ft1/D:ft2/D:id/I:mod/I:pos/I");
            cal[m]->Branch("U3",&U3.ct,"ct/L:Ecal/D:E/D:Ec/D:Ech/D:x/D:y/D:ft1/D:ft2/D:id/I:mod/I:pos/I");
	    */

	    for (m=0; m<RENACHIPS;m++){
             for (i=0;i<4;i++){ 
              for (j=0;j<2;j++){
               sprintf(tmpstring,"globhist[%d][%d][%d]",m,i,j);
    	       sprintf(tmptitle,"Global Espec RENA %d Unit %d module %d",m,i,j);
               globhist[m][i][j] = new TH1F(tmpstring,tmptitle,Ebins,E_low,E_up);
               sprintf(tmpstring,"globhist_com[%d][%d][%d]",m,i,j);
	       sprintf(tmptitle,"Global Common Espec RENA %d Unit %d module %d",m,i,j);
               globhist_com[m][i][j] = new TH1F(tmpstring,tmptitle,Ebins_com,E_low_com,E_up_com);
	      }//j
	     }//i
	    }//m





	//	enefile->ls();


	    //    for (m=0;m<RENACHIPS;m++){
      /*	  if (m==0)  {
            block = (TTree *) enefile->Get("calblock1");
	  }
          else  block = (TTree *) enefile->Get("calblock2");
      */
        sprintf(treename,"calblock");
        calblock = (TTree *) enefile->Get(treename);
        if (!calblock) {
	  cout << "error reading tree " << calblock << " from file " << rootfile <<endl;}
        cout << " Performing calibration over " << calblock->GetEntries() << " entries." <<  endl;

	/*
        block->SetBranchAddress("UNIT0",&UNIT0);
        block->SetBranchAddress("UNIT1",&UNIT1);
        block->SetBranchAddress("UNIT2",&UNIT2);
        block->SetBranchAddress("UNIT3",&UNIT3);
	*/
         calblock->SetBranchAddress("eventdata",&event);
	 /*
          calblock->SetBranchAddress("ct",&event.ct);
	  calblock->SetBranchAddress("chip",&event.chip);
	  calblock->SetBranchAddress("module",&event.module);
	  calblock->SetBranchAddress("apd",&event.apd);
	  calblock->SetBranchAddress("Ec",&event.Ec);
	  calblock->SetBranchAddress("Ech",&event.Ech);
	  calblock->SetBranchAddress("x",&event.x);
	  calblock->SetBranchAddress("y",&event.y);
	  calblock->SetBranchAddress("E",&event.E);
	  calblock->SetBranchAddress("ft",&event.ft);
	  calblock->SetBranchAddress("a",&event.a);
	  calblock->SetBranchAddress("b",&event.b);
	  calblock->SetBranchAddress("c",&event.c);
	  calblock->SetBranchAddress("d",&event.d);
	  calblock->SetBranchAddress("id",&event.id);
	  calblock->SetBranchAddress("pos",&event.pos);
	 */


	/*
        strncpy(filebase,filename,strlen(filename)-17);
        filebase[strlen(filename)-17]='\0';

       sprintf(newrootfile,"%s",filebase);
       strcat(newrootfile,".root");

          TFile *f = new TFile(newrootfile,"RECREATE");
          TTree *calblock = block->CloneTree(0);
       
	  for (j=0;j<2;j++){
            for (i=0;i<4;i++){
	      for (k=0;k<64;k++){
                sprintf(tmpname,"Ehist[%d][%d][%d]",i,j,k);
                sprintf(tmptitle,"Unit %d Module %d Pixel %d",i,j,k);
                Ehist[i][j][k] = new TH1F(tmpname,tmptitle,EBINS,EMIN,EMAX);
                sprintf(tmpname,"Efits[%d][%d][%d]",i,j,k);
                sprintf(tmptitle,"Unit %d Module %d Pixel %d",i,j,k);
                Efits[i][j][k] = new TF1(tmpname,"gaus",EBINS,EMIN,EMAX);
	      }
            }
          }
	*/

	  if (verbose) cout << " Looping over entries: " <<endl;
       for (k=0;k<calblock->GetEntries();k++){


	 //	   for (k=0;k<5e5;k++){

          calblock->GetEntry(k);
          calevent->chip=event->chip;
          calevent->m=event->module; 
          calevent->apd=event->apd;
          calevent->fin=-1;
          calevent->E=event->E;
          calevent->Ecal=0;
          calevent->x=event->x;
          calevent->y=event->y;
          calevent->ct=event->ct;
          calevent->ft=event->ft;
          calevent->id=-1;        
          calevent->pos=event->pos;

	  if (validpeaks[event->chip][event->module][event->apd]) {
           if ((event->id)>=0) {
	     	   calevent->E=event->E* Emeans[event->chip*8+event->apd*4+event->module]/CRYSTALPP[event->chip][event->module][event->apd][event->id];
	           calevent->Ecal=event->E* 511/CRYSTALPP[event->chip][event->module][event->apd][event->id];
	           calevent->Ec=-event->Ec* 511/CRYSTALPP_COM[event->chip][event->module][event->apd][event->id];
	           globhist[event->chip][event->module][event->apd]->Fill(calevent->E); 
	           globhist_com[event->chip][event->module][event->apd]->Fill(calevent->Ec); 
           calevent->id=event->id;
	   }  //event->id >0
         } // validpeaks
	  else { calevent->E=event->E; calevent->Ecal=event->E; calevent->id=72; calevent->Ec=event->Ec;}
  
	    cal->Fill();


       } // loop over entries 



	

 Int_t hibin=0;
 Int_t max=0;
 Float_t xlow,xhigh;
 // Double_t params[6];
 Double_t eres,d_eres;
 TPaveText *labeltxt = new TPaveText(.12,.8,.5,.88,"NDC");
 labeltxt->SetFillColor(kWhite);
   TCanvas *c1;
   c1 = (TCanvas*)gROOT->GetListOfCanvases()->FindObject("c1");
  if (!c1) c1 = new TCanvas("c1","c1",10,10,1000,1000);
   c1->SetCanvasSize(700,700);

   calfile->cd();
   cal->Write(); 
   ppVals->Write();
   if (verbose)   cout << "Fitting global histogram: " <<endl;

   for (m=0;m<RENACHIPS;m++){
     calfile->cd();
     sprintf(tmpstring,"RENA%d",m);
     subdir[m] = calfile->mkdir(tmpstring);
               subdir[m]->cd();
    for (Int_t i=0;i<4;i++){
     for (Int_t j=0;j<2;j++){
	    hibin=0;
            max=0;
	    xlow = Emeans[m*8+j*4+i]*0.85;
	    xhigh = Emeans[m*8+j*4+i]*1.15;
            c1->Clear();
            if (globhist[m][i][j]->GetEntries() > MINHISTENTRIES){
	    if (verbose){
	      cout << "Fitting global energy histogram RENA " << m << " UNIT " << i << " APD " << j << endl;}
      	       globfits[m][i][j]=fitapdspec(globhist[m][i][j],xlow,xhigh,1,verbose);
	       sprintf(tmpname,"globfits[%d][%d][%d]",m,i,j);
               globfits[m][i][j]->SetName(tmpname);
               globhist[m][i][j]->Draw();
               eres=2.35*globfits[m][i][j]->GetParameter(5)/globfits[m][i][j]->GetParameter(4);
               d_eres=2.35*errorprop_divide(globfits[m][i][j]->GetParameter(5),globfits[m][i][j]->GetParError(5),
                                       globfits[m][i][j]->GetParameter(4),globfits[m][i][j]->GetParError(4));
	    }
	    else{
	      // in case not enough entries we still want to store the globhist and draw it,
               globhist[m][i][j]->Draw();
	      eres=0.99;d_eres=0.99; 
	    }  // else
               sprintf(tmpstring,"Eres = %.2f #pm %.2f FWHM",100*eres,100*d_eres);
               labeltxt->Clear();
               labeltxt->AddText(tmpstring);
               sprintf(peaklocationfilename,"%s.RENA%d.unit%d_apd%d_glob",filebase,m,i,j);
               strcat(peaklocationfilename,".png");
               labeltxt->Draw();
	       c1->Print(peaklocationfilename);
	      //            peak=getpeak
              
     	    globhist[m][i][j]->Write();
		    // FIXME should create a directory on this file
	    for(k=0;k<64;k++){ Ehist[m][i][j][k]->Write();Efits[m][i][j][k]->Write();}
	  }//i
	}//j
   }//m


   if (verbose)   cout << "Fitting Common global histogram common: " <<endl;

   for (m=0;m<RENACHIPS;m++){
     calfile->cd();
     //     sprintf(tmpstring,"RENA%d",m);
     //     subdir[m] = calfile->mkdir(tmpstring);
     subdir[m]->cd();
    for (Int_t i=0;i<4;i++){
     for (Int_t j=0;j<2;j++){
	    hibin=0;
            max=0;
	    /*
	    xlow = Emeans_com[m*8+j*4+i]*0.85;
	    xhigh = Emeans_com[m*8+j*4+i]*1.15;
	    */
            xlow=400;xhigh=600;
            c1->Clear();
            if (globhist_com[m][i][j]->GetEntries() > MINHISTENTRIES){
	    if (verbose){
	      cout << "Fitting common global energy histogram RENA " << m << " UNIT " << i << " APD " << j << endl;
              cout << " Emeans_com = " << Emeans_com[m*8+j*4+i] << endl;}
      	       globfits_com[m][i][j]=fitapdspec(globhist_com[m][i][j],xlow,xhigh,1,verbose);
	       sprintf(tmpname,"globfits_com[%d][%d][%d]",m,i,j);
               globfits_com[m][i][j]->SetName(tmpname);
               globhist_com[m][i][j]->Draw();
               eres=2.35*globfits_com[m][i][j]->GetParameter(5)/globfits_com[m][i][j]->GetParameter(4);
               d_eres=2.35*errorprop_divide(globfits_com[m][i][j]->GetParameter(5),globfits_com[m][i][j]->GetParError(5),
                                       globfits_com[m][i][j]->GetParameter(4),globfits_com[m][i][j]->GetParError(4));
	    }
	    else{
	      // in case not enough entries we still want to store the globhist and draw it,
               globhist_com[m][i][j]->Draw();
	      eres=0.99;d_eres=0.99; 
	    }  // else
               sprintf(tmpstring,"Eres = %.2f #pm %.2f FWHM",100*eres,100*d_eres);
               labeltxt->Clear();
               labeltxt->AddText(tmpstring);
               sprintf(peaklocationfilename,"%s.RENA%d.unit%d_apd%d_glob_com",filebase,m,i,j);
               strcat(peaklocationfilename,".png");
               labeltxt->Draw();
	       c1->Print(peaklocationfilename);
	      //            peak=getpeak
              
              globhist_com[m][i][j]->Write();
		    // FIXME should create a directory on this file
	    for(k=0;k<64;k++){ Ehist_com[m][i][j][k]->Write();Efits_com[m][i][j][k]->Write();}
	  }//i
	}//j
   }//m


 
          calfile->Close();
         
      
	  ofstream ofile;

   for (m=0;m<RENACHIPS;m++){
     for (i=0;i<4;i++){
     for (j=0;j<2;j++){
       sprintf(peaklocationfilename,"%s.RENA%d.unit%d_apd%d_cal",filebase,m,i,j);
       strcat(peaklocationfilename,".txt");
       if (validpeaks[m][i][j]){
       ofile.open(peaklocationfilename);
       for (k=0;k<64;k++){
	 ofile << U_x[m][i][j][k] << " " << U_y[m][i][j][k] << " " << CRYSTALPP[m][i][j][k];
         if (Efits[m][i][j][k]->GetParameter(1)){
	   ofile << "  " << 235*Efits[m][i][j][k]->GetParameter(2)/Efits[m][i][j][k]->GetParameter(1) ;}
         else { ofile << "  0    " ; } 
         ofile << " " << CRYSTALPP_COM[m][i][j][k];
          if (Efits_com[m][i][j][k]->GetParameter(1)){
	   ofile << "  " << 235*Efits_com[m][i][j][k]->GetParameter(2)/Efits_com[m][i][j][k]->GetParameter(1) ;}
         else { ofile << "  0    " ; } 


         ofile << endl;}
        ofile.close();
       }
     } // j
     } //i 
   } //m

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



Float_t getpeak(TH1F *hist, Float_t xlow, Float_t xhigh, Int_t verbose){
  Int_t npeaks=0;
  Int_t i=0,corpeak=0;
  TSpectrum *ss = new TSpectrum();
  Float_t y;
  //  Float_t corpeakpos;

  y=0;

	while((npeaks < 3) && (i < 8)) {
	if(verbose) {cout << "loop " << i << " " << npeaks << endl;}
	npeaks = ss->Search(hist, 3, "", 0.9 - (Float_t) i / 10);
        i++;
	if(i > 10) {if(verbose) {cout << " Warning " << npeaks << "found !" << endl;}
				    break;}
		} //while loop

	if(verbose) {
	   cout << npeaks << " peaks found " << i << " number of iterations" << endl;
	   cout << *ss->GetPositionX() << endl;
		}

	if(npeaks != 1) {
	for(i = 0; i < npeaks; i++) {
	if(verbose) {cout << "x= " << *(ss->GetPositionX() + i) << " y = " << *(ss->GetPositionY() + i) << endl;}
	/* take largest peak with x position larger than lower fit limit */
       if((y < *(ss->GetPositionY() + i)) && (*(ss->GetPositionX() + i) > xlow) && (*(ss->GetPositionX() +i) < xhigh)) {
	corpeak = i;
        y = *(ss->GetPositionY() + i);
			}
		} // for loop

	if(verbose) {
			cout << "The correct peak out of " << npeaks << " peaks is peak number " << corpeak;
			cout << " at position: x = " << *(ss->GetPositionX() + corpeak) << endl;
		}
	//	corpeakpos=*(ss->GetPositionX() + corpeak);
            

 // FIRST CHECK ::
  if((*(ss->GetPositionX() + corpeak)) < xlow) {

      if(verbose) {cout << " Peak at " << *(ss->GetPositionX() + corpeak) << " is not within the window " << endl;}
        // move SIGMA rather than threshold
      i=0; npeaks=0; 
      Int_t j;
      Int_t stop=0;
         while((i < 8)&&(!stop)) {
        	npeaks = ss->Search(hist, 10 - i, "", 0.4);
		if(verbose) {cout << "2nd while loop " << i << " " << npeaks << endl;}
		if(verbose) {cout << npeaks << " peaks found " << i << " number of iterations" << endl;
			     cout << " peak position :: " << *ss->GetPositionX() << endl;}
	                 i++;
        	if(npeaks != 1) {
	       	for(j = 0; j < npeaks; j++) {
	       	if(verbose) {cout << "x= " << *(ss->GetPositionX() + j) << " y = " << *(ss->GetPositionY() + j) << endl;}
	/* take largest peak with x position larger than lower fit limit */
		/* CHANGE !! */
		if((y < *(ss->GetPositionY() + j)) && (*(ss->GetPositionX() + j) > xlow) && (*(ss->GetPositionX() +j) < xhigh)) {
		corpeak = j;
		y = *(ss->GetPositionY() + j);
	            		}
			} // for loop

		if(verbose) {cout << "The correct peak out of " << npeaks << " peaks is peak number " << corpeak;
			     cout << " at position: x = " << *(ss->GetPositionX() + corpeak) << endl;}
         	} // if npeaks !=1

                else corpeak=0;

              
	if((*(ss->GetPositionX() + corpeak)) < xlow) {
		if(verbose) {cout << "Peak at " << *(ss->GetPositionX() + corpeak) << " is not within the window " << endl;} }
        else { if (verbose) {cout << "Valid peak found at : " << *(ss->GetPositionX()+corpeak) << endl;} stop=1; }
       } // while loop

  if (!(stop)){ 
	y = 0;
	npeaks = ss->Search(hist, 2, "", 0.4);
	for(i = 0; i < npeaks; i++) {
	if(verbose) {cout << "x= " << *(ss->GetPositionX() + i) << " y = " << *(ss->GetPositionY() + i) << endl;}

				/* take largest peak with x position larger than lower fit limit */
			/* CHANGE !! */
	if((y < *(ss->GetPositionY() + i)) && (*(ss->GetPositionX() + i) > xlow) && (*(ss->GetPositionX() +i) < xhigh)) {
			corpeak = i;
			y = *(ss->GetPositionY() + i);
			}
		} // for loop

	if(verbose) {
			cout << "The correct peak out of " << npeaks << " peaks is peak number " << corpeak;
			cout << " at position: x = " << *(ss->GetPositionX() + corpeak) << endl;
		}
// SECOND CHECK
	if((*(ss->GetPositionX() + corpeak)) < xlow) {
			if(verbose) {cout << "Peak at " << *(ss->GetPositionX() + corpeak) << " is STILL not within the window " << endl;}
                       
         /* Lower "SIGMA" of tspectrum */
         npeaks = ss->Search(hist, 2, "", 0.3);
	for(i = 0; i < npeaks; i++) {
	  if(verbose) {cout << "x= " << *(ss->GetPositionX() + i) << " y = " << *(ss->GetPositionY() + i) << endl;}
				/* take largest peak with x position larger than lower fit limit */
			/* CHANGE !! */
	 if((y < *(ss->GetPositionY() + i)) && (*(ss->GetPositionX() + i) > xlow) && (*(ss->GetPositionX() +i) < xhigh)) {
		corpeak = i;
		y = *(ss->GetPositionY() + i);
			}
		} // for loop

		if(verbose) {
			cout << "The correct peak out of " << npeaks << " peaks is peak number " << corpeak;
			cout << " at position: x = " << *(ss->GetPositionX() + corpeak) << endl;
		}

// THIRD CHECK
   if((*(ss->GetPositionX() + corpeak)) < xlow) {
			if(verbose) {
	cout << "Peak at " << *(ss->GetPositionX() + corpeak) << " is STILL not within the window " << endl;
			}

     /* Lower "SIGMA, threshold" of tspectrum */
 		npeaks = ss->Search(hist, 2, "", 0.3);
		for(i = 0; i < npeaks; i++) {
		if(verbose) {cout << "x= " << *(ss->GetPositionX() + i) << " y = " << *(ss->GetPositionY() + i) << endl;}
				/* take largest peak with x position larger than lower fit limit */
			
			/* CHANGE !! */
	if((y < *(ss->GetPositionY() + i)) && (*(ss->GetPositionX() + i) > xlow) && (*(ss->GetPositionX() +i) < xhigh)) {
		corpeak = i;
		y = *(ss->GetPositionY() + i);
			}
		} // for loop

		if(verbose) {
			cout << "The correct peak out of " << npeaks << " peaks is peak number " << corpeak;
			cout << " at position: x = " << *(ss->GetPositionX() + corpeak) << endl;
		}

// FOURTH CHECK
		if((*(ss->GetPositionX() + corpeak)) < xlow) {
			if(verbose) {
				cout << "Peak at " << *(ss->GetPositionX() + corpeak) << " is STILL not within the window " << endl;
			}
                  

 // Giving up, taking the highest peak 

			y = 0;
			for(i = 0; i < npeaks; i++) {
				if(y < *(ss->GetPositionX() + i)) {
					corpeak = i;
					y = *(ss->GetPositionX() + i);
				}
			}	/* for loop */

			if(verbose) {
				cout << "The correct peak out of " << npeaks << " peaks is peak number " << corpeak;
				cout << " at position: x = " << *(ss->GetPositionX() + corpeak) << endl;
			}
		} // giving up
	 } //npeaks = ss->Search(hist, 2, "", 0.3);
   } //npeaks = ss->Search(hist, 2, "", 0.3);

} // 4th check
} // 3rd check
} // 2nd check
 // first check if the location of peak is okay.
	return *(ss->GetPositionX()+corpeak);} // else corresponding to  if (com)
