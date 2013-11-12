TFile *f = new TFile("PT_DAQ_1310091033_all.merged.root")
TTree *m = (TTree *)f->Get("merged")
TH1F *ehist = new TH1F("ehist","Ehist",200,0,800)
m->Draw("E1>>ehist","crystal1<65&&crystal1>-1")
TF1 *fitfunc2 = new TF1("fitfunc2", DoubleGaussConst, 0, 20000, 6);
TF1 *fitfunc = new TF1("fitfunc", DoubleGauss, 0, 20000, 5);
fitfunc2->SetParameters(400e3,511,40,200e3,480,10000)
fitfunc->SetParameters(400e3,511,40,200e3,480)
fitfunc->SetLineColor(kBlue)
ehist->Fit("fitfunc","","",440,580)
ehist->Fit("fitfunc","","",440,580)
ehist->Fit("fitfunc","","",480,580)
ehist->Fit("fitfunc","","",440,580)
ehist->Fit("fitfunc","","",440,6000)
ehist->Fit("fitfunc","","",440,600)
ehist->Fit("fitfunc","","",440,520)
ehist->Fit("fitfunc","","",440,540)
ehist->Fit("fitfunc","","",440,550)
ehist->Fit("fitfunc","","",440,560)
ehist->Fit("fitfunc","","",440,570)
ehist->Fit("fitfunc","","",450,570)
26.75*2.35/530
ehist->Fit("fitfunc2","","",450,570)
fitfunc2->SetLineColor(kRed)
ehist->Fit("fitfunc2","","",450,570)
25*2.35/528

TF1 *g1 = new TF1("g1","gaus",0,800);
TF1 *g2 = new TF1("g2","gaus",0,800);
TF1 *expfit = new TF1("expfit","TMath::Exp([0]+[1]*x*x)",0,800);
TF1 *constfit = new TF1("constfit","pol0",0,800);




constfit->SetLineColor(kBlue)


g2->SetParameters(fitparameters[0],fitparameters[1],fitparameters[2]);
g1->SetParameters(fitparameters[3],fitparameters[4],fitparameters[2]);
constfit->SetParameter(0, fitparameters[5]);
expfit->SetParameters(fitparameters[6],fitparameters[7]);

constfit->SetLineColor(kBlue);
g1->SetLineColor(kBlue);
g2->SetLineColor(kBlue);
g2->SetLineStyle(kDashed);
expfit->SetLineColor(kBlue);


g1->Draw("same");g2->Draw("same");constfit->Draw("same");expfit->Draw("same");




ehist->Draw();expfit->Draw("same");g1->Draw("same");g2->Draw("same");constfit->Draw("same");fitfunc2->Draw("same")
gROOT->ProcessLine(".L specfit.C");
TF1 *DGEfit = new TF1("DGEfit",DoubleGaussConstExpo,0,800,8)
DGEfit->SetParameters(4.2e5,528,25,2.18e5,468,42e3,15,-9e-3)
ehist->Fit("DGEfit","","",450,570)
DGEfit->SetLineColor(kRed)
ehist->Fit("DGEfit","","",450,570)
ehist->Fit("DGEfit","","",400,570)
ehist->Fit("DGEfit","","",380,570)
ehist->Fit("DGEfit","","",320,570)
ehist->Fit("DGEfit","","",340,570)
ehist->Fit("DGEfit","","",340,570)
ehist->Fit("DGEfit","","",330,570)
ehist->Fit("DGEfit","","",350,570)
ehist->Fit("DGEfit","","",360,570)
24.1*2.35/529
expfit->SetParameters(13.8,-3.5e-3)
g1->SetParameters(3.9647e5,529.3,24.1)
g2->SetParameters(4.73062,473,24.1)
constfit->SetParameter(0,-782076e4)
expfit->Draw("same");g1->Draw("same");g2->Draw("same");constfit->Draw("same");
ehist->Fit("DGEfit","","",300,570)
ehist->Fit("DGEfit","","",310,570)
ehist->Fit("DGEfit","","",315,570)
ehist->Fit("DGEfit","","",315,590)
expfit->SetParameters(16.1,-1.09e-2)
expfit->Draw("same");g1->Draw("same");g2->Draw("same");constfit->Draw("same");
24.8*2.35/528
Double_t *fipar ;
DGEfit->GetParameters(&fipar)
DGEfit->GetParameters(fipar)
.q!
 Double_t *fiterror = DGEfit->GetParErrors()
root [79] cout < #pm  << errorprop_divide(fitparameters[2]*2.35,2.35*fiterror[2],fitparameters[1],fiterror[1])
Error: << Illegal operator for real number (tmpfile):1:
*** Interpreter error recovered ***

root [80] cout << #pm  << errorprop_divide(fitparameters[2]*2.35,2.35*fiterror[2],fitparameters[1],fiterror[1])
#pm 0.000235744(class ostream)140706362293376
root [81] cout << ERES =  << fitparameters[2]*2.35/fitparameters[1] << endl;
ERES = 0.106515

