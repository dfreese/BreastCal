#include "TROOT.h"
#include "TStyle.h"
#include "Riostream.h"
#include "TTree.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TSpectrum.h"
#include "TSpectrum2.h"
#include "TH2F.h"
#include "TVector.h"
#include "THistPainter.h"
#include "TPaletteAxis.h"
#include "TMath.h"
#include <sys/stat.h>
#include "decoder.h"
#include "DetCluster.h"
#include "FindNext.h"
#include "Kmeans.h"
#include "Cluster.h"
#include "Apd_Sort16_v2.h"
#include <string>

// this parameter is used to check if the segmentation is valid ::
#define COSTTHRESHOLD 300

// comment this if you don't want the additional Pictorial Structure based search.
#define sortps

//comment this if you don't want the traditional 16 peak search.
#define SRT16

TGraph *PeakSearch(
        TH2F *flood,
        Char_t filebase[200],
        Int_t verbose,
        Int_t &validflag,
        Float_t &cost,
        Bool_t APD,
        TString fDIR,
        Float_t yoffset,
        Float_t xoffset);
Double_t findmean(
        TH1D * h1,
        Int_t verbose,
        Bool_t APD);
Double_t findmean8(
        TH1D * h1,
        Int_t verbose,
        Int_t X,
        Bool_t APD);
Int_t updatesegmentation(
        TH2F *hist,
        Double_t x[16],
        Double_t y[16],
        TGraph *updated,
        Int_t verbose);
Int_t updatesegpoint(
        TH2F *hist,
        Double_t x,
        Double_t y,
        Double_t &xxx,
        Double_t &yyy,
        Int_t verbose);
Int_t validate_sort(
        TGraph *gr,
        Int_t verbose);

void usage(void) {
    cout << "anafloods [-v -h] -f [filename]\n"
         << "  -c [cartridgeId] : specify fin\n"
         << "  -l [finId] : specify fin\n"
         << "  -m [moduleId] : specify module\n"
         << "  -a [apdId]    : specify apd\n"
         << "  -yoff [value] : y cordinate of split\n"
         << "  -xoff [value] : x cordinate of split\n"
         << endl;
}

int main(int argc, char ** argv) {
    if (argc == 1) {
        usage();
        return(0);
    }

    cout << " Welcome to anafloods. Program performs Segmentation.  " ;

    string filename;
    Int_t verbose = 0;
    Int_t firstcartridge = 0;
    Int_t lastcartridge = CARTRIDGES_PER_PANEL;
    Int_t firstfin = 0;
    Int_t lastfin = FINS_PER_CARTRIDGE;
    Int_t firstmodule = 0;
    Int_t lastmodule = MODULES_PER_FIN;
    Int_t firstapd = 0;
    Int_t lastapd = APDS_PER_MODULE;
    Float_t yoffset = -99;
    Float_t xoffset = -99;

    // Arguments not requiring input
    for (int ix = 1; ix < argc; ix++) {
        if(strcmp(argv[ix], "-v") == 0) {
            cout << "Verbose Mode " << endl;
            verbose = 1;
        }
        if(strcmp(argv[ix], "-h") == 0) {
            usage();
            return(0);
        }
    }

    // Arguments requiring input
    for(int ix = 1; ix < (argc - 1); ix++) {
        if(strcmp(argv[ix], "-c") == 0) {
            firstcartridge = atoi(argv[++ix]);
            lastcartridge = firstcartridge+1;
            if (firstcartridge >= CARTRIDGES_PER_PANEL || firstcartridge < 0) {
                cout << " cartridge out of range: " << firstcartridge << endl;
                return(-1);
            }
        }
        if(strncmp(argv[ix], "-l", 2) == 0) {
            firstfin = atoi(argv[++ix]);
            lastfin = firstfin + 1;
            if (firstfin >= FINS_PER_CARTRIDGE || firstfin < 0) {
                cout << " fin out of range: " << firstfin << endl;
                return(-2);
            }
        }
        if(strcmp(argv[ix], "-m") == 0) {
            firstmodule = atoi(argv[++ix]);
            lastmodule = firstmodule + 1;
            if (firstmodule >= MODULES_PER_FIN || firstmodule < 0) {
                cout << " module out of range: " << firstmodule << endl;
                return(-3);
            }
        }
        if(strcmp(argv[ix], "-a") == 0) {
            firstapd = atoi(argv[++ix]);
            lastapd = firstapd + 1;
            if (firstapd >= APDS_PER_MODULE || firstapd < 0) {
                cout << " apd out of range: " << firstapd << endl;
                return(-4);
            }
        }
        if(strcmp(argv[ix], "-yoff") == 0 ) {
            yoffset = atof(argv[++ix]);
            cout << "Using y = " << yoffset << " for quadrant splitting\n ";
        }
        if(strcmp(argv[ix], "-xoff") == 0 ) {
            xoffset = atof(argv[++ix]);
            cout << "Using x = " << xoffset << " for quadrant splitting\n ";
        }
        if(strcmp(argv[ix], "-f") == 0) {
            filename = string(argv[++ix]);
        }
    }

    cout << " Inputfile : " << filename << "." << endl;

    rootlogon(verbose);
    set2dcolor(4);

    if (!verbose) {
        gErrorIgnoreLevel = kBreak;
    }

    TFile *rfile = new TFile(filename.c_str(),"OPEN");
    if (!rfile || rfile->IsZombie()) {
        cout << "problems opening file " << filename << ". Exiting." << endl;
        return(-5);
    }

    TH1F *E[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE];
    TH2F *floods[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE];

    Char_t tmpstring[60];
    Char_t pngstring[FILENAMELENGTH];
    ofstream peaklocationfile;
    Char_t peaklocationfilename[FILENAMELENGTH];
    TCanvas *c1;
    Bool_t modevents[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE];


    size_t root_file_ext_pos(filename.rfind(".root"));
    if (root_file_ext_pos == string::npos) {
        cerr << "Unable to find .root extension in: \"" << filename << "\"" << endl;
        cerr << "...Exiting." << endl;
        return(-6);
    }
    string filebase(filename, 0, root_file_ext_pos);


    c1 = (TCanvas*)gROOT->GetListOfCanvases()->FindObject("c1");
    if (!c1) {
        c1 = new TCanvas("c1","c1",10,10,1000,1000);
    }
    c1->SetCanvasSize(700,700);


    // It is important that the names stay consistent between reading and
    // writing in the different programs

    /* Read in the flood and energy histograms */
    for (int c = 0; c < CARTRIDGES_PER_PANEL; c++) {
        for (int f=0; f < FINS_PER_CARTRIDGE; f++) {
            for (int i = 0; i < MODULES_PER_FIN; i++) {
                for (int j = 0; j < APDS_PER_MODULE; j++) {
                    // Pulling the Spatial Energy Histograms from the root file
                    sprintf(tmpstring,"C%dF%d/E[%d][%d][%d][%d]",c,f,c,f,i,j);
                    E[c][f][i][j] = (TH1F *) rfile->Get(tmpstring);
                    if (!(E[c][f][i][j])) {
                        cerr << "Unable to find energy histogram: "
                             << tmpstring << endl;
                        cerr << "Exiting" << endl;
                        return(-7);
                    }

                    if (E[c][f][i][j]->GetEntries() > MINHISTENTRIES) {
                        modevents[c][f][i][j] = true;
                    } else {
                        modevents[c][f][i][j] = false;
                    }
                    // Pulling the Flood Histograms from the root file
                    sprintf(tmpstring,"C%dF%d/flood_C%dF%dM%dA%d",c,f,c,f,i,j);
                    floods[c][f][i][j] = (TH2F *) rfile->Get(tmpstring);
                    if (!floods[c][f][i][j]) {
                        cerr << "Unable to find flood: " << tmpstring << endl;
                        cerr << "Exiting" << endl;
                        return(-8);
                    }
                }
            }
        }
    }

    Double_t *xsort,*ysort;
    Int_t nvalid=0;
    Bool_t  validsegment[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE]= {{{{0}}}};
    Float_t costs[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE]= {{{{0}}}};
    TGraph *peaks[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE];
    Int_t flag=0;

    TString fDIR="SEGMENTATION";
    TString fSDIR="./SEGMENTATION/FLOODS";

    if ( access( fDIR.Data(), 0 ) != 0 ) {
        cout << "creating dir " << fDIR << endl;
        if ( mkdir(fDIR, 0777) != 0 ) {
            cout << " Error making directory " << fDIR << endl;
            return -2;
        }
    }

    if ( access( fSDIR.Data(), 0 ) != 0 ) {
        cout << "creating dir " << fSDIR << endl;
        if ( mkdir(fSDIR, 0777) != 0 ) {
            cout << " Error making directory " << fDIR << endl;
            return -2;
        }
    }



    for (int c = firstcartridge; c < lastcartridge; c++) {
        for (int f = firstfin; f < lastfin; f++) {
            cout << " Analyzing fin .. " << f << " in cartridge " << c << endl;
            for (int i = firstmodule; i < lastmodule; i++) {
                for (int j = firstapd; j < lastapd; j++) {
                    if (modevents[c][f][i][j]) {
                        c1->Clear();
                        c1->Divide(2,2);
                        if (verbose) {
                            cout << " Crystal segmentation Module " << i << ",  apd " << j << endl;
                        }

                        if (verbose) {
                            cout << " ********************************************************* "  << endl;
                            cout << " * Determining Crystal Locations  C" << c << "F" << f << "M" << i << "A" << j << " *"<<endl;
                            cout << " ********************************************************* "  << endl;
                            cout << " Entries Ehist :: " << E[c][f][i][j]->GetEntries() ;
                            cout << " Entries Floods :: " << floods[c][f][i][j]->GetEntries() ;

                        }


                        if (verbose)	{
                            cout << "C"<<c<<"F"<<f<<"M"<<i<<"A"<<j<< endl;
                        }

                        /* Look for the 64 crystal positions */
                        sprintf(peaklocationfilename,"%s.C%dF%dM%dA%d",filebase.c_str(),c,f,i,j);
                        peaks[c][f][i][j] =
                                PeakSearch(floods[c][f][i][j],
                                           peaklocationfilename,
                                           verbose,
                                           flag,
                                           costs[c][f][i][j],
                                           j,
                                           fDIR,
                                           yoffset,
                                           xoffset);
                    } else {
                        if (verbose) {
                            cout << " --------- Skipping C" << c << "F" << f<< " MODULE " << i << " APD " << j << " ----------- " << endl;
                            cout << " ------------- Not enough events ------------------------- " << endl;
                        } //verbose
                        peaks[c][f][i][j] = new TGraph(64);
                        flag=0;
                    }
                    sprintf(tmpstring,"peaks[%d][%d][%d][%d]",c,f,i,j);
                    peaks[c][f][i][j]->SetName(tmpstring);

                    if (verbose) {
                        cout << "CHECK FLAG == " << flag <<  endl;
                    }
                    /* NEED TO REWRITE ALL THIS CODE :: */

                    c1->SetCanvasSize(700,700);

                    c1->Clear();
                    set2dcolor(4);
                    c1->Divide(2,2);

                    c1->cd(1);
                    floods[c][f][i][j]->Draw("colz");
                    c1->cd(2);
                    floods[c][f][i][j]->Draw("histcolz");
                    c1->cd(3);
                    peaks[c][f][i][j]->Draw("APL");
                    c1->cd(4);
                    floods[c][f][i][j]->Draw("colz");
                    peaks[c][f][i][j]->Draw("PL");

                    if (flag==15) {
                        validsegment[c][f][i][j]=1;
                        nvalid++;
                        sprintf(peaklocationfilename,"%s.C%dF%d.module%d_apd%d_peaks",filebase.c_str(),c,f,i,j);
                    } else  {
                        validsegment[c][f][i][j]=0;
                        sprintf(peaklocationfilename,"%s.C%dF%d.module%d_apd%d_peaks.failed",filebase.c_str(),c,f,i,j);
                    }
                    strcat(peaklocationfilename,".txt");
                    peaklocationfile.open(peaklocationfilename);

                    xsort = peaks[c][f][i][j]->GetX();
                    ysort = peaks[c][f][i][j]->GetY();
                    for (int k=0; k<PEAKS; k++) {
                        peaklocationfile << k << " " << xsort[k] << " " << ysort[k] <<endl;
                    }

                    peaklocationfile.close();

                    if (verbose) {
                        cout <<" Made it here too! " <<endl;
                    }
                    sprintf(pngstring,"%s/%s.C%dF%dM%dA%d.flood.png",fSDIR.Data(),filebase.c_str(),c,f,i,j);
                    c1->Update();
                    c1->Print(pngstring);
                }
            }
        }
    }

    sprintf(peaklocationfilename,"%s.segsum.txt",filebase.c_str());
    peaklocationfile.open(peaklocationfilename);

    peaklocationfile << "\n*****************************************************"  << endl;
    peaklocationfile << "* " <<  nvalid << " Valid segmentations for " << RENACHIPS*8 << " APDs:                *" << endl;

    for (int c = firstcartridge; c < lastcartridge; c++) {
        for (int f = firstfin; f < lastfin; f++) {
            if (verbose) {
                cout << " Analyzing fin .. " << f << " in cartridge " << c << endl;
            }
            for (int i = firstmodule; i < lastmodule; i++) {
                for (int j = firstapd; j < lastapd; j++) {

                    if (validsegment[c][f][i][j]==1) {
                        peaklocationfile << "* C" << c << "F" << f << "M" <<  i  << "A" << j << "  "  << validsegment[c][f][i][j] << "  (min cost: " << costs[c][f][i][j] <<" )      *" <<endl;
                    }
                }
            }
        }
    }
    peaklocationfile << "*                                                   *"  << endl;
    peaklocationfile << "*****************************************************"  << endl;
    peaklocationfile.close();
    rfile->Close();

    return 0;
}

