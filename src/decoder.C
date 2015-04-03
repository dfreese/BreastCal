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
#include "chipevent.h"

using namespace std;


#define SHIFTFACCOM 4
#define SHIFTFACSPAT 12
#define BYTESPERCOMMON 12 // ( 4 commons per module * 3 values per common )
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
    cout << " -up [panel_id]: calculate uv centers from pedestals (sine wave must be off)" << endl;
    cout << " -uvo : calculate uv centers only" << endl;
    cout << " -o : optional outputfilename " <<endl;
    cout << " -cmap [DAQBOARD_FILE] : specify which DAQ BOARD nfo file to use rather than the default in $ANADIR/nfo" << endl;
    return;
}

int pedana(double & mean, double & rms, int events, short value)
{
    double val = value;
    // if the value is an outlier (or < 0) , then we reset the value to the current mean.
    // Otherwise I had to include an event array for every channel value

    if (value < 0) {
        val = mean;
    }
    if (events > 10) {
        if (std::abs(val - mean) > 12 * (std::sqrt(rms/double(events)))) {
            val = mean;
        }
    }

    double tmp = mean;
    mean += (val - tmp) / (events);
    rms += (val - mean) * (val - tmp);
    return(0);
}


int main(int argc, char *argv[])
{
    float pedestals[CARTRIDGES_PER_PANEL][RENAS_PER_CARTRIDGE][MODULES_PER_RENA][CHANNELS_PER_MODULE]= {{{{0}}}};
    bool calculate_pedestal_values_flag(false);
    bool calculate_uv_centers_flag(false);
    bool calculate_uv_pedestals_flag(false);
    int calculate_uv_panel_id(0);
    int uvthreshold = -1000;

    bool calculate_uv_centers_only_flag(false);

    Int_t verbose = 0;
    string filename;
    string outfilename;
    string pedfilename;
    string pedvaluefilename;
    int pedfilenamespec=0;


    int outfileset=0;
    int filenamespec=0;
    Int_t sourcepos=0;
    int threshold=DEFAULTTHRESHOLD;
    int nohit_threshold=DEFAULT_NOHIT_THRESHOLD;
    int debugmode=0;


    bool cmapspec = false;
    string nfofile;

    // Arguments not requiring input
    for (int ix = 1; ix < argc; ix++) {
        if (strcmp(argv[ix], "-v") == 0) {
            verbose = 1;
            cout << " Running verbose mode " << endl;
        }
        if (strcmp(argv[ix], "-d") == 0) {
            debugmode = 1;
        }
        if (strcmp(argv[ix], "-uv") == 0) {
            calculate_uv_centers_flag = true;
        }
        if (strcmp(argv[ix], "-uvo") == 0) {
            calculate_uv_centers_flag = true;
            calculate_uv_centers_only_flag = true;
        }
        if (strcmp(argv[ix], "-t") == 0) {
            threshold = atoi(argv[ix+1]);
            ix++;
        }
        if (strcmp(argv[ix], "-p") == 0) {
            calculate_pedestal_values_flag = true;
        }
    }
    // Arguments requiring input
    for (int ix = 1; ix < (argc - 1); ix++) {
        if (strcmp(argv[ix], "-uvt") == 0) {
            uvthreshold = atoi(argv[ix + 1]);
        }
        if (strcmp(argv[ix], "-t") == 0) {
            threshold = atoi(argv[ix + 1]);
        }
        if (strcmp(argv[ix], "-e") == 0) {
            no_events_exclude_beginning = atoi(argv[ix + 1]);
        }
        if (strcmp(argv[ix], "-n") == 0) {
            nohit_threshold = atoi(argv[ix+1]);
        }
        if (strcmp(argv[ix], "-pedfile") == 0) {
            pedfilenamespec = 1;
            pedvaluefilename = string(argv[ix + 1]);
        }
        if (strcmp(argv[ix], "-pos") == 0) {
            sourcepos = atoi(argv[ix + 1]);
        }
        if (strcmp(argv[ix], "-cmap") == 0) {
            cmapspec = true;
            nfofile = string(argv[ix + 1]);
        }
        if (strcmp(argv[ix], "-f") == 0) {
            filename = string(argv[ix + 1]);
            filenamespec = 1;
        }
        if (strcmp(argv[ix], "-o") == 0) {
            outfileset = 1;
            outfilename = string(argv[ix + 1]);
            pedfilename = string(argv[ix + 1]) + ".ped";
        }
        if (strcmp(argv[ix], "-up") == 0) {
            calculate_uv_panel_id = atoi(argv[ix + 1]);
            calculate_pedestal_values_flag = true;
            calculate_uv_pedestals_flag = true;
        }
    }

    if (calculate_pedestal_values_flag) {
        calculate_uv_centers_flag = false;
    }
    if (!pedfilenamespec) {
        calculate_uv_centers_flag = false;
        calculate_uv_centers_only_flag = false;
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
        int read_ped_status(ReadPedestalFile(pedvaluefilename, pedestals));

        if (read_ped_status < 0) {
            cerr << "Error reading Pedestal Value File: "
                 << read_ped_status << endl;
            cerr << "Exiting." << endl;
            return(-3);
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
    int doubletriggers[CARTRIDGES_PER_PANEL][RENAS_PER_CARTRIDGE][MODULES_PER_RENA]= {{{0}}};
    int belowthreshold[CARTRIDGES_PER_PANEL][RENAS_PER_CARTRIDGE][MODULES_PER_RENA]= {{{0}}};
    int totaltriggers[CARTRIDGES_PER_PANEL][RENAS_PER_CARTRIDGE][MODULES_PER_RENA][APDS_PER_MODULE]= {{{{0}}}};

    TTree *mdata=0;

    TVector *uu_c[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE];
    TVector *vv_c[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE];
    Long64_t uventries[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][2]={{{{0}}}};

    for (int c=0; c<CARTRIDGES_PER_PANEL; c++) {
        for (int f=0; f<FINS_PER_CARTRIDGE; f++) {
            uu_c[c][f] = new TVector(APDS_PER_MODULE*MODULES_PER_FIN);
            vv_c[c][f] = new TVector(APDS_PER_MODULE*MODULES_PER_FIN);
        }
    }


    if (pedfilenamespec || calculate_uv_pedestals_flag) {
        if (verbose) {
            cout << " Creating energy histograms " << endl;
        }
        for (int c=0; c<CARTRIDGES_PER_PANEL; c++) {
            for (int f=0; f<FINS_PER_CARTRIDGE; f++) {
                for (int i=0; i<MODULES_PER_FIN; i++) {
                    for (int j=0; j<APDS_PER_MODULE; j++) {
                        char tmpstring[30];
                        char titlestring[50];
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

    TFile * hfile;
    if (pedfilenamespec || debugmode || calculate_uv_pedestals_flag) {
        hfile = new TFile(outfilename.c_str(),"RECREATE");
    }

    chipevent rawevent;
    TTree * rawdata;

    if (debugmode) {
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
    }

    vector<char> packBuffer;

    long byteCounter(0);
    long totalPckCnt(0);
    long droppedIntentional(0);
    long droppedPckCnt(0);
    long droppedFirstLast(0);
    long droppedOutOfRange(0);
    long droppedTrigCode(0);
    long droppedSize(0);

    long total_raw_events(0);

    int pedestals_no_events_skipped(0);
    const int pedestals_no_events_to_skip(20);

    double pedestal_mean[CARTRIDGES_PER_PANEL][RENAS_PER_CARTRIDGE][MODULES_PER_RENA][CHANNELS_PER_MODULE]= {{{{0}}}};
    double pedestal_rms[CARTRIDGES_PER_PANEL][RENAS_PER_CARTRIDGE][MODULES_PER_RENA][CHANNELS_PER_MODULE]= {{{{0}}}};
    int pedestal_events[CARTRIDGES_PER_PANEL][RENAS_PER_CARTRIDGE][MODULES_PER_RENA]= {{{0}}};

    double uv_pedestal_mean[CARTRIDGES_PER_PANEL][RENAS_PER_CARTRIDGE][MODULES_PER_RENA][4]= {{{{0}}}};
    double uv_pedestal_rms[CARTRIDGES_PER_PANEL][RENAS_PER_CARTRIDGE][MODULES_PER_RENA][4]= {{{{0}}}};
    int uv_pedestal_events[CARTRIDGES_PER_PANEL][RENAS_PER_CARTRIDGE][MODULES_PER_RENA]= {{{0}}};

    dataFile.seekg(0, std::ios::end);
    std::streamsize dataFileSize = dataFile.tellg();
    dataFile.seekg(0, std::ios::beg);

    std::vector<char> buffer(dataFileSize);
    if (dataFile.read(buffer.data(), dataFileSize)) {
        cout << "read worked" << endl;
    }
    //dataFile.seekg(0, std::ios::beg);

    char cc;
    std::vector<char>::iterator buffer_iter(buffer.begin());
    //while (dataFile.get(cc)) {
    //    packBuffer.push_back(cc);
    while (buffer_iter != buffer.end()) {
        packBuffer.push_back(*(buffer_iter++));
        byteCounter++;
        //if ((unsigned char)cc==0x81) {
        if ((unsigned char)packBuffer.back()==0x81) {
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

            total_raw_events += packet_info.no_modules_triggered;

            std::vector<chipevent> raw_events;
            int packet_status(PacketToRawEvents(packet_info,
                                                raw_events,
                                                cartridge_id,
                                                sourcepos));

            for (size_t ii = 0; ii < raw_events.size(); ii++) {

                if (calculate_pedestal_values_flag) {
                    if (pedestals_no_events_skipped < pedestals_no_events_to_skip) {
                        pedestals_no_events_skipped++;
                    } else {
                        int chip = raw_events.at(ii).chip;
                        int module = raw_events.at(ii).module;
                        int cartridge = raw_events.at(ii).cartridge;

                        double * mean(pedestal_mean[cartridge][chip][module]);
                        double * rms(pedestal_rms[cartridge][chip][module]);

                        pedestal_events[cartridge][chip][module]++;
                        pedana(mean[0], rms[0], pedestal_events[cartridge][chip][module], raw_events.at(ii).a );
                        pedana(mean[1], rms[1], pedestal_events[cartridge][chip][module], raw_events.at(ii).b );
                        pedana(mean[2], rms[2], pedestal_events[cartridge][chip][module], raw_events.at(ii).c );
                        pedana(mean[3], rms[3], pedestal_events[cartridge][chip][module], raw_events.at(ii).d );
                        pedana(mean[4], rms[4], pedestal_events[cartridge][chip][module], raw_events.at(ii).com1 );
                        pedana(mean[5], rms[5], pedestal_events[cartridge][chip][module], raw_events.at(ii).com1h );
                        pedana(mean[6], rms[6], pedestal_events[cartridge][chip][module], raw_events.at(ii).com2 );
                        pedana(mean[7], rms[7], pedestal_events[cartridge][chip][module], raw_events.at(ii).com2h );

                        if (calculate_uv_pedestals_flag) {
                            mean = uv_pedestal_mean[cartridge][chip][module];
                            rms = uv_pedestal_rms[cartridge][chip][module];
                            uv_pedestal_events[cartridge][chip][module]++;
                            pedana(mean[0], rms[0], uv_pedestal_events[cartridge][chip][module], raw_events.at(ii).u1h);
                            pedana(mean[1], rms[1], uv_pedestal_events[cartridge][chip][module], raw_events.at(ii).v1h);
                            pedana(mean[2], rms[2], uv_pedestal_events[cartridge][chip][module], raw_events.at(ii).u2h);
                            pedana(mean[3], rms[3], uv_pedestal_events[cartridge][chip][module], raw_events.at(ii).v2h);
                        }
                    }
                }

                if (debugmode) {
                    // Add the event to the raw data tree
                    rawevent = raw_events.at(ii);
                    rawdata->Fill();
                }

                if (pedfilenamespec) {
                    ModuleDat processed_event;
                    int process_status(RawEventToModuleDat(raw_events.at(ii),
                                                           processed_event,
                                                           pedestals,
                                                           threshold,
                                                           nohit_threshold,
                                                           panel_id));
                    if (process_status == 0) {
                        // Add the event to the root tree
                        event = &processed_event;

                        totaltriggers[raw_events.at(ii).cartridge]
                                [raw_events.at(ii).chip]
                                [raw_events.at(ii).module]
                                [processed_event.apd]++;

                        // If we're just calculating the uv centers, then skip
                        // over the fill commands.
                        if (!calculate_uv_centers_only_flag) {
                            mdata->Fill();
                            E[raw_events.at(ii).cartridge]
                                    [processed_event.fin]
                                    [processed_event.module]
                                    [processed_event.apd]->Fill(processed_event.E);

                            E_com[raw_events.at(ii).cartridge]
                                    [processed_event.fin]
                                    [processed_event.module]
                                    [processed_event.apd]->Fill(-processed_event.Ec);
                        }

                        if (calculate_uv_centers_flag) {
                            if (processed_event.Ech < uvthreshold) {
                                uventries[processed_event.cartridge]
                                        [processed_event.fin]
                                        [processed_event.module]
                                        [processed_event.apd]++;
                                short uh = (bool) processed_event.apd ?
                                            raw_events.at(ii).u2h : raw_events.at(ii).u1h;
                                short vh = (bool) processed_event.apd ?
                                            raw_events.at(ii).v2h : raw_events.at(ii).v1h;
                                (*uu_c[processed_event.cartridge][processed_event.fin])[processed_event.module*2+processed_event.apd] +=
                                        (Float_t)(uh - (*uu_c[processed_event.cartridge][processed_event.fin])[processed_event.module*2+processed_event.apd]) /
                                        uventries[processed_event.cartridge]
                                                [processed_event.fin]
                                                [processed_event.module]
                                                [processed_event.apd];
                                (*vv_c[processed_event.cartridge][processed_event.fin])[processed_event.module*2+processed_event.apd] +=
                                        (Float_t)(vh - (*vv_c[processed_event.cartridge][processed_event.fin])[processed_event.module*2+processed_event.apd]) /
                                        uventries[processed_event.cartridge]
                                                [processed_event.fin]
                                                [processed_event.module]
                                                [processed_event.apd];
                            }
                        }
                    } else if (process_status == -1) {
                        belowthreshold[raw_events.at(ii).cartridge]
                                [raw_events.at(ii).chip]
                                [raw_events.at(ii).module]++;
                    } else if (process_status == -2) {
                        doubletriggers[raw_events.at(ii).cartridge]
                                [raw_events.at(ii).chip]
                                [raw_events.at(ii).module]++;
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
        int ped_write_status(WritePedestalFile(pedfilename,
                                               pedestal_mean,
                                               pedestal_rms,
                                               pedestal_events));
        if (ped_write_status < 0) {
            cerr << "Error writing out pedestal value file" << endl;
            cerr << "Exiting." << endl;
            return(-4);
        }
    }

    // need to write histograms to disk ::
    // FIXME :: in principle we could fill histograms even without pedestal subtraction
    if (pedfilenamespec || calculate_uv_pedestals_flag) {

        Int_t totaldoubletriggers=0;
        Int_t totalbelowthreshold=0;

        for (int c=0; c<CARTRIDGES_PER_PANEL; c++) {
            for (int f=0; f<FINS_PER_CARTRIDGE; f++) {
                char tmpstring[30];
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

        cout << " Total events :: " << total_raw_events <<  endl;
        cout << " Total accepted :: " << totalacceptedtriggers ;
        cout << setprecision(1) << fixed;
        if (total_raw_events) {
            cout << " (= " << 100* (float) totalacceptedtriggers/total_raw_events << " %) " ;
            cout << " Total double triggers :: " << totaldoubletriggers ;
            cout << " (= " << 100* (float) totaldoubletriggers/total_raw_events << " %) " ;
            cout << " Total below threshold :: " << totalbelowthreshold;
            cout << " (= " << 100* (float) totalbelowthreshold/total_raw_events << " %) " <<endl;
        }

        hfile->cd();
        mdata->Write();
    }

    if (calculate_uv_centers_flag || calculate_uv_pedestals_flag) {
        hfile->cd();
        TDirectory *timing =  hfile->mkdir("timing");
        timing->cd();
        if (calculate_uv_pedestals_flag) {
            for (int cartridge = 0;
                 cartridge < CARTRIDGES_PER_PANEL;
                 cartridge++)
            {
                for (int rena = 0; rena < RENAS_PER_CARTRIDGE; rena++) {
                    for (int local_module = 0;
                         local_module < MODULES_PER_RENA;
                         local_module++)
                    {
                        short fin(0);
                        short module(0);
                        getfinmodule(calculate_uv_panel_id,
                                     rena, local_module,
                                     module, fin);
                        double * mean(uv_pedestal_mean[cartridge]
                                                      [rena]
                                                      [local_module]);
                        (*uu_c[cartridge][fin])[module*2+0] = mean[0];
                        (*vv_c[cartridge][fin])[module*2+0] = mean[1];
                        (*uu_c[cartridge][fin])[module*2+1] = mean[2];
                        (*vv_c[cartridge][fin])[module*2+1] = mean[3];
                    }
                }
            }
        }
        // store uvcenters
        for (int c=0; c<CARTRIDGES_PER_PANEL; c++) {
            for (int f=0; f<FINS_PER_CARTRIDGE; f++) {
                char tmpstring[30];
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
    if (pedfilenamespec || debugmode || calculate_uv_pedestals_flag) {
        hfile->Close();
    }

    if (verbose) {
        cout << " byteCounter = " << byteCounter << endl;
        cout << " totalPckCnt = " << totalPckCnt << endl;
        cout << " droppedPckCnt = " << droppedPckCnt << endl;
    }

    return(0);
}
