#include <stdlib.h>
#include "getfloods.h"
#include "Sel_GetFloods.h"
#include "TTreePerfStats.h"
#include "TChain.h"
#include "TProof.h"
#include <sys/stat.h>

Int_t main(int argc, Char_t *argv[])
{
  cout << " Welcome to Getfloods. Program obtains Energy and flood histograms. " ;

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	Char_t		filename[FILENAMELENGTH] = "";
	Int_t		verbose = 0 ;


	Int_t		ix;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	//	ModuleDat *event = 0;
	//UNIT0,UNIT1,UNIT2,UNIT3;

	//     module TESTISD

	for(ix = 1; ix < argc; ix++) {

		/*
		 * Verbose '-v'
		 */
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
		}
	}

        cout << " Inputfile : " << filename << endl;

	//    rootlogon(verbose);
        set2dcolor(4);


	if (!verbose) {
   //  Setting "gErrorIgnoreLevel = kWarning;" results in  Warnings being shown, Info being shown 
   //  Setting "gErrorIgnoreLevel = kError;" results in  Warnings being shown, Info being shown 
   //  Setting "gErrorIgnoreLevel = kFatal;" results in  Warnings being shown, Info being shown 
   //  Setting "gErrorIgnoreLevel = 1001;" results in  Warnings being shown, Info being shown 
   //  Setting "gErrorIgnoreLevel = kPrint;" results in  Warnings being shown, Info being shown 
   //  Setting "gErrorIgnoreLevel = kInfo;" results in  Warnings being shown, Info being shown 
	gErrorIgnoreLevel = kFatal;
	  //	  gErrorIgnoreLevel = kError; //, kWarning, kError, kBreak, kSysError, kFatal;
		}
       

	//        TVector *ppVals[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE];
	//        TVector *ppVals_com[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE];
        TH1F *E[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE];
        TH1F *E_com[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE];
        TH2F *floods[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE];


        Int_t c,f,i,j;
	/*
        for (c=0;c<CARTRIDGES_PER_PANEL;c++){ 
	  for (f=0;f<FINS_PER_CARTRIDGE;f++){
            ppVals[c][f] = new TVector(APDS_PER_MODULE*MODULES_PER_FIN);
            ppVals_com[c][f] = new TVector(APDS_PER_MODULE*MODULES_PER_FIN);
	  }
	}
	*/
	TFile *rfile = new TFile(filename,"UPDATE");
        if (!rfile || rfile->IsZombie()) {  
         cout << "problems opening file " << filename << "\n.Exiting" << endl; 
         return -11;}


        Char_t tmpstring[60],titlestring[60];
	//,cutstring[120],cutstring2[120];
        Char_t filebase[FILENAMELENGTH];
	//        Int_t l
        Int_t entries;
        TCanvas *c1;
        Char_t treename[20];
 

	//        for (Int_t kk=0;kk<RENACHIPS;kk++){

	  //         cout << " Analyzing chip " << kk << endl;

	//         TTree *block;
	 sprintf(treename,"mdata");         
	 //         block = (TTree *)  rfile->Get(treename);
	 //	 Perf ana code
         TChain *block;
         block = (TChain *) rfile->Get(treename);
	 //         blockchain->Merge("mergedchain.root");
	 //         TFile *mc = new TFile("mergedchain.root");
	 //       block = (TTree *)  mc->Get("mdata");

	 //         block->SetCacheSize(300000000);
	 //         block->AddBranchToCache("*");

         TTreePerfStats *ps = new TTreePerfStats("ioperf",block);
         if (!block) {
	   cout << " Problem reading Tree " << treename  << " from file " << filename << endl;
           cout << " Exiting " << endl;
           return -10;}
	 entries=block->GetEntries();
      
	 /*
         if ( entries < MINENTRIES) {
	   cout << " not enough entries for RENA chip " << kk << endl;
           cout << " We only got " << entries << " entries " << endl;
           continue; }
	 */
         if (verbose)  cout << " Ok, we got " << entries << " entries. " << endl;

	 //	 block->SetBranchAddress("eventdata",&event);


        strncpy(filebase,filename,strlen(filename)-5);
        filebase[strlen(filename)-5]='\0';
        if (verbose) cout << " filebase = " << filebase << endl;


        c1 = (TCanvas*)gROOT->GetListOfCanvases()->FindObject("c1");
        if (!c1) c1 = new TCanvas("c1","c1",10,10,1000,1000);
        c1->SetCanvasSize(700,700);
      	 c1->Clear();


	 for (c=0;c<CARTRIDGES_PER_PANEL;c++){
	   for (f=0;f<FINS_PER_CARTRIDGE;f++){
           if (verbose) cout << " Obtaining histograms FIN " << f << endl;
            for (i=0;i<MODULES_PER_FIN;i++){
	      for (j=0;j<APDS_PER_MODULE;j++){
		sprintf(tmpstring,"C%dF%d/E[%d][%d][%d][%d]",c,f,c,f,i,j);
                E[c][f][i][j]= (TH1F *) rfile->Get(tmpstring); //new TH1F(tmpstring,titlestring,Ebins,E_low,E_up);
 	        sprintf(tmpstring,"C%dF%d/E_com[%d][%d][%d][%d]",c,f,c,f,i,j);
                E_com[c][f][i][j]= (TH1F *) rfile->Get(tmpstring); //new TH1F(tmpstring,titlestring,Ebins_com,E_low_com,E_up_com);

     	      } // i
            } // j
	 }//f
	 } //c



       Double_t pp_right,pp_low,pp_up;
       Double_t ppeaks[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE];
       //       Double_t ppeaks_com[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE];

       PPeaks *ph = new PPeaks("PhotoPeaks");

       Double_t pp_right_com,pp_low_com,pp_up_com;

      
       if (verbose) cout << " Determining photopeak position " << endl;

       for (c=0;c<CARTRIDGES_PER_PANEL;c++){
      	 for (f=0;f<FINS_PER_CARTRIDGE;f++){
           for (i=0;i<MODULES_PER_FIN;i++){
	     for (j=0;j<APDS_PER_MODULE;j++){
	       pp_low= PP_LOW_EDGE;
	       pp_up= PP_UP_EDGE;
	       pp_low_com= PP_LOW_EDGE_COM;
	       pp_up_com= PP_UP_EDGE_COM;

  
	 if (verbose) { 
            cout << " ******************************************************* "  << endl; 
            cout << " * Determining Photopeak postion CARTRIDGE " << c << " FIN " << f << " MODULE " << i << " APD " << j << " *"<<endl;
            cout << " ******************************************************* "  << endl;  } 

	 if ( E[c][f][i][j]->GetEntries() > MINHISTENTRIES ) {
           pp_right=GetPhotopeak_v1(E[c][f][i][j],pp_low,pp_up,verbose,12);
	   if (verbose) cout << " pp_right = " << pp_right << endl; 
           pp_low=0.7*pp_right;
           pp_up=1.3*pp_right;
         if (verbose) { 
           cout << ", pp_low = " << pp_low ;
       	   cout << " pp_up = " << pp_up << endl;
	   cout << " --------- Common ----------- " <<endl;} 
	 pp_right_com=GetPhotopeak_v1(E_com[c][f][i][j],pp_low_com,pp_up_com,verbose,12);
           pp_low_com=0.7*pp_right_com;
           pp_up_com=1.3*pp_right_com;
         if (verbose) { 
           cout << "pp_right_com = " << pp_right_com ;
           cout << " pp_low_com = " << pp_low_com ;
	   cout << " pp_up_com = " << pp_up_com << endl;} 


         } // > MINHISTENTRIES
         else {
	   if (verbose ){ cout << " Not enough entries in histogram E["<<c<<"]["<<f<<"]["<<i<<"]["<<j<<"]. Skipping."<<endl;}
           pp_right=0;
           pp_right_com=0;
	 }
	   ph->spat[c][f][i][j]=pp_right;
	   //           ppeaks[c][f][i][j]=pp_right;
	   //           ppeaks_com[c][f][i][j]=pp_right_com;
           ph->com[c][f][i][j]=pp_right_com;
	   if ( verbose ) { cout << " ppeaks[" << c << "][" << f << "][" << i << "][" << j << "] = " << ph->spat[c][f][i][j] << endl;}

	   sprintf(tmpstring,"floods[%d][%d][%d][%d]",c,f,i,j);
	   sprintf(titlestring,"C%dF%d, Module %d, PSAPD %d",c,f, i,j);
           floods[c][f][i][j]=new TH2F(tmpstring,titlestring,256,-1,1,256,-1,1);
	   } //loop over j 
	   } // loop over i

#define _NRFLOODSTODRAW 4

	   makeplot(E,c,f,_NRFLOODSTODRAW,"E",filebase);
	   makeplot(E_com,c,f,_NRFLOODSTODRAW,"Ecom",filebase);
                    
	 } // f
       }// c       

       Sel_GetFloods *m_getfloods = new Sel_GetFloods();
       //   TSelector *m_getfloods = new TSelector();



       //         PPeaks *thesepeaks = new PPeaks("thesepeaks");
	 //    thesepeaks->SetName("Photopeaks");


       //       m_getfloods->SetOutputFileName(filename);
       m_getfloods->SetOutputFileName("Testing.root");
       m_getfloods->SetFileBase(filebase);
       //       m_getfloods->SetPhotoPeaks(ppeaks);
       m_getfloods->SetPhotoPeaks(ph->spat);

        #define USEPROOF