TGraph *PeakSearch(
        TH2F *flood,
        Char_t filebase[200],
        Int_t verbose,
        Int_t &validflag,
        Float_t &cost,
        Bool_t APD,
        TString fDIR,
        Float_t yoffset,
        Float_t xoffset)
{

    cost=999.999;

    /* Step one: split in four */
    TH2F *floodsq[4];
    TH1D *projX;
    TH1D *projY;
    Float_t meanX,meanY;
    Int_t midbinx,midbiny,binsx,binsy,newbin;
    Char_t tmpstr[40],tmpstr2[220];
    Char_t pngstring[220];
    Double_t xval,yval,value;

    /* find mean based on projection*/
    Float_t xc,yc;
    kmeans2d( flood, &xc, &yc, verbose)   ;
    if (verbose) {
        cout << " KMEANS :: " << xc << "  " << yc << endl;
    }

    if (verbose) {
        cout << " welcome to PeakSearch " << endl;
    }
    projX = (TH1D*) flood->ProjectionX("",120,136);
    projX->SetName("projX");

    if (verbose) {
        cout << " finding X center " << endl;
    }
    meanX = findmean8(projX,verbose,1,APD);



    Int_t xlowbin,xhighbin;

    if (meanX!=12345) {
        xlowbin= projX->FindBin(meanX)-5;
        xhighbin= projX->FindBin(meanX)+5;
    } else {
        xlowbin=120;
        xhighbin=136;
    }

    projY = (TH1D*) flood->ProjectionY("",xlowbin,xhighbin);
    projY->SetName("projY");



    if (( projY->GetEntries() < 1000 ) && ( meanX!=12345) ) {
        projY->Reset();
        projY = (TH1D*) flood->ProjectionY("",xlowbin-5,xhighbin+5);
        projY->SetName("projY");
    }

    if ((  projY->GetEntries()/projY->GetNbinsX() ) < 5 ) {
        projY->Rebin(2);
    }

    if (verbose) {
        cout << " finding Y center " << endl;
    }
    meanY = findmean8(projY,verbose,0,APD);

    // if findmean8 doesn't identify 8 peaks in the projection histogram we do an alternative methode.

    if (meanX==12345) {
        projX = (TH1D*) flood->ProjectionX();
        meanX = findmean(projX,verbose,APD);
    }

    if (meanY==12345) {
        projY = (TH1D*) flood->ProjectionY();
        meanY = findmean(projY,verbose,APD);
    }

    projX->SetAxisRange(-.25,.25);
    projY->SetAxisRange(-.25,.25);


    // overwrite with k-means value
    meanX = xc;
    meanY = yc;

    // override with user specified values
    if (yoffset>-99) {
        meanY = yoffset;
    }
    if (xoffset>-99) {
        meanX = xoffset;
    }


    midbinx=flood->GetXaxis()->FindBin(meanX);
    midbiny=flood->GetYaxis()->FindBin(meanY);

    if (verbose) {
        cout << " midbinx  = "  << midbinx ;
        cout << " midbiny  = "  << midbiny <<endl;
    }



    TCanvas *c1;
    c1 = (TCanvas*)gROOT->GetListOfCanvases()->FindObject("c1");
    if (!c1) {
        c1 = new TCanvas("c1","c1",10,10,1000,1000);
    }
    c1->SetCanvasSize(700,700);
    c1->Clear();
    gStyle->SetOptDate(0);

    TLine *lineX = new TLine();
    TLine *lineY = new TLine();
    lineX->SetY1(projY->GetBinLowEdge(1));
    lineX->SetY2(projY->GetBinLowEdge(1)+projY->GetBinWidth(1)*projY->GetNbinsX());
    lineX->SetX1(meanX);
    lineX->SetX2(meanX);
    lineY->SetX1(projX->GetBinLowEdge(1));
    lineY->SetX2(projX->GetBinLowEdge(1)+projX->GetBinWidth(1)*projX->GetNbinsX());
    lineY->SetY1(meanY);
    lineY->SetY2(meanY);

    Int_t i,j;

    /* creating the histograms */
    for (int k = 0; k < 4; k++) {
        sprintf(tmpstr,"floodsq[%d]",k);
        sprintf(tmpstr2,"%s, Quadrant %d",filebase,k);
        switch(k) {
        case 0:
            binsx=(1-(meanX-(projX->GetBinWidth(1)/2.)))/projX->GetBinWidth(1);
            binsy=(1-(meanY-(projY->GetBinWidth(1)/2.)))/projY->GetBinWidth(1);
            if (verbose) {
                cout << "Creating Histo with " << meanX-(projX->GetBinWidth(1)/2.);
                cout << " < X <  1 ( " <<  binsx << " bins ) and with ";
                cout << (meanY-(projY->GetBinWidth(1)/2.));
                cout << " < Y <  1 ( " <<  binsy << " bins )  " <<endl;
            }
            floodsq[k]= new TH2F(tmpstr,tmpstr2,binsx,meanX-(projX->GetBinWidth(1)/2.),1,binsy,(meanY-(projY->GetBinWidth(1)/2.)),1);
            break;
        case 1:
            binsx=(1-(meanX-(projX->GetBinWidth(1)/2.)))/projX->GetBinWidth(1);
            binsy=(1+(meanY-(projY->GetBinWidth(1)/2.)))/projY->GetBinWidth(1);
            if (verbose) {
                cout << "Creating Histo with " << meanX-(projX->GetBinWidth(1)/2.);
                cout << " < X <  1 ( " <<  binsx << " bins ) and with ";
                cout << -(meanY-(projY->GetBinWidth(1)/2.));
                cout << " < Y <  1 ( " <<  binsy << " bins )  " <<endl;
            }
            floodsq[k]= new TH2F(tmpstr,tmpstr2,binsx,meanX-(projX->GetBinWidth(1)/2.),1,binsy,-(meanY-(projY->GetBinWidth(1)/2.)),1);
            break;
        case 2:
            binsx=(1+(meanX-(projX->GetBinWidth(1)/2.)))/projX->GetBinWidth(1);
            binsy=(1+(meanY-(projY->GetBinWidth(1)/2.)))/projY->GetBinWidth(1);
            if (verbose) {
                cout << "Creating Histo with " << -(meanX-(projX->GetBinWidth(1)/2.));
                cout << " < X <  1 ( " <<  binsx << " bins ) and with ";
                cout << -(meanY-(projY->GetBinWidth(1)/2.));
                cout << " < Y <  1 ( " <<  binsy << " bins )  " <<endl;
            }
            floodsq[k]= new TH2F(tmpstr,tmpstr2,binsx,-(meanX-(projX->GetBinWidth(1)/2.)),1,binsy,-(meanY-(projY->GetBinWidth(1)/2.)),1);
            break;
        case 3:
            binsx=(1+(meanX-(projX->GetBinWidth(1)/2.)))/projX->GetBinWidth(1);
            binsy=(1-(meanY-(projY->GetBinWidth(1)/2.)))/projY->GetBinWidth(1);
            if (verbose) {
                cout << "Creating Histo with " << -(meanX-(projX->GetBinWidth(1)/2.));
                cout << " < X <  1 ( " <<  binsx << " bins ) and with ";
                cout << (meanY-(projY->GetBinWidth(1)/2.));
                cout << " < Y <  1 ( " <<  binsy << " bins )  " <<endl;
            }
            floodsq[k]= new TH2F(tmpstr,tmpstr2,binsx,-(meanX-(projX->GetBinWidth(1)/2.)),1,binsy,(meanY-(projY->GetBinWidth(1)/2.)),1);
            break;
        }
    }

    for (int k = 0; k < (flood->GetNbinsX()); k++) {
        for (int l = 0; l < (flood->GetNbinsY()); l++) {
            xval = flood->GetXaxis()->GetBinCenter(k);
            yval = flood->GetYaxis()->GetBinCenter(l);
            value =  flood->GetBinContent(k,l);

            if (k > midbinx) {
                if (l > midbiny) {
                    i=0;
                    newbin = floodsq[0]->FindBin(xval,yval);
                } else {
                    i=1;
                    newbin = floodsq[1]->FindBin(xval,-yval);
                }
            } else {
                if (l > midbiny) {
                    newbin = floodsq[3]->FindBin(-xval,yval);
                    i=3;
                } else {
                    newbin = floodsq[2]->FindBin(-xval,-yval);
                    i=2;
                }
            }
            floodsq[i]->SetBinContent(newbin,value);
        }
    }

    TSpectrum2 *peaks[4];
    Int_t npeaks_q[4];
    Double_t *xsort_q[4],*ysort_q[4];
    TGraph *peaks_remapped[4];
    for (int k = 0; k < 4; k++) {
        peaks_remapped[k] = new TGraph(16);
    }
    Float_t *xpeaks_q[4];
    Float_t *ypeaks_q[4];
    validflag=0;
    Char_t histname[40];
    Char_t psfilename[60];
    Float_t totcost[4];
    TH2F *input[4];
    TH2F *inp[4];
    TH2F *penalty[4];
    TH2F *costfunc[4];
    TH2F *updatehist[4];
    TH2F *deformfunc[4];
    bin *cluster[4];
    TF1 *cf_angle[15];
    TF1 *cf_length[15];
    Double_t xxx,yyy,ttt;
    Double_t xx[16],yy[16];
    Int_t ll,xbin2,ybin2,zbin2,maxbin,ii,jj;
    Int_t binx,biny,loopcount;
    Double_t rmsfactor;

#define MAXM 35

    Int_t maxbins[MAXM];
    Int_t vetoangle[MAXM];
    Float_t maxbin_content[MAXM];
    Int_t x_maxbin[MAXM],y_maxbin[MAXM],z_maxbin[MAXM];
    Float_t xcorner,ycorner;
    Float_t xmaxbin[MAXM],ymaxbin[MAXM];
    Float_t length;
    Float_t angle;
    Float_t anglematch;
    Float_t lengthmatch;
    Int_t m,start;
    Int_t nosegupdate;
    Int_t setstone[15];
    Int_t springattempt[15];

    for (int k = 0; k < 4; k++) {
        sprintf(histname,"peaks_remapped[%d]",k);
        peaks_remapped[k]->SetName(histname);
        totcost[k]=0;
    }
    for (int k = 0; k < 4; k++) {
        peaks[k] = new TSpectrum2();
    }
    
    for (int k = 0; k < 4; k++) {
        if (verbose) {
            cout << endl;
            cout << " ===============================================================" <<endl;
            cout << "               Searching Peaks in Quadrant " << k << endl;
            cout << " ===============================================================" <<endl;
        }

#ifdef SRT16

// Loop over four quadrants

        npeaks_q[k]=peaks[k]->Search(floodsq[k],1.2,"noMarkov",0.15);
        if (verbose) {
            cout << " Peaks in quadrant " << k  << ": " << npeaks_q[k] <<endl;
        }
        if (npeaks_q[k] != 16 ) {
            npeaks_q[k]=peaks[k]->Search(floodsq[k],2.0,"noMarkov",0.1);
            if (verbose) {
                cout << " New limits :: Peaks in quadrant " << k  << ": " << npeaks_q[k] <<endl;
            }
        }

        if (npeaks_q[k] != 16 ) {
            npeaks_q[k]=peaks[k]->Search(floodsq[k],1.4,"noMarkov",0.15);
            if (verbose) {
                cout << " New limits :: Peaks in quadrant " << k  << ": " << npeaks_q[k] <<endl;
            }
        }

        if (npeaks_q[k] < 16 ) {
            npeaks_q[k]=peaks[k]->Search(floodsq[k],1.1,"noMarkov",0.15);
            if (verbose) {
                cout << " New limits :: Peaks in quadrant " << k  << ": " << npeaks_q[k] <<endl;
            }
        }
        // if too many peaks, increase widht of peak searching algorithm
        if (npeaks_q[k] > 16 ) {
            npeaks_q[k]=peaks[k]->Search(floodsq[k],2.0,"noMarkov",0.15);
            if (verbose) {
                cout << " New limits :: Peaks in quadrant " << k  << ": " << npeaks_q[k] <<endl;
            }
        }
        if (npeaks_q[k] > 16 ) {
            npeaks_q[k]=peaks[k]->Search(floodsq[k],3.0,"noMarkov",0.15);
            if (verbose) {
                cout << " New limits :: Peaks in quadrant " << k  << ": " << npeaks_q[k] <<endl;
            }
        }

        if (npeaks_q[k] > 16 ) {
            npeaks_q[k]=peaks[k]->Search(floodsq[k],3.75,"noMarkov",0.15);
            if (verbose) {
                cout << " New limits :: Peaks in quadrant " << k  << ": " << npeaks_q[k] <<endl;
            }
        }

        if (npeaks_q[k] > 16 ) {
            npeaks_q[k]=peaks[k]->Search(floodsq[k],3.75,"noMarkov",0.4);
            if (verbose) {
                cout << " New limits :: Peaks in quadrant " << k  << ": " << npeaks_q[k] <<endl;
            }
        }

        if (npeaks_q[k] != 16 ) {
            npeaks_q[k]=peaks[k]->Search(floodsq[k],1.11,"noBackgroundnoMarkov",0.35);
            if (verbose) {
                cout << " New limits :: Peaks in quadrant " << k  << ": " << npeaks_q[k] <<endl;
            }
        }

        // sorting the peaks

        if (npeaks_q[k] == 16 ) {
            xpeaks_q[k]=peaks[k]->GetPositionX() ;
            ypeaks_q[k]=peaks[k]->GetPositionY() ;
            if (verbose) {
                cout << "Going to SORT16" <<endl;
            }

            xcorner=floodsq[k]->GetXaxis()->GetBinCenter(0);
            ycorner=floodsq[k]->GetYaxis()->GetBinCenter(0);

            if (verbose) {
                cout << " Flood corner :: " << xcorner << " " << ycorner << endl;
            }
            sort16(xpeaks_q[k],ypeaks_q[k],&ysort_q[k],&xsort_q[k],peaks_remapped[k],xcorner,ycorner,verbose);
            if ( ! validate_sort(peaks_remapped[k],verbose) ) {
                validflag |= ( 1 << k) ;
            }
            if (verbose ) {
                cout << " Done with SORT16 " << endl;
            }
        } else {
            if (verbose) {
                cout << " not enough peaks found in histogram " << k << endl;
            }
        }

#endif

#ifdef sortps

        Double_t oria,a,b;
        oria=0;
        Int_t stepback=0;
        Int_t forcepoint=0;
        Int_t alloops=0;

        if ((validflag&(1<<k))==0) {
            if (verbose) {
                cout << " Trying Pictorial Structures. Nr of entries in histogram ::  " << floodsq[k]->Integral(0,floodsq[k]->GetNbinsX(),0,floodsq[k]->GetNbinsY()) << endl;
            }

            /* reset genfuncs to original */
            genfuncs(cf_length,cf_angle,1);

            Float_t FIRST_CLUSTER_SEARCH_TH=0.25;
            Int_t LENGTHFACTOR=20;
            Int_t ANGLEFACTOR=20;
            Float_t HARDTHRESHOLD=1.;
            Bool_t ENABLEVETO=kTRUE;

            if (floodsq[k]->Integral(0,128,0,128) < 1000 ) {
                if (verbose) {
                    cout << "adjusting threshold first cluster ." << endl ;
                }
                FIRST_CLUSTER_SEARCH_TH=0.2;
                HARDTHRESHOLD=0.5;
                ENABLEVETO=kFALSE;
            }

            nosegupdate=0;
            input[k] = (TH2F *) floodsq[k]->Clone();
            sprintf(histname,"input[%d]",k);
            input[k]->SetName(histname);
            normalise_1(input[k]);
            hist_threshold(input[k],0.05) ;
            inp[k] = (TH2F *) floodsq[k]->Clone();
            sprintf(histname,"inp[%d]",k);
            inp[k]->SetName(histname);
            normalise_1(inp[k]);
            hist_threshold(inp[k],FIRST_CLUSTER_SEARCH_TH) ;

            penalty[k] = (TH2F *) input[k]->Clone();
            sprintf(histname,"penalty[%d]",k);
            penalty[k]->SetName(histname);
            penalty[k]->Reset();
            binx=0;
            biny=0;
            for (i=0; i<15; i++) {
                setstone[i]=0;
                springattempt[i]=0;
            }
            j=1 ;

            // STEP 1: FIND THE LOWER LEFT CORNER //

            while (j<inp[k]->GetNbinsX()) {
                for  (i=1; i<=j; i++) {
                    if (verbose) {
                        cout <<  inp[k]->GetBinContent(i,j) <<  "  " << inp[k]->GetBinContent(j,i) << "  ";
                    }
                    // .25 used to be 1e-10
                    if ((inp[k]->GetBinContent(i,j) >= FIRST_CLUSTER_SEARCH_TH )|| (inp[k]->GetBinContent(j,i) >= FIRST_CLUSTER_SEARCH_TH )) {
                        if (inp[k]->GetBinContent(i,j) >= FIRST_CLUSTER_SEARCH_TH ) {
                            binx=i;
                            biny=j;
                        } else {
                            binx=j;
                            biny=i;
                        }
                        j=inp[k]->GetNbinsX();
                        break;
                    }
                }
                if ( verbose) {
                    cout << endl;
                }
                j++;
            }
            cluster[k] = new bin;
            //    cluster[k] = &
            ClusterSize(inp[k], binx,biny, cluster[k]);
            if (cluster[k]->pixels<0) {
                nosegupdate=1;
            }
            if (verbose) {
                cout << " Size of the cluster : " << cluster[k]->pixels << " @ " << cluster[k]->x <<","<<cluster[k]->y;
                cout <<  " = (" << inp[k]->GetXaxis()->GetBinCenter(cluster[k]->x) ;
                cout <<  "," <<  inp[k]->GetYaxis()->GetBinCenter(cluster[k]->y) << ")" <<endl;
            }
            peaks_remapped[k]->SetPoint(0,inp[k]->GetXaxis()->GetBinCenter(cluster[k]->x), inp[k]->GetYaxis()->GetBinCenter(cluster[k]->y));


            costfunc[k]=(TH2F *) input[k]->Clone();
            sprintf(histname,"costfunc[%d]",k);
            costfunc[k]->SetName(histname);
            deformfunc[k]=(TH2F *) input[k]->Clone();
            sprintf(histname,"deformfunc[%d]",k);
            deformfunc[k]->SetName(histname);
            updatehist[k] = (TH2F *) input[k]->Clone();
            sprintf(histname,"updatehist[%d]",k);
            updatehist[k]->SetName(histname);
            if (verbose) {
                cout << " Bins :: " << costfunc[k]->GetNbinsX() << " ";
                cout << costfunc[k]->GetNbinsY() << endl;
            }

            c1->Clear();
            c1->Divide(2,2);

            if (verbose) {
                cout << "setting the first point ::" <<endl;
            }
            updatepoint(input[k],0,cluster[k]->x,cluster[k]->y,xx,yy);

            peaks_remapped[k]->SetPoint(0,xx[0], yy[0]);
            for (ll=0; ll<15; ll++) {
                alloops++;
                if (alloops > 50 ) {
                    ll=20;
                    totcost[k]=-9999;
                    continue;
                }
                loopcount=0;
                while (1) {
                    if(verbose) {
                        cout << " Getting Spring ll = " << ll << "; xxx = " << xxx << ", yyy = " << yyy << endl;
                    }
                    loopcount++;
                    if (setstone[10]) { /* spring 6 was found, as well as its valid corresponding "L", so now we evaluate stretch */
                        Double_t  stretch=TMath::Sqrt(TMath::Power(xx[0]-xx[7],2)+TMath::Power(yy[0]-yy[7],2)) ;
                        Double_t ratio = stretch/( cf_length[6]->GetParameter(1 ));
                        if (verbose) {
                            cout << " STRETCH EVAL (k=" << k << ") :: length spring 6 :: " ;
                            cout << stretch << " Average :: " << cf_length[6]->GetParameter(1) ;
                            cout << ", Ratio : " << ratio  << endl;
                        }
                        if ((ratio-1)> 0.25 ) {
                            genfuncs(cf_length,cf_angle,ratio) ;
                        }
                    }

                    for (i=1; i<=costfunc[k]->GetNbinsX(); i++) {
                        // COST  = DEFORM + MATCH

                        for (j=1; j<=costfunc[k]->GetNbinsY(); j++) {
                            deformfunc[k]->SetBinContent(i,j,CalcCost(input[k],ll,i,j,xx,yy,cf_length,cf_angle));
                            //    costfunc[k]->SetBinContent(i,j,deformfunc[k]->GetBinContent(i,j)+12*input[k]->GetBinContent(i,j));

                            /* higher threshold for points closer to the center */
                            if (ll<12) {
                                if ( ( ll==2) || (ll==5) || (ll==8) || (ll==10 )) {
                                    if (input[k]->GetBinContent(i,j)> (HARDTHRESHOLD*0.1)) {
                                        costfunc[k]->SetBinContent(i,j,deformfunc[k]->GetBinContent(i,j)+20*input[k]->GetBinContent(i,j));
                                    } else {
                                        costfunc[k]->SetBinContent(i,j,0);
                                    }

                                }  else {
                                    // these thresholds should probably be dynamic
                                    if (input[k]->GetBinContent(i,j)> (HARDTHRESHOLD*0.3)) {
                                        costfunc[k]->SetBinContent(i,j,deformfunc[k]->GetBinContent(i,j)+15*input[k]->GetBinContent(i,j));
                                    } else {
                                        costfunc[k]->SetBinContent(i,j,0);
                                    }
                                }
                                if (ll==11) {
                                    // at l=11 we're pretty far down the array.
                                    if (input[k]->GetBinContent(i,j)> (HARDTHRESHOLD*0.2)) {
                                        costfunc[k]->SetBinContent(i,j,deformfunc[k]->GetBinContent(i,j)+15*input[k]->GetBinContent(i,j));
                                    } else {
                                        costfunc[k]->SetBinContent(i,j,0);
                                    }

                                }
                            } else { // case where (ll>=12)
                                if (input[k]->GetBinContent(i,j)> (HARDTHRESHOLD*0.1)) {
                                    costfunc[k]->SetBinContent(i,j,deformfunc[k]->GetBinContent(i,j)+25*input[k]->GetBinContent(i,j));
                                } else {
                                    costfunc[k]->SetBinContent(i,j,0);
                                }
                            }//}

                            //adding penalty
                            costfunc[k]->SetBinContent(i,j,costfunc[k]->GetBinContent(i,j)+penalty[k]->GetBinContent(i,j));
                        } //loop over j
                    } // loop over i

                    // WE HAVE NOW DEFINED OUR COSTFUNCTION !!
                    // CHECKING FOR THE BEST MATCH !!!

                    maxbin = costfunc[k]->GetMaximumBin();
                    for (m=0; m<MAXM; m++) {
                        maxbins[m]=costfunc[k]->GetMaximumBin();
                        maxbin_content[m]=costfunc[k]->GetBinContent(maxbins[m]);
                        costfunc[k]->GetBinXYZ(maxbins[m],x_maxbin[m],y_maxbin[m],z_maxbin[m]) ;
                        //set it to zero so that in the next iteration we find another point.
                        costfunc[k]->SetBinContent(maxbins[m],-100);
                    }
                    start=ll;
                    if ((ll==3)||(ll==6)) {
                        start=0;
                    }
                    if ((ll==9)||(ll==11)) {
                        start=7;
                    }
                    if ((ll==13)||(ll==14)) {
                        start=12;
                    }
                    if (verbose) {
                        cout << " k=" << k <<"; Checking " << MAXM ;
                        cout << " highest bins for connection between point = " << ll+1 ;
                        cout << " and point " << ll << " ( quadrant " << k << ")" << endl;
                    }
                    for (m=0; m<MAXM; m++) {
                        //we are going to plot the costfunc, so put the value back
                        costfunc[k]->SetBinContent(maxbins[m],maxbin_content[m]);
                        xmaxbin[m]=costfunc[k]->GetXaxis()->GetBinCenter(x_maxbin[m]);
                        ymaxbin[m]=costfunc[k]->GetYaxis()->GetBinCenter(y_maxbin[m]);
                        // calculate length and angle
                        length=TMath::Sqrt( TMath::Power(xmaxbin[m]-xx[start],2)+TMath::Power(ymaxbin[m]-yy[start],2));
                        angle=TMath::ATan2(ymaxbin[m]-yy[start],xmaxbin[m]-xx[start]);
                        //            lengthmatch=3*cf_length[ll]->Eval(length);
                        lengthmatch=LENGTHFACTOR*cf_length[ll]->Eval(length);
                        // anglematch=3*cf_angle[k]->Eval(angle);
                        anglematch=ANGLEFACTOR*cf_angle[ll]->Eval(angle);
                        vetoangle[m]=0;
                        // double check angle
                        // THIS IS NOT RIGHT 60 degrees is too flexible, should be 70 !

                        if ((ll==0)||(ll==1)||(ll==2)||(ll==7)||(ll==8)||(ll==12)) {
                            // angle should be around 90 -> larger than 60 degrees
                            if (ll==12) {
                                if (angle < 5.5*TMath::Pi()/18.) {
                                    vetoangle[m]=1;
                                }
                            } else {
                                if (angle <  6.5*TMath::Pi()/18.) {
                                    vetoangle[m]=1;
                                }
                            }

                        } else {
                            if ((ll==3)||(ll==4)||(ll==5)||(ll==9)||(ll==10)||(ll==13)) {
                                if (angle > TMath::Pi()/6.) {
                                    vetoangle[m]=1;
                                }
                                // angle should be around 0 ->  smaller than 30 degrees
                            } else {
                                if (( angle > TMath::Pi()/3. ) || ( angle < 25*TMath::Pi()/180. )) {
                                    vetoangle[m]=1;
                                }
                                // so ll==6 or ll==11 or ll==14, angle around 45 degrees
                            }
                        }
                        vetoangle[m]&=ENABLEVETO;
                        if (verbose) {
                            cout <<  m  << " :: " <<  maxbin_content[m] ;
                            cout << " x: " << xmaxbin[m];
                            cout << " y: " << ymaxbin[m];
                            cout << " ; matchfunc = " << lengthmatch+anglematch << "(" << deformfunc[k]->GetBinContent(maxbins[m])  ;
                            cout << " ,length = " << lengthmatch << ", angle = " << anglematch << "; l="<<length<<", a="<<angle<<")" ;
                            cout << " ; valuematch = " << input[k]->GetBinContent(maxbins[m]);
                            cout << " ; totmatch = " << input[k]->GetBinContent(maxbins[m])+(lengthmatch+anglematch);
                            cout << " ; veto : " << vetoangle[m] ;
                            cout << endl;
                        }

                    }
                    m=0;
                    while (( deformfunc[k]->GetBinContent(maxbins[m]) < 9 )|| (vetoangle[m] )) {
                        m++;
                        if ( m >= MAXM ) {
                            m=0;
                            maxbin_content[m]=29;
                            break;
                        }
                    }

                    if (verbose) {
                        cout << " using point m = " << m << endl;
                    }

                    if ((maxbin_content[m]>30)||(forcepoint)) {
                        stepback=0;
                        if (forcepoint) {
                            if (verbose) {
                                cout << " FORCEPOINT set, setstone[" << ll <<"] = 1" << endl;
                            }
                            setstone[ll]=1;
                            forcepoint=0;
                            while (vetoangle[m]) {
                                m++;
                                if (m>MAXM) {
                                    break;
                                }
                            }
                            if (verbose) {
                                cout << " first non vetoed point :  m = " << m << endl;
                            }
                            if (m>=MAXM) {
                                m=0;
                            }
                        }

                        if (verbose) {
                            cout << " Setting point " << m << " with value : " << maxbin_content[m];
                            cout <<  " xbin: " << x_maxbin[m] << ", ybin: " << y_maxbin[m] << endl;
                        }
                        maxbin=maxbins[m];

                        totcost[k]+=costfunc[k]->GetBinContent(maxbin);
                        costfunc[k]->GetBinXYZ( maxbin, xbin2, ybin2, zbin2);
                        //     cout << "updating point " << endl;

                        updatepoint(input[k],ll+1,xbin2,ybin2,xx,yy);
                        for (i=1; i<=input[k]->GetNbinsX(); i++) {
                            for (j=1; j<=input[k]->GetNbinsY(); j++) {
                                updatehist[k]->SetBinContent(i,j,input[k]->GetBinContent(i,j));
                            }
                        }
                        if (nosegupdate==0) {
                            updatesegpoint(updatehist[k],xx[ll+1],yy[ll+1],xxx,yyy,verbose);
                        }
                        if ((TMath::Abs(xx[ll]-xxx)<0.005)&&(TMath::Abs(yy[ll]-yyy)<0.005)) {
                            if (verbose ) {
                                cout << " not updating point (" << xxx << "," << yyy << "), too close to";
                                cout << " previous point (" << xx[ll] << "," << yy[ll] << ")." << endl;
                            }
                        } else {
                            if (verbose) {
                                cout << " overlap check passed, updating point " << endl;
                            }
                            ttt=xx[ll+1];
                            xx[ll+1]=xxx;
                            xxx=ttt;
                            ttt=yy[ll+1];
                            yy[ll+1]=yyy;
                            yyy=ttt;
                        }

                        peaks_remapped[k]->SetPoint(ll+1,xx[ll+1], yy[ll+1]);
                        if (verbose) {
                            //	  hist_threshold(costfunc[k],0.1);
                            hist_threshold(costfunc[k],0.1);
                            c1->cd(1);
                            costfunc[k]->Draw("histcolz");
                            c1->cd(2);
                            hist_threshold(deformfunc[k],-150.);
                            deformfunc[k]->Draw("colz");
                            c1->cd(3);
                            input[k]->Draw("colz");
                            c1->cd(4);
                            floodsq[k]->Draw("colzhist");
                            peaks_remapped[k]->Draw("PL");
                            if (ll==0) {
                                sprintf(psfilename,"costfunc_q%d.ps(",k);
                            } else if (ll==14) {
                                sprintf(psfilename,"costfunc_q%d.ps)",k);
                            } else {
                                sprintf(psfilename,"costfunc_q%d.ps",k);
                            }
                            c1->Print(psfilename);
                        }
                        if (verbose) {
                            cout <<  " Resetting penalty " << endl;
                        }
                        penalty[k]->Reset();
                        if (ll==2) {
                            for (ii=0; ii<3; ii++) {
                                setstone[ii]=1;
                            }
                        }
                        if (ll==5) {
                            for (ii=3; ii<6; ii++) {
                                setstone[ii]=1;
                            }
                        }
                        if (ll==10) {
                            for (ii=6; ii<11; ii++) {
                                setstone[ii]=1;
                            }
                        }
                        if (ll==13) {
                            for (ii=11; ii<14; ii++) {
                                setstone[ii]=1;
                            }
                        }
                        if (ll==14) {
                            setstone[14]=1;
                        }
                        break;
                    }  // had a valid point

                    rmsfactor=1.5;
                    cf_length[ll]->SetParameter(2, cf_length[ll]->GetParameter(2)*rmsfactor);
                    cf_angle[ll]->SetParameter(2, cf_angle[ll]->GetParameter(2)*rmsfactor);
                    cf_length[ll]->SetParameter(0,cf_length[ll]->GetParameter(0)/cf_length[ll]->GetMaximum());
                    cf_angle[ll]->SetParameter(0,cf_angle[ll]->GetParameter(0)/cf_angle[ll]->GetMaximum());
                    if (loopcount>4) {
                        oria=cf_length[ll]->GetParameter(5);
                        a=-12.5;
                        b=2*a*cf_length[ll]->GetParameter(1);
                        cf_length[ll]->SetParameter(5,a);
                        cf_length[ll]->SetParameter(4,-b);
                        cf_length[ll]->SetParameter(3,b*b/(4*a));
                    }

                    if (verbose) {
                        cout << " No valid match found for ll = " << ll << "; quadrant " << k << ", adjusting matchfunc " << endl;
                        cout << " RMS cf_length[ " << ll << "] = " << cf_length[ll]->GetParameter(2) <<endl;
                    }
                    if (loopcount>5) {
                        cf_length[ll]->SetParameter(2, cf_length[ll]->GetParameter(2)/(TMath::Power(rmsfactor,6)));
                        a=oria;
                        b=2*a*cf_length[ll]->GetParameter(1);
                        cf_length[ll]->SetParameter(5,a);
                        cf_length[ll]->SetParameter(4,-b);
                        cf_length[ll]->SetParameter(3,b*b/(4*a));
                        cf_length[ll]->SetParameter(0,cf_length[ll]->GetParameter(0)/cf_length[ll]->GetMaximum());

                        cf_angle[ll]->SetParameter(2, cf_angle[ll]->GetParameter(2)/(TMath::Power(rmsfactor,6)));
                        cf_angle[ll]->SetParameter(0,cf_angle[ll]->GetParameter(0)/cf_angle[ll]->GetMaximum());

                        if (verbose) {
                            cout << " No Match found; did " << loopcount << " iterations. Giving up on this point. " ;
                            cout << "Reseting RMS cf_length[" << ll << "] = " << cf_length[ll]->GetParameter(2) <<endl;
                        }
                        if ( (! setstone[ll-1] )&&(ll>0)&&(springattempt[ll-1]<5)) {
                            if (verbose) {
                                cout << " Going back to ll = " << ll-1 << "." ;
                                cout << endl;
                                cout << " Suspicion that previous point was wrong :: " << xx[ll] << " and " << yy[ll];
                                cout << ", after segupdate, original point was: " << xxx << " and " << yyy << endl;
                            }
                            maxbin=input[k]->FindBin(xx[ll],yy[ll]);
                            input[k]->GetBinXYZ( maxbin, xbin2, ybin2, zbin2);
                            for (ii=-2; ii<=2; ii++) {
                                for ( jj=-2; jj<=2; jj++) {
                                    penalty[k]->SetBinContent(xbin2+ii,ybin2+jj,-1000);
                                }
                            }

                            if (TMath::Sqrt(TMath::Power(xxx-xx[ll],2)+TMath::Power(yyy-yy[ll],2)) < 0.035 ) {
                                if (verbose) {
                                    cout << " NEED TO UPDATE PENALTY !!! " << endl;
                                }
                                maxbin=input[k]->FindBin(xxx,yyy);
                                input[k]->GetBinXYZ( maxbin, xbin2, ybin2, zbin2);
                                for (ii=-2; ii<=2; ii++) {
                                    for ( jj=-2; jj<=2; jj++) {
                                        penalty[k]->SetBinContent(xbin2+ii,ybin2+jj,-1000);
                                    }
                                }

                            }

                            if (verbose) {
                                cout << " with xbin = " << xbin2 << ", and ybin = " << ybin2 << endl;
                            }
                            ll--;
                            springattempt[ll]++;
                            if (verbose) {
                                cout << " spring " << ll << " attempt : " << springattempt[ll] << endl;
                            }
                            ll--;
                        } else {
                            if (verbose) {
                                cout << " We can't go back to " << ll-1 ;
                                cout << " have to think about something else. Setting FORCEPOINT" << endl;
                            }
                            ll--;
                            // think about getting the max value anyway; we will take our first point
                            penalty[k]->Reset();
                            forcepoint=1;
                        }
                        if (verbose) {
                            cout << " stepback = " << stepback << endl;
                        }

                        // it may be that the entire previous cluster was wrong, need to exclude the previous point somehow

                        stepback++;

                        if (stepback>2) {
                            if (verbose) {
                                cout << "let's just break here " << endl;
                            }
                            break;
                        }
                        break;
                    }
                }

            }  // loop ll

            if (verbose) {
                cout << " graph " << k <<" :: " << endl;
            }
            Double_t xtmp,ytmp;
            // TGraph *points = new TGraph(16);
            if (verbose) {
                for (ll=0; ll<16; ll++) {
                    cout << ll << " " << xx[ll] << " " << yy[ll] ;
                    peaks_remapped[k]->GetPoint(ll,xtmp,ytmp);
                    cout << " " << xtmp << " " <<ytmp<< endl;
                }
                cout << "Totcost quadrant " << k << " = " << totcost[k] <<endl;
            }
            if (totcost[k] > COSTTHRESHOLD )  {
                validflag |= ( 1 << k) ;
                if (nosegupdate==0) {
                    updatesegmentation(input[k],xx,yy,peaks_remapped[k],verbose);
                }
            }

            /* check overlap */
            for (ll=1; ll<16; ll++) {
                if (TMath::Sqrt(TMath::Power(xx[ll]-xx[ll-1],2)+TMath::Power(yy[ll]-yy[ll-1],2)) < 0.01 ) {
                    if (verbose) {
                        cout << " Point " << ll-1 << " and " << ll << " overlap !" <<endl;
                    }
                    totcost[k]=-99.99;
                }
            }

            if (totcost[k] < cost) {
                cost=totcost[k];
            }
        }
#endif
    }

    if (verbose) {
        cout << " -------> VALIDFLAG = " << validflag <<endl;
    }

    TGraph *peaks_sorted = new TGraph(64);
    peaks_sorted->SetName("peaks_sorted");
    mergegraphs(peaks_remapped,peaks_sorted);


    c1->Clear();
    c1->Divide(2,3)  ;
    c1->SetCanvasSize(600,900);

    for (int k=0; k<4; k++) {
        //    cout << "Totcost quadrant " << k << " = " << totcost[k] <<endl;
        c1->cd(k+3);
        floodsq[k]->Draw("colzhist");
        peaks_remapped[k]->Draw("PL");
    }

    c1->cd(1);
    flood->Draw("colz");
    peaks_sorted->Draw("PL");

    c1->cd(2);
    flood->Draw("colz");
    flood->Draw("colz");
    lineX->Draw();
    lineY->Draw();


    sprintf(pngstring,"%s/%s.quadrants.png",fDIR.Data(),filebase);
    c1->Update();
    c1->Print(pngstring);


    for (int k=0; k<4; k++) {
        delete floodsq[k];
    }

    return peaks_sorted;
}

