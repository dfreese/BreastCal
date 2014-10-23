
/*!
 * \brief Generates plots of the pedestal values for each channel type
 *
 * This macro generates two image files which are graphs of all of the
 * pedestal values for each module scaled the expected range of the given type
 * of channel.
 *
 */

Int_t plot_pedestal(
        string s,
        Int_t cartridge,
        int panel = 0,
        bool debug = false)
{

    TH2F *spatAhisto = new TH2F("spatAhisto","Spatial A Pedestal Width", 16,0,16,8,0,8);
    TH2F *spatBhisto = new TH2F("spatBhisto","Spatial B Pedestal Width", 16,0,16,8,0,8);
    TH2F *spatChisto = new TH2F("spatChisto","Spatial C Pedestal Width", 16,0,16,8,0,8);
    TH2F *spatDhisto = new TH2F("spatDhisto","Spatial D Pedestal Width", 16,0,16,8,0,8);

    TH2F *comL0histo = new TH2F("comL0histo","Common Low Gain APD 0 Pedestal Width", 16,0,16,8,0,8);
    TH2F *comH0histo = new TH2F("comH0histo","Common High Gain APD 0 Pedestal Width", 16,0,16,8,0,8);
    TH2F *comL1histo = new TH2F("comL1histo","Common Low Gain APD 1 Pedestal Width", 16,0,16,8,0,8);
    TH2F *comH1histo = new TH2F("comH1histo","Common High Gain APD 1 Pedestal Width", 16,0,16,8,0,8);

    spatAhisto->SetXTitle("Module Nr");
    spatAhisto->SetYTitle("Fin Nr");

    spatAhisto->SetXTitle("Module Nr");
    spatAhisto->SetYTitle("Fin Nr");

    spatAhisto->SetXTitle("Module Nr");
    spatAhisto->SetYTitle("Fin Nr");

    spatAhisto->SetXTitle("Module Nr");
    spatAhisto->SetYTitle("Fin Nr");

    comL0histo->SetXTitle("Module Nr");
    comL0histo->SetYTitle("Fin Nr");

    comH0histo->SetXTitle("Module Nr");
    comH0histo->SetYTitle("Fin Nr");

    comL1histo->SetXTitle("Module Nr");
    comL1histo->SetYTitle("Fin Nr");

    comH1histo->SetXTitle("Module Nr");
    comH1histo->SetYTitle("Fin Nr");

    cout << " Opening file " << s << endl;

    ifstream infile;
    infile.open(s.c_str());
    Char_t dummy[10];
    Int_t total(0);
    Int_t spata_mean(0);
    Double_t spata_std(0);
    Int_t spatb_mean(0);
    Double_t spatb_std(0);
    Int_t spatc_mean(0);
    Double_t spatc_std(0);
    Int_t spatd_mean(0);
    Double_t spatd_std(0);
    Int_t coml0_mean(0);
    Double_t coml0_std(0);
    Int_t comh0_mean(0);
    Double_t comh0_std(0);
    Int_t coml1_mean(0);
    Double_t coml1_std(0);
    Int_t comh1_mean(0);
    Double_t comh1_std(0);

    Int_t j(0);

    while ( infile >> dummy >> total >> spata_mean >> spata_std >> spatb_mean >>
            spatb_std >> spatc_mean >> spatc_std >> spatd_mean >> spatd_std >>
            coml0_mean >> coml0_std >> comh0_mean >> comh0_std >> coml1_mean >>
            coml1_std >> comh1_mean >> comh1_std)
    {
        j++;
        Int_t finnr(0);
        Int_t modnr(0);
        Int_t chipnr(0);
        Int_t cartridgeId(0);
        Int_t finId(0);
        Int_t unitnr(0);
        sscanf(dummy,"C%dR%dM%d",&cartridgeId,&chipnr,&unitnr);

        Int_t DAQ(TMath::Floor(chipnr/8));

        if (cartridgeId != cartridge) {
            continue;
        }

        finnr = (3-(TMath::Floor((chipnr % 8)/2)))*2;
        if (DAQ >= 2) {
            if (panel == 0) {
                finnr++;
            }
        } else {
            if (panel == 1) {
                finnr++;
            }
        }

        modnr = unitnr + 4*(chipnr % 2);
        if (DAQ % 2) {
            modnr += 8;
        }

        if (panel == 1 ) {
            modnr = 15 - modnr;
        }

        if (debug) { 
            cout << dummy << " DAQ BOARD : " << DAQ << " RENA : " << chipnr << " FIN : " << finnr << " MODULE : " << modnr << endl; 
        }

        spatAhisto->SetBinContent( modnr+1, finnr+1, spata_std);
        spatBhisto->SetBinContent( modnr+1, finnr+1, spatb_std);
        spatChisto->SetBinContent( modnr+1, finnr+1, spatc_std);
        spatDhisto->SetBinContent( modnr+1, finnr+1, spatd_std);
        comL0histo->SetBinContent( modnr+1, finnr+1, coml0_std);
        comH0histo->SetBinContent( modnr+1, finnr+1, comh0_std);
        comL1histo->SetBinContent( modnr+1, finnr+1, coml1_std);
        comH1histo->SetBinContent( modnr+1, finnr+1, comh1_std);
    }

    int spatial_max = 10;
    int spatial_min = 1;
    float lowgain_max = 7.5;
    int lowgain_min = 1;
    int highgain_max = 16;
    int highgain_min = 1;

    spatAhisto->SetMaximum(spatial_max);
    spatAhisto->SetMinimum(spatial_min);
    spatBhisto->SetMaximum(spatial_max);
    spatBhisto->SetMinimum(spatial_min);
    spatChisto->SetMaximum(spatial_max);
    spatChisto->SetMinimum(spatial_min);
    spatDhisto->SetMaximum(spatial_max);
    spatDhisto->SetMinimum(spatial_min);

    comL0histo->SetMaximum(lowgain_max);
    comL0histo->SetMinimum(lowgain_min);
    comL1histo->SetMaximum(lowgain_max);
    comL1histo->SetMinimum(lowgain_min);
    comH0histo->SetMaximum(highgain_max);
    comH0histo->SetMinimum(highgain_min);
    comH1histo->SetMaximum(highgain_max);
    comH1histo->SetMinimum(highgain_min);


    Double_t levels[18];
    for (Int_t i=0;i<18;i++) {
        levels[i]=1+i*0.5;
    }
    spatAhisto->SetContour(18,levels);
    spatBhisto->SetContour(18,levels);
    spatChisto->SetContour(18,levels);
    spatDhisto->SetContour(18,levels);


    Double_t low_gain_levels[18];
    for (Int_t i=0;i<18;i++) {
        low_gain_levels[i]=1+i*0.5;
    }
    comL0histo->SetContour(13,low_gain_levels);
    comL1histo->SetContour(13,low_gain_levels);


    Double_t high_gain_levels[30];
    for (Int_t i=0;i<30;i++) {
        high_gain_levels[i]=1+i*0.5;
    }
    comH0histo->SetContour(30,high_gain_levels);
    comH0histo->SetContour(30,high_gain_levels);

    if (debug) {
        cout << " We read " << j << " lines " << endl;
    }
    infile.close();


    TCanvas *c1 = new TCanvas("c1","Spatial Pedestals",400,800);
    c1->Divide(1,4);
    c1->cd(1);
    spatAhisto->Draw("colz");
    c1->cd(2);
    spatBhisto->Draw("colz");
    c1->cd(3);
    spatChisto->Draw("colz");
    c1->cd(4);
    spatDhisto->Draw("colz");

    TCanvas *c2 = new TCanvas("c2","Common Pedestals",400,800);
    c2->Divide(1,4);
    c2->cd(1);
    comL0histo->Draw("colz");
    c2->cd(2);
    comH0histo->Draw("colz");
    c2->cd(3);
    comL1histo->Draw("colz");
    c2->cd(4);
    comH1histo->Draw("colz");


    TString outputfilename;

    outputfilename.Form("spatial_pedestals_P%dC%d.png", panel, cartridge);
    c1->Print(outputfilename.Data());

    outputfilename.Form("common_pedestals_P%dC%d.png", panel, cartridge);
    c2->Print(outputfilename.Data());

    delete spatAhisto;
    delete spatBhisto;
    delete spatChisto;
    delete spatDhisto;

    delete comL0histo;
    delete comL1histo;
    delete comH0histo;
    delete comH1histo;

    delete c1;
    delete c2;

}
