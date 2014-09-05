#define GAIN_SPAT 0x3;
#define GAIN_COM  0xC;
#define ERES_SPAT 0x300;
#define ERES_COM  0xC00;
#define FOM_CTR   0x30;
#define FOM_TOP   0xC0;
#define RATIO     0x3000;

#define GAIN_SPAT_INT 0x2;
#define GAIN_COM_INT  0x8;
#define ERES_SPAT_INT 0x200;
#define ERES_COM_INT  0x800;
#define FOM_CTR_INT   0x20;
#define FOM_TOP_INT   0x80;
#define RATIO_INT     0x2000;

#define EDGE_FOM_CTR  0xC000;
#define EDGE_FOM_TOP  0x30000;

#define EDGE_FOM_CTR_INT  0x8000;
#define EDGE_FOM_TOP_INT  0x20000;

#define ERES_COM_RMS  0x300000;
#define ERES_SPAT_RMS 0xC0000;
#define ERES_SPAT_RMS_INT 0x80000;
#define ERES_COM_RMS_INT  0x200000;

#define FLOOD_UNI_INT 0x800000;
#define FLOOD_UNI 0xC00000;

//#define PNGFILE 0

#include "TROOT.h"
#include "TCanvas.h"
#include "Riostream.h"
#include "Syspardef.h"
#include "stdlib.h"
#include "myrootlib.h"
//#include "coinc_ana.h"
//#include "apd_fit.h"
//#include "apd_peaktovalley.h"
#include "Apd_Fit.h"
#include "PixelCal.h"
//#include "apd_tac_ana.h"
#include "TFile.h"
#include "libInit_avdb.h"
#include "TMinuit.h"
#include "TStyle.h"
#include "TColor.h"
#include "TVector.h"
#include "TPaveText.h"

#define MODULESTRINGLENGTH 20


Int_t rmd_ana(Char_t[], Int_t, Int_t, Int_t);
Int_t GiveCol(Float_t,Float_t,Float_t,Int_t * );
Double_t *ptv_ana(TH1F *hist, TF1 *posgausfits[64], TCanvas *c1, Int_t nr, Int_t x,Int_t verbose);
/*
   =======================================================================================================================
   =======================================================================================================================
   */

void Usage(void){
    cout << " Please use :: modana -f [filename] -C [cartridgeId] -F [finId] -M [moduleId] -A [apdId] [ -t [modulename] -v ] " << endl;
}

Int_t rangecheck(Int_t val, Int_t min, Int_t up, TString valname){
    if (( val >= min ) && ( val < up) ) return 0;
    else { 
        cout << " Error ! " <<  valname  << " needs to be < " << up ;
        cout << " and >= " << min << ". You specified " << valname <<"  = " << val << endl;
        Usage();
        return -1;}
}


