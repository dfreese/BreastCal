#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include "Riostream.h"
#include "TMath.h"
#include "TVector.h"
#include "decoder.h"
#include "ModuleDat.h"
#include "daqboardmap.h"
#include "daqpacket.h"
#include "decoderlib.h"

#define FILENAMELENGTH 120


using namespace std;


#define SHIFTFACCOM 4
#define SHIFTFACSPAT 12
#define BYTESPERCOMMON 12 // 4 * 3 = 12 ( 4 commons per module, 3 values per common )
#define VALUESPERSPATIAL 4

// This the number of events that is excluded at the beginning of the file
int no_events_exclude_beginning(0);

void usage(void)
{
    int t=DEFAULTTHRESHOLD;
    int tnohit=DEFAULT_NOHIT_THRESHOLD;
    cout << " decode  -f [filename] [-v -o [outputfilename] -p  -d -t [threshold] " ;
    cout << " -uv -pedfile [pedfilename] ]" <<endl;
    cout << " -d : debug mode, a tree with unparsed data will be written (non-pedestal corrected). " <<endl;
    cout << "      you get access to u,v" << endl;
    cout << " -t : threshold for hits (trigger threshold), default = " << t << endl;
    cout << " -e : number of events to exclude at beginning default =  " << no_events_exclude_beginning << endl;
    cout << " -n : no hit threshold in other APD on same module, default = " << tnohit << endl;
    cout << " -uv: UV circle centers will be calculated " << endl;
    cout << " -uvt: UV circle centers threshold ( for pulser applications, NEGATIVE value ) " << endl;
    cout << " -pedfile [pedfilename] : pedestal file name " << endl;
    cout << " -p : calculate pedestals " << endl;
    cout << " -o : optional outputfilename " <<endl;
    cout << " -cmap [DAQBOARD_FILE] : specify which DAQ BOARD nfo file to use rather than the default in $ANADIR/nfo" << endl;
    return;
}

void getfinmodule(Short_t panel, Short_t chip, Short_t &module, Short_t &fin)
{
    Short_t fourup=TMath::Floor(chip/8);
    Short_t localchip=chip%8;
    fin = 6-2*TMath::Floor(localchip/2);
    if (panel) {
        if (chip < 16) {
            fin++;
        }
        module=(3-module)%4;
        if (!(chip%2)) {
            module+=MODULES_PER_RENA;
        }
        if (!(fourup%2)) {
            module+=(MODULES_PER_FIN/2 );
        }
    } else {
        if (chip>=16) {
            fin++;
        }
        if (chip%2) {
            module+=MODULES_PER_RENA;
        }
        if (fourup%2) {
            module+=(MODULES_PER_FIN/2);
        }
    }
}

int pedana(double * mean, double * rms, int * events, Short_t value)
{
    double val = value;
    // if the value is an outlier (or < 0) , then we reset the value to the current mean.
    // Otherwise I had to include an event array for every channel value

    if (value < 0) {
        val=*mean;
    }
    if (*events > 10) {
        if (TMath::Abs(value - *mean) > 12 * (TMath::Sqrt(*rms / *events))) {
            val=*mean;
        }
    }

    double tmp = *mean;
    *mean += (val - tmp) / (*events);
    *rms += (val - *mean) * (val - tmp);
    return(0);
}


