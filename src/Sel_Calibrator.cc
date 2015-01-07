#define Sel_Calibrator_cxx
// The class definition in Sel_Calibrator.h has been generated automatically
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
// Root > T->Process("Sel_Calibrator.C")
// Root > T->Process("Sel_Calibrator.C","some options")
// Root > T->Process("Sel_Calibrator.C+")
//

#include "Sel_Calibrator.h"
#include <TH2.h>
#include <TStyle.h>
#include <TSystem.h>
//#include "/home/miil/MODULE_ANA/ANA_V5/SpeedUp/include/Apd_Fit.h"
#include "Apd_Fit.h"
#include <TPaveText.h>
//#include "/home/miil/root/macros/include/myrootlib.h"
#include "myrootlib.h"
#include <sys/stat.h>

ClassImp(Sel_Calibrator)

void Sel_Calibrator::Begin(TTree * /*tree*/)
{
   // The Begin() function is called at the start of the query.
   // When running with PROOF Begin() is only called on the client.
   // The tree argument is deprecated (on PROOF 0 is passed).

#ifdef DEBUG
 cout << " Welcome to Begin Sel_Calibrator " << endl;
#endif 

  TString option = GetOption();
 starttime = time(NULL);
}

void Sel_Calibrator::SlaveBegin(TTree * /*tree*/)
{
   // The SlaveBegin() function is called after the Begin() function.
   // When running with PROOF SlaveBegin() is called on each slave server.
   // The tree argument is deprecated (on PROOF 0 is passed).

#ifdef DEBUG
  cout << " Welcome to SlaveBegin Sel_Calibrator " << endl;
#endif

  TString hn;
  TString ht;
  TString option = GetOption();

  

  startloop = time (NULL);

//#define USEPROOF

//#ifdef USEPROOF 
  if (fUseProof) {
fInput->ls();
fCrysCal = dynamic_cast<PixelCal *>(fInput->FindObject("CrysCalPar"));
  }
//#endif

  endloop = time (NULL);
  if (!fQuiet){
  cout << " Time to find fCrysCal :: " <<  endloop-startloop  << endl;
  }

#ifdef DEBUG
  cout << " fCrysCal->ValidPeaksX[0][0][2][1] = " << fCrysCal->validpeaks[0][0][2][1] << endl;
  cout << " fCrysCal->X[0][0][2][1][0] = " << fCrysCal->X[0][0][2][1][0] << endl;
  cout << " fCrysCal->GainSpat[0][0][2][1][0] = " << fCrysCal->GainSpat[0][0][2][1][0] << endl;
  cout << " *uu_c[0][0])[2*2+1]  = " << (*uu_c[0][0])[2*2+1]  << endl;
#endif
  
  startloop = time (NULL);
  

    TString fCalFilename;   



#ifdef DEBUG  
   cout << " fFileBase " << endl;
#endif 

   fCalFilename = fFileBase + ".cal.root" ;

  // see: http://root.cern.ch/drupal/content/handling-large-outputs-root-files
  // TNamed *nm = dynamic_cast<TNamed *> (fInput->FindObject(fCalFilename.Data()));



//#ifdef USEPROOF
   if (fUseProof){
   //  TNamed *nm = dynamic_cast<TNamed *> (fInput->FindObject("damnit.root"));
   TNamed *nm = dynamic_cast<TNamed *> (fInput->FindObject(fCalFilename.Data()));

  if (nm) {
      // Just create the object
      UInt_t opt = TProofOutputFile::kRegister | TProofOutputFile::kOverwrite | TProofOutputFile::kVerify;
      fProofFile = new TProofOutputFile(fCalFilename.Data(),TProofOutputFile::kDataset, opt, nm->GetTitle());
      //  fProofFile = new TProofOutputFile("damnit.root",TProofOutputFile::kDataset, opt, nm->GetTitle());
   } else {
      // For the ntuple, we use the automatic file merging facility
      // Check if an output URL has been given
      TNamed *out = (TNamed *) fInput->FindObject("PROOF_OUTPUTFILE_LOCATION");
      Info("SlaveBegin", "PROOF_OUTPUTFILE_LOCATION: %s", (out ? out->GetTitle() : "undef"));
     fProofFile = new TProofOutputFile(fCalFilename.Data(), (out ? out->GetTitle() : "M"));
     // fProofFile = new TProofOutputFile("damnit.root", (out ? out->GetTitle() : "M"));
      out = (TNamed *) fInput->FindObject("PROOF_OUTPUTFILE");
      if (out) fProofFile->SetOutputFileName(out->GetTitle());
   }


  
 // Open the file
   TDirectory *savedir = gDirectory;

 if (!(fFile = fProofFile->OpenFile("RECREATE"))) {
   Warning("SlaveBegin", "problems opening file: %s/%s",
	   fProofFile->GetDir(), fProofFile->GetFileName());
 }

// #else 
   } else {
 fFile = new TFile(fCalFilename.Data(),"RECREATE");
   }
//#endif
  
    fCalTree = new TTree("CalTree","Energy calibrated LYSO-PSAPD module data ");
    
    fCalTree->SetDirectory(fFile);
    fCalTree->AutoSave();
    fCalEvent   =  new ModuleCal();
    fCalTree->Branch("Calibrated Event Data",&fCalEvent);

if (fUseProof){
//#ifdef USEPROOF
   fOutput->Add(fCalTree);
//#endif   
       }
   
   for (Int_t cc=0;cc<CARTRIDGES_PER_PANEL;cc++){
     for (Int_t f=0;f<FINS_PER_CARTRIDGE;f++){
       for (Int_t m=0;m<MODULES_PER_FIN;m++){
         for (Int_t j=0;j<APDS_PER_MODULE;j++){
	     hn.Form("GlobEhist_C%dF%dM%dA%d",cc,f,m,j);
             ht.Form("C%dF%d Module %d APD %d",cc,f,m,j);
	     fGlobHist[cc][f][m][j] = new TH1F(hn.Data(),ht.Data(), Ebins_pixel,E_low,E_up);
             hn.Form("GlobEhist_com_C%dF%dM%dA%d",cc,f,m,j);
             ht.Form("C%dF%d Module %d APD %d Common",cc,f,m,j);
             fGlobHist_com[cc][f][m][j] = new TH1F(hn.Data(),ht.Data(), Ebins_com_pixel,E_low_com,E_up_com);
	     fOutput->Add(fGlobHist[cc][f][m][j]);
	     fOutput->Add(fGlobHist_com[cc][f][m][j]);
         }
       }
     } 
   }
   
    
   if (!fQuiet){
  cout << " ... SlaveBegin Done " << endl;
   }
   }