Int_t main(int argc, Char_t *argv[])
{
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    Char_t	modulestring[MODULESTRINGLENGTH] = "XXX-XX-XX";
    Char_t modulestringcfma[MODULESTRINGLENGTH+10];
    Char_t          filename[MAXFILELENGTH];
    Int_t	verbose = 0;
    Int_t	ix;
    Int_t	tac = 0;
    Int_t pleak = 0;
    Int_t cc=99;
    Int_t ff=99;
    Int_t mm=99;
    Int_t aa=99;
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    cout << "Welcome " << endl;

    /*~~~~~~~~~~~~~~*/
    /*
     * TNtuple *datantuple;
     * TH2F *datahist;
     */
    ifstream	files;
    /*~~~~~~~~~~~~~~*/

    /* Reading input arguments */
    for(ix = 1; ix < argc; ix++) {

        /*
         * cout << argv[ix] << endl;
         * ;
         * Verbose '-v'
         */
        if (strncmp(argv[ix], "-t", 2) == 0 ){
            if(strlen(argv[ix + 1]) < MODULESTRINGLENGTH) {
                sprintf(modulestring, "%s", argv[ix + 1]);
            }
            else {
                cout << "module name " << argv[ix + 1] << " too long !" << endl;
                cout << "Exiting.." << endl;
                return -99;
            }
            ix++;
        }


        if(strncmp(argv[ix], "-v", 2) == 0) {
            cout << "Verbose Mode " << endl;
            verbose = 1;
        }


        if(strncmp(argv[ix], "-C", 2) == 0) {
            ix++;
            cc = atoi(argv[ix]);
        }

        if(strncmp(argv[ix], "-F", 2) == 0) {
            ix++;
            ff = atoi(argv[ix]);
        }

        if(strncmp(argv[ix], "-M", 2) == 0) {
            ix++;
            mm = atoi(argv[ix]);
        }

        if(strncmp(argv[ix], "-A", 2) == 0) {
            ix++;
            aa = atoi(argv[ix]);
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
            ix++;
        }
    }

    /*
     * TMinuit* minu = new TMinuit();
     * minu->SetPrintLevel(-1);
     */
    rootlogon(verbose);
    set2dcolor(4);

    if (!(verbose)) { gErrorIgnoreLevel=kError;}


    if ( 
            rangecheck(cc,0,CARTRIDGES_PER_PANEL,"cartridgeId") ||
            rangecheck(ff,0,FINS_PER_CARTRIDGE,"finId") ||
            rangecheck(mm,0,MODULES_PER_FIN,"moduleId") ||
            rangecheck(aa,0,APDS_PER_MODULE,"apdId") )
    {return -2;}


    sprintf(modulestringcfma,"C%dF%dM%dA%d_%s",cc,ff,mm,aa,modulestring);

    TFile *rootfile;  

    TH1F *Spatials[64];
    TH1F *Commons[64];
    TF1 *SpatFit[64];
    TF1 *ComFit[64];
    TGraphErrors *SpatMean = new TGraphErrors(64);
    TGraphErrors *SpatEres = new TGraphErrors(64);
    TGraphErrors *ComMean = new TGraphErrors(64);
    TGraphErrors *ComEres = new TGraphErrors(64);

    Double_t mean, mean_spat, mean_e, eres, eres_e;

    TString HistString;

    TCanvas		*c1, *c2;
    /*~~~~~~~~~~~~~~~~~~~~~~~~~*/

    c1 = (TCanvas *) gROOT->FindObject("c1");
    if(!c1)
        c1 = new TCanvas("c1", "c1", 10, 10, 1000, 900);
    else
        c1->Clear();

    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    TH1F		*xpos[64];
    TF1             *xfits[64];
    TH1F		*xsums[8];
    Double_t	vals[4];
    Double_t	pixvals[64][9];
    Double_t	com_vals[4];
    Double_t	com_pixvals[64][9];
    Double_t	tac_vals[4];
    Double_t	tac_fitvals[64][5];
    Char_t		basestring[MAXFILELENGTH];
    Char_t		curoutfile[MAXFILELENGTH + 10];
    Char_t          histtitle[20];

    Char_t          parfilename[MAXFILELENGTH];
    Char_t          fomfilename[MAXFILELENGTH];
    Char_t          globrootfilename[MAXFILELENGTH];
    Char_t       tmpstring[100];
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    strncpy(basestring, filename, strlen(filename) - 5);
    basestring[strlen(filename) - 5] = '\0';
    cout << " BASE STRING :: " << basestring <<endl;

    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    /*
     * cout << filename << endl;
     * cout << strlen(filename) << endl;
     * cout << basestring << endl;
     */
    TFile	*f = new TFile(filename,"READ");

    if (f->IsZombie()) { cout << " Error opening file : " << filename << "\nExiting" << endl; return -2;}

    sprintf(parfilename,"%s.par.root",basestring);
    sprintf(fomfilename,"%s.fom.root",basestring);
    sprintf(globrootfilename,"%s.glob.root",basestring);

    TFile *parfile = new TFile(parfilename);

    if (parfile->IsZombie()) { cout << " Error opening file : " << parfilename << "\nExiting" << endl; return -2;}

    TFile *fomfile = new TFile(fomfilename);
    if (fomfile->IsZombie()) { cout << " Error opening file : " << fomfilename << ".\nExiting" << endl; return -2;}

    TFile *globrootfile = new TFile(globrootfilename);
    if (globrootfile->IsZombie()) { cout << " Error opening file : " << globrootfilename << ".\nExiting" << endl; return -2;}



    TH1F *ratio = new TH1F("ratio","",100,0,5);
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    //       	f->ls();

    if (verbose) {cout << "Obtaining calibration parameters " <<endl;}

    PixelCal *CrysCal = new PixelCal("CrysCalPar");
    CrysCal->SetVerbose(verbose);

    CrysCal->ReadCal(basestring);

    if (!CrysCal->validpeaks[cc][ff][mm][aa]){
        cout << " No Valid Peaks ! " << endl;
        if (verbose) {cout << "Obtaining flood histogram " <<endl;}
        sprintf(tmpstring,"C%dF%d/flood_C%dF%dM%dA%d",cc,ff,cc,ff,mm,aa);
        TH2F	*back2 = (TH2F *) f->Get(tmpstring);

        TCanvas *c3 = new TCanvas("c3",tmpstring,500,500);
        back2->Draw("colz");
        sprintf(tmpstring,"%s_%s.flood.png",basestring,modulestringcfma);
        c3->Print(tmpstring);
        return -2;
    }


    if (verbose) {cout << "Obtaining pixel histograms " <<endl;}

    for (Int_t k=0;k<64;k++){
        HistString.Form("C%dF%d/M%d/A%d/Spatial/Ehist_C%dF%dM%dA%dP%d",cc,ff,mm,aa,cc,ff,mm,aa,k);
        Spatials[k] = (TH1F *) parfile->Get(HistString.Data());
        SpatFit[k] = (TF1 *) Spatials[k]->GetListOfFunctions()->FindObject("spatfit");
        if (SpatFit[k]){
            mean = SpatFit[k]->GetParameter(1);
            mean_e = SpatFit[k]->GetParError(1);
            if ( mean ){
                eres = 2.35*SpatFit[k]->GetParameter(2)/SpatFit[k]->GetParameter(1);
                eres_e = 2.35*errorprop_divide(SpatFit[k]->GetParameter(2),SpatFit[k]->GetParError(2),SpatFit[k]->GetParameter(1),SpatFit[k]->GetParError(1));
            }
        } else {
            mean = 0; mean_e =0; eres= .99; eres_e = 1;
        }
        mean_spat=mean;
        SpatMean->SetPoint(k,k+1,mean);
        SpatEres->SetPoint(k,k+1,eres);
        SpatMean->SetPointError(k,0,mean_e);
        SpatEres->SetPointError(k,0,eres_e);

        HistString.Form("C%dF%d/M%d/A%d/Common/Ehist_com_C%dF%dM%dA%dP%d",cc,ff,mm,aa,cc,ff,mm,aa,k);
        Commons[k] = (TH1F *) parfile->Get(HistString.Data());
        ComFit[k] = (TF1 *) Commons[k]->GetListOfFunctions()->FindObject("comfit");
        if (ComFit[k]){
            mean = ComFit[k]->GetParameter(1);
            mean_e = ComFit[k]->GetParError(1);
            if ( mean ){
                eres = 2.35*ComFit[k]->GetParameter(2)/ComFit[k]->GetParameter(1);
                eres_e = 2.35*errorprop_divide(ComFit[k]->GetParameter(2),ComFit[k]->GetParError(2),ComFit[k]->GetParameter(1),ComFit[k]->GetParError(1));
            }
        } else {
            mean = 0; mean_e =0; eres= .99; eres_e = 1;
        }
        ComMean->SetPoint(k,k+1,mean);
        ComEres->SetPoint(k,k+1,eres);
        ComMean->SetPointError(k,0,mean_e);
        ComEres->SetPointError(k,0,eres_e);

        //cout << k << " :: " << mean << " " << eres << endl;

        if (mean) ratio->Fill((Float_t) mean_spat/mean); 


    }  // loop over k

    if (verbose) {cout << "Obtaining FOM histograms " <<endl;}


    for(Int_t k = 0; k < 64; k++) {  
        //               TH1F *hi = (TH1F *) f->Get("C0F6/M8A0/xhist[6][8][0][3]")
        sprintf(tmpstring, "C%dF%d/M%dA%d/xhist[%d][%d][%d][%d]", cc,ff,mm,aa,ff,mm,aa,k);
        xpos[k] = (TH1F *) fomfile->Get(tmpstring);
        xfits[k] = (TF1 *) xpos[k]->GetListOfFunctions()->FindObject("inifit"); 
        if (!xfits[k]) {
            cout << " Couldn't extract xfits["<<k<<"]" << endl;  
            xfits[k] = new TF1("inifit","gaus"); xfits[k]->SetParameters(1,0,1);
        }
    }

    for(Int_t k=0 ;k<8;k++){
        sprintf(tmpstring, "C%dF%d/M%dA%d/xsums[%d][%d][%d][%d]", cc,ff,mm,aa,ff,mm,aa,k);
        xsums[k] = (TH1F *) fomfile->Get(tmpstring);
    }

    if (verbose) {
        cout << "Obtaining flood histogram " <<endl;
    }
    sprintf(tmpstring,"C%dF%d/flood_C%dF%dM%dA%d",cc,ff,cc,ff,mm,aa);
    TH2F	*back2 = (TH2F *) f->Get(tmpstring);

    // need to read in the peaklocationfilename to draw the segmentation::
    Char_t peaklocationfilename[FILENAMELENGTH];
    sprintf(peaklocationfilename,"./CHIPDATA/%s.C%dF%d.module%d_apd%d_peaks",basestring,cc,ff,mm,aa);
    strcat(peaklocationfilename,".txt");
    ifstream infile;
    infile.open(peaklocationfilename);
    Int_t lines = 0;
    Int_t k = 0;
    Float_t xpeak,ypeak;
    TGraph *seggraph = new TGraph(64);
    seggraph->SetName("seggraph");
    while (1){ 
      if (!infile.good()) break;
      infile >> k >>  xpeak >> ypeak;
      if (k < 64) { seggraph->SetPoint(k,xpeak,ypeak);}
       //       cout << k << ", " << U_x[i][j][k]<< ", " << U_y[i][j][k] << endl;
               lines++;      
                }
      if (verbose) cout << "Found " << lines-1 << " peaks in  file " << peaklocationfilename << endl;
      infile.close();


    TCanvas *c3 = new TCanvas("c3",tmpstring,500,500);
    back2->Draw("colz");
    seggraph->Draw("PL");
    sprintf(tmpstring,"%s_%s.flood.png",basestring,modulestringcfma);
    c3->Print(tmpstring);


    sprintf(curoutfile,"%s_%s_E_fits.spat",basestring,modulestringcfma);
    writ(Spatials, SpatFit, c1, curoutfile, 0, PNGFILE);
    if (verbose) {
        cout << " Just finished the Spatials,  " <<endl;
    }

    sprintf(curoutfile,"%s_%s_E_fits.com",basestring,modulestringcfma);
    writ(Commons, ComFit, c1, curoutfile, 1,PNGFILE);


    if(verbose) {
        cout << " Looking at global energy resolution :: " << endl;
    }

    TH1F *espec_glob_spat(0);
    TH1F *espec_glob_com(0);

    HistString.Form("C%dF%d/GlobEhist_C%dF%dM%dA%d",cc,ff,cc,ff,mm,aa);
    espec_glob_spat = (TH1F *) globrootfile->Get(HistString.Data());
    if (espec_glob_spat == 0) {
        cout << "Error: \"" << HistString.Data() << "\" not found. exiting.\n";
        exit(-1);
    }
    espec_glob_spat->SetTitle(modulestring);

    HistString.Form("C%dF%d/GlobEhist_com_C%dF%dM%dA%d",cc,ff,cc,ff,mm,aa);
    espec_glob_com = (TH1F *) globrootfile->Get(HistString.Data());
    if (espec_glob_com == 0) {
        cout << "Error: \"" << HistString.Data() << "\" not found. exiting.\n";
        exit(-1);
    }

    // Counting the number of hits in all xpos ::
    Int_t totalentries(0);
    Int_t totalentriessq(0);
    Double_t stdentries(0);
    Int_t goodpix(0);

    for (Int_t j=0;j<PEAKS;j++){
        totalentries += xpos[j]->GetEntries();
        totalentriessq += TMath::Power(xpos[j]->GetEntries(),2);
    }
    stdentries=TMath::Sqrt((Double_t)totalentriessq/64.- TMath::Power((Double_t)totalentries/64.,2));
    if (verbose){
        cout << "Note: Total entries = " << totalentries;
        cout << " ; average entries = " << totalentries/64.;
        cout << " ; std dev = " << stdentries << endl;
    }

    for (Int_t j=0; j<PEAKS; j++) {
        // 3.22 based on the Grubb test  http://en.wikipedia.org/wiki/Grubbs'_test_for_outliers 
        //  http://www.graphpad.com/quickcalcs/grubbs2.cfm
        if ( xpos[j]->GetEntries() < (totalentries/64.-3.22*stdentries) ) {
            cout << " PEAK " << j << " doesn't have enough hits; n = "  << xpos[j]->GetEntries();
            cout << " ( threshold = " << totalentries/64.-3.22*stdentries << ")\n"; 
            cout << " Common has " << Commons[j]->GetEntries() << " entries, Spatial has " ;
            cout << Spatials[j]->GetEntries() << endl;
        } else {
            goodpix++;
        }
    }


    if (verbose) cout << "Reading Global fits from file :: " << endl;

    TF1 *glob_spat_fit(0);
    TList * espec_glob_spat_func_list(0);
    espec_glob_spat_func_list = espec_glob_spat->GetListOfFunctions();
    Double_t *glob_spat_fit_par;
    Double_t *glob_spat_fit_par_err;
    Bool_t kFailedReadGlobFit=kFALSE;
    if (espec_glob_spat_func_list != 0) {
      glob_spat_fit = (TF1 *) espec_glob_spat_func_list->FindObject("fitfunc");
        if (glob_spat_fit != 0) {
	  glob_spat_fit->SetName("glob_spat_fit");
	  glob_spat_fit_par = glob_spat_fit->GetParameters();
	  glob_spat_fit_par_err = glob_spat_fit->GetParErrors();
	}
        else {
	  kFailedReadGlobFit = kTRUE;
	}
    } 
    else {
      kFailedReadGlobFit = kTRUE;
    }
    if (kFailedReadGlobFit){    
      glob_spat_fit_par = new Double_t[6];
      glob_spat_fit_par_err = new Double_t[6];
      for ( Int_t jj=0;jj<6;jj++){
        glob_spat_fit_par[jj] = 99+jj*0.1;
        glob_spat_fit_par_err[jj] = 1+jj*0.1;
      }
    }


    TF1 * glob_com_fit(0);
    TList * espec_glob_com_func_list(0);
    espec_glob_com_func_list = espec_glob_com->GetListOfFunctions();
    Double_t *glob_com_fit_par;
    Double_t *glob_com_fit_par_err;
    Bool_t kFailedReadGlobComFit=kFALSE;
    if (espec_glob_com_func_list != 0) {
      glob_com_fit = (TF1 *) espec_glob_com_func_list->FindObject("fitfunc");
        if (glob_com_fit != 0) {
	  glob_com_fit->SetName("glob_com_fit");
	  glob_com_fit_par = glob_com_fit->GetParameters();
	  glob_com_fit_par_err = glob_com_fit->GetParErrors();
	}
        else {
	  cout << " Failed 2nd attempt " << endl;
	  kFailedReadGlobComFit = kTRUE;
	}
    } 
    else {
	  cout << " Failed 1st attempt " << endl;
      kFailedReadGlobComFit = kTRUE;
    }
   if (kFailedReadGlobComFit){    
      glob_com_fit_par = new Double_t[6];
      glob_com_fit_par_err = new Double_t[6];
      for ( Int_t jj=0;jj<6;jj++){
        glob_com_fit_par[jj] = 59+jj*0.1;
        glob_com_fit_par_err[jj] = 1+jj*0.1;
      }
    }

    /*
    if (espec_glob_com_func_list == 0) {
        cout << "list of functions not found in espec_glob_com. Exiting" << endl;
        exit(-1);
    } else {
        glob_com_fit = (TF1 *) espec_glob_com_func_list->FindObject("fitfunc");
        if (glob_com_fit == 0) {
            cout << "fitfunc not found in espec_glob_com. Exiting" << endl;
            exit(-1);
        }
    }
    */
   /*
    glob_com_fit->SetName("glob_com_fit"); 
    Double_t *glob_com_fit_par = glob_com_fit->GetParameters();
    Double_t *glob_com_fit_par_err = glob_com_fit->GetParErrors();
   */

    Int_t score;
    if (verbose) {
        cout << "done with fitting\n";
    }

    sprintf(curoutfile, "%s_%s_glob_fits.ps", basestring,modulestringcfma);
    c1->Clear(); 
    c1->Divide(1,2);
    c1->cd(1); 
    espec_glob_spat->Draw();if (!kFailedReadGlobFit) glob_spat_fit->Draw("same");
    TPaveText *scorecard_glob_spat = new TPaveText(.65,.45,.85,.55,"NDC");
    scorecard_glob_spat->SetShadowColor(0);
    sprintf(tmpstring,"E_{res} = %.1f #pm %.1f", 100 * 2.35*glob_spat_fit_par[5] / glob_spat_fit_par[4] , 100 * 2.35* errorprop_divide(glob_spat_fit_par[5],glob_spat_fit_par_err[5],glob_spat_fit_par[4],glob_spat_fit_par_err[4]));
    scorecard_glob_spat->SetFillColor(GiveCol( (Float_t) 100*2.35*glob_spat_fit_par[5]/glob_spat_fit_par[4],12.,15.,&score));
    scorecard_glob_spat->AddText(tmpstring);
    scorecard_glob_spat->Draw();
    c1->cd(2);
    espec_glob_com->Draw();if (!kFailedReadGlobComFit) glob_com_fit->Draw("same");
    TPaveText *scorecard_glob_com = new TPaveText(.65,.45,.85,.55,"NDC");
    scorecard_glob_com->SetShadowColor(0);
    sprintf(tmpstring,"E_{res} = %.1f #pm %.1f", 100 * 2.35*glob_com_fit_par[5] / glob_com_fit_par[4] , 100 * 2.35* errorprop_divide(glob_com_fit_par[5],glob_com_fit_par_err[5],glob_com_fit_par[4],glob_com_fit_par_err[4]));
    scorecard_glob_com->SetFillColor(GiveCol( (Float_t) 100*2.35*glob_com_fit_par[5]/glob_com_fit_par[4],12.,15.,&score));
    scorecard_glob_com->AddText(tmpstring);
    scorecard_glob_com->Draw();


    c1->Print(curoutfile);
    sprintf(curoutfile, "%s_%s_glob_fits.C", basestring,modulestringcfma);
    c1->Print(curoutfile);
    sprintf(curoutfile, "%s_%s_glob_fits.png", basestring,modulestringcfma);
    c1->Print(curoutfile);
    c1->Update();
    sprintf(curoutfile, "%s_%s_xxx.png", basestring,modulestringcfma);        

    if(verbose) {
        cout << " Energy resolution analyzed, now looking at FOM " << endl;
    }

    Double_t	*FOM[8];

    sprintf(curoutfile, "%s_%s_FOM.ps", basestring,modulestringcfma);
    for(Int_t j = 0; j < 8; j++) {
        FOM[j] = ptv_ana(xsums[j], xfits, c1, j, 0, verbose);
        sprintf(curoutfile, "%s_%s_FOM_%d.png", basestring,modulestringcfma, j);
        c1->Print(curoutfile);
    }

    Double_t FOM_CTR_AV ;
    if (FOM[3][2] && FOM[4][2]) {
        FOM_CTR_AV = ( FOM[3][0]/FOM[3][2] + FOM[4][0]/FOM[4][2]);
    } else {
        FOM_CTR_AV = 0;
    }
    FOM_CTR_AV/=2;

    Double_t FOM_TOP_AV ;
    if (FOM[0][2] && FOM[7][2]) {
        FOM_TOP_AV= ( FOM[0][0]/FOM[0][2] + FOM[7][0]/FOM[7][2]);
    } else {
        FOM_TOP_AV = 0;
    }
    FOM_TOP_AV/=2;

    Double_t EDGE_FOM_CTR_AV ;
    if (FOM[3][6] && FOM[4][6]) {
        EDGE_FOM_CTR_AV = ( FOM[3][4]/FOM[3][6] + FOM[4][4]/FOM[4][6]);
    } else {
        EDGE_FOM_CTR_AV = 0;
    }
    EDGE_FOM_CTR_AV/=2;

    Double_t EDGE_FOM_TOP_AV ;
    if (FOM[0][2] && FOM[7][2]) {
        EDGE_FOM_TOP_AV= ( FOM[0][4]/FOM[0][6] + FOM[7][4]/FOM[7][6]);
    } else {
        EDGE_FOM_TOP_AV = 0;
    }
    EDGE_FOM_TOP_AV/=2;



    // New Code HERE (from Email) 

    sprintf(curoutfile,"%s_%s_FOM.txt",basestring,modulestringcfma);
    ofstream FOMfile;
    FOMfile.open(curoutfile);

    for (Int_t i=0;i<8;i++){
        FOMfile << FOM[i][0] << "  ";
        FOMfile << FOM[i][1] << "  ";
        FOMfile << FOM[i][2] << "  ";
        FOMfile << FOM[i][3] << "  ";

        if (FOM[i][2]) { 
            FOMfile << FOM[i][0]/FOM[i][2] << "  ";
            FOMfile << errorprop_divide(FOM[i][0],FOM[i][1],FOM[i][2],FOM[i][3]) << "  "; 
        }
        else FOMfile << "0  0" << " ";

        FOMfile << endl;

    }

    c2 = (TCanvas *) gROOT->FindObject("c2");
    if(!c2) {
        c2 = new TCanvas("c2", basestring, 0, 0, 1500, 750);
    } else {
        c2->Clear();
    }

    c2->SetWindowSize(1500, 750);

    gStyle->SetOptTitle(0);

    SpatMean->Draw("APL");
    Double_t *SpatMeans = SpatMean->GetY();
    Int_t spatmap[64];
    Double_t *ComMeans = ComMean->GetY();
    Int_t commap[64];

    TMath::Sort(64,SpatMeans,spatmap,0);
    TMath::Sort(64,ComMeans,commap,0);

    Float_t gain_range, gain_range_com;
    gain_range=       100*( SpatMeans[spatmap[63]]  - SpatMeans[spatmap[0]] ) / ( SpatMeans[spatmap[63]] );
    gain_range_com =  100*( ComMeans[commap[63]]    - ComMeans[commap[0]] )  / ( ComMeans[commap[63]] ) ;

    Double_t min =TMath::Min( SpatMeans[spatmap[0]], ComMeans[commap[0]] ) ;
    Double_t max =TMath::Max( SpatMeans[spatmap[63]], ComMeans[commap[63]] ) ;


    Double_t range=max-min;

    TH1F *meancomhist = new TH1F("meancomhist","",100,0,64.5);
    TH1F *meanspathist = new TH1F("meanspathist","",100,0,64.5);
    TH1F *erescomhist = new TH1F("meancomhist","",100,0,64.5);
    TH1F *eresspathist = new TH1F("meanspathist","",100,0,64.5);

    meancomhist->SetStats(false);
    meanspathist->SetStats(false);

    meancomhist->SetMinimum(min - 0.1 * range);
    meanspathist->SetMinimum(min - 0.1 * range);

    meancomhist->SetMaximum(max + 0.1 * range);
    meanspathist->SetMaximum(max + 0.1 * range);

    meancomhist->GetYaxis()->SetLabelSize(0.075);
    meancomhist->GetYaxis()->SetTitleSize(0.09);
    meancomhist->GetYaxis()->SetTitleOffset((Float_t) 0.65 * 1.16);
    meancomhist->GetXaxis()->SetTitle();
    meancomhist->GetXaxis()->SetLabelSize(0);

    meanspathist->GetXaxis()->SetTitle();
    meanspathist->GetXaxis()->SetLabelSize(0);
    meanspathist->GetYaxis()->SetTitle();
    meanspathist->GetYaxis()->SetLabelSize(0);



    erescomhist->SetMinimum(0.05);
    erescomhist->SetMaximum(0.3);
    erescomhist->SetStats(false);

    eresspathist->SetMinimum(0.05);
    eresspathist->SetMaximum(0.3);
    eresspathist->SetStats(false);


    erescomhist->GetYaxis()->SetLabelSize(1.16 * 0.075);
    erescomhist->GetYaxis()->SetTitleSize(1.16 * 0.09);
    erescomhist->GetYaxis()->SetTitleOffset(0.65);
    erescomhist->SetMarkerStyle(7);
    eresspathist->SetMarkerStyle(7);



    meancomhist->SetMarkerStyle(7);
    meanspathist->SetMarkerStyle(7);

    meancomhist->SetMarkerSize(1.8);
    meanspathist->SetMarkerSize(0.9);

    xsums[0]->SetStats(false);
    xsums[0]->GetYaxis()->SetLabelSize(0.075);
    xsums[0]->GetXaxis()->SetLabelSize(0.075);
    xsums[3]->GetXaxis()->SetLabelSize(0.075);
    xsums[3]->SetStats(false);

    normalise_noerr(xsums[0]);
    normalise_noerr(xsums[3]);

    max = TMath::Max(xsums[0]->GetMaximum(), xsums[3]->GetMaximum());
    xsums[0]->SetMaximum(max + 0.003);
    xsums[3]->SetMaximum(max + 0.003);


    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    TLine	*sp_m = new TLine(1, TMath::Mean(64,SpatEres->GetY()), 64, TMath::Mean(64,SpatEres->GetY()));
    TLine	*com_m = new TLine(1, TMath::Mean(64,ComEres->GetY()), 64, TMath::Mean(64,ComEres->GetY()));
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    sp_m->SetLineColor(kRed);
    com_m->SetLineColor(kRed);

    sp_m->SetLineWidth(2);
    com_m->SetLineWidth(2);

    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    Double_t	up = TMath::Mean(64,SpatEres->GetY()) + TMath::RMS(64,SpatEres->GetY());
    Double_t	low = TMath::Mean(64,SpatEres->GetY()) - TMath::RMS(64,SpatEres->GetY());
    TBox		*box_sp = new TBox(1.5, up, 63.5, low);
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    up = TMath::Mean(64,ComEres->GetY()) + TMath::RMS(64,ComEres->GetY());
    low = TMath::Mean(64,ComEres->GetY()) - TMath::RMS(64,ComEres->GetY());
    //    cout << " COM Mean :: " << TMath::Mean(64,ComEres->GetY()) << " RMS :: " << TMath::RMS(64,ComEres->GetY()) << endl;
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    TBox	*box_com = new TBox(1.5, up, 63.5, low);
    Int_t	ci; /* for color index setting */
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    ci = TColor::GetColor("#efb800");

    box_sp->SetFillColor(ci);
    box_sp->SetFillStyle(3001);

    /*
     * box->SetLineWidth(2);
     */
    box_com->SetFillColor(ci);
    box_com->SetFillStyle(3001);

    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    TPad	*pad2 = new TPad("pad2", "", 0.65, 0.35, 1, 1, 0);
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    pad2->SetMargin(0, 0.0807, 0, 0.0807);

    /*
     * pad2->rottomMargin(0);
     * pad2->SetTopMargin(0.15);
     * pad2->SetLeftMargin(0);
     * pad2->SetRightMargin(0.15);
     */
    c2->Clear();
    pad2->Draw();
    pad2->cd();
    back2->SetStats(false);
    back2->Draw("colzhist");

    c2->cd();

    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    TPad	*pad5 = new TPad("pad5", "", 0.35, 0.35, .65, 0.65, 0);
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    pad5->SetMargin(0, 0, 0, 0);
    pad5->Draw();
    pad5->cd();
    eresspathist->Draw();
    box_sp->Draw();
    sp_m->Draw();
    SpatEres->Draw("P");
    c2->cd();


    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    TPad	*pad6 = new TPad("pad6", "", 0.0, 0.35, .35, 0.65, 0);
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    pad6->SetMargin(0.15, 0, 0, 0);
    pad6->Draw();
    pad6->cd();
    erescomhist->Draw();

    box_com->Draw();
    com_m->Draw();
    ComEres->Draw("P");
    c2->cd();


    TPad *pad4 = new TPad("pad4", "", 0.0, 0.65, .35, 1, 0);
    pad4->SetMargin(0.15, 0, 0, 0.15);
    pad4->Draw();
    pad4->cd();
    meancomhist->Draw();
    ComMean->Draw("P");
    c2->cd();

    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    TPad	*pad3 = new TPad("pad3", "", 0.35, 0.65, .65, 1, 0);
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    pad3->SetMargin(0, 0, 0, .15);
    pad3->Draw();
    pad3->cd();
    meanspathist->Draw();
    SpatMean->Draw("P");
    c2->cd();

    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    TPad	*pad7 = new TPad("pad7", "", 0.35, 0, .65, 0.35, 0);
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    pad7->SetMargin(0, 0, 0.15, 0);
    pad7->Draw();
    pad7->cd();
    //	xsums[3]->Draw();
    espec_glob_spat->SetStats(false);
    espec_glob_spat->SetLabelSize(.1,"X");
    espec_glob_spat->Draw();
    scorecard_glob_spat->SetX2NDC(.964);
    scorecard_glob_spat->SetX1NDC(.6);
    scorecard_glob_spat->SetY2NDC(.9);
    scorecard_glob_spat->SetY1NDC(.75);
    scorecard_glob_spat->Draw();
    c2->cd();

    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    TPad	*pad8 = new TPad("pad8", "", 0, 0, .35, 0.35, 0);
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    pad8->SetMargin(0.15, 0, 0.15, 0);
    pad8->Draw();
    pad8->cd();
    espec_glob_com->SetStats(false);
    espec_glob_com->SetLabelSize(.1,"X");
    espec_glob_com->SetLabelSize(.001,"Y");
    espec_glob_com->Draw();
    scorecard_glob_com->SetX2NDC(.964);
    scorecard_glob_com->SetX1NDC(.6);
    scorecard_glob_com->SetY2NDC(.9);
    scorecard_glob_com->SetY1NDC(.75);
    scorecard_glob_com->Draw();
    c2->cd();

    Int_t SCOREMASK=0;


    // make scorecard
    TPaveText *SCORE = new TPaveText(.66,.31,.98,.345,"NDC");
    SCORE->SetShadowColor(0);
    SCORE->Draw();

    gStyle->SetTextFont(52);


    TPaveText *scorecard_gain = new TPaveText(.68,.255,.82,.295,"NDC");
    scorecard_gain->SetShadowColor(0);
    sprintf(tmpstring,"Gain Uni SPA = %.1f%s",gain_range,"%");
    // if (gain_range < 25 ) {
    if (gain_range < 35 ) {
        scorecard_gain->SetFillColor(kGreen+1);
        SCOREMASK |= GAIN_SPAT;
    } else {
        //   if (gain_range < 35 )  {scorecard_gain->SetFillColor(kOrange);SCOREMASK |= GAIN_SPAT_INT;}
        if (gain_range < 50 )  {
            scorecard_gain->SetFillColor(kOrange);SCOREMASK |= GAIN_SPAT_INT;
        } else {
            scorecard_gain->SetFillColor(kRed);
        }
    }
    scorecard_gain->AddText(tmpstring);
    scorecard_gain->Draw();

    TPaveText *scorecard_gain_com = new TPaveText(.83,.255,.97,.295,"NDC");
    scorecard_gain_com->SetShadowColor(0);
    sprintf(tmpstring,"Gain Uni COM = %.1f%s",gain_range_com,"%");
    // if (gain_range_com < 25 ) {
    if (gain_range_com < 35 ) {
        scorecard_gain_com->SetFillColor(kGreen+1); SCOREMASK |= GAIN_COM ;
    } else {
        // if (gain_range_com < 35 ) {scorecard_gain_com->SetFillColor(kOrange); SCOREMASK |= GAIN_COM_INT;}
        if (gain_range_com < 50 ) {
            scorecard_gain_com->SetFillColor(kOrange); SCOREMASK |= GAIN_COM_INT;
        } else {
            scorecard_gain_com->SetFillColor(kRed);
        }
    }
    scorecard_gain_com->AddText(tmpstring);
    scorecard_gain_com->Draw();


    /* Here we're giving the summary */




    TPaveText *scorecard_FOM_top = new TPaveText(.68,.2,.745,.24,"NDC");
    scorecard_FOM_top->SetShadowColor(0);
    sprintf(tmpstring,"FOM_{t} = %.1f",FOM_TOP_AV);
    scorecard_FOM_top->SetFillColor(GiveCol( (Float_t) 1/FOM_TOP_AV,1/4.,1/4.,&score));
    if (score & 0x1) {
        SCOREMASK |= FOM_TOP;
    } else if (score & 0x2) {
        SCOREMASK |= FOM_TOP_INT;
    }
    scorecard_FOM_top->AddText(tmpstring);
    scorecard_FOM_top->Draw();


    TPaveText *scorecard_FOM_edge_center = new TPaveText(.83,.2,.895,.24,"NDC");
    scorecard_FOM_edge_center->SetShadowColor(0);
    sprintf(tmpstring,"FOM_{c}^{e} = %.1f",EDGE_FOM_CTR_AV);
    scorecard_FOM_edge_center->SetFillColor(GiveCol( (Float_t) 1/EDGE_FOM_CTR_AV,1/2.0,1/1.4,&score));
    if (score & 0x1) {
        SCOREMASK |= EDGE_FOM_CTR;
    } else if (score & 0x2) {
        SCOREMASK |= EDGE_FOM_CTR_INT;
    }
    scorecard_FOM_edge_center->AddText(tmpstring);
    scorecard_FOM_edge_center->Draw();

    TPaveText *scorecard_FOM_edge_top = new TPaveText(.905,.2,.97,.24,"NDC");
    scorecard_FOM_edge_top->SetShadowColor(0);
    sprintf(tmpstring,"FOM_{t}^{e} = %.1f",EDGE_FOM_TOP_AV);
    scorecard_FOM_edge_top->SetFillColor(GiveCol( (Float_t) 1/EDGE_FOM_TOP_AV,1/3.,1/2.5,&score));
    if (score & 0x1) {
        SCOREMASK |= EDGE_FOM_TOP;
    } else if (score & 0x2) {
        SCOREMASK |= EDGE_FOM_TOP_INT;
    }
    scorecard_FOM_edge_top->AddText(tmpstring);
    scorecard_FOM_edge_top->Draw();

    TPaveText *scorecard_FOM_center = new TPaveText(.755,.2,.82,.24,"NDC");
    scorecard_FOM_center->SetShadowColor(0);
    sprintf(tmpstring,"FOM_{c} = %.1f",FOM_CTR_AV);
    scorecard_FOM_center->SetFillColor(GiveCol( (Float_t) 1/FOM_CTR_AV,1/3.,1/3.,&score));
    // printf("FOM CTR score = 0x%x\n ",score);
    if (score & 0x1) {
        SCOREMASK |= FOM_CTR;
    } else if (score & 0x2) {
        SCOREMASK |= FOM_CTR_INT;
    }
    scorecard_FOM_center->AddText(tmpstring);
    scorecard_FOM_center->Draw();

    TPaveText *scorecard_uniform = new TPaveText(.68,.035,.90,.075,"NDC");
    scorecard_uniform->SetShadowColor(0);
    sprintf(tmpstring,"Gain Ratio = %.2f RMS = %.3f",ratio->GetMean(),ratio->GetRMS());
    scorecard_uniform->AddText(tmpstring);
    scorecard_uniform->SetFillColor( GiveCol(ratio->GetRMS(), 0.04,0.25,&score ));
    // cout << "RATIO SCORE = " << score << endl;
    if (score & 0x1) {
        SCOREMASK |= RATIO;
    } else if (score & 0x2) {
        SCOREMASK |= RATIO_INT;
    }
    scorecard_uniform->Draw();
    TPaveText *scorecard_flood = new TPaveText(.91,.035,.97,.075,"NDC");
    scorecard_flood->SetShadowColor(0);
    sprintf(tmpstring,"FL = %d",64-goodpix);
    scorecard_flood->AddText(tmpstring);
    // cout << "goodpix  =  " << goodpix <<endl;
    scorecard_flood->SetFillColor( GiveCol(64-goodpix, 1, 1, &score ));
    // cout << "FLOOD SCORE = " << score << endl;
    if (score & 0x1) {
        SCOREMASK |= FLOOD_UNI;
    } else if (score & 0x2) {
        SCOREMASK |= FLOOD_UNI_INT;
    }
    scorecard_flood->Draw();


    TPaveText *scorecard_eres = new TPaveText(.68,.145,.82,.185,"NDC");
    scorecard_eres->SetShadowColor(0);
    sprintf(tmpstring,"SPA E_{res} = %.1f%s",100*TMath::Mean(64,SpatEres->GetY()),"%");
    scorecard_eres->AddText(tmpstring);
    scorecard_eres->SetFillColor( GiveCol(100*TMath::Mean(64,SpatEres->GetY()), 10, 20,&score ) );
    if (score & 0x1) {
        SCOREMASK |= ERES_SPAT;
    } else if (score & 0x2) {
        SCOREMASK |= ERES_SPAT_INT;
    }
    scorecard_eres->Draw();

    TPaveText *scorecard_eres_com = new TPaveText(.68,.09,.82,.13,"NDC");
    scorecard_eres_com->SetShadowColor(0);
    sprintf(tmpstring,"COM E_{res} = %.1f%s ",100*TMath::Mean(64,ComEres->GetY()),"%");
    scorecard_eres_com->SetFillColor( GiveCol(100*TMath::Mean(64,ComEres->GetY()), 10, 20,&score ) );
    scorecard_eres_com->AddText(tmpstring);
    if (score & 0x1) {
        SCOREMASK |= ERES_COM;
    } else if (score & 0x2) {
        SCOREMASK |= ERES_COM_INT;
    }
    scorecard_eres_com->Draw();


    TPaveText *scorecard_eres_rms = new TPaveText(.83,.145,.97,.185,"NDC");
    scorecard_eres_rms->SetShadowColor(0);
    sprintf(tmpstring,"SPA E_{res} RMS = %.1f%s",100*TMath::RMS(64,SpatEres->GetY()),"%");
    scorecard_eres_rms->AddText(tmpstring);
    scorecard_eres_rms->SetFillColor( GiveCol(100*TMath::RMS(64,SpatEres->GetY()), 2, 3,&score ) );
    if (score & 0x1) {
        SCOREMASK |= ERES_SPAT_RMS;
    } else if (score & 0x2) {
        SCOREMASK |= ERES_SPAT_RMS_INT;
    }
    scorecard_eres_rms->Draw();


    TPaveText *scorecard_eres_com_rms = new TPaveText(.83,.09,.97,.13,"NDC");
    scorecard_eres_com_rms->SetShadowColor(0);
    sprintf(tmpstring,"COM E_{res} RMS = %.1f%s",100*TMath::RMS(64,ComEres->GetY()),"%");
    scorecard_eres_com_rms->SetFillColor( GiveCol(100*TMath::RMS(64,ComEres->GetY()), 2, 3.5,&score ) );
    scorecard_eres_com_rms->AddText(tmpstring);
    if (score & 0x1) {
        SCOREMASK |= ERES_COM_RMS;
    } else if (score & 0x2) {
        SCOREMASK |= ERES_COM_RMS_INT;
    }
    scorecard_eres_com_rms->Draw();


    sprintf(tmpstring,"Score ::\t\t0x%x",SCOREMASK);
    SCORE->AddText(tmpstring);


    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    TPaveText	*tit = new TPaveText(.7, .94, .95, .99, "NDC");
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    sprintf(tmpstring,"%s_A%d",modulestring,aa);
    tit->AddText(tmpstring );
    tit->SetFillColor(18);
    tit->SetTextFont(32);
    tit->SetTextColor(49);
    tit->Draw();

    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    TPaveText	*tit_spat = new TPaveText(.4, .96, .6, .99, "NDC");
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    tit_spat->AddText("Spatial");
    tit_spat->SetFillColor(18);
    tit_spat->SetTextFont(32);
    tit_spat->SetTextColor(49);
    tit_spat->Draw();

    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    TPaveText	*tit_com = new TPaveText(.1, .96, .3, .99, "NDC");
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    tit_com->AddText("Common");
    tit_com->SetFillColor(18);
    tit_com->SetTextFont(32);
    tit_com->SetTextColor(49);
    tit_com->Draw();



    sprintf(curoutfile, "%s_%s.sum.eps", basestring,modulestringcfma);
    c2->Print(curoutfile);



    sprintf(curoutfile, "%s_%s.sum.png", basestring,modulestringcfma);
    c2->Print(curoutfile);

    printf("SCORE = 0x%x\n",SCOREMASK);
    sprintf(curoutfile, "%s_%s.sum.txt", basestring,modulestringcfma);
    ofstream globfile;
    globfile.open(curoutfile);

    globfile << FOM[0][0]/2+FOM[7][0]/2 << "  ";
    globfile << TMath::Sqrt(FOM[0][1]*FOM[0][1]+FOM[7][1]*FOM[7][1])/2. << "  ";
    globfile << FOM[0][2]/2+FOM[7][2]/2 << "  ";
    globfile << TMath::Sqrt(FOM[0][3]*FOM[0][3]+FOM[7][3]*FOM[7][3])/2. << "  ";

    if(FOM[0][2]){
        globfile << ( FOM[0][0] / FOM[0][2] + FOM[7][0] / FOM[7][2] )/2. << "  ";
        globfile << TMath::Sqrt(TMath::Power(errorprop_divide(FOM[0][0],FOM[0][1],FOM[0][2],FOM[0][3]),2)  +
                TMath::Power(errorprop_divide(FOM[7][0],FOM[7][1],FOM[7][2],FOM[7][3]),2))
            << "  " ;
    } else {
        globfile << "0  0 " << " ";
    }

    globfile << FOM[3][0]/2+FOM[4][0]/2 << "  ";
    globfile << TMath::Sqrt(FOM[3][1]*FOM[3][1]+FOM[4][1]*FOM[4][1])/2. << "  ";
    globfile << FOM[3][2]/2+FOM[4][2]/2 << "  ";
    globfile << TMath::Sqrt(FOM[3][3]*FOM[3][3]+FOM[4][3]*FOM[4][3])/2. << "  ";

    if(FOM[3][2]) {
        globfile << ( FOM[3][0] / FOM[3][2]  +  FOM[4][0] / FOM[4][2] ) /2.<< "  ";
        globfile << TMath::Sqrt(TMath::Power(errorprop_divide(FOM[3][0],FOM[3][1],FOM[3][2],FOM[3][3]),2)  +
                TMath::Power(errorprop_divide(FOM[4][0],FOM[4][1],FOM[4][2],FOM[4][3]),2))
            << "  ";
    } else {
        globfile << "0  0" << " ";
    }



    if (!kFailedReadGlobFit){
      for (Int_t k=0;k<6;k++){
        globfile <<  glob_spat_fit->GetParameter(k) << " ";
      }
      if (glob_spat_fit->GetParameter(4)){
        globfile << 100 * 2.35 *glob_spat_fit->GetParameter(5) / glob_spat_fit->GetParameter(4) << "  ";
        globfile << 100 * 2.35 * TMath::Sqrt( TMath::Power((glob_spat_fit->GetParError(5) / glob_spat_fit->GetParameter(4)), 2) + 
                                              TMath::Power((glob_spat_fit->GetParameter(5) * glob_spat_fit->GetParError(4) 
                                              / (glob_spat_fit->GetParameter(4)*glob_spat_fit->GetParameter(4))), 2) ) << "  ";
      }  else {
        globfile <<  "0   0" ;
      }
    } else {
      globfile << "0 0 0 0 0 0 0 0" ;
    }


    if (!kFailedReadGlobComFit){
      for (Int_t k=0;k<6;k++){
	globfile <<  glob_com_fit->GetParameter(k) << " ";
      }
      if (glob_com_fit->GetParameter(4)) {
	globfile << 100 * 2.35 *glob_com_fit->GetParameter(5) / glob_com_fit->GetParameter(4) << "  ";
	globfile << 100 * 2.35 * TMath::Sqrt( TMath::Power((glob_com_fit->GetParError(5) / glob_com_fit->GetParameter(4)), 2) + 
                                              TMath::Power((glob_com_fit->GetParameter(5) * glob_com_fit->GetParError(4)
                                                      / (glob_com_fit->GetParameter(4)*glob_com_fit->GetParameter(4))), 2) ) << "  ";
      } else {
         globfile <<  "0   0" ;
      }
    }else {
	globfile << " 0  0 0  0 0  0  0  0" << endl;
    }



    globfile << EDGE_FOM_CTR_AV << " ";
    globfile << EDGE_FOM_TOP_AV << " ";

    globfile << "0x" << hex << SCOREMASK << " ";

    globfile.close();

    return 0;
}


