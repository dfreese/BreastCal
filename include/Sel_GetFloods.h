//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Tue Dec 17 13:01:33 2013 by ROOT version 5.34/01
// from TChain mdata/mdata
//////////////////////////////////////////////////////////

#ifndef Sel_GetFloods_h
#define Sel_GetFloods_h
#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TSelector.h>
#include <TH2F.h>
// Header file for the classes stored in the TTree if any.
#include <TObject.h>
#include "Syspardef.h"
#include "PPeaks.h"
//#include "/home/miil/MODULE_ANA/ANA_V5/SpeedUp/include/Syspardef.h"
//#include "/home/miil/MODULE_ANA/ANA_V5/SpeedUp/include/PPeaks.h"
#include <Riostream.h>

// Fixed size dimensions of array or collections stored in the TTree if any.

class Sel_GetFloods : public TSelector {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   TH2F           *fH2F;
   Int_t           fNfloods;
   TH2F            *fFloods[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE];
   TString         fOutputfilename;
   string         fFileBase;
   Long64_t        fNumberOfEvents;
   Double_t        fPhotoPeaks[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE];
   PPeaks          *fPH;
   //  vector<vector<vector<vector<Double_t> > > >  fPhotoPeaks; 

   // Declaration of leaf types
 //ModuleDat       *eventdata;
   UInt_t          fUniqueID;
   UInt_t          fBits;
   Long64_t        ct;
   Short_t         cartridge;
   Short_t         fin;
   Short_t         chip;
   Short_t         module;
   Short_t         apd;
   Float_t         Ec;
   Float_t         Ech;
   Float_t         x;
   Float_t         y;
   Float_t         E;
   Double_t        ft;
   Int_t           pos;
   Short_t         aa;
   Short_t         bb;
   Short_t         cc;
   Short_t         dd;
   Short_t         id;

   // List of branches
   TBranch        *b_eventdata_fUniqueID;   //!
   TBranch        *b_eventdata_fBits;   //!
   TBranch        *b_eventdata_ct;   //!
   TBranch        *b_eventdata_cartridge;   //!
   TBranch        *b_eventdata_fin;   //!
   TBranch        *b_eventdata_chip;   //!
   TBranch        *b_eventdata_module;   //!
   TBranch        *b_eventdata_apd;   //!
   TBranch        *b_eventdata_Ec;   //!
   TBranch        *b_eventdata_Ech;   //!
   TBranch        *b_eventdata_x;   //!
   TBranch        *b_eventdata_y;   //!
   TBranch        *b_eventdata_E;   //!
   TBranch        *b_eventdata_ft;   //!
   TBranch        *b_eventdata_pos;   //!
   TBranch        *b_eventdata_a;   //!
   TBranch        *b_eventdata_b;   //!
   TBranch        *b_eventdata_c;   //!
   TBranch        *b_eventdata_d;   //!
   TBranch        *b_eventdata_id;   //!