Double_t findmean8(
        TH1D * h1,
        Int_t verbose,
        Int_t X,
        Bool_t APD)
{
    TSpectrum s;
    Int_t nfound;
    Float_t *xx,*yy;
    Int_t ind[8];
    nfound = s.Search(h1);

    if (nfound != 8) {
        nfound = s.Search(h1, 1.5, "", 0.05);
    }

    if (verbose) {
        cout << nfound << " peaks found in findmean8 " << endl;
    }
    if (nfound == 8) {
        xx = s.GetPositionX();
        yy = s.GetPositionY();
        TMath::Sort(nfound,xx,ind);
        if (verbose) {
            cout << " Central peaks at : " << xx[ind[3]] << " (height = " << yy[ind[3]] << ") and " ;
            cout << xx[ind[4]] << " (height = " << yy[ind[4]] << ")" << endl;
        }
        Double_t mean = (Double_t) (xx[ind[4]] + xx[ind[3]]) / 2.;
        if (verbose) {
            cout << " mean at :: " << mean << endl;
        }
        return mean;
    }

    // NOTE !!!
    // the y-coordinate is always slightly below 0 ! due to some remaining asymmetry
    // DO NOT FORGET THIS !!!
    // HOWEVER, FOR APD 1 it is slightly ABOVE 0, due to the mirroring

    if (verbose) {
        cout << "FINDMEAN8 FAILED !! " << endl;
    }
    if ((nfound < 3) || (nfound > 11)) {
        return 12345;
    }
    // let's try to calculate the distances between any two peaks:
    xx = s.GetPositionX();
    yy = s.GetPositionY();
    Int_t ind2[12];
    TMath::Sort(nfound,xx,ind2);

    Float_t meanloc[12],absmeanloc[12];
    if (verbose)  {
        cout << " nfound = " << nfound << endl;
        for (int i = 0; i < nfound; i++) {
            cout << " Peak " << ind2[i] << " at  " << xx[ind2[i]] << endl;
        }
    }
    for (int i = 0; i < (nfound - 1); i++) {
        meanloc[i]=xx[ind2[i+1]] - ( xx[ind2[i+1]]-xx[ind2[i]])/ 2.;
        if (verbose) {
            cout << " middle between " << i+1 << " and " << i << " : " << meanloc[i] << endl;
        }
        absmeanloc[i] = TMath::Abs(meanloc[i]);
    }
    TMath::Sort(nfound-1,absmeanloc,ind2,kFALSE);
    if (X != 0) {
        if (verbose) {
            cout << "looking at X" << endl;
        }
        return meanloc[ind2[0]];
    }

    if (verbose) {
        cout << "looking at Y" << endl;
    }
    if (APD) {
        int i = 0;
        while (meanloc[ind2[i]]<0.0075) {
            if (verbose) {
                cout <<  " candidate :: " << meanloc[ind2[i]] << endl;
            }
            if ( i< ( nfound-2) ) {
                i++;
            } else {
                return 12345;
            }
        }
        if ( verbose ) {
            cout << " returning : " << meanloc[ind2[i]] << endl;
        }
        return meanloc[ind2[i]];
    } else {
        int i = 0;
        while (meanloc[ind2[i]]>0) {
            if (verbose) {
                cout <<  " candidate :: " << meanloc[ind2[i]] << endl;
            }
            if ( i< ( nfound-2) ) {
                i++;
            } else {
                return 12345;
            }
        }
        if ( verbose ) {
            cout << " returning : " << meanloc[ind2[i]] << endl;
        }
        return meanloc[ind2[i]];
    }
}