#ifdef USEPROOF      
       //  TProof *proof = new TProof("proof");
       //   proof->Open("");
       TProof *p = TProof::Open("");


             char *libpath = getenv("ANADIR");
	     cout << " Loading Shared Library from " << libpath << endl;
	     cout << " (note ANADIR = " << getenv("ANADIR") << " )" << endl;
	     TString exestring;

     
#ifdef USEPAR

       // This is an example of the method to use PAR files  -- will need to use an environment var here to make it location independent */

	     //	     exestring.Form("gSystem->Load(\"%slibModuleAna.so\")","/home/miil/MODULE_ANA/ANA_V5/SpeedUp/lib/");
	     exestring.Form("%s/PAR/Sel_GetFloods.par",libpath);
             
             p->UploadPackage(exestring.Data());
             p->EnablePackage("Sel_GetFloods");

#else
       /* Loading the shared library */
             exestring.Form("gSystem->Load(\"%s/lib/libModuleAna.so\")",libpath);
	     p->Exec(exestring.Data());

	     //   return -1;

#endif

       /*     
#ifdef USEPAR
       // This is an example of the method to use PAR files  --  need to use an environment var here to make it location independent 
       p->UploadPackage("/home/miil/MODULE_ANA/ANA_V5/SpeedUp/PAR/Sel_GetFloods.par");
       p->EnablePackage("Sel_GetFloods");
#else
       // Loading the shared library 
	     p->Exec("gSystem->Load(\"/home/miil/MODULE_ANA/ANA_V5/SpeedUp/lib/libModuleAna.so\")")
#endif
       */

        p->AddInput(ph);
	// This didn't work ::            gProof->Load("m_getfloods");
           
       block->SetProof();
       block->Process(m_getfloods);
