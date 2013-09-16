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
//#include "apd_sort.h"
#include "THistPainter.h"
#include "TPaletteAxis.h"
#include "TMath.h"

#include "./decoder.h"
#include "./DetCluster.h"
#include "./FindNext.h"

//#include "./convertconfig.h"

#include "Apd_Sort16_v2.h"
 
//#define CHIPS 2
//#define DEVELOP
// wil generate some more eps style floods
//#define PUBPLOT

// comment this if you don't want the additional Pictorial Structure based search.
#define sortps



//comment this if you don't want the ps file created
//#define DRAWSEG

//comment this if you don't want the traditional 16 peak search.
#define SRT16

// comment this if you don't want timing
//#define TIMING
#ifdef TIMING
#include "time.h"
#endif

TGraph *PeakSearch(TH2F *flood,Char_t filebase[200],Int_t verbose,Int_t &validflag,Float_t &cost, Bool_t APD);
Double_t findmean( TH1D * h1, Int_t verbose, Bool_t APD);
Double_t findmean8( TH1D * h1, Int_t verbose,Int_t X, Bool_t APD);
Int_t updatesegmentation(TH2F *hist, Double_t x[16], Double_t y[16], TGraph *updated,Int_t verbose);
Int_t updatesegpoint(TH2F *hist, Double_t x, Double_t y, Double_t &xxx, Double_t &yyy, Int_t verbose) ;
Int_t validate_sort(TGraph *gr, Int_t verbose);

Int_t main(int argc, Char_t *argv[])
{


  cout << " Welcome to anafloods. Program performs Segmentation.  " ; 

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	Char_t		filename[FILENAMELENGTH] = "";
	Int_t		verbose = 0;
	Int_t		ix;
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
				sprintf(filename, "%s", argv[ix + 1]);
			}
			else {
				cout << "Filename " << argv[ix + 1] << " too long !" << endl;
				cout << "Exiting.." << endl;
				return -99;
			}
		}
	}

	cout << " Inputfile : " << filename << "." << endl; 

        rootlogon(verbose);
        set2dcolor(4);



	if (!verbose) {
	gErrorIgnoreLevel = kBreak;
		}
       

	TFile *rfile = new TFile(filename,"OPEN");
         if (!rfile || rfile->IsZombie()) {
            cout << "problems opening file " << filename << ". Exiting." << endl;
            return -11;}
   
	/*
        TTree *block1 = (TTree *) rfile->Get("block1");
        cout << " Entries block1 : " << block1->GetEntries() << endl;
        TTree *block2 = (TTree *) rfile->Get("block2");
        cout << " Entries block2 : " << block2->GetEntries() << endl;
	*/

        TH1F *E[RENACHIPS][4][2];
        TH2F *floods[RENACHIPS][4][2];
        Char_t tmpstring[60]; 
        Char_t filebase[FILENAMELENGTH],pngstring[FILENAMELENGTH];
        ofstream peaklocationfile;
        Char_t peaklocationfilename[FILENAMELENGTH];
        Int_t k,i,j,m;
        TCanvas *c1;
	//        TSpectrum2 *s = new TSpectrum2(); 
        Int_t chipevents[RENACHIPS];
        Bool_t modevents[RENACHIPS][MODULES][2];

        strncpy(filebase,filename,strlen(filename)-5);
        filebase[strlen(filename)-5]='\0';
	if (verbose)        cout << " filebase = " << filebase << endl;


        c1 = (TCanvas*)gROOT->GetListOfCanvases()->FindObject("c1");
        if (!c1) c1 = new TCanvas("c1","c1",10,10,1000,1000);
        c1->SetCanvasSize(700,700);



	// DON'T CHANGE THESE HERE !!

	/* Read in the flood and energy histograms */
	for (m=0;m<RENACHIPS;m++){
        if (verbose)   cout << " Analyzing chip " << m << endl;
            chipevents[m]=0;
       for (j=0;j<2;j++){
         if (verbose)   cout << " Analyzing apd " << j << endl;
	 //	 c1->Clear();
	 // c1->Divide(2,2); 
        for (i=0;i<4;i++){
	  sprintf(tmpstring,"E[%d][%d][%d]",m,i,j);
            E[m][i][j]= (TH1F *) rfile->Get(tmpstring);
            if (!(E[m][i][j])){ 
	      if(verbose){ cout << " did not find Energy for chip " << m << " apd " << j <<" unit " << i << endl;}
                                continue; }
            if ( E[m][i][j]->GetEntries() > MINHISTENTRIES ) modevents[m][i][j]=true;
            else modevents[m][i][j]=false;
	    if (verbose) {cout << " E hist read; " ;}
	    sprintf(tmpstring,"floods[%d][%d][%d]",m,i,j);
		 if (verbose) { cout << " -- Getting TH2F :: ( i = " << i << ", j = " << j << ") --" ;}
            floods[m][i][j]= (TH2F *) rfile->Get(tmpstring);
	    //   if (!(floods[i][j])) floods[i][j] = new TH2F();
	    if (verbose) cout << " flood hist read; " <<endl;
        }
       } // j - loop
	} // m-loop


	TVector* ppVals = (TVector *) rfile->Get("pp_spat;1");
	 //       TVector* ppVals_com = (TVector *) rfile->Get("pp_com;1");

       if (!(ppVals)) cout << "Error :: ppVals not found " <<endl;
       //       if (!(ppVals_com)) cout << "Error :: ppVals_com not found " <<endl;
       //Get("ppVals");

       // cout << "ppvals :: " << (*ppVals)(1) << endl; 

 Double_t pp_low,pp_up;
 Double_t *xsort,*ysort;
 TGraph *peaks_remapped64;
 peaks_remapped64 = new TGraph(PEAKS);
 Int_t nvalid=0;
 Bool_t  validsegment[RENACHIPS][4][2];
 Float_t costs[RENACHIPS][4][2];
 Int_t nfound;
 TGraph *peaks[RENACHIPS][4][2];
 Int_t flag=0;
 #ifdef TIMING
 time_t starttime,startloop,endloop;
 time_t seconds;
