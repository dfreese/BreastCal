#include "calibrate.h"
#include "decoder.h"
#include "TCanvas.h"
#include "time.h"
#include "Sel_Calibrator.h"
#include "TProof.h"

#define FIRSTCHIP 0
#define LASTCHIP RENACHIPS


/*
#define FIRSTCHIP 8
#define LASTCHIP 9
*/

int main(int argc, Char_t *argv[])
{
 	cout << " Welcome to enefit. Calibrate every pixel & Fill calibrated Tree. " ;

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	Char_t		filename[FILENAMELENGTH] = "";
	Int_t		verbose = 0;//, threshold=-800;
	Int_t		ix;
        ModuleDat *event = 0;
        ModuleCal   *calevent =  new ModuleCal();
        Bool_t genplots=0;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	//see eg http://root.cern.ch/phpBB3/viewtopic.php?f=14&t=3498
	gErrorIgnoreLevel=kError;
	for(ix = 1; ix < argc; ix++) {

		/*
		 * Verbose '-v'
		 */
		if(strncmp(argv[ix], "-v", 2) == 0) {
			cout << "Verbose Mode " << endl;
			gErrorIgnoreLevel=-1;
			verbose = 1;
		}


		if(strncmp(argv[ix], "-plots", 6) == 0) {
			cout << "Generating plots " << endl;
			genplots=1;

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

        cout <<  " Input file :: " << filename << "." << endl ;

        rootlogon(verbose);
        time_t starttime = time(NULL);


        Char_t filebase[FILENAMELENGTH],peaklocationfilename[FILENAMELENGTH],rootfile[FILENAMELENGTH]; 
        Char_t tmpname[20],tmptitle[50];
        Int_t i,j,k,m,lines;
        Int_t validpeaks[RENACHIPS][4][2]={{{0}}};
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
              
	if (verbose)	cout << "Rootfile to open :: " << rootfile << endl;

       	TFile *rfile = new TFile(rootfile,"OPEN");       
        if (!rfile || rfile->IsZombie()) {  
         cout << "problems opening file " << rootfile << "\n.Exiting" << endl; 
         return -11;}


	/*
        TVector* ppVals = (TVector *) enefile->Get("pp_spat");
        TVector* ppVals_com = (TVector *) enefile->Get("pp_com");
	*/


	//	enefile->ls() ;


	//        for (m=0;m<RENACHIPS;m++){
 
	if (verbose) {
	  cout <<  " File content :: " << endl;
	  //          enefile->ls();
} //verbose
	

 



  
	TCanvas *c1;
     c1  = (TCanvas*)gROOT->GetListOfCanvases()->FindObject("c1");
    if (!c1) c1 = new TCanvas("c1","c1",10,10,1000,1000);
  
    c1->SetCanvasSize(1754,1240);
	  // plot histograms
 

	//        TFile *calfile = new TFile(rootfile,"RECREATE");
	//	if (verbose) cout << " Creating file " << rootfile << endl;
             
          // Create Tree //

	//        TTree *cal;

	Char_t treename[20];

	/*
        sprintf(treename,"calblock");
        calblock = (TTree *) enefile->Get(treename);
        if (!calblock) {
	  cout << "error reading tree " << calblock << " from file " << rootfile <<endl;}
        if (verbose)  cout << " Performing calibration over " << calblock->GetEntries() << " entries." <<  endl;

         calblock->SetBranchAddress("eventdata",&event);


	*/
   strncpy(filebase,filename,strlen(filename)-5);
        filebase[strlen(filename)-5]='\0';
        if (verbose) cout << " filebase = " << filebase << endl;

	  sprintf(treename,"mdata");
           TChain *block;
         block = (TChain *) rfile->Get(treename);

         if (!block) {
	   cout << " Problem reading Tree " << treename  << " from file " << filename << endl;
           cout << " Exiting " << endl;
           return -10;}
	 //	 entries=block->GetEntries();

         if (verbose) cout << " Number of entries to process :: " << block->GetEntries() << endl;

         cout << " Read block from file :: " << time(NULL)-starttime << endl;
         cout << " Read block from file :: " << time(NULL)-starttime << endl;
         TString calparfile;
        calparfile.Form("%s.par.root",filebase);
 
         TFile *calfile = new TFile(calparfile);

         PixelCal *CrysCal = new PixelCal("CrysCalPar");
         CrysCal = (PixelCal *) calfile->Get("CrysCalPar");
	   


         cout << " CrysCal->X[0][0][2][1][0] = " << CrysCal->X[0][0][2][1][0] << endl;
         cout << " CrysCal->GainSpat[0][0][2][1][0] = " << CrysCal->GainSpat[0][0][2][1][0] << endl;

         Sel_Calibrator *m_cal = new Sel_Calibrator();
         m_cal->SetFileBase(filebase);

       cout << "FYI:: Size of Sel_Calibrator :: " << sizeof(Sel_Calibrator) << endl;


       //       m_getEhis->SetFileBase(filebase);


       //    TFile *rfi;

       //  if (!fitonly){

#define USEPROOF

#ifdef USEPROOF      
       //  TProof *proof = new TProof("proof");
       //   proof->Open("");
             	 TProof *p = TProof::Open("workers=2");
       //	 	 TProof *p = TProof::Open("");
       //       gProof->UploadPackage("/home/miil/MODULE_ANA/ANA_V5/SpeedUp/PAR/ModuleDatDict.par");
       //       gProof->EnablePackage("ModuleDatDict");

         cout << " Proof open :: " << time(NULL)-starttime << endl;

	 #define USEPAR
     
#ifdef USEPAR
       /* This is an example of the method to use PAR files  -- will need to use an environment var here to make it location independent */

             p->UploadPackage("/home/miil/MODULE_ANA/ANA_V5/SpeedUp/PAR/Sel_Calibrator.par");
             p->EnablePackage("Sel_Calibrator");

#else
       /* Loading the shared library */
	     p->Exec("gSystem->Load(\"/home/miil/MODULE_ANA/ANA_V5/SpeedUp/lib/libModuleAna.so\")");
#endif
       m_cal->ReadUVCenters(rfile);
       p->AddInput(CrysCal);

       block->SetProof();
       /*
       TProofOutputFile outf = TProofOutputFile("mycalout.root");
       p->GetOutputList()->Add(outf);
       */
        cout << " Proof ready to process :: " << time(NULL)-starttime << endl;
        block->Process(m_cal);
        cout << " Proof processed :: " << time(NULL)-starttime << endl;

       m_cal->SetPixelCal(CrysCal);

#else
       //      block->Process("Sel_GetFloods.cc+");
       m_cal->ReadUVCenters(rfile);
       m_cal->SetPixelCal(CrysCal);
       cout << " Ready to process :: " << endl;
      block->Process(m_cal);
#endif

  

      m_cal->FitAllGlobal();
    /*
         PPeaks *thesePPeaks = (PPeaks *) rfile->Get("PhotoPeaks");
	 if (!(thesePPeaks)) {
	   cout << " Warning :: Couldn't read object PhotoPeaks from file " << rfile->GetName() << endl;
	   cout << " rfile->ls() :: "<< endl;
	   rfile->ls();
           exit(-1);
	 }

	 m_getEhis->SetPPeaks(thesePPeaks);
      */

      /*
        cout << " Before WriteHist :: " << time(NULL)-starttime << endl;
	rfi = new TFile("CrysPixs.root","RECREATE");
       m_getEhis->WriteHists(rfi);
        cout << " After WriteHist :: " << time(NULL)-starttime << endl;
      */

	//  }


	//       else { m_getEhis->LoadEHis(rfile); m_getEhis->SetPixelCal(CrysCal);}

    
       // m_getEhis->FitApdEhis(0,0,0,0);

      /*
       cout << " writing CrysCal :: " << time(NULL)-starttime << endl;
     CrysCal->Write(); 
      */


	 /*

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


	 */
	

	 // here's fitting the global E_spec

  


 Int_t hibin=0;
 Int_t max=0;
 Float_t xlow,xhigh;
 // Double_t params[6];
 Double_t eres,d_eres;
 TPaveText *labeltxt = new TPaveText(.12,.8,.5,.88,"NDC");
 labeltxt->SetFillColor(kWhite);

 c1->SetCanvasSize(700,700);

 //   calfile->cd();
 //   cal->Write(); 
 //   ppVals->Write();




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
     
  
     else {if (verbose) {
      cout << "No associated histogram found !" << endl;
      cout << " Entry :  x = " << x << " y = " << y <<endl; }
     }
    min=10000;
    histnr=9999;
    
  return -2;}



Float_t getpeak(TH1F *hist, Float_t xlow, Float_t xhigh, Bool_t force, Int_t verbose){
  Int_t npeaks=0;
  Int_t i=0,corpeak=0;
  TSpectrum *ss = new TSpectrum();
  Float_t y;
  //  Float_t corpeakpos;
  if (verbose) { cout << " Funtion getpeak. Looking for peak between " << xlow << " and " << xhigh << endl;}
  y=0;

	while((npeaks < 2) && (i < 9)) {
	if(verbose) {cout << "loop " << i << " " << npeaks << endl;}
	npeaks = ss->Search(hist, 3, "", 0.9 - (Float_t) i / 10);
        i++;
	if(i > 10) {if(verbose) {cout << " Warning " << npeaks << "found !" << endl;}
				    break;}
		} //while loop

	if(verbose) {
	   cout << npeaks << " peaks found " << i << " number of iterations" << endl;
		}

	if(npeaks != 1) {
	for(i = 0; i < npeaks; i++) {
	if(verbose) {cout << "x= " << *(ss->GetPositionX() + i) << " y = " << *(ss->GetPositionY() + i) << endl;}
	/* take largest peak with x position larger than lower fit limit */
       if( (y < *(ss->GetPositionY() + i) ) && (*(ss->GetPositionX() + i) > xlow) && (*(ss->GetPositionX() +i) < xhigh)) {
	corpeak = i;
        y = *(ss->GetPositionY() + i);
        if (verbose) cout << " Assuming peak " << corpeak << " at x=" << *(ss->GetPositionX() + corpeak) << endl;
			}
		} // for loop
        } 
        else corpeak=0; 
            
	if(verbose) {
			cout << "The correct peak out of " << npeaks << " peaks is peak number " << corpeak;
			cout << " at position: x = " << *(ss->GetPositionX() + corpeak) << endl;
		}
	//	corpeakpos=*(ss->GetPositionX() + corpeak);
	  

 // FIRST CHECK ::
	if(( (*(ss->GetPositionX() + corpeak)) < xlow) || ((*(ss->GetPositionX() + corpeak)) > xhigh))  {

      if(verbose) {cout << " Peak at " << *(ss->GetPositionX() + corpeak) << " is not within the window " << endl;}
        // move SIGMA rather than threshold

      if (!(force)) return NOVALIDPEAKFOUND;

      i=0; npeaks=0; 
      Int_t j;
      Int_t stop=0;
      /* I find that sigma = 3 is too small, limit it to 4 by demanding i<7 */ 
      while((i < 7)&&(!stop)) {
        	npeaks = ss->Search(hist, 10 - i, "nobackground", 0.4);

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

              
		if(((*(ss->GetPositionX() + corpeak)) < xlow) || ((*(ss->GetPositionX() + corpeak)) > xhigh ) ) {
		if(verbose) {cout << "Peak at " << *(ss->GetPositionX() + corpeak) << " is not within the window " << endl;} }
        else { if (verbose) {cout << "Valid peak found at : " << *(ss->GetPositionX()+corpeak) << endl;} stop=1; }
       } // while loop

  if (!(stop)){ 
	y = 0;

	/* note 9-6-13: changed sigma from 2 to 3,  need to clean up FIXME */
	npeaks = ss->Search(hist, 3, "", 0.4);
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
			if(verbose) {cout << "Peak at " << *(ss->GetPositionX() + corpeak) << " is STILL (II) not within the window " << endl;}
                       
         /* Lower "SIGMA" of tspectrum */
         npeaks = ss->Search(hist, 3, "", 0.3);
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
	cout << "Peak at " << *(ss->GetPositionX() + corpeak) << " is STILL (III) not within the window " << endl;
			}

     /* Lower "SIGMA, threshold" of tspectrum  FIXME -- mind this too  */
 		npeaks = ss->Search(hist, 3, "", 0.3);
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
				cout << "Peak at " << *(ss->GetPositionX() + corpeak) << " is STILL (IV) not within the window " << endl;
			}
                  

 // Giving up, taking the highest peak 
 
			if (verbose) cout << " Returning NOVALIDPEAKFOUND " << endl;

			return  NOVALIDPEAKFOUND;
                                              
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
		} // corpeak < xlow
          } // corpeak < xlow
	} //corpeak < xlow 

  } //  !stop
  } // corpeak < xlow
  //	} // 2nd check  -- npeaks != 1 ;
 // first check if the location of peak is okay.


	return *(ss->GetPositionX()+corpeak);} // else corresponding to  if (com)


Float_t getmean(Float_t *array){
  Double_t mean=0.;
  for (Int_t i=0;i<64;i++){
    mean+=array[i];
  }

  return mean/64.;}