Bool_t Sel_Calibrator::Process(Long64_t entry)
{
   // The Process() function is called for each entry in the tree (or possibly
   // keyed object in the case of PROOF) to be processed. The entry argument
   // specifies which entry in the currently loaded tree is to be processed.
   // It can be passed to either Sel_Calibrator::GetEntry() or TBranch::GetEntry()
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



  // also need time stamp calc.

  // need to calc average gain for each PSAPD -- note :: perhaps should do a weighting or so, also probably better to do it in Sel_GetEhis.cc 
  //   calevent->E=event->E* Emeans[event->chip*8+event->apd*4+event->module]/CRYSTALPP[event->chip][event->module][event->apd][event->id];

   
   fChain->GetTree()->GetEntry(entry);

  //   if (entry&&((entry%1000000)==0)) cout << entry << " Events processed " << endl;

   
    fCalEvent->ct=ct;
    fCalEvent->cartridge=cartridge;
    fCalEvent->fin=fin;
    fCalEvent->chip=chip;
    fCalEvent->m=module; 
    fCalEvent->apd=apd;

    fCalEvent->E=E;
    fCalEvent->Ecal=0;
    fCalEvent->x=x;
    fCalEvent->y=y;

    fCalEvent->id=-1;        
    fCalEvent->pos=pos;

    if (fCrysCal->validpeaks[cartridge][fin][module][apd]) {
      Int_t _id=fCrysCal->GetCrystalId(x,y,cartridge,fin,module,apd);
      fCalEvent->Ecal= E* 511/fCrysCal->GainSpat[cartridge][fin][module][apd][_id];
      fCalEvent->E= E* Getmean(fCrysCal->GainSpat[cartridge][fin][module][apd])/fCrysCal->GainSpat[cartridge][fin][module][apd][_id];
      fCalEvent->Ec= -Ec* 511/fCrysCal->GainCom[cartridge][fin][module][apd][_id];
      fGlobHist[cartridge][fin][module][apd]->Fill(fCalEvent->E); 
      fGlobHist_com[cartridge][fin][module][apd]->Fill(fCalEvent->Ec); 
      fCalEvent->id=_id;
      
       fCalEvent->ft = FineCalc(ft,  (*uu_c[cartridge][fin])[module*2+apd], (*vv_c[cartridge][fin])[module*2+apd]);
    }

        fCalTree->Fill();

   return kTRUE;
}