#endif  

      for (m=0;m<RENACHIPS;m++){
       for (i=0;i<4;i++){ 
         for (j=0;j<2;j++){
            validsegment[m][i][j]=0; }}}


       for (m=0;m<RENACHIPS;m++){
      //                     for (m=6;m<7;m++){
           // m=6;{
		 // m=5;{
	  //         if (!chipevents[m]) continue;
         if (verbose)  cout << " Analyzing chip .. " << m << endl;   

          for (i=0;i<4;i++){
	      // i=2;{
          for (j=0;j<2;j++){
	 //  i=2; j=0; {{
#ifdef TIMING
      startloop = time(NULL);    
#endif

      if (modevents[m][i][j]){
  	 c1->Clear();
	 c1->Divide(2,2);
	 if (verbose)        cout << " Crystal segmentation Unit " << i << ",  apd " << j << endl;

					     //	i=2;{		
			 //	 i=0;{
	 //	   for (k=0;k<64;k++) { peaks_remapped64->SetPoint(k,0,0);
	     //             test1d->SetPoint(k,0,0);}
          pp_low=250;
          pp_up=2500;
  
        if (verbose) { 
            cout << " ********************************************************* "  << endl; 
            cout << " * Determining Crystal Locations  CHIP " << m << " UNIT " << i << " APD " << j << " *"<<endl;
            cout << " ********************************************************* "  << endl;  
            cout << " Entries Ehist :: " << E[m][i][j]->GetEntries() ;
            cout << " Entries Floods :: " << floods[m][i][j]->GetEntries() ;
 
       } 
         
     /* Look for the 64 crystal positions */
     nfound=0;


     //  m=0;

     sprintf(peaklocationfilename,"%s.RENA%d.unit%d_apd%d",filebase,m,i,j);

#ifdef TIMING
     starttime = time (NULL );
#endif
     //   cout << " Starttime : " << starttime;
     //     verbose=0;
     peaks[m][i][j] =   PeakSearch(floods[m][i][j],peaklocationfilename,verbose, flag, costs[m][i][j],j);
     //     verbose=1;
      }
      else{
	if (verbose) {
          cout << " --------- Skipping CHIP " << m << " MODULE " << i << " APD " << j << " ----------- " << endl;
          cout << " ------------- Not enough events ------------------------- " << endl;
	} //verbose
          peaks[m][i][j] = new TGraph(64);
      } // else - modevents[m][i][j]
     sprintf(tmpstring,"peaks[%d][%d][%d]",m,i,j);
     peaks[m][i][j]->SetName(tmpstring);
#ifdef TIMING
     seconds = time (NULL);
     cout << " time to finish PeakSearch : " << seconds-starttime << endl;
#endif

     if (verbose){     cout << "CHECK FLAG == " << flag <<  endl;}
     /* NEED TO REWRITE ALL THIS CODE :: */

  c1->Clear();
  //  set2dcolor(4);
  c1->Divide(2,2);
   
   c1->cd(1);
   floods[m][i][j]->Draw("colz");
   c1->cd(2);floods[m][i][j]->Draw("histcolz");
   c1->cd(3);peaks[m][i][j]->Draw("APL");
    c1->cd(4);floods[m][i][j]->Draw("colz");
    peaks[m][i][j]->Draw("PL");	
    
    if (flag==15) { 
      validsegment[m][i][j]=1;nvalid++;
      sprintf(peaklocationfilename,"%s.RENA%d.unit%d_apd%d_peaks",filebase,m,i,j);
      }
    else  {
      validsegment[m][i][j]=0; 
      sprintf(peaklocationfilename,"%s.RENA%d.unit%d_apd%d_peaks.failed",filebase,m,i,j);
    }
   strcat(peaklocationfilename,".txt");
     peaklocationfile.open(peaklocationfilename);

    xsort = peaks[m][i][j]->GetX();
    ysort = peaks[m][i][j]->GetY();
   for (k=0;k<PEAKS;k++){ 
     peaklocationfile << k << " " << xsort[k] << " " << ysort[k] <<endl;    }
    
   peaklocationfile.close();
 

   //	 }// i
	 //  } //j
       //    c1->Clear();
       //   floods[i][j]->Draw("colz");
       // }
   if (verbose) { cout <<" Made it here too! " <<endl;}
   sprintf(pngstring,"%s.RENA%d.unit%d_apd%d_flood",filebase,m,i,j);
   strcat(pngstring,".png");
   //   cout << pngstring << endl;
   c1->Update();
   c1->Print(pngstring);
#ifdef PUBPLOT
   set2dcolor(4);
   c1->Clear();
  sprintf(pngstring,"%s.RENA%d.unit%d_apd%d_flood",filebase,m,i,j);
   strcat(pngstring,".eps");
floods[m][i][j]->Draw("colz");
 peaks[m][i][j]->Draw("PL");	
   c1->Print(pngstring);
   sprintf(pngstring,"%s.RENA%d.unit%d_apd%d_singleflood",filebase,m,i,j);
   strcat(pngstring,".eps");
 TPaletteAxis *palette = (TPaletteAxis*)floods[m][i][j]->GetListOfFunctions()->FindObject("palette");
 palette->SetLabelSize(0.04);
  floods[m][i][j]->Draw("colz");
  palette->Draw();
   c1->Print(pngstring);
#endif
#ifdef TIMING
   endloop = time(NULL);
   cout << " time to finish loop      : " << endloop-startloop << " time to finish all but Peaksearch: " << endloop-startloop-seconds+starttime <<endl;
#endif
	    } //loop over j
       } //loop over i
      }//look over m
   

   sprintf(peaklocationfilename,"%s.segsum.txt",filebase);
   peaklocationfile.open(peaklocationfilename);

  peaklocationfile << "\n*****************************************************"  << endl;
  peaklocationfile << "* " <<  nvalid << " Valid segmentations for " << RENACHIPS*8 << " APDs:                *" << endl;
  for (m=0;m<RENACHIPS;m++){
   for (i=0;i<4;i++){ 
    for (j=0;j<2;j++){
      if (validsegment[m][i][j]==1) { 
	peaklocationfile << "* RENA " << m << " UNIT " <<  i  << " APD " << j << "  "  << validsegment[m][i][j] << "  (min cost: " << costs[m][i][j] <<" )      *" <<endl;
      }}}}
  peaklocationfile << "*                                                   *"  << endl;
  peaklocationfile << "*****************************************************"  << endl;
  peaklocationfile.close();
	rfile->Close();


	return 0;}



