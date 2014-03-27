#define Sel_GetEhis_cxx
// The class definition in Sel_GetEhis.h has been generated automatically
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
// Root > T->Process("Sel_GetEhis.C")
// Root > T->Process("Sel_GetEhis.C","some options")
// Root > T->Process("Sel_GetEhis.C+")
//

#include "Sel_GetEhis.h"
#include <TStyle.h>
#include <TDirectory.h>
#include <TCanvas.h>
#include "time.h"
#include <sstream>

ClassImp(Sel_GetEhis)


void Sel_GetEhis::Begin(TTree * /*tree*/)
{
   // The Begin() function is called at the start of the query.
   // When running with PROOF Begin() is only called on the client.
   // The tree argument is deprecated (on PROOF 0 is passed).
  starttime = time(NULL);
  startloop = time (NULL);
  //  DefFFuncs();
  endloop = time (NULL);
  cout << " Time to create functions :: " <<  endloop-startloop  << endl;
}


void Sel_GetEhis::SlaveBegin(TTree * /*tree*/)
{
   // The SlaveBegin() function is called after the Begin() function.
   // When running with PROOF SlaveBegin() is called on each slave server.
   // The tree argument is deprecated (on PROOF 0 is passed).
  TString hn,ht;

   
  cout << " ----  Welcome to SlaveBegin !  ---- " << endl; 
 
  //#define USEPROOF
 
    /*
#ifdef USEPROOF
    fInput->ls();
    //  fInput->Print();
#endif
    */


  fInput->ls();


   TString option = GetOption();

  startloop = time (NULL);
   fCrysCal = dynamic_cast<PixelCal *>(fInput->FindObject("CrysCalPar"));
  endloop = time (NULL);
  cout << " Time to find fCrysCal :: " <<  endloop-startloop  << endl;



   cout << " fCrysCal->X[0][6][8][1][0] = " << fCrysCal->X[0][6][8][1][0] << endl;

   startloop = time (NULL);

    for (Int_t cc=0;cc<CARTRIDGES_PER_PANEL;cc++){
     for (Int_t f=0;f<FINS_PER_CARTRIDGE;f++){
       for (Int_t m=0;m<MODULES_PER_FIN;m++){
	 for (Int_t j=0;j<APDS_PER_MODULE;j++){
	   for (Int_t k=0;k<XTALS_PER_APD;k++){
	     hn.Form("Ehist_C%dF%dM%dA%dP%d",cc,f,m,j,k);
             ht.Form("C%dF%d Unit %d Module %d Pixel %d",cc,f,m,j,k);
	     fEhist[cc][f][m][j][k] = new TH1F(hn.Data(),ht.Data(), Ebins_pixel,E_low,E_up);
	     hn.Form("Ehist_com_C%dF%dM%dA%dP%d",cc,f,m,j,k);
             ht.Form("C%dF%d Unit %d Module %d Common Pixel %d",cc,f,m,j,k);
	     fEhist_com[cc][f][m][j][k] = new TH1F(hn.Data(),ht.Data(), Ebins_com_pixel,E_low_com,E_up_com);
	     fOutput->Add(fEhist[cc][f][m][j][k]);
	     fOutput->Add(fEhist_com[cc][f][m][j][k]);
	   }
	 }
       }
     } 
   }

  endloop = time (NULL);
  cout << " Time to create histos :: " <<  endloop-startloop  << endl;

}

