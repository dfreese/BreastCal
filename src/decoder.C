/* AVDB 12-11-13

   Update the program to handle ethernet based data structure. 

   - pedestals and timing organized per RENA chip, runs as ChipId*FOURUPBOARDNR

   - program will also take calibration values as an input 

*/

/* AVDB 11-30-12 
   Conversion Program for RENA data as it comes from the DAQ
   to a root TREE 
   Function takes tree input arguments:
   -v  :: verbosity switch
   -f  :: inputfilename
   -o  :: outputfilename
   -p  :: genearates .ped file with average and RMS values of the channels
   -t  :: threshold for calculating UV
   only the -f switch is necessary ! 

   PROGRAM needs to:
   read in pedestals if specified.
   calculate uv
   write tree in debug mode
   parse data 
   -d :: a debug mode that creates a root ouput file of the first pass
   -pedfile :: run pedestal subtraction, calculate x,y
   -uv :: run timecalc  
   -pos :: position
   -uvt :: set uv threshold [ for pulser applications ]
   */


//#define DEBUG 

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>


#include "TROOT.h"
#include "TFile.h"
#include "TNetFile.h"
#include "TRandom.h"
#include "TTree.h"
#include "Riostream.h"
#include "TMath.h"
#include "TVector.h"
#include "./decoder.h"
#include "./ModuleDat.h"
//#include "./convertconfig.h"


#define FILENAMELENGTH 120

// This the number of events that is excluded at the beginning of the file
int no_events_exclude_beginning(0);

void usage(void);

