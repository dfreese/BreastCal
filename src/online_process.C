#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include "Riostream.h"
#include "TMath.h"
#include "TVector.h"
#include "Util.h"
#include "ModuleDat.h"
#include "daqboardmap.h"
#include "daqpacket.h"
#include "decoderlib.h"
#include "chipevent.h"
#include "libInit_avdb.h"
#include "myrootlib.h"
#include "Syspardef.h"

using namespace std;




// Energy histograms ::
TH1F *E[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE];
TH1F *E_com[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE];

// U - V vectors
#include "TVector.h"
TVector *uu_c[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE];
TVector *vv_c[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE];
Long64_t uventries[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][2]={{{{0}}}};

TDirectory *subdir[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE];


void usage(void)
{
    int t=DEFAULTTHRESHOLD;
    int tnohit=DEFAULT_NOHIT_THRESHOLD;
    cout << " decode  -f [filename] [-v -o [outputfilename] -p  -d -t [threshold] " ;
    cout << " -uv -pedfile [pedfilename] ]" <<endl;
    cout << " -d : debug mode, a tree with unparsed data will be written (non-pedestal corrected). " <<endl;
    cout << "      you get access to u,v" << endl;
    cout << " -t : threshold for hits (trigger threshold), default = " << t << endl;
    cout << " -n : no hit threshold in other APD on same module, default = " << tnohit << endl;
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
    if (argc == 1) {
        usage();
        return(0);
    }

    float pedestals[CARTRIDGES_PER_PANEL][RENAS_PER_CARTRIDGE][MODULES_PER_RENA][CHANNELS_PER_MODULE]= {{{{0}}}};

    Int_t verbose = 0;
    string filename;
    string outfilename;
    bool outfile_spec_flag(false);

    string pedfilename;
    string pedvaluefilename;

    Int_t sourcepos=0;
    int threshold=DEFAULTTHRESHOLD;
    int nohit_threshold=DEFAULT_NOHIT_THRESHOLD;

    bool cmap_spec_flag(false);
    string cmap_filename;

    // Arguments not requiring input
    for (int ix = 1; ix < argc; ix++) {
        if (strcmp(argv[ix], "-v") == 0) {
            verbose = 1;
            cout << " Running verbose mode " << endl;
        }
        if (strcmp(argv[ix], "-t") == 0) {
            threshold = atoi(argv[ix+1]);
            ix++;
        }
    }
    // Arguments requiring input
    for (int ix = 1; ix < (argc - 1); ix++) {
        if (strcmp(argv[ix], "-t") == 0) {
            threshold = atoi(argv[ix + 1]);
        }
        if (strcmp(argv[ix], "-n") == 0) {
            nohit_threshold = atoi(argv[ix+1]);
        }
        if (strcmp(argv[ix], "-pedfile") == 0) {
            pedvaluefilename = string(argv[ix + 1]);
        }
        if (strcmp(argv[ix], "-pos") == 0) {
            sourcepos = atoi(argv[ix + 1]);
        }
        if (strcmp(argv[ix], "-cmap") == 0) {
            cmap_spec_flag = true;
            cmap_filename = string(argv[ix + 1]);
        }
        if (strcmp(argv[ix], "-f") == 0) {
            filename = string(argv[ix + 1]);
        }
        if (strcmp(argv[ix], "-o") == 0) {
            outfile_spec_flag = true;
            outfilename = string(argv[ix + 1]);
            pedfilename = string(argv[ix + 1]) + ".ped";
        }
    }

    if (filename == "") {
        printf("Please Specify Filename !!\n");
        usage();
        return(-1);
    }

    if (pedvaluefilename == "") {
        cerr << "Pedestal Value Filename not specified.  Exiting." << endl;
        return(-2);
    }

    if (!cmap_spec_flag) {
        string libpath = string(getenv("ANADIR"));
        cout << " Loading Shared Library from " << libpath << endl;
        cout << " (note ANADIR = " << getenv("ANADIR") << " )" << endl;
        cmap_filename = libpath + "/nfo/DAQ_Board_Map.nfo";
    }
    cout <<" Using Cartridge map file " << cmap_filename << endl;

    DaqBoardMap daq_board_map;
    int daq_board_map_status(daq_board_map.loadMap(cmap_filename));

    if (daq_board_map_status < 0) {
        cerr << "Error: failed to load DAQ Board map: " << cmap_filename << endl;
        cerr << "Exiting." << endl;
        return(-2);
    }

    int read_ped_status(ReadPedestalFile(pedvaluefilename, pedestals));

    if (read_ped_status < 0) {
        cerr << "Error reading Pedestal Value File: "
             << read_ped_status << endl;
        cerr << "Exiting." << endl;
        return(-3);
    }

    if (verbose) {
        cout << " threshold :: " << threshold << endl;
    }

    if (!outfile_spec_flag) {
        outfilename = filename + ".root";
        pedfilename = filename + ".ped";
    }

    ModuleDat *event = new ModuleDat();
    int doubletriggers[CARTRIDGES_PER_PANEL][RENAS_PER_CARTRIDGE][MODULES_PER_RENA]= {{{0}}};
    int belowthreshold[CARTRIDGES_PER_PANEL][RENAS_PER_CARTRIDGE][MODULES_PER_RENA]= {{{0}}};
    int totaltriggers[CARTRIDGES_PER_PANEL][RENAS_PER_CARTRIDGE][MODULES_PER_RENA][APDS_PER_MODULE]= {{{{0}}}};

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
    TTree *mdata = new TTree("mdata","Converted RENA data");
    mdata->Branch("eventdata",&event);

    ifstream dataFile;
    dataFile.open(filename.c_str(), ios::in | ios::binary);
    if (!dataFile.good()) {
        cerr << "Cannot open file \"" << filename
             << "\" for read operation." << endl;
        cerr << "Exiting." << endl;
        return(-1);
    }

    TFile * hfile = new TFile(outfilename.c_str(),"RECREATE");

    vector<char> packBuffer;

    long byteCounter(0);
    long totalPckCnt(0);
    long droppedPckCnt(0);
    long droppedFirstLast(0);
    long droppedOutOfRange(0);
    long droppedTrigCode(0);
    long droppedSize(0);

    long total_raw_events(0);


    dataFile.seekg(0, std::ios::end);
    std::streamsize dataFileSize = dataFile.tellg();
    dataFile.seekg(0, std::ios::beg);

    std::vector<char> buffer(dataFileSize);
    if (!dataFile.read(buffer.data(), dataFileSize)) {
        cerr << "File read failed" << endl;
        return(-2);
    }

    std::vector<char>::iterator buffer_iter(buffer.begin());

    while (buffer_iter != buffer.end()) {
        packBuffer.push_back(*(buffer_iter++));
        byteCounter++;
        if ((unsigned char)packBuffer.back()==0x81) {
            // end of package (start processing)
            totalPckCnt++;

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

                    mdata->Fill();
                    E[raw_events.at(ii).cartridge]
                            [processed_event.fin]
                            [processed_event.module]
                            [processed_event.apd]->Fill(processed_event.E);

                    E_com[raw_events.at(ii).cartridge]
                            [processed_event.fin]
                            [processed_event.module]
                            [processed_event.apd]->Fill(-processed_event.Ec);
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

    hfile->Close();

    if (verbose) {
        cout << " byteCounter = " << byteCounter << endl;
        cout << " totalPckCnt = " << totalPckCnt << endl;
        cout << " droppedPckCnt = " << droppedPckCnt << endl;
    }

    return(0);
}
