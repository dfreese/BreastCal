#include <iostream>
#include <sstream>
#include <string>
#include <deque>
#include <vector>
#include <fstream>
#include <miil/SystemConfiguration.h>
#include <miil/process/processing.h>
#include <miil/process/ProcessParams.h>
#include <miil/process/ProcessInfo.h>
#include <miil/util.h>
#include <TH2F.h>
#include <TGraph.h>
#include <TSpectrum.h>
#include <TSpectrum2.h>
#include <TFile.h>
#include <TMath.h>
#include <TF1.h>

using namespace std;

// this parameter is used to check if the segmentation is valid ::
#define COSTTHRESHOLD 300

// comment this if you don't want the additional Pictorial Structure based search.
#define sortps

//comment this if you don't want the traditional 16 peak search.
#define SRT16

#define SPEAKS 16

template <typename hist_type>
void normalise_1(hist_type *hist) {
    if (hist->GetMaximum()) {
        hist->Scale(1/hist->GetMaximum());
    }
    return;
}

void hist_threshold(TH2F *hist, Float_t threshold) {
    for (int i = 0; i <= hist->GetNbinsX(); i++) {
        for (int j = 0; j <= hist->GetNbinsY(); j++) {
            if (hist->GetBinContent(i,j) < threshold) {
                hist->SetBinContent(i,j,0);
            }
        }
    }
    return;
}

struct bin {
    Int_t x;
    Int_t y;
    Float_t val;
    Int_t pixels;
};

class Cluster {
public:
    Cluster() {}
    float newmeanx;
    float newmeany;
    int newpoints;
    double wcss;
    float curmeanx;
    float curmeany;
    int curpoints;
    void Add(float x, float y, int binval){
        for (int i=0; i<binval; i++) {
            newmeanx = ( x + newmeanx*newpoints) / (newpoints+1);
            newmeany = ( y + newmeany*newpoints) / (newpoints+1);
            wcss +=  TMath::Power(curmeanx-x,2)+TMath::Power(curmeany-y,2);
            newpoints++;
        }
    }
    void NewIteration() {
        curmeanx = newmeanx;
        curmeany = newmeany;
        newmeanx = 0;
        newmeany = 0;
        newpoints = 0;
        wcss = 0;
    }
    float Distance(float x, float y) {
        return(TMath::Power(curmeanx - x, 2) + TMath::Power(curmeany - y, 2));
    }
    void Init(float x, float y) {
        Cluster::NewIteration();
        curmeanx=x;
        curmeany=y;
    }
};


Double_t rcos(Double_t *x, Double_t *par)
{
    Double_t mu=par[1];
    Double_t rms=par[2];
    //transfer rms to s -- approximatly but close enough
    Double_t s=TMath::Sqrt(rms*rms/0.13);
    Double_t val;
    val= (1+TMath::Cos( TMath::Pi()*(x[0]-mu)/s));
    val/=(2*s);
    val*=par[0];
    if (( x[0] > ( mu - s)  )&&(x[0]<(mu+s))) {
        return val;
    } else {
        return 0. ;
    }
}

Double_t costfun(Double_t *x,Double_t *par)
{
    Double_t *p1 = &par[0];
    Double_t *p2 = &par[3];
    Double_t *p3 = &par[6];
    //  Double_t result = par[0]*(rcos(x,p1) + p2[0] + p2[1]*x[0] + p2[2]*x[0]*x[0] - TMath::Exp(p3[0]+p3[1]*x[0]));// -expo(p3) ;
    Double_t result = rcos(x,p1) + p2[0] + p2[1]*x[0] + p2[2]*x[0]*x[0] - TMath::Exp(p3[0]+p3[1]*x[0]);// -expo(p3) ;
    return result;
}

Int_t genfuncs(TF1 *cf_length[],TF1 *cf_angle[],Double_t scale=1.)
{
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

    Int_t i;

    Char_t title[50];
    Double_t mean,a,b;

    for (i=0; i<15; i++) {
        lengs[i]*=scale;
        sprintf(title,"cf_angle[%d]",i);
        //    cf_angle[i] = new TF1(title,"gaus(0)+pol2(3)",-TMath::Pi()/2.,TMath::Pi());
        cf_angle[i] = new TF1(title,costfun,-TMath::Pi()/2.,TMath::Pi(),8);
        //    cf_angle[i]->SetParameters(2,angs[i],angs_rms[i]);
        mean=angs[i];
        //   a=-2;
        a=-2;
        b= 2*a*mean;
        if ((i==6)||(i==11)||(i==14)) {
            a=-5;
            b=2*a*mean;
            cf_angle[i]->SetParameters(2.5,mean,2.5*angs_rms[i],b*b/(4*a),-b,a,-1e100,0);
        } else {
            if ( i>11) {
                cf_angle[i]->SetParameters(4,mean,4*angs_rms[i],b*b/(4*a),-b,a,-1e100,0);
            } else {
                cf_angle[i]->SetParameters(.7,mean,angs_rms[i],b*b/(4*a),-b,a,-1e100,0);
            }
        }

        cf_angle[i]->SetParameter(0,cf_angle[i]->GetParameter(0)/cf_angle[i]->Eval(mean));


        sprintf(title,"cf_length[%d]",i);
        //   cf_length[i] = new TF1(title,"gaus(0)+pol2(3)-expo(6)",0,0.5);
        cf_length[i] = new TF1(title,costfun,0,0.5,8);
        mean=lengs[i];
        a=-25;
        b= 2*a*mean;
        if ((i==6)||(i==11)||(i==14)) { // cf_length[i]->SetParameters(2,mean,lengs_rms[i],b*b/(4*a),-b,a,1.5,-10);
            // a=-50;//
            a=-25;
            b= 2*a*mean;
            cf_length[i]->SetParameters(.3,mean,1.25*lengs_rms[i],b*b/(4*a),-b,a,2,-25);
        }
        /*  if ((i==6)||(i==11)||(i==14)) {
          // a will set the slope at which the function decreases for large values
          a=-50;
          // c will be the slope at which the function decreases for small values
          c=-250;
          cf_length[i]->SetParameters(2,mean,lengs_rms[i],b*b/(4*a),-b,a,4,c);}*/
        else {
            if ( i > 11  ) {
                cf_length[i]->SetParameters(.3,mean,1.3*lengs_rms[i],b*b/(4*a),-b,a,3,-75);
            } else {
                cf_length[i]->SetParameters(.3,mean,1.15*lengs_rms[i],b*b/(4*a),-b,a,3,-75);
            }
        }
        //  cf_length[i]->SetParameters(2,mean,lengs_rms[i],b*b/(4*a),-b,a,1.5,-50);
        //   cf_length[i]->SetParameters(2,lengs[i],lengs_rms[i]);
        //   cf_length[i]->SetParameter(0,1/cf_length[i]->GetMaximum());
        cf_length[i]->SetParameter(0,cf_length[i]->GetParameter(0)/cf_length[i]->Eval(mean));
    }
    return 0;
}

