/*************

 ************/
#include "TROOT.h"
#include "Riostream.h"
#include "TTree.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TSpectrum.h"
#include "TSpectrum2.h"
#include "TH2F.h"
#include "TVector.h"
#include "TMath.h"
#include "TF1.h"
#include "Apd_Fit.h"
#include "./decoder.h"
#include "Apd_Peaktovalley.h"
#include "./ModuleCal.h"
#include "PixelCal.h"

int main(int argc, Char_t *argv[])
{


	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	Char_t		filenamel[FILENAMELENGTH] = "", curoutfile[FILENAMELENGTH];
	Int_t		verbose = 0;
	Int_t		ix;
        Int_t           cc=-99;
        ModuleCal       *event=0;
        Bool_t          filenamespec;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

        
    
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
				sprintf(filenamel, "%s", argv[ix + 1]);
				filenamespec=true;
			}
			else {
				cout << "Filename " << argv[ix + 1] << " too long !" << endl;
				cout << "Exiting.." << endl;
				return -99;
			}
		}

		if(strncmp(argv[ix], "-c", 2) == 0) {
                  ix++ ;
		  cc=atoi(argv[ix]);
			  cout << " Analyzing cartridge " << cc << endl;
		}

		

	}

       cout << "Welcome to FOM ana." ; if (verbose) cout << endl;

	if (!filenamespec) { 
             cout << " Please Specify filename:: fom_ana -f [filename] -c [cartridgeId] .\n Exiting. " << endl ;
	  return -2;}

        if (cc==-99) {
	  cout << " Please Specify Cartridge nubmer:: fom_ana -f [filename] -c [cartridgeId].\n Exiting." << endl;
	  return -2;}

             TCanvas *c1;
             c1 = (TCanvas*)gROOT->GetListOfCanvases()->FindObject("c1");
             if (!c1) c1 = new TCanvas("c1","c1",10,10,1000,1000);



	     if (!verbose) gErrorIgnoreLevel=kError;

	     rootlogon(verbose);


        Double_t aa, bb;
        Char_t filebase[FILENAMELENGTH],rootfile[FILENAMELENGTH]; 
        Char_t tmpname[20];//,tmptitle[50];
        Int_t j,k,m,lines;
         Int_t ii;
        ifstream infile;
         TH1F *xhist[FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE][64];
         TF1 *xfits[FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE][64];

	 Char_t peaklocationfilename[FILENAMELENGTH],histname[40],histtitle[120];
        strncpy(filebase,filenamel,strlen(filenamel)-9);
        filebase[strlen(filenamel)-9]='\0';
       
        if (verbose){ 
	cout << " filename = " << filenamel << endl;
	cout << " filebase = " << filebase << endl;
        }

        TString calparfile;
        calparfile.Form("%s.par.root",filebase);
 
         TFile *calfile = new TFile(calparfile);

         PixelCal *CrysCal = new PixelCal("CrysCalPar");
         CrysCal = (PixelCal *) calfile->Get("CrysCalPar");


	//        strncpy(filebase,filename,strlen(filename)-17);
	//        filebase[strlen(filename)-17]='\0';
	//        sprintf(rootfile,"%s",filename);
	//        strcat(rootfile,".root");
              
	//	cout << "Rootfile to open :: " << rootfile << endl;

	 for (Short_t ff=0;ff<FINS_PER_CARTRIDGE;ff++){
	   for (m=0;m<MODULES_PER_FIN;m++){
             for (j=0;j<APDS_PER_MODULE;j++){
               for (k=0;k<64;k++){
		 sprintf(tmpname,"xhist[%d][%d][%d][%d]",ff,m,j,k);
                 xhist[ff][m][j][k] = new TH1F(tmpname,tmpname,NRPOSBINS, POSMIN, POSMAX);
                  }}}}

	 if (verbose) cout << " Opening file " << filenamel << endl;
        TFile *file_left = new TFile(filenamel,"OPEN");

	  // Open Calfile //
	//        strncpy(filebase,filenamel,strlen(filenamel)-13);
	//        filebase[strlen(filenamel)-13]='\0';
        sprintf(rootfile,"%s",filebase);
        strcat(rootfile,".fom.root");

	Char_t tmpstring[60];

        if (verbose) cout << " Opening file " << rootfile << " for writing " << endl;
        TFile *fomfile = new TFile(rootfile,"UPDATE");