Double_t findmean(TH1D * h1, Int_t verbose, Bool_t APD) {
    Double_t leftmax = 0;
    Int_t leftbinmax = 0;
    Double_t rightmax = 0;
    Int_t rightbinmax = 0;
    h1->SetAxisRange(-.25,.25);
    Int_t meanbin = h1->FindBin(h1->GetMean());
    normalise_1(h1);

    Int_t initialbin = h1->FindBin(0);

    //we want to start from a low value ::
    if (verbose)  {
        cout << "looking left; initialbin ::" << initialbin << " at position " ;
        cout << h1->GetBinCenter(initialbin) << " value: " << h1->GetBinContent(initialbin) << endl;
        cout << "meanbin :: " << meanbin << " at position " << h1->GetBinCenter(meanbin) ;
        cout << " value : " << h1->GetBinContent(meanbin) << endl;
    }
    if ( h1->GetBinContent(meanbin) < h1->GetBinContent(initialbin)) {
        initialbin=meanbin;
    }
    Double_t curval=0;
    //start a little right from zero and go left
    for (int i = initialbin; i > (initialbin - 35); i--) {
        Double_t prevval = curval;
        if (verbose) {
            cout << " i = " << i << " Content: " << h1->GetBinContent(i) << endl;
        }
        if (h1->GetBinContent(i) > leftmax) {
            leftbinmax = i;
            leftmax = h1->GetBinContent(i);
        }
        curval = h1->GetBinContent(i);
        // when the gradient becomes largely negative, stop the iteration except for the first 5 bins.
        if ((curval-prevval)<-.25) {
            if (i<(initialbin-5)) {
                i=initialbin-100;
            }
        }
    }
    if (verbose) {
        cout << "looking right; initialbin ::" << initialbin << endl;
    }
    curval=0;
    for (int i = initialbin; i < (initialbin + 35); i++) {
        Double_t prevval = curval;
        if (verbose) {
            cout << " i = " << i << " Content: " << h1->GetBinContent(i) << endl;
        }
        if (h1->GetBinContent(i) > rightmax) {
            rightbinmax = i;
            rightmax = h1->GetBinContent(i);
        }
        curval = h1->GetBinContent(i);
        if ((curval-prevval)<-.25) {
            if (i>(initialbin+5)) {
                i=initialbin+100;
            }
        }
    }
    if (verbose) {
        cout << " leftmax = " << leftmax << " @ " << h1->GetBinCenter(leftbinmax);
        cout << " rightmax = " << rightmax << " @ " << h1->GetBinCenter(rightbinmax);
        cout <<endl;
    }
    Double_t mean = (h1->GetBinCenter(rightbinmax) +
                     h1->GetBinCenter(leftbinmax)) / 2.0;

    return mean;
}

