/*

This program fills the energy histograms for every crystal, needed to do energy
calibration. If we don't have a valid segmnetation, the crystal id remains -1.

 A new file is created, this is a little redundant. Crystal id's are assigned.
 */
#include <string>
#include "TROOT.h"
#include "Riostream.h"
#include "TTree.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TSpectrum.h"
#include "TSpectrum2.h"
#include "TH2F.h"
#include "TVector.h"
#include "TMath.h"
#include "TF1.h"
#include "TChain.h"
#include "ModuleDat.h"
#include "Syspardef.h"
#include "libInit_avdb.h"
#include "myrootlib.h"
#include "PixelCal.h"
#include "PPeaks.h"


#define MINPIXENTRIES 1000
#define NOVALIDPEAKFOUND -8989898

void usage()
{
    cout << "enecal [-v -h] -f [filename]\n"
         << endl;
}

Float_t GetPeak(
        TH1F *hist,
        Float_t xlow,
        Float_t xhigh,
        Bool_t force,
        Int_t verbose = 0)
{
    Int_t npeaks=0;
    Int_t i=0,corpeak=0;
    TSpectrum ss;
    Float_t yy;

    if (verbose) {
        cout << " Funtion GetPeak. Looking for peak between " << xlow << " and " << xhigh <<  " in histogram " << hist->GetName() << endl;
    }
    yy=0;

    while((npeaks < 2) && (i < 10)) {
        if(verbose) {
            cout << "loop " << i << " " << npeaks << endl;
        }
        if (i < 9 )  {
            npeaks = ss.Search(hist, 3, "", 0.9 - (Float_t) i / 10);
        } else {
            npeaks = ss.Search(hist, 3, "", 0.05);
        }
        i++;
        if(i > 10) {
            if(verbose) {
                cout << " Warning " << npeaks << "found !" << endl;
            }
            break;
        }
    } //while loop

    if(verbose) {
        cout << npeaks << " peaks found " << i << " number of iterations" << endl;
    }

    if(npeaks != 1) {
        for(i = 0; i < npeaks; i++) {
            if(verbose) {
                cout << "x= " << *(ss.GetPositionX() + i) << " y = " << *(ss.GetPositionY() + i) << endl;
            }
            /* take largest peak with x position larger than lower fit limit */
            if( (yy < *(ss.GetPositionY() + i) ) && (*(ss.GetPositionX() + i) > xlow) && (*(ss.GetPositionX() +i) < xhigh)) {
                corpeak = i;
                yy = *(ss.GetPositionY() + i);
                if (verbose) {
                    cout << " Assuming peak " << corpeak << " at x=" << *(ss.GetPositionX() + corpeak) << endl;
                }
            }
        } // for loop
    } else {
        corpeak=0;
    }

    if(verbose) {
        cout << "The correct peak out of " << npeaks << " peaks is peak number " << corpeak;
        cout << " at position: x = " << *(ss.GetPositionX() + corpeak) << endl;
    }

// FIRST CHECK ::
    if(( (*(ss.GetPositionX() + corpeak)) < xlow) || ((*(ss.GetPositionX() + corpeak)) > xhigh))  {

        if(verbose) {
            cout << " Peak at " << *(ss.GetPositionX() + corpeak) << " is not within the window " << endl;
        }
        // move SIGMA rather than threshold

        if (!(force)) {
            return NOVALIDPEAKFOUND;
        }

        i=0;
        npeaks=0;
        Int_t j;
        Int_t stop=0;
        /* I find that sigma = 3 is too small, limit it to 4 by demanding i<7 */
        while((i < 7)&&(!stop)) {
            npeaks = ss.Search(hist, 10 - i, "nobackground", 0.4);

            if(verbose) {
                cout << "2nd while loop " << i << " " << npeaks << endl;
            }
            if(verbose) {
                cout << npeaks << " peaks found " << i << " number of iterations" << endl;
                cout << " peak position :: " << *ss.GetPositionX() << endl;
            }
            i++;
            if(npeaks != 1) {
                for(j = 0; j < npeaks; j++) {
                    if(verbose) {
                        cout << "x= " << *(ss.GetPositionX() + j) << " y = " << *(ss.GetPositionY() + j) << endl;
                    }
                    /* take largest peak with x position larger than lower fit limit */
                    /* CHANGE !! */
                    if((yy < *(ss.GetPositionY() + j)) && (*(ss.GetPositionX() + j) > xlow) && (*(ss.GetPositionX() +j) < xhigh)) {
                        corpeak = j;
                        yy = *(ss.GetPositionY() + j);
                    }
                } // for loop

                if(verbose) {
                    cout << "The correct peak out of " << npeaks << " peaks is peak number " << corpeak;
                    cout << " at position: x = " << *(ss.GetPositionX() + corpeak) << endl;
                }
            } // if npeaks !=1

            else {
                corpeak=0;
            }


            if(((*(ss.GetPositionX() + corpeak)) < xlow) || ((*(ss.GetPositionX() + corpeak)) > xhigh ) ) {
                if(verbose) {
                    cout << "Peak at " << *(ss.GetPositionX() + corpeak) << " is not within the window " << endl;
                }
            } else {
                if (verbose) {
                    cout << "Valid peak found at : " << *(ss.GetPositionX()+corpeak) << endl;
                }
                stop=1;
            }
        } // while loop

        if (!(stop)) {
            yy = 0;

            /* note 9-6-13: changed sigma from 2 to 3,  need to clean up FIXME */
            npeaks = ss.Search(hist, 3, "", 0.4);
            for(i = 0; i < npeaks; i++) {
                if(verbose) {
                    cout << "x= " << *(ss.GetPositionX() + i) << " y = " << *(ss.GetPositionY() + i) << endl;
                }

                /* take largest peak with x position larger than lower fit limit */
                /* CHANGE !! */
                if((yy < *(ss.GetPositionY() + i)) && (*(ss.GetPositionX() + i) > xlow) && (*(ss.GetPositionX() +i) < xhigh)) {
                    corpeak = i;
                    yy = *(ss.GetPositionY() + i);
                }
            } // for loop

            if(verbose) {
                cout << "The correct peak out of " << npeaks << " peaks is peak number " << corpeak;
                cout << " at position: x = " << *(ss.GetPositionX() + corpeak) << endl;
            }
// SECOND CHECK
            if((*(ss.GetPositionX() + corpeak)) < xlow) {
                if(verbose) {
                    cout << "Peak at " << *(ss.GetPositionX() + corpeak) << " is STILL (II) not within the window " << endl;
                }

                /* Lower "SIGMA" of tspectrum */
                npeaks = ss.Search(hist, 3, "", 0.3);
                for(i = 0; i < npeaks; i++) {
                    if(verbose) {
                        cout << "x= " << *(ss.GetPositionX() + i) << " y = " << *(ss.GetPositionY() + i) << endl;
                    }
                    /* take largest peak with x position larger than lower fit limit */
                    /* CHANGE !! */
                    if((yy < *(ss.GetPositionY() + i)) && (*(ss.GetPositionX() + i) > xlow) && (*(ss.GetPositionX() +i) < xhigh)) {
                        corpeak = i;
                        yy = *(ss.GetPositionY() + i);
                    }
                } // for loop

                if(verbose) {
                    cout << "The correct peak out of " << npeaks << " peaks is peak number " << corpeak;
                    cout << " at position: x = " << *(ss.GetPositionX() + corpeak) << endl;
                }

// THIRD CHECK
                if((*(ss.GetPositionX() + corpeak)) < xlow) {
                    if(verbose) {
                        cout << "Peak at " << *(ss.GetPositionX() + corpeak) << " is STILL (III) not within the window " << endl;
                    }

                    /* Lower "SIGMA, threshold" of tspectrum  FIXME -- mind this too  */
                    npeaks = ss.Search(hist, 2, "", 0.2);
                    for(i = 0; i < npeaks; i++) {
                        if(verbose) {
                            cout << "x= " << *(ss.GetPositionX() + i) << " y = " << *(ss.GetPositionY() + i) << endl;
                        }
                        /* take largest peak with x position larger than lower fit limit */

                        /* CHANGE !! */
                        if((yy < *(ss.GetPositionY() + i)) && (*(ss.GetPositionX() + i) > xlow) && (*(ss.GetPositionX() +i) < xhigh)) {
                            corpeak = i;
                            yy = *(ss.GetPositionY() + i);
                        }
                    } // for loop

                    if(verbose) {
                        cout << "The correct peak out of " << npeaks << " peaks is peak number " << corpeak;
                        cout << " at position: x = " << *(ss.GetPositionX() + corpeak) << endl;
                    }

// FOURTH CHECK
                    if((*(ss.GetPositionX() + corpeak)) < xlow) {
                        if(verbose) {
                            cout << "Peak at " << *(ss.GetPositionX() + corpeak) << " is STILL (IV) not within the window " << endl;
                        }


// Giving up, taking the highest peak

                        if (verbose) {
                            cout << " Returning NOVALIDPEAKFOUND " << endl;
                        }

                        return  NOVALIDPEAKFOUND;

                        yy = 0;
                        for(i = 0; i < npeaks; i++) {
                            if(yy < *(ss.GetPositionX() + i)) {
                                corpeak = i;
                                yy = *(ss.GetPositionX() + i);
                            }
                        }

                        if(verbose) {
                            cout << "The correct peak out of " << npeaks << " peaks is peak number " << corpeak;
                            cout << " at position: x = " << *(ss.GetPositionX() + corpeak) << endl;
                        }
                    }
                }
            }
        }
    }
    return *(ss.GetPositionX()+corpeak);
}

