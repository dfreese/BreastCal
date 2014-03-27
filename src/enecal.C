/*

This program fills the energy histograms for every crystal, needed to do energy calibration. If we don't have a valid segmnetation, the crystal id remains -1.

 A new file is created, this is a little redundant. Crystal id's are assigned.

 */


#include "enecal.h" 
#include "time.h"
//#include "Sel_Calibrator.h"
//#include "TProof.h"
//#include "TChain.h"
//void usage(void);

//void usage(void){
// cout << " Parsetomodule -f [filename] [-p [pedfile] -v -o [outputfilename]] -n [nrfiles in loop] -t [threshold]" <<endl;
//  return;}


int main(int argc, Char_t *argv[])
{
    cout << " Welcome to EneCal. Performs Crystal Binning." ; 

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	Char_t		filename[FILENAMELENGTH] = "";
	Char_t		tmpstring[FILENAMELENGTH] = "";
	Int_t		verbose = 0;
	Int_t		ix;
        Bool_t          fileset=0;
        Bool_t          floodlut=kFALSE;
	Bool_t          fitonly=kFALSE;
        ModuleDat *event = 0;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	for(ix = 1; ix < argc; ix++) {

		/*
		 * Verbose '-v'
		 */
		if(strncmp(argv[ix], "-v", 2) == 0) {
			cout << "Verbose Mode " << endl;
			verbose = 1;
		}


		if(strncmp(argv[ix], "-fit", 4) == 0) {
			cout << "Only Perform Fitting " << endl;
			fitonly = 1;
			ix++;
		}


		/*
		 * Verbose '-map'
		 */
		if(strncmp(argv[ix], "-map", 4) == 0) {
			cout << "Using Flood Map " << endl;
			floodlut = kTRUE;
		}

		/* filename '-f' */
		if(strncmp(argv[ix], "-f", 2) == 0) {
			if(strlen(argv[ix + 1]) < FILENAMELENGTH) {
				sprintf(filename, "%s", argv[ix + 1]);
				fileset=1;
			}
			else {
				cout << "Filename " << argv[ix + 1] << " too long !" << endl;
				cout << "Exiting.." << endl;
				return -99;
			}
		}
	}

        rootlogon(verbose);
        time_t starttime = time(NULL);


	if (!fileset) { cout << " Please specify Filename. Exiting. " << endl; return -99;}

        cout << " Inputfile :: " << filename << endl;

	TFile *rfile = new TFile(filename,"UPDATE");
       
        Char_t filebase[FILENAMELENGTH],newrootfile[FILENAMELENGTH]; 
        Char_t tmpname[50],tmptitle[50];
        Int_t i,j,k,m,lines;
        Double_t aa, bb;
        ifstream infile;


        TVector* ppVals = (TVector *) rfile->Get("pp_spat");
        TVector* ppVals_com = (TVector *) rfile->Get("pp_com");

	//        TVector* uu_c = (TVector *) rfile->Get("uu_c");
	//        TVector* vv_c = (TVector *) rfile->Get("vv_c");

	/*
        if (!((ppVals) && (ppVals_com) && (uu_c) && (vv_c))){
	    cout << " Problem reading ppVals and/or uu/cc circle centers.\nExiting. " << endl;
            return -3;
	  }
	*/

	//        TCanvas *c1 =new TCanvas();
        Char_t treename[40];
        TDirectory *subdir[RENACHIPS];

        strncpy(filebase,filename,strlen(filename)-5);
        filebase[strlen(filename)-5]='\0';
        if (verbose) cout << " filebase = " << filebase << endl;


	/*
      if (verbose) { 
        cout << " Circle centers :: " << endl;
     
       for (int kk=0;kk<RENACHIPS;kk++){
         cout << " Chip " << kk << " : " ;
           for (i=0;i<4;i++){
	     cout << " M" << i << ":: " ;
              for (j=0;j<2;j++){
		cout << "A" << j << "- " ;
                cout << (*uu_c)(kk+i*RENACHIPS+j*MODULES*RENACHIPS) << " " << (*vv_c)(kk+i*RENACHIPS+j*MODULES*RENACHIPS) << " ";
	      }
	   }
	   cout << endl;
       }
      } // verbose
	*/

      //	TChain *block;
      //  TProof::Open("");

	   //          block = new TChain("mdata","Chain of 1");
	   //          block->Add(filename);
	   //          block->SetProof();  




	/*
	if (verbose){
	  for (m=0;m<RENACHIPS;m++){
        for (j=0;j<4;j++){
	  for (i=0;i<2;i++){
            cout << " m = " << m << " i = " << i << " j = " << j << " : " <<validpeaks[m][j][i] <<endl;
	  }}
	}
	}
	*/



	//	cout << " U_x[0][0][21] ="<< U_x[0][0][21] << " U_y[0][0][21] = " << U_y[0][0][21]<<endl;

	
      /*
       TFile *f;
       sprintf(newrootfile,"%s.enecal",filebase);
       strcat(newrootfile,".root");
       if (verbose) {
	 cout << "Creating New Root file : " << newrootfile << endl;}
       f = new TFile(newrootfile,"RECREATE");

*/
	/* loop over the chips !! */

	//	for (m=0;m<RENACHIPS;m++){

       /* Creating histograms */


	  /*

	  for (j=0;j<4;j++){
            for (i=0;i<2;i++){
	      for (k=0;k<64;k++){
                sprintf(tmpname,"Ehist[%d][%d][%d][%d]",m,j,i,k);
                sprintf(tmptitle,"RENA %d Unit %d Module %d Pixel %d",m,j,i,k);
                Ehist[m][j][i][k] = new TH1F(tmpname,tmptitle,Ebins_pixel,E_low,E_up);
                sprintf(tmpname,"Ehist_com[%d][%d][%d][%d]",m,j,i,k);
                sprintf(tmptitle,"RENA %d Unit %d Module %d Pixel %d Common",m,j,i,k);
                Ehist_com[m][j][i][k] = new TH1F(tmpname,tmptitle,Ebins_com_pixel,E_low_com,E_up_com);
		//                sprintf(tmpname,"Efits[%d][%d][%d]",i,j,k);
		//                sprintf(tmptitle,"Unit %d Module %d Pixel %d",i,j,k);
		//                Efits[i][j][k] = new TF1(tmpname,"gaus",EBINS,EMIN,EMAX);
	      }
            }
          }
	} // m

	  */
	  /*
	  if (m==0)  block = (TTree *) rfile->Get("block1");
          else  block = (TTree *) rfile->Get("block2");
	  */

	  sprintf(treename,"mdata");
           TChain *block;
         block = (TChain *) rfile->Get(treename);

         if (!block) {
	   cout << " Problem reading Tree " << treename  << " from file " << filename << endl;
           cout << " Exiting " << endl;
           return -10;}
	 //	 entries=block->GetEntries();


         cout << " Read block from file :: " << time(NULL)-starttime << endl;
         cout << " Entries in block :: " << block->GetEntries() << endl;
         PixelCal *CrysCal = new PixelCal("CrysCalPar");
         CrysCal->SetVerbose(verbose);

         CrysCal->ReadCal(filebase);
         cout << " CrysCal->X[0][0][2][1][0] = " << CrysCal->X[0][0][2][1][0] << endl;
  cout << " fCrysCal->X[0][6][8][0][0] = " << CrysCal->X[0][6][8][0][0] << endl;
       Sel_GetEhis *m_getEhis = new Sel_GetEhis();

       cout << "FYI:: Size of Sel_GetEhis :: " << sizeof(Sel_GetEhis) << endl;

       CrysCal->Print();
 
       m_getEhis->SetFileBase(filebase);


       TFile *rfi;

       if (!fitonly){

#define USEPROOF

#ifdef USEPROOF      
       //  TProof *proof = new TProof("proof");
       //   proof->Open("");
	 	 TProof *p = TProof::Open("workers=1");
	 //	 TProof *p = TProof::Open("");
       //       gProof->UploadPackage("/home/miil/MODULE_ANA/ANA_V5/SpeedUp/PAR/ModuleDatDict.par");
       //       gProof->EnablePackage("ModuleDatDict");

         cout << " Proof open :: " << time(NULL)-starttime << endl;

//#define USEPAR


             char *libpath = getenv("CURDIR");
	     cout << " Loading Shared Library from " << libpath << endl;
	       cout << " (note CURDIR = " << getenv("CURDIR") << " )" << endl;
	     TString exestring;

//FIXME NOT SURE WHY IT DOESN'T WORK WHEN USEPAR IS NOT DEFINED ... 
#define USEPAR
     
#ifdef USEPAR

       /* This is an example of the method to use PAR files  -- will need to use an environment var here to make it location independent */

	     //	     exestring.Form("gSystem->Load(\"%slibModuleAna.so\")","/home/miil/MODULE_ANA/ANA_V5/SpeedUp/lib/");
	     exestring.Form("%s/PAR/Sel_GetEhis.par",libpath);
             
             p->UploadPackage(exestring.Data());
             p->EnablePackage("Sel_GetEhis");

#else
       /* Loading the shared library */
             exestring.Form("gSystem->Load(\"%s/lib/libModuleAna.so\")",libpath);
	     p->Exec(exestring.Data());

	     //   return -1;

#endif


	           p->AddInput(CrysCal);

       block->SetProof();
//       m_getEhis->LS();
         cout << " Proof ready to process :: " << time(NULL)-starttime << endl;
        block->Process(m_getEhis);
        cout << " Proof processed :: " << time(NULL)-starttime << endl;
       m_getEhis->SetPixelCal(CrysCal);

#else
       //      block->Process("Sel_GetFloods.cc+");
	//     m_getEhis->SetPixelCal(CrysCal);
      block->Process(m_getEhis);
#endif

         PPeaks *thesePPeaks = (PPeaks *) rfile->Get("PhotoPeaks");
	 if (!(thesePPeaks)) {
	   cout << " Warning :: Couldn't read object PhotoPeaks from file " << rfile->GetName() << endl;
	   cout << " rfile->ls() :: "<< endl;
	   rfile->ls();
           exit(-1);
	 }

	 m_getEhis->SetPPeaks(thesePPeaks);

        cout << " Before WriteHist :: " << time(NULL)-starttime << endl;
        TString calparfile;
        calparfile.Form("%s.par.root",filebase);
 	rfi = new TFile(calparfile,"RECREATE");
        m_getEhis->WriteHists(rfi);
        cout << " After WriteHist :: " << time(NULL)-starttime << endl;


	 }


          else { m_getEhis->LoadEHis(rfile); m_getEhis->SetPixelCal(CrysCal);}

    
       // m_getEhis->FitApdEhis(0,0,0,0);

       cout << " writing CrysCal :: " << time(NULL)-starttime << endl;
       CrysCal = m_getEhis->GetPixelCal();

       cout << "  CrysCal->GainSpat[0][0][2][1][0] = " <<   CrysCal->GainSpat[0][0][2][1][0] <<endl;

       CrysCal->WriteCalTxt(filebase);
       CrysCal->Write(); 

      /*
	  f=calblock->GetCurrentFile();
        f->Write();     
      */

      /*
       TFile *fhi;
       sprintf(newrootfile,"%s.enecal.hist",filebase);
       strcat(newrootfile,".root");
       if (verbose) {
	 cout << "Creating New Root file : " << newrootfile << endl;}
       fhi = new TFile(newrootfile,"RECREATE");

	  fhi->cd();
          ppVals->Write("pp_spat");
          ppVals_com->Write("pp_com");
	  //    calblock->AutoSave();
	  //  
	  for (m=0;m<RENACHIPS;m++){
           sprintf(tmpname,"RENA%d",m);
           subdir[m] = fhi->mkdir(tmpname);
           subdir[m]->cd();
	  for (j=0;j<2;j++){
            for (i=0;i<4;i++){
	      for (k=0;k<64;k++){
                Ehist[m][i][j][k]->Write();
                Ehist_com[m][i][j][k]->Write();
	      }
            }
          }
	} // loop over m

	//	Ehist[1][0][0][34]->Draw(); c1->Print("test.png");
 
          fhi->Close();
        
      */ 
       cout << " closing file  rfi :: " << time(NULL)-starttime << endl;
       rfi->Close();
       cout << " closing file  rfile :: " << time(NULL)-starttime << endl;
	rfile->Close();
       cout << " done :: " << time(NULL)-starttime << endl;
	
	return 0;}