Int_t updatepoint(TH2F *hist, Int_t k, Int_t xbin, Int_t ybin, Double_t xx[], Double_t yy[])
{
    xx[k]=hist->GetXaxis()->GetBinCenter(xbin);
    yy[k]=hist->GetYaxis()->GetBinCenter(ybin);

    return 0;
}


Float_t CalcCost(TH2F *hist, Int_t k, Int_t xbin, Int_t ybin, Double_t xx[], Double_t yy[], TF1* cf_length[], TF1* cf_angle[] )
{
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
    if ((k==3)||(k==6)) {
        start=0;
    }
    if ((k==9)||(k==11)) {
        start=7;
    }
    if ((k==13)||(k==14)) {
        start=12;
    }

    length=TMath::Sqrt( TMath::Power(xcurr-xx[start],2)+TMath::Power(ycurr-yy[start],2));
    angle=TMath::ATan2(ycurr-yy[start],xcurr-xx[start]);
    lengthmatch=20*cf_length[k]->Eval(length);
    //  anglematch=3*cf_angle[k]->Eval(angle);
    //  anglematch=5*cf_angle[k]->Eval(angle);
    anglematch=20*cf_angle[k]->Eval(angle);
    //  lengthmatch=0;
    //  anglematch=0;
    if ( length > 1e-3) {
        value = (anglematch+lengthmatch);
    } else {
        value =0 ;
        //    if (verbose) {  cout << xx[start] << " " << yy[start] << "; " <<xcurr << " " << ycurr << endl;}
    }
    // value *= (hist->GetBinContent(xbin,ybin));
    return value;
}

Int_t countcolumn(TH2F *hist, Int_t binx,Int_t biny, Int_t order, bin *max)
{
    Int_t count=0;
    Float_t totcount=0;
    Float_t thisbin=0;
    Int_t i;

    if (order >= 0 ) {
        i=binx+1;
    } else {
        i=binx-1;
    }

    while ((i>=0)&&(i<=hist->GetNbinsX())) {
        thisbin=hist->GetBinContent(i,biny);
        if (thisbin<1e-10) {
            break;
        } else {
            if (thisbin > max->val) {
                max->val=thisbin;
                max->x=i;
                max->y=biny;
            }
            totcount+=thisbin;
            count++;
            if (order >=0 ) {
                i++;
            } else {
                i--;
            }
        }
    }
    return count;
}

