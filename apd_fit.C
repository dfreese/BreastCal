/*$T indentinput.C GC 1.140 08/21/09 12:28:01 */


/*$6
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */

//#include "coinc_ana.h"
#define FILENAMELENGTH  120
#define MAXFILELENGTH   160
#define BINS_PER_SPAT_PHOTOPEAK 38*5
#define BINS_PER_COM_PHOTOPEAK 38*5
//FIXME :: This should link to a global setting
#define PP_UP_EDGE 2800
#include "apd_fit.h"
//#include "decoder.h"


Float_t			escape[PEAKS][4];
TH1F			*mean, *fwhm, *chndf, *fwhmE;
TH1F			*mean3, *fwhm3, *chndf3, *fwhmE3;

TGraphErrors	*mean2, *fwhm2;
TGraph			*chndf2;
Double_t		means[PEAKS], fwhms[PEAKS], chndfs[PEAKS];
Double_t		meansE[PEAKS], fwhmsE[PEAKS], x[PEAKS], xe[PEAKS];
Int_t			validEflag[PEAKS];

/*
 =======================================================================================================================
    TF1 *ffunc[PEAKS];
 =======================================================================================================================
 */
Double_t Background(Double_t *x, Double_t *par)

/* The background function */
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	Double_t	val = par[2] + par[0] * TMath::Exp(-x[0] * par[1]);
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	return val;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
Double_t Signal(Double_t *x, Double_t *par)

/* The signal function: a gaussian */
{
	/*~~~~~~~~~~~~~~~~*/
	Double_t	arg = 0;
	/*~~~~~~~~~~~~~~~~*/

	if(par[2]) arg = (x[0] - par[1]) / par[2];

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	Double_t	sig = par[0] * TMath::Exp(-0.5 * arg * arg);
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	return sig;
}
 
/*
 =======================================================================================================================
 =======================================================================================================================
 */
Double_t Doublegaus(Double_t *x, Double_t *par)

/* The signal function: a gaussian */
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	Double_t	sig = Signal(x, &par[0]) + Signal(x, &par[3]);
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	return sig;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
Double_t Total(Double_t *x, Double_t *par)

/* Combined background + signal */
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	Double_t	tot = Background(x, par) + Signal(x, &par[3]);
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	return tot;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
Double_t Total2(Double_t *x, Double_t *par)

/* Combined background + signal */
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	Double_t	tot = par[2] + Signal(x, &par[3]);
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	return tot;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
Double_t Total3(Double_t *x, Double_t *par)

/* Combined background + signal */
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	Double_t	tot = Signal(x, &par[0]) + Signal(x, &par[3]);
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	return tot;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
Double_t Total4(Double_t *x, Double_t *par)

/* Combined background + signal */
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	Double_t	tot = par[6] + Signal(x, &par[0]) + Signal(x, &par[3]);
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	return tot;
}



/*
 =======================================================================================================================
 =======================================================================================================================
 */
Double_t Total5(Double_t *x, Double_t *par)

/* The signal function: a gaussian */
{
	/*~~~~~~~~~~~~~~~~*/
	Double_t	arg = 0;
	Double_t	arg2 = 0;
	/*~~~~~~~~~~~~~~~~*/

	if(par[5]) { arg = (x[0] - par[1]) / par[5]; arg2 = (x[0] - par[4]) / par[5];}
                    

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	Double_t	sig = par[0] * TMath::Exp(-0.5 * arg * arg) + par[3] * TMath::Exp(-0.5 * arg2 *arg2 );
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	return sig;
}
 
/*
 =======================================================================================================================
 =======================================================================================================================
 */