Bool_t Sel_GetEhis::Process(Long64_t entry)
{
   // The Process() function is called for each entry in the tree (or possibly
   // keyed object in the case of PROOF) to be processed. The entry argument
   // specifies which entry in the currently loaded tree is to be processed.
   // It can be passed to either Sel_GetEhis::GetEntry() or TBranch::GetEntry()
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
 

  Int_t _id;



   fChain->GetTree()->GetEntry(entry);
     // perform time calibration
   if ((apd==0) || (apd==1) ) 
     //  event->ft=finecalc(event->ft,(*uu_c)(event->chip+RENACHIPS*event->module+event->apd*MODULES*RENACHIPS),(*vv_c)(event->chip+RENACHIPS*event->module+event->apd*MODULES*RENACHIPS) );

   if (fCrysCal->validpeaks[cartridge][fin][module][apd]){
              
	      // if (validpeaks[m][0][0]&&UNIT0.com1h<threshold)
	      if (fCrysCal->floodlut)
		{ cout << " WIP " << endl; return kFALSE;}
		//     { id=floodmap[chip][module][apd]->GetBinContent(floodmap[chip][module][apd]->FindBin(x,y));}
              else
		{   _id=fCrysCal->GetCrystalId(x,y,cartridge,fin,module,apd);}
	      if ((_id >= 0)&&( _id< 64)){  
                    fEhist[cartridge][fin][module][apd][_id]->Fill(E); 
                    fEhist_com[cartridge][fin][module][apd][_id]->Fill(-Ec); 
		} // if valid crystalid 
            } // if validpeaks

   //             calblock->Fill();

   
   return kTRUE;
}

void Sel_GetEhis::SlaveTerminate()
{
  cout << " Processing done :: " << time(NULL)-startloop << endl;

   // The SlaveTerminate() function is called after all entries or objects
   // have been processed. When running with PROOF SlaveTerminate() is called
   // on each slave server.

}

void Sel_GetEhis::Terminate()
{
   // The Terminate() function is the last function to be called during
   // a query. It always runs on the client, it can be used to present
   // the results graphically or save the results to file.

   cout << " Welcome to Terminate ! " << endl;
   cout << " time since start :: " << time(NULL)-starttime << endl;

   TString hn;
   //   fOutput->ls();


    for (Int_t cc=0;cc<CARTRIDGES_PER_PANEL;cc++){
	 for (Int_t f=0;f<FINS_PER_CARTRIDGE;f++){
           for (Int_t i=0;i<MODULES_PER_FIN;i++){
	     for (Int_t j=0;j<APDS_PER_MODULE;j++){
	       for (Int_t k=0;k<XTALS_PER_APD;k++){
		 hn.Form("Ehist_C%dF%dM%dA%dP%d",cc,f,i,j,k);
		 fEhist[cc][f][i][j][k] = dynamic_cast<TH1F *> (fOutput->FindObject(hn.Data()));
		 hn.Form("Ehist_com_C%dF%dM%dA%dP%d",cc,f,i,j,k);
		 fEhist_com[cc][f][i][j][k] = dynamic_cast<TH1F *> (fOutput->FindObject(hn.Data()));
	       } //k
	     } //j 
	   } //i
	 } //f 
       } //cc


   cout << " Terminate done time since start :: " << time(NULL)-starttime << endl;

}

//#ifdef IFOUNDIT

