//#define DEBUG
#include "DetCluster.h"


/*
This algorithm is not 100 % accurate, but I intend to use it as a minimum, the inprecision in the algorithm will provide a lower limit of the cluster size only ( because countcolumn stops when cells are 0 in the array  so ( 0 xx 0 0 xx xx xx 0 0 ) would either count as one or 3, but not as 4.
*/


// 07182013 :: change maxbin to pointer

Int_t ClusterSize(TH2F *hist, Int_t xbin, Int_t ybin, bin *maxbin)
{

    Int_t pixels=0;
    Int_t neighbour=0;
    Int_t origx=xbin;
    maxbin->x = xbin;
    maxbin->y = ybin;
    maxbin->val = 0;

#ifdef DEBUG
    cout << "Welcome to clustersize xbin = " << xbin << "; ybin = " << ybin <<endl;
    Int_t i,j,size,totsum;
    for ( i=xbin-1; i<=xbin+1; i++ ) {
        for ( j=ybin-1; j<=ybin+1; j++ ) {
            cout << hist->GetBinContent(i,j) << "  " ;
            if ((i!=xbin)&&(j!=ybin)) {
                if (hist->GetBinContent(i,j)) {
                    size++;
                }
                totsum+=hist->GetBinContent(i,j);
            }
        }
        cout << endl;
    }

#endif

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

#ifdef DEBUG
    cout << " Maximum pixel value : " << maxbin->val << endl;
    cout << " ========================== Looking Left ===================== " << endl;
#endif

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
#ifdef DEBUG
        cout << " Maximum pixel value : " << maxbin->val << endl;
        cout << " Found " << thisflight << " pixels in column around " << xbin;
        cout << "," << neighbour << "(above="<< above <<", below="<< below <<")"<<endl;
#endif
        //        neighbour++;

        neighbour--;
        if ( above == 0 ) {
            // if below is also 0, there was only one pixel in the column and we break the search
            // note that in fact we could alternatively still look in the next column
            if ( below == 0 ) {
                break;
            }
            //find first nonzero bin, looking down
            //	   	  while (!(hist->GetBinContent(xbin,neighbour))&&(xbin<hist->GetNbinsX())){
            while (!(hist->GetBinContent(xbin,neighbour))&&(xbin<(origx+3))) {
#ifdef DEBUG

                cout << hist->GetBinContent(xbin,neighbour) << " ";
                cout << " xbin = " << xbin  << " neighbour = " << neighbour << endl;
#endif
                xbin++;
            }
        }
        if ( below == 0 ) {
            //	  while (!(hist->GetBinContent(xbin,neighbour))&&(xbin>=1)){
            while (!(hist->GetBinContent(xbin,neighbour))&&(xbin>(origx+3))) {
#ifdef DEBUG
                cout << hist->GetBinContent(xbin,neighbour) << " ";
                cout << " xbin = " << xbin  << " neighbour = " << neighbour << endl;
#endif
                xbin--;
            }
        }
        thisflight=0;


#ifdef DEBUG
        cout << " Maximum pixel value : " << maxbin->val  << endl;
        cout << " Found " << thisflight << " pixels in column around " << xbin;
        cout << "," << neighbour << endl;
#endif
        //        neighbour--;
        thisflight=0;
    }

    thisflight=0;

#ifdef DEBUG
    cout << " ========================== Looking Right ===================== " << endl;
#endif

    neighbour=ybin+1;
    while (neighbour<hist->GetNbinsY()) {
#ifdef DEBUG
        cout << " Searching in column " << xbin << "," << neighbour ;
        cout << " (value = " << hist->GetBinContent(xbin,neighbour) <<")" << endl;
#endif
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
#ifdef DEBUG
        cout << " Found " << thisflight << " pixels in column around " << xbin;
        cout << "," << neighbour << "(above="<< above <<", below="<< below <<")"<<endl;
#endif
        neighbour++;

        //        if ( thisflight!=1) neighbour++;
        if ( above == 0 ) {
            // if below is also 0, there was only one pixel in the column and we break the search
            // note that in fact we could alternatively still look in the next column
            if ( below == 0 ) {
                break;
            }
            //find first nonzero bin, looking down
            //	   	  while (!(hist->GetBinContent(xbin,neighbour))&&(xbin<hist->GetNbinsX())){
            while (!(hist->GetBinContent(xbin,neighbour))&&(xbin<(origx+3))) {
#ifdef DEBUG
                cout << hist->GetBinContent(xbin,neighbour) << " ";
                cout << " xbin = " << xbin  << " neighbour = " << neighbour << endl;
#endif
                xbin++;
            }
        }
        if ( below == 0 ) {
            //	  while (!(hist->GetBinContent(xbin,neighbour))&&(xbin>=1)){
            while (!(hist->GetBinContent(xbin,neighbour))&&(xbin>(origx+3))) {
#ifdef DEBUG
                cout << hist->GetBinContent(xbin,neighbour) << " ";
                cout << " xbin = " << xbin  << " neighbour = " << neighbour << endl;
#endif
                xbin--;
            }
        }
        //        if ( thisflight==1) neighbour++;
        thisflight=0;
    }

    maxbin->pixels=pixels;
#ifdef DEBUG
    cout << endl;
    cout << " Maximum pixel value : " << maxbin->val  ;
    cout << " at position : " << maxbin->x << "," << maxbin->y <<endl;
    cout << " Found " << pixels << " pixels in the cluster " << endl;
    cout << " ---------------- END DETCLUSTER ---------------- " << endl;
#endif

    return maxbin->pixels;
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

#ifdef DEBUG
    cout <<" Function Countcolumn. i = " << i << ", order = " << order << " hist->GetNbinsx() = " << hist->GetNbinsX() << endl;
#endif

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
#ifdef DEBUG
    cout << count << " bins found with total content : " << totcount;
    cout << ", maximum bin content = " << max->val << endl;

#endif
    return count;
}

/*

Int_t floodfillcount(TH2F *hist, Int_t binx, Int_t biny){
  Int_t pixels;
  if (hist->GetBinContent(binx,biny)==0) return 0;
  else {
    pixels=1;
    cout << " Going East :: ( this bin = " << hist->GetBinContent(binx,biny) << ")" << endl;
    pixels+=floodfillcount(hist,binx-1,biny-1);
    cout << " Going West :: ( this bin = " << hist->GetBinContent(binx,biny) << ")" << endl;
    pixels+=floodfillcount(hist,binx+1,biny+1);
    //    pixels+=floodfillcount(hist,binx-1,biny);
    //    pixels+=floodfillcount(hist,binx+1,biny);
    return pixels;}
}
*/