TGraph *PeakSearch(TH2F *flood,Char_t filebase[200],Int_t verbose,Int_t &validflag,Float_t &cost, Bool_t APD){

  cost=999.999;

  /* Step one: split in four */
   TH2F *floodsq[4];
   TH1D *projX;
   TH1D *projY;
   Float_t meanX,meanY;
   Int_t midbinx,midbiny,binsx,binsy,curbin,newbin;
   Char_t tmpstr[40],tmpstr2[220];
   Char_t pngstring[220];
   Double_t xval,yval,value;

   /* find mean based on projection*/
   if (verbose) cout << " welcome to PeakSearch " << endl;
   projX = (TH1D*) flood->ProjectionX("",120,136);
   projX->SetName("projX");
   projY = (TH1D*) flood->ProjectionY("",120,136);
   projY->SetName("projY");

   if (verbose)     cout << " finding X center " << endl;
   meanX = findmean8(projX,verbose,1,APD);
     if (verbose)  cout << " finding Y center " << endl;
     meanY = findmean8(projY,verbose,0,APD);

#ifdef DEVELOP
   TFile *f = new TFile("proj.root","RECREATE");
   projX->Write();
   projY->Write();
   f->Close();
#endif

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
   midbinx=projX->FindBin(meanX);
   midbiny=projY->FindBin(meanY);


   if (verbose){
   cout << " midbinx  = "  << midbinx ;
   cout << " midbiny  = "  << midbiny <<endl;
   }
   


   TCanvas *c1;
   c1 = (TCanvas*)gROOT->GetListOfCanvases()->FindObject("c1");
   if (!c1) c1 = new TCanvas("c1","c1",10,10,1000,1000);
   c1->SetCanvasSize(700,700);
   c1->Clear();
   gStyle->SetOptDate(0);

#ifdef DEVELOP
   c1->Divide(2,1); c1->cd(1); projX->Draw();c1->cd(2); projY->Draw(); c1->Print("projection.png");
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
   c1->Clear();
   flood->Draw("colz"); //lineX->Draw(); lineY->Draw(); 
   c1->Print("split.ps");
#endif 
  
   Int_t i,j,k,l;

   /* creating the histograms */
  for (k=0;k<4;k++){
     sprintf(tmpstr,"floodsq[%d]",k);
     sprintf(tmpstr2,"%s, Quadrant %d",filebase,k);
     switch(k){
     case 0:
       binsx=(1-(meanX-(projX->GetBinWidth(1)/2.)))/projX->GetBinWidth(1);
       binsy=(1-(meanY-(projY->GetBinWidth(1)/2.)))/projY->GetBinWidth(1);
       if (verbose){
         cout << "Creating Histo with " << meanX-(projX->GetBinWidth(1)/2.);
         cout << " < X <  1 ( " <<  binsx << " bins ) and with ";
         cout << (meanY-(projY->GetBinWidth(1)/2.));
         cout << " < Y <  1 ( " <<  binsy << " bins )  " <<endl;}
       floodsq[k]= new TH2F(tmpstr,tmpstr2,binsx,meanX-(projX->GetBinWidth(1)/2.),1,binsy,(meanY-(projY->GetBinWidth(1)/2.)),1);
       break;
     case 1:
       binsx=(1-(meanX-(projX->GetBinWidth(1)/2.)))/projX->GetBinWidth(1);
       binsy=(1+(meanY-(projY->GetBinWidth(1)/2.)))/projY->GetBinWidth(1);
       if (verbose){
         cout << "Creating Histo with " << meanX-(projX->GetBinWidth(1)/2.);
         cout << " < X <  1 ( " <<  binsx << " bins ) and with ";
         cout << -(meanY-(projY->GetBinWidth(1)/2.));
         cout << " < Y <  1 ( " <<  binsy << " bins )  " <<endl;}
       floodsq[k]= new TH2F(tmpstr,tmpstr2,binsx,meanX-(projX->GetBinWidth(1)/2.),1,binsy,-(meanY-(projY->GetBinWidth(1)/2.)),1);
       break;
case 2:
       binsx=(1+(meanX-(projX->GetBinWidth(1)/2.)))/projX->GetBinWidth(1);
       binsy=(1+(meanY-(projY->GetBinWidth(1)/2.)))/projY->GetBinWidth(1);
       if (verbose){
         cout << "Creating Histo with " << -(meanX-(projX->GetBinWidth(1)/2.));
         cout << " < X <  1 ( " <<  binsx << " bins ) and with ";
         cout << -(meanY-(projY->GetBinWidth(1)/2.));
         cout << " < Y <  1 ( " <<  binsy << " bins )  " <<endl;}
       floodsq[k]= new TH2F(tmpstr,tmpstr2,binsx,-(meanX-(projX->GetBinWidth(1)/2.)),1,binsy,-(meanY-(projY->GetBinWidth(1)/2.)),1);
       break;
     case 3:
       binsx=(1+(meanX-(projX->GetBinWidth(1)/2.)))/projX->GetBinWidth(1);
       binsy=(1-(meanY-(projY->GetBinWidth(1)/2.)))/projY->GetBinWidth(1);
       if (verbose){
         cout << "Creating Histo with " << -(meanX-(projX->GetBinWidth(1)/2.));
         cout << " < X <  1 ( " <<  binsx << " bins ) and with ";
         cout << (meanY-(projY->GetBinWidth(1)/2.));
         cout << " < Y <  1 ( " <<  binsy << " bins )  " <<endl;}
       floodsq[k]= new TH2F(tmpstr,tmpstr2,binsx,-(meanX-(projX->GetBinWidth(1)/2.)),1,binsy,(meanY-(projY->GetBinWidth(1)/2.)),1);
       break;
     } //switch
  }  // loop k
  // FIXME :: yval should be yval= projY()->GetBinCenter(l); --> FIXED 5-3-2012

 for (k=0;k<(projX->GetNbinsX());k++){
     for (l=0;l< (projY->GetNbinsX());l++){
       if (k > midbinx) {
         if (l > midbiny) {
           xval = projX->GetBinCenter(k); 
           yval = projY->GetBinCenter(l); 
           curbin = flood->FindBin(xval,yval);
           value =  flood->GetBinContent(curbin);
           newbin = floodsq[0]->FindBin(xval,yval);
           //      cout << " x = " << xval << " y = " << yval;
           //      cout << " curbin = " << curbin << " newbin = " << newbin;
           //      cout << " value = " << value <<endl;
           floodsq[0]->SetBinContent(newbin,value);
           }
         else {
           xval = projX->GetBinCenter(k); 
           yval = projY->GetBinCenter(l); 
           curbin = flood->FindBin(xval,yval);
           value = flood->GetBinContent(curbin);
           newbin = floodsq[1]->FindBin(xval,-yval);
           floodsq[1]->SetBinContent(newbin,value);
         } 
       } // k > midbinx
       else {
         if (l > midbiny) {
           xval = projX->GetBinCenter(k); 
           yval = projY->GetBinCenter(l); 
           curbin = flood->FindBin(xval,yval);
           value =  flood->GetBinContent(curbin);
           newbin = floodsq[3]->FindBin(-xval,yval);
           //      cout << " x = " << xval << " y = " << yval;
           //      cout << " curbin = " << curbin << " newbin = " << newbin;
           //      cout << " value = " << value <<endl;
           floodsq[3]->SetBinContent(newbin,value);
           }
         else {
           xval = projX->GetBinCenter(k); 
           yval = projY->GetBinCenter(l); 
           curbin = flood->FindBin(xval,yval);
           value = flood->GetBinContent(curbin);
           newbin = floodsq[2]->FindBin(-xval,-yval);
           floodsq[2]->SetBinContent(newbin,value);
         } 
       } // else k
     } // loop l
 } // loop k

#ifdef DEVELOP
   TFile *ff = new TFile("floodq.root","RECREATE");
   for (k=0;k<4;k++){
     floodsq[k]->Write();
   }
   flood->Write();
   ff->Close();
#endif

 TSpectrum2 *peaks[4];
 Int_t npeaks_q[4]; 
 Double_t *xsort_q[4],*ysort_q[4];
 TGraph *peaks_remapped[4]; 
 for (k=0;k<4;k++){ peaks_remapped[k] = new TGraph(16); }
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
 // Float_t xcurr;
 // Float_t ycurr;
 Float_t anglematch;
 Float_t lengthmatch;
 Int_t m,start;
  Int_t nosegupdate;
Int_t setstone[15];
 Int_t springattempt[15];


  for (k=0;k<4;k++){
    sprintf(histname,"peaks_remapped[%d]",k);
    peaks_remapped[k]->SetName(histname);
       totcost[k]=0;
 }

genfuncs(cf_length,cf_angle);


 for (k=0;k<4;k++){ peaks[k] = new TSpectrum2();}
 for (k=0;k<4;k++){ 
    
   if (verbose) {
     cout << endl;
     cout << " ===============================================================" <<endl;
     cout << "               Searching Peaks in Quadrant " << k << endl;
     cout << " ===============================================================" <<endl;}

#ifdef SRT16
 
 // Loop over four quadrants 


   
   npeaks_q[k]=peaks[k]->Search(floodsq[k],1.2,"noMarkov",0.15);
   if (verbose) cout << " Peaks in quadrant " << k  << ": " << npeaks_q[k] <<endl;
   if (npeaks_q[k] != 16 ) { 
        npeaks_q[k]=peaks[k]->Search(floodsq[k],2.0,"noMarkov",0.1);
        if (verbose) cout << " New limits :: Peaks in quadrant " << k  << ": " << npeaks_q[k] <<endl;
    }

if (npeaks_q[k] != 16 ) { 
        npeaks_q[k]=peaks[k]->Search(floodsq[k],1.4,"noMarkov",0.15);
        if (verbose) cout << " New limits :: Peaks in quadrant " << k  << ": " << npeaks_q[k] <<endl;
    }

   if (npeaks_q[k] < 16 ) {  
        npeaks_q[k]=peaks[k]->Search(floodsq[k],1.1,"noMarkov",0.15);
        if (verbose) cout << " New limits :: Peaks in quadrant " << k  << ": " << npeaks_q[k] <<endl;
   }
   // if too many peaks, increase widht of peak searching algorithm 
   if (npeaks_q[k] > 16 ) { 
        npeaks_q[k]=peaks[k]->Search(floodsq[k],2.0,"noMarkov",0.15);
        if (verbose) cout << " New limits :: Peaks in quadrant " << k  << ": " << npeaks_q[k] <<endl;
    }
   if (npeaks_q[k] > 16 ) { 
        npeaks_q[k]=peaks[k]->Search(floodsq[k],3.0,"noMarkov",0.15);
        if (verbose) cout << " New limits :: Peaks in quadrant " << k  << ": " << npeaks_q[k] <<endl;
    }

   if (npeaks_q[k] > 16 ) { 
        npeaks_q[k]=peaks[k]->Search(floodsq[k],3.75,"noMarkov",0.15);
        if (verbose) cout << " New limits :: Peaks in quadrant " << k  << ": " << npeaks_q[k] <<endl;
    }

   if (npeaks_q[k] > 16 ) { 
        npeaks_q[k]=peaks[k]->Search(floodsq[k],3.75,"noMarkov",0.4);
        if (verbose) cout << " New limits :: Peaks in quadrant " << k  << ": " << npeaks_q[k] <<endl;
    }

   if (npeaks_q[k] != 16 ) { 
     npeaks_q[k]=peaks[k]->Search(floodsq[k],1.11,"noBackgroundnoMarkov",0.35);
        if (verbose) cout << " New limits :: Peaks in quadrant " << k  << ": " << npeaks_q[k] <<endl;
    }



   // sorting the peaks 

   if (npeaks_q[k] == 16 ){
       xpeaks_q[k]=peaks[k]->GetPositionX() ; 
       ypeaks_q[k]=peaks[k]->GetPositionY() ;
       if (verbose) {  cout << "Going to SORT16" <<endl;}
       //       cout << sort16( npeaks_q[k], xpeaks_q[k], ypeaks_q[k], &ysort_q[k], &xsort_q[k], peaks_remapped[k], verbose) << endl; 
       
       xcorner=floodsq[k]->GetXaxis()->GetBinCenter(0);
       ycorner=floodsq[k]->GetYaxis()->GetBinCenter(0);
        
       //        xcorner=0; ycorner=0;
       if (verbose) cout << " Flood corner :: " << xcorner << " " << ycorner << endl;
       sort16(xpeaks_q[k],ypeaks_q[k],&ysort_q[k],&xsort_q[k],peaks_remapped[k],xcorner,ycorner,verbose);
       if ( ! validate_sort(peaks_remapped[k],verbose) ) validflag |= ( 1 << k) ;
   } // if (npeaks == 16 )
   else { if (verbose) cout << " not enough peaks found in histogram " << k << endl;  } //invalid=1;}

 
#endif 

#ifdef sortps
   Double_t oria,a,b;
   oria=0;
   Int_t stepback=0;
   Int_t forcepoint=0;
   Int_t alloops=0;


   //   for (k=0;k<4;k++){
     if ((validflag&(1<<k))==0) {
       if (verbose)     cout << " Trying Pictorial Structures " << endl; 
       nosegupdate=0;
     input[k] = (TH2F *) floodsq[k]->Clone();
      sprintf(histname,"input[%d]",k);
      input[k]->SetName(histname);
      //   input[k]->Rebin2D(2,2);
     normalise_1(input[k]);
     hist_threshold(input[k],0.05) ;
     inp[k] = (TH2F *) floodsq[k]->Clone();
      sprintf(histname,"inp[%d]",k);
      inp[k]->SetName(histname);
      //   input[k]->Rebin2D(2,2);
     normalise_1(inp[k]);
     hist_threshold(inp[k],0.25) ;

     penalty[k] = (TH2F *) input[k]->Clone();
      sprintf(histname,"penalty[%d]",k);
      penalty[k]->SetName(histname);
      penalty[k]->Reset();
     binx=0;
     biny=0;
     for (i=0;i<15;i++){ setstone[i]=0; springattempt[i]=0;}
    j=1 ;
    

    // STEP 1: FIND THE LOWER LEFT CORNER //

    while (j<inp[k]->GetNbinsX()) { 
        for  (i=1;i<=j;i++) { 
	  if (verbose) cout <<  inp[k]->GetBinContent(i,j) <<  "  " << inp[k]->GetBinContent(j,i) << "  "; 
	    // .25 used to be 1e-10 
            if ((inp[k]->GetBinContent(i,j) > .25 )|| (inp[k]->GetBinContent(j,i) > .25 )) { 
              if (inp[k]->GetBinContent(i,j) > .25 ){ binx=i; biny=j;}
              else { binx=j; biny=i;}
              j=inp[k]->GetNbinsX(); 
              break; }}  
	if ( verbose) cout << endl;
              j++;} 
    cluster[k] = new bin;
    //    cluster[k] = &
    ClusterSize(inp[k], binx,biny, cluster[k]);
    if (cluster[k]->pixels<0) nosegupdate=1;
    if (verbose){
    cout << " Size of the cluster : " << cluster[k]->pixels << " @ " << cluster[k]->x <<","<<cluster[k]->y;
    cout <<  " = (" << inp[k]->GetXaxis()->GetBinCenter(cluster[k]->x) ;
    cout <<  "," <<  inp[k]->GetYaxis()->GetBinCenter(cluster[k]->y) << ")" <<endl;}
    //    gues[k]->SetName(histname);
    //    gues[k]->SetPoint(0,input[k]->GetXaxis()->GetBinCenter(cluster[k]->x), input[k]->GetYaxis()->GetBinCenter(cluster[k]->y));
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
    if (verbose){
    cout << " Bins :: " << costfunc[k]->GetNbinsX() << " ";
    cout << costfunc[k]->GetNbinsY() << endl;
    }
    //    cout << " List of Objects :: "<< endl;
    //    gROOT->GetListOfFunctions()->Print();
    c1->Clear(); c1->Divide(2,2);

    if (verbose) { cout << "setting the first point ::" <<endl; }
   updatepoint(input[k],0,cluster[k]->x,cluster[k]->y,xx,yy);

   peaks_remapped[k]->SetPoint(0,xx[0], yy[0]);
   //   set2dcolor(2);
   //  set2dcolor(4);
   //  cout << "looking for other points :" << endl;
   // STEP 2 :: MATCHING THE PATTERN 
     for (ll=0;ll<15;ll++) {
       alloops++;
       if (alloops > 50 ) { ll=20; totcost[k]=-9999; continue;}
       loopcount=0;
       while (1) {
	 if(verbose){ cout << " Getting Spring ll = " << ll << "; xxx = " << xxx << ", yyy = " << yyy << endl;}
	 loopcount++;
          for (i=1;i<=costfunc[k]->GetNbinsX();i++) { 
	    // COST  = DEFORM + MATCH 

              for (j=1;j<=costfunc[k]->GetNbinsY();j++) { 
                deformfunc[k]->SetBinContent(i,j,CalcCost(input[k],ll,i,j,xx,yy,cf_length,cf_angle));
		//    costfunc[k]->SetBinContent(i,j,deformfunc[k]->GetBinContent(i,j)+12*input[k]->GetBinContent(i,j));

	      /* higher threshold for points closer to the center */
      	if (ll<12){
	  if ( ( ll==2) || (ll==5) || (ll==8) || (ll==10 )) {
              if (input[k]->GetBinContent(i,j)>0.1) {
                  costfunc[k]->SetBinContent(i,j,deformfunc[k]->GetBinContent(i,j)+20*input[k]->GetBinContent(i,j));}
                else{ costfunc[k]->SetBinContent(i,j,0);}
	   
		}  else { 
	    // these thresholds should probably be dynamic
	      if (input[k]->GetBinContent(i,j)>0.3) {
                  costfunc[k]->SetBinContent(i,j,deformfunc[k]->GetBinContent(i,j)+15*input[k]->GetBinContent(i,j));}
                else{ costfunc[k]->SetBinContent(i,j,0);}
	     }  
          if (ll==11) {
	    // at l=11 we're pretty far down the array.
               if (input[k]->GetBinContent(i,j)>0.2) {
                  costfunc[k]->SetBinContent(i,j,deformfunc[k]->GetBinContent(i,j)+15*input[k]->GetBinContent(i,j));}
                else{ costfunc[k]->SetBinContent(i,j,0);}
	   
	  }
	}
		else {// case where (ll>=12)
	      if (input[k]->GetBinContent(i,j)>0.1) {
                  costfunc[k]->SetBinContent(i,j,deformfunc[k]->GetBinContent(i,j)+25*input[k]->GetBinContent(i,j));}
                else{ costfunc[k]->SetBinContent(i,j,0);}
		}//}
              
		//adding penalty 
	      	costfunc[k]->SetBinContent(i,j,costfunc[k]->GetBinContent(i,j)+penalty[k]->GetBinContent(i,j));

		/*
Some debugging Crap
		if (ll==11) { 
		  if ((i<55)&&(i>45)){
		    if ((j<50)&&(j>40)) {
	      cout << "pen[" << i <<","<<j<< "] = " << penalty[k]->GetBinContent(i,j) << " " ;
	      cout << endl; }}}
		*/

	      } //loop over j
	  } // loop over i 

          // WE HAVE NOW DEFINED OUR COSTFUNCTION !!
          // CHECKING FOR THE BEST MATCH !!!

	  //          costfunc[k]->GetXaxis()->SetRangeUser(0,0.75);
	  //          costfunc[k]->GetYaxis()->SetRangeUser(0,0.75);
	  //          if (k==2){ 
          //      cout << "estimating max cost func " << endl;
          maxbin = costfunc[k]->GetMaximumBin();  
	  for (m=0;m<MAXM;m++){
	    maxbins[m]=costfunc[k]->GetMaximumBin();
 	    maxbin_content[m]=costfunc[k]->GetBinContent(maxbins[m]);
            costfunc[k]->GetBinXYZ(maxbins[m],x_maxbin[m],y_maxbin[m],z_maxbin[m]) ;
	    //set it to zero so that in the next iteration we find another point.
            costfunc[k]->SetBinContent(maxbins[m],-100);
	  }
          start=ll;
          if ((ll==3)||(ll==6)) start=0;
          if ((ll==9)||(ll==11)) start=7;
          if ((ll==13)||(ll==14)) start=12;
          if (verbose){
	    cout << " k=" << k <<"; Checking " << MAXM ;
            cout << " highest bins for connection between point = " << ll+1 ;
           cout << " and point " << ll << " ( quadrant " << k << ")" << endl;}
	  for (m=0;m<MAXM;m++){
	    //we are going to plot the costfunc, so put the value back
            costfunc[k]->SetBinContent(maxbins[m],maxbin_content[m]);
            xmaxbin[m]=costfunc[k]->GetXaxis()->GetBinCenter(x_maxbin[m]);
            ymaxbin[m]=costfunc[k]->GetYaxis()->GetBinCenter(y_maxbin[m]);
            // calculate length and angle
            length=TMath::Sqrt( TMath::Power(xmaxbin[m]-xx[start],2)+TMath::Power(ymaxbin[m]-yy[start],2));
            angle=TMath::ATan2(ymaxbin[m]-yy[start],xmaxbin[m]-xx[start]);
	    //            lengthmatch=3*cf_length[ll]->Eval(length);
            lengthmatch=20*cf_length[ll]->Eval(length);
            // anglematch=3*cf_angle[k]->Eval(angle);
            anglematch=20*cf_angle[ll]->Eval(angle);
            vetoangle[m]=0;
	  // double check angle
          // THIS IS NOT RIGHT 60 degrees is too flexible, should be 70 !

          if ((ll==0)||(ll==1)||(ll==2)||(ll==7)||(ll==8)||(ll==12)) {
            // angle should be around 90 -> larger than 60 degrees 
	    //	    if ( verbose ) cout << " checking angle " << angle << " < " << TMath::Pi()/3. << endl;
            if (ll==12) { if (angle < 5.5*TMath::Pi()/18.) vetoangle[m]=1;}
	    else { if (angle <  6.5*TMath::Pi()/18.) vetoangle[m]=1;}
             
          }
          else { 
            if ((ll==3)||(ll==4)||(ll==5)||(ll==9)||(ll==10)||(ll==13)) {
	      if (angle > TMath::Pi()/6.) vetoangle[m]=1;
	      // angle should be around 0 ->  smaller than 30 degrees
	    }else{
              if (( angle > TMath::Pi()/3. ) || ( angle < 25*TMath::Pi()/180. )) vetoangle[m]=1;
	      // so ll==6 or ll==11 or ll==14, angle around 45 degrees 
	    }
	  }
            if (verbose){
            cout <<  m  << " :: " <<  maxbin_content[m] ;
            cout << " x: " << xmaxbin[m];
            cout << " y: " << ymaxbin[m];
            cout << " ; matchfunc = " << lengthmatch+anglematch << "(" << deformfunc[k]->GetBinContent(maxbins[m])  ;
            cout << " ,length = " << lengthmatch << ", angle = " << anglematch << "; l="<<length<<", a="<<angle<<")" ;
            cout << " ; valuematch = " << input[k]->GetBinContent(maxbins[m]);
            cout << " ; totmatch = " << input[k]->GetBinContent(maxbins[m])+(lengthmatch+anglematch);
            cout << " ; veto : " << vetoangle[m] ; 
            cout << endl;}
	  
          }
          m=0;
	  while (( deformfunc[k]->GetBinContent(maxbins[m]) < 9 )|| (vetoangle[m] )) {
	    m++; 
	    if ( m >= MAXM ) { m=0; maxbin_content[m]=29;  break;}}
 
          if (verbose) cout << " using point m = " << m << endl;

	  if ((maxbin_content[m]>30)||(forcepoint)) { stepback=0; 
	    if (forcepoint) {
	      if (verbose)   cout << " FORCEPOINT set, setstone[" << ll <<"] = 1" << endl;
               setstone[ll]=1;  forcepoint=0;
               while (vetoangle[m]){m++; if (m>MAXM) break;}
	       if (verbose) { cout << " first non vetoed point :  m = " << m << endl;}
               if (m>=MAXM) m=0;}
             
	    if (verbose) { cout << " Setting point " << m << " with value : " << maxbin_content[m];
	      cout <<  " xbin: " << x_maxbin[m] << ", ybin: " << y_maxbin[m] << endl;}
            maxbin=maxbins[m];

          totcost[k]+=costfunc[k]->GetBinContent(maxbin);
          costfunc[k]->GetBinXYZ( maxbin, xbin2, ybin2, zbin2); 
          //     cout << "updating point " << endl;

         updatepoint(input[k],ll+1,xbin2,ybin2,xx,yy);
	  for (i=1;i<=input[k]->GetNbinsX();i++) { 
              for (j=1;j<=input[k]->GetNbinsY();j++) { 
		updatehist[k]->SetBinContent(i,j,input[k]->GetBinContent(i,j));}}
	  if (nosegupdate==0)  updatesegpoint(updatehist[k],xx[ll+1],yy[ll+1],xxx,yyy,verbose); 
	            if ((TMath::Abs(xx[ll]-xxx)<0.005)&&(TMath::Abs(yy[ll]-yyy)<0.005)) {
		      if (verbose ) {
			cout << " not updating point (" << xxx << "," << yyy << "), too close to";
                        cout << " previous point (" << xx[ll] << "," << yy[ll] << ")." << endl;}
		    } else {
	    if (verbose)   cout << " overlap check passed, updating point " << endl;
	    ttt=xx[ll+1];
            xx[ll+1]=xxx;
            xxx=ttt;
            ttt=yy[ll+1];
            yy[ll+1]=yyy;
            yyy=ttt;
	   }
	  
          //          cout << " Setting grids and guesses :: " << endl;
          //    gues[k]->SetPoint(ll+1,xx[ll+1], yy[ll+1]);
          peaks_remapped[k]->SetPoint(ll+1,xx[ll+1], yy[ll+1]);
          //      cout << "ll +1 = " << ll+1 <<endl;
	  if (verbose){
	    //	  hist_threshold(costfunc[k],0.1);
	  hist_threshold(costfunc[k],0.1);
          c1->cd(1);  costfunc[k]->Draw("colz");
	  //	           c1->cd(2);  hist_threshold(deformfunc[k],-5.);deformfunc[k]->Draw("colz");
		    c1->cd(2);  hist_threshold(deformfunc[k],-150.);deformfunc[k]->Draw("colz");
          c1->cd(3);  input[k]->Draw("colz");
          c1->cd(4);  floodsq[k]->Draw("colzhist"); peaks_remapped[k]->Draw("PL");
 	  if (ll==0) sprintf(psfilename,"costfunc_q%d.ps(",k);
	  else if (ll==14) sprintf(psfilename,"costfunc_q%d.ps)",k);
	  else sprintf(psfilename,"costfunc_q%d.ps",k);
	  c1->Print(psfilename);
	  }
	  if (verbose) cout <<  " Resetting penalty " << endl;
          penalty[k]->Reset();
          if (ll==2) {for (ii=0;ii<3;ii++) { setstone[ii]=1;}}
          if (ll==5) {for (ii=3;ii<6;ii++) {setstone[ii]=1;}}
          if (ll==10) { for (ii=6;ii<11;ii++) {setstone[ii]=1;}}
          if (ll==13) { for (ii=11;ii<14;ii++) { setstone[ii]=1;}}
          if (ll==14) { setstone[14]=1;}
	  break;   }  // had a valid point

          rmsfactor=1.5;
          cf_length[ll]->SetParameter(2, cf_length[ll]->GetParameter(2)*rmsfactor);
          cf_angle[ll]->SetParameter(2, cf_angle[ll]->GetParameter(2)*rmsfactor);
	  //    cf_length[ll]->SetParameter(2, cf_length[ll]->GetParameter(2)*rmsfactor);
          cf_length[ll]->SetParameter(0,cf_length[ll]->GetParameter(0)/cf_length[ll]->GetMaximum());
          cf_angle[ll]->SetParameter(0,cf_angle[ll]->GetParameter(0)/cf_angle[ll]->GetMaximum());
          if (loopcount>4) {
            oria=cf_length[ll]->GetParameter(5);
	    a=-12.5;
            b=2*a*cf_length[ll]->GetParameter(1);
            cf_length[ll]->SetParameter(5,a);
            cf_length[ll]->SetParameter(4,-b);
            cf_length[ll]->SetParameter(3,b*b/(4*a)); }

          if (verbose){
          cout << " No valid match found for ll = " << ll << "; quadrant " << k << ", adjusting matchfunc " << endl;
          cout << " RMS cf_length[ " << ll << "] = " << cf_length[ll]->GetParameter(2) <<endl; 
	  }
          if (loopcount>5) 
             { 
               cf_length[ll]->SetParameter(2, cf_length[ll]->GetParameter(2)/(TMath::Power(rmsfactor,6)));
               a=oria;
               b=2*a*cf_length[ll]->GetParameter(1);
               cf_length[ll]->SetParameter(5,a);
               cf_length[ll]->SetParameter(4,-b);
               cf_length[ll]->SetParameter(3,b*b/(4*a)); 
               cf_length[ll]->SetParameter(0,cf_length[ll]->GetParameter(0)/cf_length[ll]->GetMaximum());

               cf_angle[ll]->SetParameter(2, cf_angle[ll]->GetParameter(2)/(TMath::Power(rmsfactor,6)));
               cf_angle[ll]->SetParameter(0,cf_angle[ll]->GetParameter(0)/cf_angle[ll]->GetMaximum());

	       if (verbose){
               cout << " No Match found; did " << loopcount << " iterations. Giving up on this point. " ;
               cout << "Reseting RMS cf_length[" << ll << "] = " << cf_length[ll]->GetParameter(2) <<endl; 
	       }
               if ( (! setstone[ll-1] )&&(ll>0)&&(springattempt[ll-1]<5)){
		 if (verbose){
               cout << " Going back to ll = " << ll-1 << "." ;
               cout << endl;
               cout << " Suspicion that previous point was wrong :: " << xx[ll] << " and " << yy[ll];
               cout << ", after segupdate, original point was: " << xxx << " and " << yyy << endl; }
               maxbin=input[k]->FindBin(xx[ll],yy[ll]);
               input[k]->GetBinXYZ( maxbin, xbin2, ybin2, zbin2);
               for (ii=-2;ii<=2;ii++){
		 for ( jj=-2;jj<=2;jj++){
		   penalty[k]->SetBinContent(xbin2+ii,ybin2+jj,-1000);}}

               if (TMath::Sqrt(TMath::Power(xxx-xx[ll],2)+TMath::Power(yyy-yy[ll],2)) < 0.035 ) {
		 // i updated it .... AVDB 10-19-2012
		 if (verbose){ cout << " NEED TO UPDATE PENALTY !!! " << endl;}
               maxbin=input[k]->FindBin(xxx,yyy);
               input[k]->GetBinXYZ( maxbin, xbin2, ybin2, zbin2);
               for (ii=-2;ii<=2;ii++){
		 for ( jj=-2;jj<=2;jj++){
		   penalty[k]->SetBinContent(xbin2+ii,ybin2+jj,-1000);}}

	       }
           
	       if (verbose) {
		 cout << " with xbin = " << xbin2 << ", and ybin = " << ybin2 << endl;}
                ll--; 
                springattempt[ll]++;
                if (verbose){
		  cout << " spring " << ll << " attempt : " << springattempt[ll] << endl;}
                 ll--;
               }
               else { 
		 if (verbose){
		 cout << " We can't go back to " << ll-1 ;
                 cout << " have to think about something else. Setting FORCEPOINT" << endl;}
                 ll--;
                 // think about getting the max value anyway; we will take our first point
                 penalty[k]->Reset();        
                 forcepoint=1;
                     }
               if (verbose) cout << " stepback = " << stepback << endl;            
        
	      // it may be that the entire previous cluster was wrong, need to exclude the previous point somehow
 
               stepback++;
          
      	       if (stepback>2) { if (verbose) cout << "let's just break here " << endl; break;}
               break;}              
       }  
 
     }  // loop ll

     if (verbose) {cout << " graph " << k <<" :: " << endl;}
     Double_t xtmp,ytmp;
   // TGraph *points = new TGraph(16);
     if (verbose){
     for (ll=0;ll<16;ll++){  
       cout << ll << " " << xx[ll] << " " << yy[ll] ;
       peaks_remapped[k]->GetPoint(ll,xtmp,ytmp);   
       cout << " " << xtmp << " " <<ytmp<< endl;
       }
     //points->SetPoint(k,xx[k],yy[k]); }                            
     // input[0]->Draw("colz"); points->Draw("PL"); 


     cout << "Totcost quadrant " << k << " = " << totcost[k] <<endl;}
     if (totcost[k] > 550 )  {validflag |= ( 1 << k) ;
	  if (nosegupdate==0) updatesegmentation(input[k],xx,yy,peaks_remapped[k],verbose); }




     /* check overlap */
     for (ll=1;ll<16;ll++){  
       if (TMath::Sqrt(TMath::Power(xx[ll]-xx[ll-1],2)+TMath::Power(yy[ll]-yy[ll-1],2)) < 0.01 ) {
	 if (verbose) cout << " Point " << ll-1 << " and " << ll << " overlap !" <<endl;
         totcost[k]=-99.99;
       }}


     if (totcost[k] < cost) cost=totcost[k];
     } // matches validflag check
#endif
   // } // matches !validflag
#ifdef DEVELOP
     TFile *dd;
     if (k==0) dd = new TFile("quadrants.root","RECREATE");
     floodsq[k]->Write();
     if (k==3) dd->Close();
#endif

 } // loop over k 

 if (verbose) cout << " -------> VALIDFLAG = " << validflag <<endl;




  c1->Clear();c1->Divide(2,2)  ;
  for (k=0;k<4;k++){
    //    cout << "Totcost quadrant " << k << " = " << totcost[k] <<endl;
   c1->cd(k+1);floodsq[k]->Draw("colzhist");peaks_remapped[k]->Draw("PL");}
  

  sprintf(pngstring,"%s_quadrants.png",filebase);
  c1->Update();
  c1->Print(pngstring);

#ifdef PUBPLOT
  set2dcolor(4);
  sprintf(pngstring,"%s_quadrants.eps",filebase);

  c1->Print(pngstring);
#endif

 TGraph *peaks_sorted = new TGraph(64);
 peaks_sorted->SetName("peaks_sorted");
 mergegraphs(peaks_remapped,peaks_sorted);

 for (k=0;k<4;k++){ 
   delete floodsq[k];
   // these deletes make the program crash, because arrays are only created
   // if the SRT16 failed, stupid bug easy to fix
   //   delete input[k];
   // delete inp[k];
   //delete penalty[k];
   //delete costfunc[k];
   //delete updatehist[k];
   //delete deformfunc[k];

 }
 
 return peaks_sorted;}

