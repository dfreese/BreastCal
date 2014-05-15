#include <iostream>
#include "Kmeans.h"
#include "Cluster.h"
#include "TROOT.h"
#include "TH2F.h"
#include "TMath.h"

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
  for (Int_t i=64;i<192;i++){
  for (Int_t j=64;j<192;j++){
      // cout << i << "," << j << " :: " << hist->GetBinContent(i,j) << endl;
    if (hist->GetBinContent(i,j)>0 ){
	for (Int_t k=0;k<4;k++){
	  d[k] = clus[k].Distance(hist->GetXaxis()->GetBinCenter(i),hist->GetYaxis()->GetBinCenter(j));
	  //  cout << " d[" << k << "] :" << d[k] << endl;
	}
	TMath::Sort(4,d,kind,kFALSE);
	//	cout << kind[0] << endl;
	clus[kind[0]].Add(hist->GetXaxis()->GetBinCenter(i),hist->GetYaxis()->GetBinCenter(j),hist->GetBinContent(i,j));
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
          xav += clus[k].newmeanx;
          yav += clus[k].newmeany;
          clus[k].NewIteration();
	}
	if (verbose){
	  std::cout << " Totmatch :: " << totmatch ;
	  std::cout << " " << xav/2. << " " <<yav/2. ;
	  std::cout << " Delta : " << prevmatch-totmatch << std::endl;}
        if (( (prevmatch-totmatch) < 0.01)&&(l>2) ) {l=10000;}
        prevmatch=totmatch;
}

	*x=xav/2.;
        *y=yav/2.;

  return 0;
}
