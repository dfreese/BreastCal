#define Sel_GetFloods_cxx
// The class definition in Sel_GetFloods.h has been generated automatically
// by the ROOT utility TTree::MakeSelector(). This class is derived
// from the ROOT class TSelector. For more information on the TSelector
// framework see $ROOTSYS/README/README.SELECTOR or the ROOT User Manual.

// The following methods are defined in this file:
//    Begin():        called every time a loop on the tree starts,
//                    a convenient place to create your histograms.
//    SlaveBegin():   called after Begin(), when on PROOF called only on the
//                    slave servers.
//    Process():      called for each event, in this function you decide what
//                    to read and fill your histograms.
//    SlaveTerminate: called at the end of the loop on the tree, when on PROOF
//                    called only on the slave servers.
//    Terminate():    called at the end of the loop on the tree,
//                    a convenient place to draw/fit your histograms.
//
// To use this file, try the following session on your Tree T:
//
// Root > T->Process("Sel_GetFloods.C")
// Root > T->Process("Sel_GetFloods.C","some options")
// Root > T->Process("Sel_GetFloods.C+")
//

#include "Sel_GetFloods.h"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <Riostream.h>
#include <TMath.h>
#include <sys/stat.h>
//#include "getfloods.h"
//#include "decoder.h" 
#define USEPROOF


ClassImp(Sel_GetFloods)


void Sel_GetFloods::Begin(TTree * /*tree*/)
{
   // The Begin() function is called at the start of the query.
   // When running with PROOF Begin() is only called on the client.
   // The tree argument is deprecated (on PROOF 0 is passed).

cout << " Welcome to Begin " << endl;

  TString option = GetOption();
  /*
     fPH = dynamic_cast<PPeaks *>(fInput->FindObject("PhotoPeaks")); 
    if (ph) cout << " Got PH @Begin ;-) " << endl;
    else cout << " TOO BASIC @Begin :-( " << endl; 
  */

}

void Sel_GetFloods::SlaveBegin(TTree * /*tree*/)
{
   // The SlaveBegin() function is called after the Begin() function.
   // When running with PROOF SlaveBegin() is called on each slave server.
   // The tree argument is deprecated (on PROOF 0 is passed).

  cout << " ----  Welcome to SlaveBegin with fOutPutFilename " << fOutputfilename << " ---- " << fPhotoPeaks[0][0][2][0] << endl;
  fPhotoPeaks[0][0][2][0]=1200;
   TString option = GetOption();

   //   fFloods = new TH2F*[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE];
  
   TString hn;

    fH2F =    new TH2F("fH2F","Dummy", 256, -1,1, 256, -1 ,1 );
    fOutput->Add(fH2F);

#ifdef USEPROOF
    fInput->ls();
    //  fInput->Print();
#endif

     fPH = dynamic_cast<PPeaks *>(fInput->FindObject("PhotoPeaks")); 
    
    //    if (fPH) cout << " Got PH ;-) " << endl;
    //   else cout << " TOO BASIC :-( " << endl; 
    //    cout <<"    -- " << fPH->spat[0][0][2][0] << " --" << endl; 
    

    //    fPhotoPeaks = dynamic_cast<Double_t >(fInput->FindObject() );


   for (Int_t c=0;c<CARTRIDGES_PER_PANEL;c++){
     for (Int_t f=0;f<FINS_PER_CARTRIDGE;f++){
       for (Int_t m=0;m<MODULES_PER_FIN;m++){
	 for (Int_t j=0;j<APDS_PER_MODULE;j++){
	   hn.Form("flood_C%dF%dM%dA%d",c,f,m,j);
	   fFloods[c][f][m][j] = new TH2F(hn.Data(),hn.Data(), 256, -1,1, 256, -1 ,1 );
           fOutput->Add(fFloods[c][f][m][j]);
	 }
       }
     } 
   }
}

Bool_t Sel_GetFloods::Process(Long64_t entry)
{
   // The Process() function is called for each entry in the tree (or possibly
   // keyed object in the case of PROOF) to be processed. The entry argument
   // specifies which entry in the currently loaded tree is to be processed.
   // It can be passed to either Sel_GetFloods::GetEntry() or TBranch::GetEntry()
   // to read either all or the required parts of the data. When processing
   // keyed objects with PROOF, the object is already loaded and is available
   // via the fObject pointer.
   //
   // This function should contain the "body" of the analysis. It can contain
   // simple or elaborate selection criteria, run algorithms on the data
   // of the event and typically fill histograms.
   //
   // The processing can be stopped by calling Abort().
   //
   // Use fStatus to set the return value of TTree::Process().
   //
   // The return value is currently not used.

  Double_t pp_low,pp_up;

    fChain->GetTree()->GetEntry(entry);
    /*   if (fPhotoPeaks[cartridge][fin][module][apd] > 0 ) {
      
           pp_low=.7*fPhotoPeaks[cartridge][fin][module][apd];
	   pp_up=1.3*fPhotoPeaks[cartridge][fin][module][apd];
	   */
  if (fPH->spat[cartridge][fin][module][apd] > 0 ){  
           pp_low=.7*fPH->spat[cartridge][fin][module][apd];
	   pp_up=1.3*fPH->spat[cartridge][fin][module][apd];
	   
           if ((E>pp_low)&&(E<pp_up)) {
           fFloods[cartridge][fin][module][apd]->Fill(x,y);
           if ((cartridge==0) && ( fin == 0 ) && (module == 2 )  && (apd == 0 ) ){// ( fin==7 ) ) { //&& ( apd==0 )  && ( module == 2 ) ) 
           fH2F->Fill(x,y);}
	   }
    }
    //    cout << " fin = " << fin << endl;

    fNumberOfEvents++;

   return kTRUE;
}

