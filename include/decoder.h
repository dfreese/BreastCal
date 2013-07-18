#ifndef ROOTPLOTTER_H
#define ROOTPLOTTER_H

#include <fstream>
#include <string>


#include "config.h"
#include "util.h"
//#define _HOMEDIR=/Users/arne
//FIXME :: need something more robust
//#include "/Users/arne/root/libInit_avdb.h"
//#include "/Users/arne/root/macros/myrootlib.h"
#include "libInit_avdb.h"
#include "myrootlib.h"



ifstream dataFile;
streampos lastPos;
streampos endPos;
//vector<char> currentPack;
vector<char> packBuffer;
unsigned long long int byteCounter;
#define PAULS_PANELID true
//#define MODULE_BASED_READOUT false
#define MODULE_BASED_READOUT true
//#define NUM_RENA_PER_DAQ 8

#define CHANPERMODULE 8
#define RENACHIPS 8
#define MODULES 4
#define RENACHIPS 8
#define MINENTRIES 10000
#define MINEHISTENTRIES 10

#define FOURUPBOARDS 4
#define MAXFILELENGTH      160
#define E_up 3000
#define E_low -100
#define Ebins 310
#define E_up_com 1400
#define E_low_com -100
#define Ebins_com 150
#define PP_LOW_EDGE 600
#define PP_UP_EDGE 2800
#define PP_LOW_EDGE_COM 350
#define PP_UP_EDGE_COM 1200

#define MINHISTENTRIES 200

#define FILENAMELENGTH	120
#define MAXFILELENGTH	160
#define PEAKS 64

#define EFITMIN  0 
#define EFITMAX  2400

#define EFITMIN_COM  0 
#define EFITMAX_COM  1200


#define MAXHITS 5000000
#define MAXCUTS 50
#define DEFAULTTHRESHOLD -600;
#define INFTY 1e99;

#define EGATEMIN 400
#define EGATEMAX 650
// rollovertime = 2^42 
#define ROLLOVERTIME 4398046511104 

//TROOT::gErrorIgnoreLevel=1001;
  //TROOT::gErrorIgnoreLevel = 1001;


			     /*
struct modulecal
{ Long64_t ct;
  Short_t chip;
  Short_t module;
  Short_t apd;
  Float_t Ecal;
  Float_t E;
  Float_t Ec;
  Float_t Ech;
  Float_t x;
  Float_t y;
  Float_t ft;
  Int_t pos;
  Short_t id;
  //  Double_t ft2;
  //  Short_t id;
  //  Int_t apd;

 };



struct modmerge
{ Long64_t ct;
  Double_t ft;
  Double_t E;
  Double_t Ec;
  Double_t Ech;
  Double_t x;
  Double_t y;
  Int_t chip;
  Int_t fin;
  Int_t m;
  Int_t apd;
  Int_t id;
  Int_t pos;
 };

*/

 /*
struct modmerge
{ Long64_t ct;
 // these all can be Floats 
  Double_t ft;
  Float_t E;
  Float_t Ec;
  Float_t Ech;
  Float_t x;
  Float_t y;
 // Chip can be short for one panel 
  Short_t chip;
// These can be shorts ( short goes to  128 ) 
  Short_t fin;
  Short_t m;
  Short_t apd;
  Short_t id;
// Has to be Int_t 
  Int_t pos;
 };
 */

struct chipevent
{ Long64_t ct;
  Int_t pos;
  Short_t chip;
  Short_t module;
  Short_t com1;
  Short_t com2;
  Short_t com1h;
  Short_t com2h;
  Short_t u1;
  Short_t v1;
  Short_t u2;
  Short_t v2;
  Short_t u1h;
  Short_t v1h;
  Short_t u2h;
  Short_t v2h;
  Short_t a;
  Short_t b;
  Short_t c;
  Short_t d;

};


/*
#include "TCanvas.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TTree.h"
#include "TFile.h"
#include "TLine.h"

#include "device.h"
#include "param.h"
#include "config.h"
#include "moduleevent.h"
#include "calibrator.h"
#include "moduleprocessor.h"
#include "circularbuffer.h"
#include "qsingleitemsquarelayout.h"
#include "chipeventratedisplay.h"
//#include "modulehist.h"
*/

namespace Ui {
    class RootPlotter;
}


using namespace std;

// TEMP_BEGIN

/*

class ChannelSetting {
        public:
                bool readout;
                bool readSlowChannel;
                bool readFastChannel;
};
// TEMP_END


*/

