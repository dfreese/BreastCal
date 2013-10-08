#define MAXFILES 50
#define MAXFILELENGTH 180
#include "TROOT.h"
#include "TGraph.h"
#include "Riostream.h"
#include "TCanvas.h"
#include "TMath.h"
#include "/home/miil/root/macros/myrootlib.h"
#include "/home/miil/root/libInit_avdb.h"

Int_t segmentlength(TGraph *gr, Double_t *length, Double_t *angle );

TGraph *gr[MAXFILES];
TGraph *quadrants[4][MAXFILES];
TH1F *fr;

Int_t main(Int_t argc, Char_t** argv){
  Int_t i=0;
  Char_t filelist[80];
  for (i=0;i<argc;i++){
    if (strncmp(argv[i],"-f",2)==0){
      sprintf(filelist,"%s",argv[i+1]);
      cout << " Using filelist :: " << filelist << endl;
	      i++;}
  }

  rootlogon(0);

  ifstream f;
  Char_t filename[MAXFILELENGTH];
  ifstream peakfile;
  Double_t xlist[8],ylist[8],distlist[8];
  Double_t meanx,meany,dist;
  Double_t x,y;
  Int_t k,j,l,nrfiles;
  i=0;

  f.open(filelist);
  while ( f >> filename ){
    cout << " Processing file " << filename <<endl;
    gr[i] = new TGraph(64);
    cout << " opening file :: " << endl;
    peakfile.open(filename);
    while (      peakfile >> j ) {
      peakfile >> x; peakfile >> y;
      //      cout << " j = " << j << endl;
      gr[i]->SetPoint(j,x,y); 
    }
    cout << " Closing file " << endl;
    peakfile.close();

    i++;

    if (i>=MAXFILES) {
      cout << " Too many files in the filelist ! " << endl;
      cout << " Current maximum supported files = " << MAXFILES << endl;
      cout << " Update MAXFILES in source code " << endl;
      cout << " Exiting. " << endl;
      return -2;}
 
  }

  f.close();
  nrfiles = i ;

  cout << " processed " << nrfiles << " files. " << endl;

  Double_t *xps;
  Double_t *yps;
  for (i=0;i<nrfiles;i++){

    // YES X AND Y ARE INTENTIONALLY SWAPPED !! 
    yps = gr[i]->GetY();
    xps = gr[i]->GetX();
    cout << " Creating quadrants " << endl;
    for (j=0;j<4;j++ ) quadrants[j][i]= new TGraph(16);  
    cout << " Assigning histograms " << endl;

    quadrants[0][i]->SetPoint(0,-xps[27],-yps[27]);
    quadrants[0][i]->SetPoint(1,-xps[26],-yps[26]);
    quadrants[0][i]->SetPoint(2,-xps[25],-yps[25]);
    quadrants[0][i]->SetPoint(3,-xps[24],-yps[24]);
    quadrants[0][i]->SetPoint(4,-xps[19],-yps[19]);
    quadrants[0][i]->SetPoint(5,-xps[11],-yps[11]);
    quadrants[0][i]->SetPoint(6,-xps[3],-yps[3]);
    quadrants[0][i]->SetPoint(7,-xps[18],-yps[18]);
    quadrants[0][i]->SetPoint(8,-xps[17],-yps[17]);
    quadrants[0][i]->SetPoint(9,-xps[16],-yps[16]);
    quadrants[0][i]->SetPoint(10,-xps[10],-yps[10]);
    quadrants[0][i]->SetPoint(11,-xps[2],-yps[2]);
    quadrants[0][i]->SetPoint(12,-xps[9],-yps[9]);
    quadrants[0][i]->SetPoint(13,-xps[8],-yps[8]);
    quadrants[0][i]->SetPoint(14,-xps[1],-yps[1]);
    quadrants[0][i]->SetPoint(15,-xps[0],-yps[0]);

    quadrants[1][i]->SetPoint(0,-xps[28],yps[28]);
    quadrants[1][i]->SetPoint(1,-xps[29],yps[29]);
    quadrants[1][i]->SetPoint(2,-xps[30],yps[30]);
    quadrants[1][i]->SetPoint(3,-xps[31],yps[31]);
    quadrants[1][i]->SetPoint(4,-xps[20],yps[20]);
    quadrants[1][i]->SetPoint(5,-xps[12],yps[12]);
    quadrants[1][i]->SetPoint(6,-xps[4],yps[4]);
    quadrants[1][i]->SetPoint(7,-xps[21],yps[21]);
    quadrants[1][i]->SetPoint(8,-xps[22],yps[22]);
    quadrants[1][i]->SetPoint(9,-xps[23],yps[23]);
    quadrants[1][i]->SetPoint(10,-xps[13],yps[13]);
    quadrants[1][i]->SetPoint(11,-xps[5],yps[5]);
    quadrants[1][i]->SetPoint(12,-xps[14],yps[14]);
    quadrants[1][i]->SetPoint(13,-xps[15],yps[15]);
    quadrants[1][i]->SetPoint(14,-xps[6],yps[6]);
    quadrants[1][i]->SetPoint(15,-xps[7],yps[7]);

    quadrants[2][i]->SetPoint(0,xps[35],-yps[35]);
    quadrants[2][i]->SetPoint(1,xps[34],-yps[34]);
    quadrants[2][i]->SetPoint(2,xps[33],-yps[33]);
    quadrants[2][i]->SetPoint(3,xps[32],-yps[32]);
    quadrants[2][i]->SetPoint(4,xps[43],-yps[43]);
    quadrants[2][i]->SetPoint(5,xps[51],-yps[51]);
    quadrants[2][i]->SetPoint(6,xps[59],-yps[59]);
    quadrants[2][i]->SetPoint(7,xps[42],-yps[42]);
    quadrants[2][i]->SetPoint(8,xps[41],-yps[41]);
    quadrants[2][i]->SetPoint(9,xps[40],-yps[40]);
    quadrants[2][i]->SetPoint(10,xps[50],-yps[50]);
    quadrants[2][i]->SetPoint(11,xps[58],-yps[58]);
    quadrants[2][i]->SetPoint(12,xps[49],-yps[49]);
    quadrants[2][i]->SetPoint(13,xps[48],-yps[48]);
    quadrants[2][i]->SetPoint(14,xps[57],-yps[57]);
    quadrants[2][i]->SetPoint(15,xps[56],-yps[56]);

    quadrants[3][i]->SetPoint(0,xps[36],yps[36]);
    quadrants[3][i]->SetPoint(1,xps[37],yps[37]);
    quadrants[3][i]->SetPoint(2,xps[38],yps[38]);
    quadrants[3][i]->SetPoint(3,xps[39],yps[39]);
    quadrants[3][i]->SetPoint(4,xps[44],yps[44]);
    quadrants[3][i]->SetPoint(5,xps[52],yps[52]);
    quadrants[3][i]->SetPoint(6,xps[60],yps[60]);
    quadrants[3][i]->SetPoint(7,xps[45],yps[45]);
    quadrants[3][i]->SetPoint(8,xps[46],yps[46]);
    quadrants[3][i]->SetPoint(9,xps[47],yps[47]);
    quadrants[3][i]->SetPoint(10,xps[53],yps[53]);
    quadrants[3][i]->SetPoint(11,xps[61],yps[61]);
    quadrants[3][i]->SetPoint(12,xps[54],yps[54]);
    quadrants[3][i]->SetPoint(13,xps[55],yps[55]);
    quadrants[3][i]->SetPoint(14,xps[62],yps[62]);
    quadrants[3][i]->SetPoint(15,xps[63],yps[63]);
  }
    TCanvas *c1 = new TCanvas();

    c1->Divide(2,2);
    for (j=0;j<4;j++){
      c1->cd(j+1);
      quadrants[j][0]->Draw("APL");
    }

    c1->Print("test.png");

Double_t lens[MAXFILES][4][15],angs[MAXFILES][4][15];

TH1F *all_segs[15],*all_angs[15];

for (i=0;i<nrfiles;i++){
  for(k=0;k<4;k++){segmentlength(quadrants[k][i],lens[i][k],angs[i][k]); }}

Char_t tmpstr[40] ; 
 for (i=0;i<15;i++) { 
     sprintf(tmpstr, "all_segs[%d]",i); 
     all_segs[i] = new TH1F(tmpstr,tmpstr, 50, 0,0.5); 
     sprintf(tmpstr,"all_angs[%d]",i); 
     all_angs[i]= new TH1F(tmpstr,tmpstr, 40, -0.2, 3.1415 );}

for (i=0;i<nrfiles;i++){
  for(k=0;k<4;k++){
     for(l=0;l<15;l++) {
       all_segs[l]->Fill(lens[i][k][l]);
       all_angs[l]->Fill(angs[i][k][l]);    }}}

c1->Clear();c1->Divide(4,4);
for (i=0;i<15;i++) { c1->cd(i+1); all_segs[i]->Draw();}
c1->Print("lengths.png");
c1->Clear();c1->Divide(4,4);
for (i=0;i<15;i++) { c1->cd(i+1); all_angs[i]->Draw();}
c1->Print("angs.png");

for (i=0;i<15;i++) { cout << "lengs["<<i<<"] = " << all_segs[i]->GetMean() << "; " << endl; }
for (i=0;i<15;i++) { cout << "lengs_rms["<<i<<"] = " << all_segs[i]->GetRMS() << "; " << endl; }
for (i=0;i<15;i++) { cout << "angs["<<i<<"] = " << all_angs[i]->GetMean() << "; " << endl; }
for (i=0;i<15;i++) { cout << "angs_rms["<<i<<"] = " << all_angs[i]->GetRMS() << "; " << endl; }


 /*  nrfiles=8;
  for (i=0;i<64;i++){
    for (j=0;j<nrfiles;j++){
      gr[j]->GetPoint(i,xlist[j],ylist[j]);
        }
     meanx= TMath::Mean(nrfiles,xlist); 
     meany= TMath::Mean(nrfiles,ylist);
     for (j=0;j<nrfiles;j++){
       distlist[j]=TMath::Sqrt(TMath::Power(meanx-xlist[j],2)+TMath::Power(meany-ylist[j],2));}
       dist = TMath::Mean(nrfiles,distlist);
       cout << "Circle " << i << ": " << meanx << " " << meany << " " <<dist << endl;
       //   circles[i] = new TArc(meanx,meany,dist,0,360);
       */

  

    /*
  fr = (TH1F *) gROOT->FindObject("fr");
  if ( !fr)  fr = new TH1F("fr","",100,-.8,.8);   
 fr->SetMinimum(-.8);fr->SetMaximum(.8);fr->SetLineColor(kWhite);
 fr->Draw();
 for (i=0;i<64;i++){ circles[i]->Draw();}
for (i=0;i<nrfiles;i++){ gr[i]->Draw("P");}
    */
  return 0;}



Int_t segmentlength(TGraph *gr, Double_t *length, Double_t *angle ){
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
      //       gr->GetPoint(curcorner+j+1+l*(3-i),x2,y2);
       gr->GetPoint(point1,x1,y1);
       gr->GetPoint(point2,x2,y2);

  //       cout << " length segment betweeen point " << point1 << " and " << point2 << " = ";
  //       cout << TMath::Sqrt((x2-x1)**2+(y2-y1)**2) ;//<< endl ;    
  //       cout << " angle : ";
  //       cout << TMath::ATan2(y2-y1,x2-x1) << " (" <<  180*TMath::ATan2(y2-y1,x2-x1)/TMath::Pi() << ")" << endl ; 
       length[k]=TMath::Sqrt(TMath::Power((x2-x1),2)+TMath::Power((y2-y1),2)) ;
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
   length[k]=TMath::Sqrt(TMath::Power((x2-x1),2)+TMath::Power((y2-y1),2)) ;
   angle[k]= TMath::ATan2(y2-y1,x2-x1);
   k++;
   curcorner=point2; 
  }
  return 0;}
