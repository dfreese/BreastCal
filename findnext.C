#include <TROOT.h>
#include <TF1.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TMath.h>
#include <Riostream.h>

/* From the ROOT interface 
  genfuncs();
  costfunc=(TH2F *) input[0]->Clone();
  costfunc->SetName("costfunc");
  updatepoint(input[0],0,13,12);
  Int_t ll,xbin2,ybin2,zbin2,maxbin;
  k=0;
    for (ll=0;ll<15;ll++) {for (i=1;i<costfunc->GetNbinsX();i++) { for (j=1;j<costfunc->GetNbinsY();j++) { costfunc->SetBinContent(i,j,CalcCost(input[k],ll,i,j));}} maxbin = costfunc->GetMaximumBin();  costfunc->GetBinXYZ( maxbin, xbin2, ybin2, zbin2); updatepoint(input[k],ll+1,xbin2,ybin2);}
 TGraph *points = new TGraph(16);
  for (k=0;k<16;k++){ points->SetPoint(k,xx[k],yy[k]); }                     
 input[0]->Draw("colz"); points->Draw("PL"); 

 */


//TF1 *cf_length[15];
//TF1 *cf_angle[15];

//Double_t xx[16];
//Double_t yy[16];

//  s*s/3. - 2* TMath::Power (s / TMath::Pi() ,2)


Double_t rcos(Double_t *x, Double_t *par){
  Double_t mu=par[1];
  Double_t rms=par[2];
  //transfer rms to s -- approximatly but close enough
  Double_t s=TMath::Sqrt(rms*rms/0.13);
  Double_t val;
  val= (1+TMath::Cos( TMath::Pi()*(x[0]-mu)/s)); 
  val/=(2*s);
  val*=par[0];
  if (( x[0] > ( mu - s)  )&&(x[0]<(mu+s)))  return val;
  else  return 0. ;}

Double_t costfun(Double_t *x,Double_t *par){
  Double_t *p1 = &par[0];
  Double_t *p2 = &par[3];
  Double_t *p3 = &par[6];
  //  Double_t result = par[0]*(rcos(x,p1) + p2[0] + p2[1]*x[0] + p2[2]*x[0]*x[0] - TMath::Exp(p3[0]+p3[1]*x[0]));// -expo(p3) ;
  Double_t result = rcos(x,p1) + p2[0] + p2[1]*x[0] + p2[2]*x[0]*x[0] - TMath::Exp(p3[0]+p3[1]*x[0]);// -expo(p3) ;
  return result; }
 