Int_t ClusterSize(TH2F *hist, Int_t xbin, Int_t ybin, bin *maxbin)
{
    Int_t pixels=0;
    Int_t neighbour=0;
    Int_t origx=xbin;
    maxbin->x = xbin;
    maxbin->y = ybin;
    maxbin->val = 0;

    if ( hist->GetBinContent(xbin,ybin) < 1e-10) {
        maxbin->pixels=0;
        return maxbin->pixels;
    }


    maxbin->val= hist->GetBinContent(xbin,ybin);

    pixels=1;
    pixels+=countcolumn(hist,xbin,ybin,1,maxbin);
    if(ybin>1) {
        pixels+=countcolumn(hist,xbin,ybin,-1,maxbin);
    }

    Int_t thisflight=0;
    Int_t above,below;
    // check out left neighbor
    neighbour=ybin-1;
    while (neighbour>0) {
        if (hist->GetBinContent(xbin,neighbour) > 1e-10) {
            pixels++;
            thisflight++;
        }
        below=countcolumn(hist,xbin,neighbour,1,maxbin);
        pixels+=below;
        thisflight+=below;
        above=countcolumn(hist,xbin,neighbour,-1,maxbin);
        pixels+=above;
        thisflight+=above;
        if (thisflight==0) {
            break;    // no pixels in column -> stop searching
        }

        // if only pixels above or below we go one pixels down or up in xbin
        neighbour--;
        if ( above == 0 ) {
            // if below is also 0, there was only one pixel in the column and we break the search
            // note that in fact we could alternatively still look in the next column
            if ( below == 0 ) {
                break;
            }
            //find first nonzero bin, looking down
            while (!(hist->GetBinContent(xbin,neighbour))&&(xbin<(origx+3))) {
                xbin++;
            }
        }
        if ( below == 0 ) {
            while (!(hist->GetBinContent(xbin,neighbour))&&(xbin>(origx+3))) {
                xbin--;
            }
        }
        thisflight=0;
    }

    thisflight=0;

    neighbour=ybin+1;
    while (neighbour<hist->GetNbinsY()) {
        if (hist->GetBinContent(xbin,neighbour) > 1e-10) {
            pixels++;
            thisflight++;
        }
        below=countcolumn(hist,xbin,neighbour,1,maxbin);
        pixels+=below;
        thisflight+=below;
        above=countcolumn(hist,xbin,neighbour,-1,maxbin);
        pixels+=above;
        thisflight+=above;
        if (thisflight==0) {
            break;    // no pixels in column -> stop searching
        }
        // if only pixels above or below we go one pixels down or up in xbin
        neighbour++;

        if ( above == 0 ) {
            // if below is also 0, there was only one pixel in the column and we break the search
            // note that in fact we could alternatively still look in the next column
            if ( below == 0 ) {
                break;
            }
            //find first nonzero bin, looking down
            while (!(hist->GetBinContent(xbin,neighbour))&&(xbin<(origx+3))) {
                xbin++;
            }
        }
        if ( below == 0 ) {
            while (!(hist->GetBinContent(xbin,neighbour))&&(xbin>(origx+3))) {
                xbin--;
            }
        }
        thisflight=0;
    }

    maxbin->pixels=pixels;
    return maxbin->pixels;
}