Int_t Sel_GetEhis::WriteHists(TFile *rfile){
  Char_t tmpstring[40];     
  TDirectory *m_subdir;
  TDirectory *a_subdir;
  TDirectory *f_subdir;
  TDirectory *s_subdir;
  //  TDirectory *gDirectory;

   cout << " Writehists time since start :: " << time(NULL)-starttime << endl;
   startloop = time(NULL);
  rfile->cd();
 


       for (Int_t c=0;c<CARTRIDGES_PER_PANEL;c++){
	 for (Int_t f=0;f<FINS_PER_CARTRIDGE;f++){
   	   sprintf(tmpstring,"C%dF%d",c,f);
           f_subdir = (TDirectory *) rfile->Get(tmpstring);
	   if (!(f_subdir)) f_subdir = rfile->mkdir(tmpstring);
	   if (verbose){ 
	     f_subdir->cd();
	     cout << "F" << f << " : "  << endl;
	     f_subdir->pwd();
	     cout << " global :: "  << endl;
	     gDirectory->pwd(); }
           for (Int_t i=0;i<MODULES_PER_FIN;i++){
       	     sprintf(tmpstring,"M%d",i);
             m_subdir = (TDirectory *) f_subdir->Get(tmpstring);              
	     if (!(m_subdir)) m_subdir = f_subdir->mkdir(tmpstring);
	     m_subdir->cd();
	     if (verbose) {cout << "F" << f << "M" << i << " : " << endl;
	       gDirectory->pwd() ;}
	     for (Int_t j=0;j<APDS_PER_MODULE;j++){
       	     sprintf(tmpstring,"A%d",j);
	     a_subdir = (TDirectory *) m_subdir->Get(tmpstring);              
	     if (!(a_subdir)) a_subdir = m_subdir->mkdir(tmpstring);
             a_subdir->cd();
	     if(verbose){ cout << "F" << f << "M" << i << "A" << j << " : " << endl;
	       gDirectory->pwd();}
		 // place to fit these histograms -- FIXME
	     if ( FitApdEhis(c,f,i,j) ) cout << " Something Wrong. Exiting function WriteHists.\n return -1;" << endl;
       	     sprintf(tmpstring,"Spatial");
	     s_subdir = (TDirectory *) a_subdir->Get(tmpstring);              
	     if (!(s_subdir)) s_subdir = a_subdir->mkdir(tmpstring);
             s_subdir->cd();
	     if (verbose){ 
	       cout << "F" << f << "M" << i << "A" << j << " Spatial : " << endl;
	       gDirectory->pwd();}
	       for (Int_t k=0;k<XTALS_PER_APD;k++){
		 fEhist[c][f][i][j][k]->Write();
	       }
	       a_subdir->cd();
       	     sprintf(tmpstring,"Common");
	     s_subdir = (TDirectory *) a_subdir->Get(tmpstring);              
	     if (!(s_subdir)) s_subdir = a_subdir->mkdir(tmpstring);
             s_subdir->cd();
	     if (verbose){
	       cout << "F" << f << "M" << i << "A" << j << " Common : " << endl;
	       gDirectory->pwd();}
	       for (Int_t k=0;k<XTALS_PER_APD;k++){
		 fEhist_com[c][f][i][j][k]->Write();
		 //   if ( verbose ) { cout << " ppeaks[" << c << "][" << f << "][" << i << "][" << j << "] = " << fPH->spat[c][f][i][j] ;}
	       } // k 
               a_subdir->cd();
	       m_subdir->cd();
	       //	       delete a_subdir; 
	     } // j
             f_subdir->cd();
	     //	     delete m_subdir;
	   } // i
           rfile->cd();
       if (verbose) cout << endl;
       	} //loop over f (fins)
       } // loop over c


   cout << " Writehists done time since start :: " << time(NULL)-starttime << endl;
   cout << "                 time for Function:: " << time(NULL) - startloop  << endl;

       return 0;}