Int_t genfuncs(TF1 *cf_length[],TF1 *cf_angle[]){
//  Int_t genfuncs(void){
Double_t angs[15],angs_rms[15];
Double_t lengs[15],lengs_rms[15];

lengs[0] = 0.105393; 
lengs[1] = 0.0906205; 
lengs[2] = 0.0618664; 
lengs[3] = 0.121688; 
lengs[4] = 0.0981613; 
lengs[5] = 0.0652975; 
lengs[6] = 0.181654; 
lengs[7] = 0.107233; 
lengs[8] = 0.0761921; 
lengs[9] = 0.113585; 
lengs[10] = 0.0783797; 
lengs[11] = 0.223795; 
lengs[12] = 0.10645; 
lengs[13] = 0.11022; 
lengs[14] = 0.243015; 
lengs_rms[0] = 0.0112099; 
lengs_rms[1] = 0.011053; 
lengs_rms[2] = 0.00810675; 
lengs_rms[3] = 0.0210004; 
lengs_rms[4] = 0.0118834; 
lengs_rms[5] = 0.00660159; 
lengs_rms[6] = 0.0229445; 
lengs_rms[7] = 0.0128896; 
lengs_rms[8] = 0.010938; 
lengs_rms[9] = 0.0131741; 
lengs_rms[10] = 0.00866178; 
lengs_rms[11] = 0.0199808; 
lengs_rms[12] = 0.0175271; 
lengs_rms[13] = 0.0149924; 
lengs_rms[14] = 0.0332595; 
angs[0] = 1.53579; 
angs[1] = 1.48772; 
angs[2] = 1.44674; 
angs[3] = 0.0356644; 
angs[4] = 0.0660163; 
angs[5] = 0.101398; 
angs[6] = 0.718202; 
angs[7] = 1.31625; 
angs[8] = 1.22019; 
angs[9] = 0.210212; 
angs[10] = 0.29043; 
angs[11] = 0.74681; 
angs[12] = 1.06694; 
angs[13] = 0.442986; 
angs[14] = 0.745766; 
angs_rms[0] = 0.0576733; 
angs_rms[1] = 0.0851815; 
angs_rms[2] = 0.117938; 
angs_rms[3] = 0.0503444; 
angs_rms[4] = 0.0571585; 
angs_rms[5] = 0.105067; 
angs_rms[6] = 0.0549161; 
angs_rms[7] = 0.0568055; 
angs_rms[8] = 0.0839113; 
angs_rms[9] = 0.0552911; 
angs_rms[10] = 0.0770093; 
angs_rms[11] = 0.0358353; 
angs_rms[12] = 0.0722649; 
angs_rms[13] = 0.058228; 
angs_rms[14] = 0.0387027; 

/*
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

angs_rms[0] = 0.0557415; 
angs_rms[1] = 0.0800722; 
angs_rms[2] = 0.1628848; 
angs_rms[3] = 0.0556414; 
 angs_rms[4] = 0.095;
angs_rms[5] = 0.25;
angs_rms[6] = 0.096456; 
 angs_rms[7] = 0.095;
angs_rms[8] = 0.15; 
 angs_rms[9] = 0.10;
angs_rms[10] = 0.1752884; 
angs_rms[11] = 0.059681; 
 angs_rms[12] = 0.126;
angs_rms[13] = 0.153272; 
angs_rms[14] = 0.1552269; 
*/

/*
lengs[0]=0.138055; lengs_rms[0]=0.00989064; 
lengs[1]=0.112081; lengs_rms[1]=0.0119271; 
lengs[2]=0.0710406; lengs_rms[2]=0.00978138; 
lengs[3]=0.159867; lengs_rms[3]=0.0362222; 
lengs[4]=0.104405; lengs_rms[4]=0.0110806; 
lengs[5]=0.0574878; lengs_rms[5]=0.00789643; 
lengs[6]=0.234013; lengs_rms[6]=0.0344262; 
lengs[7]=0.127312; lengs_rms[7]=0.0140677; 
lengs[8]=0.085341; lengs_rms[8]=0.0137469; 
lengs[9]=0.116224; lengs_rms[9]=0.0144078; 
lengs[10]=0.0656261; lengs_rms[10]=0.0122476; 
lengs[11]=0.23091; lengs_rms[11]=0.0340111; 
lengs[12]=0.111544; lengs_rms[12]=0.0227386; 
lengs[13]=0.0838081; lengs_rms[13]=0.0226727; 
lengs[14]=0.217941; lengs_rms[14]=0.0472327; 

angs[6]=0.719324; angs_rms[6]=0.0685421;
 angs[6]=0.719324; angs_rms[6]=0.09;
*/

/*
lengs[0] = 0.146288; 
lengs[1] = 0.119; 
lengs[2] = 0.0781698; 
lengs[3] = 0.147265; 
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
lengs_rms[3] = 0.051347; 
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
*/
 Int_t i;
 Char_t title[50];
 Double_t mean,a,b,c;

 for (i=0;i<15;i++){
    sprintf(title,"cf_angle[%d]",i);
    //    cf_angle[i] = new TF1(title,"gaus(0)+pol2(3)",-TMath::Pi()/2.,TMath::Pi());
    cf_angle[i] = new TF1(title,costfun,-TMath::Pi()/2.,TMath::Pi(),8);
    //    cf_angle[i]->SetParameters(2,angs[i],angs_rms[i]);
   mean=angs[i];
   //   a=-2; 
   a=-2; 
   b= 2*a*mean; 
   if ((i==6)||(i==11)||(i==14)) {
     a=-5;b=2*a*mean;
     cf_angle[i]->SetParameters(2.5,mean,2.5*angs_rms[i],b*b/(4*a),-b,a,-1e100,0);}
   else {  if ( i>11) cf_angle[i]->SetParameters(4,mean,4*angs_rms[i],b*b/(4*a),-b,a,-1e100,0);  
     else  cf_angle[i]->SetParameters(.7,mean,angs_rms[i],b*b/(4*a),-b,a,-1e100,0);}

   cf_angle[i]->SetParameter(0,cf_angle[i]->GetParameter(0)/cf_angle[i]->Eval(mean));


   sprintf(title,"cf_length[%d]",i);
   //   cf_length[i] = new TF1(title,"gaus(0)+pol2(3)-expo(6)",0,0.5);
   cf_length[i] = new TF1(title,costfun,0,0.5,8);
   mean=lengs[i];
   a=-25; 
   b= 2*a*mean; 
   if ((i==6)||(i==11)||(i==14))  // cf_length[i]->SetParameters(2,mean,lengs_rms[i],b*b/(4*a),-b,a,1.5,-10);
     { // a=-50;//
        a=-25; 
        b= 2*a*mean; 
	cf_length[i]->SetParameters(.3,mean,1.25*lengs_rms[i],b*b/(4*a),-b,a,2,-25);}
   /*  if ((i==6)||(i==11)||(i==14)) {
     // a will set the slope at which the function decreases for large values
     a=-50;
     // c will be the slope at which the function decreases for small values
     c=-250;
     cf_length[i]->SetParameters(2,mean,lengs_rms[i],b*b/(4*a),-b,a,4,c);}*/
   else { 
     if ( i > 11  ) { 
       cf_length[i]->SetParameters(.3,mean,1.3*lengs_rms[i],b*b/(4*a),-b,a,3,-75); }
     else  cf_length[i]->SetParameters(.3,mean,1.15*lengs_rms[i],b*b/(4*a),-b,a,3,-75);}
   //  cf_length[i]->SetParameters(2,mean,lengs_rms[i],b*b/(4*a),-b,a,1.5,-50);
   //   cf_length[i]->SetParameters(2,lengs[i],lengs_rms[i]);
   //   cf_length[i]->SetParameter(0,1/cf_length[i]->GetMaximum());
   cf_length[i]->SetParameter(0,cf_length[i]->GetParameter(0)/cf_length[i]->Eval(mean));
 }
 return 0;}