#else
       //      block->Process("Sel_GetFloods.cc+");
      block->Process(m_getfloods);
#endif
 
	      //              block->Process("Sel_GetFloods.cc");

	      // 


       cout << " Proof output " << endl;
       //     gProof->GetOutputList()->Print();

       /*
       if (verbose) cout << " Obtaining Flood histograms " << endl;
        for(l=0;l<block->GetEntries();l++){
         block->GetEntry(l);
	 if (ppeaks[event->cartridge][event->fin][event->module][event->apd] > 0 ){
           pp_low=.7*ppeaks[event->cartridge][event->fin][event->module][event->apd];
	   pp_up=1.3*ppeaks[event->cartridge][event->fin][event->module][event->apd];
           if ((event->E>pp_low)&&(event->E<pp_up)) {
           floods[event->cartridge][event->fin][event->module][event->apd]->Fill(event->x,event->y);
           }
         } // ppeaks[][][][] > 0
	} // loop over entries (l) 
       */
	if (verbose) cout << " Done Looping. "  << endl;

	//         ps->SaveAs("perfstat.root");


	if (verbose) cout << " Setting ppVals " << endl;



	/*	
       for (c=0;c<CARTRIDGES_PER_PANEL;c++){
	 for (f=0;f<FINS_PER_CARTRIDGE;f++){
           for (i=0;i<MODULES_PER_FIN;i++){
	     for (j=0;j<APDS_PER_MODULE;j++){
	       // FIXME 

	       (*ppVals[c][f])[i*APDS_PER_MODULE+j] = ppeaks[c][f][i][j];
	       (*ppVals_com[c][f])[i*APDS_PER_MODULE+j] = ppeaks_com[c][f][i][j];
	     } //j 
	   } //i
                c1->cd(i+1);
       		makeplot(floods,c,f,_NRFLOODSTODRAW,"floods",filebase);
	 } //f 
       } //c
	*/
     

	/*
       
       for (c=0;c<CARTRIDGES_PER_PANEL;c++){
	 for (f=0;f<FINS_PER_CARTRIDGE;f++){
	   	   sprintf(tmpstring,"C%dF%d",c,f);
	   //   subdir[c][f] = rfile->mkdir(tmpstring);
	   //subdir[c][f]->cd();
		   rfile->cd(tmpstring);
           for (i=0;i<MODULES_PER_FIN;i++){
	     for (j=0;j<APDS_PER_MODULE;j++){
           
	       //    floods[c][f][i][j]->Write();
	       //	       E[c][f][i][j]->Write();
	       if ( verbose ) { cout << " ppeaks[" << c << "][" << f << "][" << i << "][" << j << "] = " << ppeaks[c][f][i][j] ;}
	     } // j
	   } // i

	       sprintf(tmpstring,"pp_spat[%d][%d]",c,f);
	       ppVals[c][f]->Write(tmpstring);
	       sprintf(tmpstring,"pp_com[%d][%d]",c,f);
	       ppVals_com[c][f]->Write(tmpstring);


       if (verbose) cout << endl;
       	} //loop over f (fins)
       } // loop over c
*/

        rfile->cd(); 
        m_getfloods->WriteFloods(rfile,verbose);
        rfile->cd();
	ph->Write();

        block->Write();
        rfile->Close();
    

	return 0;}