void Sel_Calibrator::SlaveTerminate()
{
   // The SlaveTerminate() function is called after all entries or objects
   // have been processed. When running with PROOF SlaveTerminate() is called
   // on each slave server.

  if (!fQuiet) { cout << " Welcome to SlaveTerminate ! " << endl;}



   // Write the ntuple to the file
   if (fFile) {
      Bool_t cleanup = kFALSE;
//#ifdef USEPROOF
      TDirectory *savedir;
      if (fUseProof) {
      savedir = gDirectory;
      }
//#endif
      if (fCalTree->GetEntries() > 0) {
         fFile->cd();
         fCalTree->Write();
	 if (fUseProof) {
//#ifdef USEPROOF
	     fProofFile->Print();
	     fOutput->Add(fProofFile);
	 }
//#endif
	 } else {
         cleanup = kTRUE;
      }
      fCalTree->SetDirectory(0);
    if (fUseProof) {
//#ifdef USEPROOF
      gDirectory = savedir;
    }
//#endif
      fFile->Close();
      // Cleanup, if needed
      if (cleanup) {
         TUrl uf(*(fFile->GetEndpointUrl()));
         SafeDelete(fFile);
         gSystem->Unlink(uf.GetFile());
         SafeDelete(fProofFile);
      }
   }



}

void Sel_Calibrator::Terminate()
{
   // The Terminate() function is the last function to be called during
   // a query. It always runs on the client, it can be used to present
   // the results graphically or save the results to file.
  if (!fQuiet){
  cout << " Welcome to Terminate ! " << endl;
  cout << " time since start :: " << time(NULL)-starttime << endl;
  }

  TString hn;
  //      fOutput->ls();

  //  TFile *_f = new TFile("caltest.root","RECREATE");

  if (!fQuiet){
      cout << " \n Getting CalTree \n" << endl;
  }
      //  fCalTree = dynamic_cast<TTree *> (fOutput->FindObject("CalTree"));


      //  cout << " Entries in fCalTree :: " << fCalTree->GetEntries() << endl;

  
  

    for (Int_t cc=0;cc<CARTRIDGES_PER_PANEL;cc++){
	 for (Int_t f=0;f<FINS_PER_CARTRIDGE;f++){
           for (Int_t i=0;i<MODULES_PER_FIN;i++){
	     for (Int_t j=0;j<APDS_PER_MODULE;j++){
	       hn.Form("GlobEhist_C%dF%dM%dA%d",cc,f,i,j);
	       fGlobHist[cc][f][i][j] = dynamic_cast<TH1F *> (fOutput->FindObject(hn.Data()));
	       hn.Form("GlobEhist_com_C%dF%dM%dA%d",cc,f,i,j);
	        fGlobHist_com[cc][f][i][j] = dynamic_cast<TH1F *> (fOutput->FindObject(hn.Data()));

	     } //j
	   } //i
	 } //f 
       } //cc

 
  
    if (!fQuiet){
    cout << " Terminate done time since start :: " << endl; 
    }
   
// << time(NULL)-starttime << endl;
    //    fCalTree->SetDirectory(_f);
    //  fCalTree->Write();

    //  _f->Close();
}

Int_t Sel_Calibrator::ReadCal(TFile *r) {
  fCrysCal = (PixelCal *) r->Get("CrysCalPar");


  return 0;}


Double_t Sel_Calibrator::FineCalc(Double_t uv, Float_t u_cent, Float_t v_cent){
  Double_t tmp;
  Int_t UV = (Int_t) uv;
  Int_t u = (( UV & 0xFFFF0000 ) >> 16 );
  Int_t v = ( UV & 0xFFFF );
  // cout << " finecalc : u = " << u << " v = " << v  << " ( center :: " << u_cent << ","<< v_cent << ")";
  tmp=TMath::ATan2(u-u_cent,v-v_cent);
  //  cout << " tmp = " << tmp << endl;
  if (tmp < 0. ) { tmp+=2*3.141592;}
  return tmp;///(2*3.141592*CIRCLEFREQUENCY);
}




