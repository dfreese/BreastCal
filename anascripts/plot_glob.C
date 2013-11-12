 TH2F *ereshisto_0 = new TH2F("ereshisto_0","global eres", 16,0,16,8,0,8);
 TH2F *ereshisto_1 = new TH2F("ereshisto_1","global eres", 16,0,16,8,0,8);
 TH2F *pphisto_0 = new TH2F("pphisto_0","pp global ", 16,0,16,8,0,8);
 TH2F *pphisto_1 = new TH2F("pphisto_1","pp global ", 16,0,16,8,0,8);


Int_t plot_glob(string s, Int_t DAQ){
  ifstream infile;
 
  cout << " Opening file " << s << endl;

 infile.open(s.c_str());
 Char_t dummy[10];
 Double_t eres, d_eres;
 Double_t pp;
 Int_t j=0;
 Int_t finnr;
 Int_t modnr;
 Int_t chipnr;
 Int_t apdnr;
 Int_t unitnr;
 /*
 TH2F *ereshisto_0 = new TH2F(ereshisto,"global eres", 16,0,16,8,0,8);
 TH2F *ereshisto_1 = new TH2F(ereshisto,"global eres", 16,0,16,8,0,8);
 */

 while ( infile >> dummy >> eres >> d_eres >> pp ){
   j++;
   sscanf(dummy,"R%dM%dA%d",&chipnr,&unitnr,&apdnr);

   finnr=2*(TMath::Floor(chipnr/2));
   modnr=unitnr+4*(chipnr%2);
 
   if (DAQ%2) finnr++;
   if (DAQ/2) modnr+=8;
 
   finnr=7-finnr;

   if ( apdnr==1 ){ 
    ereshisto_1->SetBinContent( modnr+1, finnr+1,eres);
    pphisto_1->SetBinContent( modnr+1, finnr+1,pp);
   }
   else{
    ereshisto_0->SetBinContent(modnr+1, finnr+1,  eres ) ;
    pphisto_0->SetBinContent(modnr+1, finnr+1,  pp ) ;
 }
 }
 ereshisto_0->SetMaximum(0.25); ereshisto_0->SetMinimum(0.05);
 ereshisto_1->SetMaximum(0.25); ereshisto_1->SetMinimum(0.05);

 pphisto_0->SetMaximum(2400); pphisto_0->SetMinimum(800);
 pphisto_1->SetMaximum(2400); pphisto_1->SetMinimum(800);

 Double_t levels[16];
   for (Int_t i=0;i<16;i++) { levels[i]=800+i*100;}
 pphisto_0->SetContour(16,levels);
 pphisto_1->SetContour(16,levels);

 Double_t levels2[20];
   for (Int_t i=0;i<20;i++) { levels2[i]=0.05+i*0.01;}
 ereshisto_0->SetContour(20,levels2);
 ereshisto_1->SetContour(20,levels2);


 cout << " We read " << j << " lines " << endl;

 infile.close();
}