Double_t  GetPhotopeak_v1(TH1F *hist,Double_t pp_low,Double_t pp_up,Int_t verbose, Int_t width){
   Double_t pp_right;
   Int_t kkk,l;
   TSpectrum *sp = new TSpectrum();
   Int_t npeaks,efound;
   
   if ( verbose ) { cout << " ====== Welcome to GetPhotoPeak_v1, width =  " << width << " ====== " << endl; }

    kkk=0;
    efound=0;

    /* Initialize peak to lower bound */
    pp_right=pp_low;

     while (1){
       // changing this from 10 to 3  080513
         npeaks = sp->Search( hist,width,"",0.6-(float) kkk/10);
      if (verbose ) {  
         cout << npeaks << " peaks found in the Spatial energy spectrum " << endl; }
      // get the peak between E_low and E_up ;
      for (l=0;l<npeaks;l++){
         // look for the peak with the highest x value
        if (verbose){
        cout << " l = " << l <<" " <<*(sp->GetPositionX()+l) << "  " << *(sp->GetPositionY()+l) << endl;}
    if (  ( *(sp->GetPositionX()+l) > pp_low ) &&  
          ( *(sp->GetPositionX()+l) < pp_up ) && 
          ( *(sp->GetPositionX()+l) > pp_right) ){
          if (verbose) cout << " Found correct peak @ " << *(sp->GetPositionX()+l) << endl;
          pp_right = (*(sp->GetPositionX()+l));
          efound=1;
          if (verbose) { cout << "pp_right = " << pp_right ; }
              //       i=npeaks+2;       
       }
   } // loop over npeaks;
   //    cout << " i = " << i << endl;
   //   if ((npeaks <3 ) && (j<4)){  j++;}
   if ((!efound ) && (kkk<5)){  kkk++;}
   else break;
     }

     if (verbose) { cout << " while loop done. pp_right = " << pp_right << ". Efound = " << efound << endl; } 

     if ((!efound)&&(width>3)){
       width-=2;
       if (verbose) { 
       cout << " Changing width of photopeak window to :: " ;
       cout << width  << endl;}       
       pp_right=GetPhotopeak_v1(hist,pp_low,pp_up,verbose,width);
       // try different width
         }

     if (verbose) { cout << " retURning pp_right = " << pp_right << endl;}

     return pp_right;
}

