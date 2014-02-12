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

