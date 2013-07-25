/*$T indentinput.C GC 1.140 08/21/09 12:28:45 */


/*$6
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */


#define INFO

#include "Apd_Fit.h"
#include "Apd_Peaktovalley.h"
#include "libInit_avdb.h"
#include "myrootlib.h"

//#include "apd_ana_v2.h"

/*
 * TH1F *xpos[PEAKS];
 * ;
 * TH1F *ypos[PEAKS];
 */
/*
TF1	*posgausfits[PEAKS];
TH1F	*xsums[8];
TH1F    *tacs2[64];
*/
/*
 =======================================================================================================================
 =======================================================================================================================
 */

Float_t *te(TNtuple *data)
{

	/*~~~~~~~~*/
	/* ,Float_t *xp){ */
	Float_t *tt;
	/*~~~~~~~~*/

	tt = new Float_t[3];
	tt[1] = 12;
	tt[0] = 9;
	tt[2] = 123;
	data->Draw("E");

	/*
	 * cout << xp[3] <<endl;
	 * xpos[4]->Draw();
	 */
	return tt;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */



Int_t peaktovalley_refill
(
	Double_t	values[PEAKS][9],
        Double_t	com_values[PEAKS][9],
	TNtuple		*data,
	Double_t	*xpeak,
	Double_t	*ypeak,
	TH1F		*xpos[PEAKS],
	TH1F		*ypos[PEAKS],
	Int_t		algo,
	Float_t		pmt_low,
	Float_t		pmt_up,
	TH1F		*tacs[64],
        TH1F            *spat_glob,
        TH1F            *com_glob,
	Int_t		verbose = 0,
        Int_t           dotac = 0
)
{
	/*~~~~~~~~~*/
	Int_t	i, k;
        Char_t tmpstring[80];
	/*~~~~~~~~~*/

	/* Clear the existing histograms */
	for(i = 0; i < PEAKS; i++) {
		xpos[i]->Reset();
		ypos[i]->Reset();
                if (dotac){
                tacs[i]->Reset();
                sprintf(tmpstring,"tacs2[%d]",i); 
		//                tacs2[i] = new TH1F(tmpstring,tmpstring,200,90,200); //IMPORTANT
		//                tacs2[i]->SetMarkerColor(kBlue);}
		}
	}

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	Float_t a, b, c, d, x, y, E, com, pmt, tac;
	Float_t x2corr, y2corr, xdata, ydata;
        Double_t av_pp_com=0;
        Double_t av_pp_spat=0;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

        for(i=0;i<64;i++){
          av_pp_com+=com_values[i][0];
          av_pp_spat+=values[i][0];
           }

          av_pp_com/=64;
          av_pp_spat/=64;  

// 	data->Draw("e1");
	data->SetBranchAddress("x", &x);
	data->SetBranchAddress("y", &y);
	data->SetBranchAddress("E", &E);
	data->SetBranchAddress("a1", &a);
	data->SetBranchAddress("a2", &b);
	data->SetBranchAddress("a3", &c);
	data->SetBranchAddress("a4", &d);
	data->SetBranchAddress("com", &com);
        if (dotac){
	data->SetBranchAddress("pmt", &pmt);
	data->SetBranchAddress("tac", &tac);
	}
	/*~~~~~~~~~~~~~~~~~~*/
	Double_t	min, dist;
	Int_t		histnr;
	/*~~~~~~~~~~~~~~~~~~*/

	min = 100000;
	histnr = 9999;

#ifdef DEG
 Int_t counter=0;
#endif
	cout << "Looping over NTuple entries ";
//       cout<< " << xpeak[0] = " << xpeak[0] <<" .. ypeak[0] = " << ypeak[0] <<endl;
	/* loop over entries */
 //  cout << "pmt_low = " <<pmt_low << " pmt_high = " << pmt_up <<endl;
	for(i = 0; i < data->GetEntries(); i++) {
		data->GetEntry(i);
		if(algo == CHIN) {
			if((c + a) && (d + b) && (a < 9.95) && (b < 9.95) && (c < 9.95) && (d < 9.95)) {
				x2corr = (d - b) / (d + b);
				y2corr = (a - c) / (a + c);
				ydata = x2corr * cos(PI / 4) - y2corr * sin(PI / 4);
				xdata = y2corr * cos(PI / 4) + x2corr * sin(PI / 4);
			}
			else
				continue;
		}		/* algo = chin */
		else {
			xdata = x;
			ydata = y;
		}

		/*
		 * cout << nr << " = nr " <<endl;
		 * ;
		 * if (ROTATE_HIS[nr]){ tmpxy=ydata;
		 * ydata=xdata;
		 * xdata=tmpxy;
		 * }
		 */
		for(k = 0; k < PEAKS; k++) {
 			dist = TMath::Power(( (Float_t) xpeak[k] - xdata),2)  + TMath::Power( ( (Float_t) ypeak[k] - ydata),2);
                        if(dist < min) {
				histnr = k;
				min = dist;
			}
		}		/* for loop */

		if(histnr != 9999) {
		  if (dotac){
                if((pmt > pmt_low) && (pmt < pmt_up)) {
					spat_glob->Fill(E*av_pp_spat/values[histnr][0]);
                                     //   if (( E*av_pp_spat/values[histnr][0] ) < 0.5 ) cout << "histnr = " << histnr << " E : " << E*av_pp_spat/values[histnr][0] << endl;
                                        com_glob->Fill(com*av_pp_com/com_values[histnr][0]);
                            }
		  }
                  else {
					spat_glob->Fill(E*av_pp_spat/values[histnr][0]);
                                        com_glob->Fill(-1*com*av_pp_com/com_values[histnr][0]);
		  }
			/*
			 * if (histnr==2){ cout << "E = " << E <<endl;
			 */
//  Convert back from FWHM to sigma, and include 3 sigma of photopeak
		    //if((E > (values[histnr][0] - 40 * values[histnr][2]/2.35)) && (E < (values[histnr][0] + 3 * values[histnr][2]/2.35))) { // To test hypothesis about time resolution		
	if((E > (values[histnr][0] - 4 * values[histnr][2]/2.35)) && (E < (values[histnr][0] + 3 * values[histnr][2]/2.35))) {
		        	xpos[histnr]->Fill(xdata);
				ypos[histnr]->Fill(ydata);
				if   (dotac){	
               		if((pmt > pmt_low) && (pmt < pmt_up)) {
			  //					tacs2[histnr]->Fill( (Float_t) tac / TACCAL);
			}			
#ifdef DEG                      
                      if (histnr==34){
                          counter++;
                          cout << " Event " << counter << ":: i = " << i << " E = " << E << " tac = " << tac/TACCAL <<"  pmt = " << pmt << endl;
                          cout << " x = " << xdata << " y = " << ydata  << " Elim  :: " <<   values[histnr][0] - 4 * values[histnr][2]/2.35 << "  " <<   values[histnr][0] + 3 * values[histnr][2]/2.35 << " xpeak = " << xpeak[histnr] << " ypeak = " << ypeak[histnr] << endl;
}
#endif
}
				if((i < 20) && (verbose)) {
					cout << endl;
					cout << "E = " << E << " x = " << xdata << " y= " << ydata << " histnr = " << histnr << endl;
					cout <<
						"values[histnr][0] = " <<
						values[histnr][0] <<
						" x[histnr] = " <<
						xpeak[histnr] <<
						" y[histnr] = " <<
						ypeak[histnr] <<
						endl;
				}
			}	/* if E is within the required range */
		}
		else {
			cout << "No associated histogram found !" << endl;
			cout << " Entry : " << i << " x = " << x << " y = " << y << endl;
		}

		min = 10000;
		histnr = 9999;
	}			/* for loop */

	cout << " ......... Done ! " << endl;

	/*
	 * for (k=0;
	 * k<PEAKS;
	 * k++){ normalise_noerr(xpos[k]);
	 * normalise_noerr(ypos[k]);
	 * } ;
	 * maybe I should really define a Class for analyzing all this data, containing
	 * the ntruple, peak positions, and all that stuff
	 */
	return 0;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
/*
Int_t peaktovalley(TH1F *xpos[PEAKS], TCanvas *c1)
{
//
	Char_t	histname[40], histtitle[40];
//

	c1->Clear();
	c1->Divide(2, 4);
	for(Int_t i = 0; i < 8; i++) {
		sprintf(histname, "xsums[%d]", i);
		sprintf(histtitle, "Peak to valley row %d", i);
		xsums[i] = new TH1F(histname, histtitle, NRPOSBINS, POSMIN, POSMAX);
		for(Int_t j = 0; j < 8; j++) {

		  //	
		  //	 * xsums[i]->Add(xpos[8*i+j],1);
		  //	 * } ;
		  //	 * normalise(xpos[i+j*8]);
		  //	 
			xsums[i]->Add(xpos[i * 8 + j], 1);
		}

		c1->cd(i + 1);
		xsums[i]->Draw();
	}

	return 0;
}
*/
/*
 =======================================================================================================================
 =======================================================================================================================
 */
Double_t *ptv_ana(TH1F *hist, TF1 *posgausfits[],TCanvas *c1, Int_t nr, Int_t x, Int_t verbose)
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	Double_t	av_fwhm, av_mean_dist, av_mean_dist_e, av_fwhm_e;
	Int_t		i, index;
	Char_t		tmpstring[60];
	Double_t	*ptvalues;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	ptvalues = new Double_t[8];
	av_mean_dist = 0;
	av_mean_dist_e = 0;
	av_fwhm = 0;
	av_fwhm_e = 0;

	c1->Clear();
	c1->Divide(1, 2);
	c1->cd(1);
	hist->Draw();

	/*~~~~~~~~~~*/
	Int_t	next;
	Int_t	p = 1;
	/*~~~~~~~~~~*/

//TODO  FIX FOM when there are no events in xpos[i] and posgausfits[i]-> parameters are 0,0,0 ;

	for(i = p; i < (8 - p); i++) {
		if(x) {
			index = 8 * nr + i;
			next = 8 * nr + i + 1;
		}
		else {
			index = 8 * i + nr;
			next = 8 * (i + 1) + nr;
		}

		if(verbose) {
			cout << " index = " << index << " next =" << next << endl;
		}

		posgausfits[index]->Draw("same");

		/*
		 * gPad->Update();
		 * gPad->Update();
		 */
		if(i < (7 - p)) {
			av_mean_dist += posgausfits[next]->GetParameter(1) - posgausfits[index]->GetParameter(1);
			av_mean_dist_e += power(posgausfits[index]->GetParError(1), 2) + power(posgausfits[next]->GetParError(1), 2);
		}

		av_fwhm += posgausfits[index]->GetParameter(2);
		av_fwhm_e += power(posgausfits[index]->GetParError(2), 2);
		if(verbose) {
			cout << i << " mean = " << posgausfits[index]->GetParameter(1) << " sigma = ";
			cout << posgausfits[index]->GetParameter(2) << " mean_dist = ";
			if(i < (7 - p)) {
				cout << posgausfits[next]->GetParameter(1) - posgausfits[index]->GetParameter(1);
				cout << " error : " << TMath::Sqrt(power(posgausfits[index]->GetParError(1), 2) + power(posgausfits[next]->GetParError(1), 2)) << "  mean_dist_e = " << av_mean_dist_e << endl;
			}
			else
				cout << " NA " << endl;
		}	/* verbose */
	}

	av_mean_dist_e = TMath::Sqrt(av_mean_dist_e);
	av_fwhm_e = TMath::Sqrt(av_fwhm_e);
	av_mean_dist /= (7 - 2 * p);
	av_mean_dist_e /= (7 - 2 * p);
	av_fwhm /= (8 - 2 * p);
	av_fwhm_e /= (8 - 2 * p);

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	Double_t	mean_variance, fwhm_variance;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	mean_variance = 0;
	fwhm_variance = 0;

	for(i = p; i < (8 - p); i++) {
		if(x) {
			index = 8 * nr + i;
			next = 8 * nr + i + 1;
		}
		else {
			index = 8 * i + nr;
			next = 8 * (i + 1) + nr;
		}

		if(i < (7 - p)) {
			mean_variance += power
				(
					av_mean_dist - (posgausfits[next]->GetParameter(1) - posgausfits[index]->GetParameter(1)),
					2
				);
		}

		fwhm_variance += power(av_fwhm - (posgausfits[index]->GetParameter(2)), 2);
	}

	mean_variance = TMath::Sqrt(mean_variance / (8 - 2 * p - 1));
	fwhm_variance = TMath::Sqrt(fwhm_variance / (8 - 2 * p));

	av_fwhm *= 2.345;
	av_fwhm_e *= 2.345;
	fwhm_variance *= 2.345;
	if(verbose) {
		cout << "MEAN DISTANCE = " << av_mean_dist << " +/- " << av_mean_dist_e << endl;
		cout << "MEAN FWHM     = " << av_fwhm << " +/- " << av_fwhm_e << endl;
		cout << "MEAN DISTANCE RMS = " << mean_variance << " RMS/sqrt(" << 8-2*p-1 << ") = " << mean_variance/TMath::Sqrt(8 - 1 - 2 * p) << endl;
		cout << "FWHM RMS = " << fwhm_variance << " RMS/sqrt(" << 8 - 2 * p << ") = " << fwhm_variance / TMath::Sqrt(8 - 2 * p) << endl;
	}

   // Analyze the edge
  Double_t edge_fwhm, edge_dist;
  Double_t edge_fwhm_e, edge_dist_e;
  edge_fwhm=0;
  edge_dist=0;
  edge_fwhm_e=0;
  edge_dist_e=0;

  for (i=0;i<2;i++){
   if (x) { index = 8*nr+i*6;next=8*nr+i*6+1;}
   else {index = 8*i*6+nr;next=8*(i*6+1)+nr;}
   edge_dist+=posgausfits[next]->GetParameter(1)-posgausfits[index]->GetParameter(1);
  edge_dist_e+=power(posgausfits[index]->GetParError(1),2)+power(posgausfits[next]->GetParError(1),2);
  edge_fwhm+=posgausfits[index]->GetParameter(2);
  edge_fwhm+=posgausfits[next]->GetParameter(2);
  edge_fwhm_e+=power(posgausfits[index]->GetParError(2),2);
  edge_fwhm_e+=power(posgausfits[next]->GetParError(2),2);
    }
  edge_dist_e=TMath::Sqrt(edge_dist_e);
  edge_fwhm_e=TMath::Sqrt(edge_fwhm_e);
  edge_dist/=2;edge_dist_e/=2;
  edge_fwhm/=4;edge_fwhm_e/=4;

   Double_t edge_mean_variance,edge_fwhm_variance;
  edge_mean_variance=0;
  edge_fwhm_variance=0;

  for (i=0;i<2;i++){
  if (x) { index = 8*nr+i*6;next=8*nr+i*6+1;}
   else {index = 8*i*6+nr;next=8*(i*6+1)+nr;}
    edge_mean_variance+=power( edge_dist - (posgausfits[next]->GetParameter(1)-posgausfits[index]->GetParameter(1)) ,2);
    if (verbose){
      cout << " i = " << i << " :: " << posgausfits[next]->GetParameter(1)-posgausfits[index]->GetParameter(1) << " edge_mean variance :: " <<
 edge_mean_variance <<  "  edge mean :: " << edge_dist << endl;}
   edge_fwhm_variance+=power( edge_fwhm -( posgausfits[index]->GetParameter(2)) ,2);   edge_fwhm_variance+=power( edge_fwhm -( posgausfits[next]->GetParameter(2)) ,2);
  }

  edge_mean_variance=TMath::Sqrt(edge_mean_variance/(2-1));
  edge_fwhm_variance=TMath::Sqrt(edge_fwhm_variance/(4-1));

  //  cout << " edge_mean variance :: " << edge_mean_variance << endl;

  edge_fwhm*=2.345;edge_fwhm_e*=2.345;
  edge_fwhm_variance*=2.345;



	c1->cd(2);

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	TPaveText	*result = new TPaveText(.16, .16, .85, .69, "NDC");
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	sprintf
	(
		tmpstring,
		"MEAN DISTANCE = %.4f #pm %.4f (RMS/sqrt(%d) = %.4f)",
		av_mean_dist,
		av_mean_dist_e,
		8 - 2 * p - 1,
		mean_variance / TMath::Sqrt(8 - 2 * p - 1)
	);
	result->AddText(tmpstring);
	sprintf
	(
		tmpstring,
		"MEAN FWHM = %.4f #pm %.4f (RMS/sqrt(%d) = %.4f)",
		av_fwhm,
		av_fwhm_e,
		8 - 2 * p,
		fwhm_variance / TMath::Sqrt(8 - 2 * p)
	);
	result->AddText(tmpstring);
	result->Draw();

	ptvalues[0] = av_mean_dist;
	ptvalues[1] = mean_variance / TMath::Sqrt(8 - 1 - 2 * p);
	ptvalues[2] = av_fwhm;
	ptvalues[3] = fwhm_variance / TMath::Sqrt(8 - 2 * p);

        ptvalues[4]=edge_dist;
	ptvalues[5]=edge_mean_variance/TMath::Sqrt(2-1);
	ptvalues[6]=edge_fwhm;
	ptvalues[7]=edge_fwhm_variance/TMath::Sqrt(4-1);


	return ptvalues;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
Int_t ptv_ana_old(TH1F *hist, TCanvas *c1)
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	TSpectrum	*ptv_top = new TSpectrum();
	TSpectrum	*ptv_bot = new TSpectrum();
	Int_t		nrtoppeaks, nrbotpeaks;
	Float_t		*top_xpeaks, *bot_xpeaks;
	/* Due to the way the valleys are determined, we need space for 9 valleys */
	Float_t		peaks[8];
	Double_t	fitvars[8 * 3];
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	nrtoppeaks = ptv_top->Search(hist, 2, "", 0.5);
	top_xpeaks = ptv_top->GetPositionX();

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	TH1F	*histinv = (TH1F *) hist->Clone();
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	histinv->Scale(-1);
	nrbotpeaks = ptv_bot->Search(histinv, 2, "", 0.5);
	bot_xpeaks = ptv_bot->GetPositionX();

	/*
	 * for (Int_t i=0;
	 * i<nrtoppeaks;
	 * i++){ ;
	 * cout << i << " " << top_xpeaks[i] << endl;
	 * } ;
	 * Float_t dummy[10];
	 * bubbleSort(top_xpeaks,nrtoppeaks,dummy);
	 */
	for(Int_t i = 0; i < nrtoppeaks; i++) {
		peaks[i] = hist->GetBinContent(hist->FindBin(top_xpeaks[i]));

		/*
		 * cout << i << " " << top_xpeaks[i] << " " <<
		 * hist->GetBinContent(hist->FindBin(top_xpeaks[i])) <<endl;
		 */
	}

	cout << endl;

	/*
	 * for (Int_t i=0;
	 * i<nrbotpeaks;
	 * i++){ ;
	 * cout << i << " " << bot_xpeaks[i] << " " << endl;
	 * }
	 */
	cout << endl;

	/*
	 * bubbleSort(bot_xpeaks,nrbotpeaks,dummy);
	 */
	cout << nrbotpeaks << endl;

	/*
	 * for (Int_t i=1;
	 * i<nrbotpeaks-1;
	 * i++){ valleys[i-1]= hist->GetBinContent(hist->FindBin(bot_xpeaks[i]));
	 * cout << i-1 << " " << valleys[i-1] << endl;
	 */
	cout << nrtoppeaks << " peaks found " << endl;

	for(Int_t i = 0; i < nrtoppeaks; i++) {
		fitvars[3 * i] = hist->GetBinContent(hist->FindBin(top_xpeaks[i]));
		fitvars[1 + 3 * i] = top_xpeaks[i];
		fitvars[2 + 3 * i] = 0.01;
	}

	/*
	 * TF1 *fitfunc = new TF1("fitfunc",TotalPTV,-1,1,8*3);
	 * fitfunc->SetParameters(fitvars);
	 * hist->Fit("fitfunc");
	 * ;
	 * for (Int_t i=0;
	 * i<4;
	 * i++){ if (valleys[i]) ratio[i]=peaks[i]/valleys[i];
	 * else ratio[i]=peaks[i];
	 * // if valley value is one cout << i << " " << peaks[i] << " " << valleys[i] <<endl;
	 * } for (Int_t i=4;
	 * i<8;
	 * i++){ if (valleys[i]) ratio[i]=peaks[i]/valleys[i];
	 * else ratio[i]=peaks[i];
	 * // if valley value is one cout << i << " " << peaks[i] << " " << valleys[i] <<endl;
	 * }
	 */
	c1->Clear();
	hist->Draw();

	/*
	 * fitfunc->Draw();
	 */
	return 0;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */

Int_t fitpos(TH1F *xpos[PEAKS], TF1 *posgausfits[PEAKS],Int_t verbose = 0)
{
	/*~~~~~~~~~~~~~~~~~~*/
	Char_t	tmpstring[40];
	Int_t	i;
	/*~~~~~~~~~~~~~~~~~~*/

	for(i = 0; i < PEAKS; i++) {
		if(verbose) {
			cout << "Fitting Histogram xpos[" << i << "]" << endl;
		}

		/*
		 * xpos[i]->Fit("gaus");
		 */
                if ( xpos[i]->GetEntries() > 10 ) {
		  posgausfits[i] = fitgaus_old_lim(xpos[i], !(verbose), 0.05);
		sprintf(tmpstring, "posgausfits[%d]", i);
		posgausfits[i]->SetName(tmpstring);
		if((posgausfits[i]->GetChisquare() == 0) || (posgausfits[i]->GetParameter(0) > 1e4) ||
                   (posgausfits[i]->GetNDF() < 4) ) {
		  //		        if (verbose) xpos[i]->Fit("gaus", "");
		  //                        else xpos[i]->Fit("gaus", "Q");
		  //          posgausfits[i]=fitgaus(xpos[i], !(verbose), 8);
		                         posgausfits[i]=fitgaus_peak(xpos[i], !(verbose));
		       //			posgausfits[i] = (TF1 *) xpos[i]->GetListOfFunctions()->FindObject("gaus");
			posgausfits[i]->SetLineColor(kBlue);
		}
               }  else {
                  if (verbose) cout << "Warning:: no events found in histogram xpos[" << i << "] ." << endl;
                    sprintf(tmpstring, "posgausfits[%d]", i); 
                    posgausfits[i] = new TF1(tmpstring,"gaus",-1,1);
                    posgausfits[i]->SetParameters(0,0,1);  
                }
 		/*
		 * cout << " done " <<endl;
		 * posgausfits[i]->SetRange(-1,1);
		 */
	}

	return 0;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */

//Double_t TotalPTV(Double_t *x, Double_t *par)

// Combined background + signal 
//{ 
	/*~~~~~~~~~~~~~~~~*/
  //	Double_t	tot = 0;
		/*~~~~~~~~~~~~~~~~*/
	/*
	for(Int_t i = 0; i < 8; i++) {
		tot += Signal(x, &par[3 * i]);
	}

	return tot;
}
*/