Int_t sort16(Float_t *xpeaks, Float_t *ypeaks, Double_t **sortedxpeaks, Double_t **sortedypeaks, TGraph *peaks_remapped, Float_t xcorner, Float_t ycorner,Int_t verbose)
{
    //  verbose=0;
    if (verbose) {
        cout << "Welcome to the peak sorting algorithm " << endl;
    }

    Int_t i,j,jj,k,l;
    Int_t *map,*xmap,*ymap;
    Int_t *flagged;

    Double_t *remapped_x;
    Double_t *remapped_y;
    Double_t *rdist;
    Double_t *xcost;
    Double_t *ycost;
    Double_t angle,length;
    Int_t curcorner=0;
    Int_t prevpoint;
    remapped_x= new Double_t[SPEAKS];
    remapped_y= new Double_t[SPEAKS];
    rdist = new Double_t[SPEAKS];
    xcost = new Double_t[SPEAKS];
    ycost = new Double_t[SPEAKS];
    map = new Int_t[SPEAKS];
    xmap = new Int_t[SPEAKS];
    ymap = new Int_t[SPEAKS];
    flagged = new Int_t[SPEAKS];


    // First step: calculate and sort distance from origin
    for (i=0; i<SPEAKS; i++) {
        flagged[i]=0;
        rdist[i]=(xpeaks[i]-xcorner)*(xpeaks[i]-xcorner)+(ypeaks[i]-ycorner)*(ypeaks[i]-ycorner);
        xcost[i]=ypeaks[i]+3*xpeaks[i];
        ycost[i]=xpeaks[i]+3*ypeaks[i];
    }

    TMath::Sort(SPEAKS,rdist,map,0);
    TMath::Sort(SPEAKS,xcost,xmap,0);
    TMath::Sort(SPEAKS,ycost,ymap,0);

//  We plan a recursive algorithm, need to find 7, 5, 3 and 1 points in subsequent steps, corners will be at 0,7,12 and 15

    for ( k=0; k<4; k++) {
        // find pont closest to the origin that hasn't been used
        j=0;
        while(1) {
            if (flagged[map[j]]) {
                j++;
            } else {
                break;
            }
        }
        if (verbose) {
            cout << " setting point : " << curcorner << " (corner)  ( j = " << j << " ) @ " << xpeaks[map[j]] << ", " << ypeaks[map[j]]<< endl;
        }
        remapped_x[curcorner]=xpeaks[map[j]];
        remapped_y[curcorner]=ypeaks[map[j]];
        flagged[map[j]]=1;

        for ( l=0; l<(3-k); l++) {
            // find the lowest point in $x$ that hasn't been used ::
            j=0;
            if (l) {
                prevpoint=curcorner+l;
            } else {
                prevpoint=curcorner;
            }
            while(j<16) {
                //      cout << j<<endl;
                if (flagged[xmap[j]]) {
                    j++;
                } else  {
                    // potentially good point, let's check the angle ::
                    angle=atan2(ypeaks[xmap[j]]-remapped_y[curcorner+l],xpeaks[xmap[j]]-remapped_x[curcorner+l]);
                    if (l==0 ) {
                        if (verbose) {
                            cout << " Angle = " << angle << "(k= " << k << ", j= " << j << ")" <<endl;
                        }
                    }
                    if (angle < ((Double_t)4*3.141592/16.)) {
                        if (verbose) {
                            cout << " ---> Angle too small " << endl;
                        }
                        //increase j and start loop again
                        j++;
                        continue ;
                    } else { // we have a valid angle, let's do some more checks ::
                        if (verbose) {
                            cout << "length line segment : " ;
                            length= TMath::Sqrt(TMath::Power(ypeaks[xmap[j]]-remapped_y[prevpoint],2)+
                                                TMath::Power(xpeaks[xmap[j]]-remapped_x[prevpoint],2));
                            cout << length << " ( prevpoint = " << prevpoint << ")";
                            cout <<  "(x,y)_prev = (" << remapped_x[prevpoint] << "," << remapped_y[prevpoint] <<"); (x,y)_cur = (";
                            cout << xpeaks[xmap[j]] << "," << ypeaks[xmap[j]] << "); ( j = " << j << " ).";
                            cout <<endl;
                            cout << " l = " << l << " k = " << k << endl;
                        }
                        if (l!=(2-k)) {
                            /* check next unused point */
                            // late night fix: sometimes the next point along $x$ is taken first ( a point with higher $x$ value ) ,
                            // that results in an error, which will appear from the angle between the current candidate point and
                            // the next point to be around -90 degrees, smaller than 0  which we check for.
                            if (j<15) {
                                jj=j+1;
                            } else {
                                jj=j;
                            }
                            while (jj<16) {
                                if (flagged[xmap[jj]]) {
                                    jj++;
                                } else {
                                    break;
                                }
                            }

                            if (verbose) {
                                cout << " jj after  while loop :: " << jj << endl;
                            }
                            if (jj>15) {
                                return -1;    // AVDB 5-7-2014 ADDED THIS STATEMENT
                            }
                            angle=atan2(ypeaks[xmap[jj]]-ypeaks[xmap[j]],xpeaks[xmap[jj]]-xpeaks[xmap[j]]);
                            length=TMath::Sqrt(TMath::Power(ypeaks[xmap[j]]-ypeaks[xmap[jj]],2)+
                                               TMath::Power(xpeaks[xmap[j]]-xpeaks[xmap[jj]],2));
                            if (verbose) {
                                cout << " Angle with next point :: " ;
                                cout << angle << ", length = " << length;
                                cout << " (x,y)_next = (" << xpeaks[xmap[jj]] << "," << ypeaks[xmap[jj]] <<")" << endl;
                            }
                            if ((length<0.075 )&&(angle < 0.)) {
                                if (verbose) {
                                    cout << " ----> Negative, increasing j !  (cur j=" << j <<")"<<endl;
                                }
                                j++;
                            }
                            //     else { //
                            //we finish the loop here, because the situation we checked for only occurs if the wrong point along the same
                            // line was choosen
                            break;//}
                            // check of next unused point
                        }// we need this break statement for the case that l == (2-k)
                        else {
                            break;
                        }
                    } // first angle check passed
                } // unused point
            } // while loop

            if (verbose) {
                cout << " curcorner : " << curcorner << ", l = " << l << endl;
            }

            if (verbose) {
                if (j<16) {
                    cout << " setting point : " << curcorner+l+1 << " (j=" << j << "); x=" << xpeaks[xmap[j]] << ", y=" << ypeaks[xmap[j]]<<endl;
                }
            }
            if (j>15) {
                j=curcorner+l+1;
            }
            remapped_x[curcorner+l+1]=xpeaks[xmap[j]];
            remapped_y[curcorner+l+1]=ypeaks[xmap[j]];
            flagged[xmap[j]]=1;
        }


        for ( l=0; l<(3-k); l++) {
            // find the lowest point in $y$ that hasn't been used ::
            j=0;
            if (l) {
                prevpoint=curcorner+l+(3-k);
            } else {
                prevpoint=curcorner;
            }
            while(j<16) {
                if (flagged[ymap[j]]) {
                    j++;
                } else  {
                    if (verbose) {
                        cout << "length line segment : " ;
                        length= TMath::Sqrt(TMath::Power(ypeaks[ymap[j]]-remapped_y[prevpoint],2)+
                                            TMath::Power(xpeaks[ymap[j]]-remapped_x[prevpoint],2));
                        cout << length << " ( prevpoint = " << prevpoint << ")";
                        cout <<  "(x,y)_prev = (" << remapped_x[prevpoint] << "," << remapped_y[prevpoint] <<"); (x,y)_cur = (";
                        cout << xpeaks[ymap[j]] << "," << ypeaks[ymap[j]] << ")";
                        cout <<endl;
                    }
                    if (l!=(2-k)) {
                        /* check next unused point */
                        // late night fix: sometimes the next point along $x$ is taken first ( a point with higher $x$ value ) ,
                        // that results in an error, which will appear from the angle between the current candidate point and
                        // the next point to be almost 180 degrees, much larger than 90 which we check for.
                        jj=j+1;
                        while (jj<16) {
                            if (flagged[ymap[jj]]) {
                                jj++;
                            } else {
                                break;
                            }
                        }
                        angle=atan2(ypeaks[ymap[jj]]-ypeaks[ymap[j]],xpeaks[ymap[jj]]-xpeaks[ymap[j]]);

                        if (verbose) {
                            cout << " Angle with next point :: " ;
                            cout << angle << " (x,y)_next = (" << xpeaks[ymap[jj]] << "," << ypeaks[ymap[jj]] <<")" << endl;
                        }
                        if ( angle > TMath::Pi()/2. ) {
                            j++;
                        }
                    }
                    break;
                }
            }
            if (verbose) {
                cout << " setting point : " << curcorner+l+1+(3-k) << endl;
            }
            remapped_x[curcorner+l+1+(3-k)]=xpeaks[ymap[j]];
            remapped_y[curcorner+l+1+(3-k)]=ypeaks[ymap[j]];
            flagged[ymap[j]]=1;
        }

        if (verbose) {
            cout << " Corner " << k << ": " << remapped_x[curcorner] << " "<<  remapped_y[curcorner] <<endl;
        }
        curcorner+=(4-k)*2-1;

    } // loop over k

    *sortedxpeaks=remapped_x;
    *sortedypeaks=remapped_y;

    for (i=0; i<SPEAKS; i++) {
        peaks_remapped->SetPoint(i,remapped_x[i],remapped_y[i]);
    }

    return 0;
}