TF1 *fitapdspec(TH1F *hist, Float_t xlow, Float_t xhigh, Int_t com, Int_t verbose)
{

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	/* PEAK SEARCH ALGORITHM */
	TSpectrum	*ss = new TSpectrum();
	Int_t		i;
	Int_t		npeaks = 0;
        Int_t hibin=0;
        Int_t max=0;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
        /*~~~~~~~~~~~~~~~~~~~~*/
	/*
	 * In case multiple peaks were found, the heighest peak found should be our
	 * photopeak ;
	 * this is most likely the first entry in array returned by GetPositionX ;
	 * but we check it anyway
	 */
	Double_t	y = 0;
	Int_t		corpeak = 0;
	/*~~~~~~~~~~~~~~~~~~~~*/

	/*
	 * The common spectrum looks different than the spatial spectra, the photopeak is
	 * broader ;
	 * in case of the common, therefore sigma of the peaks can be 4. the photopeak may
	 * be lower ;
	 * than the pedestal peak.
	 */
	i = 0;
	if(com==1) { 
	  if (xhigh > SATURATION) xhigh=SATURATION;
	  if (verbose) { cout << "Global fit :: xlow = " << xlow << ", xhigh = " << xhigh <<", max = " << max << endl;}
            for (i=hist->FindBin(xlow);i<hist->FindBin(xhigh);i++){
               if (hist->GetBinContent(i)>max) { 
		 if (verbose) cout << " i = " << i << "; max = " << max << "; x  = " <<  hist->GetBinLowEdge(i)  << endl;
                  hibin=i;max=hist->GetBinContent(i);}
                 }
	//	npeaks = ss->Search(hist, 4, "", 0.4);
	}
	else {
        if (com==2){ 
                 for (i=hist->FindBin(xlow);i<hist->FindBin(xhigh);i++){ if (hist->GetBinContent(i)>max) {hibin=i;max=hist->GetBinContent(i);}}
           }//comm ==2

        else {
		while((npeaks < 3) && (i < 8)) {
			if(verbose) {cout << "loop " << i << " " << npeaks << endl;}

			/*
			 * npeaks = ss->Search(hist,2,"",0.6-(float )i/10);
			 * // AVDB 6/9/2009 change 2 -->3
			 */
			npeaks = ss->Search(hist, 3, "", 0.9 - (float) i / 10);
			i++;
			if(i > 10) {if(verbose) {cout << " Warning " << npeaks << "found !" << endl;}
				    break;}
		} //while loop

		if(verbose) {
			cout << npeaks << " peaks found " << i << " number of iterations" << endl;
			cout << *ss->GetPositionX() << endl;
		}
	

	
	if(npeaks != 1) {
		for(i = 0; i < npeaks; i++) {
			if(verbose) {cout << "x= " << *(ss->GetPositionX() + i) << " y = " << *(ss->GetPositionY() + i) << endl;}

				/* take largest peak with x position larger than lower fit limit */
			

			/* CHANGE !! */
			if((y < *(ss->GetPositionY() + i)) && (*(ss->GetPositionX() + i) > xlow) && (*(ss->GetPositionX() +i) < xhigh)) {
				corpeak = i;
				y = *(ss->GetPositionY() + i);
			}
		} // for loop

		if(verbose) {
			cout << "The correct peak out of " << npeaks << " peaks is peak number " << corpeak;
			cout << " at position: x = " << *(ss->GetPositionX() + corpeak) << endl;
		}
	




 // FIRST CHECK ::
	if((*(ss->GetPositionX() + corpeak)) < xlow) {
		if(verbose) {cout << " Peak at " << *(ss->GetPositionX() + corpeak) << " is not within the window " << endl;}


// move SIGMA rather than threshold
i=0; npeaks=0; 
Int_t j;
Int_t stop=0;
         while((i < 8)&&(!stop)) {
			
			/*
			 * npeaks = ss->Search(hist,2,"",0.6-(float )i/10);
			 * // AVDB 6/9/2009 change 2 -->3
			 */
			npeaks = ss->Search(hist, 10 - i, "", 0.4);
			if(verbose) {cout << "2nd while loop " << i << " " << npeaks << endl;}
			
		
		if(verbose) {cout << npeaks << " peaks found " << i << " number of iterations" << endl;
			     cout << " peak position :: " << *ss->GetPositionX() << endl;}
	                 i++;
        	if(npeaks != 1) {
	        	for(j = 0; j < npeaks; j++) {
		        	if(verbose) {cout << "x= " << *(ss->GetPositionX() + j) << " y = " << *(ss->GetPositionY() + j) << endl;}
				/* take largest peak with x position larger than lower fit limit */
			/* CHANGE !! */
			if((y < *(ss->GetPositionY() + j)) && (*(ss->GetPositionX() + j) > xlow) && (*(ss->GetPositionX() +j) < xhigh)) {
				corpeak = j;
				y = *(ss->GetPositionY() + j);
	            		}
			} // for loop

		if(verbose) {cout << "The correct peak out of " << npeaks << " peaks is peak number " << corpeak;
			     cout << " at position: x = " << *(ss->GetPositionX() + corpeak) << endl;}
         	} // if npeaks !=1

                else corpeak=0;

              
	if((*(ss->GetPositionX() + corpeak)) < xlow) {
		if(verbose) {cout << "Peak at " << *(ss->GetPositionX() + corpeak) << " is not within the window " << endl;} }
        else { if (verbose) {cout << "Valid peak found at : " << *(ss->GetPositionX()+corpeak) << endl;} stop=1; }
       } // while loop

  if (!(stop)){ 
		y = 0;
		npeaks = ss->Search(hist, 2, "", 0.4);
		for(i = 0; i < npeaks; i++) {
			if(verbose) {cout << "x= " << *(ss->GetPositionX() + i) << " y = " << *(ss->GetPositionY() + i) << endl;}

				/* take largest peak with x position larger than lower fit limit */
			/* CHANGE !! */
			if((y < *(ss->GetPositionY() + i)) && (*(ss->GetPositionX() + i) > xlow) && (*(ss->GetPositionX() +i) < xhigh)) {
				corpeak = i;
				y = *(ss->GetPositionY() + i);
			}
		} // for loop

		if(verbose) {
			cout << "The correct peak out of " << npeaks << " peaks is peak number " << corpeak;
			cout << " at position: x = " << *(ss->GetPositionX() + corpeak) << endl;
		}
// SECOND CHECK
		if((*(ss->GetPositionX() + corpeak)) < xlow) {
			if(verbose) {cout << "Peak at " << *(ss->GetPositionX() + corpeak) << " is STILL not within the window " << endl;}
                       
               /* Lower "SIGMA" of tspectrum */
            	npeaks = ss->Search(hist, 2, "", 0.3);
		for(i = 0; i < npeaks; i++) {
		        if(verbose) {cout << "x= " << *(ss->GetPositionX() + i) << " y = " << *(ss->GetPositionY() + i) << endl;}
				/* take largest peak with x position larger than lower fit limit */
			/* CHANGE !! */
			if((y < *(ss->GetPositionY() + i)) && (*(ss->GetPositionX() + i) > xlow) && (*(ss->GetPositionX() +i) < xhigh)) {
				corpeak = i;
				y = *(ss->GetPositionY() + i);
			}
		} // for loop

		if(verbose) {
			cout << "The correct peak out of " << npeaks << " peaks is peak number " << corpeak;
			cout << " at position: x = " << *(ss->GetPositionX() + corpeak) << endl;
		}

// THIRD CHECK
		if((*(ss->GetPositionX() + corpeak)) < xlow) {
			if(verbose) {
				cout << "Peak at " << *(ss->GetPositionX() + corpeak) << " is STILL not within the window " << endl;
			}

     /* Lower "SIGMA, threshold" of tspectrum */
 

		npeaks = ss->Search(hist, 2, "", 0.3);
		for(i = 0; i < npeaks; i++) {
			if(verbose) {cout << "x= " << *(ss->GetPositionX() + i) << " y = " << *(ss->GetPositionY() + i) << endl;}
				/* take largest peak with x position larger than lower fit limit */
			
			/* CHANGE !! */
			if((y < *(ss->GetPositionY() + i)) && (*(ss->GetPositionX() + i) > xlow) && (*(ss->GetPositionX() +i) < xhigh)) {
				corpeak = i;
				y = *(ss->GetPositionY() + i);
			}
		} // for loop

		if(verbose) {
			cout << "The correct peak out of " << npeaks << " peaks is peak number " << corpeak;
			cout << " at position: x = " << *(ss->GetPositionX() + corpeak) << endl;
		}

// FOURTH CHECK
		if((*(ss->GetPositionX() + corpeak)) < xlow) {
			if(verbose) {
				cout << "Peak at " << *(ss->GetPositionX() + corpeak) << " is STILL not within the window " << endl;
			}
                  

 // Giving up, taking the highest peak 

			y = 0;
			for(i = 0; i < npeaks; i++) {
				if(y < *(ss->GetPositionX() + i)) {
					corpeak = i;
					y = *(ss->GetPositionX() + i);
				}
			}	/* for loop */

			if(verbose) {
				cout << "The correct peak out of " << npeaks << " peaks is peak number " << corpeak;
				cout << " at position: x = " << *(ss->GetPositionX() + corpeak) << endl;
			}
		} // giving up
	 } //npeaks = ss->Search(hist, 2, "", 0.3);
   } //npeaks = ss->Search(hist, 2, "", 0.3);

} // 4th check
} // 3rd check
} // 2nd check
} // first check if the location of peak is okay.
} // else corresponding to  if (com)


	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	/* GET READY TO FIT :: FIT GAUSSIAN TO ESTIMATED PEAK */
	Int_t		npar = 6;
	Double_t	params[6] = { 500, 0.005, 5, 80, 9, 0.5 };
        TF1 *fitfunc;
        if (com==1){ fitfunc = new TF1("fitfunc", Total5, 0, 20000, npar);}
        else   fitfunc = new TF1("fitfunc", Total, 0, 2000, npar);
	TF1			*signal = new TF1("signal", Signal, 0, 2000, 3);
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	fitfunc->SetLineColor(kRed);
        if (com){
         if (verbose) { cout << " We will use peak position at x = " << hist->GetBinLowEdge(hibin) << endl;}

         params[3] = (Double_t) (max);
	 params[4] = (Double_t) (hist->GetBinLowEdge(hibin));
         params[5] =  (xhigh-xlow ) /4 ;
              }
        else {
	params[3] = (Double_t) * (ss->GetPositionY() + corpeak);
	params[4] = (Double_t) * (ss->GetPositionX() + corpeak);
        params[5] =  (xhigh-xlow ) /4 ;}

	if(verbose) {
		cout << "Params before gauss fit: ";
		for(i = 0; i < 3; i++) cout << params[i + 3] << " ";
		cout << endl;
	}

	/*
	 * fitfunc->SetParameters(&par);
	 * ;
	 * fitfunc->SetParameter(0,40);
	 * fitfunc->SetParameter(1,hist->GetMean());
	 * fitfunc->SetParameter(2,hist->GetRMS());
	 * fitfunc->SetParameter(3,0);
	 * fitfunc->SetParameter(4,-3);
	 */
	signal->SetParLimits(2, 0, 1000);
        signal->SetParLimits(0,0,1e6);
	/* 5-21 changed this from 1500 to 2000 */
	signal->SetParLimits(1, -10, PP_UP_EDGE);
	signal->SetParameters(&params[3]);

	/*~~~~~~~~~~~~*/
	Int_t	fitstat;
	/*~~~~~~~~~~~~*/

        if (com==1) {
              signal->SetParLimits(0,0,1.5*max); 
            if(verbose) {
                cout << "Fitting signal between " << params[4]-0.5*params[5]  << " and " << params[4]+params[5] << endl;
                

        fitstat = hist->Fit("signal", "", "", params[4]-0.5*params[5] , params[4] + params[5]);
//	fitstat = hist->Fit("signal", "", "", xlow, xhigh);

//         fitstat = hist->Fit("signal", "", "", xlow, xhigh);
//          hist->Fit("gaus","","",xlow,xhigh);
	cout << "Fitstat = " << fitstat << endl ; }
	else
 		fitstat = hist->Fit("signal", "Q", "", params[4]-0.5*params[5] , params[4] + params[5]);

      if(verbose) {
		cout << "Params after gauss fit: ";
		for(i = 0; i < 3; i++) cout << signal->GetParameter(i) << " ";
		cout << endl;
	}



        if (TMath::Abs((signal->GetParameter(1) - params[4])/params[4]) > 0.1) {
           if (verbose) { cout << "fit parameter difference too large :: " << (signal->GetParameter(1)-params[4])/params[4] << endl;
              cout << "Fitting signal between " << params[4]-0.2*params[5]  << " and " << params[4]+2*params[5] << endl;
                

        fitstat = hist->Fit("signal", "", "", params[4]-0.2*params[5] , params[4] + 2*params[5]);
//	fitstat = hist->Fit("signal", "", "", xlow, xhigh);

//         fitstat = hist->Fit("signal", "", "", xlow, xhigh);
//          hist->Fit("gaus","","",xlow,xhigh);
	cout << "Fitstat = " << fitstat << endl;}
	else
 		fitstat = hist->Fit("signal", "Q", "", params[4]-0.1*params[5] , params[4] + 2*params[5]); 
	}
   signal->GetParameters(&params[3]);
   //   params[2]=hist->GetBinContent(hist->FindBin(params[4]+5*params[5]));
   params[2]=0.9*params[5];
   params[1]=params[4]*450/511;
   if(verbose) cout << " initial value params[2] :: " <<hist->GetBinContent(hist->FindBin(params[4]+5*params[5])) << endl; 

     //    fitfunc->SetParLimits(2, -1, params[5]*10);
     fitfunc->FixParameter(2,0);
    fitfunc->SetParLimits(1, params[4]*450/511-2*params[5], params[4]);
    fitfunc->SetParameters(&params[0]);

   if (verbose) { for (Int_t kk=0;kk<6;kk++){  cout << " initial value params[" <<kk<<"] :: " << params[kk] << " "; } cout << endl;}
   if (verbose) { cout << "Lets do the fit now between " << params[4]-2*params[5] << " and " <<  params[4] + 1.75*params[5] <<endl;
    fitstat = hist->Fit("fitfunc", "", "", params[4]-2.2*params[5] , params[4] + 1.75*params[5]);
    fitstat = hist->Fit("fitfunc", "", "", params[4]-2.2*params[5] , params[4] + 1.75*params[5]);}
   else {
    fitstat = hist->Fit("fitfunc", "Q", "", params[4]-2.2*params[5] , params[4] + 1.75*params[5]);
    fitstat = hist->Fit("fitfunc", "Q", "", params[4]-2.2*params[5] , params[4] + 1.75*params[5]);
   }    
   

          return fitfunc;
        }  // if com 
        else {
	if(verbose) {
                cout << "Fitting signal between " << params[4] - params[5] << " and " << params[4]+params[5] << endl;
                cout << "Could we use xlow = " << xlow << " and xhigh = " << xhigh << " ? ----> NO !" << endl;

        fitstat = hist->Fit("signal", "", "", params[4] - params[5], params[4] + params[5]);
//	fitstat = hist->Fit("signal", "", "", xlow, xhigh);

//         fitstat = hist->Fit("signal", "", "", xlow, xhigh);
//          hist->Fit("gaus","","",xlow,xhigh);
                 cout << "Fitstat = " << fitstat << endl;}
	else
 		fitstat = hist->Fit("signal", "Q", "", params[4] - params[5], params[4] + params[5]);
//	fitstat = hist->Fit("signal", "Q", "", xlow,xhigh ); 

	/*
	 * Try this as of 6-09-2009 ;
	 * if (verbose) fitstat = hist->Fit("signal","","",params[4]-80,params[4]+100);
	 * else fitstat = hist->Fit("signal","Q","",params[4]-80,params[4]+100);
	 */

        }


	if ((fitstat)|| (TMath::Abs( (params[4]-signal->GetParameter(1) )/params[4]) > 0.25)) {

		/* bad fit ... try again */
        	signal->SetParameters(&params[3]);
		if(verbose) {  
		  cout << "params[4] = " << params[4] << " params[5] = " <<params[5]<<endl;
			cout << " !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! " << endl;
			cout << "                  First fit returned " << fitstat << "  ..... REFITTING !" << endl;
			cout << " !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! " << endl;
			fitstat = hist->Fit("signal", "", "", params[4] - .125*params[5], params[4] + .25*params[5]);
		}
		else
			fitstat = hist->Fit("signal", "Q", "", params[4] - .125*params[5], params[4] + .25*params[5]);
	}

	signal->GetParameters(&params[3]);

	if(verbose) {
		cout << "Params after gauss fit: ";
		for(i = 0; i < 3; i++) cout << params[i + 3] << " ";
		cout << endl;
	}


       
         

        if (com==1) { xlow = params[4] - 2 * params[5]; xhigh = params[4] + 2.5 * params[5];}
	else { xlow = params[4] - 3 * params[5]; 
	  if ( params[5]*2.35/params[4]<.25) xhigh = params[4] + 3 * params[5];
          else  xhigh = params[4] + 1.5 * params[5];
     }


	/*
	 * These are values for the RMD ana :: where we fit with 2 Gauss Functions
         */
         if (com==1){ 
             params[0]=.3*params[3];
	     params[1]=.8*params[4];
	     params[2]=0.5; 
	     //             fitfunc->SetParLimits(1,params[4]/1.05-2*params[5],params[4]/1.05+params[5] );
	     //             fitfunc->SetParLimits(2, 0, 10);
	                  }
	params[4] = 1.05 * params[4];

	/*
	 * params[6]=20;
	 */
	if(verbose) {
		cout << " Initial fit parameters : " << endl;
		for(i = 0; i < npar; i++) {
			cout << i << " :: " << params[i] << endl;
		}

		/*
		 * cout << params[4]/1.05-params[5]<< " < params[4] < " <<
		 * params[4]/1.05+params[5] <<endl;
		 * * cout << params[4]/1.05-2*params[5]<< " < params[1] < " <<
		 * params[4]/1.05+params[5] <<endl;
		 */
	}			/* verbose */

	fitfunc->SetParameters(params);

	/*
	 * fitfunc->SetParLimits(4,params[4]/1.05-params[5],params[4]/1.05+params[5]);
	 * fitfunc->SetParLimits(3,0,50000);
	 * fitfunc->SetParLimits(5,0,300);
	 * fitfunc->SetParLimits(0,0,5000);
	 * // 5-21 changed this from 1500 to the current expression
	 * fitfunc->SetParLimits(1,params[4]/1.05-2*params[5],params[4]/1.05+params[5] );
	 * fitfunc->SetParLimits(2,0,300);
	 */

	fitfunc->SetParLimits(0, 0, 5000);
        fitfunc->SetParLimits(1, 0,10000);
	fitfunc->SetParLimits(3, 0,50000);
	fitfunc->SetParLimits(4, 0, 2200);
	fitfunc->SetParLimits(5, 0,  150);

	/* ACTUAL FIT */
	if(verbose) {
		cout << "Fitting between " << xlow << " and  " << xhigh << endl;
	}

        Int_t retfit=4;
         i=0;
        while ((retfit)&&(i<5)){
      	if(verbose) {
	  retfit =	hist->Fit("fitfunc", "", "", xlow, xhigh);
          cout << "Return code fitting with fitfunc :: " << retfit <<endl; 
               }
	else
          retfit = hist->Fit("fitfunc", "Q", "", xlow, xhigh);
          
         if (retfit){
          xlow -=  params[5];
           if (verbose)   cerr << " ......... Refitting ..........  i = " << i <<endl; }

           if(verbose) {
            cout << "Retfit .... not 0 .. Refitting " <<endl;
 	  retfit =	hist->Fit("fitfunc", "", "", xlow, xhigh);
           cout << "Return code fitting with fitfunc :: " << retfit <<endl; 
                }
 	else
           retfit = hist->Fit("fitfunc", "Q", "", xlow, xhigh);
	   if (i>2) xlow=1.1*xlow;
          i++;
         }
        

        if (retfit){
          xhigh +=  params[5];
 	if(verbose) {
          cout << "Last Attempt .. " << endl;
//  YES TWO TIMES !!!
	  retfit =	hist->Fit("fitfunc", "", "", xlow, xhigh);
 	  retfit =	hist->Fit("fitfunc", "", "", xlow, xhigh);
          cout << "Return code fitting with fitfunc :: " << retfit <<endl; 
               }
	else {
          retfit = hist->Fit("fitfunc", "Q", "", xlow, xhigh); 
         retfit = hist->Fit("fitfunc", "Q", "", xlow, xhigh);
          }
}

	fitfunc->SetRange(xlow, xhigh);

	/*~~~~~~~~~~~~~~~~~~~~~~*/
	Double_t	chi2_s, ndf_s;
	Double_t	chi2_t, ndf_t;
	/*~~~~~~~~~~~~~~~~~~~~~~*/

	chi2_s = signal->GetChisquare();
	ndf_s = signal->GetNDF();
	chi2_t = fitfunc->GetChisquare();
	ndf_t = fitfunc->GetNDF();

	if(verbose) {
		cout << "NDF_T = " << ndf_t << endl;
	}

	if(ndf_t > 1e-4) {
		if((chi2_t == 0) || ((chi2_t / ndf_t) > 10)) {
			if(verbose) {
				printf("\n>>>>>>>>>>>>>>>>>>>>>>>>    BAD FIT  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
				printf(">> Fit to PE peak :          chi2 = %3.3f         ndf = % .1f    <<\n", chi2_s, ndf_s);
				printf(">> Fit to entire specturm:   chi2 = % .3f         ndf = % .1f    <<\n", chi2_t, ndf_t);
			}	/* verbose */
		}
	}
	else {
		if(verbose) {
			printf("\n>>>>>>>>>>>>>>>>>>>>>>>>    BAD FIT  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
			printf(">> Fit to PE peak :          chi2 = %3.3f         ndf = % .1f    <<\n", chi2_s, ndf_s);
			printf(">> Fit to entire specturm:   chi2 = % .3f         ndf = % .1f    <<\n", chi2_t, ndf_t);
		}		/* verbose */
	}

	return fitfunc;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
Int_t fitall
(
	TH1F		*hi[PEAKS],
	TF1			*ffunc[PEAKS],
	Double_t	*values,
	Double_t	pixvals[PEAKS][9],
	Float_t		lowfit,
	Float_t		hifit,
	TCanvas		*c1,
	Int_t		verbose
)
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	Char_t	fnam[60];
	Int_t	i, validEpeaks;
	Float_t this_lowfit, this_hifit;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	mean = (TH1F *) gROOT->FindObject("mean");
	if(!(mean)) {
		mean = new TH1F("mean", "Mean of Gaussian fit", PEAKS, 1, PEAKS);
	}
	else
		mean->Reset();

	fwhm = (TH1F *) gROOT->FindObject("fwhm");
	if(!(fwhm)) {
		fwhm = new TH1F("fwhm", "FWHM of Gaussian fit", PEAKS, 1, PEAKS);
	}
	else
		fwhm->Reset();

	chndf = (TH1F *) gROOT->FindObject("chndf");
	if(!(chndf)) {
		chndf = new TH1F("chndf", "CHI2/NDF of Gaussian fit", PEAKS, 1, PEAKS);
	}
	else
		chndf->Reset();

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	Double_t	mean_av, mean_av_e, fwhm_av, fwhm_av_e;
	Int_t		badchi;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	mean_av = 0;
	mean_av_e = 0;
	fwhm_av = 0;
	fwhm_av_e = 0;

	validEpeaks = 0;

	for(i = 0; i < PEAKS; i++) {
		this_lowfit = lowfit;
		this_hifit = hifit;
		validEflag[i] = 0;
		if((i == 0) || (i == 7) || (i == 63) || (i == 56)) {

			/*
			 * this_lowfit=lowfit-.2*(hifit-lowfit);
			 * --> CHANGE TO :: ON 6/9/2009
			 */
			this_lowfit = lowfit - .2 * (hifit - lowfit);
			this_hifit = hifit * .95;
		}
		else {

			/*
			 * this_lowfit=E_low-.3*(E_up-E_low);
			 */
			this_hifit = hifit + .3 * (hifit - lowfit);

			/*
			 * this_lowfit=E_low+.05*(E_up-E_low);
			 */
		}

		/*
		 * CHANGE ;
		 * lowfit=5.5;
		 * hifit=9.5;
		 */
		if(verbose) {
			printf("\n");
			printf("#####################################################################\n");
			printf("#                   Fitting Histogram %2d                            #\n", i);
			printf("#                   Fitting between %5.2f and %5.2f                 #\n", this_lowfit, this_hifit);
			printf("#####################################################################\n");
		}

		ffunc[i] = fitapdspec(hi[i], this_lowfit, this_hifit, 2, verbose);

#define LEASTNDF	3
		sprintf(fnam, "ffunc[%d]", i);
		ffunc[i]->SetName(fnam);
		mean->SetBinContent(i + 1, ffunc[i]->GetParameter(4));
                if (TMath::IsNaN(ffunc[i]->GetParError(4))) mean->SetBinError(i+1,0.1*ffunc[i]->GetParameter(4));
                else mean->SetBinError(i + 1, ffunc[i]->GetParError(4));
		fwhm->SetBinContent(i + 1, 2.345 * ffunc[i]->GetParameter(5));
                if (TMath::IsNaN(ffunc[i]->GetParError(5))) mean->SetBinError(i+1,0.1*ffunc[i]->GetParameter(5));
                else fwhm->SetBinError(i + 1, 2.345 * ffunc[i]->GetParError(5));
		if((ffunc[i]->GetNDF() > LEASTNDF)) {
			chndf->SetBinContent(i + 1, ffunc[i]->GetChisquare() / ffunc[i]->GetNDF());
		}
		else
			chndf->SetBinContent(i + 1, -1);

		if(verbose) {
			cout << " <<<<<<<<<<< " << chndf->GetBinContent(i + 1) << "<<<<<<<<<<<<" << endl;
		}

		if((0 < chndf->GetBinContent(i + 1)) && (chndf->GetBinContent(i + 1) < 10)) {
			if(verbose) {
				cout << "Valid Fit !!!!" << endl;
			}

			validEpeaks++;
			validEflag[i] = 1;
			badchi = 0;
		}
		else
			badchi = 1;

		/* If we got bad fit parameters, we rebin and try to fit again. */
		if
		(
			(badchi)
		||	(2.345 * ffunc[i]->GetParameter(5) / ffunc[i]->GetParameter(4) > .3)
		||	(ffunc[i]->GetParError(4) / ffunc[i]->GetParameter(4) > .5)
		||	(TMath::IsNaN(ffunc[i]->GetParError(4)))
		//||	(ffunc[i]->GetProb() < 1e-21)
		) {
			if(verbose) { cout << "Something's wrong with our Fit  NDF ::" << ffunc[i]->GetNDF() << endl;
                                      cout << " 2.345 * ffunc[i]->GetParameter(5) / ffunc[i]->GetParameter(4) :: "  << 2.345 * ffunc[i]->GetParameter(5) / ffunc[i]->GetParameter(4) << endl;
                                      cout << " (ffunc[i]->GetParError(4) / ffunc[i]->GetParameter(4) > .5) ::" << (ffunc[i]->GetParError(4) / ffunc[i]->GetParameter(4) > .5) << endl;
                                      cout << " ffunc[i]->GetProb() : " << ffunc[i]->GetProb() << endl;
                               }
			if(validEflag[i]) {
				validEpeaks--;
				validEflag[i] = 0;
			}		/* if we are here, it was not a valid fit, despite a good chi2 ! */

			if(ffunc[i]->GetNDF() > 11) {

				/*
				 * hi[i]->Rebin(2);
				 */
				if(verbose) cout << "Rebinning + Refitting the APD spectrum " << endl;
				ffunc[i] = fitapdspec(hi[i], this_lowfit, this_hifit, 0, verbose);
				ffunc[i]->SetName(fnam);
				mean->SetBinContent(i + 1, ffunc[i]->GetParameter(4));
				if (TMath::IsNaN(ffunc[i]->GetParError(4))) mean->SetBinError(i+1,0.1*ffunc[i]->GetParameter(4));
		                else mean->SetBinError(i + 1, ffunc[i]->GetParError(4));
				fwhm->SetBinContent(i + 1, 2.345 * ffunc[i]->GetParameter(5));
		                if (TMath::IsNaN(ffunc[i]->GetParError(5))) mean->SetBinError(i+1,0.1*ffunc[i]->GetParameter(5));
                		else fwhm->SetBinError(i + 1, 2.345 * ffunc[i]->GetParError(5));
				
				if((ffunc[i]->GetNDF() > LEASTNDF)) {
					chndf->SetBinContent(i + 1, ffunc[i]->GetChisquare() / ffunc[i]->GetNDF());
				}
				else
					chndf->SetBinContent(i + 1, -1);
				if(verbose) {
					cout << " <<<<<<<<<<< REFITTED    <<<<<< " << chndf->GetBinContent(i + 1) << "<<<<<<<<<<<<" << endl;
				}

				/* Judge whether the refitted fit is a good one */
				if((ffunc[i]->GetNDF() > LEASTNDF)) {
					chndf->SetBinContent(i + 1, ffunc[i]->GetChisquare() / ffunc[i]->GetNDF());
					if ( ((0 < chndf->GetBinContent(i + 1)) && (chndf->GetBinContent(i + 1) < 10)) ) {
                                           if ((2.345 * ffunc[i]->GetParameter(5) / ffunc[i]->GetParameter(4) > .3)      ||
					       (ffunc[i]->GetParError(4) / ffunc[i]->GetParameter(4) > .5)               ||
					       (TMath::IsNaN(ffunc[i]->GetParError(4)))	
                                         //	||	(ffunc[i]->GetProb() < 1e-21)
                                          ) {validEflag[i] = 0; if(verbose) cout << "Invalid Fit !!!!" << endl;	}
                                           else {   
                                               if(verbose) { cout << "Valid Fit !!!!" << endl;
						}

						validEpeaks++;
						validEflag[i] = 1;
					}
					} else
						validEflag[i] = 0;
				}	/* if 0> chi/ndf or chi/ndf > 10 */
				else {
					chndf->SetBinContent(i + 1, -1);
					validEflag[i] = 0;
				}	/* if NDF is too little */
			}		/* Rebin+Refit attempt */
			else {
				validEflag[i] = 0;
			}		/* no need to rebin if our NDF is low ! */
		}

		if(validEflag[i]) {
			mean_av += ffunc[i]->GetParameter(4);
			fwhm_av += 2.345 * ffunc[i]->GetParameter(5);
		}

		means[i] = ffunc[i]->GetParameter(4);
		meansE[i] = ffunc[i]->GetParError(4);
		fwhms[i] = 2.345 * ffunc[i]->GetParameter(5);
		fwhmsE[i] = 2.345 * ffunc[i]->GetParError(5);
		if(ffunc[i]->GetNDF()) {
			chndfs[i] = ffunc[i]->GetChisquare() / ffunc[i]->GetNDF();
		}
		else
			chndfs[i] = -1;
		x[i] = i;
		xe[i] = 0;
		pixvals[i][0] = means[i];
                if ( TMath::IsNaN(meansE[i])) { cout << "NAN ::: pixel :: " << i << endl;
                                                    cout << "value = " << pixvals[i][1] << endl ;
                                                    meansE[i]=0.1*means[i];}
                pixvals[i][1] = meansE[i];
		pixvals[i][2] = fwhms[i];
                if ( TMath::IsNaN(fwhmsE[i])) { cout << "NAN ::: pixel :: " << i << endl;
                                                    cout << "value = " << pixvals[i][3] << endl ;
                                                    fwhmsE[i]=0.1*fwhmsE[i];}
		pixvals[i][3] = fwhmsE[i];
		pixvals[i][4] = chndfs[i];
		pixvals[i][5] = ffunc[i]->GetParameter(1);
		pixvals[i][6] = ffunc[i]->GetParError(1);
		pixvals[i][7] = 2.345 * ffunc[i]->GetParameter(2);
		pixvals[i][8] = 2.345 * ffunc[i]->GetParError(2);
		escape[i][0] = ffunc[i]->GetParameter(1);
		escape[i][1] = ffunc[i]->GetParError(1);
		escape[i][2] = 2.345 * ffunc[i]->GetParameter(2);
		escape[i][3] = 2.345 * ffunc[i]->GetParError(2);

		if(verbose) {
			cout <<
				"E_res:: " <<
				2.345 *
				ffunc[i]->GetParameter(5) /
				ffunc[i]->GetParameter(4) <<
				"  CUR AV ::" <<
				mean_av <<
				"  Valid Fits :: " <<
				validEpeaks <<
				endl;
		}
	}

	mean2 = new TGraphErrors(PEAKS, x, means, xe, meansE);
	mean2->SetName("mean2");
	fwhm2 = new TGraphErrors(PEAKS, x, fwhms, xe, fwhmsE);
	fwhm2->SetName("fwhm2");
	chndf2 = new TGraphErrors(PEAKS, x, chndfs);
	chndf2->SetName("chndf2");

	/*~~~~~~~~~~~~~~~~~*/
	Double_t	min, max;
	/*~~~~~~~~~~~~~~~~~*/

	min = mean->GetMinimum();
	max = mean->GetMaximum();

	mean3 = (TH1F *) gROOT->FindObject("mean3");
	if(!mean3)
		mean3 = new TH1F("mean3", "histogram of MEAN", 100, min - (max - min), max + (max - min));
	else
		mean3->Reset();

	min = fwhm->GetMinimum();
	max = fwhm->GetMaximum();

	fwhm3 = (TH1F *) gROOT->FindObject("fwhm3");
	if(!fwhm3)
		fwhm3 = new TH1F("fwhm3", "histogram of FWHM", 100, min - (max - min), max + (max - min));
	else
		fwhm3->Reset();

	min = chndf->GetMinimum();
	max = chndf->GetMaximum();

	chndf3 = (TH1F *) gROOT->FindObject("chndf3");
	if(!chndf3)
		chndf3 = new TH1F("chndf3", "histogram of CHI2", 100, min - (max - min), max + (max - min));
	else
		chndf3->Reset();

	fwhmE = (TH1F *) fwhm->Clone();
	fwhmE->SetName("fwhmE");
	fwhmE->Divide(mean);
	min = fwhmE->GetMinimum();
	max = fwhmE->GetMaximum();

	fwhmE3 = (TH1F *) gROOT->FindObject("fwhmE3");
	if(!fwhmE3)
		fwhmE3 = new TH1F("fwhmE3", "histogram of FWHM/E", 100, min - (max - min), max + (max - min));
	else
		fwhmE3->Reset();

	for(Int_t i = 1; i <= fwhmE->GetNbinsX(); i++) {
		fwhmE3->Fill(fwhmE->GetBinContent(i));
	}

	if(verbose) {
		cout << "FINAL ::::: CUR AV ::" << mean_av << endl;
	}

	mean_av /= validEpeaks;
	fwhm_av /= validEpeaks;
	if(verbose) {
		cout << "FINAL ::::: CUR AV ::" << mean_av << endl;
	}

	for(i = 0; i < PEAKS; i++) {
		if(validEflag[i]) {
			mean_av_e += TMath::Power((mean_av - means[i]), 2);
			fwhm_av_e += TMath::Power((fwhm_av - fwhms[i]), 2);
		}

		mean3->Fill(means[i]);
		fwhm3->Fill(fwhms[i]);
		chndf3->Fill(chndfs[i]);
	}

	mean_av_e = TMath::Sqrt(mean_av_e / validEpeaks);
	fwhm_av_e = TMath::Sqrt(fwhm_av_e / validEpeaks);

	if(verbose) {
		printf("Mean = %.3f RMS: %.3f\n", mean_av, mean_av_e);
		printf("FWHM = %.3f RMS: %.3f\n", fwhm_av, fwhm_av_e);
		printf("nr of valid Epeaks : %d\n", validEpeaks);
	}

	values[0] = mean_av;
	values[1] = mean_av_e;
	values[2] = fwhm_av;
	values[3] = fwhm_av_e;

	/* mean2 = new TH1F("mean2","Mean" */
	c1->Clear();
	c1->Divide(1, 3);
	c1->cd(1);
	mean->Draw("E");
	c1->cd(2);
	fwhm->Draw("E");
	c1->cd(3);
	chndf->Draw("p");

        cout << " Fitting of all 64 peaks done; number of valid fits .......... " << validEpeaks <<endl;

	return 0;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
Int_t writ(TH1F *hi[PEAKS], TF1 *ff[PEAKS], TCanvas *ccc, Char_t filename[MAXFILELENGTH], Int_t drawfunc)
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	Int_t	k;
	Char_t	filenameo[MAXFILELENGTH+1], filenamec[MAXFILELENGTH+1];
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

         // cout << "filename = " << filename << endl;

	strcpy(filenameo, filename);
	strcpy(filenamec, filename);
	strcat(filenameo, "(");
	strcat(filenamec, ")");

	/*
	  cout << "in : " << filenameo<<endl;
	  * TCanvas *ccc = new TCanvas("ccc","Energy Spectra",10,10,1000,900);
	 */
	ccc->Clear();
	ccc->Divide(2, 4);

	for(k = 1; k < PEAKS + 1; k++) {
		if(k % 8)
			ccc->cd(k % 8);
		else
			ccc->cd(8);

		/*
		 * cout << k << " " << k%8 <<endl;
		 */
		hi[k - 1]->Draw("E");

		/*
		 * cout << ff[k-1]->GetName() <<endl;
		 */
		if(drawfunc) ff[k - 1]->Draw("same");
		if(!(k % 8)) {

			/*
			 * cout << k << endl;
			 */
			if(k == 8) {
				ccc->Print(filenameo);
			}
			else {
				if(k == PEAKS)
					ccc->Print(filenamec);
				else {
					ccc->Print(filename);
				}
			}

			/*
			 * if (k!=PEAKS){ ;
			 * cout << "Clearing and Dividing " <<endl;
			 */
			ccc->Clear();
			ccc->Divide(2, 4);

			/* } */
		}
	}

	return 0;
}