Int_t Sel_GetEhis::FitApdEhis(Int_t cc, Int_t f, Int_t i, Int_t j){
  Float_t Emean, Emean_com,peak;
  Char_t tmpstring[40];
  TF1 *spatfit  = new TF1("spatfit","gaus", EFITMIN, EFITMAX);
  TF1 *comfit =  new TF1("comfit","gaus",EFITMIN_COM,EFITMAX_COM);


  // verbose=kTRUE;
	  if (verbose) { 
              cout << endl; 
              cout << " --------------- C" << cc << "F" <<f << " MODULE " << i << " PSAPD " << j;
	      cout << " ---------------- " << endl;}
       
         if (!fCrysCal) { cout << " Warning :: fCrysCal not defined. Please do : " << endl ;
	    cout << " PixelCal *CrysCal = new PixelCal(\"CrysCalPar\"); \n CrysCal->ReadCal(filebase); \n  Sel_GetEhis::SetPixelCal(CrysCal); " << endl;
	    cout << " exiting. " << endl;
            return -1;
	  }
 
	 if (!fPPeaks) { cout << " Warning :: fPPeaks not defined. Please check. " << endl ;
	    cout << " exiting. " << endl;
            return -1;
	  }
 

	  if (fCrysCal->validpeaks[cc][f][i][j]){
	    if (verbose) {cout << " Getting mean energy :: " << endl;
               cout << " fPPeaks : " << fPPeaks << endl;
               cout << " fPPeaks->spat[cc][f][i][j] = " << fPPeaks->spat[cc][f][i][j] <<endl;
               cout << " fPPeaks->com[cc][f][i][j] = " << fPPeaks->com[cc][f][i][j] <<endl;}
	       Emean = fPPeaks->spat[cc][f][i][j];
	       Emean_com = fPPeaks->com[cc][f][i][j];
	       /*
	       Emeans[m*8+j*4+i] = (Float_t )  (*ppVals)(m*8+j*4+i);
	       Emeans_com[m*8+j*4+i] = (Float_t )  (*ppVals_com)(m*8+j*4+i);
	       */

	       if (verbose)  { cout  << " S :: ppVals : " << Emean   << " Entries :" << fEhist[cc][f][i][j][0]->GetEntries()   << endl;}
	       if (verbose)  { cout  << " C :: ppVals : " << Emean_com   << " Entries :" << fEhist_com[cc][f][i][j][0]->GetEntries()   << endl;}

               for (Int_t k=0;k<XTALS_PER_APD;k++){
	      //	      fitall(Ehist[i][j], ftmp, &vals[0], pixvals, E_low, E_up, c1, verbose);
		 //		 sprintf(tmpstring,"fEfits[%d][%d][%d][%d][%d]",cc,f,i,j,k);
                 if ( fEhist[cc][f][i][j][k]->GetEntries() < MINPIXENTRIES ) fEhist[cc][f][i][j][k]->Rebin(2); 
		 //     if ( Ehist[m][i][j][k]->GetEntries() < MINPIXENTRIES/2 ) Ehist[m][i][j][k]->Rebin(2); 
		 if (verbose) {
		   cout << "===================================================================" << endl;
		   cout << "= Histogram["<<cc<<"]["<<f<<"]["<<i<<"]["<<j<<"]["<<k<<"]"                         << endl;
		   cout << "===================================================================" << endl;
		   cout << " ----------- Spatials ---------------- " <<endl;
		   cout << " Hist entries : " << fEhist[cc][f][i][j][k]->GetEntries() << " Efits mean :" ;
		   cout << fPPeaks->spat[cc][f][i][j] << " nrbins :: " << fEhist[cc][f][i][j][k]->GetNbinsX() << endl;}
		 // edge has lower gain
		 if ((k==0)||(k==7)||(k==54)||(k==63)){ 
		   peak=GetPeak( fEhist[cc][f][i][j][k], Emean*0.7,Emean*1.15,0);
		   if (peak==NOVALIDPEAKFOUND) { 
		     if (verbose) cout << " That didn't go well. Retry to get peak with larger margins " << endl;
		     peak=GetPeak( fEhist[cc][f][i][j][k], Emean*0.6,Emean*1.5,1);
		   }}
		 else { peak=GetPeak( fEhist[cc][f][i][j][k], Emean*0.85,Emean*1.15,0);
		   if (peak==NOVALIDPEAKFOUND) { 
		     if (verbose) cout << " That didn't go well. Retry to get peak with larger margins " << endl;
		     peak=GetPeak( fEhist[cc][f][i][j][k], Emean*0.7,Emean*1.5,1);}
		}
		 if (peak==NOVALIDPEAKFOUND) peak=Emean;
		 //		 fEfit[cc][f][i][j][k]->SetParameter(1,peak);
		 //	 fEfit[cc][f][i][j][k]->SetParameter(2,0.04*peak);
		         spatfit->SetParameter(1,peak);
		 	 spatfit->SetParameter(2,0.04*peak);

		 if (verbose) {
		   cout << " Peak @ " << peak <<", fitting between : " << peak*0.85 << " and " << peak*1.15 << endl;}
		 if ( (fEhist[cc][f][i][j][k]->GetEntries()) >  MINEHISTENTRIES ){
		   fEhist[cc][f][i][j][k]->Fit(spatfit,"Q","",peak*0.85,peak*1.15);}
		 //fEfit[cc][f][i][j][k]->SetLineColor(kRed);
		 fCrysCal->GainSpat[cc][f][i][j][k]=spatfit->GetParameter(1);
		 if (TMath::Abs(fCrysCal->GainSpat[cc][f][i][j][k]/Emean -1 ) > 0.3 )          {
		   if (verbose) { cout << " BAD FIT to histogram ["<<c<<"]["<<f<<"]["<<i<<"]["<<j<<"]["<<k<<"]" << endl; 
		     cout << " taking PP @ " << Emean << " ( peak = " << peak << " ) " << endl;}
		   fCrysCal->GainSpat[cc][f][i][j][k]=Emean;}
		 if (spatfit->GetParameter(1)){
		   fCrysCal->EresSpat[cc][f][i][j][k]=235*spatfit->GetParameter(2)/spatfit->GetParameter(1) ;}
                 else fCrysCal->EresSpat[cc][f][i][j][k]=.99;

		 if (verbose) {
		   cout << " ------------ Common ----------------- " <<endl;
		   cout << " Hist entries : " << fEhist_com[cc][f][i][j][k]->GetEntries() << " Efits mean :" ;
		   cout << fPPeaks->com[cc][f][i][j] << endl;}
		 fEhist_com[cc][f][i][j][k]->SetAxisRange(E_low_com,SATURATIONPEAK,"X");
		 peak=GetPeak( fEhist_com[cc][f][i][j][k], Emean_com*0.85,Emean_com*1.25,0);
		 if (peak==NOVALIDPEAKFOUND) { 
		  if (verbose) cout << " That didn't go well. Retry to get peak with larger margins " << endl;
                  if ((k==0)||(k==7)||(k==54)||(k==63)){ peak=GetPeak( fEhist_com[cc][f][i][j][k], Emean_com*0.6,Emean_com*1.25,1);}
		  else {peak=GetPeak( fEhist_com[cc][f][i][j][k], Emean_com*0.7,Emean_com*1.5,1);}
		 }
		 if (peak==NOVALIDPEAKFOUND) peak=Emean_com;
		 //		 fEfit_com[cc][f][i][j][k]->SetParameter(1,peak);
		 //		 fEfit_com[cc][f][i][j][k]->SetParameter(2,0.04*peak);
		 comfit->SetParameter(1,peak);
		 comfit->SetParameter(2,0.04*peak);
		 if (verbose) {
		   cout << " Peak @ " << peak <<", fitting between : " << peak*0.85 << " and " << peak*1.15 << endl;}
		 if ( (fEhist_com[cc][f][i][j][k]->GetEntries()) >  MINEHISTENTRIES ){
		   fEhist_com[cc][f][i][j][k]->Fit(comfit,"Q","",peak*0.85,peak*1.15);}
		 //fEfit_com[cc][f][i][j][k]->SetLineColor(kBlue);
		 fCrysCal->GainCom[cc][f][i][j][k]=comfit->GetParameter(1);
		 if (TMath::Abs(fCrysCal->GainCom[cc][f][i][j][k]/Emean_com -1 ) > 0.35 )          {
		   if (verbose) {cout << " BAD FIT to histogram ["<<cc<<"]["<<f<<"]["<<i<<"]["<<j<<"]["<<k<<"]" << endl;
                     cout << " taking PP @ " << Emean_com << " ( peak = " << peak << " ) " << endl;}
		   fCrysCal->GainCom[cc][f][i][j][k]=Emean_com;}

		 if (comfit->GetParameter(1)){
		   fCrysCal->EresCom[cc][f][i][j][k]=235*comfit->GetParameter(2)/comfit->GetParameter(1) ;}
                 else fCrysCal->EresCom[cc][f][i][j][k]=.99;

	       }  // loop over k
	  } // validpeaks


	  //   cout << " CrysCal->GainSpat[0][0][2][1][0] = "<<  fCrysCal->GainSpat[0][0][2][1][0] << endl;

	  delete spatfit;
	  delete comfit;

	  return 0;}