void Sel_GetFloods::SlaveTerminate()
{
   // The SlaveTerminate() function is called after all entries or objects
   // have been processed. When running with PROOF SlaveTerminate() is called
   // on each slave server.

 printf("Processed %lld number of events on Slave \n",fNumberOfEvents);

 

}

void Sel_GetFloods::Terminate()
{
   // The Terminate() function is the last function to be called during
   // a query. It always runs on the client, it can be used to present
   // the results graphically or save the results to file.
 
  
  //    TFile *d = new TFile(fOutputfilename,"UPDATE"); 

  cout << " Welcome to Terminate ! " << endl;
  // fOutput->ls();

  //   TCanvas *c2 = new TCanvas();

    // note this fH2F object is just for testing 
/*
    fH2F = dynamic_cast<TH2F *> (fOutput->FindObject("flood_C0F0M2A0")); 
    //    fH2F = dynamic_cast<TH2F *> (fOutput->FindObject("fH2F")); 
    if (fH2F) { cout << " fH2F Found ! " << endl; fH2F->Draw("colz");
       cout << " Entries :: " << fH2F->GetEntries() << endl;
       cout << " Name :: " << fH2F->GetName() << endl;
   }
    c2->Print("Sel_Test.png");
    delete c2;
*/
    TString hn;

#define _NRFLOODSTODRAW 4
		
       for (Int_t c=0;c<CARTRIDGES_PER_PANEL;c++){
	 for (Int_t f=0;f<FINS_PER_CARTRIDGE;f++){
           for (Int_t i=0;i<MODULES_PER_FIN;i++){
	     for (Int_t j=0;j<APDS_PER_MODULE;j++){
	       hn.Form("flood_C%dF%dM%dA%d",c,f,i,j);
               fFloods[c][f][i][j] = dynamic_cast<TH2F *> (fOutput->FindObject(hn.Data())); 
	     } //j 
	   } //i
	   //                c1->cd(i+1);

#ifndef __CINT__
	   Makeplot(fFloods,c,f,_NRFLOODSTODRAW,"floods");
#endif
	 } //f 
       } //c

   

  //  fH2F->Draw("colz");
    printf("Processed %lld  events\n",fNumberOfEvents);

}

#ifndef __CINT__
Int_t Sel_GetFloods::Makeplot(TH2F *hist[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE],Int_t c, Int_t f, Int_t NRFLOODSTODRAW,const Char_t suffix[40]) {

    TString fDIR="FLOODS";

// We're going to write to directory fDIR, need to make sure it exists:
    if ( access( fDIR.Data(), 0 ) != 0 ) {
	cout << "creating dir " << fDIR << endl; 
        if ( mkdir(fDIR, 0777) != 0 ) { 
	    cout << " Error making directory " << fDIR << endl;
	    return -2;
	}
    }


       TCanvas *c1;
       Char_t pngstring[FILENAMELENGTH];

        c1 = (TCanvas*)gROOT->GetListOfCanvases()->FindObject("c1");
        if (!c1) c1 = new TCanvas("c1","c1",10,10,1000,1000);
        c1->SetCanvasSize(1000,500);
      	 c1->Clear();

	     Int_t kk,i,j;
	   for (kk=0;kk<TMath::Floor(MODULES_PER_FIN/NRFLOODSTODRAW);kk++){
         c1->Clear();
         c1->Divide(NRFLOODSTODRAW,APDS_PER_MODULE);
	 for (i=kk*NRFLOODSTODRAW;i<NRFLOODSTODRAW*(kk+1);i++){ 
          for (j=0;j<APDS_PER_MODULE;j++){
            c1->cd((i%NRFLOODSTODRAW)+1+j*NRFLOODSTODRAW);
            hist[c][f][i][j]->Draw("colz"); } //j
	 } //i
	 sprintf(pngstring,"%s/%s.C%dF%dM%d-%d.%s.",fDIR.Data(),fFileBase.c_str(),c,f,kk*NRFLOODSTODRAW,i-1,suffix);
         strcat(pngstring,"png");
         c1->Print(pngstring);
       } // kk
	   return 0;	   }
#endif

Int_t Sel_GetFloods::WriteFloods(TFile *rfile,Int_t verbose=0){
  Char_t tmpstring[40];     



       for (Int_t c=0;c<CARTRIDGES_PER_PANEL;c++){
	 for (Int_t f=0;f<FINS_PER_CARTRIDGE;f++){
   	   sprintf(tmpstring,"C%dF%d",c,f);
	   rfile->cd(tmpstring);
           for (Int_t i=0;i<MODULES_PER_FIN;i++){
	     for (Int_t j=0;j<APDS_PER_MODULE;j++){
             fFloods[c][f][i][j]->Write();
	     // if ( verbose ) { cout << " ppeaks[" << c << "][" << f << "][" << i << "][" << j << "] = " << fPH->spat[c][f][i][j] ;}
	     } // j
	   } // i

       if (verbose) cout << endl;
       	} //loop over f (fins)
       } // loop over c

       return 0;}

