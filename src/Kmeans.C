#include <iostream>
#include "Kmeans.h"
#include "Cluster.h"
#include "TROOT.h"
#include "TH2F.h"
#include "TMath.h"
#include "TCanvas.h"
#include "TMarker.h"
#include "TLine.h"

Int_t kmeans2d( TH2F *hist, Float_t *x, Float_t *y, Int_t verbose=0) {
  Cluster clus[4];
  Double_t d[4];
  Int_t kind[4]={0,1,2,3};
  //  for (Int_t k=0;k<4;k++){
  //    clus[k]; }

  clus[0].Init(.3,.3);
  clus[1].Init(.3,-.3);
  clus[2].Init(-.3,.3);
  clus[3].Init(-.3,-.3);

  if (verbose){
	for (Int_t k=0;k<4;k++){
	  std::cout << k <<   " :: " << clus[k].curmeanx << std::endl;
	}
  }

  //  rect.set_values (3,4);
  //  cout << "area: " << rect.area();

Double_t prevmatch;
  Double_t totmatch =0,xav=0,yav=0;
  
	for(Int_t l=0;l<10;l++){
	  if (verbose)	  std::cout << " \n Iteration " << l << std::endl;
	  /* for (Int_t i=0;i<hist->GetNbinsX();i++){
    for (Int_t j=0;j<hist->GetNbinsY();j++){
  */
	  /*
  for (Int_t i=64;i<192;i++){
  for (Int_t j=64;j<192;j++){
	  */
	  /*
  for (Int_t i=32;i<224;i++){
  for (Int_t j=32;j<224;j++){
	  */

	  Float_t maxval = hist->GetMaximum();

  for (Int_t i=0;i<256;i++){
  for (Int_t j=0;j<256;j++){

      // cout << i << "," << j << " :: " << hist->GetBinContent(i,j) << endl;
    if (hist->GetBinContent(i,j)>0 ){
	for (Int_t k=0;k<4;k++){
	  d[k] = clus[k].Distance(hist->GetXaxis()->GetBinCenter(i),hist->GetYaxis()->GetBinCenter(j));
	  //  cout << " d[" << k << "] :" << d[k] << endl;
	}
	TMath::Sort(4,d,kind,kFALSE);
	//	cout << kind[0] << endl;
	//	clus[kind[0]].Add(hist->GetXaxis()->GetBinCenter(i),hist->GetYaxis()->GetBinCenter(j),hist->GetBinContent(i,j));
	// add some weight to the high focal points (ie. the crystal locations), but not too much otherwise there will be bias towards the front edge ( because of exponential attenuation in the array ) 
        if (hist->GetBinContent(i,j)>(maxval*0.2)) {
	  clus[kind[0]].Add(hist->GetXaxis()->GetBinCenter(i),hist->GetYaxis()->GetBinCenter(j),5); }
        else {
	  clus[kind[0]].Add(hist->GetXaxis()->GetBinCenter(i),hist->GetYaxis()->GetBinCenter(j),1);}
        
      }
    }
  }

  totmatch =0;xav=0;yav=0;
	for (Int_t k=0;k<4;k++){
	  //	  cout << k <<  " :: " << clus[k].Print();
	  if (verbose){
	    std::cout << k <<   " :: " << clus[k].newmeanx;
	    std::cout << " " << clus[k].newmeany << " " << clus[k].newpoints;
	    std::cout << " " << clus[k].wcss << std::endl;}
	  totmatch += clus[k].wcss;
          if (k<2) xav += clus[k].newmeanx;
          else xav -= clus[k].newmeanx ;
          if (k%2) yav -= clus[k].newmeany;
	  else yav += clus[k].newmeany;
          clus[k].NewIteration();
	}

	*x=(clus[0].curmeanx+clus[1].curmeanx)/2.-xav/4.;
        *y=(clus[0].curmeany+clus[2].curmeany)/2.-yav/4.;


     
	if (verbose){
	  std::cout << " Totmatch :: " << totmatch ;
	  std::cout << " " << xav/2. << " " << yav/2. ;
	  std::cout << " " << *x << " " << *y  ; 
	  std::cout << " Delta : " << prevmatch-totmatch << std::endl;}
        if (( (prevmatch-totmatch) < 0.01)&&(l>2) ) {l=10000;}
        prevmatch=totmatch;
}


#ifdef DEVELOP

 TH1F *histclone = (TH1F *)hist->Clone();
 histclone->SetName("histclone");

 for (Int_t i=0;i<256;i++){
  for (Int_t j=0;j<256;j++){

      // cout << i << "," << j << " :: " << hist->GetBinContent(i,j) << endl;
    if (hist->GetBinContent(i,j)>0 ){
	for (Int_t k=0;k<4;k++){
	  d[k] = clus[k].Distance(hist->GetXaxis()->GetBinCenter(i),hist->GetYaxis()->GetBinCenter(j));
	  //  cout << " d[" << k << "] :" << d[k] << endl;
	}
	TMath::Sort(4,d,kind,kFALSE);
	//	cout << kind[0] << endl;
	//	clus[kind[0]].Add(hist->GetXaxis()->GetBinCenter(i),hist->GetYaxis()->GetBinCenter(j),hist->GetBinContent(i,j));
	histclone->SetBinContent(i,j,kind[0]+0.01);
      }
    }
 }

 
 TCanvas *c3 =  new TCanvas();
 TMarker *c[4];
 histclone->Draw("colz");
 TLine *lineX = new TLine();
 TLine *lineY = new TLine();
 lineX->SetY1(hist->GetYaxis()->GetBinLowEdge(1));
 lineX->SetY2(hist->GetYaxis()->GetBinLowEdge(1)+hist->GetYaxis()->GetBinWidth(1)*hist->GetNbinsX());
   lineX->SetX1(*x);
   lineX->SetX2(*x);
   lineY->SetX1(hist->GetYaxis()->GetBinLowEdge(1));
   lineY->SetX2(hist->GetYaxis()->GetBinLowEdge(1)+hist->GetYaxis()->GetBinWidth(1)*hist->GetNbinsX());
   lineY->SetY1(*y);
   lineY->SetY2(*y);

   lineX->Draw();
   lineY->Draw();

 for (Int_t k=0;k<4;k++){
   c[k] = new TMarker(clus[k].curmeanx,clus[k].curmeany,20+k);
   c[k]->SetMarkerColor(kRed);
   std::cout << " MARKER :: " << clus[k].newmeanx << " " << clus[k].newmeany << " " << c[k]->GetX() << " " << c[k]->GetY() << std::endl;
   c[k]->Draw();

 }
 c3->Print("kmeans.eps"); 


#endif


  return 0;
}