Double_t findmean8( TH1D * h1, Int_t verbose,Int_t X, Bool_t APD){
  TSpectrum *s = new TSpectrum();
  Int_t nfound;
  Float_t *xx,*yy;
  Int_t ind[8];
  Double_t mean;
  Int_t i;


  nfound = s->Search(h1);


  if (verbose) cout << nfound << " peaks found in findmean8 " << endl;
  if (nfound==8){
    xx = s->GetPositionX();
    yy = s->GetPositionY();
    TMath::Sort(nfound,xx,ind); 
    if (verbose) {
       cout << " Central peaks at : " << xx[ind[3]] << " (height = " << yy[ind[3]] << ") and " ;
       cout << xx[ind[4]] << " (height = " << yy[ind[4]] << ")" << endl;}
    mean = (Double_t) (xx[ind[4]] + xx[ind[3]]) / 2.;
    if (verbose) cout << " mean at :: " << mean << endl;
    return mean; }

  // NOTE !!!
  // the y-coordinate is always slightly below 0 ! due to some remaining asymmetry
  // DO NOT FORGET THIS !!!
  // HOWEVER, FOR APD 1 it is slightly ABOVE 0, due to the mirroring

  else {
    if (verbose){    cout << "FINDMEAN8 FAILED !! " << endl;}
    // let's try to calculate the distances between any two peaks:
    if ((nfound>2)&&(nfound<12)) {
    xx = s->GetPositionX();
    yy = s->GetPositionY();
    Int_t ind2[12];
    TMath::Sort(nfound,xx,ind2);  

    Float_t meanloc[12],absmeanloc[12];
     if (verbose)  {cout << " nfound = " << nfound << endl;
       for ( i=0;i<nfound;i++ ) { cout << " Peak " << ind2[i] << " at  " << xx[ind2[i]] << endl; }}
    for (  i=0;i<(nfound-1);i++){
      meanloc[i]=xx[ind2[i+1]] - ( xx[ind2[i+1]]-xx[ind2[i]])/ 2.;
      if (verbose)  cout << " middle between " << i+1 << " and " << i << " : " << meanloc[i] << endl;
      absmeanloc[i] = TMath::Abs(meanloc[i]);
      }
    TMath::Sort(nfound-1,absmeanloc,ind2,kFALSE);
    i=0;
    if ( X==0){
      if (verbose) cout << "looking at Y" << endl;
      if (APD) {
       while (meanloc[ind2[i]]<0) { 
       if (verbose) { cout <<  " candidate :: " << meanloc[ind2[i]] << endl;}
       if ( i< ( nfound-2) ) i++; else return 12345;}
       if ( verbose ) cout << " returning : " << meanloc[ind2[i]] << endl;      
       return meanloc[ind2[i]];     
      }
      else {
    while (meanloc[ind2[i]]>0) { 
    if (verbose) { cout <<  " candidate :: " << meanloc[ind2[i]] << endl;}
    if ( i< ( nfound-2) ) i++; else return 12345;}
    if ( verbose ) cout << " returning : " << meanloc[ind2[i]] << endl;      
    return meanloc[ind2[i]];}
    }
    else { 
      if (verbose) cout << "looking at X" << endl;
      return meanloc[ind2[0]];}
    } //nfound > 2 && nfound < 12
     else {return 12345;}
    
}
}