Int_t Sel_Calibrator::ReadUVCenters(TFile *rfile){

  //fixme - need return statement if vectors can't be read.
  Char_t tmpstring[40];  
  for (Int_t cc=0;cc<CARTRIDGES_PER_PANEL;cc++){
    for (Int_t f=0;f<FINS_PER_CARTRIDGE;f++){
      sprintf(tmpstring,"timing/uu_c[%d][%d]",cc,f);
      uu_c[cc][f]= (TVector *) rfile->Get(tmpstring); //new TH1F(tmpstring,titlestring,Ebins,E_low,E_up);
      sprintf(tmpstring,"timing/vv_c[%d][%d]",cc,f);
      vv_c[cc][f]= (TVector *) rfile->Get(tmpstring); //new TH1F(tmpstring,titlestring,Ebins,E_low,E_up);
    }
  }

#ifdef DEBUG
 cout << " *uu_c[0][0])[2*2+1]  = " << (*uu_c[0][0])[2*2+1]  << endl;
#endif

  return 0;}



Int_t Sel_Calibrator::WriteTree(TFile *rfile){
  if (!fQuiet){
  cout << " WriteTree to file " << rfile->GetName() << endl;
  }
  //  cout << " Entries in fCalTree :: " << fCalTree->GetEntries() << endl;
  // rfile->cd();
  TFile *_f = new TFile("caltest.root","RECREATE");

  //  fCalTree->SetDirectory(_f);
  // fCalTree->Write();

  _f->Close();
  if (!fQuiet){
  cout << " Done. " << endl;
  }
  return 0;
}




Int_t Sel_Calibrator::FitAllGlobal(Int_t C /* = -1 */ , Int_t F /* = -1 */)
{
  /* Note: By Default C and F are -1, if they are anything else we will only fit a certain cartridge and/or fin */

  
   TCanvas *c1;
   c1 = (TCanvas*)gROOT->GetListOfCanvases()->FindObject("c1");
   if (!c1) c1 = new TCanvas("c1","c1",10,10,1000,1000);

   ofstream globpeaks;
   ofstream globpeaks_com;
   Char_t peaklocationfilename[200];
   Char_t tmpstring[100];
   Float_t mean;
   Double_t xlow,xhigh,eres, d_eres;
   TString fDIR="GLOBFITS";
   
   if ( access( fDIR.Data(), 0 ) != 0 ) 
     {
       if (!fQuiet) cout << "creating dir " << fDIR << endl; 
       if ( mkdir(fDIR, 0777) != 0 ) 
	 { 
	  if (!fQuiet) cout << " Error making directory " << fDIR << endl;
	  return -2;
	 }
     }
   

   sprintf(peaklocationfilename,"%s/%s_globfits_spat.txt",fDIR.Data(),fFileBase.Data());
   globpeaks.open(peaklocationfilename);
   sprintf(peaklocationfilename,"%s/%s_globfits_com.txt",fDIR.Data(),fFileBase.Data());
   globpeaks_com.open(peaklocationfilename);

   TPaveText *labeltxt_spat = new TPaveText(.12,.8,.5,.88,"NDC");
   TPaveText *labeltxt_com = new TPaveText(.12,.8,.5,.88,"NDC");
   labeltxt_spat->SetShadowColor(0);
   labeltxt_com->SetShadowColor(0);

    for (Int_t cc=0;cc<CARTRIDGES_PER_PANEL;cc++){
      if ( ( C != -1) && ( C != cc) ) continue; 
     for (Int_t f=0;f<FINS_PER_CARTRIDGE;f++){
      if ( ( F != -1) && ( F != f) ) continue; 
       for (Int_t m=0;m<MODULES_PER_FIN;m++){
         for (Int_t j=0;j<APDS_PER_MODULE;j++){
	    if (verbose){
	      cout << "Fitting global energy histogram cartridge " << cc << " Fin " << f << " Module " << m << " APD " << j << endl;}
              c1->Clear();
              c1->Divide(1,2);
              c1->cd(1);

              labeltxt_spat->SetFillColor(kWhite);
 	      mean = Getmean (fCrysCal->GainSpat[cc][f][m][j]);
	      xlow= mean*0.85;
	      xhigh= mean*1.15;
              FitGlobal(fGlobHist[cc][f][m][j],xlow,xhigh,eres,d_eres);
	      if (eres > 0.9 ) mean = 5000;
	      WriteGlobPeaks(globpeaks,cc,f,m,j,eres,d_eres,mean);
               sprintf(tmpstring,"Eres = %.2f #pm %.2f FWHM",100*eres,100*d_eres);
               labeltxt_spat->Clear();
               labeltxt_spat->AddText(tmpstring);
	       labeltxt_spat->Draw();

	       c1->cd(2);

              labeltxt_com->SetFillColor(kWhite); 	  
	      mean = Getmean (fCrysCal->GainCom[cc][f][m][j]);
	      xlow= 400;//mean*0.85;
	      xhigh= 600;//mean*1.15;
              FitGlobal(fGlobHist_com[cc][f][m][j],xlow,xhigh,eres,d_eres);
	      if (eres > 0.9 ) mean = 5000;
              WriteGlobPeaks(globpeaks_com,cc,f,m,j,eres,d_eres,mean);
               sprintf(tmpstring,"Eres = %.2f #pm %.2f FWHM",100*eres,100*d_eres);
               labeltxt_com->Clear();
               labeltxt_com->AddText(tmpstring);
	       labeltxt_com->Draw();

               sprintf(peaklocationfilename,"%s/%s.C%dF%dM%dA%d_glob",fDIR.Data(),fFileBase.Data(),cc,f,m,j);
               strcat(peaklocationfilename,".png");
	       c1->Print(peaklocationfilename);

	 }
       }
     }
    }

    globpeaks.close();
    globpeaks_com.close();

    return 0;
}

