#include <stdlib.h>
#include "getfloods.h"

Int_t main(int argc, Char_t *argv[])
{
  cout << " Welcome to Getfloods. Program obtains Energy and flood histograms. " ;

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	Char_t		filename[FILENAMELENGTH] = "";
	Int_t		verbose = 0 ;


	Int_t		ix;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	ModuleDat *event = 0;
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
	gErrorIgnoreLevel = kError;
	  //	  gErrorIgnoreLevel = kError; //, kWarning, kError, kBreak, kSysError, kFatal;
		}
       

        TVector ppVals(8*RENACHIPS);
        TVector ppVals_com(8*RENACHIPS);
	TFile *rfile = new TFile(filename,"UPDATE");
        if (!rfile || rfile->IsZombie()) {  
         cout << "problems opening file " << filename << "\n.Exiting" << endl; 
         return -11;}
        TTree *block;
        TH1F *E[RENACHIPS][MODULES][2];
        TH1F *E_com[RENACHIPS][MODULES][2];
        TH2F *floods[RENACHIPS][MODULES][2];
        Char_t tmpstring[60],titlestring[60];
	//,cutstring[120],cutstring2[120];
        Char_t filebase[FILENAMELENGTH],pngstring[FILENAMELENGTH];
        Int_t i,j,l,entries;
        TCanvas *c1;
        Char_t treename[20];
 

	//        for (Int_t kk=0;kk<RENACHIPS;kk++){

	  //         cout << " Analyzing chip " << kk << endl;
	 sprintf(treename,"mdata");
         block = (TTree *) rfile->Get(treename);
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

	 block->SetBranchAddress("eventdata",&event);

	 /*
          block->SetBranchAddress("ct",&event.ct);
	  block->SetBranchAddress("chip",&event.chip);
	  block->SetBranchAddress("module",&event.module);
	  block->SetBranchAddress("apd",&event.apd);
	  block->SetBranchAddress("Ec",&event.Ec);
	  block->SetBranchAddress("Ech",&event.Ech);
	  block->SetBranchAddress("x",&event.x);
	  block->SetBranchAddress("y",&event.y);
	  block->SetBranchAddress("E",&event.E);
	  //	  block->SetBranchAddress("ft",&event.ft);
	  //	  block->SetBranchAddress("id",&event.id);
	  //	  block->SetBranchAddress("pos",&event.pos);
	  */

        strncpy(filebase,filename,strlen(filename)-5);
        filebase[strlen(filename)-5]='\0';
        if (verbose) cout << " filebase = " << filebase << endl;


        c1 = (TCanvas*)gROOT->GetListOfCanvases()->FindObject("c1");
        if (!c1) c1 = new TCanvas("c1","c1",10,10,1000,1000);
        c1->SetCanvasSize(700,700);
      	 c1->Clear();


      for (int kk=0;kk<RENACHIPS;kk++){
           if (verbose) cout << " Obtaining histograms RENA " << kk << endl;
          for (j=0;j<2;j++){
	 //
	 //	 c1->Divide(2,2); 
            for (i=0;i<4;i++){
	      sprintf(tmpstring,"RENA%d/E[%d][%d][%d]",kk,kk,i,j);
	     //    	     sprintf(titlestring,"E RENA %d, Module %d, PSAPD %d",kk,i,j);
             E[kk][i][j]= (TH1F *) rfile->Get(tmpstring); //new TH1F(tmpstring,titlestring,Ebins,E_low,E_up);
             sprintf(tmpstring,"RENA%d/E_com[%d][%d][%d]",kk,kk,i,j);
	     //    	     sprintf(titlestring,"ECOM RENA %d, Module %d, PSAPD %d",kk,i,j);
             E_com[kk][i][j]= (TH1F *) rfile->Get(tmpstring); //new TH1F(tmpstring,titlestring,Ebins_com,E_low_com,E_up_com);

	    //   sprintf(tmpstring,"UNIT%d.E>>E[%d][%d][%d]",i,kk,i,j);
	    //   sprintf(cutstring,"UNIT%d.com%dh<%d",i,j+1,threshold);
	    // if (verbose){
	    //  cout << "Drawstring = " << tmpstring << endl;
            //cout << "Condition  = " << cutstring << endl;}
	    //	    c1->cd(i+1);
	    //	    block->Draw(tmpstring,cutstring);
     	      }
	//	sprintf(pngstring,"%s.Emod%d",filebase,j);
	//       strcat(pngstring,".png");
	//	c1->Print(pngstring);
            } // j
	 }//kk



      //    cout << " E[3][0][1]->GetEntries() :: " <<       E[3][0][1]->GetEntries()  << endl;


       //       TSpectrum *sp = new TSpectrum();
       //       Int_t npeaks,efound;
       Double_t pp_right,pp_low,pp_up;
       Double_t ppeaks[RENACHIPS][MODULES][2];
       Double_t pp_right_com,pp_low_com,pp_up_com;
       Double_t ppeaks_com[RENACHIPS][MODULES][2];


       if (verbose) cout << " Filling Energy Histograms " << endl;
       /*
       for(l=0;l<block->GetEntries();l++){
	 block->GetEntry(l);
         if ( ((l%(entries/5))==0)&&(l>0)) {
           cout << l/1e6 << " Million Events processed ( = " <<  (double) 100*(l)/entries  << " %)"<<endl;}
         E[event->chip][event->module][event->apd]->Fill(event->E);  
         E_com[event->chip][event->module][event->apd]->Fill(-event->Ec);  
       } // l 

       */

       for (int kk=0;kk<RENACHIPS;kk++){
        for (j=0;j<2;j++){
          c1->Clear();
           for (i=0;i<4;i++){
   
            pp_low= PP_LOW_EDGE;
            pp_up= PP_UP_EDGE;
            pp_low_com= PP_LOW_EDGE_COM;
            pp_up_com= PP_UP_EDGE_COM;

  
	 if (verbose) { 
            cout << " ******************************************************* "  << endl; 
            cout << " * Determining Photopeak postion RENA " << kk << " MODULE " << i << " APD " << j << " *"<<endl;
            cout << " ******************************************************* "  << endl;  } 

	 if ( E[kk][i][j]->GetEntries() > MINHISTENTRIES ) {
           pp_right=GetPhotopeak_v1(E[kk][i][j],pp_low,pp_up,verbose);
           pp_low=0.7*pp_right;
           pp_up=1.3*pp_right;
         if (verbose) { 
           cout << "pp_low = " << pp_low ;
	  cout << " pp_up = " << pp_up << endl;
	  cout << " --------- Common ----------- " <<endl;} 
           pp_right_com=GetPhotopeak_v1(E_com[kk][i][j],pp_low_com,pp_up_com,verbose);
           pp_low_com=0.7*pp_right_com;
           pp_up_com=1.3*pp_right_com;
         if (verbose) { 
           cout << "pp_right_com = " << pp_right_com ;
           cout << " pp_low_com = " << pp_low_com ;
	   cout << " pp_up_com = " << pp_up_com << endl;} 


         } // > MINHISTENTRIES
         else {
	   if (verbose ){ cout << " Not enough entries in histogram E["<<kk<<"]["<<i<<"]["<<j<<"]. Skipping."<<endl;}
           pp_right=0;
           pp_right_com=0;
	 }
          ppeaks[kk][i][j]=pp_right;
          ppeaks_com[kk][i][j]=pp_right_com;
	   } //loop over i 
       } // loop over j

       for (j=0;j<2;j++){
         c1->Clear();
         c1->Divide(2,2);
	 for (i=0;i<4;i++){
           c1->cd(i+1);
	   E[kk][i][j]->Draw(); } //i
           sprintf(pngstring,"%s.RENA%d.Eapd%d",filebase,kk,j);
           strcat(pngstring,".png");
 	   c1->Print(pngstring);
       } // j

       for (j=0;j<2;j++){
         c1->Clear();
         c1->Divide(2,2);
	 for (i=0;i<4;i++){
           c1->cd(i+1);
	   E_com[kk][i][j]->Draw(); } //i
           sprintf(pngstring,"%s.RENA%d.Eapd_com%d",filebase,kk,j);
           strcat(pngstring,".png");
 	   c1->Print(pngstring);
       } // j


           
       for (j=0;j<2;j++){
        for (i=0;i<4;i++){
	  sprintf(tmpstring,"floods[%d][%d][%d]",kk,i,j);
	  sprintf(titlestring,"RENA %d, Module %d, PSAPD %d",kk, i,j);
          floods[kk][i][j]=new TH2F(tmpstring,titlestring,256,-1,1,256,-1,1);
	    //	    sprintf(tmpstring,"UNIT%d.x:UNIT%d.y>>floods[%d][%d][%d]",i,i,kk,i,j);
	    //            pp_low=0.7*ppeaks[kk][i][j];
	    //            pp_up=1.3*ppeaks[kk][i][j];	  
	  //            if (verbose){
	  //            cout << "low : " << 0.7*ppeaks[kk][i][j] << endl ;
	  //            cout << "up  : " << 1.3*ppeaks[kk][i][j] << endl;}
	    //	     sprintf(cutstring2,"UNIT%d.com%dh<%d&&UNIT%d.E>%.2f&&UNIT%d.E<%.2f",i,j+1,threshold,i,(Float_t) pp_low,i,(Float_t) pp_up);
	    //sprintf(cutstring2,"UNIT%d.com%dh<%d&&UNIT%d.E>100&&UNIT%d.E<2500",i,j+1,threshold,i,i);

	    //            if (verbose){
	    //	      cout << "Drawstring = " << tmpstring << endl;
	    //              cout << "Condition  = " << cutstring2 << endl;}
	    //	       c1->cd(i+1);
	    //	       block->Draw(tmpstring,cutstring2,"colz");
	    //	       cout << "Done ." <<endl;
	} // i
       } // j
       }// kk       

       if (verbose) cout << " Obtaining Flood histograms " << endl;
        for(l=0;l<block->GetEntries();l++){
         block->GetEntry(l);
	 if (ppeaks[event->chip][event->module][event->apd] > 0 ){
           pp_low=.7*ppeaks[event->chip][event->module][event->apd];
	   pp_up=1.3*ppeaks[event->chip][event->module][event->apd];
           if ((event->E>pp_low)&&(event->E<pp_up)) {
           floods[event->chip][event->module][event->apd]->Fill(event->y,event->x);
           }
         } // ppeaks[][][] > 0
	} // loop over entries (l) 

	for (int kk=0;kk<RENACHIPS;kk++){
          for (j=0;j<2;j++){
        	 c1->Clear();
	         c1->Divide(2,2); 
             for (i=0;i<4;i++){
                ppVals(kk*8+j*4+i) = ppeaks[kk][i][j];
                ppVals_com(kk*8+j*4+i) = ppeaks_com[kk][i][j];
                c1->cd(i+1);
                floods[kk][i][j]->Draw("colz");
	     } // loop i
           sprintf(pngstring,"%s.RENA%d.floodsapd%d",filebase,kk,j);
           strcat(pngstring,".png");
           if (verbose) cout << pngstring << endl;
           c1->Print(pngstring);
       }

       for (j=0;j<2;j++){
         for (i=0;i<4;i++){
           floods[kk][i][j]->Write();
           E[kk][i][j]->Write();
	   ppVals(kk*8+j*4+i) = ppeaks[kk][i][j];
	   ppVals_com(kk*8+j*4+i) = ppeaks_com[kk][i][j];
	 } // i
       } // j
       	} //loop over kk (chips)

       ppVals.Write("pp_spat");
       ppVals_com.Write("pp_com");
       block->Write();
       rfile->Close();
     

	return 0;}

Double_t  GetPhotopeak_v1(TH1F *hist,Double_t pp_low,Double_t pp_up,Int_t verbose){
   Double_t pp_right;
   Int_t kkk,l;
   TSpectrum *sp = new TSpectrum();
   Int_t npeaks,efound;
   


    kkk=0;
    efound=0;

    /* Initialize peak to lower bound */
    pp_right=pp_low;

     while (1){
        npeaks = sp->Search( hist,10,"",0.4-(float) kkk/10);
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

     return pp_right;
}