Double_t findmean( TH1D * h1, Int_t verbose, Bool_t APD){
  Int_t initialbin,i,leftbinmax,rightbinmax,meanbin;
  Double_t leftmax, rightmax, mean, curval, prevval;
  //  h1->Rebin(2);
  leftmax=0; leftbinmax=0;
  rightmax=0; rightbinmax=0;
  h1->SetAxisRange(-.25,.25);
  meanbin = h1->FindBin(h1->GetMean());
  normalise_1(h1);
  

   initialbin = h1->FindBin(0);

  //we want to start from a low value ::
  if (verbose)  { cout << "looking left; initialbin ::" << initialbin << " at position " ;
    cout << h1->GetBinCenter(initialbin) << " value: " << h1->GetBinContent(initialbin) << endl;
    cout << "meanbin :: " << meanbin << " at position " << h1->GetBinCenter(meanbin) ;
    cout << " value : " << h1->GetBinContent(meanbin) << endl;}
  if ( h1->GetBinContent(meanbin) < h1->GetBinContent(initialbin)){initialbin=meanbin;}
   curval=0;
   //   initialbin=meanbin; 
   //start a little right from zero and go left
  for (i=initialbin; i>(initialbin-35);i--){
    prevval=curval; 
    if (verbose)    cout << " i = " << i << " Content: " << h1->GetBinContent(i) << endl;
    if (h1->GetBinContent(i) > leftmax) {
      leftbinmax = i;
      leftmax = h1->GetBinContent(i);}
      curval = h1->GetBinContent(i);
      // when the gradient becomes largely negative, stop the iteration except for the first 5 bins.
      if ((curval-prevval)<-.25){
	if (i<(initialbin-5)) i=initialbin-100;}
      //        else {leftmax=curval;leftbinmax=1;}}
  }
      if (verbose)    cout << "looking right; initialbin ::" << initialbin << endl;
   curval=0;
   for (i=initialbin; i<(initialbin+35);i++){ 
    prevval=curval; 
    if (verbose)     cout << " i = " << i << " Content: " << h1->GetBinContent(i) << endl;
    if (h1->GetBinContent(i) > rightmax) {
      rightbinmax = i;
      rightmax = h1->GetBinContent(i);}
      curval = h1->GetBinContent(i);
      if ((curval-prevval)<-.25){ 
	if (i>(initialbin+5)) i=initialbin+100;}
	//        else {rightmax=curval;rightbinmax=1;}}
  }
   if (verbose){
    cout << " leftmax = " << leftmax << " @ " << h1->GetBinCenter(leftbinmax);
    cout << " rightmax = " << rightmax << " @ " << h1->GetBinCenter(rightbinmax);
    cout <<endl;}
  mean=(h1->GetBinCenter(rightbinmax)+h1->GetBinCenter(leftbinmax))/2.;

  return mean;}