Int_t validate_sort(TGraph *gr, Int_t verbose)
{
    Double_t angs[15];
    Double_t angs_rms[15];

    angs[0] = 1.53504;
    angs[1] = 1.50076;
    angs[2] = 1.43341;
    angs[3] = 0.0217294;
    angs[4] = 0.0792454;
    angs[5] = 0.194861;
    angs[6] = 0.657383;
    angs[7] = 1.34685;
    angs[8] = 1.29371;
    angs[9] = 0.215902;
    angs[10] = 0.414625;
    angs[11] = 0.81231;
    angs[12] = 1.23019;
    angs[13] = 0.619622;
    angs[14] = 1.04278;

    angs_rms[0] = 0.0957415;
    angs_rms[1] = 0.11;
    angs_rms[2] = 0.1628848;
    angs_rms[3] = 0.062414;
    angs_rms[4] = 0.095;
    angs_rms[5] = 0.19;
    angs_rms[6] = 0.096456;
    angs_rms[7] = 0.095;
    angs_rms[8] = 0.15;
    angs_rms[9] = 0.10;
    angs_rms[10] = 0.181;
    angs_rms[11] = 0.0759681;
    angs_rms[12] = 0.145;
    angs_rms[13] = 0.153272;
    angs_rms[14] = 0.1552269;

    Double_t angle[15];
    Double_t x1, y1, x2, y2;
    Int_t curcorner,point1,point2;
    Int_t k = 0;
    curcorner=0;
    for (int i = 0; i < 3; i++) {
        for (int l = 0; l < 2; l++) {
            for (int j = 0; j < (3 - i); j++) {
                if ((l == 1) && (j == 0)) {
                    point1 = curcorner + j;
                } else {
                    point1 = curcorner + j + l * (3 - i);
                }
                point2 = curcorner + j + 1 + l * (3 - i);
                gr->GetPoint(point1,x1,y1);
                gr->GetPoint(point2,x2,y2);
                angle[k]= TMath::ATan2(y2-y1,x2-x1);
                k++;
            }
        }
        point1 = curcorner;
        point2 = curcorner + (7 - i * 2);
        gr->GetPoint(point1, x1, y1);
        gr->GetPoint(point2, x2, y2);
        angle[k] = TMath::ATan2(y2 - y1, x2 - x1);
        k++;
        curcorner = point2;
    }

// now calculate the deviation

// option one: calculate an average deviation

//option two : calculate threshold for every value
    for (int i = 0; i < 15; i++) {
        if (TMath::Abs(angle[i] - angs[i])  > 3.5 * angs_rms[i]) {
            if (verbose) {
                cout << " Graph  failed at angle " << i ;
                cout << ": angle = " << angle[i] << " reference = " << angs[i];
                cout << "  3.5*RMS = " << 3.5 * angs_rms[i] << endl;
            }
            return -1;
        }
    }
    return 0;
}