void Sel_GetEhis::DefFFuncs(void){
  cout << "  Defining Fit Funcs .. " << endl;
  /*
   TString hn;
   for (Int_t cc=0;cc<CARTRIDGES_PER_PANEL;cc++){
     for (Int_t f=0;f<FINS_PER_CARTRIDGE;f++){
       for (Int_t m=0;m<MODULES_PER_FIN;m++){
	 for (Int_t j=0;j<APDS_PER_MODULE;j++){
	   for (Int_t k=0;k<XTALS_PER_APD;k++){
	     hn.Form("Efit_C%dF%dM%dA%dP%d",cc,f,m,j,k);
	     fEfit[cc][f][m][j][k] = new TF1(hn.Data(),"gaus", EFITMIN, EFITMAX);
	     hn.Form("Efit_com_C%dF%dM%dA%dP%d",cc,f,m,j,k);
	     fEfit_com[cc][f][m][j][k] = new TF1(hn.Data(),"gaus",EFITMIN_COM,EFITMAX_COM);
	   }
	 }
       }
     } 
   }
  */
   cout << "  .... Got Funcs ! "  << endl;
}


void Sel_GetEhis::LoadEHis(TFile *rfile)
{

   cout << " Welcome to Load Histogram Function ! " << endl;
   Char_t tmpstring[40];     

   TString hn;
   rfile->ls();

   fPPeaks = (PPeaks *) rfile->Get("PhotoPeaks");

   cout << " Defining Fit Functions " << endl;
   // DefFFuncs();

   cout << " Reading Histograms " << endl;
    for (Int_t cc=0;cc<CARTRIDGES_PER_PANEL;cc++){
	 for (Int_t f=0;f<FINS_PER_CARTRIDGE;f++){
           for (Int_t i=0;i<MODULES_PER_FIN;i++){
	     for (Int_t j=0;j<APDS_PER_MODULE;j++){
	       for (Int_t k=0;k<XTALS_PER_APD;k++){
		 hn.Form("C%dF%d/M%d/A%d/Spatial/Ehist_C%dF%dM%dA%dP%d",cc,f,i,j,cc,f,i,j,k);
		 fEhist[cc][f][i][j][k] = (TH1F *) (rfile->Get(hn.Data()));
		 hn.Form("Efit_C%dF%dM%dA%dP%d",cc,f,i,j,k);
		 //   fEfit[cc][f][i][j][k] = (TF1 *) fEhist[cc][f][i][j][k]->GetListOfFunctions()->FindObject(hn.Data());
		 hn.Form("C%dF%d/M%d/A%d/Common/Ehist_com_C%dF%dM%dA%dP%d",cc,f,i,j,cc,f,i,j,k);
		 fEhist_com[cc][f][i][j][k] = (TH1F *) (rfile->Get(hn.Data()));
		 hn.Form("Efit_com_C%dF%dM%dA%dP%d",cc,f,i,j,k);
		 //   fEfit[cc][f][i][j][k] = (TF1 *) fEhist[cc][f][i][j][k]->GetListOfFunctions()->FindObject(hn.Data());
	       } //k
	     } //j 
	   } //i
	 } //f 
       } //c


    cout << " Histograms Loaded." << endl;

   /*
    for (Int_t cc=0;cc<CARTRIDGES_PER_PANEL;cc++){
	 for (Int_t f=0;f<FINS_PER_CARTRIDGE;f++){
           sprintf(tmpstring,"C%dF%d",c,f);
           f_subdir = (TDirectory *) rfile->Get(tmpstring);
	   f_subdir->cd();
           for (Int_t i=0;i<MODULES_PER_FIN;i++){
	     sprintf(tmpstring,"M%d",i);
	     m_subdir = (TDirectory *) f_subdir->Get(tmpstring);
	     m_subdir->cd();
	     for (Int_t j=0;j<APDS_PER_MODULE;j++){
	       sprintf(tmpstring,"A%d",j);
	       a_subdir = (TDirectory *) m_subdir->Get(tmpstring);
	       a_subdir->cd();
	       sprintf(tmpstring,"Spatial");
	       s_subdir = (TDirectory *) a_subdir->Get(tmpstring);
	       s_subdir->cd();
	       for (Int_t k=0;k<XTALS_PER_APD;k++){
		 hn.Form("Ehist_C%dF%dM%dA%dP%d",cc,f,i,j,k);
		 fEhist[cc][f][i][j][k] = dynamic_cast<TH1F *> (s_subdir->FindObject(hn.Data()));
	       }
	       a_subdir->cd();
	       sprintf(tmpstring,"Common");
	       s_subdir = (TDirectory *) a_subdir->Get(tmpstring);
	       s_subdir->cd();
	       for (Int_t k=0;k<XTALS_PER_APD;k++){
		 hn.Form("Ehist_com_C%dF%dM%dA%dP%d",cc,f,i,j,k);
		 fEhist_com[cc][f][i][j][k] = dynamic_cast<TH1F *> (s_subdir->FindObject(hn.Data()));
	       } //k
	       a_subdir->cd();
	     } //j 
	     m_subdir->cd();
	   } //i
	   f_subdir->cd();
	 } //f 
       } //c

   */

}