Int_t validate_sort(TGraph *gr, Int_t verbose){
Double_t angs[15],angs_rms[15];
Double_t lengs[15],lengs_rms[15];

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

//angs_rms[0] = 0.0557415; 
angs_rms[0] = 0.0957415; 
//angs_rms[1] = 0.0860722; 
 angs_rms[1] = 0.11;
angs_rms[2] = 0.1628848; 
angs_rms[3] = 0.062414; 
 angs_rms[4] = 0.095;
angs_rms[5] = 0.19;
angs_rms[6] = 0.096456; 
 angs_rms[7] = 0.095;
angs_rms[8] = 0.15; 
 angs_rms[9] = 0.10;
 //angs_rms[10] = 0.1752884; 
angs_rms[10] = 0.181; 
//angs_rms[11] = 0.059681; 
angs_rms[11] = 0.0759681; 
// angs_rms[12] = 0.126;
 angs_rms[12] = 0.145;
angs_rms[13] = 0.153272; 
angs_rms[14] = 0.1552269; 

lengs[0] = 0.146288; 
lengs[1] = 0.119; 
lengs[2] = 0.0781698; 
lengs[3] = 0.197265; 
lengs[4] = 0.122032; 
lengs[5] = 0.0638559; 
lengs[6] = 0.273927; 
lengs[7] = 0.135607; 
lengs[8] = 0.0934954; 
lengs[9] = 0.136071; 
lengs[10] = 0.0735089; 
lengs[11] = 0.260118; 
lengs[12] = 0.11599; 
lengs[13] = 0.0892242; 
lengs[14] = 0.22247; 

lengs_rms[0] = 0.0172834; 
lengs_rms[1] = 0.0150015; 
lengs_rms[2] = 0.0112432; 
lengs_rms[3] = 0.055347; 
lengs_rms[4] = 0.0244204; 
lengs_rms[5] = 0.0149949; 
lengs_rms[6] = 0.0524222; 
lengs_rms[7] = 0.0173137; 
lengs_rms[8] = 0.0178917; 
lengs_rms[9] = 0.0260743; 
lengs_rms[10] = 0.0169378; 
lengs_rms[11] = 0.0413933; 
lengs_rms[12] = 0.0204735; 
lengs_rms[13] = 0.023514; 
lengs_rms[14] = 0.0436819; 

Double_t angle[15],length[15];
Double_t x1,y1,x2,y2;
Int_t i,j,l,curcorner,point1,point2;
Int_t k=0;
curcorner=0;
for (i=0;i<3;i++){
  for (l=0;l<2;l++){
    for (j=0;j<(3-i);j++){
      if ((l==1)&&(j==0)) {point1=curcorner+j;}
      else { point1=curcorner+j+l*(3-i);}
      point2=curcorner+j+1+l*(3-i); 
       gr->GetPoint(point1,x1,y1);
       gr->GetPoint(point2,x2,y2);
  //       cout << " length segment betweeen point " << point1 << " and " << point2 << " = ";
  //       cout << TMath::Sqrt((x2-x1)**2+(y2-y1)**2) ;//<< endl ;    
  //       cout << " angle : ";
  //       cout << TMath::ATan2(y2-y1,x2-x1) << " (" <<  180*TMath::ATan2(y2-y1,x2-x1)/TMath::Pi() << ")" << endl ; 
       length[k]=TMath::Sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1)) ;
       angle[k]= TMath::ATan2(y2-y1,x2-x1);
       k++;
    }
   }
   point1=curcorner;
   point2=curcorner+(7-i*2); // + 5  + 3
   gr->GetPoint(point1,x1,y1);
   gr->GetPoint(point2,x2,y2);
   // cout << " length segment betweeen point " << point1 << " and " << point2 << " = ";
   //   cout << TMath::Sqrt((x2-x1)**2+(y2-y1)**2) ;    
   //   cout << " angle : ";
   //   cout << TMath::ATan2(y2-y1,x2-x1) << " (" << 180*TMath::ATan2(y2-y1,x2-x1)/TMath::Pi() << ")" << endl <<endl;
   length[k]=TMath::Sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1)) ;
   angle[k]= TMath::ATan2(y2-y1,x2-x1);
   k++;

   curcorner=point2; 
 } // loop over i