Int_t Sel_Calibrator::WriteGlobPeaks(ofstream &globpeaks, Int_t cc, Int_t f, Int_t i, Int_t j, Double_t eres, Double_t d_eres, Float_t mean){
  if (globpeaks){
    globpeaks << " C" << cc << "F" << f << "M" << i << "A" << j << " " << eres << " " << d_eres << " " << mean << endl;
    return 0; }
  else return -1;

}



Float_t Sel_Calibrator::Getmean(Float_t *array){
  Double_t mean=0.;
  for (Int_t i=0;i<64;i++){
    mean+=array[i];
  }
  return mean/64.;}


Int_t Sel_Calibrator::WriteGlobHist(TString fileappendix){
  TString hfilename = fFileBase + fileappendix;
  TDirectory *subdir[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE];
  Char_t tmpstring[80];
  TFile *rfile = new TFile(hfilename.Data(),"RECREATE");
  for (Int_t cc=0;cc<CARTRIDGES_PER_PANEL;cc++){
     for (Int_t f=0;f<FINS_PER_CARTRIDGE;f++){
       sprintf(tmpstring,"C%dF%d",cc,f);
        subdir[cc][f] = rfile->mkdir(tmpstring);
        subdir[cc][f]->cd();
       for (Int_t m=0;m<MODULES_PER_FIN;m++){
         for (Int_t j=0;j<APDS_PER_MODULE;j++){
	   fGlobHist[cc][f][m][j]->Write();
	   fGlobHist_com[cc][f][m][j]->Write();
	 }
       }
     }
  }
  rfile->Close();

  return 0;
}
 


Int_t Sel_Calibrator::FitGlobal(TH1F *globhist, Double_t xlow, Double_t xhigh, Double_t &eres, Double_t &d_eres){

   if (verbose)   cout << "Fitting global histogram: " <<endl;



	    //	    xlow = Emeans[m*8+j*4+i]*0.85;
	    //	    xhigh = Emeans[m*8+j*4+i]*1.15;

            if (globhist->GetEntries() > MINHISTENTRIES){
      	       TF1 *globfits = fitapdspec(globhist,xlow,xhigh,1,verbose);

	       //               globfits[m][i][j]->SetName(tmpname);
	       //  globhist[m][i][j]->Draw();
               eres=2.35*globfits->GetParameter(5)/globfits->GetParameter(4);
               d_eres=2.35*errorprop_divide(globfits->GetParameter(5),globfits->GetParError(5),
                                       globfits->GetParameter(4),globfits->GetParError(4));
	       //   mean = getmean(CRYSTALPP[m][i][j]);
	    }
	    else{
	      // in case not enough entries we still want to store the globhist and draw it,
	      // globhist[m][i][j]->Draw();
	      eres=0.99;d_eres=0.99; //mean=5000;
	    }  // else

	    return 0;
} //Fitglobal


