#include "TROOT.h"
#include "Riostream.h"
#include "TMath.h"
#include "TGraph.h"

#include "apd_sort16.h"
#define SPEAKS 16

/* This version deals with the more pincushioned floods with the capacitor in between channels */
/* July 2012 AVDB */

Int_t sort16(Int_t npeaks, Float_t *xpeaks, Float_t *ypeaks, Double_t **sortedxpeaks, Double_t **sortedypeaks, TGraph *peaks_remapped, Float_t xcorner, Float_t ycorner,Int_t verbose){
  //  verbose=0;
  if (verbose) cout << "Welcome to the peak sorting algorithm " << endl;

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
  xcorner;
  ycorner;



  // First step: calculate and sort distance from origin
    for (i=0;i<SPEAKS;i++){
      flagged[i]=0;
      rdist[i]=(xpeaks[i]-xcorner)*(xpeaks[i]-xcorner)+(ypeaks[i]-ycorner)*(ypeaks[i]-ycorner);
      xcost[i]=ypeaks[i]+3*xpeaks[i];
      ycost[i]=xpeaks[i]+3*ypeaks[i];
    }

    TMath::Sort(SPEAKS,rdist,map,0);
    TMath::Sort(SPEAKS,xcost,xmap,0);
    TMath::Sort(SPEAKS,ycost,ymap,0);

 //  We plan a recursive algorithm, need to find 7, 5, 3 and 1 points in subsequent steps, corners will be at 0,7,12 and 15

 for ( k=0;k<4;k++){
   // find pont closest to the origin that hasn't been used
   j=0;
   while(1) { if (flagged[map[j]]) j++;
     else 
         break; }
   if (verbose)  cout << " setting point : " << curcorner << " (corner)  ( j = " << j << " ) @ " << xpeaks[map[j]] << ", " << ypeaks[map[j]]<< endl;
  remapped_x[curcorner]=xpeaks[map[j]];
  remapped_y[curcorner]=ypeaks[map[j]];
  flagged[map[j]]=1;

  for ( l=0;l<(3-k);l++){
    // find the lowest point in $x$ that hasn't been used ::
    j=0;
    if (l) prevpoint=curcorner+l;
    else prevpoint=curcorner;
    while(j<16) {
      //      cout << j<<endl;
      if (flagged[xmap[j]]) j++;        
      else  { 
	// potentially good point, let's check the angle :: 
          angle=atan2(ypeaks[xmap[j]]-remapped_y[curcorner+l],xpeaks[xmap[j]]-remapped_x[curcorner+l]);
  	  if (l==0 ) { if (verbose) cout << " Angle = " << angle << "(k= " << k << ", j= " << j << ")" <<endl;}
          if (angle < ((Double_t)4*3.141592/16.)) {
                if (verbose) cout << " ---> Angle too small " << endl; 
                //increase j and start loop again
                j++; continue ; }
          else{  // we have a valid angle, let's do some more checks ::
            if (verbose) { cout << "length line segment : " ;
                       length= TMath::Sqrt(TMath::Power(ypeaks[xmap[j]]-remapped_y[prevpoint],2)+
                                           TMath::Power(xpeaks[xmap[j]]-remapped_x[prevpoint],2));
                       cout << length << " ( prevpoint = " << prevpoint << ")";
                       cout <<  "(x,y)_prev = (" << remapped_x[prevpoint] << "," << remapped_y[prevpoint] <<"); (x,y)_cur = (";
                       cout << xpeaks[xmap[j]] << "," << ypeaks[xmap[j]] << ")";
                       cout <<endl;  } 
        if (l!=(2-k)){
	  /* check next unused point */
	  // late night fix: sometimes the next point along $x$ is taken first ( a point with higher $x$ value ) ,
          // that results in an error, which will appear from the angle between the current candidate point and 
	  // the next point to be around -90 degrees, smaller than 0  which we check for. 
	  jj=j+1;
          while (jj<16) { if (flagged[xmap[jj]]) jj++;
			  else break;}
        	       angle=atan2(ypeaks[xmap[jj]]-ypeaks[xmap[j]],xpeaks[xmap[jj]]-xpeaks[xmap[j]]);
                       length=TMath::Sqrt(TMath::Power(ypeaks[xmap[j]]-ypeaks[xmap[jj]],2)+
                                           TMath::Power(xpeaks[xmap[j]]-xpeaks[xmap[jj]],2));
		       if (verbose){
                         cout << " Angle with next point :: " ;
	                 cout << angle << ", length = " << length;
                         cout << " (x,y)_next = (" << xpeaks[xmap[jj]] << "," << ypeaks[xmap[jj]] <<")" << endl;}
	               if ((length<0.075 )&&(angle < 0.)) { if (verbose) cout << " ----> Negative, increasing j !  (cur j=" << j <<")"<<endl;
			   j++; }
			 //     else { // 
			 //we finish the loop here, because the situation we checked for only occurs if the wrong point along the same
                         // line was choosen 
			 break;//}
			  // check of next unused point
	}// we need this break statement for the case that l == (2-k)
	else break;
	  } // first angle check passed
      } // unused point
    } // while loop

    if (verbose) { if (j<16) cout << " setting point : " << curcorner+l+1 << " (j=" << j << "); x=" << xpeaks[xmap[j]] << ", y=" << ypeaks[xmap[j]]<<endl; }
    if (j>15) j=curcorner+l+1;
  remapped_x[curcorner+l+1]=xpeaks[xmap[j]];
  remapped_y[curcorner+l+1]=ypeaks[xmap[j]];
  flagged[xmap[j]]=1;
  }


  for ( l=0;l<(3-k);l++){
    // find the lowest point in $y$ that hasn't been used ::
    j=0;
    if (l) prevpoint=curcorner+l+(3-k);
    else prevpoint=curcorner;
    while(j<16) { if (flagged[ymap[j]]) j++;        
      else  {
        if (verbose) { cout << "length line segment : " ;
                       length= TMath::Sqrt(TMath::Power(ypeaks[ymap[j]]-remapped_y[prevpoint],2)+
                                           TMath::Power(xpeaks[ymap[j]]-remapped_x[prevpoint],2));
                       cout << length << " ( prevpoint = " << prevpoint << ")";
                       cout <<  "(x,y)_prev = (" << remapped_x[prevpoint] << "," << remapped_y[prevpoint] <<"); (x,y)_cur = (";
                       cout << xpeaks[ymap[j]] << "," << ypeaks[ymap[j]] << ")";
                       cout <<endl;  } 
        if (l!=(2-k)){
	  /* check next unused point */
	  // late night fix: sometimes the next point along $x$ is taken first ( a point with higher $x$ value ) ,
          // that results in an error, which will appear from the angle between the current candidate point and 
	  // the next point to be almost 180 degrees, much larger than 90 which we check for. 
	  jj=j+1;
          while (jj<16) { if (flagged[ymap[jj]]) jj++;
			  else break;}
        	       angle=atan2(ypeaks[ymap[jj]]-ypeaks[ymap[j]],xpeaks[ymap[jj]]-xpeaks[ymap[j]]);
          
		       if (verbose){
                         cout << " Angle with next point :: " ;
	                 cout << angle << " (x,y)_next = (" << xpeaks[ymap[jj]] << "," << ypeaks[ymap[jj]] <<")" << endl;}
	               if ( angle > TMath::Pi()/2. ) j++;
                    }
	// angle calc not needed
 	//     angle=atan2(ypeaks[xmap[j]]-remapped_y[curcorner+l],xpeaks[xmap[j]]-remapped_x[curcorner+l]);
	//  if (angle<0.) j++;
        //  else     
	 break;}
    }
    if (verbose)    cout << " setting point : " << curcorner+l+1+(3-k) << endl;
    remapped_x[curcorner+l+1+(3-k)]=xpeaks[ymap[j]];
    remapped_y[curcorner+l+1+(3-k)]=ypeaks[ymap[j]];
  flagged[ymap[j]]=1;
  }

  if (verbose)  cout << " Corner " << k << ": " << remapped_x[curcorner] << " "<<  remapped_y[curcorner] <<endl;  
  curcorner+=(4-k)*2-1;
 
 } // loop over k

 //remapped_x[SPEAKS-1]=xpeaks[map[SPEAKS-1]];
 // remapped_y[SPEAKS-1]=ypeaks[map[SPEAKS-1]];
 //flagged[map[SPEAKS-1]]=1;


 *sortedxpeaks=remapped_x;
 *sortedypeaks=remapped_y;

 for (i=0;i<SPEAKS;i++){
  peaks_remapped->SetPoint(i,remapped_x[i],remapped_y[i]);}

  return 0;}


Int_t mergegraphs(TGraph *graphs[4], TGraph *merged){

  Double_t *xpeaks;
  xpeaks = new Double_t[64];
  Double_t *ypeaks;
  ypeaks = new Double_t[64];
  Double_t *xps[4];
  Double_t *yps[4]; 
  Int_t i,j;

    for (i=0;i<4;i++){
     xps[i] = graphs[i]->GetX();
     yps[i] = graphs[i]->GetY();
   }
 
    for (i=0;i<4;i++){
      for (j=0;j<16;j++){
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
    //  for (i=0;i<64;i++){
    //    cout << i << " " << xpeaks[i] << " " << ypeaks[i] << " ";
    //    if (!((i+1)%16)) cout << endl;}
  
  
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

// merged->SetPoint(i,xpeaks[i],ypeaks[i]);}
  
  return 0;}