Int_t FitApdEhis(
        Int_t cc,
        Int_t f,
        Int_t i,
        Int_t j,
        PixelCal * fCrysCal,
        PPeaks * fPPeaks,
        TH1F *fEhist[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE][XTALS_PER_APD],
        TH1F *fEhist_com[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE][XTALS_PER_APD],
        Int_t verbose = 0)
{
    Float_t Emean, Emean_com,peak;
    TF1 spatfit("spatfit","gaus", EFITMIN, EFITMAX);
    TF1 comfit("comfit","gaus",EFITMIN_COM,EFITMAX_COM);


    if (verbose) {
        cout << endl;
        cout << " --------------- C" << cc << "F" <<f << " MODULE " << i << " PSAPD " << j;
        cout << " ---------------- " << endl;
    }

    if (fCrysCal->validpeaks[cc][f][i][j]) {
        if (verbose) {
            cout << " Getting mean energy :: " << endl;
            cout << " fPPeaks : " << fPPeaks << endl;
            cout << " fPPeaks->spat[cc][f][i][j] = " << fPPeaks->spat[cc][f][i][j] <<endl;
            cout << " fPPeaks->com[cc][f][i][j] = " << fPPeaks->com[cc][f][i][j] <<endl;
        }
        Emean = fPPeaks->spat[cc][f][i][j];
        Emean_com = fPPeaks->com[cc][f][i][j];

        if (verbose)  {
            cout  << " S :: ppVals : " << Emean   << " Entries :" << fEhist[cc][f][i][j][0]->GetEntries()   << endl;
        }
        if (verbose)  {
            cout  << " C :: ppVals : " << Emean_com   << " Entries :" << fEhist_com[cc][f][i][j][0]->GetEntries()   << endl;
        }

        for (Int_t k=0; k<XTALS_PER_APD; k++) {
            if ( fEhist[cc][f][i][j][k]->GetEntries() < MINPIXENTRIES ) {
                fEhist[cc][f][i][j][k]->Rebin(2);
            }
            if (verbose) {
                cout << "===================================================================" << endl;
                cout << "= Histogram["<<cc<<"]["<<f<<"]["<<i<<"]["<<j<<"]["<<k<<"]"                         << endl;
                cout << "===================================================================" << endl;
                cout << " ----------- Spatials ---------------- " <<endl;
                cout << " Hist entries : " << fEhist[cc][f][i][j][k]->GetEntries() << " Efits mean :" ;
                cout << fPPeaks->spat[cc][f][i][j] << " nrbins :: " << fEhist[cc][f][i][j][k]->GetNbinsX() << endl;
            }
            // edge has lower gain
            if ((k==0)||(k==7)||(k==54)||(k==63)) {
                peak=GetPeak( fEhist[cc][f][i][j][k], Emean*0.7,Emean*1.15,0);
                if (peak==NOVALIDPEAKFOUND) {
                    if (verbose) {
                        cout << " That didn't go well. Retry to get peak with larger margins " << endl;
                    }
                    peak=GetPeak( fEhist[cc][f][i][j][k], Emean*0.6,Emean*1.5,1);
                }
            } else {
                peak=GetPeak( fEhist[cc][f][i][j][k], Emean*0.85,Emean*1.15,0);
                if (peak==NOVALIDPEAKFOUND) {
                    if (verbose) {
                        cout << " That didn't go well. Retry to get peak with larger margins " << endl;
                    }
                    peak=GetPeak( fEhist[cc][f][i][j][k], Emean*0.7,Emean*1.5,1);
                }
            }
            if (peak==NOVALIDPEAKFOUND) {
                peak=Emean;
            }
            spatfit.SetParameter(1,peak);
            spatfit.SetParameter(2,0.04*peak);

            if (verbose) {
                cout << " Peak @ " << peak <<", fitting between : " << peak*0.85 << " and " << peak*1.15 << endl;
            }
            if ( (fEhist[cc][f][i][j][k]->GetEntries()) >  MINEHISTENTRIES ) {
                fEhist[cc][f][i][j][k]->Fit(&spatfit,"Q","",peak*0.85,peak*1.15);
            }
            fCrysCal->GainSpat[cc][f][i][j][k]=spatfit.GetParameter(1);
            if (TMath::Abs(fCrysCal->GainSpat[cc][f][i][j][k]/Emean -1 ) > 0.3 )          {
                if (verbose) {
                    cout << " BAD FIT to histogram ["<<cc<<"]["<<f<<"]["<<i<<"]["<<j<<"]["<<k<<"]" << endl;
                    cout << " taking PP @ " << Emean << " ( peak = " << peak << " ) " << endl;
                }
                fCrysCal->GainSpat[cc][f][i][j][k]=Emean;
            }
            if (spatfit.GetParameter(1)) {
                fCrysCal->EresSpat[cc][f][i][j][k]=235*spatfit.GetParameter(2)/spatfit.GetParameter(1) ;
            } else {
                fCrysCal->EresSpat[cc][f][i][j][k]=.99;
            }

            if (verbose) {
                cout << " ------------ Common ----------------- " <<endl;
                cout << " Hist entries : " << fEhist_com[cc][f][i][j][k]->GetEntries() << " Efits mean :" ;
                cout << fPPeaks->com[cc][f][i][j] << endl;
            }
            fEhist_com[cc][f][i][j][k]->SetAxisRange(E_low_com,SATURATIONPEAK,"X");
            peak=GetPeak( fEhist_com[cc][f][i][j][k], Emean_com*0.85,Emean_com*1.25,0);
            if (peak==NOVALIDPEAKFOUND) {
                if (verbose) {
                    cout << " That didn't go well. Retry to get peak with larger margins " << endl;
                }
                if ((k==0)||(k==7)||(k==54)||(k==63)) {
                    peak=GetPeak( fEhist_com[cc][f][i][j][k], Emean_com*0.6,Emean_com*1.25,1);
                } else {
                    peak=GetPeak( fEhist_com[cc][f][i][j][k], Emean_com*0.7,Emean_com*1.5,1);
                }
            }
            if (peak==NOVALIDPEAKFOUND) {
                peak=Emean_com;
            }
            comfit.SetParameter(1,peak);
            comfit.SetParameter(2,0.04*peak);
            if (verbose) {
                cout << " Peak @ " << peak <<", fitting between : " << peak*0.85 << " and " << peak*1.15 << endl;
            }
            if ( (fEhist_com[cc][f][i][j][k]->GetEntries()) >  MINEHISTENTRIES ) {
                fEhist_com[cc][f][i][j][k]->Fit(&comfit,"Q","",peak*0.85,peak*1.15);
            }
            //fEfit_com[cc][f][i][j][k]->SetLineColor(kBlue);
            fCrysCal->GainCom[cc][f][i][j][k]=comfit.GetParameter(1);
            if (TMath::Abs(fCrysCal->GainCom[cc][f][i][j][k]/Emean_com -1 ) > 0.35 )          {
                if (verbose) {
                    cout << " BAD FIT to histogram ["<<cc<<"]["<<f<<"]["<<i<<"]["<<j<<"]["<<k<<"]" << endl;
                    cout << " taking PP @ " << Emean_com << " ( peak = " << peak << " ) " << endl;
                }
                fCrysCal->GainCom[cc][f][i][j][k]=Emean_com;
            }

            if (comfit.GetParameter(1)) {
                fCrysCal->EresCom[cc][f][i][j][k]=235*comfit.GetParameter(2)/comfit.GetParameter(1) ;
            } else {
                fCrysCal->EresCom[cc][f][i][j][k]=.99;
            }

        }
    }

    return 0;
}