// now calculate the deviation

// option one: calculate an average deviation

//option two : calculate threshold for every value
 for (i=0;i<15;i++){
   if ( TMath::Abs(angle[i] -  angs[i] )  > 3.5*angs_rms[i] ){
   if (verbose) {
         cout << " Graph  failed at angle " << i ;
         cout << ": angle = " << angle[i] << " reference = " << angs[i];
         cout << "  3.5*RMS = " << 3.5*angs_rms[i] <<endl;}
       return -1;
   }
 }   
return 0;}
 

// EXPERIMENTAL !!
Int_t updatesegmentation(TH2F *hist, Double_t x[16], Double_t y[16], TGraph *updatedgraph,Int_t verbose) {
  // hist is our histogram, start by cloning it
  if (verbose) { cout << " Updating Histogram Segmentation "  << endl ;}
  Int_t updated[16];
  Float_t xxx[16],yyy[16];
  Int_t i,j;
#define THRESHOLDS 3
  Float_t thresholds[THRESHOLDS]={0.1,0.2,0.3};
  Int_t maxcluster[THRESHOLDS]={75,35,30};
  Int_t xbin,ybin,zbin;
  bin *clusr = new bin;
  for (i=0;i<16;i++) updated[i]=0;
  for (j=0;j<THRESHOLDS;j++){
    if (verbose){    cout << " Iteration " << j << endl;}
  hist_threshold(hist,thresholds[j]);
   for (i=0;i<16;i++) {
      hist->GetBinXYZ(hist->FindBin(x[i],y[i]),xbin,ybin,zbin);  
      ClusterSize(hist,xbin,ybin,clusr); 
      if ((clusr->pixels<maxcluster[j])&&(updated[i]==0)&&(clusr->pixels>5)) {  
       xxx[i]=hist->GetXaxis()->GetBinCenter(clusr->x); 
       yyy[i]=hist->GetYaxis()->GetBinCenter(clusr->y); 
       if (verbose)  {
	 cout << " candidate point:  (" << xxx[i] << "," << yyy[i] << ")"; 
         cout << " distance to original : (" << x[i] << "," << y[i] << ") :";
         cout <<  TMath::Sqrt(TMath::Power(xxx[i]-x[i],2)+TMath::Power(yyy[i]-y[i],2)) << endl;}
      if ( TMath::Sqrt(TMath::Power(xxx[i]-x[i],2)+TMath::Power(yyy[i]-y[i],2)) < 0.035 ) {
        updated[i]=1;
	if (verbose){
        cout << " updating point " << i << " to (" << xxx[i] << "," << yyy[i] << ")" ;
        cout << " cluster size :  " << clusr->pixels  << endl;}
	}
	}
   }}
  for (i=0;i<16;i++)  {
    if (updated[i]==0) {
      // if not updated, use originals
    if (verbose)      cout << " Point " << i <<  " at (" << x[i] << "," << y[i] << ") not updated." << endl;  
      xxx[i]=x[i];
      yyy[i]=y[i];
    }
    updatedgraph->SetPoint(i,xxx[i],yyy[i]);
  }    
       return 0;}