Double_t  GetPhotopeak_v2(TH1F *hist,Double_t pp_low,Double_t pp_up,Int_t verbose){
   Double_t pp_right;
   Int_t kkk,l;
   TSpectrum *sp = new TSpectrum();
   Int_t npeaks,efound;


    kkk=0;
    efound=0;

    /* Initialize peak to lower bound */
    pp_right=pp_low;

     while (1){
        npeaks = sp->Search( hist,8,"",0.99-(float) kkk/10);
      if (verbose ) {  
         cout << npeaks << " peaks found in the Spatial energy spectrum " << endl; }
      // get the peak between E_low and E_up ;
      for (l=0;l<npeaks;l++){
         // look for the peak with the highest x value
        if (verbose){
        cout << " l = " << l <<" " <<*(sp->GetPositionX()+l) << "  " << *(sp->GetPositionY()+l) << endl;}
    if (  ( *(sp->GetPositionX()+l) > pp_low ) &&  
          ( *(sp->GetPositionX()+l) < pp_up ) && 
          ( *(sp->GetPositionX()+l) > pp_right) ){
          if (verbose) cout << " Found correct peak @ " << *(sp->GetPositionX()+l) << endl;
          pp_right = (*(sp->GetPositionX()+l));
          efound=1;
          if (verbose) { cout << "pp_right = " << pp_right ; }
              //       i=npeaks+2;       
       }
   } // loop over npeaks;
   //    cout << " i = " << i << endl;
   //   if ((npeaks <3 ) && (j<4)){  j++;}
   if ((!efound ) && (kkk<4)){  kkk++;}
   else break;
     }

 if (verbose) { cout << " before return pp_right = " << pp_right ; }

     return pp_right;
}


Int_t	   makeplot(TH1F *hist[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE],Int_t c, Int_t f, Int_t NRFLOODSTODRAW,const Char_t suffix[40],Char_t filebase[FILENAMELENGTH]) {

        TCanvas *c1;
  Char_t pngstring[FILENAMELENGTH];

  TString fDIR;

  if (strncmp(suffix,"Ecom",4)==0){
    fDIR.Form("EHIST_COM");}
  else fDIR.Form("EHIST_SPAT");

// We're going to write to directory fDIR, need to make sure it exists:
    if ( access( fDIR.Data(), 0 ) != 0 ) {
	cout << " Creating dir " << fDIR  << endl; 
        if ( mkdir(fDIR, 0777) != 0 ) { 
	    cout << " Error making directory " << fDIR << endl;
	    return -2;
	}
    }
        c1 = (TCanvas*)gROOT->GetListOfCanvases()->FindObject("c1");
        if (!c1) c1 = new TCanvas("c1","c1",10,10,1000,1000);
        c1->SetCanvasSize(700,700);
      	 c1->Clear();

	     Int_t kk,i,j;
	   for (kk=0;kk<TMath::Floor(MODULES_PER_FIN/NRFLOODSTODRAW);kk++){
         c1->Clear();
         c1->Divide(NRFLOODSTODRAW,APDS_PER_MODULE);
	 for (i=kk*NRFLOODSTODRAW;i<NRFLOODSTODRAW*(kk+1);i++){ 
          for (j=0;j<APDS_PER_MODULE;j++){
            c1->cd((i%NRFLOODSTODRAW)+1+j*NRFLOODSTODRAW);
            hist[c][f][i][j]->Draw(); } //j
	 } //i
	 sprintf(pngstring,"%s/%s.C%dF%dM%d-%d.%s.",fDIR.Data(),filebase,c,f,kk*NRFLOODSTODRAW,i-1,suffix);
         strcat(pngstring,"png");
         c1->Print(pngstring);
	   } // kk
	   return 0;}

