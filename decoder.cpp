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
#include "./decoder.h"
//#include "./convertconfig.h"


#define FILENAMELENGTH 120
void usage(void);

void usage(void){
  int t=DEFAULTTHRESHOLD;
  cout << " decode  -f [filename] [-v -o [outputfilename] -p  -d -t [threshold] " ;
  cout << " -uv -pedfile [pedfilename] ]" <<endl;
  cout << " -d : debug mode, a tree with unparsed data will be written (non-pedestal corrected). " <<endl;
  cout << "      you get access to u,v" << endl;
  cout << " -t : threshold for hits, default = " << t << endl;
  cout << " -uv: UV circle centers will be calculated " << endl;
  cout << " -pedfile [pedfilename] : pedestal file name " << endl;
  cout << " -p : calculate pedestals " << endl;
  cout << " -o : optional outputfilename " <<endl;
  return;}

using namespace std;
void parsePack(const vector<char> &packBuffer, int coin = -1, int usbNum = -1);
Float_t finecalc(Short_t u, Short_t v, Float_t u_cent, Float_t v_cent);

Int_t pedana( double* mean, double* rms, int  events, Short_t value ) ;

// huh, global variable ..
unsigned long totalPckCnt=0;
unsigned long droppedPckCnt=0;
vector<unsigned int> chipRate;