/*        sprintf(tmpstring,"C%d",cc);
	TDirectory *c_subdir = (TDirectory *) fomfile->Get(tmpstring);
        if (c_subdir) c_subdir->rmdir(tmpstring);
        if (!(c_subdir)) fomfile->mkdir(tmpstring);
        c_subdir->cd();
*/
        TTree *block;
        Char_t treename[40];

         sprintf(treename,"CalTree");
         block = (TTree *) file_left->Get(treename);
         if (!block) {
           cout << " Problem reading Tree " << treename  << " from file " << filenamel << endl;
           cout << " Exiting " << endl;
           return -10;}
         //      entries=block->GetEntries();

         cout << " Entries : " << block->GetEntries() << endl;
        
	 block->SetBranchAddress("Calibrated Event Data",&event);
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
          block->SetBranchAddress("ft",&event.ft);
          block->SetBranchAddress("id",&event.id);
          block->SetBranchAddress("pos",&event.pos);
	 */


   
        // Create Tree //

       Int_t entries_ch1_l = block->GetEntries();

       if (verbose) cout << " Looping over " << entries_ch1_l << " entries " <<endl;

       //       entries_ch1_l/=100;

       for (ii=0;ii<entries_ch1_l;ii++){
       //       for (ii=0;ii<1;ii++){
	    block->GetEntry(ii);
       //       cout << "UL2.mod = " << UL2.mod << "; UL2.x = "<<  UL2.x << "; UL2.id = ";
       //       cout << UL2.id << "; UL2.Ecal = " << UL2.Ecal << endl;
       
       if ((event->id>=0)&&(event->id<64)) {
         if ((event->Ecal>EGATEMIN)&&(event->Ecal<EGATEMAX)) {
	   if ((event->apd==1)||(event->apd==0)){
             Short_t ff=event->fin;
             if ((ff>=0)&&(ff<FINS_PER_CARTRIDGE)) {
               if ((event->m<16)&&(event->m>=0)) {
		 //		 cout << " m = " << m << ", event->m=" << event->m << ", event->apd=" << event->apd << ", event->id=" << event->id << endl;
		 xhist[ff][event->m][event->apd][event->id]->Fill(event->x); 
               }}}}}
    
       
       } // for loop

	if (verbose) cout << " Looping Done. " << endl;
	//	} // loop over chips



	Double_t *FOM[FINS_PER_CARTRIDGE][MODULES_PER_FIN][2][8];
        TH1F *xsums[FINS_PER_CARTRIDGE][MODULES_PER_FIN][2][8];

	for (Short_t ff=0;ff<FINS_PER_CARTRIDGE;ff++){
	sprintf(tmpstring,"C%dF%d",cc,ff);
	TDirectory *f_subdir = (TDirectory *) fomfile->Get(tmpstring);
        if (f_subdir) { f_subdir->rmdir(tmpstring); }
        if (!(f_subdir)) f_subdir = fomfile->mkdir(tmpstring);
        f_subdir->cd();
	if( verbose){ gDirectory->pwd();}
//        fomfile->ls();
	 for (m=0;m<MODULES_PER_FIN;m++){
           for (j=0;j<2;j++){
   	       if (CrysCal->validpeaks[cc][ff][m][j]){
	       sprintf(tmpstring,"M%dA%d",m,j);
	       TDirectory *m_subdir = (TDirectory *) f_subdir->Get(tmpstring);
	       if (m_subdir) {  m_subdir->rmdir(tmpstring); }
	       if (!(m_subdir)) m_subdir = f_subdir->mkdir(tmpstring);
	       if( verbose){ gDirectory->pwd();}
	       m_subdir->cd();

               if (verbose) {cout << " ========  F"<<ff<<"M"<<m<<"A"<<j<<" ========= " << endl;}
               fitpos(xhist[ff][m][j],xfits[ff][m][j],0);
               for (k=0;k<64;k++){
	   	 sprintf(histname, "xfits[%d][%d][%d][%d]", ff,m,j,k);
                 xfits[ff][m][j][k]->SetName(histname);
                 xhist[ff][m][j][k]->Write();
	       }
	       //	     }
	       //	   }
	       //	 }
               for( k = 0; k < 8; k++) {
		 sprintf(histname, "xsums[%d][%d][%d][%d]", ff, m,j,k);
                 sprintf(histtitle, "Peak to valley row %d Cartridge %d Fin %d module %d apd %d ", k,cc,ff,m,j);
                 xsums[ff][m][j][k] = new TH1F(histname, histtitle, NRPOSBINS, POSMIN, POSMAX);
		 for(Int_t ll = 0; ll < 8; ll++) {
		  //                        xsums[m][i][j][k]->Add(xhist[m][i][j][k * 8 + ll], 1);
                        xsums[ff][m][j][k]->Add(xhist[ff][m][j][k + ll*8], 1);
			//			cout << " m = " << m << " i= " << i << " j= " << j << " k= " << k*8+ll << endl;
		 }
		
	       }
	       FOM[ff][m][j][0] = ptv_ana(xsums[ff][m][j][0], xfits[ff][m][j],c1, 0, 0, verbose);
	       sprintf(curoutfile, "%s.C%dF%dM%dA%d_FOM.ps(", filebase, cc,ff,m,j);

        /*
         * c1->Print("042109_409-1-17-A_FOM.ps(");
         */
	//	  xsums[m][i][j][0]->Draw();
	       c1->Print(curoutfile);

	       sprintf(curoutfile, "%s.C%dF%dM%dA%d_FOM.ps", filebase, cc,ff,m,j);
	//	sprintf(curoutfile, "%s_FOM.ps", filebase);
	       for( k = 1; k < 7; k++) {
		 FOM[ff][m][j][k] = ptv_ana(xsums[ff][m][j][k], xfits[ff][m][j], c1, k, 0, verbose);
	  //xsums[m][i][j][k]->Draw();
		 c1->Print(curoutfile);
	       }

	       FOM[ff][m][j][7] = ptv_ana(xsums[ff][m][j][7],  xfits[ff][m][j], c1, 7, 0, verbose);
	       for( k = 0; k < 8; k++) { xsums[ff][m][j][k]->Write();}
	       sprintf(curoutfile, "%s.C%dF%dM%dA%d_FOM.ps)",filebase, cc,ff,m,j);
	//        sprintf(curoutfile, "%s_FOM.ps)", filebase);
	//        xsums[m][i][j][7]->Draw();
	       c1->Print(curoutfile);


	       f_subdir->cd();
	     } // validpeaks
	   } //j 
	 } //m 
	 fomfile->cd();
	} //ff

	 ofstream fomout;
         Char_t fomfilename[MAXFILELENGTH];
         sprintf(fomfilename,"%s.fom.txt",filebase);  
	 fomout.open(fomfilename);