/*
class RootPlotter : public QWidget
{
    Q_OBJECT

public:
    explicit RootPlotter(QWidget *parent = 0, Device *dev = 0);
    ~RootPlotter();


signals:
    void histLimitsChanged();
    void usbStatsUpdatedRootPlotter (   unsigned long sent,
                                        unsigned long retransmit,
                                        unsigned long received,
                                        unsigned long packetProcessed,
                                        unsigned long packetDropped,
                                        unsigned long bytesWritten,
                                        unsigned long bytesRead);
    void moduleTestUpdatedByRootPlotter(unsigned long packetProcessed, unsigned long packetDropped);
    void transStageUpdatedByRootPlotter(unsigned long packetProcessed, unsigned long packetDropped);
    void stopParser();
    void updatePlotsChanged(bool state);
    void updateEventRates(vector<unsigned int> *rates);

public slots:
    void parseData();
    void updatePlots();
    void updateMaxEnergy(int value);
    void updateMinEnergy(int value);
    void changeModule(int moduleId);
    void changeApd(int apdId);
    void changeViewMode(int value);
    void updateHistogramRange(int moduleId, int apdId, char type, double valMin, double valMax, bool applyAll = false);
    void clearHistograms();
    void getPedestals();
    void clearPedestals();
    void startOver();
    void autoDetectEnergyLimits(int moduleId, int apdId, bool applyAll = false);
    void autoDetectWindowLimits(int moduleId, int apdId, bool applyAll = false);
    void autoDetectFloodLimits(int moduleId, int apdId, bool applyAll = false);
    void updateUsbStats();
    void updateModuleTest();
    void updateTransStage();
    void changeFloodColor(int value);
    void daqStarted();
    void daqStopped();
    void reprocess();
    void resetPacketCounters();
    void toggleUpdatePlots();
    void requestEventRate();


private:
    Ui::RootPlotter *ui;
    Calibrator *calibrator;
    ModuleProcessor *moduleProcessor;
    CalibrationData *calibrationData;
    void parsePack(const vector<char> &packBuffer, int coin = -1, int usbNum = -1);
    void drawPlots();
    void catchUp();
    void catchUpAdd(int moduleId, int adpId, char type);
    bool catchUpRemove(int moduleId, int apdId, char type);
    bool catchUpRemove(int groupId, char type);
    void catchUpRemove(int index);
    int catchUpFindIndex(int groupId, char type);
    void catchUpClearAll();
    void findHistLimits(TH1F *energyHist, int &lowE, int &highE);
    void pushToBuffer(ModuleEvent event);
    vector<int> catchUpGroupIdVec;
    vector<int> catchUpAccessCodeVec;
    vector<char> catchUpTypeVec;    // E - Energy Hist
                                    // F - Flood Hist
                                    // A - All
    //vector<int> catchUpCntVec;
    //vector<int> catchUpModuleIdVec;
    //vector<int> catchUpApdIdVec;
    CircularBuffer<ModuleEvent> *buffer;

    int currentModuleId;
    int currentApdId;
    int currentViewMode;

    Device *device;
    string filename;
    string filenameLeft;
    string filenameRight;
    vector<string> filenameLeftVec;
    vector<string> filenameRightVec;
    ifstream dataFile;
    ifstream dataFileLeft;
    ifstream dataFileRight;
    vector<ifstream*> dataFileLeftVec;
    vector<ifstream*> dataFileRightVec;
    streampos lastPos;
    streampos lastPosLeft;
    streampos lastPosRight;
    vector<streampos> lastPosLeftVec;
    vector<streampos> lastPosRightVec;
    streampos endPos;
    streampos endPosLeft;
    streampos endPosRight;
    vector<streampos> endPosLeftVec;
    vector<streampos> endPosRightVec;

    vector<char> currentPack;
    vector<char> currentPackLeft;
    vector<char> currentPackRight;
    vector< vector<char> > currentPackLeftVec;
    vector< vector<char> > currentPackRightVec;

    unsigned long droppedPckCnt;
    unsigned long totalPckCnt;
    unsigned long receivedPckCnt; //Get this from DAQSaveThread object.
    TQtWidget *rootWidget00;
    TQtWidget *rootWidget01;
    TQtWidget *rootWidget10;
    TQtWidget *rootWidget11;
    QSingleItemSquareLayout *rootWidgetLayout00;
    QSingleItemSquareLayout *rootWidgetLayout01;
    QSingleItemSquareLayout *rootWidgetLayout10;
    QSingleItemSquareLayout *rootWidgetLayout11;
    TLine *lineL00;
    TLine *lineL01;
    TLine *lineL10;
    TLine *lineL11;
    TLine *lineM00;
    TLine *lineM01;
    TLine *lineM10;
    TLine *lineM11;
    TLine *lineH00;
    TLine *lineH01;
    TLine *lineH10;
    TLine *lineH11;
    bool showPedestals;
    bool showWindows;


    // TEMP_BEGIN
    TH2F *floodHist0;
    TH1F *energyHist0;
    TH2F *floodHist1;
    TH1F *energyHist1;
    TFile *rootFile;
    TTree *eventTree;
    vector<ChannelSetting> channelSettings;
    vector<ChannelEvent> channels;
    int minEnergy;
    int maxEnergy;
    //TEMP_END

    FT_HANDLE ftHandle; //DEBUG - REMOVE LATER
    bool isOpen;    //DEBUG - REMOVE LATER

    int fileCounter;
    int fileCounterLeft;
    int fileCounterRight;
    vector<int> fileCounterLeftVec;
    vector<int> fileCounterRightVec;
    unsigned long long int byteCounter;
    unsigned long long int byteCounterLeft;
    unsigned long long int byteCounterRight;
    vector<unsigned long long int> byteCounterLeftVec;
    vector<unsigned long long int> byteCounterRightVec;

    int binaryFileCounter;
    int binaryFileCounterLeft;
    int binaryFileCounterRight;
    vector<int> binaryFileCounterLeftVec;
    vector<int> binaryFileCounterRightVec;
    unsigned long long int binaryByteCounter;
    unsigned long long int binaryByteCounterLeft;
    unsigned long long int binaryByteCounterRight;
    vector<unsigned long long int> binaryByteCounterLeftVec;
    vector<unsigned long long int> binaryByteCounterRightVec;

    string binaryFilename;
    string binaryFilenameLeft;
    string binaryFilenameRight;
    vector<string> binaryFilenameLeftVec;
    vector<string> binaryFilenameRightVec;

    ofstream binaryFile;
    ofstream binaryFileLeft;
    ofstream binaryFileRight;
    vector<ofstream*> binaryFileLeftVec;
    vector<ofstream*> binaryFileRightVec;

    bool daqRunning;
    void initializeInternalCounters();
    bool updatePlotsFlag;

    vector<unsigned int> chipRate;
};
*/

#endif // ROOTPLOTTER_H