// EXPERIMENTAL !!
Int_t updatesegpoint(TH2F *hist, Double_t x, Double_t y, Double_t &xxx, Double_t &yyy,Int_t verbose) {
  // hist is our histogram, start by cloning it

  if (verbose) { cout << " ==== Welcome to Update Seg Point === " << endl; } 

  Int_t updated;
  //  Float_t xxx,yyy;
  Int_t j;
#define THRESHOLDS2 4
  Float_t thresholds[THRESHOLDS2]={0.1,0.2,0.3,0.35};
  Int_t maxcluster[THRESHOLDS2]={65,90,65,25};
  Int_t xbin,ybin,zbin;
  bin *clusr = new bin;
		  updated=0;
  for (j=0;j<THRESHOLDS2;j++){
    if (verbose)    cout << " Iteration " << j << " : ";
  hist_threshold(hist,thresholds[j]);
  //   for (i=0;i<16;i++) {
  hist->GetBinXYZ(hist->FindBin(x,y),xbin,ybin,zbin);  
  ClusterSize(hist,xbin,ybin,clusr); 
      if (verbose)      {  cout << " cluster size :  " << clusr->pixels ;
      cout << " center at (" << hist->GetXaxis()->GetBinCenter(clusr->x) << "," ;
      cout <<  hist->GetYaxis()->GetBinCenter(clusr->y) << ")."  << endl;}
      if ((clusr->pixels<maxcluster[j])&&(updated==0)&&(clusr->pixels>2)) {  
       xxx=hist->GetXaxis()->GetBinCenter(clusr->x); 
       yyy=hist->GetYaxis()->GetBinCenter(clusr->y); 
       if (verbose)  {
	 cout << " candidate point:  (" << xxx << "," << yyy << ")"; 
         cout << " distance to original : (" << x << "," << y << ") :";
         cout <<  TMath::Sqrt(TMath::Power(xxx-x,2)+TMath::Power(yyy-y,2)) << endl;}
       if ( TMath::Sqrt(TMath::Power(xxx-x,2)+TMath::Power(yyy-y,2)) < 0.035 ) {
	updated=1;
    if (verbose)        cout << " updating point  to (" << xxx << "," << yyy << ")" << endl;
       }}
	}
  //}
//  for (i=0;i<16;i++)  {
    if (updated==0) {
      // if not updated, use originals
    if (verbose)      cout << " Point at (" << x << "," << y << ") not updated." << endl;  
      xxx=x;
      yyy=y;
      //    }
      //    updatedgraph->SetPoint(i,xxx[i],yyy[i]);
  }    
       return 0;}