Int_t mergegraphs(TGraph *graphs[4], TGraph *merged) {
    Double_t *xpeaks;
    xpeaks = new Double_t[64];
    Double_t *ypeaks;
    ypeaks = new Double_t[64];
    Double_t *xps[4];
    Double_t *yps[4];
    Int_t i,j;

    for (i=0; i<4; i++) {
        xps[i] = graphs[i]->GetX();
        yps[i] = graphs[i]->GetY();
    }

    for (i=0; i<4; i++) {
        for (j=0; j<16; j++) {
            xpeaks[j+i*16] = xps[i][j];
            ypeaks[j+i*16] = yps[i][j];
            switch (i) {
            case 0:
                break;
            case 1:
                ypeaks[j+i*16]*=-1;
                break;
            case 2:
                xpeaks[j+i*16]*=-1;
                ypeaks[j+i*16]*=-1;
                break;
            case 3:
                xpeaks[j+i*16]*=-1;
                break;
            }
        }
    }

    merged->SetPoint(0,xpeaks[47],ypeaks[47]);
    merged->SetPoint(1,xpeaks[46],ypeaks[46]);
    merged->SetPoint(2,xpeaks[43],ypeaks[43]);
    merged->SetPoint(3,xpeaks[38],ypeaks[38]);
    merged->SetPoint(4,xpeaks[54],ypeaks[54]);
    merged->SetPoint(5,xpeaks[59],ypeaks[59]);
    merged->SetPoint(6,xpeaks[62],ypeaks[62]);
    merged->SetPoint(7,xpeaks[63],ypeaks[63]);
    merged->SetPoint(8,xpeaks[45],ypeaks[45]);
    merged->SetPoint(9,xpeaks[44],ypeaks[44]);
    merged->SetPoint(10,xpeaks[42],ypeaks[42]);
    merged->SetPoint(11,xpeaks[37],ypeaks[37]);
    merged->SetPoint(12,xpeaks[53],ypeaks[53]);
    merged->SetPoint(13,xpeaks[58],ypeaks[58]);
    merged->SetPoint(14,xpeaks[60],ypeaks[60]);
    merged->SetPoint(15,xpeaks[61],ypeaks[61]);
    merged->SetPoint(16,xpeaks[41],ypeaks[41]);
    merged->SetPoint(17,xpeaks[40],ypeaks[40]);
    merged->SetPoint(18,xpeaks[39],ypeaks[39]);
    merged->SetPoint(19,xpeaks[36],ypeaks[36]);
    merged->SetPoint(20,xpeaks[52],ypeaks[52]);
    merged->SetPoint(21,xpeaks[55],ypeaks[55]);
    merged->SetPoint(22,xpeaks[56],ypeaks[56]);
    merged->SetPoint(23,xpeaks[57],ypeaks[57]);
    merged->SetPoint(24,xpeaks[35],ypeaks[35]);
    merged->SetPoint(25,xpeaks[34],ypeaks[34]);
    merged->SetPoint(26,xpeaks[33],ypeaks[33]);
    merged->SetPoint(27,xpeaks[32],ypeaks[32]);
    merged->SetPoint(28,xpeaks[48],ypeaks[48]);
    merged->SetPoint(29,xpeaks[49],ypeaks[49]);
    merged->SetPoint(30,xpeaks[50],ypeaks[50]);
    merged->SetPoint(31,xpeaks[51],ypeaks[51]);
    merged->SetPoint(32,xpeaks[19],ypeaks[19]);
    merged->SetPoint(33,xpeaks[18],ypeaks[18]);
    merged->SetPoint(34,xpeaks[17],ypeaks[17]);
    merged->SetPoint(35,xpeaks[16],ypeaks[16]);
    merged->SetPoint(36,xpeaks[0],ypeaks[0]);
    merged->SetPoint(37,xpeaks[1],ypeaks[1]);
    merged->SetPoint(38,xpeaks[2],ypeaks[2]);
    merged->SetPoint(39,xpeaks[3],ypeaks[3]);
    merged->SetPoint(40,xpeaks[25],ypeaks[25]);
    merged->SetPoint(41,xpeaks[24],ypeaks[24]);
    merged->SetPoint(42,xpeaks[23],ypeaks[23]);
    merged->SetPoint(43,xpeaks[20],ypeaks[20]);
    merged->SetPoint(44,xpeaks[4],ypeaks[4]);
    merged->SetPoint(45,xpeaks[7],ypeaks[7]);
    merged->SetPoint(46,xpeaks[8],ypeaks[8]);
    merged->SetPoint(47,xpeaks[9],ypeaks[9]);
    merged->SetPoint(48,xpeaks[29],ypeaks[29]);
    merged->SetPoint(49,xpeaks[28],ypeaks[28]);
    merged->SetPoint(50,xpeaks[26],ypeaks[26]);
    merged->SetPoint(51,xpeaks[21],ypeaks[21]);
    merged->SetPoint(52,xpeaks[5],ypeaks[5]);
    merged->SetPoint(53,xpeaks[10],ypeaks[10]);
    merged->SetPoint(54,xpeaks[12],ypeaks[12]);
    merged->SetPoint(55,xpeaks[13],ypeaks[13]);
    merged->SetPoint(56,xpeaks[31],ypeaks[31]);
    merged->SetPoint(57,xpeaks[30],ypeaks[30]);
    merged->SetPoint(58,xpeaks[27],ypeaks[27]);
    merged->SetPoint(59,xpeaks[22],ypeaks[22]);
    merged->SetPoint(60,xpeaks[6],ypeaks[6]);
    merged->SetPoint(61,xpeaks[11],ypeaks[11]);
    merged->SetPoint(62,xpeaks[14],ypeaks[14]);
    merged->SetPoint(63,xpeaks[15],ypeaks[15]);
    return 0;
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


Double_t findmean(TH1D * h1) {
    Double_t leftmax = 0;
    Int_t leftbinmax = 0;
    Double_t rightmax = 0;
    Int_t rightbinmax = 0;
    h1->SetAxisRange(-.25,.25);
    Int_t meanbin = h1->FindBin(h1->GetMean());
    normalise_1(h1);

    Int_t initialbin = h1->FindBin(0);

    //we want to start from a low value ::
    if ( h1->GetBinContent(meanbin) < h1->GetBinContent(initialbin)) {
        initialbin=meanbin;
    }
    Double_t curval=0;
    //start a little right from zero and go left
    for (int i = initialbin; i > (initialbin - 35); i--) {
        Double_t prevval = curval;
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
    curval=0;
    for (int i = initialbin; i < (initialbin + 35); i++) {
        Double_t prevval = curval;
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
    Double_t mean = (h1->GetBinCenter(rightbinmax) +
                     h1->GetBinCenter(leftbinmax)) / 2.0;

    return mean;
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

Int_t kmeans2d(TH2F *hist, Float_t *x, Float_t *y) {
    Cluster clus[4];
    Double_t d[4];
    Int_t kind[4]= {0,1,2,3};
    clus[0].Init(.3,.3);
    clus[1].Init(.3,-.3);
    clus[2].Init(-.3,.3);
    clus[3].Init(-.3,-.3);

    Double_t prevmatch;
    Double_t totmatch =0,xav=0,yav=0;

    for(Int_t l=0; l<10; l++) {
        Float_t maxval = hist->GetMaximum();
        for (Int_t i=0; i<256; i++) {
            for (Int_t j=0; j<256; j++) {
                if (hist->GetBinContent(i,j) > 0) {
                    for (Int_t k = 0; k < 4; k++) {
                        d[k] = clus[k].Distance(hist->GetXaxis()->GetBinCenter(i),hist->GetYaxis()->GetBinCenter(j));
                    }
                    TMath::Sort(4, d, kind, kFALSE);
                    if (hist->GetBinContent(i, j) > (maxval * 0.2)) {
                        clus[kind[0]].Add(hist->GetXaxis()->GetBinCenter(i),hist->GetYaxis()->GetBinCenter(j),5);
                    } else {
                        clus[kind[0]].Add(hist->GetXaxis()->GetBinCenter(i),hist->GetYaxis()->GetBinCenter(j),1);
                    }
                }
            }
        }

        totmatch =0;
        xav=0;
        yav=0;
        for (Int_t k=0; k<4; k++) {
            totmatch += clus[k].wcss;
            if (k<2) {
                xav += clus[k].newmeanx;
            } else {
                xav -= clus[k].newmeanx ;
            }
            if (k%2) {
                yav -= clus[k].newmeany;
            } else {
                yav += clus[k].newmeany;
            }
            clus[k].NewIteration();
        }

        *x=(clus[0].curmeanx+clus[1].curmeanx)/2.-xav/4.;
        *y=(clus[0].curmeany+clus[2].curmeany)/2.-yav/4.;

        if (((prevmatch - totmatch) < 0.01) && (l > 2) ) {
            l = 10000;
        }
        prevmatch = totmatch;
    }

    return(0);
}

int PeakSearch(
        TGraph * peaks_sorted,
        TH2F *flood,
        Int_t verbose,
        Int_t &validflag,
        Float_t &cost,
        Bool_t APD)
{

    cost=999.999;

    /* Step one: split in four */
    TH2F *floodsq[4];
    TH1D *projX;
    TH1D *projY;
    Float_t meanX,meanY;
    Int_t midbinx,midbiny,binsx,binsy,newbin;
    Double_t xval,yval,value;

    /* find mean based on projection*/
    Float_t xc,yc;
    kmeans2d( flood, &xc, &yc);

    projX = (TH1D*) flood->ProjectionX("",120,136);
    projX->SetName("projX");

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

    meanY = findmean8(projY,verbose,0,APD);

    // if findmean8 doesn't identify 8 peaks in the projection histogram we do an alternative methode.

    if (meanX==12345) {
        projX = (TH1D*) flood->ProjectionX();
        meanX = findmean(projX);
    }

    if (meanY==12345) {
        projY = (TH1D*) flood->ProjectionY();
        meanY = findmean(projY);
    }

    projX->SetAxisRange(-.25,.25);
    projY->SetAxisRange(-.25,.25);


    // overwrite with k-means value
    meanX = xc;
    meanY = yc;

    midbinx=flood->GetXaxis()->FindBin(meanX);
    midbiny=flood->GetYaxis()->FindBin(meanY);

    Int_t i,j;

    /* creating the histograms */
    for (int k = 0; k < 4; k++) {
        Char_t tmpstr[40];
        Char_t tmpstr2[220];
        sprintf(tmpstr,"floodsq[%d]", k);
        sprintf(tmpstr2,"Quadrant %d", k);
        switch(k) {
        case 0:
            binsx=(1-(meanX-(projX->GetBinWidth(1)/2.)))/projX->GetBinWidth(1);
            binsy=(1-(meanY-(projY->GetBinWidth(1)/2.)))/projY->GetBinWidth(1);
            floodsq[k]= new TH2F(tmpstr,tmpstr2,binsx,meanX-(projX->GetBinWidth(1)/2.),1,binsy,(meanY-(projY->GetBinWidth(1)/2.)),1);
            break;
        case 1:
            binsx=(1-(meanX-(projX->GetBinWidth(1)/2.)))/projX->GetBinWidth(1);
            binsy=(1+(meanY-(projY->GetBinWidth(1)/2.)))/projY->GetBinWidth(1);
            floodsq[k]= new TH2F(tmpstr,tmpstr2,binsx,meanX-(projX->GetBinWidth(1)/2.),1,binsy,-(meanY-(projY->GetBinWidth(1)/2.)),1);
            break;
        case 2:
            binsx=(1+(meanX-(projX->GetBinWidth(1)/2.)))/projX->GetBinWidth(1);
            binsy=(1+(meanY-(projY->GetBinWidth(1)/2.)))/projY->GetBinWidth(1);
            floodsq[k]= new TH2F(tmpstr,tmpstr2,binsx,-(meanX-(projX->GetBinWidth(1)/2.)),1,binsy,-(meanY-(projY->GetBinWidth(1)/2.)),1);
            break;
        case 3:
            binsx=(1+(meanX-(projX->GetBinWidth(1)/2.)))/projX->GetBinWidth(1);
            binsy=(1-(meanY-(projY->GetBinWidth(1)/2.)))/projY->GetBinWidth(1);
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

#ifdef SRT16

// Loop over four quadrants

        npeaks_q[k]=peaks[k]->Search(floodsq[k],1.2,"noMarkov",0.15);
        if (npeaks_q[k] != 16 ) {
            npeaks_q[k]=peaks[k]->Search(floodsq[k],2.0,"noMarkov",0.1);
        }

        if (npeaks_q[k] != 16 ) {
            npeaks_q[k]=peaks[k]->Search(floodsq[k],1.4,"noMarkov",0.15);
        }

        if (npeaks_q[k] < 16 ) {
            npeaks_q[k]=peaks[k]->Search(floodsq[k],1.1,"noMarkov",0.15);
        }
        // if too many peaks, increase widht of peak searching algorithm
        if (npeaks_q[k] > 16 ) {
            npeaks_q[k]=peaks[k]->Search(floodsq[k],2.0,"noMarkov",0.15);
        }
        if (npeaks_q[k] > 16 ) {
            npeaks_q[k]=peaks[k]->Search(floodsq[k],3.0,"noMarkov",0.15);
        }

        if (npeaks_q[k] > 16 ) {
            npeaks_q[k]=peaks[k]->Search(floodsq[k],3.75,"noMarkov",0.15);
        }

        if (npeaks_q[k] > 16 ) {
            npeaks_q[k]=peaks[k]->Search(floodsq[k],3.75,"noMarkov",0.4);
        }

        if (npeaks_q[k] != 16 ) {
            npeaks_q[k]=peaks[k]->Search(floodsq[k],1.11,"noBackgroundnoMarkov",0.35);
        }

        // sorting the peaks

        if (npeaks_q[k] == 16 ) {
            xpeaks_q[k]=peaks[k]->GetPositionX();
            ypeaks_q[k]=peaks[k]->GetPositionY();

            xcorner=floodsq[k]->GetXaxis()->GetBinCenter(0);
            ycorner=floodsq[k]->GetYaxis()->GetBinCenter(0);

            sort16(xpeaks_q[k],ypeaks_q[k],&ysort_q[k],&xsort_q[k],peaks_remapped[k],xcorner,ycorner,verbose);
            if ( ! validate_sort(peaks_remapped[k],verbose) ) {
                validflag |= ( 1 << k) ;
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

    mergegraphs(peaks_remapped, peaks_sorted);

    for (int k=0; k<4; k++) {
        delete floodsq[k];
    }

    return(0);
}

void usage() {
    cout << "fit_floods [-vh] -c [config] -f [filename]\n"
         << "  -o [name]    : crystal location output filename\n"
         << endl;
}

int main(int argc, char ** argv) {
    if (argc == 1) {
        usage();
        return(0);
    }

    bool verbose = false;
    string filename;
    string filename_config;
    string filename_output;

    // Arguments not requiring input
    for (int ix = 1; ix < argc; ix++) {
        string argument(argv[ix]);
        if (argument == "-v") {
            verbose = true;
            cout << "Running in verbose mode " << endl;
        }
        if (argument == "-h" || argument == "--help") {
            usage();
            return(0);
        }
    }
    // Arguments requiring input
    for (int ix = 1; ix < (argc - 1); ix++) {
        string argument(argv[ix]);
        string following_argument(argv[ix + 1]);
        if (argument == "-f") {
            filename = following_argument;
        }
        if (argument == "-c") {
            filename_config = following_argument;
        }
        if (argument == "-o") {
            filename_output = following_argument;
        }
    }

    if (filename == "") {
        cerr << "No input filename specified" << endl;
        return(-4);
    }

    if (filename_output == "") {
        filename_output = filename + ".loc";
    }

    if (verbose) {
        cout << "filename       : " << filename << endl;
        cout << "filename_config: " << filename_config << endl;
        cout << "filename_output: " << filename_output << endl;
    }

    SystemConfiguration config;
    int config_load_status = config.load(filename_config);
    if (verbose) {
        cout << "config_load_status: " << config_load_status << endl;
    }
    if (config_load_status < 0) {
        cerr << "SystemConfiguration.load() failed with status: "
             << config_load_status
             << endl;
        return(-2);
    }


    if (verbose) {
        cout << "loading floods: " << filename << endl;
    }
    vector<vector<vector<vector<vector<TH2F*> > > > > floods;
    vector<vector<vector<vector<vector<TGraph*> > > > > peaks;
    config.resizeArrayPCFMA<TH2F*>(floods, 0);
    config.resizeArrayPCFMA<TGraph*>(peaks, 0);


    TFile * input_file = new TFile(filename.c_str());
    if (input_file->IsZombie()) {
        cerr << "Unable to open root file: " << filename << endl;
        return(-3);
    }

    for (int p = 0; p < config.panels_per_system; p++) {
        for (int c = 0; c < config.cartridges_per_panel; c++) {
            for (int f = 0; f < config.fins_per_cartridge; f++) {
                for (int m = 0; m < config.modules_per_fin; m++) {
                    for (int a = 0; a < config.apds_per_module; a++) {

                        // Create all of the 2D flood histograms with the
                        // appropriate names.
                        char name_string[30];
                        sprintf(name_string,
                                "floods[%d][%d][%d][%d][%d]",
                                p, c, f, m, a);
                        floods[p][c][f][m][a] =
                                (TH2F*) input_file->Get(name_string);
                        if (!floods[p][c][f][m][a]) {
                            cerr << "Unable to get flood: " << name_string << endl;
                        }


                        sprintf(name_string,
                                "peaks[%d][%d][%d][%d][%d]",
                                p, c, f, m, a);
                        peaks[p][c][f][m][a] = new TGraph(64);
                        peaks[p][c][f][m][a]->SetName(name_string);
                    }
                }
            }
        }
    }
    input_file->Close();


    for (int p = 0; p < config.panels_per_system; p++) {
        for (int c = 0; c < config.cartridges_per_panel; c++) {
            for (int f = 0; f < config.fins_per_cartridge; f++) {
                for (int m = 0; m < config.modules_per_fin; m++) {
                    for (int a = 0; a < config.apds_per_module; a++) {
                        Int_t validflag;
                        Float_t cost;
                        PeakSearch(
                                peaks[p][c][f][m][a],
                                floods[p][c][f][m][a],
                                0, validflag, cost, a);
                    }
                }
            }
        }
    }


//    long events_filled = 0;
//    long events_egated = 0;
//    if (verbose) {
//        cout << "Filling Flood Histograms" << endl;
//    }
//    deque<char> file_data;
//    ProcessInfo info;
//    for (size_t ii = 0; ii < filenames.size(); ii++) {
//        string & filename = filenames[ii];
//        int read_status = readFileIntoDeque(filename, file_data);
//        if (verbose) {
//            cout << filename << " read with status: " << read_status << endl;
//        }
//        if (read_status < 0) {
//            cerr << "Unable to load: " << filename << endl;
//            return(-3);
//        }
//        vector<EventRaw> raw_events;

//        ProcessParams::DecodeBuffer(file_data, raw_events, info, &config);
//        ProcessParams::ClearProcessedData(file_data, info);
//        for (size_t ii = 0; ii < raw_events.size(); ii++) {
//            EventRaw & event = raw_events[ii];
//            int apd, module, fin;
//            float x, y, energy;
//            int calc_status = CalculateXYandEnergy(
//                        event, &config, x, y, energy, apd, module, fin);
//            if (calc_status < 0) {
//                continue;
//            }

//            TH2F * flood =
//                    floods[event.panel][event.cartridge][fin][module][apd];

//            float egate_lo =
//                    egate_los[event.panel][event.cartridge][fin][module][apd];
//            float egate_hi =
//                    egate_his[event.panel][event.cartridge][fin][module][apd];

//            if ((energy < egate_hi) && (energy > egate_lo)) {
//                flood->Fill(x, y);
//                events_filled++;
//            } else {
//                events_egated++;
//            }
//        }
//    }

//    if (verbose) {
//        cout << info.getDecodeInfo();
//    }

//    if (verbose) {
//        cout << "Events Used in Floods  : " << events_filled << endl;
//        cout << "Events Rejected (egate): " << events_egated << endl;
//    }

//    if (verbose) {
//        cout << "Writing out floods: " << filename_output << endl;
//    }

//    TFile * output_file = new TFile(filename_output.c_str(), "RECREATE");
//    output_file->cd();
//    for (int p = 0; p < config.panels_per_system; p++) {
//        for (int c = 0; c < config.cartridges_per_panel; c++) {
//            for (int f = 0; f < config.fins_per_cartridge; f++) {
//                for (int m = 0; m < config.modules_per_fin; m++) {
//                    for (int a = 0; a < config.apds_per_module; a++) {
//                        TH2F * flood = floods[p][c][f][m][a];
//                        flood->Write();
//                    }
//                }
//            }
//        }
//    }
//    output_file->Close();
    return(0);
}