Int_t Sel_GetEhis::FitAll() {



  // need to define ppVals and ppVals_com  and Emean and Emeans

  for (Int_t cc=0;cc<CARTRIDGES_PER_PANEL;cc++){
    for (Int_t f=0;f<FINS_PER_CARTRIDGE;f++){
      for (Int_t i=0;i<MODULES_PER_FIN;i++){
	for (Int_t j=0;j<APDS_PER_MODULE;j++){
          FitApdEhis(cc,f,i,j);
	   } // j 
      } //j
    } // f
  } //c
  return 0 ;}

Float_t Sel_GetEhis::GetPeak(TH1F *hist, Float_t xlow, Float_t xhigh, Bool_t force){
  Int_t npeaks=0;
  Int_t i=0,corpeak=0;
  TSpectrum *ss = new TSpectrum();
  Float_t yy;
  //  Float_t corpeakpos;
  if (verbose) { cout << " Funtion GetPeak. Looking for peak between " << xlow << " and " << xhigh << endl;}
  yy=0;

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
       if( (yy < *(ss->GetPositionY() + i) ) && (*(ss->GetPositionX() + i) > xlow) && (*(ss->GetPositionX() +i) < xhigh)) {
	corpeak = i;
        yy = *(ss->GetPositionY() + i);
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
		if((yy < *(ss->GetPositionY() + j)) && (*(ss->GetPositionX() + j) > xlow) && (*(ss->GetPositionX() +j) < xhigh)) {
		corpeak = j;
		yy = *(ss->GetPositionY() + j);
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
	yy = 0;

	/* note 9-6-13: changed sigma from 2 to 3,  need to clean up FIXME */
	npeaks = ss->Search(hist, 3, "", 0.4);
	for(i = 0; i < npeaks; i++) {
	if(verbose) {cout << "x= " << *(ss->GetPositionX() + i) << " y = " << *(ss->GetPositionY() + i) << endl;}

				/* take largest peak with x position larger than lower fit limit */
			/* CHANGE !! */
	if((yy < *(ss->GetPositionY() + i)) && (*(ss->GetPositionX() + i) > xlow) && (*(ss->GetPositionX() +i) < xhigh)) {
			corpeak = i;
			yy = *(ss->GetPositionY() + i);
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
	 if((yy < *(ss->GetPositionY() + i)) && (*(ss->GetPositionX() + i) > xlow) && (*(ss->GetPositionX() +i) < xhigh)) {
		corpeak = i;
		yy = *(ss->GetPositionY() + i);
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
	if((yy < *(ss->GetPositionY() + i)) && (*(ss->GetPositionX() + i) > xlow) && (*(ss->GetPositionX() +i) < xhigh)) {
		corpeak = i;
		yy = *(ss->GetPositionY() + i);
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
                                              
			yy = 0;
			for(i = 0; i < npeaks; i++) {
				if(yy < *(ss->GetPositionX() + i)) {
					corpeak = i;
					yy = *(ss->GetPositionX() + i);
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

Int_t Sel_GetEhis::GenPlots(Int_t cc, Int_t firstfin, Int_t lastfin){
  TCanvas *c1 =new TCanvas();
  stringstream filebase;

  string tmpstring;
  if (firstfin <0) firstfin = 0;
  if (lastfin > FINS_PER_CARTRIDGE ) lastfin = FINS_PER_CARTRIDGE;
  if (( cc > CARTRIDGES_PER_PANEL ) || (cc < 0 )) { cout << " Error in GenPlots. Please specify correct cartridge " << endl; }

  Bool_t firstloop=1;
    for (Int_t f=firstfin;f<lastfin;f++){
      filebase << fFileBase << "_crystalpeakfits_C"  << cc << "F"  << f  << "_com" ;
      for (Int_t i=0;i<MODULES_PER_FIN;i++){
	for (Int_t j=0;j<APDS_PER_MODULE;j++){
            for (Int_t kk=0;kk<2;kk++){
             c1->Clear();
             c1->Divide(8,4);
             for (Int_t k=0;k<32;k++){
	       c1->cd(k+1);
                fEhist_com[cc][f][i][j][k+kk*32]->Draw();
                //Efits_com[cc][f][i][j][k+kk*32]->Draw("same");
	      }
             if (firstloop) { tmpstring = filebase.str() + ".ps(" ; firstloop=0;}
             else { tmpstring = filebase.str() + ".ps" ;}
	     c1->Print(tmpstring.c_str());
	    }  // loop kk
	} // loop j
      }	   // loop over i
    } // loop over f


    tmpstring = filebase.str() + ".ps)";
    c1->Print(tmpstring.c_str());

   firstloop=1;

   filebase.str("");
    for (Int_t f=firstfin;f<lastfin;f++){
      filebase << fFileBase << "_crystalpeakfits_C" << cc << "F" << f <<"_com" ;
      for (Int_t i=0;i<MODULES_PER_FIN;i++){
	for (Int_t j=0;j<APDS_PER_MODULE;j++){
            for (Int_t kk=0;kk<2;kk++){
             c1->Clear();
             c1->Divide(8,4);
             for (Int_t k=0;k<32;k++){
	       c1->cd(k+1);
                fEhist[cc][f][i][j][k+kk*32]->Draw();
		//                Efits[m][i][j][k+kk*32]->Draw("same");
	      }
             if (firstloop) {  tmpstring = filebase.str() + ".ps(" ; firstloop=0;}
             else { tmpstring =filebase.str() + ".ps" ;}
	     c1->Print(tmpstring.c_str());
	    } //kk
           } // loop j
	  } // loop over i
         } // loop over f 
    tmpstring = filebase.str() + ".ps)";
    c1->Print(tmpstring.c_str());


   return 0;}


/*
void Sel_GetEhis::ReadEHistos(TFile *rfile)
{
   // The Terminate() function is the last function to be called during
   // a query. It always runs on the client, it can be used to present
   // the results graphically or save the results to file.


   DefHistos();

   TString hn;
   fOutput->ls();

    for (Int_t cc=0;cc<CARTRIDGES_PER_PANEL;cc++){
	 for (Int_t f=0;f<FINS_PER_CARTRIDGE;f++){
           for (Int_t i=0;i<MODULES_PER_FIN;i++){
	     for (Int_t j=0;j<APDS_PER_MODULE;j++){
	       for (Int_t k=0;k<XTALS_PER_APD;k++){
		 hn.Form("Ehist_C%dF%dM%dA%dP%d",cc,f,i,j,k);
		 fEhist[cc][f][i][j][k] = dynamic_cast<TH1F *> (rfile->FindObject(hn.Data()));
		 hn.Form("Ehist_com_C%dF%dM%dA%dP%d",cc,f,i,j,k);
		 fEhist_com[cc][f][i][j][k] = dynamic_cast<TH1F *> (rfile->FindObject(hn.Data()));
	       } //k
	     } //j 
	   } //i

	 } //f 
       } //c



}
*/


 //#endif