// EXPERIMENTAL !!
Int_t updatesegmentation(
        TH2F *hist,
        Double_t x[16],
        Double_t y[16],
        TGraph *updatedgraph,
        Int_t verbose)
{
    // hist is our histogram, start by cloning it
    if (verbose) {
        cout << " Updating Histogram Segmentation "  << endl ;
    }
    Int_t updated[16];
    Float_t xxx[16],yyy[16];
#define THRESHOLDS 3
    Float_t thresholds[THRESHOLDS]= {0.1,0.2,0.3};
    Int_t maxcluster[THRESHOLDS]= {75,35,30};
    Int_t xbin,ybin,zbin;
    bin *clusr = new bin;
    for (int i = 0; i < 16; i++) {
        updated[i]=0;
    }
    for (int j = 0; j < THRESHOLDS; j++) {
        if (verbose) {
            cout << " Iteration " << j << endl;
        }
        hist_threshold(hist,thresholds[j]);
        for (int i = 0; i < 16; i++) {
            hist->GetBinXYZ(hist->FindBin(x[i],y[i]),xbin,ybin,zbin);
            ClusterSize(hist,xbin,ybin,clusr);
            if ((clusr->pixels<maxcluster[j])&&(updated[i]==0)&&(clusr->pixels>5)) {
                xxx[i]=hist->GetXaxis()->GetBinCenter(clusr->x);
                yyy[i]=hist->GetYaxis()->GetBinCenter(clusr->y);
                if (verbose)  {
                    cout << " candidate point:  (" << xxx[i] << "," << yyy[i] << ")";
                    cout << " distance to original : (" << x[i] << "," << y[i] << ") :";
                    cout <<  TMath::Sqrt(TMath::Power(xxx[i]-x[i],2)+TMath::Power(yyy[i]-y[i],2)) << endl;
                }
                if ( TMath::Sqrt(TMath::Power(xxx[i]-x[i],2)+TMath::Power(yyy[i]-y[i],2)) < 0.035 ) {
                    updated[i]=1;
                    if (verbose) {
                        cout << " updating point " << i << " to (" << xxx[i] << "," << yyy[i] << ")" ;
                        cout << " cluster size :  " << clusr->pixels  << endl;
                    }
                }
            }
        }
    }
    for (int i = 0; i < 16; i++)  {
        if (updated[i]==0) {
            // if not updated, use originals
            if (verbose) {
                cout << " Point " << i <<  " at (" << x[i] << "," << y[i] << ") not updated." << endl;
            }
            xxx[i]=x[i];
            yyy[i]=y[i];
        }
        updatedgraph->SetPoint(i,xxx[i],yyy[i]);
    }
    return 0;
}



