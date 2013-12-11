


Int_t plot_cartrigdge(Char_t filebase[50]="PT_DAQ_1309181722"){

gROOT->ProcessLine(".L /home/miil/MODULE_ANA/ANA_V5/ModuleClass/anascripts/plot_glob.C");

  Char_t filename[100];


  for (Int_t i=0; i<4;i++){
  sprintf(filename,"./L%d/%s_L%d_globfits_spat.txt",i,filebase,i);
  plot_glob(filename,i);
  }

/*
plot_glob("./L0/PT_DAQ_1309181722_L0_globfits_spat.txt",0);
plot_glob("./L1/PT_DAQ_1309181722_L1_globfits_spat.txt",1);
plot_glob("./L2/PT_DAQ_1309181722_L2_globfits_spat.txt",2);
plot_glob("./L3/PT_DAQ_1309181722_L3_globfits_spat.txt",3);
*/


 TH2F *Left_0 = ereshisto_0->Clone();
 Left_0->SetName("Left_0");
 Left_0->SetTitle("Global Energy Resolution LEFT APD0");
 TH2F *Left_1 = ereshisto_1->Clone();
 Left_1->SetName("Left_1");
 Left_1->SetTitle("Global Energy Resolution LEFT APD1");

 ereshisto_0->Reset();
 ereshisto_1->Reset();

 TH2F *Leftpp_0 = pphisto_0->Clone();
 Leftpp_0->SetName("Leftpp_0");
 Leftpp_0->SetTitle("Global PP LEFT APD0");
 TH2F *Leftpp_1 = pphisto_1->Clone();
 Leftpp_1->SetName("Leftpp_1");
 Leftpp_1->SetTitle("Global PP LEFT APD1");

 pphisto_0->Reset();
 pphisto_1->Reset();



 TCanvas *c1 = new TCanvas("c1","Left",400,800);
c1->Divide(1,4);
c1->cd(1);
Left_0->Draw("colz");
c1->cd(2);
Left_1->Draw("colz");

c1->cd(3);
Leftpp_0->Draw("colz");
c1->cd(4);
Leftpp_1->Draw("colz");

 c1->Print("plot_cartridge_glob_L.png");
 c1->Print("plot_cartridge_glob_L.C");

 DumpContent(Leftpp_0);
 DumpContent(Leftpp_1);

for (Int_t i=0; i<4;i++){
 sprintf(filename,"./R%d/%s_R%d_globfits_spat.txt",i,filebase,i);
 plot_glob(filename,i);
 }

/* 
plot_glob("./R0/PT_DAQ_1309181722_R0_globfits_spat.txt",0);
plot_glob("./R1/PT_DAQ_1309181722_R1_globfits_spat.txt",1);
plot_glob("./R2/PT_DAQ_1309181722_R2_globfits_spat.txt",2);
plot_glob("./R3/PT_DAQ_1309181722_R3_globfits_spat.txt",3);
*/

 TH2F *Right_0 = ereshisto_0->Clone();
 Right_0->SetName("Right_0");
 Right_0->SetTitle("Global Energy Resolution RIGHT APD0");
 TH2F *Right_1 = ereshisto_1->Clone();
 Right_1->SetName("Right_1");
 Right_1->SetTitle("Global Energy Resolution RIGHT APD1");

 TH2F *Rightpp_0 = pphisto_0->Clone();
 Rightpp_0->SetName("Rightpp_0");
 Rightpp_0->SetTitle("Global PP RIGHT APD0");
 TH2F *Rightpp_1 = pphisto_1->Clone();
 Rightpp_1->SetName("Rightpp_1");
 Rightpp_1->SetTitle("Global PP RIGHT APD1");


 TCanvas *c2 = new TCanvas("c2","Right",400,800);
c2->Divide(1,4);
c2->cd(1);
Right_0->Draw("colz");
c2->cd(2);
Right_1->Draw("colz");


c2->cd(3);
Rightpp_0->Draw("colz");
c2->cd(4);
Rightpp_1->Draw("colz");
c2->Print("plot_cartridge_glob_R.png");
c2->Print("plot_cartridge_glob_R.C");

 DumpContent(Rightpp_0);
 DumpContent(Rightpp_1);



}


Int_t DumpContent(TH2F *hist){
  Int_t i,j;
  ofstream outputfile;
  Char_t outputfilename[60];
  sprintf(outputfilename,"%s.txt",hist->GetName() );
  outputfile.open(outputfilename);
  cout << " Outputfile :: " << outputfilename << endl;
  cout <<  " Bin Content " << hist->GetName() << endl;
  for (i=0;i<hist->GetNbinsY();i++) { 
    for (j=0;j<hist->GetNbinsX();j++) { 
      outputfile << i << " " << j << " " << hist->GetBinContent(j+1,i+1) << endl; } }

  outputfile.close();
 }
