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
ClassImp(Sel_Calibrator)

void Sel_Calibrator::Begin(TTree * /*tree*/)
{
   // The Begin() function is called at the start of the query.
   // When running with PROOF Begin() is only called on the client.
   // The tree argument is deprecated (on PROOF 0 is passed).

   TString option = GetOption();

}

void Sel_Calibrator::SlaveBegin(TTree * /*tree*/)
{
   // The SlaveBegin() function is called after the Begin() function.
   // When running with PROOF SlaveBegin() is called on each slave server.
   // The tree argument is deprecated (on PROOF 0 is passed).

  cout << " Welcome to SlaveBegin Sel_Calibrator " << endl;
 
  TString hn;
  TString ht;
   TString option = GetOption();
   fCalTree = new TTree("CalTree","Energy calibrated LYSO-PSAPD module data ");
   fCalEvent   =  new ModuleCal();
   fCalTree->Branch("Calibrated Event Data",&fCalEvent);


   fOutput->Add(fCalTree);

   for (Int_t cc=0;cc<CARTRIDGES_PER_PANEL;cc++){
     for (Int_t f=0;f<FINS_PER_CARTRIDGE;f++){
       for (Int_t m=0;m<MODULES_PER_FIN;m++){
         for (Int_t j=0;j<APDS_PER_MODULE;j++){
	   hn.Form("GlobEhist_C%dF%dM%dA%d",cc,f,m,j);
             ht.Form("C%dF%d Unit %d Module %d",cc,f,m,j);
             fGlobHist[cc][f][m][j] = new TH1F(hn.Data(),ht.Data(), Ebins_pixel,E_low,E_up);
             hn.Form("GlobEhist_com_C%dF%dM%dA%d",cc,f,m,j);
             ht.Form("C%dF%d Unit %d Module %d Common",cc,f,m,j);
             fGlobHist_com[cc][f][m][j] = new TH1F(hn.Data(),ht.Data(), Ebins_com_pixel,E_low_com,E_up_com);
             fOutput->Add(fGlobHist[cc][f][m][j]);
             fOutput->Add(fGlobHist_com[cc][f][m][j]);
         }
       }
     } 
   }


  cout << " ... SlaveBegin Done " << endl;

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
      fCalEvent->Ecal=E* 511/fCrysCal->GainSpat[cartridge][fin][module][apd][_id];
      fCalEvent->Ec=-Ec* 511/fCrysCal->GainCom[cartridge][fin][module][apd][_id];
      fGlobHist[cartridge][fin][module][apd]->Fill(fCalEvent->E); 
      fGlobHist_com[cartridge][fin][module][apd]->Fill(fCalEvent->Ec); 
      fCalEvent->id=_id;
      
      Float_t VC;
      fCalEvent->ft = FineCalc(ft,VC,VC);
    }

   return kTRUE;
}

void Sel_Calibrator::SlaveTerminate()
{
   // The SlaveTerminate() function is called after all entries or objects
   // have been processed. When running with PROOF SlaveTerminate() is called
   // on each slave server.

}

void Sel_Calibrator::Terminate()
{
   // The Terminate() function is the last function to be called during
   // a query. It always runs on the client, it can be used to present
   // the results graphically or save the results to file.

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

#ifdef GOTOWORK

Int_t Sel_Calibrator::ReadUVCenters(TFile *rfile){
  Char_t tmpstring[40];  
  for (Int_t c=0;c<CARTRIDGES_PER_PANEL;c++){
    for (Int_t r=0;r<RENAS_PER_CARTRIDGE;r++){
      sprintf(tmpstring,"timing/uu_c[%d][%d]",c,r);
      uu_c[c][rena]= (TVector *) rfile->Get(tmpstring); //new TH1F(tmpstring,titlestring,Ebins,E_low,E_up);
      sprintf(tmpstring,"timing/vv_c[%d][%d]",c,r);
      vv_c[c][rena]= (TVector *) rfile->Get(tmpstring); //new TH1F(tmpstring,titlestring,Ebins,E_low,E_up);
    }
  }

  return 0;}


Int_t Sel_Calibrator::FitAllGlobal(){

   ofstream globpeaks;
   Float_t mean;
   sprintf(peaklocationfilename,"%s_globfits_spat.txt",filebase);
   globpeaks.open(peaklocationfilename);
    for (Int_t cc=0;cc<CARTRIDGES_PER_PANEL;cc++){
     for (Int_t f=0;f<FINS_PER_CARTRIDGE;f++){
       for (Int_t m=0;m<MODULES_PER_FIN;m++){
         for (Int_t j=0;j<APDS_PER_MODULE;j++){
	    if (verbose){
	      cout << "Fitting global energy histogram cartridge " << cc << " Fin " << f << " Module " << m << " APD " << j << endl;}
        	   FitGlobal(fGlobHist[cc][f][m][j]);

}


Int_t Sel_Calibrator::FitGlobal(TH1F *globhist){

   if (verbose)   cout << "Fitting global histogram: " <<endl;

	    hibin=0;
            max=0;
	    xlow = Emeans[m*8+j*4+i]*0.85;
	    xhigh = Emeans[m*8+j*4+i]*1.15;
            c1->Clear();
            if (globhist->GetEntries() > MINHISTENTRIES){
      	       globfits[m][i][j]=fitapdspec(globhist[m][i][j],xlow,xhigh,1,verbose);
	       sprintf(tmpname,"globfits[%d][%d][%d]",m,i,j);
               globfits[m][i][j]->SetName(tmpname);
               globhist[m][i][j]->Draw();
               eres=2.35*globfits[m][i][j]->GetParameter(5)/globfits[m][i][j]->GetParameter(4);
               d_eres=2.35*errorprop_divide(globfits[m][i][j]->GetParameter(5),globfits[m][i][j]->GetParError(5),
                                       globfits[m][i][j]->GetParameter(4),globfits[m][i][j]->GetParError(4));
                       mean = getmean(CRYSTALPP[m][i][j]);
	    }
	    else{
	      // in case not enough entries we still want to store the globhist and draw it,
               globhist[m][i][j]->Draw();
	       eres=0.99;d_eres=0.99; mean=5000;
	    }  // else

 	    globpeaks << " R" << m << "M" << i << "A" << j << " " << eres << " " << d_eres << " " << mean << endl;
 
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

   globpeaks.close();


   if (verbose)   cout << "Fitting Common global histogram common: " <<endl;

   //   sprintf(peaklocationfilename,"%s_globfits_com.txt",filebase);
   //   globpeaks.open(peaklocationfilename);


   //   for (m=FIRSTCHIP;m<LASTCHIP;m++){
   //     calfile->cd();
         //     sprintf(tmpstring,"RENA%d",m);
        //     subdir[m] = calfile->mkdir(tmpstring);
   //     subdir[m]->cd();
   //    for (Int_t i=0;i<4;i++){
   //     for (Int_t j=0;j<2;j++){
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
                       mean = getmean(CRYSTALPP_COM[m][i][j]);
	    }
	    else{
	      // in case not enough entries we still want to store the globhist and draw it,
               globhist_com[m][i][j]->Draw();
	       eres=0.99;d_eres=0.99; mean=5000;
	    }  // else
	    globpeaks << " R" << m << "M" << i << "A" << j << " " << eres << " " << d_eres << " " << mean << endl;
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


   globpeaks.close();
          calfile->Close();
         
      
	  ofstream ofile;

   for (m=FIRSTCHIP;m<LASTCHIP;m++){
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
}
#endif