int main(int argc, char *argv[])
{
    float pedestals[CARTRIDGES_PER_PANEL][RENAS_PER_CARTRIDGE][MODULES_PER_RENA][CHANNELS_PER_MODULE]= {{{{0}}}};
    bool calculate_pedestal_values_flag(false);
    bool calculate_uv_centers_flag(false);
    int uvthreshold = -1000;

    TFile *hfile;

    Int_t verbose = 0;
    string filename;
    string outfilename;
    string pedfilename;
    string pedvaluefilename;
    int pedfilenamespec=0;

    ofstream pedfile;
    int outfileset=0;
    int filenamespec=0;
    Int_t sourcepos=0;
    int threshold=DEFAULTTHRESHOLD;
    int nohit_threshold=DEFAULT_NOHIT_THRESHOLD;
    int debugmode=0;


    bool cmapspec = false;
    string nfofile;

    for (int ix=1; ix< argc; ix++) {
        if (strncmp(argv[ix], "-v", 2) == 0) {
            verbose = 1;
            cout << " Running verbose mode " << endl;
        }

        if (strncmp(argv[ix], "-d", 2) == 0) {
            debugmode = 1;
        }

        if (strncmp(argv[ix], "-uv", 3) == 0) {
            if (strncmp(argv[ix], "-uvt", 4) == 0) {
                uvthreshold = atoi(argv[ix+1]);
                ix++;
            } else {
                calculate_uv_centers_flag = true;
            }
        }

        if (strncmp(argv[ix], "-t", 2) == 0) {
            threshold = atoi(argv[ix+1]);
            ix++;
        }

        if (strncmp(argv[ix], "-e", 2) == 0) {
            no_events_exclude_beginning = atoi(argv[ix+1]);
            ix++;
        }

        if (strncmp(argv[ix], "-n", 2) == 0) {
            nohit_threshold = atoi(argv[ix+1]);
            ix++;
        }

        if (strncmp(argv[ix], "-p", 2) == 0) {
            if (strncmp(argv[ix], "-pedfile", 8) == 0) {
                pedfilenamespec=1;
                pedvaluefilename = string(argv[ix + 1]);
                ix++;
            } else if (strncmp(argv[ix],"-pos",4) == 0) {
                sourcepos=atoi(argv[ix+1]);
                ix++;
            } else {
                calculate_pedestal_values_flag = true;
            }
        }

        if (strncmp(argv[ix], "-cmap", 5) == 0) {
            cmapspec = true;
            nfofile = string(argv[ix+1]);
        }

        if (strncmp(argv[ix], "-f", 2) == 0) {
            filename = string(argv[ix+1]);
            filenamespec = 1;
        }

        if (strncmp(argv[ix], "-o", 2) == 0) {
            outfileset = 1;
            outfilename = string(argv[ix+1]);
            pedfilename = string(argv[ix+1]) + ".ped";
        }
    }

    if (calculate_pedestal_values_flag) {
        calculate_uv_centers_flag = 0;
    }

    if (!(filenamespec)) {
        printf("Please Specify Filename !!\n");
        usage();
        return(-1);
    }

    if (!cmapspec) {
        string libpath = string(getenv("ANADIR"));
        cout << " Loading Shared Library from " << libpath << endl;
        cout << " (note ANADIR = " << getenv("ANADIR") << " )" << endl;
        nfofile = libpath + "/nfo/DAQ_Board_Map.nfo";
    }
    cout <<" Using Cartridge map file " << nfofile << endl;

    DaqBoardMap daq_board_map;
    int daq_board_map_status(daq_board_map.loadMap(nfofile));

    if (daq_board_map_status < 0) {
        cerr << "Error: failed to load DAQ Board map: " << nfofile << endl;
        cerr << "Exiting." << endl;
        return(-2);
    }

    if (pedfilenamespec) {
        ifstream pedvals;
        int events;
        double tmp;
        Char_t idstring[12]; // form C#R#M#
        pedvals.open(pedvaluefilename.c_str());
        if (!pedvals) {
            cout << " Error opening file "
                 << pedvaluefilename << "\n.Exiting.\n";
            return(-10);
        }
        cout << " Decoding file " << filename
             << " with pedestals " << pedvaluefilename << endl;
        while (pedvals >> idstring) {
            int cartridge;
            int chip;
            int module;
            sscanf(idstring,"C%dR%dM%d",&cartridge,&chip,&module);
            pedvals >> events;
            if ((chip >= RENAS_PER_CARTRIDGE) ||
                    (module >= MODULES_PER_RENA) ||
                    (cartridge >= CARTRIDGES_PER_PANEL))
            {
                cout << " Error reading pedfile, module or chipnumber too high: " << endl;
                cout << " module = " << module << ", chip = " << chip << ".\nExiting." << endl;
                return(-2);
            }
            for (int ii=0; ii< CHANNELS_PER_MODULE; ii++) {
                pedvals >> pedestals[cartridge][chip][module][ii] ;
                pedvals >> tmp;
            }
        }

        if (verbose) {
            cout << " Pedestal values :: " << endl;
            for (int c=0; c<CARTRIDGES_PER_PANEL; c++) {
                for (int r=0; r<RENAS_PER_CARTRIDGE; r++) {
                    for (int i=0; i<MODULES_PER_RENA; i++) {
                        for (int k=0; k<CHANNELS_PER_MODULE; k++) {
                            cout << pedestals[c][r][i][k] << " " ;
                        }
                        cout << endl;
                    }
                }
            }
        }
    } else {
        cout << " Decoding file " << filename << " without pedestals " << endl;
    }

    if (verbose) {
        cout << " threshold :: " << threshold << endl;
    }

    if (!calculate_uv_centers_flag) {
        cout << " ======================================================================== " << endl;
        cout << " |  Warning you did not enable UV calc, this will affect chain_parsed ! | " << endl;
        cout << " ======================================================================== " << endl;
    }

    if (!(outfileset)) {
        outfilename = filename + ".root";
        pedfilename = filename + ".ped";
    }

    ModuleDat *event = new ModuleDat();
    Int_t doubletriggers[CARTRIDGES_PER_PANEL][RENAS_PER_CARTRIDGE][MODULES_PER_RENA]= {{{0}}};
    Int_t belowthreshold[CARTRIDGES_PER_PANEL][RENAS_PER_CARTRIDGE][MODULES_PER_RENA]= {{{0}}};
    Int_t totaltriggers[CARTRIDGES_PER_PANEL][RENAS_PER_CARTRIDGE][MODULES_PER_RENA][APDS_PER_MODULE]= {{{{0}}}};

    TTree *mdata=0;

    for (int c=0; c<CARTRIDGES_PER_PANEL; c++) {
        for (int f=0; f<FINS_PER_CARTRIDGE; f++) {
            uu_c[c][f] = new TVector(APDS_PER_MODULE*MODULES_PER_FIN);
            vv_c[c][f] = new TVector(APDS_PER_MODULE*MODULES_PER_FIN);
        }
    }


    Char_t tmpstring[30];

    if (pedfilenamespec) {
        Char_t titlestring[50];
        if (verbose) {
            cout << " Creating energy histograms " << endl;
        }
        for (int c=0; c<CARTRIDGES_PER_PANEL; c++) {
            for (int f=0; f<FINS_PER_CARTRIDGE; f++) {
                for (int i=0; i<MODULES_PER_FIN; i++) {
                    for (int j=0; j<APDS_PER_MODULE; j++) {
                        sprintf(tmpstring,"E[%d][%d][%d][%d]",c,f,i,j);
                        sprintf(titlestring,"E C%dF%d, Module %d, PSAPD %d ",c,f,i,j);
                        E[c][f][i][j]=new TH1F(tmpstring,titlestring,Ebins,E_low,E_up);
                        sprintf(tmpstring,"E_com[%d][%d][%d][%d]",c,f,i,j);
                        sprintf(titlestring,"ECOM C%dF%d, Module %d, PSAPD %d",c,f,i,j);
                        E_com[c][f][i][j]=new TH1F(tmpstring,titlestring,Ebins_com,E_low_com,E_up_com);
                    }
                }
            }
        }

        if (verbose) {
            cout << " Creating tree " << endl;
        }
        mdata = new TTree("mdata","Converted RENA data");
        mdata->Branch("eventdata",&event);
    }

    ifstream dataFile;
    dataFile.open(filename.c_str(), ios::in | ios::binary);
    if (!dataFile.good()) {
        cerr << "Cannot open file \"" << filename
             << "\" for read operation." << endl;
        cerr << "Exiting." << endl;
        return(-1);
    }
    hfile = new TFile(outfilename.c_str(),"RECREATE");


    chipevent rawevent;
    TTree *rawdata;
    rawdata =  new TTree("rawdata", "Converted Raw RENA data");
    if (!debugmode) {
        rawdata->SetDirectory(0);
    }
    rawdata->Branch("ct",&rawevent.ct,"ct/L");
    rawdata->Branch("chip",&rawevent.chip,"chip/S");
    rawdata->Branch("cartridge",&rawevent.cartridge,"cartridge/S");
    rawdata->Branch("module",&rawevent.module,"module/S");
    rawdata->Branch("com1",&rawevent.com1,"com1/S");
    rawdata->Branch("com2",&rawevent.com2,"com2/S");
    rawdata->Branch("com1h",&rawevent.com1h,"com1h/S");
    rawdata->Branch("com2h",&rawevent.com2h,"com2h/S");
    rawdata->Branch("u1",&rawevent.u1,"u1/S");
    rawdata->Branch("v1",&rawevent.v1,"v1/S");
    rawdata->Branch("u2",&rawevent.u2,"u2/S");
    rawdata->Branch("v2",&rawevent.v2,"v2/S");
    rawdata->Branch("u1h",&rawevent.u1h,"u1h/S");
    rawdata->Branch("v1h",&rawevent.v1h,"v1h/S");
    rawdata->Branch("u2h",&rawevent.u2h,"u2h/S");
    rawdata->Branch("v2h",&rawevent.v2h,"v2h/S");
    rawdata->Branch("a",&rawevent.a,"a/S");
    rawdata->Branch("b",&rawevent.b,"b/S");
    rawdata->Branch("c",&rawevent.c,"c/S");
    rawdata->Branch("d",&rawevent.d,"d/S");
    rawdata->Branch("pos",&rawevent.pos,"pos/I");

    vector<char> packBuffer;

    long byteCounter(0);
    long totalPckCnt(0);
    long droppedIntentional(0);
    long droppedPckCnt(0);
    long droppedFirstLast(0);
    long droppedOutOfRange(0);
    long droppedTrigCode(0);
    long droppedSize(0);

    char cc;
    while (dataFile.get(cc)) {
        packBuffer.push_back(cc);
        byteCounter++;
        if ((unsigned char)cc==0x81) {
            // end of package (start processing)
            totalPckCnt++;

            if (droppedIntentional < no_events_exclude_beginning) {
                droppedIntentional++;
                packBuffer.clear();
                continue;
            }

            DaqPacket packet_info;
            int decode_status(DecodePacketByteStream(packBuffer, packet_info));

            if (decode_status < 0) {
                if (decode_status == -1) {
                    droppedFirstLast++;
                    if (verbose) {
                        cout << "Dropped: Empty Packet Vector" << endl;
                    }
                } else if (decode_status == -2){
                    droppedFirstLast++;
                    if (verbose) {
                        cout << "Dropped: First Byte not 0x80" << endl;
                    }
                } else if (decode_status == -3) {
                    droppedTrigCode++;
                    if (verbose) {
                        cout << "Dropped: Trigger Code = 0" << endl;
                    }
                } else if (decode_status == -4) {
                    droppedSize++;
                    if (verbose) {
                        cout << "Dropped: Packet Size Wrong - "
                             << packBuffer.size() << endl;
                    }
                }
                droppedPckCnt++;
                packBuffer.clear();
                continue;
            }

            int panel_id;
            int cartridge_id;
            int address_status(daq_board_map.getPanelCartridgeNumber(packet_info.backend_address,
                                                                     panel_id,
                                                                     cartridge_id));

            if (address_status < 0) {
                droppedPckCnt++;
                droppedOutOfRange++;
                if (verbose) {
                    cout << "Dropped: Invalid Address Byte" << endl;
                }
                packBuffer.clear();
                continue;
            }

            int kk=0;
            for (int module = 0; module < 4; module++) {
                if (packet_info.module_trigger_flags[module]) {

                    int chipId = packet_info.rena + 2 * packet_info.fpga
                                 + packet_info.daq_board * RENAS_PER_FOURUPBOARD;

                    rawevent.ct = packet_info.timestamp;
                    rawevent.chip = chipId;
                    rawevent.cartridge = cartridge_id;
                    rawevent.module = module;

                    int even = chipId % 2;

                    int channel_offset = kk * BYTESPERCOMMON
                                         + even *
                                           packet_info.no_modules_triggered *
                                           SHIFTFACCOM;

                    int spatial_offset = BYTESPERCOMMON *
                                         packet_info.no_modules_triggered
                                         + kk * VALUESPERSPATIAL
                                         - even *
                                           packet_info.no_modules_triggered *
                                           SHIFTFACSPAT;

                    rawevent.com1h = packet_info.adc_values[0 + channel_offset];
                    rawevent.u1h = packet_info.adc_values[1 + channel_offset];
                    rawevent.v1h = packet_info.adc_values[2 + channel_offset];
                    rawevent.com1 = packet_info.adc_values[3 + channel_offset];
                    rawevent.u1 = packet_info.adc_values[4 + channel_offset];
                    rawevent.v1 = packet_info.adc_values[5 + channel_offset];
                    rawevent.com2h = packet_info.adc_values[6 + channel_offset];
                    rawevent.u2h = packet_info.adc_values[7 + channel_offset];
                    rawevent.v2h = packet_info.adc_values[8 + channel_offset];
                    rawevent.com2 = packet_info.adc_values[9 + channel_offset];
                    rawevent.u2 = packet_info.adc_values[10 + channel_offset];
                    rawevent.v2 = packet_info.adc_values[11 + channel_offset];

                    rawevent.a = packet_info.adc_values[0 + spatial_offset];
                    rawevent.b = packet_info.adc_values[1 + spatial_offset];
                    rawevent.c = packet_info.adc_values[2 + spatial_offset];
                    rawevent.d = packet_info.adc_values[3 + spatial_offset];
                    rawevent.pos=sourcepos;

                    kk++;
                    rawdata->Fill();

                    if (pedfilenamespec) {
                        event->module=module;
                        event->ct = packet_info.timestamp;
                        event->chip=rawevent.chip;
                        event->cartridge=rawevent.cartridge;
                        event->a=rawevent.a - pedestals[event->cartridge][chipId][module][0];
                        event->b=rawevent.b - pedestals[event->cartridge][chipId][module][1];
                        event->c=rawevent.c - pedestals[event->cartridge][chipId][module][2];
                        event->d=rawevent.d - pedestals[event->cartridge][chipId][module][3];

                        getfinmodule(panel_id,
                                     event->chip,
                                     event->module,
                                     event->fin);

                        event->E = event->a + event->b + event->c + event->d;
                        event->x = event->c + event->d - (event->b + event->a);
                        event->y = event->a + event->d - (event->b + event->c);

                        event->x /= event->E;
                        event->y /= event->E;

                        event->apd = -1;
                        event->id = -1;
                        event->pos = rawevent.pos;

                        if ((rawevent.com1h - pedestals[event->cartridge][chipId][module][5]) < threshold) {
                            if ((rawevent.com2h - pedestals[event->cartridge][chipId][module][7]) > nohit_threshold) {
                                totaltriggers[event->cartridge][chipId][module][0]++;
                                event->apd = 0;
                                event->ft = (((rawevent.u1h & 0xFFFF) << 16) | (rawevent.v1h & 0xFFFF));
                                event->Ec = rawevent.com1 - pedestals[event->cartridge][chipId][module][4];
                                event->Ech = rawevent.com1h - pedestals[event->cartridge][chipId][module][5];
                                mdata->Fill();
                            } else {
                                doubletriggers[event->cartridge][chipId][module]++;
                            }
                        } else if ((rawevent.com2h - pedestals[event->cartridge][chipId][module][7]) < threshold ) {
                            if ( (rawevent.com1h - pedestals[event->cartridge][chipId][module][5]) > nohit_threshold ) {
                                totaltriggers[event->cartridge][chipId][module][1]++;
                                event->apd = 1;
                                event->y *= -1;
                                event->ft = (((rawevent.u2h & 0xFFFF) << 16) | (rawevent.v2h & 0xFFFF));
                                event->Ec = rawevent.com2 - pedestals[event->cartridge][chipId][module][6];
                                event->Ech = rawevent.com2h- pedestals[event->cartridge][chipId][module][7];
                                mdata->Fill();
                            } else {
                                doubletriggers[event->cartridge][chipId][module]++;
                            }
                        } else {
                            belowthreshold[event->cartridge][chipId][module]++;
                        }
                        if ((event->apd == 1) || (event->apd == 0)) {
                            E[event->cartridge][event->fin][event->module][event->apd]->Fill(event->E);
                            E_com[event->cartridge][event->fin][event->module][event->apd]->Fill(-event->Ec);
                        }
                    }

                    if (calculate_uv_centers_flag) {
                        if ((event->apd == 1) || (event->apd == 0)) {
                            if ((rawevent.com1h - pedestals[cartridge_id][chipId][module][5]) < uvthreshold) {
                                uventries[cartridge_id][event->fin][event->module][0]++;
                                (*uu_c[cartridge_id][event->fin])[event->module*2+0]+=(Float_t)(rawevent.u1h- (*uu_c[cartridge_id][event->fin])[event->module*2+0])/uventries[cartridge_id][event->fin][event->module][0];
                                (*vv_c[cartridge_id][event->fin])[event->module*2+0]+=(Float_t)(rawevent.v1h- (*vv_c[cartridge_id][event->fin])[event->module*2+0])/uventries[cartridge_id][event->fin][event->module][0];
                            }
                            if (( rawevent.com2h - pedestals[cartridge_id][chipId][module][7] ) < uvthreshold ) {
                                uventries[cartridge_id][event->fin][event->module][1]++;
                                (*uu_c[cartridge_id][event->fin])[event->module*2+1]+=(Float_t)(rawevent.u2h- (*uu_c[cartridge_id][event->fin])[event->module*2+1])/uventries[cartridge_id][event->fin][event->module][1];
                                (*vv_c[cartridge_id][event->fin])[event->module*2+1]+=(Float_t)(rawevent.v2h- (*vv_c[cartridge_id][event->fin])[event->module*2+1])/uventries[cartridge_id][event->fin][event->module][1];
                            }
                        }
                    }
                }
            }
            packBuffer.clear();
        }
    }

    if (totalPckCnt) {
        cout << "File Processed.\n"
             << "Dropped: " << droppedPckCnt << " out of "
             << totalPckCnt << " (" << setprecision(2)
             << 100*droppedPckCnt/totalPckCnt <<"%)." << endl;
    } else {
        cout << " File Processed. No packets found." << endl;
    }
    cout << setprecision(1) << fixed;

    if (droppedPckCnt) {
        cout << "Start Code: "
             << (float) 100 * droppedFirstLast / droppedPckCnt << "% ";
        cout << "Out of Range: "
             << (float) 100 * droppedOutOfRange / droppedPckCnt << "% ";
        cout << "Tigger Code: "
             << (float) 100 * droppedTrigCode / droppedPckCnt << "% ";
        cout << "Packet Size: "
             << (float) 100 * droppedSize / droppedPckCnt << "% " << endl;
    }

    if (calculate_uv_centers_flag) {
        if (verbose) {
            cout <<  " Averaging the circle Centers " << endl;
        }

        if (verbose) {
            for (int c=0; c<CARTRIDGES_PER_PANEL; c++) {
                for (int f=0; f<FINS_PER_CARTRIDGE; f++) {
                    for (int i=0; i<MODULES_PER_FIN; i++) {
                        for (int j=0; j<APDS_PER_MODULE; j++) {
                            cout << " Circle Center Cartridge " << c
                                 << " Fin "<< f << " Module " << i
                                 << " APD " << j << ": "  ;
                            cout << "uu_c = " << (*uu_c[c][f])[i*2+j]
                                 << " vv_c = " << (*vv_c[c][f])[i*2+j]
                                 << " nn_entries = " << uventries[c][f][i][j]
                                 << endl;
                        }
                    }
                }
            }
        }
    }

    if (calculate_pedestal_values_flag) {
        pedfile.open(pedfilename.c_str());
        if (verbose) {
            cout << "Calculating pedestal values "  << endl;
        }

        double pedestal_mean[CARTRIDGES_PER_PANEL][RENAS_PER_CARTRIDGE][MODULES_PER_RENA][CHANNELS_PER_MODULE]= {{{{0}}}};
        double pedestal_rms[CARTRIDGES_PER_PANEL][RENAS_PER_CARTRIDGE][MODULES_PER_RENA][CHANNELS_PER_MODULE]= {{{{0}}}};
        int pedestal_events[CARTRIDGES_PER_PANEL][RENAS_PER_CARTRIDGE][MODULES_PER_RENA]= {{{0}}};

        // skip first 20 entries to prevent outliers
        for (int ii = 20; ii < rawdata->GetEntries(); ii++) {
            rawdata->GetEntry(ii);

            int chip = rawevent.chip;
            int module = rawevent.module;
            int cartridge = rawevent.cartridge;

            pedestal_events[cartridge][chip][module]++;
            pedana(&pedestal_mean[cartridge][chip][module][0], &pedestal_rms[cartridge][chip][module][0], &pedestal_events[cartridge][chip][module], rawevent.a );
            pedana(&pedestal_mean[cartridge][chip][module][1], &pedestal_rms[cartridge][chip][module][1], &pedestal_events[cartridge][chip][module], rawevent.b );
            pedana(&pedestal_mean[cartridge][chip][module][2], &pedestal_rms[cartridge][chip][module][2], &pedestal_events[cartridge][chip][module], rawevent.c );
            pedana(&pedestal_mean[cartridge][chip][module][3], &pedestal_rms[cartridge][chip][module][3], &pedestal_events[cartridge][chip][module], rawevent.d );
            pedana(&pedestal_mean[cartridge][chip][module][4], &pedestal_rms[cartridge][chip][module][4], &pedestal_events[cartridge][chip][module], rawevent.com1 );
            pedana(&pedestal_mean[cartridge][chip][module][5], &pedestal_rms[cartridge][chip][module][5], &pedestal_events[cartridge][chip][module], rawevent.com1h );
            pedana(&pedestal_mean[cartridge][chip][module][6], &pedestal_rms[cartridge][chip][module][6], &pedestal_events[cartridge][chip][module], rawevent.com2 );
            pedana(&pedestal_mean[cartridge][chip][module][7], &pedestal_rms[cartridge][chip][module][7], &pedestal_events[cartridge][chip][module], rawevent.com2h );
        }

        if (verbose) {
            cout << " pedana done. " << endl;
        }

        for (int c=0; c<CARTRIDGES_PER_PANEL; c++) {
            for (int r=0; r<RENAS_PER_CARTRIDGE; r++) {
                for (int i=0; i<MODULES_PER_RENA; i++) {
                    pedfile << std::setfill('0');
                    pedfile << "C" << std::setw(1) << c
                            << "R" << std::setw(3) << r
                            << "M" << std::setw(1) << i;
                    pedfile << std::setfill(' ') << std::setw(9)
                            << pedestal_events[c][r][i];
                    for (int jjj=0; jjj<CHANNELS_PER_MODULE; jjj++) {
                        pedfile << std::setprecision(0) << std::fixed << std::setw(7);
                        pedfile << pedestal_mean[c][r][i][jjj];
                        pedfile <<  std::setprecision(2) << std::fixed << std::setw(8);
                        if (pedestal_events[c][r][i]) {
                            pedfile << TMath::Sqrt(pedestal_rms[c][r][i][jjj]/pedestal_events[c][r][i]);
                        } else {
                            pedfile << 0;
                        }
                    }
                    pedfile << endl;
                }
            }
        }
        pedfile.close();
    }

    // need to write histograms to disk ::
    // FIXME :: in principle we could fill histograms even without pedestal subtraction
    if (pedfilenamespec) {

        Int_t totaldoubletriggers=0;
        Int_t totalbelowthreshold=0;

        for (int c=0; c<CARTRIDGES_PER_PANEL; c++) {
            for (int f=0; f<FINS_PER_CARTRIDGE; f++) {
                sprintf(tmpstring,"C%dF%d",c,f);
                subdir[c][f] = hfile->mkdir(tmpstring);
                subdir[c][f]->cd();
                for (int m=0; m<MODULES_PER_FIN; m++) {
                    for (int j=0; j<APDS_PER_MODULE; j++) {
                        E[c][f][m][j]->Write();
                        E_com[c][f][m][j]->Write();
                    }
                }
            }
        }

        cout << "========== Double Triggers =============== " << endl;
        for (int c=0; c<CARTRIDGES_PER_PANEL; c++) {
            for (int r=0; r<RENAS_PER_CARTRIDGE; r++) {
                for (int i=0; i<MODULES_PER_RENA; i++ ) {
                    cout << doubletriggers[c][r][i] << " " ;
                    totaldoubletriggers+=doubletriggers[c][r][i];
                    totalbelowthreshold+=belowthreshold[c][r][i];
                } // i
                cout << endl;
            } //r
        } //c

        cout << "========== Total Triggers =============== " << endl;
        Long64_t totalmoduletriggers[CARTRIDGES_PER_PANEL][RENAS_PER_CARTRIDGE][MODULES_PER_RENA]= {{{0}}};
        Long64_t totalchiptriggers[CARTRIDGES_PER_PANEL][RENAS_PER_CARTRIDGE]= {{0}};
        Long64_t totalacceptedtriggers=0;
        for (int c=0; c<CARTRIDGES_PER_PANEL; c++) {
            for (int r=0; r<RENAS_PER_CARTRIDGE; r++) {
                for (int i=0; i<MODULES_PER_RENA; i++ ) {
                    cout << totaltriggers[c][r][i][0] << " " << totaltriggers[c][r][i][1] << " ";
                    totalmoduletriggers[c][r][i]=totaltriggers[c][r][i][0]+totaltriggers[c][r][i][1];
                    cout << totalmoduletriggers[c][r][i] << " " ;
                    totalchiptriggers[c][r]+=totalmoduletriggers[c][r][i];
                }
                cout << " || " << totalchiptriggers[c][r]  << endl;
                totalacceptedtriggers+=totalchiptriggers[c][r]    ;
            }
        }

        cout << " Total events :: " << rawdata->GetEntries() <<  endl;
        cout << " Total accepted :: " << totalacceptedtriggers ;
        cout << setprecision(1) << fixed;
        if (rawdata->GetEntries()) {
            cout << " (= " << 100* (float) totalacceptedtriggers/rawdata->GetEntries() << " %) " ;
            cout << " Total double triggers :: " << totaldoubletriggers ;
            cout << " (= " << 100* (float) totaldoubletriggers/rawdata->GetEntries() << " %) " ;
            cout << " Total below threshold :: " << totalbelowthreshold;
            cout << " (= " << 100* (float) totalbelowthreshold/rawdata->GetEntries() << " %) " <<endl;
        }

        hfile->cd();
        mdata->Write();
    }

    if (calculate_uv_centers_flag) {
        hfile->cd();
        TDirectory *timing =  hfile->mkdir("timing");
        timing->cd();
        // store uvcenters
        for (int c=0; c<CARTRIDGES_PER_PANEL; c++) {
            for (int f=0; f<FINS_PER_CARTRIDGE; f++) {
                sprintf(tmpstring,"uu_c[%d][%d]",c,f);
                uu_c[c][f]->Write(tmpstring);
                sprintf(tmpstring,"vv_c[%d][%d]",c,f);
                vv_c[c][f]->Write(tmpstring);
            }
        }
    }

    if (debugmode) {
        hfile->cd();
        rawdata->Write();
    }
    hfile->Close();
    if (verbose) {
        cout << " byteCounter = " << byteCounter << endl;
        cout << " totalPckCnt = " << totalPckCnt << endl;
        cout << " droppedPckCnt = " << droppedPckCnt << endl;
    }

    return(0);
}
