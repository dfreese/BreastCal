//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Thu Jan 23 12:01:18 2014 by ROOT version 5.34/01
// from TChain mdata/mdata
//////////////////////////////////////////////////////////

#ifndef Sel_Calibrator_h
#define Sel_Calibrator_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TSelector.h>
#include <TH1F.h>
#include <PixelCal.h>
#include <ModuleCal.h>

// Header file for the classes stored in the TTree if any.
#include <TObject.h>

// Fixed size dimensions of array or collections stored in the TTree if any.

class Sel_Calibrator : public TSelector {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain

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
   Short_t         a;
   Short_t         b;
   Short_t         c;
   Short_t         d;
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


   // will hold calibration data
   PixelCal *fCrysCal ;
   TTree *fCalTree;
   ModuleCal   *fCalEvent;
   TH1F *fGlobHist[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE];
   TH1F *fGlobHist_com[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE];
   Sel_Calibrator(TTree * /*tree*/ =0) : fChain(0) { }

   Int_t ReadCal(TFile *r) ;
   void SetCal(PixelCal *pixcal){ fCrysCal = pixcal; }
   Double_t FineCalc(Double_t uv, Float_t u_cent, Float_t v_cent);
 
   virtual ~Sel_Calibrator() { }
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


   ClassDef(Sel_Calibrator,1)

};


#endif

#ifdef Sel_Calibrator_cxx
void Sel_Calibrator::Init(TTree *tree)
{
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
   fChain->SetBranchAddress("a", &a, &b_eventdata_a);
   fChain->SetBranchAddress("b", &b, &b_eventdata_b);
   fChain->SetBranchAddress("c", &c, &b_eventdata_c);
   fChain->SetBranchAddress("d", &d, &b_eventdata_d);
   fChain->SetBranchAddress("id", &id, &b_eventdata_id);




     }

Bool_t Sel_Calibrator::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

#endif // #ifdef Sel_Calibrator_cxx