// EXPERIMENTAL !!
Int_t updatesegpoint(TH2F *hist, Double_t x, Double_t y, Double_t &xxx, Double_t &yyy,Int_t verbose)
{
    // hist is our histogram, start by cloning it

    if (verbose) {
        cout << " ==== Welcome to Update Seg Point === " << endl;
        cout << "     original point :: x= " << x <<", y=" << y << endl;
    }

    Int_t updated;
    Int_t j;
#define THRESHOLDS2 4
    Float_t thresholds[THRESHOLDS2]= {0.1,0.2,0.3,0.35};
    Int_t maxcluster[THRESHOLDS2]= {65,90,65,25};
    Int_t xbin,ybin,zbin;
    bin *clusr = new bin;
    updated=0;
    for (j=0; j<THRESHOLDS2; j++) {
        if (verbose) {
            cout << " Iteration " << j << " : ";
        }
        hist_threshold(hist,thresholds[j]);
        //   for (i=0;i<16;i++) {
        hist->GetBinXYZ(hist->FindBin(x,y),xbin,ybin,zbin);
        ClusterSize(hist,xbin,ybin,clusr);
        if (verbose)      {
            cout << " cluster size :  " << clusr->pixels ;
            cout << " center at (" << hist->GetXaxis()->GetBinCenter(clusr->x) << "," ;
            cout <<  hist->GetYaxis()->GetBinCenter(clusr->y) << ")."  << endl;
        }
        if ((clusr->pixels<maxcluster[j])&&(updated==0)&&(clusr->pixels>2)) {
            xxx=hist->GetXaxis()->GetBinCenter(clusr->x);
            yyy=hist->GetYaxis()->GetBinCenter(clusr->y);
            if (verbose)  {
                cout << " candidate point:  (" << xxx << "," << yyy << ")";
                cout << " distance to original : (" << x << "," << y << ") :";
                cout <<  TMath::Sqrt(TMath::Power(xxx-x,2)+TMath::Power(yyy-y,2)) << endl;
            }
            if ( TMath::Sqrt(TMath::Power(xxx-x,2)+TMath::Power(yyy-y,2)) < 0.035 ) {
                updated=1;
                if (verbose) {
                    cout << " updating point  to (" << xxx << "," << yyy << ")" << endl;
                }
            }
        }
    }
    if (updated==0) {
        // if not updated, use originals
        if (verbose) {
            cout << " Point at (" << x << "," << y << ") not updated." << endl;
        }
        xxx=x;
        yyy=y;
    }
    return 0;
}