Double_t FOM_CTR_AV ; Double_t FOM_TOP_AV ; Double_t EDGE_FOM_CTR_AV ; Double_t EDGE_FOM_TOP_AV ;
Double_t FOM_CTR_AV_E=0 ; Double_t FOM_TOP_AV_E=0 ; Double_t EDGE_FOM_CTR_AV_E=0 ; Double_t EDGE_FOM_TOP_AV_E=0 ;
	 for (Short_t ff=0;ff<FINS_PER_CARTRIDGE;ff++){
    

	 fomout << "==============================================================" <<endl;
	 fomout << "== FOM DATA FIN " << ff  <<endl;
	 fomout << "==============================================================" <<endl;
         fomout << "      |    FOM_center   |  FOM_top   | FOM_center_edge  |FOM_top_edge|" << endl;


	   for (m=0;m<MODULES_PER_FIN;m++){
             for (j=0;j<2;j++){
       	       if (CrysCal->validpeaks[cc][ff][m][j]){



	       if (FOM[ff][m][j][3][2] && FOM[ff][m][j][4][2]) {
    FOM_CTR_AV = ( FOM[ff][m][j][3][0]/FOM[ff][m][j][3][2] + FOM[ff][m][j][4][0]/FOM[ff][m][j][4][2]);
    FOM_CTR_AV_E = TMath::Sqrt( TMath::Power(errorprop_divide(FOM[ff][m][j][3][0],FOM[ff][m][j][3][1],FOM[ff][m][j][3][2],FOM[ff][m][j][3][3]),2) +
                                TMath::Power(errorprop_divide(FOM[ff][m][j][4][0],FOM[ff][m][j][4][1],FOM[ff][m][j][4][2],FOM[ff][m][j][4][3]),2))/2.;}
    else FOM_CTR_AV = 0;
    FOM_CTR_AV/=2;


    if (FOM[ff][m][j][0][2] && FOM[ff][m][j][7][2])  {
    FOM_TOP_AV= ( FOM[ff][m][j][0][0]/FOM[ff][m][j][0][2] + FOM[ff][m][j][7][0]/FOM[ff][m][j][7][2]);
    FOM_TOP_AV_E = TMath::Sqrt( TMath::Power(errorprop_divide(FOM[ff][m][j][0][0],FOM[ff][m][j][0][1],FOM[ff][m][j][0][2],FOM[ff][m][j][0][3]),2) +
                                TMath::Power(errorprop_divide(FOM[ff][m][j][7][0],FOM[ff][m][j][7][1],FOM[ff][m][j][7][2],FOM[ff][m][j][7][3]),2))/2.;}
    else FOM_TOP_AV = 0;
    FOM_TOP_AV/=2;


    if (FOM[ff][m][j][3][6] && FOM[ff][m][j][4][6]) {
    EDGE_FOM_CTR_AV = ( FOM[ff][m][j][3][4]/FOM[ff][m][j][3][6] + FOM[ff][m][j][4][4]/FOM[ff][m][j][4][6]);
    EDGE_FOM_CTR_AV_E = TMath::Sqrt( TMath::Power(errorprop_divide(FOM[ff][m][j][3][4],FOM[ff][m][j][3][5],FOM[ff][m][j][3][6],FOM[ff][m][j][3][7]),2) +
				     TMath::Power(errorprop_divide(FOM[ff][m][j][4][4],FOM[ff][m][j][4][5],FOM[ff][m][j][4][6],FOM[ff][m][j][4][7]),2))/2.;}

    else EDGE_FOM_CTR_AV = 0;
    EDGE_FOM_CTR_AV/=2;


    if (FOM[ff][m][j][0][2] && FOM[ff][m][j][7][2]) {
  EDGE_FOM_TOP_AV = ( FOM[ff][m][j][0][4]/FOM[ff][m][j][0][6] + FOM[ff][m][j][7][4]/FOM[ff][m][j][7][6]);
  EDGE_FOM_TOP_AV_E = TMath::Sqrt( TMath::Power(errorprop_divide(FOM[ff][m][j][0][4],FOM[ff][m][j][0][5],FOM[ff][m][j][0][6],FOM[ff][m][j][0][7]),2) +
				   TMath::Power(errorprop_divide(FOM[ff][m][j][7][4],FOM[ff][m][j][7][5],FOM[ff][m][j][7][6],FOM[ff][m][j][7][7]),2))/2.;}

 else EDGE_FOM_TOP_AV = 0;
 EDGE_FOM_TOP_AV/=2;

 Char_t pm;
 pm=126;
    fomout.precision(3);
    fomout << fixed;
    //    fomout << setw(5);
    //    fomout << setfixed(5);
    fomout << " F" << ff << "M" << m << "A" << j << ": ";
    fomout <<  FOM_CTR_AV << " " << pm << " " << FOM_CTR_AV_E << " | " ;
    fomout <<  FOM_TOP_AV << " " << pm << " " << FOM_TOP_AV_E << " | " ;
    fomout <<  EDGE_FOM_CTR_AV << " " << pm << " " << EDGE_FOM_CTR_AV_E << " | " ;
    fomout <<  EDGE_FOM_TOP_AV << " " << pm << " " << EDGE_FOM_TOP_AV_E << " | " ;
    fomout << endl;


	       }// validpeaks
	     }
	   }
	 } // loop ff
	 //#endif
	 fomout.close();
       fomfile->Close();



       return 0;}