int main(int argc, char *argv[]){
  float pedestals[RENACHIPS][MODULES][8]={{{0}}};
  Int_t calcpedestal=0;
  int uvcalc=0;
 int chip;
 int module;

 Int_t CHANNELLIST[36]={0};
 Int_t CHANNEL;
 // Long64_t TIMESTAMP;
 Int_t newevent,nrhits;
 // FILE *f;
 TFile *hfile;
 Int_t i,ix,nlines,verbose;
 char filename[FILENAMELENGTH],outfilename[FILENAMELENGTH+5],pedfilebase[FILENAMELENGTH+10], ascifilename[FILENAMELENGTH+6], pedfilename[FILENAMELENGTH+10],pedvaluefilename[FILENAMELENGTH+10];
 int pedfilenamespec=0;

 ofstream pedfile,ascifile;
 int outfileset=0;
 int genascifile=0;
 int filenamespec=0;
 verbose=0;
 Int_t sourcepos=0;
 int threshold=DEFAULTTHRESHOLD;

 int debugmode=0;
 /*
 TTree *rena[RENACHIPS]; 
 Int_t evtsrena[RENACHIPS];
 */

// For compatibility with the original code
CHANNEL = 36;
for (i = 0; i < 36; i++) {
  CHANNELLIST[i] = i;
}
// End

  for ( ix=1;ix< argc;ix++){
    //    cout << argv[ix] << endl;
    /* Verbose  '-v' */
    if (strncmp(argv[ix], "-v", 2) == 0) {
      verbose = 1;
    }

    if (strncmp(argv[ix], "-d", 2) == 0) {
      debugmode = 1;
    }

    if (strncmp(argv[ix], "-uv", 3) == 0) {
      uvcalc = 1;
    }

    if (strncmp(argv[ix], "-t", 2) == 0) {
      threshold = atoi(argv[ix+1]);
       ix++;
    }


    if (strncmp(argv[ix], "-pedfile", 8) == 0) {
      ix++;
       if (strlen(argv[ix])<FILENAMELENGTH) {  
	sprintf( pedvaluefilename,"%s", argv[ix]);
        pedfilenamespec=1; }
        else  {
        cout << "Filename " << argv[ix] << " too long !" <<endl;
        cout << "Exiting.." <<endl;
	return -99;}
    }


    if (strncmp(argv[ix],"-pos",4) == 0){
        sourcepos=atoi(argv[ix+1]);
	//        cout << " sourcepos :: " << sourcepos << endl;
	ix++;
      }

    /* Pedestal  '-p' -- needs to come after -pedfile and -pos !!*/
    if (strncmp(argv[ix], "-p", 2) == 0) {
      calcpedestal = 1;
    }

    /* filename '-f' */
    if (strncmp(argv[ix], "-f", 2) == 0){
      if (strlen(argv[ix+1])<FILENAMELENGTH) {  
	sprintf( filename,"%s", argv[ix+1]);
        filenamespec=1;}
      else  {
        cout << "Filename " << argv[ix+1] << " too long !" <<endl;
        cout << "Exiting.." <<endl;
	return -99;}
    }
 
  /* Outputfile  '-o' */

    if (strncmp(argv[ix], "-o", 2) == 0) {
      outfileset = 1;
      if (strlen(argv[ix+1])<FILENAMELENGTH) {  
	sprintf( outfilename,"%s", argv[ix+1]);
        sprintf( pedfilebase,"%s.ped", argv[ix+1]);
	//        sprintf( pedfilename2,"%s.ped.RENA2", argv[ix+1]);
        sprintf( ascifilename,"%s.ascii",argv[ix+1]);
    }
      else  {
        cout << "Output Filename " << argv[ix+1] << " too long !" <<endl;
        cout << "Exiting.." <<endl;
	return -98;}
    }

  /* Outputfile  '-o' */
    if (strncmp(argv[ix], "-a", 2) == 0) { genascifile =1 ;}

  
  } // loop input arguments

  if (!(filenamespec)) { printf("Please Specify Filename !!\n");
    usage();
    return(-1);
  }

  if (pedfilenamespec) {
     ifstream pedvals;
     int events;
     double tmp;
     pedvals.open(pedvaluefilename);
     while ( pedvals >> chip ){ 
       pedvals >> module;
       pedvals >> events;
       if (( chip >= RENACHIPS )||(module >= MODULES )) {
	 cout << " Error reading pedfile, module or chipnumber too high: " << endl;
         cout << " module = " << module << ", chip = " << chip << ".\nExiting." << endl;
         return -2;
       }
       for ( int ii=0; ii< 8;ii++){
	 pedvals >> pedestals[chip][module][ii] ;
         pedvals >> tmp;
       } 
   }

     if (verbose) { cout << " Pedestal values :: " << endl;
     for (int ii=0 ; ii < RENACHIPS ; ii++ ){
       for (int jj=0 ; jj < MODULES ; jj++ ){ 
         for (int kk=0 ; kk <8;kk++){
	 cout << pedestals[ii][jj][kk] << " " ;
         }
	 cout << endl;
       }
     } // loop ii
     } // verbose
  } //pedfilenamespec
 
   
  if (verbose) {
    cout << " threshold :: " << threshold << endl; }
  

  if (!(outfileset)) {
     sprintf(outfilename,"%s.root",filename);
     sprintf(pedfilebase,"%s.ped",filename);
     //     sprintf(pedfilename2,"%s.ped.RENA2",filename);
     sprintf(ascifilename,"%s.ascii",filename);}


  if (genascifile){ ascifile.open(ascifilename);}
  /*
 printf("Conversion program for Rena Data\n");
 printf("On this machine one short is %ld bytes\n",sizeof(short));
 printf("On this machine Timestamp is %ld bytes\n",sizeof(Long64_t));
 printf("On this machine Integer is %ld bytes\n",sizeof(Int_t));
 printf("Converting the File :::::: %s\n",filename);
  */
 hfile = new TFile(outfilename,"RECREATE"); 

 Char_t treename[10],treetitle[40];
 chipevent rawevent;
 TTree *rawdata; 

 // for ( i=0; i < RENACHIPS; i++ ){
  sprintf(treename,"rawdata");
  sprintf(treetitle,"Converted Raw RENA data" );
  rawdata =  new TTree(treename,treetitle);
  rawdata->Branch("ct",&rawevent.ct,"ct/L");
  rawdata->Branch("chip",&rawevent.chip,"chip/S");
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
  //}




   dataFile.open(filename, ios::in | ios::binary);
  if (!dataFile.good()) {
    cout << "Cannot open file \"" << filename << "\" for read operation. Exiting." << endl;
    return -1;     }

  if (dataFile)
 {  // for ( i=0;i<10;i++){
  nlines=0;
  newevent=1;
  nrhits=0;
  /*
  Int_t j;
  Int_t events;
  Int_t intbuff;
  char delim = 'X';
  */
  char c;
   dataFile.seekg(0, ios::end);
   endPos = dataFile.tellg();
   dataFile.seekg(lastPos);
   if (verbose){
   cout << " endPos :: " << endPos << endl;
   cout << " lastPos :: " << lastPos << endl;
   }

    int chipId = 0;
    int trigCode = 0;


    //for (int i=0; i<10000; i++) { //fast return
   //      if (i>=(endPos-lastPos))
    //           break;
 for (int i=0; i<(endPos-lastPos-1); i++) { //slow return
            dataFile.get(c);
            packBuffer.push_back(c);
            byteCounter++;
          if ((unsigned char)c==0x81) {

	    

                // end of package (start processing)
                 totalPckCnt++;
 
		 //   if (totalPckCnt%100==0) {
	    //          cout << totalPckCnt << " (" << droppedPckCnt << ")" << endl;    }


    if (PAULS_PANELID) {
        //if (USB_VER == USB_2_0) {
        // ChipId format changed on 06/05/2012
        vector<bool> fpgaIdVec;
        fpgaIdVec.push_back((packBuffer[2] & 0x20) != 0);
        fpgaIdVec.push_back((packBuffer[2] & 0x10) != 0);
        int fpgaId = Util::boolVec2Int(fpgaIdVec);
        chipId = 2*fpgaId;
        if ((packBuffer[2] & 0x40) != 0) {
            chipId++;
        }
        trigCode = int(packBuffer[2] & 0x0F);
	//		        cout << "trigCode = 0x" << hex << trigCode << dec ;
	//		cout << " chipId = " << chipId << "; fpgaId = " << fpgaId << endl;
        //}
    }
    else {
        //if (USB_VER == USB_2_0) {
        // This is the old ChipId format which can be used to process old data
        vector<bool> fpgaIdVec;
        fpgaIdVec.push_back((packBuffer[1] & 0x10) != 0);
        fpgaIdVec.push_back((packBuffer[1] & 0x08) != 0);
        int fpgaId = Util::boolVec2Int(fpgaIdVec);
        chipId = 2*fpgaId;
        if ((packBuffer[1] & 0x20) != 0) {
            chipId++;
        }
        //}
    }
    /*
    if (coin == 1) {
        chipId += Config::numChipPerPanel;
    }

    if (usbNum != -1) {
        chipId += usbNum*Config::NUM_RENA_PER_DAQ;
    }

    */

    // At this point we extracted the chipId from the packet
    // Now we need to get the who_triggered bits


    //cout << "coin = " << coin << " " << "chipId = " << chipId << endl;

    //  RMChip* rmChip = device->readoutMap->getChipById(chipId);

    //RENAChip *currentChip = device->getRENAChipPtr(chipId);

    //if (packBuffer.size()!=device->getPackSize()) {
    //    droppedPckCnt++;
    //    return;
	    // }

	    
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
      //  packetSize = rmChip->packetSize;
      packetSize=146; //FIXME
      moduletriggers=4;
    }
    else { // MODULE_BASED_READOUT
        if (trigCode == 0) {
            droppedPckCnt++;
            continue;
        }
        if (PAULS_PANELID) { packetSize =10; }
        else packetSize = 9;
	//	packetSize += 4*2 ;
        for (int ii=0;ii<4;ii++) {
	  packetSize +=  32*( ( trigCode >> ii ) &  0x1  ) ;
        	moduletriggers +=  ( ( trigCode >> ii ) & 0x1) ;}
        // packetSize = rmChip->packetSizeMap[trigCode];
    } // else module based readout
    //        cout << " packetSize :: " << packetSize  << "; packBuffer.size() = " << packBuffer.size() << endl;
    
    

    // FIXME  BAD HARDCODED VALUE 
    
    //   packetSize=146; // need to figure this out one way or another ..

    if (packBuffer.size()!=packetSize) {
        droppedPckCnt++;
        cout << "Dropped = " << packBuffer.size() << endl;
        packBuffer.clear();
        continue;
    }

    
    //  chipRate[chipId] = chipRate[chipId] + 1;
    
    vector<Short_t> adcBlock;

    // Parse the package here!
    Long64_t timestamp;
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
        timestamp = 0;
        for (int ii=3; ii<=8; ii++){
            timestamp = timestamp << 7;
            timestamp += Long64_t(packBuffer[ii]);
        }
    }
    else {
        timestamp = 0;
        for (int ii=2; ii<=7; ii++){
            timestamp = timestamp << 7;
            timestamp += Long64_t(packBuffer[ii]);
        }
    }
    

    // Remaining bytes are ADC data for each channel
    if (PAULS_PANELID) {
        Short_t value;
        for (unsigned int counter = 9; counter<(packetSize-1); counter+=2) {
            value = (Short_t)packBuffer[counter];
            value = value << 6;
            value += (Short_t)packBuffer[counter+1];
            adcBlock.push_back(value);
        }
    }
    else {
        Short_t value;
        for (unsigned int counter = 8; counter<(packetSize-1); counter+=2) {
            value = (Short_t)packBuffer[counter];
            value = value << 6;
            value += (Short_t)packBuffer[counter+1];
            adcBlock.push_back(value);
        }
    }

    //    cout << " Size ADCBLOCK :: " << adcBlock.size() << endl; 

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

    for (int ii=0;ii<4;ii++) { nrchips+= (( trigCode >> ii ) & ( 0x1 ) );}

    // if ( chipId == 1 ){

/*
   cout << "Dropped = " << droppedPckCnt << endl;
        for (int iii=0; iii<adcBlock.size(); iii++) {
	  cout << iii << " = " << adcBlock[iii] << " | ";
          if ( (( iii+1 )%12 )==0 ) cout << endl;
        }
        cout << endl;
 */
#define SHIFTFACCOM 4  // 4*4
#define SHIFTFACSPAT 12 // 4*12
#define BYTESPERCOMMON 12   // 4 * 3 = 12 ( 4 commons per module, 3 values per common )
#define VALUESPERSPATIAL 4

    //#define UNUSEDCHANNELOFFSET 2
#define UNUSEDCHANNELOFFSET 0

      /*
      cout << " adcBlock.size() :: " << adcBlock.size() << " ( nrmodules :: " << nrchips << " )" << endl;
      for (int iii=0;iii<adcBlock.size();iii++){
	cout << " " << adcBlock[iii];
        if (( (iii+1)%16)==0 ) cout << endl;
      }
      cout << endl;
      */
    int kk=0;
    for (int iii=0;iii<4;iii++ ) {
      if ( trigCode & ( 0x1 << iii )) {
              rawevent.ct=timestamp;
              rawevent.chip= chipId;
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
	      //	      cout << " module :: " << rawevent.module << endl;
      } // trigcode
    } // for loop iii

    //  } // chipID

	    //                parsePack(packBuffer);
                packBuffer.clear();
            }
 } // loop over ii

 // Calculate pedestal



  //pedestal analysis
  if (calcpedestal==1){
      sprintf(pedfilename,"%s",pedfilebase);
      pedfile.open(pedfilename);
      if (verbose)  cout << "Calculating pedestal values "  << endl;  


    double ped_ana[RENACHIPS][MODULES][CHANPERMODULE][2]={{{{0}}}};
    int ped_evts[RENACHIPS][MODULES]={{0}};


 for (int  ii = 0 ; ii< rawdata->GetEntries(); ii++ ){
   rawdata->GetEntry(ii);
   chip= rawevent.chip;
   module = rawevent.module;
   //   if (rawevent.module==0){
   //     cout << " ped_evts[ " << chip << "][" << module << "] :::: "<< ped_evts[chip][module] << endl;
     ped_evts[chip][module]++;
     pedana( &ped_ana[chip][module][0][0], &ped_ana[chip][module][0][1], ped_evts[chip][module], rawevent.a );
     pedana( &ped_ana[chip][module][1][0], &ped_ana[chip][module][1][1], ped_evts[chip][module], rawevent.b );
     pedana( &ped_ana[chip][module][2][0], &ped_ana[chip][module][2][1], ped_evts[chip][module], rawevent.c );
     pedana( &ped_ana[chip][module][3][0], &ped_ana[chip][module][3][1], ped_evts[chip][module], rawevent.d );
     pedana( &ped_ana[chip][module][4][0], &ped_ana[chip][module][4][1], ped_evts[chip][module], rawevent.com1 );
     pedana( &ped_ana[chip][module][5][0], &ped_ana[chip][module][5][1], ped_evts[chip][module], rawevent.com1h );
     pedana( &ped_ana[chip][module][6][0], &ped_ana[chip][module][6][1], ped_evts[chip][module], rawevent.com2 );
     pedana( &ped_ana[chip][module][7][0], &ped_ana[chip][module][7][1], ped_evts[chip][module], rawevent.com2h );
   }

 
 for (int j=0; j<RENACHIPS;j++){
   for (int jj=0;jj<MODULES;jj++){
     pedfile << j << " " << jj << " " << ped_evts[j][jj] << " ";    
       for (int jjj=0;jjj<8;jjj++){
	 pedfile << ped_ana[j][jj][jjj][0] << " ";
         if ( ped_evts[j][jj] ) pedfile  << TMath::Sqrt(ped_ana[j][jj][jjj][1]/ped_evts[j][jj]) << " ";
         else pedfile << "0 " ;
       }
       pedfile << endl;
     }
   }
 cout << "file:: " << filename << " ped :: " << ped_ana[1][0][0][0] << " RMS :: " << TMath::Sqrt(ped_ana[1][0][0][1]/ped_evts[1][0]);
 cout << " ( n= " << ped_evts[1][0] << ")" << endl;
 pedfile.close();

  } // if ( dopedestal )



  
  Float_t uu_c[RENACHIPS][MODULES][2]={{{0}}};
  Float_t vv_c[RENACHIPS][MODULES][2]={{{0}}};
  Long64_t uventries[RENACHIPS][MODULES][2]={{{0}}};
  //  int pedestal[RENACHIPS][MODULES][8];

  if (uvcalc){

    // to calculate circle centers we need to make sure the module triggered, so we use a high threshold
    int uvthreshold = -1000;
   

    if (verbose)    cout << " Calculating UV " << " entries :: " << rawdata->GetEntries() << endl;
 for (int  ii = 0 ; ii< rawdata->GetEntries(); ii++ ){
   rawdata->GetEntry(ii);
   chip = rawevent.chip;
   module = rawevent.module;
   if (( ii < 20 )&&(verbose)) {
     cout << " Com1h : " << rawevent.com1h << " pedestal: " << pedestals[chip][module][5] ;
     cout << " Com1h-ped: " << rawevent.com1h - pedestals[chip][module][5]  << " valid: ";
     cout <<  ((rawevent.com1h - pedestals[chip][module][5]) < uvthreshold) << endl ;
   }
   if (  ( rawevent.com1h - pedestals[chip][module][5] ) < uvthreshold ) {
     uventries[chip][module][0]++;
     uu_c[chip][module][0]+=(Float_t)(rawevent.u1h-uu_c[chip][module][0])/uventries[chip][module][0]; 
     vv_c[chip][module][0]+=(Float_t)(rawevent.v1h-vv_c[chip][module][0])/uventries[chip][module][0]; 
    }
   if (( rawevent.com2h - pedestals[chip][module][7] ) < uvthreshold ) {
      uventries[chip][module][1]++;
     uu_c[chip][module][1]+=(Float_t)(rawevent.u2h-uu_c[chip][module][1])/uventries[chip][module][1]; 
     vv_c[chip][module][1]+=(Float_t)(rawevent.v2h-vv_c[chip][module][1])/uventries[chip][module][1]; 
   }
 } // loop over Entries ( ii) 

 if (verbose)   cout <<  " Averaging the circle Centers " << endl;

if (verbose){
 for (int jj=0;jj<RENACHIPS;jj++){
   for (int ii=0;ii<MODULES;ii++) {
     for (int mm=0;mm<2;mm++) {
       cout << " Circle Center Chip " << jj << " Module " << ii << " APD " << mm << ": "  ;
       cout << "uu_c = " << uu_c[jj][ii][mm] << " vv_c = " << vv_c[jj][ii][mm] << " nn_entries = ";
       cout << uventries[jj][ii][mm] << endl;   }
   }
 }
 } // verbose

  } // uvcalc