Int_t GiveCol(Float_t aa, Float_t a1, Float_t a2, Int_t* RESULT){
    //  cout << "aa = " << aa << "  a1 = " << a1 << "  a2 = " << a2 << endl;
    *RESULT=0;
    if ( aa < a1 ) {
        (*RESULT) |= 0x1; return kGreen+1;
    } else if ( aa < a2) {
        (*RESULT) |= 0x2; return kOrange;
    } else {
        return kRed;
    }
}

Double_t *ptv_ana(TH1F *hist, TF1 *posgausfits[64], TCanvas *c1, Int_t nr, Int_t x,Int_t verbose){

    Double_t av_fwhm, av_mean_dist, av_mean_dist_e, av_fwhm_e;
    Int_t i,index;
    Char_t tmpstring[80];
    Double_t *ptvalues;
    ptvalues = new Double_t[8];
    av_mean_dist=0;av_mean_dist_e=0;
    av_fwhm=0;av_fwhm_e=0;

    c1->Clear();
    c1->Divide(1,2);
    c1->cd(1);
    hist->Draw();  

    Int_t next;
    Int_t p=1;

    for (i=p;i<(8-p);i++){
        if (x) {
            index = 8*nr+i;next=8*nr+i+1;
        } else {index = 8*i+nr;next=8*(i+1)+nr;
        }
        if (verbose) {
            cout << " index = " << index << " next =" << next << endl;
        }
        posgausfits[index]->Draw("same");
        //    gPad->Update(); gPad->Update();
        if (i<(7-p)) {
            av_mean_dist+=posgausfits[next]->GetParameter(1)-posgausfits[index]->GetParameter(1);
            av_mean_dist_e+=power(posgausfits[index]->GetParError(1),2)+power(posgausfits[next]->GetParError(1),2);
        }
        av_fwhm+=posgausfits[index]->GetParameter(2);
        av_fwhm_e+=power(posgausfits[index]->GetParError(2),2);
        if (verbose) {
            cout << i << " mean = " <<  posgausfits[index]->GetParameter(1) << " sigma = " ;
            cout << posgausfits[index]->GetParameter(2) << " mean_dist = " ;
            if (i<(7-p)) {
                cout << posgausfits[next]->GetParameter(1)-posgausfits[index]->GetParameter(1);
                cout << " error : " << TMath::Sqrt(power(posgausfits[index]->GetParError(1),2)+power(posgausfits[next]->GetParError(1),2))<< "  mean_dist_e = " << av_mean_dist_e << endl;
            } else {
                cout << " NA " << endl; 
            }
        } //verbose
    }
    av_mean_dist_e=TMath::Sqrt(av_mean_dist_e);
    av_fwhm_e=TMath::Sqrt(av_fwhm_e);
    av_mean_dist/=(7-2*p);av_mean_dist_e/=(7-2*p);
    av_fwhm/=(8-2*p);av_fwhm_e/=(8-2*p);

    Double_t mean_variance,fwhm_variance;
    mean_variance=0;
    fwhm_variance=0;

    for (i=p;i<(8-p);i++){
        if (x) {
            index = 8*nr+i;next=8*nr+i+1;
        } else {
            index = 8*i+nr;next=8*(i+1)+nr;
        }
        if (i<(7-p)){
            mean_variance+=power( av_mean_dist -( posgausfits[next]->GetParameter(1)-posgausfits[index]->GetParameter(1)) ,2);
        }
        fwhm_variance+=power( av_fwhm -( posgausfits[index]->GetParameter(2)) ,2);
    }

    mean_variance=TMath::Sqrt(mean_variance/(8-2*p-1));
    fwhm_variance=TMath::Sqrt(fwhm_variance/(8-2*p));


    av_fwhm*=2.345;av_fwhm_e*=2.345;
    fwhm_variance*=2.345;
    if (verbose){
        cout << "MEAN DISTANCE = " << av_mean_dist << " +/- " << av_mean_dist_e << endl;
        cout << "MEAN FWHM     = " << av_fwhm << " +/- " << av_fwhm_e << endl;
        cout << "MEAN DISTANCE RMS = " << mean_variance << " RMS/sqrt("<<8-2*p-1<<") = " << mean_variance/TMath::Sqrt(8-1-2*p) << endl;
        cout << "FWHM RMS = " << fwhm_variance << " RMS/sqrt("<<8-2*p<<") = " << fwhm_variance/TMath::Sqrt(8-2*p) << endl;
    }


    // Analyze the edge
    Double_t edge_fwhm, edge_dist;
    Double_t edge_fwhm_e, edge_dist_e;
    edge_fwhm=0;
    edge_dist=0;
    edge_fwhm_e=0;
    edge_dist_e=0;
    Double_t edge_fwhm_1,edge_dist_1;
    Double_t edge_fwhm_e_1, edge_dist_e_1;
    Double_t edge_fwhm_2,edge_dist_2;
    Double_t edge_fwhm_e_2, edge_dist_e_2;
    Double_t edge_fom_1,edge_fom_2;
    edge_fwhm_e_1=0;
    edge_fwhm_e_2=0;
    i=0;
    if (x) {
        index = 8*nr+i*6;next=8*nr+i*6+1;
    } else {
        index = 8*i*6+nr;
        next=8*(i*6+1)+nr;
    }
    edge_dist_1 = posgausfits[next]->GetParameter(1)-posgausfits[index]->GetParameter(1);  
    edge_dist_e_1 = power(posgausfits[index]->GetParError(1),2)+power(posgausfits[next]->GetParError(1),2);
    edge_fwhm_e_1+=power(posgausfits[index]->GetParError(2),2);                                                                      
    edge_fwhm_e_1+=power(posgausfits[next]->GetParError(2),2);    
    edge_fwhm_1 = (posgausfits[index]->GetParameter(2) + posgausfits[next]->GetParameter(2) ) / 2. ;
    edge_fwhm_1 *=2.345;
    edge_fom_1 = edge_dist_1/edge_fwhm_1 ;


    i=1;
    if (x) {
        index = 8*nr+i*6;next=8*nr+i*6+1;
    } else {
        index = 8*i*6+nr;
        next=8*(i*6+1)+nr;
    }
    edge_dist_2 = posgausfits[next]->GetParameter(1)-posgausfits[index]->GetParameter(1);  
    edge_dist_e_2 = power(posgausfits[index]->GetParError(1),2)+power(posgausfits[next]->GetParError(1),2);
    edge_fwhm_e_2+=power(posgausfits[index]->GetParError(2),2);                                                                                        
    edge_fwhm_e_2+=power(posgausfits[next]->GetParError(2),2);    
    edge_fwhm_2 = (posgausfits[index]->GetParameter(2) + posgausfits[next]->GetParameter(2) ) / 2. ;
    edge_fwhm_2 *=2.345;
    edge_fom_2 = edge_dist_2/edge_fwhm_2 ;


    edge_dist_e_1=TMath::Sqrt(edge_dist_e_1);
    edge_dist_e_2=TMath::Sqrt(edge_dist_e_2);
    edge_fwhm_e_1=TMath::Sqrt(edge_fwhm_e_1);
    edge_fwhm_e_2=TMath::Sqrt(edge_fwhm_e_2);
    edge_fwhm_e_1/=2;
    edge_fwhm_e_2/=2;
    edge_fwhm_e_1*=2.345;
    edge_fwhm_e_2*=2.345;


    for (i=0;i<2;i++){
        if (x) {
            index = 8*nr+i*6;next=8*nr+i*6+1;
        } else {
            index = 8*i*6+nr;
            next=8*(i*6+1)+nr;
        }
        edge_dist+=posgausfits[next]->GetParameter(1)-posgausfits[index]->GetParameter(1);
        edge_dist_e+=power(posgausfits[index]->GetParError(1),2)+power(posgausfits[next]->GetParError(1),2);
        edge_fwhm+=posgausfits[index]->GetParameter(2);
        edge_fwhm+=posgausfits[next]->GetParameter(2);
        edge_fwhm_e+=power(posgausfits[index]->GetParError(2),2);
        edge_fwhm_e+=power(posgausfits[next]->GetParError(2),2);
    }

    edge_dist_e=TMath::Sqrt(edge_dist_e);
    edge_fwhm_e=TMath::Sqrt(edge_fwhm_e);
    edge_dist/=2;edge_dist_e/=2;
    edge_fwhm/=4;edge_fwhm_e/=4;

    Double_t edge_mean_variance,edge_fwhm_variance;
    edge_mean_variance=0;
    edge_fwhm_variance=0;
    for (i=0;i<2;i++){
        if (x) {
            index = 8*nr+i*6;next=8*nr+i*6+1;
        } else {
            index = 8*i*6+nr;
            next=8*(i*6+1)+nr;
        }
        edge_mean_variance+=power( edge_dist - (posgausfits[next]->GetParameter(1)-posgausfits[index]->GetParameter(1)) ,2);
        if (verbose){
            cout << " i = " << i << " :: " << posgausfits[next]->GetParameter(1)-posgausfits[index]->GetParameter(1) 
                 << " edge_mean variance :: " << edge_mean_variance <<  "  edge mean :: " << edge_dist << endl;
        }
        edge_fwhm_variance+=power( edge_fwhm -( posgausfits[index]->GetParameter(2)) ,2);
        edge_fwhm_variance+=power( edge_fwhm -( posgausfits[next]->GetParameter(2)) ,2);
    }

    edge_mean_variance=TMath::Sqrt(edge_mean_variance/(2-1));
    edge_fwhm_variance=TMath::Sqrt(edge_fwhm_variance/(4-1));

    //  cout << " edge_mean variance :: " << edge_mean_variance << endl;

    if (verbose) {
        cout << "EDGE FOM left:  " << edge_fom_1 << "+/-" << errorprop_divide(edge_dist_1,edge_dist_e_1,edge_fwhm_1,edge_fwhm_e_1);
        cout <<  " dist : " << edge_dist_1 << " width : " << edge_fwhm_1 <<endl;
        cout << "EDGE FOM right: " << edge_fom_2 << "+/-" << errorprop_divide(edge_dist_2,edge_dist_e_2,edge_fwhm_2,edge_fwhm_e_2);
        cout <<  " dist : " << edge_dist_2 << " width : " << edge_fwhm_2 <<endl;
    }

    edge_fwhm*=2.345;edge_fwhm_e*=2.345;
    edge_fwhm_variance*=2.345;

    // return 0;

    c1->cd(2);
    TPaveText *result = new TPaveText(.16,.16,.85,.69,"NDC");
    sprintf(tmpstring,"MEAN DISTANCE = %.4f #pm %.4f (RMS/sqrt(%d) = %.4f)",av_mean_dist,av_mean_dist_e,8-2*p-1 , mean_variance/TMath::Sqrt(8-2*p-1));
    result->AddText(tmpstring);
    sprintf(tmpstring,"MEAN FWHM = %.4f #pm %.4f (RMS/sqrt(%d) = %.4f)",av_fwhm,av_fwhm_e,8-2*p,fwhm_variance/TMath::Sqrt(8-2*p));
    result->AddText(tmpstring);
    sprintf(tmpstring,"EDGE MEAN FWHM = %.4f #pm %.4f (RMS/sqrt(%d) = %.4f)",edge_fwhm,edge_fwhm_e,4-1,edge_fwhm_variance/TMath::Sqrt(4-1));
    result->AddText(tmpstring);
    sprintf(tmpstring,"EDGE MEAN DISTANCE = %.4f #pm %.4f (RMS/sqrt(%d) = %.4f)",edge_dist,edge_dist_e,2-1,edge_mean_variance/TMath::Sqrt(2-1));
    result->AddText(tmpstring);

    result->Draw();

    //  cout << " LOK AT ME :: p = " << p << endl;

    ptvalues[0]=av_mean_dist;
    ptvalues[1]=mean_variance/TMath::Sqrt(8-1-2*p);
    ptvalues[2]=av_fwhm;
    ptvalues[3]=fwhm_variance/TMath::Sqrt(8-2*p);
    ptvalues[4]=edge_dist;
    ptvalues[5]=edge_mean_variance/TMath::Sqrt(2-1);
    ptvalues[6]=edge_fwhm;
    ptvalues[7]=edge_fwhm_variance/TMath::Sqrt(4-1);


    // 3-28-2012 --> we want the minimum FOM edge !
    if (( edge_fom_1 ) < ( edge_fom_2 )) {
        ptvalues[4]=edge_dist_1;
        ptvalues[5]=edge_dist_e_1;
        ptvalues[6]=edge_fwhm_1;
        ptvalues[7]=edge_fwhm_e_1;

    } else {
        ptvalues[4]=edge_dist_2;
        ptvalues[5]=edge_dist_e_2;
        ptvalues[6]=edge_fwhm_2;
        ptvalues[7]=edge_fwhm_e_2;

    }
    return ptvalues;
}
