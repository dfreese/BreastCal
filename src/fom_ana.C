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

int main(int argc, Char_t *argv[])
{
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    Char_t		filenamel[FILENAMELENGTH] = "", curoutfile[FILENAMELENGTH];
    Int_t		verbose = 0;
    Int_t		ix;
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
    }

    cout << "Welcome to FOM ana." ; if (verbose) cout << endl;

    if (!filenamespec) { 
        cout << " Please Specify filename:: fom_ana -f [filename] .\n Exiting. " << endl ;
        return -2;
    }
    TCanvas *c1;
    c1 = (TCanvas*)gROOT->GetListOfCanvases()->FindObject("c1");
    if (!c1) {
        c1 = new TCanvas("c1","c1",10,10,1000,1000);
    }

    if (!verbose) {
        gErrorIgnoreLevel=kError;
    }

    rootlogon(verbose);


    Int_t validpeaks[RENACHIPS][4][2];

    Double_t U_x[RENACHIPS][4][2][64];
    Double_t U_y[RENACHIPS][4][2][64];
    Double_t aa, bb;
    Char_t filebase[FILENAMELENGTH],rootfile[FILENAMELENGTH]; 
    Char_t tmpname[20];//,tmptitle[50];
    Int_t i,j,k,m,lines;
    Int_t ii;
    ifstream infile;
    TH1F *xhist[RENACHIPS][4][2][64];
    TF1 *xfits[RENACHIPS][4][2][64];
    Char_t peaklocationfilename[FILENAMELENGTH],histname[40],histtitle[120];
    strncpy(filebase,filenamel,strlen(filenamel)-9);
    filebase[strlen(filenamel)-9]='\0';

    if (verbose){ 
        cout << " filename = " << filenamel << endl;
        cout << " filebase = " << filebase << endl;
    }

    for (m=0;m<RENACHIPS;m++) {
        for (i=0;i<4;i++) {
            for (j=0;j<2;j++) {
                validpeaks[m][i][j]=0;
                sprintf(peaklocationfilename,"./CHIPDATA/%s.RENA%d.unit%d_apd%d_peaks",filebase,m,i,j);
                strcat(peaklocationfilename,".txt");
                infile.open(peaklocationfilename);
                lines = 0;
                while (1){
                    if (!infile.good()) {
                        break;
                    }
                    infile >> k >>  aa >> bb;
                    if (k < 64) { 
                        U_y[m][i][j][k]=aa; U_x[m][i][j][k]=bb;
                    }
                    //       cout << k << ", " << U_x[i][j][k]<< ", " << U_y[i][j][k] << endl;
                    lines++;      
                }

                if (verbose) {
                    cout << "Found " << lines-1 << " peaks in  file " << peaklocationfilename << endl;
                }
                infile.close();
                if (lines==65) { 
                    if (verbose) cout << "Setting Validpeaks " << endl; validpeaks[m][i][j]=1;
                }
            }//j
        } //i 
    } //m



    //        strncpy(filebase,filename,strlen(filename)-17);
    //        filebase[strlen(filename)-17]='\0';
    //        sprintf(rootfile,"%s",filename);
    //        strcat(rootfile,".root");

    //	cout << "Rootfile to open :: " << rootfile << endl;

    for (m=0;m<RENACHIPS;m++){
        for (i=0;i<4;i++){
            for (j=0;j<2;j++){
                for (k=0;k<64;k++){
                    sprintf(tmpname,"xhist[%d][%d][%d][%d]",m,i,j,k);
                    xhist[m][i][j][k] = new TH1F(tmpname,tmpname,NRPOSBINS, POSMIN, POSMAX);
                }
            }
        }
    }

    if (verbose) {
        cout << " Opening file " << filenamel << endl;
    }
    TFile *file_left = new TFile(filenamel,"OPEN");

    // Open Calfile //
    //        strncpy(filebase,filenamel,strlen(filenamel)-13);
    //        filebase[strlen(filenamel)-13]='\0';
    sprintf(rootfile,"%s",filebase);
    strcat(rootfile,".fom.root");

    if (verbose) {
        cout << " Opening file " << rootfile << " for writing " << endl;
    }
    TFile *calfile = new TFile(rootfile,"RECREATE");
    TTree *block;
    Char_t treename[40];

    sprintf(treename,"cal");
    block = (TTree *) file_left->Get(treename);
    if (!block) {
        cout << " Problem reading Tree " << treename  << " from file " << filenamel << endl;
        cout << " Exiting " << endl;
        return -10;
    }
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

    if (verbose) {
        cout << " Looping over " << entries_ch1_l << " entries " <<endl;
    }

    //       entries_ch1_l/=100;

    for (ii=0;ii<entries_ch1_l;ii++){
        //       for (ii=0;ii<1;ii++){
        block->GetEntry(ii);
        //       cout << "UL2.mod = " << UL2.mod << "; UL2.x = "<<  UL2.x << "; UL2.id = ";
        //       cout << UL2.id << "; UL2.Ecal = " << UL2.Ecal << endl;

        if ((event->id>=0)&&(event->id<64)) {
            if ((event->Ecal>EGATEMIN)&&(event->Ecal<EGATEMAX)) {
                if ((event->apd==1)||(event->apd==0)){
                    m=event->chip;
                    if ((m>=0)&&(m<RENACHIPS)) {
                        if ((event->m<4)&&(event->m>=0)) {
                            //		 cout << " m = " << m << ", event->m=" << event->m << ", event->apd=" << event->apd << ", event->id=" << event->id << endl;
                            xhist[m][event->m][event->apd][event->id]->Fill(event->x); 
                        }
                    }
                }
            }
        }
    } // for loop

    if (verbose) {
        cout << " Looping Done. " << endl;
    }
    //	} // loop over chips



    Double_t *FOM[RENACHIPS][4][2][8];
    TH1F *xsums[RENACHIPS][4][2][8];

    for (m=0;m<RENACHIPS;m++){
        for (i=0;i<4;i++){
            for (j=0;j<2;j++){
                if (validpeaks[m][i][j]) {
                    fitpos(xhist[m][i][j],xfits[m][i][j],verbose);
                    for (k=0;k<64;k++){
                        sprintf(histname, "xfits[%d][%d][%d][%d]", m,i,j,k);
                        xfits[m][i][j][k]->SetName(histname);
                        xhist[m][i][j][k]->Write();
                    }
                    for( k = 0; k < 8; k++) {
                        sprintf(histname, "xsums[%d][%d][%d][%d]", m,i,j,k);
                        sprintf(histtitle, "Peak to valley row %d Chip %d module %d apd %d ", k,m,i,j);
                        xsums[m][i][j][k] = new TH1F(histname, histtitle, NRPOSBINS, POSMIN, POSMAX);
                        for(Int_t ll = 0; ll < 8; ll++) {
                            //                        xsums[m][i][j][k]->Add(xhist[m][i][j][k * 8 + ll], 1);
                            xsums[m][i][j][k]->Add(xhist[m][i][j][k + ll*8], 1);
                            //			cout << " m = " << m << " i= " << i << " j= " << j << " k= " << k*8+ll << endl;
                        }

                    }
                    FOM[m][i][j][0] = ptv_ana(xsums[m][i][j][0], xfits[m][i][j],c1, 0, 0, verbose);
                    sprintf(curoutfile, "%s.RENA%d.unit%d.apd%d_FOM.ps(", filebase, m,i,j);

                    /*
                     * c1->Print("042109_409-1-17-A_FOM.ps(");
                     */
                    //	  xsums[m][i][j][0]->Draw();
                    c1->Print(curoutfile);

                    sprintf(curoutfile, "%s.RENA%d.unit%d.apd%d_FOM.ps", filebase, m,i,j);
                    //	sprintf(curoutfile, "%s_FOM.ps", filebase);
                    for( k = 1; k < 7; k++) {
                        FOM[m][i][j][k] = ptv_ana(xsums[m][i][j][k], xfits[m][i][j], c1, k, 0, verbose);
                        //xsums[m][i][j][k]->Draw();
                        c1->Print(curoutfile);
                    }

                    FOM[m][i][j][7] = ptv_ana(xsums[m][i][j][7],  xfits[m][i][j], c1, 7, 0, verbose);
                    for( k = 0; k < 8; k++) {
                         xsums[m][i][j][k]->Write();
                    }
                    sprintf(curoutfile, "%s.RENA%d.unit%d.apd%d_FOM.ps)",filebase, m,i,j);
                    //        sprintf(curoutfile, "%s_FOM.ps)", filebase);
                    //        xsums[m][i][j][7]->Draw();
                    c1->Print(curoutfile);

                } // validpeaks
            } //j 
        } //i 
    } //m

    ofstream fomout;
    Char_t fomfile[MAXFILELENGTH];
    sprintf(fomfile,"%s.fom.txt",filebase);  
    fomout.open(fomfile);

    Double_t FOM_CTR_AV ; Double_t FOM_TOP_AV ; Double_t EDGE_FOM_CTR_AV ; Double_t EDGE_FOM_TOP_AV ;
    Double_t FOM_CTR_AV_E=0 ; Double_t FOM_TOP_AV_E=0 ; Double_t EDGE_FOM_CTR_AV_E=0 ; Double_t EDGE_FOM_TOP_AV_E=0 ;
    for (m=0;m<RENACHIPS;m++){

        fomout << "==============================================================" <<endl;
        fomout << "== FOM DATA CHIP " << m  <<endl;
        fomout << "==============================================================" <<endl;
        fomout << "      |    FOM_center   |  FOM_top   | FOM_center_edge  |FOM_top_edge|" << endl;

        for (i=0;i<4;i++){
            for (j=0;j<2;j++){
                if (validpeaks[m][i][j]){

                    if (FOM[m][i][j][3][2] && FOM[m][i][j][4][2]) {
                        FOM_CTR_AV = ( FOM[m][i][j][3][0]/FOM[m][i][j][3][2] + FOM[m][i][j][4][0]/FOM[m][i][j][4][2]);
                        FOM_CTR_AV_E = TMath::Sqrt( TMath::Power(errorprop_divide(FOM[m][i][j][3][0],FOM[m][i][j][3][1],FOM[m][i][j][3][2],FOM[m][i][j][3][3]),2) +
                                TMath::Power(errorprop_divide(FOM[m][i][j][4][0],FOM[m][i][j][4][1],FOM[m][i][j][4][2],FOM[m][i][j][4][3]),2))/2.;
                    } else {
                        FOM_CTR_AV = 0;
                    }
                    FOM_CTR_AV/=2;


                    if (FOM[m][i][j][0][2] && FOM[m][i][j][7][2])  {
                        FOM_TOP_AV= ( FOM[m][i][j][0][0]/FOM[m][i][j][0][2] + FOM[m][i][j][7][0]/FOM[m][i][j][7][2]);
                        FOM_TOP_AV_E = TMath::Sqrt( TMath::Power(errorprop_divide(FOM[m][i][j][0][0],FOM[m][i][j][0][1],FOM[m][i][j][0][2],FOM[m][i][j][0][3]),2) +
                                TMath::Power(errorprop_divide(FOM[m][i][j][7][0],FOM[m][i][j][7][1],FOM[m][i][j][7][2],FOM[m][i][j][7][3]),2))/2.;
                    } else {
                        FOM_TOP_AV = 0;
                    }
                    FOM_TOP_AV/=2;


                    if (FOM[m][i][j][3][6] && FOM[m][i][j][4][6]) {
                        EDGE_FOM_CTR_AV = ( FOM[m][i][j][3][4]/FOM[m][i][j][3][6] + FOM[m][i][j][4][4]/FOM[m][i][j][4][6]);
                        EDGE_FOM_CTR_AV_E = TMath::Sqrt( TMath::Power(errorprop_divide(FOM[m][i][j][3][4],FOM[m][i][j][3][5],FOM[m][i][j][3][6],FOM[m][i][j][3][7]),2) +
                                TMath::Power(errorprop_divide(FOM[m][i][j][4][4],FOM[m][i][j][4][5],FOM[m][i][j][4][6],FOM[m][i][j][4][7]),2))/2.;
                    } else {
                        EDGE_FOM_CTR_AV = 0;
                    }
                    EDGE_FOM_CTR_AV/=2;


                    if (FOM[m][i][j][0][2] && FOM[m][i][j][7][2]) {
                        EDGE_FOM_TOP_AV = ( FOM[m][i][j][0][4]/FOM[m][i][j][0][6] + FOM[m][i][j][7][4]/FOM[m][i][j][7][6]);
                        EDGE_FOM_TOP_AV_E = TMath::Sqrt( TMath::Power(errorprop_divide(FOM[m][i][j][0][4],FOM[m][i][j][0][5],FOM[m][i][j][0][6],FOM[m][i][j][0][7]),2) +
                                TMath::Power(errorprop_divide(FOM[m][i][j][7][4],FOM[m][i][j][7][5],FOM[m][i][j][7][6],FOM[m][i][j][7][7]),2))/2.;
                    } else {
                        EDGE_FOM_TOP_AV = 0;
                    }
                    EDGE_FOM_TOP_AV/=2;

                    Char_t pm;
                    pm=126;
                    fomout.precision(3);
                    fomout << fixed;
                    //    fomout << setw(5);
                    //    fomout << setfixed(5);
                    fomout << " U" << i << "M" << j << ": ";
                    fomout <<  FOM_CTR_AV << " " << pm << " " << FOM_CTR_AV_E << " | " ;
                    fomout <<  FOM_TOP_AV << " " << pm << " " << FOM_TOP_AV_E << " | " ;
                    fomout <<  EDGE_FOM_CTR_AV << " " << pm << " " << EDGE_FOM_CTR_AV_E << " | " ;
                    fomout <<  EDGE_FOM_TOP_AV << " " << pm << " " << EDGE_FOM_TOP_AV_E << " | " ;
                    fomout << endl;
                }// validpeaks
            }
        }
    } // loop m
    //#endif
    fomout.close();
    calfile->Close();
    return 0;
}

