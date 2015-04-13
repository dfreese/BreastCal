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
#include "EventCal.h"
#include "daqboardmap.h"
#include "daqpacket.h"
#include "decoderlib.h"
#include "chipevent.h"
#include "libInit_avdb.h"
#include "myrootlib.h"
#include "Syspardef.h"

using namespace std;

void usage(void)
{
    int t=DEFAULTTHRESHOLD;
    int tnohit=DEFAULT_NOHIT_THRESHOLD;
    cout << "online_process [-v] -f [filename]\n"
         << " -pedfile [pedfilename] : File holding pedestal values\n"
         << " -t [threshold]: threshold for hits, default = " << t << "\n"
         << " -n : double trigger threshold for other APD on same module\n"
         << "       default = " << tnohit << "\n"
         << " -pedfile [pedfilename] : pedestal file name\n"
         << " -o : optional outputfilename\n"
         << " -cmap [DAQBOARD_FILE] : specify DAQ BOARD nfo file\n"
         << "       default = $ANADIR/nfo/DAQ_Board_Map.nfo"
         << " -calfile [calibration filename]\n"
         << " -uvfile [uv center filename]\n" << endl;
    return;
}

int main(int argc, char *argv[])
{
    if (argc == 1) {
        usage();
        return(0);
    }

    Int_t verbose = 0;
    string filename;
    string outfilename;
    bool outfile_spec_flag(false);

    string pedfilename;
    string pedvaluefilename;

    string cal_filename;
    string uv_filename;

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
        if (strcmp(argv[ix], "-calfile") == 0) {
            cal_filename = string(argv[ix + 1]);
        }
        if (strcmp(argv[ix], "-uvfile") == 0) {
            uv_filename = string(argv[ix + 1]);
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

    float pedestals[SYSTEM_PANELS]
                   [CARTRIDGES_PER_PANEL]
                   [RENAS_PER_CARTRIDGE]
                   [MODULES_PER_RENA]
                   [CHANNELS_PER_MODULE] = {{{{{0}}}}};

    int read_ped_status(ReadPedestalFile(pedvaluefilename, pedestals));

    if (read_ped_status < 0) {
        cerr << "Error reading Pedestal Value File: "
             << read_ped_status << endl;
        cerr << "Exiting." << endl;
        return(-3);
    }

    bool use_crystal[SYSTEM_PANELS]
                     [CARTRIDGES_PER_PANEL]
                     [FINS_PER_CARTRIDGE]
                     [MODULES_PER_FIN]
                     [APDS_PER_MODULE]
                     [CRYSTALS_PER_APD] = {{{{{{false}}}}}};

    float gain_spat[SYSTEM_PANELS]
                   [CARTRIDGES_PER_PANEL]
                   [FINS_PER_CARTRIDGE]
                   [MODULES_PER_FIN]
                   [APDS_PER_MODULE]
                   [CRYSTALS_PER_APD] = {{{{{{0}}}}}};

    float gain_comm[SYSTEM_PANELS]
                   [CARTRIDGES_PER_PANEL]
                   [FINS_PER_CARTRIDGE]
                   [MODULES_PER_FIN]
                   [APDS_PER_MODULE]
                   [CRYSTALS_PER_APD] = {{{{{{0}}}}}};

    float eres_spat[SYSTEM_PANELS]
                   [CARTRIDGES_PER_PANEL]
                   [FINS_PER_CARTRIDGE]
                   [MODULES_PER_FIN]
                   [APDS_PER_MODULE]
                   [CRYSTALS_PER_APD] = {{{{{{0}}}}}};

    float eres_comm[SYSTEM_PANELS]
                   [CARTRIDGES_PER_PANEL]
                   [FINS_PER_CARTRIDGE]
                   [MODULES_PER_FIN]
                   [APDS_PER_MODULE]
                   [CRYSTALS_PER_APD] = {{{{{{0}}}}}};

    float crystal_x[SYSTEM_PANELS]
                   [CARTRIDGES_PER_PANEL]
                   [FINS_PER_CARTRIDGE]
                   [MODULES_PER_FIN]
                   [APDS_PER_MODULE]
                   [CRYSTALS_PER_APD] = {{{{{{0}}}}}};

    float crystal_y[SYSTEM_PANELS]
                   [CARTRIDGES_PER_PANEL]
                   [FINS_PER_CARTRIDGE]
                   [MODULES_PER_FIN]
                   [APDS_PER_MODULE]
                   [CRYSTALS_PER_APD] = {{{{{{0}}}}}};

    int read_cal_status(ReadCalibrationFile(cal_filename,
                                            use_crystal,
                                            gain_spat,
                                            gain_comm,
                                            eres_spat,
                                            eres_comm,
                                            crystal_x,
                                            crystal_y));

    if (read_cal_status < 0) {
        cerr << "Calibration file: \"" << cal_filename
             << "\" could not be read.  Exiting.";
        return(-4);
    }

    float circles_u[SYSTEM_PANELS]
                   [CARTRIDGES_PER_PANEL]
                   [FINS_PER_CARTRIDGE]
                   [MODULES_PER_FIN]
                   [APDS_PER_MODULE] = {{{{{0}}}}};

    float circles_v[SYSTEM_PANELS]
                   [CARTRIDGES_PER_PANEL]
                   [FINS_PER_CARTRIDGE]
                   [MODULES_PER_FIN]
                   [APDS_PER_MODULE] = {{{{{0}}}}};

    int read_uv_status(ReadUVCirclesFile(uv_filename,
                                         circles_u,
                                         circles_v));

    if (read_uv_status < 0) {
        cerr << "UV Circle Centers file: \"" << uv_filename
             << "\" could not be read.  Exiting.";
        return(-5);
    }


    if (verbose) {
        cout << " threshold :: " << threshold << endl;
    }

    if (!outfile_spec_flag) {
        outfilename = filename + ".root";
        pedfilename = filename + ".ped";
    }

    int doubletriggers[CARTRIDGES_PER_PANEL][RENAS_PER_CARTRIDGE][MODULES_PER_RENA]= {{{0}}};
    int belowthreshold[CARTRIDGES_PER_PANEL][RENAS_PER_CARTRIDGE][MODULES_PER_RENA]= {{{0}}};
    int totaltriggers[CARTRIDGES_PER_PANEL][RENAS_PER_CARTRIDGE][MODULES_PER_RENA][APDS_PER_MODULE]= {{{{0}}}};

    ifstream dataFile;
    dataFile.open(filename.c_str(), ios::in | ios::binary);
    if (!dataFile.good()) {
        cerr << "Cannot open file \"" << filename
             << "\" for read operation." << endl;
        cerr << "Exiting." << endl;
        return(-1);
    }

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
                        //cout << "Dropped: Empty Packet Vector" << endl;
                    }
                } else if (decode_status == -2){
                    droppedFirstLast++;
                    if (verbose) {
                        //cout << "Dropped: First Byte not 0x80" << endl;
                    }
                } else if (decode_status == -3) {
                    droppedTrigCode++;
                    if (verbose) {
                        //cout << "Dropped: Trigger Code = 0" << endl;
                    }
                } else if (decode_status == -4) {
                    droppedSize++;
                    if (verbose) {
                        //cout << "Dropped: Packet Size Wrong - "
                        //     << packBuffer.size() << endl;
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
                                                cartridge_id));

            for (size_t ii = 0; ii < raw_events.size(); ii++) {
                EventCal calibrated_event;

                int calibration_status = RawEventToEventCal(raw_events.at(ii),
                                                            calibrated_event,
                                                            pedestals,
                                                            circles_u,
                                                            circles_v,
                                                            gain_spat,
                                                            gain_comm,
                                                            eres_spat,
                                                            eres_comm,
                                                            crystal_x,
                                                            crystal_y,
                                                            use_crystal,
                                                            threshold,
                                                            nohit_threshold,
                                                            panel_id);

                if (calibration_status >= 0) {
                    totaltriggers[raw_events.at(ii).cartridge]
                            [raw_events.at(ii).chip]
                            [raw_events.at(ii).module]
                            [calibration_status]++;

                } else if (calibration_status == -1) {
                    belowthreshold[raw_events.at(ii).cartridge]
                            [raw_events.at(ii).chip]
                            [raw_events.at(ii).module]++;
                } else if (calibration_status == -2) {
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

    cout << "========== Double Triggers =============== " << endl;
    for (int c=0; c<CARTRIDGES_PER_PANEL; c++) {
        for (int r=0; r<RENAS_PER_CARTRIDGE; r++) {
            for (int i=0; i<MODULES_PER_RENA; i++ ) {
                cout << doubletriggers[c][r][i] << " " ;
                totaldoubletriggers+=doubletriggers[c][r][i];
                totalbelowthreshold+=belowthreshold[c][r][i];
            }
            cout << endl;
        }
    }

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

    if (verbose) {
        cout << " byteCounter = " << byteCounter << endl;
        cout << " totalPckCnt = " << totalPckCnt << endl;
        cout << " droppedPckCnt = " << droppedPckCnt << endl;
    }

    return(0);
}