 Sel_GetFloods(TTree * /*tree*/ =0) : fChain(0), fH2F(0) { 
     //             fH2F=0;  
             fNumberOfEvents=0; 
             fPH=0;
	     //             fOutputfilename = new Char_t[150]; 
	     fOutputfilename="Sel_out.root";
             fFileBase="Sel_base";
	     //    fPhotoPeaks.resize(CARTRIDGES_PER_PANEL);
               for (Int_t c=0;c<CARTRIDGES_PER_PANEL;c++){
		 //	 fPhotoPeaks[c].resize(FINS_PER_CARTRIDGE);
		 for (Int_t f=0;f<FINS_PER_CARTRIDGE;f++){
		   //fPhotoPeaks[c][f].resize(MODULES_PER_FIN);
		   for (Int_t m=0;m<MODULES_PER_FIN;m++){
		     //fPhotoPeaks[c][f][m].resize(APDS_PER_MODULE);
		     for (Int_t j=0;j<APDS_PER_MODULE;j++){
		       fFloods[c][f][m][j]=0;
		     }
		   }
		 }
	       }
             fNfloods = CARTRIDGES_PER_PANEL*FINS_PER_CARTRIDGE*MODULES_PER_FIN*APDS_PER_MODULE;
	     //	     fFloods = 0;

}
   virtual void   SetOutputFileName( const Char_t* name ) { fOutputfilename = name ;}
   virtual void   SetFileBase( const Char_t* name ) { fFileBase = name ;}
   void  SetNFloods(Int_t nf) { fNfloods = nf ; } 
   Int_t GetNFloods() { return fNfloods; }
#ifndef __CINT__
   Int_t Makeplot(TH2F *hist[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE],Int_t c, Int_t f, Int_t NRFLOODSTODRAW,const Char_t suffix[40]) ;
   Int_t WriteFloods(TFile *rfile,Int_t verbose);
   // note CINT only works up to 3D arrays see root.cern.ch/phpBB3/viewtopic.php?t=8880, hopefully this if statement solves it
   void SetPhotoPeaks(Double_t peaks[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE] ){ 
      memcpy(fPhotoPeaks,peaks,sizeof(fPhotoPeaks)); 
      fPhotoPeaks[0][0][2][0]=peaks[0][0][2][0];} 
#endif

   virtual ~Sel_GetFloods() { }
   virtual Int_t   Version() const { return 2; }
   virtual void    Begin(TTree *tree);
   virtual void    SlaveBegin(TTree *tree);
   virtual void    Init(TTree *tree);
   virtual Bool_t  Notify();
   virtual Bool_t  Process(Long64_t entry);
   virtual Int_t   GetEntry(Long64_t entry, Int_t getall = 0) { return fChain ? fChain->GetTree()->GetEntry(entry, getall) : 0; }
   virtual void    SetOption(const char *option) { fOption = option; }
   virtual void    SetObject(TObject *obj) { fObject = obj; }
   virtual void    SetInputList(TList *input) { fInput = input; }
   virtual TList  *GetOutputList() const { return fOutput; }
   virtual void    SlaveTerminate();
   virtual void    Terminate();



   ClassDef(Sel_GetFloods,6)
};

#endif

#ifdef Sel_GetFloods_cxx

void Sel_GetFloods::Init(TTree *tree)
{

   cout << " +++++ RUNNING INIT ++++++ " << endl;
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("fUniqueID", &fUniqueID, &b_eventdata_fUniqueID);
   fChain->SetBranchAddress("fBits", &fBits, &b_eventdata_fBits);
   fChain->SetBranchAddress("ct", &ct, &b_eventdata_ct);
   fChain->SetBranchAddress("cartridge", &cartridge, &b_eventdata_cartridge);
   fChain->SetBranchAddress("fin", &fin, &b_eventdata_fin);
   fChain->SetBranchAddress("chip", &chip, &b_eventdata_chip);
   fChain->SetBranchAddress("module", &module, &b_eventdata_module);
   fChain->SetBranchAddress("apd", &apd, &b_eventdata_apd);
   fChain->SetBranchAddress("Ec", &Ec, &b_eventdata_Ec);
   fChain->SetBranchAddress("Ech", &Ech, &b_eventdata_Ech);
   fChain->SetBranchAddress("x", &x, &b_eventdata_x);
   fChain->SetBranchAddress("y", &y, &b_eventdata_y);
   fChain->SetBranchAddress("E", &E, &b_eventdata_E);
   fChain->SetBranchAddress("ft", &ft, &b_eventdata_ft);
   fChain->SetBranchAddress("pos", &pos, &b_eventdata_pos);
   fChain->SetBranchAddress("a", &aa, &b_eventdata_a);
   fChain->SetBranchAddress("b", &bb, &b_eventdata_b);
   fChain->SetBranchAddress("c", &cc, &b_eventdata_c);
   fChain->SetBranchAddress("d", &dd, &b_eventdata_d);
   fChain->SetBranchAddress("id", &id, &b_eventdata_id);
}

Bool_t Sel_GetFloods::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

#endif // #ifdef Sel_GetFloods_cxx