int main(int argc, Char_t *argv[])
{
    if (argc == 1) {
        usage();
        exit(0);
    }
    cout << " Welcome to EneCal. Performs Crystal Binning." ;

    string filename;
    string treename = "mdata";
    Int_t verbose = 0;

    for (int ix = 1; ix < argc; ix++) {
        if(strcmp(argv[ix], "-h") == 0) {
            usage();
            exit(0);
        }
        if(strcmp(argv[ix], "-v") == 0) {
            cout << "Verbose Mode " << endl;
            verbose = 1;
        }
    }

    for (int ix = 1; ix < (argc - 1); ix++) {
        if(strcmp(argv[ix], "-f") == 0) {
            filename = string(argv[++ix]);
        }
    }

    rootlogon(verbose);
    time_t starttime = time(NULL);


    if (filename == "") {
        cout << " Please specify Filename. Exiting. " << endl;
        return -99;
    }

    cout << " Inputfile :: " << filename << endl;

    TFile *rfile = new TFile(filename.c_str(), "UPDATE");

    ifstream infile;

    size_t root_file_ext_pos(filename.rfind(".root"));
    if (root_file_ext_pos == string::npos) {
        cerr << "Unable to find .root extension in: \"" << filename << "\"" << endl;
        cerr << "...Exiting." << endl;
        return(-4);
    }
    string filebase(filename, 0, root_file_ext_pos);

    TChain *block;
    block = (TChain *) rfile->Get(treename.c_str());

    if (!block) {
        cout << " Problem reading Tree " << treename  << " from file " << filename << endl;
        cout << " Exiting " << endl;
        return -10;
    }


    if (verbose) {
        cout << " Read block from file :: " << time(NULL)-starttime << endl;
        cout << " Entries in block :: " << block->GetEntries() << endl;
    }
    PixelCal *CrysCal = new PixelCal("CrysCalPar");
    CrysCal->SetVerbose(verbose);

    CrysCal->ReadCal(filebase.c_str());
    if (verbose) {
        cout << " CrysCal->X[0][0][2][1][0] = " << CrysCal->X[0][0][2][1][0] << endl;
        cout << " fCrysCal->X[0][6][8][0][0] = " << CrysCal->X[0][6][8][0][0] << endl;
    }

    PPeaks *thesePPeaks = (PPeaks *) rfile->Get("PhotoPeaks");
    if (!(thesePPeaks)) {
        cout << "Couldn't read object PhotoPeaks from file " << rfile->GetName() << endl;
        exit(-1);
    }

    TH1F *fEhist[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE][XTALS_PER_APD];
    TH1F *fEhist_com[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE][XTALS_PER_APD];

    for (Int_t cc=0; cc<CARTRIDGES_PER_PANEL; cc++) {
        for (Int_t f=0; f<FINS_PER_CARTRIDGE; f++) {
            for (Int_t m=0; m<MODULES_PER_FIN; m++) {
                for (Int_t j=0; j<APDS_PER_MODULE; j++) {
                    for (Int_t k=0; k<XTALS_PER_APD; k++) {
                        TString hn;
                        TString ht;
                        hn.Form("Ehist_C%dF%dM%dA%dP%d",cc,f,m,j,k);
                        ht.Form("C%dF%d Module %d APD %d Pixel %d",cc,f,m,j,k);
                        fEhist[cc][f][m][j][k] = new TH1F(hn.Data(),ht.Data(), Ebins_pixel,E_low,E_up);
                        hn.Form("Ehist_com_C%dF%dM%dA%dP%d",cc,f,m,j,k);
                        ht.Form("C%dF%d Module %d APD %d Common Pixel %d",cc,f,m,j,k);
                        fEhist_com[cc][f][m][j][k] = new TH1F(hn.Data(),ht.Data(), Ebins_com_pixel,E_low_com,E_up_com);
                    }
                }
            }
        }
    }


    cout << "Building flood histograms" << endl;
    TTree * input_tree = (TTree *) rfile->Get(treename.c_str());
    Long64_t no_entries = input_tree->GetEntries();
    ModuleDat * input_event = 0;
    cout << "Tree entries: " << no_entries << endl;
    input_tree->SetBranchAddress("eventdata", &input_event);
    for (Long64_t entry = 0; entry < no_entries; entry++) {
        input_tree->GetEntry(entry);
        int cartridge = input_event->cartridge;
        int fin = input_event->fin;
        int module = input_event->module;
        int apd = input_event->apd;
        float E = input_event->E;
        float Ec = input_event->Ec;
        float x = input_event->x;
        float y = input_event->y;
        if ((apd==0) || (apd==1) ) {
            if (CrysCal->validpeaks[cartridge][fin][module][apd]) {
                int _id = CrysCal->GetCrystalId(x,y,cartridge,fin,module,apd);
                if ((_id >= 0)&&( _id< 64)) {
                    fEhist[cartridge][fin][module][apd][_id]->Fill(E);
                    fEhist_com[cartridge][fin][module][apd][_id]->Fill(-Ec);
                }
            }
        }
    }

    for (Int_t cc=0; cc<CARTRIDGES_PER_PANEL; cc++) {
        for (Int_t f=0; f<FINS_PER_CARTRIDGE; f++) {
            for (Int_t i=0; i<MODULES_PER_FIN; i++) {
                for (Int_t j=0; j<APDS_PER_MODULE; j++) {
                    FitApdEhis(cc,f,i,j, CrysCal, thesePPeaks, fEhist, fEhist_com);
                }
            }
        }
    }

    string calpar_filename = filebase + ".par.root";
    cout << "Writing calibration file: " << calpar_filename << endl;
    TFile * rfi = new TFile(calpar_filename.c_str(),"RECREATE");

    CrysCal->Write();
    rfi->Close();
    rfile->Close();

    return(0);
}