/*
Int_t changelengpar(Int_t i, Double_t rmsfactor, Double_t meanfactor){
  cf_length[i]->SetParameter(1,lengs[i]*meanfactor);
  cf_length[i]->SetParameter(2, cf_length[i]->GetParameter(2)*rmsfactor);
}
*/

Int_t setnextstartpoint(Int_t k, Double_t xx[], Double_t yy[], TF1 *cf_length[], TF1 *cf_angle[]){
   Int_t start;
   start=k;
   if ((k==3)||(k==6)) start=0;
   if ((k==9)||(k==11)) start=7;
   if ((k==13)||(k==14)) start=12;

   xx[k+1] = xx[start]+cf_length[k]->GetParameter(1)*TMath::Cos(cf_angle[k]->GetParameter(1)) ;
   yy[k+1] = yy[start]+cf_length[k]->GetParameter(1)*TMath::Sin(cf_angle[k]->GetParameter(1)) ;

   return 0;} 

//Int_t updatepoint(Int_t k, Int_t xbin, Int_t ybin,TH1F *hist){
Int_t updatepoint(TH2F *hist, Int_t k, Int_t xbin, Int_t ybin, Double_t xx[], Double_t yy[]){
  xx[k]=hist->GetXaxis()->GetBinCenter(xbin);
  yy[k]=hist->GetYaxis()->GetBinCenter(ybin);

  return 0;
}


Float_t CalcCost(TH2F *hist, Int_t k, Int_t xbin, Int_t ybin, Double_t xx[], Double_t yy[], TF1* cf_length[], TF1* cf_angle[] ){
  Float_t length;
  Float_t angle;
  Float_t xcurr;
  Float_t ycurr;
  Float_t anglematch;
  Float_t lengthmatch;
  Float_t value;

  xcurr = hist->GetXaxis()->GetBinCenter(xbin);  
  ycurr = hist->GetYaxis()->GetBinCenter(ybin);  
 
   Int_t start;
   start=k;
   if ((k==3)||(k==6)) start=0;
   if ((k==9)||(k==11)) start=7;
   if ((k==13)||(k==14)) start=12;

  length=TMath::Sqrt( TMath::Power(xcurr-xx[start],2)+TMath::Power(ycurr-yy[start],2));
  angle=TMath::ATan2(ycurr-yy[start],xcurr-xx[start]);
  lengthmatch=20*cf_length[k]->Eval(length);
  //  anglematch=3*cf_angle[k]->Eval(angle);
  //  anglematch=5*cf_angle[k]->Eval(angle);
  anglematch=20*cf_angle[k]->Eval(angle);
  //  lengthmatch=0;
  //  anglematch=0;
  if ( length > 1e-3)    value = (anglematch+lengthmatch);
  else {value =0 ;
    //    if (verbose) {  cout << xx[start] << " " << yy[start] << "; " <<xcurr << " " << ycurr << endl;}
   }
  // value *= (hist->GetBinContent(xbin,ybin));
  return value;
}
