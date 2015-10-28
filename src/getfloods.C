#include <stdlib.h>
#include "getfloods.h"
#include "Sel_GetFloods.h"
#include "TTreePerfStats.h"
#include "TChain.h"
#include "TProof.h"
#include <sys/stat.h>
#include <string>

int Makeplot(
        TH2F *hist[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE],
        Int_t c,
        Int_t f,
        Int_t NRFLOODSTODRAW,
        const std::string & filebase,
        const Char_t suffix[40])
{
    TString fDIR="FLOODS";

    // We're going to write to directory fDIR, need to make sure it exists:
    if (access(fDIR.Data(), 0) != 0) {
        cout << "creating dir " << fDIR << endl;
        if (mkdir(fDIR, 0777) != 0) {
            cout << " Error making directory " << fDIR << endl;
            return -2;
        }
    }

    TCanvas * c1 = (TCanvas*)gROOT->GetListOfCanvases()->FindObject("c1");
    if (!c1) c1 = new TCanvas("c1","c1",10,10,1000,1000);
    c1->SetCanvasSize(1000,500);
    c1->Clear();

    int i = 0;
    for (int kk = 0; kk < TMath::Floor(MODULES_PER_FIN / NRFLOODSTODRAW); kk++)
    {
        c1->Clear();
        c1->Divide(NRFLOODSTODRAW,APDS_PER_MODULE);
        for (i = kk*NRFLOODSTODRAW; i < NRFLOODSTODRAW *(kk + 1); i++) {
            for (int j = 0; j < APDS_PER_MODULE; j++) {
                c1->cd((i % NRFLOODSTODRAW) + 1 + j * NRFLOODSTODRAW);
                hist[c][f][i][j]->Draw("colz");
            }
        }
        Char_t pngstring[FILENAMELENGTH];
        sprintf(pngstring,
                "%s/%s.C%dF%dM%d-%d.%s.",
                fDIR.Data(),
                filebase.c_str(),
                c,
                f,
                kk*NRFLOODSTODRAW,
                i-1,
                suffix);
        strcat(pngstring, "png");
        c1->Print(pngstring);
    }
    return 0;
}

void usage() {
    cout << "getfloods [-v] -f [filename]\n"
         << "  -d: dont print and just add the flood histograms to the file\n"
         << endl;
}

