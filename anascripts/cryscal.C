#define PEAKS 64

Double_t X[64],Y[64];
TH2S *map;
TH2S *clusters[64];
THStack *clustermap;
TGraph *gr;
TH2F *mapdiff;
 
Int_t init(void){

ifstream f; f.open("SI_DAQ_1311211127_L0.RENA2.unit1_apd0_peaks.txt");

 Int_t j;
for (Int_t i=0;i<64;i++) { f >> j ; f >> X[i] ; f >> Y[i]; }
gr = new TGraph(64);
for (Int_t i=0;i<64;i++) { gr->SetPoint(i, X[i], Y[i]); }
gr->Draw("APL");
map = new TH2S("map","",340,-0.85,0.85,340,-0.85,0.85);

 Double_t xx,yy;
 Int_t crystal;
 
for (i=0;i<map->GetNbinsX();i++) { 
  xx = map->GetXaxis()->GetBinCenter(i); 
  for (j=0;j<map->GetNbinsY();j++){  
    yy= map->GetYaxis()->GetBinCenter(j); 
    crystal = getcrystal(xx,yy,X,Y,0 );
    map->SetBinContent(i,j,crystal);
}}

 Double_t levels[64];
 for (i=0;i<64;i++) {levels[i] = i ; }

 map->SetContour((sizeof(levels)/sizeof(Double_t)), levels); 
 map->SetMinimum(-0.001);

 mapdiff= (TH2F *) map->Clone();
 mapdiff->SetName("mapdiff");

Int_t prevbin=0; Int_t curbin; 
  for (i=0;i<map->GetNbinsX();i++) { 
    prevbin=map->GetBinContent(i,0);
    mapdiff->SetBinContent(i,0,0);
    for (j=1;j<map->GetNbinsY();j++){   
      curbin = map->GetBinContent(i,j) ; 
      mapdiff->SetBinContent(i,j,curbin-prevbin); 
      prevbin=curbin;}}
  
   for (j=0;j<map->GetNbinsY();j++) { 
     prevbin=map->GetBinContent(0,j);
     for (i=0;i<map->GetNbinsX();i++){   
       curbin = map->GetBinContent(i,j) ; 
       mapdiff->SetBinContent(i,j,( mapdiff->GetBinContent(i,j) || (curbin-prevbin))); 
       prevbin=curbin;}}


   mapdiff->SetMarkerStyle(6);

   mapdiff->SetMaximum(1);
   mapdiff->SetMinimum(0);
   Double_t bilevels[2]={0,1};
   mapdiff->SetContour((sizeof(bilevels)/sizeof(Double_t)),bilevels);

   TFile *f = new TFile("../SI_DAQ_1311211127_L0.root");
   TH2F *flood = (TH2F *) f->Get("floods[2][1][0]");

   flood->Draw("colz");mapdiff->Draw("colzsame");gr->Draw("P");


   /*
root [33] mapdiff->Draw("colz")
root [34] mapdiff->Draw()
root [35] flood->Draw("colz")
root [36] mapdiff->Draw("same")

   */

 return;}
 


  /*
  for (i=0;i<map->GetNbinsX();i++) { 
    //    xx = map->GetXaxis()->GetBinCenter(i); 
    for (j=0;j<map->GetNbinsY();j++){  
      // yy= map->GetYaxis()->GetBinCenter(j); 
      crystalpixel=map->GetBinContent(i,j);
        }
  }
  return 0;}
*/

Int_t getcrystal(Double_t x, Double_t y, Double_t xpos[64], Double_t ypos[64], Int_t verbose){
  //  cout << " Getcrystal :: " << " xpos[21] = " << xpos[21] << " ypos[21] = " << ypos[21] << endl;
  //  cout << " x = " << x << " y = " << y << endl;
  
  Double_t dist,min;
  Int_t histnr;

  histnr=9999;
  min=100000;
  if ((TMath::Abs(x)>1)||(TMath::Abs(y)>1)) return -3;
   for (Int_t k=0;k<PEAKS;k++){
      //      dist=(*(*xpeaks+k)-xdata)*(*(*xpeaks+k)-xdata)+(*(*ypeaks+k)-ydata)*(*(*ypeaks+k)-ydata);

      dist=TMath::Power((Float_t) ypos[k]-y,2)+TMath::Power((Float_t) xpos[k]-x,2);

     //if (debvar) cout << "k = " << k << "dist = " << dist << endl;

     if (dist<min) {histnr=k;min=dist;

       //   if (debvar) cout << "--------> k = " << k <<" min = " << min <<endl;
       }
     } //for loop 
   //      if (verbose) cout << "FINAL :: histnr = " << histnr <<" min = " << min << endl;
     if (histnr!=9999) { 
       return histnr;
        }
     
  
     else { if (verbose) 
         {cout << "No associated histogram found !" << endl;
           cout << " Entry :  x = " << x << " y = " << y <<endl;}
     }
    min=10000;
    histnr=9999;
    
  return -2;}