if (pedfilenamespec) {

 moduledat event;
 TTree *mdata; 
 int doubletriggers[RENACHIPS][MODULES]={{0}};
 int totaltriggers[RENACHIPS][MODULES][2]={{{0}}};
 // for ( i=0; i < RENACHIPS; i++ ){
  sprintf(treename,"mdata");
  sprintf(treetitle,"Converted RENA data" );
  mdata =  new TTree(treename,treetitle);
  mdata->Branch("ct",&event.ct,"ct/L");
  mdata->Branch("chip",&event.chip,"chip/S");
  mdata->Branch("module",&event.module,"module/S");
  mdata->Branch("apd",&event.apd,"apd/S");
  mdata->Branch("Ec",&event.Ec,"Ec/F");
  mdata->Branch("Ech",&event.Ech,"Ech/F");
  mdata->Branch("x",&event.x,"x/F");
  mdata->Branch("y",&event.y,"y/F");
  mdata->Branch("E",&event.E,"E/F");
  mdata->Branch("ft",&event.ft,"ft/F");
  mdata->Branch("a",&event.a,"a/S");
  mdata->Branch("b",&event.b,"b/S");
  mdata->Branch("c",&event.c,"c/S");
  mdata->Branch("d",&event.d,"d/S");
  mdata->Branch("id",&event.id,"id/S");
  mdata->Branch("pos",&event.pos,"pos/I");
  //}
for (int  ii = 0 ; ii< rawdata->GetEntries(); ii++ ){
   rawdata->GetEntry(ii);
   chip=rawevent.chip;
   module=rawevent.module;
   event.ct=rawevent.ct;
   event.chip=rawevent.chip;
   event.module=rawevent.module;
   event.a=rawevent.a - pedestals[chip][module][0];
   event.b=rawevent.b - pedestals[chip][module][1];
   event.c=rawevent.c - pedestals[chip][module][2];
   event.d=rawevent.d - pedestals[chip][module][3];
   event.E=event.a+event.b+event.c+event.d;
   event.x= event.a + event.d - ( event.b + event.c );
   event.y= event.c + event.d - ( event.b + event.a );
   event.x/=event.E;
   event.y/=event.E;


   event.id=-1;
   event.pos=rawevent.pos;

   if ( (rawevent.com1h - pedestals[chip][module][5]) < threshold ) { 
     if ( (rawevent.com2h - pedestals[chip][module][7]) > threshold ) { 
       totaltriggers[chip][module][0]++;
       event.apd=0; 
       event.ft=finecalc(rawevent.u1h,rawevent.v1h,uu_c[chip][module][0],vv_c[chip][module][0])  ;
       event.Ec= rawevent.com1 - pedestals[chip][module][4];
       event.Ech=rawevent.com1h - pedestals[chip][module][5];
       mdata->Fill(); }
     else doubletriggers[chip][module]++;
   }
   else {
     if ( (rawevent.com2h - pedestals[chip][module][5]) < threshold ) {   
     totaltriggers[chip][module][1]++;
     event.apd=1;   
     event.ft=finecalc(rawevent.u2h,rawevent.v2h,uu_c[chip][module][1],vv_c[chip][module][1])  ;
     event.Ec = rawevent.com2 - pedestals[chip][module][6];
     event.Ech=rawevent.com2h- pedestals[chip][module][7];
     mdata->Fill(); } }
 } // loop over entries ii


 cout << "========== Double Triggers =============== " << endl;
 for (int ii = 0 ;ii < RENACHIPS; ii++ ){
   for ( int jj = 0 ; jj < MODULES; jj++ ) {
     cout << doubletriggers[ii][jj] << " " ; }
   cout << endl;
 }

 cout << "========== Total Triggers =============== " << endl;
 Long64_t totalmoduletriggers[RENACHIPS][MODULES]={{0}};
 Long64_t totalchiptriggers[RENACHIPS]={0};
 Long64_t totalacceptedtriggers=0;
 for (int ii = 0 ;ii < RENACHIPS; ii++ ){
   for ( int jj = 0 ; jj < MODULES; jj++ ) {
     cout << totaltriggers[ii][jj][0] << " " << totaltriggers[ii][jj][1] << " "; 
     totalmoduletriggers[ii][jj]=totaltriggers[ii][jj][0]+totaltriggers[ii][jj][1];
     cout << totalmoduletriggers[ii][jj] << " " ;
     totalchiptriggers[ii]+=totalmoduletriggers[ii][jj];
    }
   cout << " || " << totalchiptriggers[ii]  << endl;
   totalacceptedtriggers+=totalchiptriggers[ii]    ;
     } // loop over ii 

 cout << " Total events :: " << rawdata->GetEntries() <<  " Total accepted :: " << totalacceptedtriggers ;
 cout << " ( = " << 100* (float) totalacceptedtriggers/rawdata->GetEntries() << " %) " << endl;
 mdata->Write();

  } // pedestal correction 


  if (debugmode) rawdata->Write();
 hfile->Close();
 if (verbose){
 cout << " byteCounter = " << byteCounter << endl;
 cout << " totalPckCnt = " << totalPckCnt << endl;
 cout << " droppedPckCnt = " << droppedPckCnt << endl;
 }

  }
 else 
  {
    cout << "Error opening File " << filename << endl;
    return -1;    // error opening file
   }

 
  return 0;}


 Int_t pedana( double* mean, double* rms, int  events, Short_t value ) {
   double tmp=*mean;
   *mean+=(value-tmp)/events;
   *rms+=(value-*mean)*(value-tmp);
   return 0;
 }


Float_t finecalc(Short_t u, Short_t v, Float_t u_cent, Float_t v_cent){
  Float_t tmp;
  tmp=TMath::ATan2(u-u_cent,v-v_cent);
  if (tmp < 0. ) { tmp+=2*3.141592;}
  return tmp;///(2*3.141592*CIRCLEFREQUENCY);
}