int main(int argc, Char_t *argv[])
{
    if (argc == 1) {
        usage();
        exit(0);
    }
    cout << " Welcome to Getfloods."
         << " Program obtains Energy and flood histograms.";

    Int_t verbose = 0;
    std::string filename;
    bool dont_print_flag = false;

    for(Int_t ix = 1; ix < argc; ix++) {
        if(strncmp(argv[ix], "-v", 2) == 0) {
            cout << "Verbose Mode " << endl;
            verbose = 1;
        }
        if(strcmp(argv[ix], "-d") == 0) {
            cout << "Not printing out floods" << endl;
            dont_print_flag = true;
        }
        if(strncmp(argv[ix], "-f", 2) == 0) {
            filename = std::string(argv[ix + 1]);
        }
    }

    cout << " Inputfile : " << filename << endl;

    set2dcolor(4);

    if (!verbose) {
        // kError, kWarning, kError, kBreak, kSysError, kFatal;
        gErrorIgnoreLevel = kFatal;
    }

    TH1F *E[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE];
    TH1F *E_com[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE];
    TH2F *floods[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE];

    TFile *rfile = new TFile(filename.c_str(), "UPDATE");
    if (!rfile || rfile->IsZombie()) {
        cout << "problems opening file " << filename << "\n.Exiting" << endl;
        return -11;
    }

    TCanvas *c1;

    size_t root_file_ext_pos(filename.rfind(".root"));
    if (root_file_ext_pos == string::npos) {
        cerr << "Unable to find .root extension in: \""
             << filename << "\"" << endl;
        cerr << "...Exiting." << endl;
        return(-2);
    }
    string filebase(filename, 0, root_file_ext_pos);
    if (verbose) cout << "filebase: " << filebase << endl;


    std::string treename = "mdata";
    TChain *block(0);
    block = (TChain *) rfile->Get(treename.c_str());

    if (!block) {
        cerr << " Problem reading Tree " << treename
             << " from file " << filename << endl;
        cerr << " Exiting " << endl;
        return -10;
    }
    Int_t entries=block->GetEntries();

    if (verbose) {
        cout << " Ok, we got " << entries << " entries. " << endl;
    }

    c1 = (TCanvas*)gROOT->GetListOfCanvases()->FindObject("c1");
    if (!c1) {
        c1 = new TCanvas("c1","c1",10,10,1000,1000);
    }
    c1->SetCanvasSize(700,700);
    c1->Clear();

    cout << "Retrieving Histograms" << endl;

    for (int c = 0; c < CARTRIDGES_PER_PANEL; c++) {
        for (int f = 0; f < FINS_PER_CARTRIDGE; f++) {
            if (verbose) {
                cout << " Obtaining histograms FIN " << f << endl;
            }
            for (int i = 0; i < MODULES_PER_FIN; i++) {
                for (int j = 0; j < APDS_PER_MODULE; j++) {
                    Char_t tmpstring[60];
                    sprintf(tmpstring,"C%dF%d/E[%d][%d][%d][%d]",c,f,c,f,i,j);
                    E[c][f][i][j]= (TH1F *) rfile->Get(tmpstring);
                    sprintf(tmpstring,"C%dF%d/E_com[%d][%d][%d][%d]",c,f,c,f,i,j);
                    E_com[c][f][i][j]= (TH1F *) rfile->Get(tmpstring);
                }
            }
        }
    }

    PPeaks *ph = new PPeaks("PhotoPeaks");

    cout << "Determining photopeak positions" << endl;

    for (int c=0; c<CARTRIDGES_PER_PANEL; c++) {
        for (int f=0; f<FINS_PER_CARTRIDGE; f++) {
            for (int i=0; i<MODULES_PER_FIN; i++) {
                for (int j=0; j<APDS_PER_MODULE; j++) {
                    Double_t pp_low= PP_LOW_EDGE;
                    Double_t pp_up= PP_UP_EDGE;
                    Double_t pp_low_com= PP_LOW_EDGE_COM;
                    Double_t pp_up_com= PP_UP_EDGE_COM;

                    if (verbose) {
                        cout << " ******************************************************* "  << endl;
                        cout << " * Determining Photopeak postion CARTRIDGE " << c << " FIN " << f << " MODULE " << i << " APD " << j << " *"<<endl;
                        cout << " ******************************************************* "  << endl;
                    }

                    if ( E[c][f][i][j]->GetEntries() > MINHISTENTRIES ) {
                        Double_t pp_right=GetPhotopeak_v1(E[c][f][i][j],pp_low,pp_up,0,12);
                        if (verbose) {
                            cout << " pp_right = " << pp_right << endl;
                        }
                        pp_low=0.7*pp_right;
                        pp_up=1.3*pp_right;
                        if (verbose) {
                            cout << ", pp_low = " << pp_low ;
                            cout << " pp_up = " << pp_up << endl;
                            cout << " --------- Common ----------- " <<endl;
                        }
                        Double_t pp_right_com=GetPhotopeak_v1(E_com[c][f][i][j],pp_low_com,pp_up_com,0,12);
                        pp_low_com=0.7*pp_right_com;
                        pp_up_com=1.3*pp_right_com;
                        if (verbose) {
                            cout << "pp_right_com = " << pp_right_com ;
                            cout << " pp_low_com = " << pp_low_com ;
                            cout << " pp_up_com = " << pp_up_com << endl;
                        }
                        ph->spat[c][f][i][j]=pp_right;
                        ph->com[c][f][i][j]=pp_right_com;
                    } else {
                        if (verbose ) {
                            cout << " Not enough entries in histogram E["<<c<<"]["<<f<<"]["<<i<<"]["<<j<<"]. Skipping."<<endl;
                        }
                        ph->spat[c][f][i][j] = 0;
                        ph->com[c][f][i][j] = 0;
                    }
                    if (verbose) {
                        cout << " ppeaks[" << c << "][" << f << "][" << i << "][" << j << "] = " << ph->spat[c][f][i][j] << endl;
                    }

                    Char_t titlestring[60];
                    Char_t tmpstring[60];
                    sprintf(tmpstring,
                            "flood_C%dF%dM%dA%d",
                            c, f, i, j);
                    sprintf(titlestring,
                            "C%dF%dM%dA%d Flood Histogram",
                            c, f, i, j);
                    floods[c][f][i][j] = new TH2F(tmpstring,
                                                  titlestring,
                                                  256, -1, 1,
                                                  256, -1, 1);
                }
            }
            if (!dont_print_flag) {
                makeplot(E, c, f, 4, "E", filebase);
                makeplot(E_com, c, f, 4, "Ecom", filebase);
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
        if (ph->spat[cartridge][fin][module][apd] > 0) {
             Float_t pp_low = 0.7 * ph->spat[cartridge][fin][module][apd];
             Float_t pp_up = 1.3 * ph->spat[cartridge][fin][module][apd];
             if ((input_event->E > pp_low) && (input_event->E < pp_up)) {
                 floods[cartridge][fin][module][apd]->Fill(
                        input_event->x, input_event->y);
             }
          }
    }

    if (!dont_print_flag) {
        cout << "Printing flood histograms" << endl;
        for (int c=0; c < CARTRIDGES_PER_PANEL; c++) {
            for (int f = 0; f < FINS_PER_CARTRIDGE; f++) {
                Makeplot(floods, c, f, 4, filebase, "floods");
            }
        }
    }

    rfile->cd();
    cout << "Writing flood histograms" << endl;
    for (int c = 0; c < CARTRIDGES_PER_PANEL; c++) {
        for (int f = 0; f < FINS_PER_CARTRIDGE; f++) {
            char tmpstring[30];
            sprintf(tmpstring, "C%dF%d", c, f);
            rfile->cd(tmpstring);
            for (int i = 0; i < MODULES_PER_FIN; i++) {
                for (int j = 0; j < APDS_PER_MODULE; j++) {
                    // Write the flood histogram to the file and make sure that
                    // we overwrite any existing files within the root file
                    // since we are merely updating the root file from chain
                    // parsed and getfloods could have been called before.
                    floods[c][f][i][j]->Write(
                            floods[c][f][i][j]->GetName(),
                            TObject::kOverwrite);
                }
            }
        }
    }
    cout << "Writing photopeak positions" << endl;
    rfile->cd();
    ph->Write();
    rfile->Close();

    cout << "Finished" << endl;
    return 0;
}

Double_t GetPhotopeak_v1(
        TH1F *hist,
        Double_t pp_low,
        Double_t pp_up,
        Int_t verbose,
        Int_t width)
{
    TSpectrum *sp = new TSpectrum();

    if (verbose) {
        cout << " ====== Welcome to GetPhotoPeak_v1, width =  "
             << width << " ====== " << endl;
    }

    Int_t efound=0;

    Float_t kkkf = 0.0;
    /* Initialize peak to lower bound */
    Double_t pp_right = pp_low;

    while (1) {
        // changing this from 10 to 3  080513
        Int_t npeaks = sp->Search(hist, width, "", 0.6 - (float) kkkf / 10);
        if (verbose ) {
            cout << npeaks
                 << " peaks found in the Spatial energy spectrum" << endl;
        }
        // get the peak between E_low and E_up ;
        for (Int_t l = 0; l < npeaks; l++) {
            // look for the peak with the highest x value
            if (verbose) {
                cout << " l = " << l <<" " <<*(sp->GetPositionX()+l) << "  "
                     << *(sp->GetPositionY()+l) << endl;
            }
            if (  ( *(sp->GetPositionX()+l) > pp_low ) &&
                    ( *(sp->GetPositionX()+l) < pp_up ) &&
                    ( *(sp->GetPositionX()+l) > pp_right) ) {
                if (verbose) {
                    cout << " Found correct peak @ "
                         << *(sp->GetPositionX()+l) << endl;
                }
                pp_right = (*(sp->GetPositionX()+l));
                efound=1;
                if (verbose) {
                    cout << "pp_right = " << pp_right ;
                }
            }
        }
        if ((!efound ) && (kkkf<5.5)) {
            if (kkkf<5) {
                kkkf+=1;
            } else {
                kkkf=5.5;
            }
        } else {
            break;
        }
    }

    if (verbose) {
        cout << " while loop done. pp_right = " << pp_right
             << ". Efound = " << efound << endl;
    }

    if ((!efound)&&(width>3)) {
        width-=2;
        if (verbose) {
            cout << " Changing width of photopeak window to :: " ;
            cout << width  << endl;
        }
        pp_right=GetPhotopeak_v1(hist,pp_low,pp_up,verbose,width);
        // try different width
    }

    if (verbose) {
        cout << " retURning pp_right = " << pp_right << endl;
    }

    return pp_right;
}

Double_t  GetPhotopeak_v2(
        TH1F *hist,
        Double_t pp_low,
        Double_t pp_up,
        Int_t verbose)
{
    TSpectrum *sp = new TSpectrum();

    Int_t kkk=0;
    Int_t efound=0;

    /* Initialize peak to lower bound */
    Double_t pp_right = pp_low;

    while (1) {
        Int_t npeaks = sp->Search( hist,8,"",0.99-(float) kkk/10);
        if (verbose ) {
            cout << npeaks
                 << " peaks found in the Spatial energy spectrum" << endl;
        }
        // get the peak between E_low and E_up ;
        for (Int_t l=0; l<npeaks; l++) {
            // look for the peak with the highest x value
            if (verbose) {
                cout << " l = " << l <<" " << *(sp->GetPositionX() + l)
                     << "  " << *(sp->GetPositionY() + l) << endl;
            }
            if (  ( *(sp->GetPositionX()+l) > pp_low ) &&
                    ( *(sp->GetPositionX()+l) < pp_up ) &&
                    ( *(sp->GetPositionX()+l) > pp_right) ) {
                if (verbose) {
                    cout << " Found correct peak @ "
                         << *(sp->GetPositionX()+l) << endl;
                }
                pp_right = (*(sp->GetPositionX()+l));
                efound=1;
                if (verbose) {
                    cout << "pp_right = " << pp_right;
                }
            }
        }
        if ((!efound) && (kkk<4)) {
            kkk++;
        } else {
            break;
        }
    }
    if (verbose) {
        cout << " before return pp_right = " << pp_right;
    }
    return(pp_right);
}

int makeplot(
    TH1F *hist[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE],
    Int_t c,
    Int_t f,
    Int_t NRFLOODSTODRAW,
    const std::string & suffix,
    const std::string & filebase)
{

    TCanvas *c1;
    Char_t pngstring[FILENAMELENGTH];

    TString fDIR;

    if (suffix == "Ecom") {
        fDIR.Form("EHIST_COM");
    } else {
        fDIR.Form("EHIST_SPAT");
    }

    // We're going to write to directory fDIR, need to make sure it exists:
    if ( access( fDIR.Data(), 0 ) != 0 ) {
        cout << " Creating dir " << fDIR  << endl;
        if ( mkdir(fDIR, 0777) != 0 ) {
            cout << " Error making directory " << fDIR << endl;
            return(-2);
        }
    }
    c1 = (TCanvas*)gROOT->GetListOfCanvases()->FindObject("c1");
    if (!c1) {
        c1 = new TCanvas("c1","c1",10,10,1000,1000);
    }
    c1->SetCanvasSize(700,700);
    c1->Clear();

    for (Int_t kk = 0;
         kk < TMath::Floor(MODULES_PER_FIN / NRFLOODSTODRAW);
         kk++)
    {
        Int_t i;
        c1->Clear();
        c1->Divide(NRFLOODSTODRAW,APDS_PER_MODULE);
        for (i=kk*NRFLOODSTODRAW; i<NRFLOODSTODRAW*(kk+1); i++) {
            for (Int_t j=0; j<APDS_PER_MODULE; j++) {
                c1->cd((i%NRFLOODSTODRAW)+1+j*NRFLOODSTODRAW);
                hist[c][f][i][j]->Draw();
            }
        }
        sprintf(pngstring,
                "%s/%s.C%dF%dM%d-%d.%s.",
                fDIR.Data(),
                filebase.c_str(),
                c, f, kk*NRFLOODSTODRAW,
                i-1,
                suffix.c_str());
        strcat(pngstring,"png");
        c1->Print(pngstring);
    }
    return(0);
}