void usage(void) {
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

using namespace std;
void parsePack(const vector<char> &packBuffer, int coin = -1, int usbNum = -1);
Float_t finecalc(Short_t u, Short_t v, Float_t u_cent, Float_t v_cent);
void getfinmodule(Short_t panel, Short_t chip, Short_t &module, Short_t &fin);
Int_t pedana( double* mean, double* rms, int  *events, Short_t value ) ;

// huh, global variable ..
unsigned long totalPckCnt=0;
unsigned long droppedPckCnt=0;
unsigned long droppedFirstLast=0;
unsigned long droppedOutOfRange=0;
unsigned long droppedTrigCode=0;
unsigned long droppedSize=0;
vector<unsigned int> chipRate;




int main(int argc, char *argv[]) {
    float pedestals[CARTRIDGES_PER_PANEL][RENAS_PER_CARTRIDGE][MODULES_PER_RENA][CHANNELS_PER_MODULE]={{{{0}}}};
    Int_t calcpedestal=0;
    // changed this to be always one, because ana code expects the vectors containing u,v centers to be in the root file.
    // however,this is set to 0 if the user specifies "-p" on the command line.
    int uvcalc;//=1;

    int chip;
    int module;
    int cartridge;
    int uvthreshold = -1000;
    // FIXME hardcoded value :: 
    pcnumber pclist[32];

    TFile *hfile;
    int c,r,f,i,j,m;

    Int_t ix,verbose;
    char filename[FILENAMELENGTH];
    char outfilename[FILENAMELENGTH+5];
    char pedfilebase[FILENAMELENGTH+10];
    char ascifilename[FILENAMELENGTH+6];
    char pedfilename[FILENAMELENGTH+10];
    char pedvaluefilename[FILENAMELENGTH+10];
    int pedfilenamespec=0;

    ofstream pedfile,ascifile;
    int outfileset=0;
    int genascifile=0;
    int filenamespec=0;
    verbose=0;
    Int_t sourcepos=0;
    int threshold=DEFAULTTHRESHOLD;
    int nohit_threshold=DEFAULT_NOHIT_THRESHOLD;
    int debugmode=0;


    Bool_t cmapspec=kFALSE;
    TString nfofile;

    for ( ix=1;ix< argc;ix++) {

        /* Verbose  '-v' */
        if (strncmp(argv[ix], "-v", 2) == 0) {
            verbose = 1;
            cout << " Running verbose mode " << endl;
        }

        if (strncmp(argv[ix], "-d", 2) == 0) {
            debugmode = 1;
        }

        if (strncmp(argv[ix], "-uv", 3) == 0) {
            uvcalc = 1;
        }

        if (strncmp(argv[ix], "-uvt", 4) == 0) {
            uvthreshold = atoi(argv[ix+1]);
            ix++;
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
                ix++;
                if (strlen(argv[ix])<FILENAMELENGTH) {  
                    sprintf( pedvaluefilename,"%s", argv[ix]);
                    pedfilenamespec=1;
                } else  {
                    cout << "Filename " << argv[ix] << " too long !" <<endl;
                    cout << "Exiting.." <<endl;
                    return(-99);
                }
            } else {
                if (strncmp(argv[ix],"-pos",4) == 0) {
                    sourcepos=atoi(argv[ix+1]);
                    //        cout << " sourcepos :: " << sourcepos << endl;
                    ix++;
                } else {
                    /* Pedestal  '-p' if not  '-pedfile' and not '-pos' !!*/
                    calcpedestal = 1;
                    uvcalc = 0;
                }
            }
        }

        /* Cartridge map specified !!*/
        if (strncmp(argv[ix], "-cmap", 5) == 0) {
            cmapspec = kTRUE;
            nfofile.Form("%s",argv[ix+1]);
        }

        /* filename '-f' */
        if (strncmp(argv[ix], "-f", 2) == 0) {
            if (strlen(argv[ix+1])<FILENAMELENGTH) {  
                sprintf( filename,"%s", argv[ix+1]);
                filenamespec=1;
            } else  {
                cout << "Filename " << argv[ix+1] << " too long !" <<endl;
                cout << "Exiting.." <<endl;
                return -99;
            }
        }

        /* Outputfile  '-o' */

        if (strncmp(argv[ix], "-o", 2) == 0) {
            outfileset = 1;
            if (strlen(argv[ix+1])<FILENAMELENGTH) {  
                sprintf( outfilename,"%s", argv[ix+1]);
                sprintf( pedfilebase,"%s.ped", argv[ix+1]);
                //        sprintf( pedfilename2,"%s.ped.RENA2", argv[ix+1]);
                sprintf( ascifilename,"%s.ascii",argv[ix+1]);
            } else  {
                cout << "Output Filename " << argv[ix+1] << " too long !" <<endl;
                cout << "Exiting.." <<endl;
                return(-98);
            }
        }

        /* Outputfile  '-o' */
        if (strncmp(argv[ix], "-a", 2) == 0) {
            genascifile =1 ;
        }


    } // loop input arguments

    if (!(filenamespec)) {
        printf("Please Specify Filename !!\n");
        usage();
        return(-1);
    }

    if (!cmapspec) {
        char *libpath = getenv("ANADIR");
        cout << " Loading Shared Library from " << libpath << endl;
        cout << " (note ANADIR = " << getenv("ANADIR") << " )" << endl;
        TString exestring;
        nfofile.Form("%s/nfo/%s",libpath,"DAQ_Board_Map.nfo");
        //	     nfofile.Form("%s/",ANADIR)
    }
    cout <<" Using Cartridge map file " << nfofile << endl;

    // read in cartridge map
    ifstream carmap;
    carmap.open(nfofile);
    if (! carmap.good()) {
        cout << " Error opening file " << nfofile << endl;
        cout << " Exiting " << endl;
        return(-9);
    }

    string c_id;
    string dummy;
    int id_val;
    string fileline;
    int panel_id;
    int cartridge_id;

    i=0;
    while(getline( carmap,fileline)) {
        if (fileline.size() > 0) {
            if (fileline[0] == '#') {
                // If a line starts with a pound sign, it is ignored
                continue;
            }
        }

        std::stringstream linestream(fileline);
        std::getline(linestream,c_id, ' ');
        std::getline(linestream,dummy, ' ');
        if (!(linestream >> id_val )) {
            cout << " Error parsing " << fileline << " from file " << nfofile << endl;
            break;
        }
        sscanf(c_id.c_str(),"P%dC%d",&panel_id,&cartridge_id);
        cout << " PANEL :: " << panel_id  << " CARTRIDGE :: " << cartridge_id << " ID : " << id_val << endl;
        pclist[id_val].panel=panel_id;
        pclist[id_val].cartridge=cartridge_id;
        i++;
    }

    if (verbose) {  
        cout << " pclist.daqid     = " ;
        for ( i=0;i<32;i++) {
            cout << pclist[i].panel << " ";
        }
        cout << endl;

        cout << " pclist.panel     = " ;
        for ( i=0;i<32;i++) {
            cout << pclist[i].panel << " ";
        }
        cout << endl;
        cout << " pclist.cartridge = " ;
        for (i=0;i<32;i++) {
            cout << pclist[i].cartridge << " ";
        }
        cout << endl;
    }
    carmap.close();


    if (pedfilenamespec) {
        ifstream pedvals;
        int events;
        double tmp;
        Char_t idstring[12]; // form C#R#M#
        pedvals.open(pedvaluefilename);
        if (!pedvals) {
            cout << " Error opening file " << pedvaluefilename << "\n.Exiting.\n";  return -10;
        }
        cout << " Decoding file " << filename << " with pedestals " << pedvaluefilename << endl;
        while ( pedvals >> idstring ) { 
            sscanf(idstring,"C%dR%dM%d",&cartridge,&chip,&module);
            // cout << " c = " << cartridge << " R = " << chip << " M = " << module << endl;
            pedvals >> events;
            if (( chip >= RENAS_PER_CARTRIDGE )||(module >= MODULES_PER_RENA ) ||(cartridge >= CARTRIDGES_PER_PANEL)) {
                cout << " Error reading pedfile, module or chipnumber too high: " << endl;
                cout << " module = " << module << ", chip = " << chip << ".\nExiting." << endl;
                return(-2);
            }
            for ( int ii=0; ii< CHANNELS_PER_MODULE;ii++) {
                pedvals >> pedestals[cartridge][chip][module][ii] ;
                pedvals >> tmp;
            } // loop over ii
        } // while loop	 

        if (verbose) {
            cout << " Pedestal values :: " << endl;
            for (c=0;c<CARTRIDGES_PER_PANEL;c++) {
                for (r=0;r<RENAS_PER_CARTRIDGE;r++) {
                    for (i=0;i<MODULES_PER_RENA;i++) {
                        for (int k=0;k<CHANNELS_PER_MODULE;k++) {
                            cout << pedestals[c][r][i][k] << " " ;
                        }
                        cout << endl;
                    } // loop over i
                } // loop r
            } // loop over c
        } // verbose
        //pedfilenamespec
    } else {
        cout << " Decoding file " << filename << " without pedestals " << endl;
    }

    if (verbose) {
        cout << " threshold :: " << threshold << endl;
    }

    if (!uvcalc) {
        cout << " ======================================================================== " << endl;
        cout << " |  Warning you did not enable UV calc, this will affect chain_parsed ! | " << endl;
        cout << " ======================================================================== " << endl;
    }


    if (!(outfileset)) {
        sprintf(outfilename,"%s.root",filename);
        sprintf(pedfilebase,"%s.ped",filename);
        //     sprintf(pedfilename2,"%s.ped.RENA2",filename);
        sprintf(ascifilename,"%s.ascii",filename);
    }


    Char_t treename[10],treetitle[40];

    ModuleDat *event = new ModuleDat();
    Int_t doubletriggers[CARTRIDGES_PER_PANEL][RENAS_PER_CARTRIDGE][MODULES_PER_RENA]={{{0}}};
    Int_t belowthreshold[CARTRIDGES_PER_PANEL][RENAS_PER_CARTRIDGE][MODULES_PER_RENA]={{{0}}};
    Int_t totaltriggers[CARTRIDGES_PER_PANEL][RENAS_PER_CARTRIDGE][MODULES_PER_RENA][APDS_PER_MODULE]={{{{0}}}};


    TTree *mdata=0; 



    for (int c=0;c<CARTRIDGES_PER_PANEL;c++) {
        for (int f=0;f<FINS_PER_CARTRIDGE;f++) {
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
        for (int c=0;c<CARTRIDGES_PER_PANEL;c++) {
            for (int f=0;f<FINS_PER_CARTRIDGE;f++) {
                for (int i=0;i<MODULES_PER_FIN;i++) {	
                    for (int j=0;j<APDS_PER_MODULE;j++) {

                        sprintf(tmpstring,"E[%d][%d][%d][%d]",c,f,i,j);
                        sprintf(titlestring,"E C%dF%d, Module %d, PSAPD %d ",c,f,i,j);
                        E[c][f][i][j]=new TH1F(tmpstring,titlestring,Ebins,E_low,E_up);
                        sprintf(tmpstring,"E_com[%d][%d][%d][%d]",c,f,i,j);
                        sprintf(titlestring,"ECOM C%dF%d, Module %d, PSAPD %d",c,f,i,j);
                        E_com[c][f][i][j]=new TH1F(tmpstring,titlestring,Ebins_com,E_low_com,E_up_com);
                    } //j
                } // i
            }// f
        } // c

        if (verbose) {
            cout << " Creating tree " << endl;
        }

        sprintf(treename,"mdata");
        sprintf(treetitle,"Converted RENA data" );
        mdata =  new TTree(treename,treetitle);
        mdata->Branch("eventdata",&event);
    } // pedfilenamespec

    dataFile.open(filename, ios::in | ios::binary);
    if (!dataFile.good()) {
        cout << "Cannot open file \"" << filename << "\" for read operation. Exiting." << endl;
        return -1;
    }

    if (genascifile) {
        ascifile.open(ascifilename);
    }
    /*
       printf("Conversion program for Rena Data\n");
       printf("On this machine one short is %ld bytes\n",sizeof(short));
       printf("On this machine Timestamp is %ld bytes\n",sizeof(Long64_t));
       printf("On this machine Integer is %ld bytes\n",sizeof(Int_t));
       printf("Converting the File :::::: %s\n",filename);
       */
    hfile = new TFile(outfilename,"RECREATE"); 


    chipevent rawevent;
    TTree *rawdata; 

    sprintf(treename,"rawdata");
    sprintf(treetitle,"Converted Raw RENA data" );
    rawdata =  new TTree(treename,treetitle);
    if (!debugmode) rawdata->SetDirectory(0);
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


    if (dataFile) {
        char cc;
        dataFile.seekg(0, ios::end);
        endPos = dataFile.tellg();
        dataFile.seekg(lastPos);
        if (verbose) {
            cout << " endPos :: " << endPos << endl;
            cout << " lastPos :: " << lastPos << endl;
        }

        int chipId = 0;
        int trigCode = 0;
        Short_t cartridgeId = 0;
        Short_t panelId =0;
        int intentionally_dropped(0);

        for ( i=0; i<(endPos-lastPos-1); i++) { //slow return
            dataFile.get(cc);
            packBuffer.push_back(cc);
            byteCounter++;
            if ((unsigned char)cc==0x81) {
#ifdef DEBUG
                cout << " lastPos :: " << lastPos ;
                cout << " packBuffer[0] :: " << hex << int(packBuffer[0]);
                cout << " packBuffer[1] :: " << int(packBuffer[1]);
                cout << " packBuffer[2] :: " << int(packBuffer[2]);
                cout << dec;
#endif
                // end of package (start processing)
                totalPckCnt++;

                if (intentionally_dropped < no_events_exclude_beginning) {
                    intentionally_dropped++; 
                    packBuffer.clear();
                    continue;
                }

                if ( (int(packBuffer[0] & 0xFF )) != 0x80 ) {
                    // first byte of packet needs to be 0x80
                    droppedPckCnt++;
                    droppedFirstLast++;
                    packBuffer.clear();
                    continue;
                }           

                if (PAULS_PANELID) {
                    // ChipId format changed on 06/05/2012
                    int Idnumber = int(( packBuffer[1] & 0x7C ) >> 2 );
                    //        cartridgeId =  Short_t((packBuffer[1] & 0x3c) >> 2);
                    panelId=pclist[Idnumber].panel;
                    cartridgeId=pclist[Idnumber].cartridge; 

                    if ((panelId >= SYSTEM_PANELS ) || ( cartridgeId >= CARTRIDGES_PER_PANEL ) || (panelId < 0 ) || (cartridgeId < 0)) {
                        if (verbose)  { 
                            cout << " PanelId " << panelId << " or CartridgeId " << cartridgeId ;
                            cout << " doesn't match the definitions in include/Syspardef.h" << endl;
                            cout << " Skipping." << endl;}
                        droppedPckCnt++;
                        droppedOutOfRange++;
                        if (verbose) {
                            cout << "Dropped = " << packBuffer.size() << endl;
                        }
                        packBuffer.clear();
                        continue;
                    }
                    int local_four_up_board = int((packBuffer[1] & 0x03) >> 0);
                    int four_up_board_num = local_four_up_board;
                    // MAPPING ISSUE !! -- 12/12/2013

                    vector<bool> fpgaIdVec;
                    fpgaIdVec.push_back((packBuffer[2] & 0x20) != 0);
                    fpgaIdVec.push_back((packBuffer[2] & 0x10) != 0);
                    int fpgaId = Util::boolVec2Int(fpgaIdVec);
                    chipId = 2*fpgaId;
                    if ((packBuffer[2] & 0x40) != 0) {
                        chipId++;
                    }
                    chipId+= four_up_board_num*RENAS_PER_FOURUPBOARD;
                    trigCode = int(packBuffer[2] & 0x0F);

#ifdef DEBUG
                    cout << " trigCode = 0x" << hex << trigCode << dec ;
                    cout << " idnumber = " << Idnumber ;
                    cout << " panelId = " << panelId << "; cartridgeId = " << cartridgeId << endl;
                    cout << " chipId = " << chipId << "; fpgaId = " << fpgaId << endl;
#endif
                } else {
                    // This is the old ChipId format which can be used to process old data
                    vector<bool> fpgaIdVec;
                    fpgaIdVec.push_back((packBuffer[1] & 0x10) != 0);
                    fpgaIdVec.push_back((packBuffer[1] & 0x08) != 0);
                    int fpgaId = Util::boolVec2Int(fpgaIdVec);
                    chipId = 2*fpgaId;
                    if ((packBuffer[1] & 0x20) != 0) {
                        chipId++;
                    }
                }
                // At this point we extracted the chipId from the packet
                // Now we need to get the who_triggered bits


                //cout << "coin = " << coin << " " << "chipId = " << chipId << endl;

                unsigned int packetSize;

                // packetSize:: 
                //   1 byte 0x80
                //   2 byte 0 panelID (1) layerID (4) moduleID (2)
                //   3 byte 0 chipID (1) fpgaID (2) triggerID (4)
                //   4-9 :  42 bit digital timestamp  ( 6 byte ) 
                //   4 * ( 4 * 2  + 4 * 6 ) = 4 * ( 32) = 128  
                //    1  byte
                //   ----> total : 10 + 128 = 138 -- we got 146 !

                int moduletriggers;

                if (!MODULE_BASED_READOUT) {
                    if (DAQ_BEFORE01042013) {
                        packetSize=146; //FIXME -- need to be changed for data later than 1/4/13 .. ( should be 138 )
                    } else {
                        packetSize=138;
                    }

                    moduletriggers=4;
                    trigCode = 0xF;
                } else { 
                    // MODULE_BASED_READOUT
                    if (trigCode == 0) {
                        droppedPckCnt++;
                        droppedTrigCode++;
                        packBuffer.clear();
                        continue;
                    }
                    if (PAULS_PANELID) {
                        packetSize =10;
                    } else {
                        packetSize = 9;
                    }
                    for (int ii=0;ii<4;ii++) {
                        packetSize +=  32*( ( trigCode >> ii ) &  0x1  ) ;
                        moduletriggers +=  ( ( trigCode >> ii ) & 0x1) ;
                    }
                    // packetSize = rmChip->packetSizeMap[trigCode];
                }
                //    cout << " packetSize :: " << packetSize  << "; packBuffer.size() = " << packBuffer.size() << endl;



                // FIXME  BAD HARDCODED VALUE 

                //   packetSize=146; // need to figure this out one way or another ..

                if (packBuffer.size()!=packetSize) {
                    droppedPckCnt++;
                    droppedSize++;
                    if (verbose) {
                        cout << "Dropped = " << packBuffer.size() << endl;
                    }
                    packBuffer.clear();
                    continue;
                }

                vector<Short_t> adcBlock;

                // Parse the package here!
                Long64_t timestamp(0);
                //Long64_t energy;
                //Long64_t timing1;
                //Long64_t timing2;
                // Now we can parse the packet

                // In real application parse the chip ID first
                // Store channel_data in a vector based on their receive order
                // After parsing, sort them out and assign them to their respective
                // positions.


                // Byte 1 is not used - 0x1f

                // Bytes 2 to 7 are timestamp data
                if (PAULS_PANELID) {
                    for (int ii=3; ii<=8; ii++){
                        timestamp = timestamp << 7;
                        timestamp += Long64_t((packBuffer[ii]&0x7F));
                        // cout << ii << " " << Long64_t((packBuffer[ii]&0x7F)) ;
                        // cout << " " << timestamp << endl;
                        // timestamp += Long64_t((packBuffer[ii]&0x3F));
                    }
                } else {
                    for (int ii=2; ii<=7; ii++) {
                        timestamp = timestamp << 7;
                        timestamp += Long64_t(packBuffer[ii]);
                    }
                }


                // Remaining bytes are ADC data for each channel
                if (PAULS_PANELID) {
                    Short_t value;
                    for (unsigned int counter = 9; counter<(packetSize-1); counter+=2) {
                        value = (Short_t)packBuffer[counter];
                        //	    cout << " val 1 = " << value;
                        value = value << 6;
                        value += (Short_t)packBuffer[counter+1];
                        //            cout << " val 2 = " << (Short_t)packBuffer[counter+1] << " totval: " << value << endl;
                        adcBlock.push_back(value);
                    }
                } else {
                    Short_t value;
                    for (unsigned int counter = 8; counter<(packetSize-1); counter+=2) {
                        value = (Short_t)packBuffer[counter];
                        value = value << 6;
                        value += (Short_t)packBuffer[counter+1];
                        adcBlock.push_back(value);
                    }
                }

                //        cout << " Size ADCBLOCK :: " << adcBlock.size() << endl; 

                // Now we have all values in adcBlock vector.

                //DEBUG
                //cout << "Dropped = " << droppedPckCnt << endl;
                //    for (int i=0; i<adcBlock.size(); i++) {
                //        cout << adcBlock[i] << " | ";
                //    }
                //    cout << endl;
                //END_DEBUG

                int even=(int) chipId%2;
                int nrchips=0;

                for (int ii=0;ii<4;ii++) {
                    nrchips+= (( trigCode >> ii ) & ( 0x1 ) );
                }

#define SHIFTFACCOM 4
#define SHIFTFACSPAT 12
#define BYTESPERCOMMON 12   // 4 * 3 = 12 ( 4 commons per module, 3 values per common )
#define VALUESPERSPATIAL 4

                // should be 2 for data obtained before 1/4/2013

#if DAQ_BEFORE01042013
#define UNUSEDCHANNELOFFSET 2
#else
#define UNUSEDCHANNELOFFSET 0
#endif


#define UNUSEDCHANNELOFFSET 0
                int kk=0;
                for (int iii=0;iii<4;iii++ ) {
                    if ( trigCode & ( 0x1 << iii )) {
                        rawevent.ct=timestamp;
                        rawevent.chip= chipId;
                        rawevent.cartridge = cartridgeId;
                        rawevent.module= iii;
                        rawevent.com1h = adcBlock[UNUSEDCHANNELOFFSET + kk*BYTESPERCOMMON + even*nrchips*SHIFTFACCOM];  //6
                        rawevent.u1h = adcBlock[UNUSEDCHANNELOFFSET+1 + kk*BYTESPERCOMMON + even*nrchips*SHIFTFACCOM];
                        rawevent.v1h = adcBlock[UNUSEDCHANNELOFFSET+2+kk*BYTESPERCOMMON + even*nrchips*SHIFTFACCOM];
                        rawevent.com1 = adcBlock[UNUSEDCHANNELOFFSET+3+kk*BYTESPERCOMMON + even*nrchips*SHIFTFACCOM];
                        rawevent.u1 =adcBlock[UNUSEDCHANNELOFFSET+4+kk*BYTESPERCOMMON + even*nrchips*SHIFTFACCOM];
                        rawevent.v1 =adcBlock[UNUSEDCHANNELOFFSET+5+kk*BYTESPERCOMMON+ even*nrchips*SHIFTFACCOM];
                        rawevent.com2h = adcBlock[UNUSEDCHANNELOFFSET+6 + kk*BYTESPERCOMMON + even*nrchips*SHIFTFACCOM];
                        rawevent.u2h = adcBlock[UNUSEDCHANNELOFFSET+7 + kk*BYTESPERCOMMON + even*nrchips*SHIFTFACCOM];
                        rawevent.v2h = adcBlock[UNUSEDCHANNELOFFSET+8+kk*BYTESPERCOMMON + even*nrchips*SHIFTFACCOM];
                        rawevent.com2 = adcBlock[UNUSEDCHANNELOFFSET+9+kk*BYTESPERCOMMON + even*nrchips*SHIFTFACCOM];
                        rawevent.u2=adcBlock[UNUSEDCHANNELOFFSET+10+kk*BYTESPERCOMMON + even*nrchips*SHIFTFACCOM];
                        rawevent.v2=adcBlock[UNUSEDCHANNELOFFSET+11+kk*BYTESPERCOMMON+ even*nrchips*SHIFTFACCOM];
                        rawevent.a=adcBlock[UNUSEDCHANNELOFFSET+BYTESPERCOMMON*nrchips+kk*VALUESPERSPATIAL - even*nrchips*SHIFTFACSPAT];  //2
                        rawevent.b=adcBlock[UNUSEDCHANNELOFFSET+BYTESPERCOMMON*nrchips+1+kk*VALUESPERSPATIAL - even*nrchips*SHIFTFACSPAT];  //3
                        rawevent.c=adcBlock[UNUSEDCHANNELOFFSET+BYTESPERCOMMON*nrchips+2+kk*VALUESPERSPATIAL - even*nrchips*SHIFTFACSPAT];  //4
                        rawevent.d=adcBlock[UNUSEDCHANNELOFFSET+BYTESPERCOMMON*nrchips+3+kk*VALUESPERSPATIAL - even*nrchips*SHIFTFACSPAT];  //5
                        rawevent.pos=sourcepos;
                        kk++;
                        rawdata->Fill();

                        module=iii;

                        if (pedfilenamespec) {
                            event->module=module;
                            event->ct=timestamp; 
                            event->chip=rawevent.chip;
                            event->cartridge=rawevent.cartridge;
                            event->a=rawevent.a - pedestals[event->cartridge][chipId][module][0];
                            event->b=rawevent.b - pedestals[event->cartridge][chipId][module][1];
                            event->c=rawevent.c - pedestals[event->cartridge][chipId][module][2];
                            event->d=rawevent.d - pedestals[event->cartridge][chipId][module][3];

                            getfinmodule(panelId,event->chip,event->module,event->fin);
                            //	     cout << " R" <<event->chip << "M" << event->module << "F" << event->fin <<endl;
                            event->E=event->a+event->b+event->c+event->d;
                            event->x= event->c + event->d - ( event->b + event->a );
                            event->y= event->a + event->d - ( event->b + event->c );

                            event->x/=event->E;
                            event->y/=event->E;

                            event->apd=-1;
                            event->id=-1;
                            event->pos=rawevent.pos;
#ifdef DEBUG
                            cout << " chipId = " << chipId << " module = " << module << " cartridge : " << event->cartridge << endl;
#endif 

                            if ( (rawevent.com1h - pedestals[event->cartridge][chipId][module][5]) < threshold ) { 
                                if ( (rawevent.com2h - pedestals[event->cartridge][chipId][module][7]) > nohit_threshold ) { 
                                    totaltriggers[event->cartridge][chipId][module][0]++;
                                    event->apd=0; 
                                    // FIXME
                                    //       event->ft=finecalc(rawevent.u1h,rawevent.v1h,uu_c[chipId][module][0],vv_c[chipId][module][0])  ;
                                    event->ft= (  ( ( rawevent.u1h & 0xFFFF ) << 16  )  | (  rawevent.v1h & 0xFFFF ) ) ;
                                    event->Ec= rawevent.com1 - pedestals[event->cartridge][chipId][module][4];
                                    event->Ech=rawevent.com1h - pedestals[event->cartridge][chipId][module][5];
                                    mdata->Fill();
                                } else {
                                    doubletriggers[event->cartridge][chipId][module]++;
                                }
                            }
                            else {
                                if ( (rawevent.com2h - pedestals[event->cartridge][chipId][module][7]) < threshold ) {
                                    if ( (rawevent.com1h - pedestals[event->cartridge][chipId][module][5]) > nohit_threshold ) { 
                                        totaltriggers[event->cartridge][chipId][module][1]++;
                                        event->apd=1;   
                                        event->y*=-1;
                                        // FIXME :: need to find solution for ft. 
                                        //     event->ft=finecalc(rawevent.u2h,rawevent.v2h,uu_c[chipId][module][1],vv_c[chipId][module][1])  ;
                                        event->ft= (  ( ( rawevent.u2h & 0xFFFF ) << 16  )  | (  rawevent.v2h & 0xFFFF ) ) ;
                                        //  ,rawevent.v2h,uu_c[chipId][module][1],vv_c[chipId][module][1])  ;
                                        event->Ec = rawevent.com2 - pedestals[event->cartridge][chipId][module][6];
                                        event->Ech=rawevent.com2h- pedestals[event->cartridge][chipId][module][7];
                                        mdata->Fill();
                                    } else {
                                        doubletriggers[event->cartridge][chipId][module]++;
                                    }
                                } else {
                                    belowthreshold[event->cartridge][chipId][module]++;
                                }
                            }
                            // fill energy histogram
#ifdef DEBUG
                            if (( event->module > MODULES_PER_FIN ) || ( event->apd > 1 ) || ( event->apd < 0) || (event->fin > FINS_PER_CARTRIDGE ) ) {
                                cout << "ERROR !!" ;
                                cout << " MODULE : " << event->module << ", APD : " << event->apd << ", CHIP : " << event->chip << endl;
                            }
#endif
                            if (( event->apd == 1 )||(event->apd ==0 )) { 
                                E[event->cartridge][event->fin][event->module][event->apd]->Fill(event->E);
                                E_com[event->cartridge][event->fin][event->module][event->apd]->Fill(-event->Ec);
                            }


                        } // pedfilenamespec


                        if (uvcalc) {

                            // to calculate circle centers we need to make sure the module triggered, so we use a high threshold

                            /*
                               cout << " UV :: " << rawevent.u1h << " " << rawevent.v1h << " ( cartridge = " << cartridgeId << ", chip = " << chipId << ", m = " << module <<")" <<  endl;
                               cout << "       " <<  rawevent.com1h - pedestals[cartridgeId][chipId][module][5] <<  " ";
                               cout << rawevent.com2h - pedestals[cartridgeId][chipId][module][7]  << " " << uvthreshold << endl;
                               */
                            // note uu_c[cartridgeId][chipId] is an array of pointers to a vector .
                            if (( event->apd == 1 )||(event->apd ==0 )) { 
                                if (  ( rawevent.com1h - pedestals[cartridgeId][chipId][module][5] ) < uvthreshold ) {
                                    //  cout <<   " entries:: " <<     uventries[cartridgeId][chipId][module][0] << " uu_c = " << (*uu_c[cartridgeId][chipId])[module*2] << endl ;
                                    uventries[cartridgeId][event->fin][event->module][0]++;
                                    (*uu_c[cartridgeId][event->fin])[event->module*2+0]+=(Float_t)(rawevent.u1h- (*uu_c[cartridgeId][event->fin])[event->module*2+0])/uventries[cartridgeId][event->fin][event->module][0]; 
                                    (*vv_c[cartridgeId][event->fin])[event->module*2+0]+=(Float_t)(rawevent.v1h- (*vv_c[cartridgeId][event->fin])[event->module*2+0])/uventries[cartridgeId][event->fin][event->module][0]; 
                                }
                                if (( rawevent.com2h - pedestals[cartridgeId][chipId][module][7] ) < uvthreshold ) {
                                    uventries[cartridgeId][event->fin][event->module][1]++;
                                    (*uu_c[cartridgeId][event->fin])[event->module*2+1]+=(Float_t)(rawevent.u2h- (*uu_c[cartridgeId][event->fin])[event->module*2+1])/uventries[cartridgeId][event->fin][event->module][1]; 
                                    (*vv_c[cartridgeId][event->fin])[event->module*2+1]+=(Float_t)(rawevent.v2h- (*vv_c[cartridgeId][event->fin])[event->module*2+1])/uventries[cartridgeId][event->fin][event->module][1]; 
                                }
                            } // apd =0 || apd =1 
                        } // if uvcalc

                    } // trigcode
                } // for loop iii


                //                parsePack(packBuffer);
                packBuffer.clear();
            }

            //	  cout << " Packet Processed " << endl;
        } // loop over i

        if (totalPckCnt) {
            cout << " File Processed. Dropped: " << droppedPckCnt << " out of " << totalPckCnt << " (=" << setprecision(2) << 100*droppedPckCnt/totalPckCnt <<" %)." << endl;
        }

        if (droppedPckCnt) {
            cout << setprecision(1) << fixed;
            cout << "                 StartCode: " << (Float_t )100*droppedFirstLast/droppedPckCnt << " %" ;
            cout << " Out of Range: " << (Float_t) 100*droppedOutOfRange/droppedPckCnt << " %"; 
            cout << " Tigger Code: " << (Float_t) 100*droppedTrigCode/droppedPckCnt << " %" ;
            cout << " Packet Size: " << (Float_t) 100*droppedSize/droppedPckCnt << " %" << endl;}

        if (uvcalc) {
            if (verbose) {
                cout <<  " Averaging the circle Centers " << endl;
            }

            if (verbose) {
                for (c=0;c<CARTRIDGES_PER_PANEL;c++) {
                    for (f=0;f<FINS_PER_CARTRIDGE;f++) {
                        for ( i=0;i<MODULES_PER_FIN;i++) {
                            for ( j=0;j<APDS_PER_MODULE;j++) {
                                cout << " Circle Center Cartridge " << c << " Fin " << f << " Module " << i << " APD " << j << ": "  ;
                                cout << "uu_c = " << (*uu_c[c][f])[i*2+j] << " vv_c = " << (*vv_c[c][f])[i*2+j] << " nn_entries = " << uventries[c][f][i][j] << endl;   
                            } // j
                        } //i
                    }//f
                }//c

            } // verbose

        } // uvcalc


        // Calculate pedestal

        //pedestal analysis
        if (calcpedestal==1) {
            sprintf(pedfilename,"%s",pedfilebase);
            pedfile.open(pedfilename);
            if (verbose)  cout << "Calculating pedestal values "  << endl;  


            double ped_ana[CARTRIDGES_PER_PANEL][RENAS_PER_CARTRIDGE][MODULES_PER_RENA][CHANNELS_PER_MODULE][2]={{{{{0}}}}};
            int ped_evts[CARTRIDGES_PER_PANEL][RENAS_PER_CARTRIDGE][MODULES_PER_RENA]={{{0}}};

            // skip first 20 entries to prevent outliers
            for (int ii = 20; ii< rawdata->GetEntries(); ii++ ) {

                rawdata->GetEntry(ii);

                chip= rawevent.chip;
                module = rawevent.module;
                cartridge = rawevent.cartridge;

                ped_evts[cartridge][chip][module]++;
                pedana( &ped_ana[cartridge][chip][module][0][0], &ped_ana[cartridge][chip][module][0][1], &ped_evts[cartridge][chip][module], rawevent.a );
                pedana( &ped_ana[cartridge][chip][module][1][0], &ped_ana[cartridge][chip][module][1][1], &ped_evts[cartridge][chip][module], rawevent.b );
                pedana( &ped_ana[cartridge][chip][module][2][0], &ped_ana[cartridge][chip][module][2][1], &ped_evts[cartridge][chip][module], rawevent.c );
                pedana( &ped_ana[cartridge][chip][module][3][0], &ped_ana[cartridge][chip][module][3][1], &ped_evts[cartridge][chip][module], rawevent.d );
                pedana( &ped_ana[cartridge][chip][module][4][0], &ped_ana[cartridge][chip][module][4][1], &ped_evts[cartridge][chip][module], rawevent.com1 );
                pedana( &ped_ana[cartridge][chip][module][5][0], &ped_ana[cartridge][chip][module][5][1], &ped_evts[cartridge][chip][module], rawevent.com1h );
                pedana( &ped_ana[cartridge][chip][module][6][0], &ped_ana[cartridge][chip][module][6][1], &ped_evts[cartridge][chip][module], rawevent.com2 );
                pedana( &ped_ana[cartridge][chip][module][7][0], &ped_ana[cartridge][chip][module][7][1], &ped_evts[cartridge][chip][module], rawevent.com2h );

            }

            if (verbose) {
                cout << " pedana done. " << endl;
            }


            for (c=0;c<CARTRIDGES_PER_PANEL;c++) {
                for (r=0; r<RENAS_PER_CARTRIDGE;r++) {
                    for (i=0;i<MODULES_PER_RENA;i++) {
                        pedfile << "C" << c << "R" << r << "M" << i  << " " << ped_evts[c][r][i] << " ";    
                        if (r < 10 ) pedfile << " ";
                        pedfile << std::setw(6) ;
                        for (int jjj=0;jjj<CHANNELS_PER_MODULE;jjj++) {
                            pedfile << std::setprecision(0) << std::fixed;
                            pedfile << ped_ana[c][r][i][jjj][0] << " ";
                            pedfile <<  std::setprecision(2) << std::fixed << std::setw(6);
                            if ( ped_evts[c][r][i] ) {
                                pedfile  << TMath::Sqrt(ped_ana[c][r][i][jjj][1]/ped_evts[c][r][i]) << " ";
                            } else {
                                pedfile << "0 " ;
                            }
                            pedfile <<  std::setw(6);
                        }
                        pedfile << endl;
                    }
                } // r
            } // c
            pedfile.close();

        } // if ( dopedestal )




        // need to write histograms to disk ::
        // FIXME :: in principle we could fill histograms even without pedestal subtraction
        if (pedfilenamespec ) {

            Int_t totaldoubletriggers=0;
            Int_t totalbelowthreshold=0;



            for (c=0;c<CARTRIDGES_PER_PANEL;c++) {
                for (f=0;f<FINS_PER_CARTRIDGE;f++) {
                    sprintf(tmpstring,"C%dF%d",c,f);
                    subdir[c][f] = hfile->mkdir(tmpstring);
                    subdir[c][f]->cd();
                    for (m=0;m<MODULES_PER_FIN;m++) {
                        for ( j=0;j<APDS_PER_MODULE;j++) {
                            E[c][f][m][j]->Write();
                            E_com[c][f][m][j]->Write();
                        } //j
                    } //m
                } //f
            } // c


            cout << "========== Double Triggers =============== " << endl;
            for (c=0;c<CARTRIDGES_PER_PANEL;c++){
                for (r=0;r<RENAS_PER_CARTRIDGE;r++){
                    for ( i=0;i<MODULES_PER_RENA;i++ ) {
                        cout << doubletriggers[c][r][i] << " " ; 
                        totaldoubletriggers+=doubletriggers[c][r][i]; 
                        totalbelowthreshold+=belowthreshold[c][r][i]; 
                    } // i
                    cout << endl;
                } //r
            } //c

            cout << "========== Total Triggers =============== " << endl;
            Long64_t totalmoduletriggers[CARTRIDGES_PER_PANEL][RENAS_PER_CARTRIDGE][MODULES_PER_RENA]={{{0}}};
            Long64_t totalchiptriggers[CARTRIDGES_PER_PANEL][RENAS_PER_CARTRIDGE]={{0}};
            Long64_t totalacceptedtriggers=0;
            for (c=0;c<CARTRIDGES_PER_PANEL;c++) {
                for (r=0;r<RENAS_PER_CARTRIDGE;r++) {
                    for ( i=0;i<MODULES_PER_RENA;i++ ) {
                        cout << totaltriggers[c][r][i][0] << " " << totaltriggers[c][r][i][1] << " "; 
                        totalmoduletriggers[c][r][i]=totaltriggers[c][r][i][0]+totaltriggers[c][r][i][1];
                        cout << totalmoduletriggers[c][r][i] << " " ;
                        totalchiptriggers[c][r]+=totalmoduletriggers[c][r][i];
                    } // loop over i
                    cout << " || " << totalchiptriggers[c][r]  << endl;
                    totalacceptedtriggers+=totalchiptriggers[c][r]    ;
                } // loop over r
            } //loop over c

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


        } // pedfilenamespec

        if (uvcalc) {
            hfile->cd();
            TDirectory *timing =  hfile->mkdir("timing");


            timing->cd();

            // need to store uvcenters ::
            for (c=0;c<CARTRIDGES_PER_PANEL;c++) {
                for (f=0;f<FINS_PER_CARTRIDGE;f++) {
                    sprintf(tmpstring,"uu_c[%d][%d]",c,f);
                    uu_c[c][f]->Write(tmpstring);
                    sprintf(tmpstring,"vv_c[%d][%d]",c,f);
                    vv_c[c][f]->Write(tmpstring);
                } //r
            } //c

        } // uvcalc




        if (debugmode) {
            hfile->cd(); rawdata->Write();
        }
        hfile->Close();
        if (verbose) {
            cout << " byteCounter = " << byteCounter << endl;
            cout << " totalPckCnt = " << totalPckCnt << endl;
            cout << " droppedPckCnt = " << droppedPckCnt << endl;
        }
        // should match if (datafile)
    } else {
        cout << "Error opening File " << filename << endl;
        return(-1);    // error opening file
    }


    return(0);
}

void getfinmodule(Short_t panel, Short_t chip, Short_t &module, Short_t &fin) {
    Short_t fourup=TMath::Floor(chip/8);
    Short_t localchip=chip%8;
    fin = 6-2*TMath::Floor(localchip/2) ;
    //   cout << " locchip " << localchip << " fin " << fin << endl;
    //      cout << " P" << panel << "R" << chip << "M"<< module << " (locchip: " << localchip << ", fourup: "<< fourup ;

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
        if (chip>=16) fin++; 
        if (chip%2) module+=MODULES_PER_RENA; 
        if (fourup%2)  module+=(MODULES_PER_FIN/2 );
    }

    //   cout << "): fin " << fin << " module " << module << endl; 
}

Int_t pedana( double* mean, double* rms, int*  events, Short_t value ) {

    double val = value;

    // if the value is an outlier (or < 0) , then we reset the value to the current mean. 
    // Otherwise I had to include an event array for every channel value

    if ( value < 0 ) { // *events--; 
        val=*mean;
        //      return 0;
    }

    if (*events > 10) {
        if ( TMath::Abs( value - *mean ) >  12*(TMath::Sqrt(*rms /(*events)) )) { 
            val=*mean;
            //       *events--;
            //    return 0;
        }
    }


    double tmp=*mean;
    *mean+=(val-tmp)/(*events);
    *rms+=(val-*mean)*(val-tmp);
    return(0);
}


Float_t finecalc(Short_t u, Short_t v, Float_t u_cent, Float_t v_cent){
    Float_t tmp;
    tmp=TMath::ATan2(u-u_cent,v-v_cent);
    if (tmp < 0. ) {
        tmp+=2*3.141592;
    }
    return(tmp);///(2*3.141592*CIRCLEFREQUENCY);
}
