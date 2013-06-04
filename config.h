#ifndef CONFIG_H
#define CONFIG_H

#define CONFIG_FILENAME "System_Config.nfo"
#define CHANNEL_MAP_FILENAME "Channel_Map.nfo"
#define DEF_CONFIG_FILENAME "Default_Channel_Settings.nfo"
#define MODULE_SETTINGS_FILENAME "Module_Settings.nfo"
#define PEDESTAL_FILENAME_PRE "Pedestals"
#define PEDESTAL_FILENAME_EXT ".nfo"
#define CRYSTAL_LOC_FILENAME_PRE "CrystalLoc"
#define CRYSTAL_LOC_FILENAME_EXT ".xml"
#define CRYSTAL_GAIN_FILENAME_PRE "CrystalGain"
#define CRYSTAL_GAIN_FILENAME_EXT ".xml"
#define BINARY_DATA_FILENAME_PRE "BinaryData"
#define BINARY_DATA_FILENAME_EXT ".out"
#define READOUT_MAP_FILENAME_PRE "ReadoutMap"
#define READOUT_MAP_FILENAME_EXT ".xml"
#define DATA_FILENAME_PRE "Data"
#define DATA_FILENAME_EXT ".dat"
#define RUN_INDEX_FILENAME_PRE "Run"
#define RUN_INDEX_FILENAME_EXT ".ndx"
#define HISTOGRAM_FILENAME_PRE "Histograms"
#define HISTOGRAM_FILENAME_EXT ".root"
#define DAQ_MODE_FILENAME_PRE "DAQ"
#define PED_MODE_FILENAME_PRE "PED"
#define FILE_MODE_FILENAME_PRE "FILE"
#define RESERVED_CH_NUM 0
#define ENERGY_MAX 4*4095
#define FLOOD_MAX 2.0
#define NUM_CH_PER_RENA_CONST 36
#define CIRC_BUFFER_SIZE 10000 // Store 10000 events in the buffer

#define USB_1_0 1
#define USB_2_0 2

#define USB_VER USB_2_0

#define USB2_BUFFER_SIZE 1024

#define DEFAULT_USB1_PORT "/dev/ttyUSB0"
#define DEFAULT_TRANSMIT_FILE "Transmission.log"

#define COINCIDENCE_MODE true
#define DATE_STAMP true

#define PAULS_PANELID true
#define BINARY_OUTPUT true

#define DEFAULT_DIRNAME "000000000000/" // When timestamp is not used!
#define DEFAULT_MASTER_DIRPATH "/data2/DAQ_Data"

#define SPLIT_FILES true
#define FILE_SIZE_MAX 500*1024*1024 // Split files in chunks of 500M

#define MULT_USB_PER_PANEL true // This is effective only when COINCIDENCE_MODE is true

//#define MODULE_BASED_READOUT false

// New moduleId -> make it look like an IP. Panel.Cartridge.Board.Chip.Module.Apd

#include <string>
#include <vector>

using namespace std;

class Config
{
public:
    Config();
    // Enums
    enum RunMode {DAQ, PEDESTAL, FILE};
    enum MasterMode {DEFAULT, MODULE_TEST, TRANS_STAGE};

    // Static Variables
    static int NUM_PANEL_PER_DEVICE;
    static int NUM_CART_PER_PANEL;
    static int NUM_DAQ_PER_CART;
    static int NUM_RENA_PER_DAQ;
    static int NUM_MODULE_PER_RENA;
    static int NUM_CH_PER_RENA;
    static int numModules;
    static int numChipPerPanel;
    static bool configReady;
    static int usbDev;
    static string dateStamp;
    static string dateStampMT;
    static string readoutMapFilename;
    static string binaryDataFilename;
    static string binaryDataLeftFilename;
    static string binaryDataRightFilename;
    static string pedestalFilename;
    static string crystalLocFilename;
    static string crystalGainFilename;
    static string dataFilename;
    static string dataLeftFilename;
    static string dataRightFilename;
    static string runIndexFilename;
    static string directoryName;
    static string histogramFilename;
    static string moduleTestLogFilename;
    static string moduleNameMT;
    static string resistorMT;
    static string suffixMT;
    static string modePrefixTS0;
    static string modePrefixTS1;

    static vector<string> dataLeftFilenameVec;
    static vector<string> dataRightFilenameVec;
    static vector<string> binaryDataLeftFilenameVec;
    static vector<string> binaryDataRightFilenameVec;

    static bool multUsbPP;
    static int numUsbPP;
    static int numUsbPerPanelConstant;

    static int masterMode;
    static bool masterModeLocked;
    static int voltageMT;
    static int positionUM;
    static int posStepUM;

    static bool binaryOutput;
    static bool splitFiles;
    static unsigned long long int fileSizeMax;

    static void updateFilenames();
    static bool loadRunIndex(string filename);
    static bool setForceDirname(bool value, string dirname = "");
    static void updateFilenames4ModuleTest(bool acquireDateStamp = false);
    static void updateFilenames4TransStage();

    static void setRunMode(int runMode);
    static int getRunMode();

    static void createNewDirectory();

private:
    static int runMode;
    static bool directoryReady;
    static bool forceDirname;
    static string forcedDirectoryName;
    static string masterDirPath;
    static string directoryNameOrig;
    static string directoryTechName;
    static string getDateStamp();
    static string getDateStampMT();
    static void setupDirectory();
    static void clearDir(string dirname);
    static void writeRunIndex(string filename);
    static string extractFilename(string path);
    static string extractDirname(string path);
    static bool verifyFilenames();
};


// TODO: WRITE "loadRunIndex(string filename)" function!

/*#define NUM_PANEL_PER_DEVICE 1
#define NUM_CART_PER_PANEL 1
#define NUM_DAQ_PER_CART 1
#define NUM_RENA_PER_DAQ 1
#define NUM_MODULE_PER_RENA 4

#define NUM_CH_PER_RENA 36
*/





#endif // CONFIG_H




/* NOTES:
  create a static vector of char in channel to store fast/slow hit values (default N)
  N - none
  F - fast
  S - slow
  B - both

  Each channel will update its value on this vector. So if you have one module you will
  receive the data for that module only.

  Also, you need to sort the channel number for each RENA chip because the data is received
  sequentially and we need to know what module and what channel of that module the data
  belongs to.

  In the end when we receive a packet for a RENA chip. We will first look at its chip ID.
  Based on this chip ID, we will determine the range of channels that this chip controls.
  Starting from this channel id we will look at hit values e.g. N,N,F,F,S,S,... etc.
  For each channel with value we will assign the data to that modules channel.
  For example the M0H0.U or M3SA.energy.
*/


/* ADDITIONAL NOTES:
  Each RENAChip class has a vector named channel and it contains pointers to the
  channels. The vector is sorted based on the channelID.
  Scan in a default value for unused channels or create a hardwired vector for them.
  When programming RENA make sure all the unused channels are also programmed with
  the default values. Read out the first 2 Channels of each RENA to avoid timing
  issues. In fact readout all channels and make sure you read the right registers
  i.e. fast hit - slow hit - both or none. Store these in channel objects for each
  channel.
*/
