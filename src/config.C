#include "config.h"
#include <iostream>
#include <fstream>
#include <map>
//#include <QDateTime>
//#include <QString>
//#include <QStringList>
//#include <QDir>
#include <sstream>
#include <iomanip>
//#include "tinyxml2.h"
#include "util.h"
 
using namespace std;

// Initialize Static Variables
int Config::NUM_CART_PER_PANEL;
int Config::NUM_CH_PER_RENA;
int Config::NUM_DAQ_PER_CART;
int Config::NUM_MODULE_PER_RENA;
int Config::NUM_PANEL_PER_DEVICE;
int Config::NUM_RENA_PER_DAQ;
int Config::numModules;
int Config::numChipPerPanel;
bool Config::configReady = false;
int Config::usbDev = 0;
string Config::dateStamp = "";
string Config::dateStampMT = "";
string Config::readoutMapFilename = "";
string Config::binaryDataFilename = "";
string Config::binaryDataLeftFilename = "";
string Config::binaryDataRightFilename = "";
string Config::pedestalFilename = "";
string Config::crystalLocFilename = "";
string Config::crystalGainFilename = "";
string Config::dataFilename = "";
string Config::dataLeftFilename = "";
string Config::dataRightFilename = "";
string Config::runIndexFilename = "";
string Config::directoryName = "";
string Config::directoryNameOrig = "";
string Config::forcedDirectoryName = "";
string Config::masterDirPath = "";
string Config::histogramFilename = "";
string Config::moduleTestLogFilename = "";
string Config::moduleNameMT = "";
string Config::resistorMT = "";
string Config::suffixMT = "";
string Config::modePrefixTS0 = "";
string Config::modePrefixTS1 = "";
string Config::directoryTechName = "";

vector<string> Config::dataLeftFilenameVec;
vector<string> Config::dataRightFilenameVec;
vector<string> Config::binaryDataLeftFilenameVec;
vector<string> Config::binaryDataRightFilenameVec;

bool Config::multUsbPP = MULT_USB_PER_PANEL;
int Config::numUsbPerPanelConstant = 1; // This is overriden by number of DAQ boards per panel
int Config::numUsbPP = 1; //NUM_USB_PER_PANEL;


int Config::runMode = Config::DAQ;
int Config::masterMode = Config::DEFAULT;
bool Config::masterModeLocked = false;
int Config::voltageMT = 0;
int Config::positionUM = 0;
int Config::posStepUM = 0;
bool Config::directoryReady = false;
bool Config::forceDirname = false;

bool Config::binaryOutput = BINARY_OUTPUT;
bool Config::splitFiles = SPLIT_FILES;
unsigned long long int Config::fileSizeMax = FILE_SIZE_MAX;

Config::Config()
{
    // Default Config
    map<string,int> configMap;
    configMap["NUM_PANEL_PER_DEVICE"]       = 1;
    configMap["NUM_CART_PER_PANEL"]         = 1;
    configMap["NUM_DAQ_PER_CART"]           = 1;
    configMap["NUM_RENA_PER_DAQ"]           = 1;
    configMap["NUM_MODULE_PER_RENA"]        = 4;
    NUM_CH_PER_RENA = NUM_CH_PER_RENA_CONST;

    /*

    // Change this part with an xml file.
    // Include other parameters like masterDirPath
    // for now using the default value
    masterDirPath = DEFAULT_MASTER_DIRPATH;
    // verify masterDirPath
    if (!QDir(masterDirPath.c_str()).exists()) {
        cout << "ERROR: Master Directory = \"" << masterDirPath << "\" does not exist!" << endl;
        throw(-1);
    }

    */
    ifstream inputFile(CONFIG_FILENAME, ifstream::in);
    if (!inputFile.good()){
        cout << "Can't read file \"" << CONFIG_FILENAME << "\"" << endl;
        //create a template file for the user
        throw(-1);
    }

    string varName;
    string equal;
    int value;

    unsigned int size = configMap.size();
    for (unsigned int ii=0; ii<size; ii++) {
        inputFile >> varName;
        inputFile >> equal;
        if (equal.compare("=")!=0){
            cout << "In file \"" << CONFIG_FILENAME << "\":" << endl;
            cout << "\tInvalid declaration of \"" << varName << "\""<< endl;
            throw(-1);
        }
        inputFile >> value;
        configMap[varName] = value;
    }
    if (configMap.size()!=size) {
        cout << "In file \"" << CONFIG_FILENAME << "\":" << endl;
        cout << "\tUnexpected Format!" << endl;
        throw(-1);
    }

    NUM_PANEL_PER_DEVICE = configMap["NUM_PANEL_PER_DEVICE"];
    NUM_CART_PER_PANEL = configMap["NUM_CART_PER_PANEL"];
    NUM_DAQ_PER_CART = configMap["NUM_DAQ_PER_CART"];
    NUM_RENA_PER_DAQ = configMap["NUM_RENA_PER_DAQ"];
    NUM_MODULE_PER_RENA = configMap["NUM_MODULE_PER_RENA"];
    numModules =    NUM_PANEL_PER_DEVICE *
            NUM_CART_PER_PANEL *
            NUM_DAQ_PER_CART *
            NUM_RENA_PER_DAQ *
            NUM_MODULE_PER_RENA;

    numChipPerPanel =   NUM_CART_PER_PANEL *
            NUM_DAQ_PER_CART *
            NUM_RENA_PER_DAQ;


    /*
    if (USB_VER == USB_1_0 && numModules > 4) {
        cout << "In " << CONFIG_FILENAME << ": " << endl;
        cout << "\t You can not use USB1.0 with more than 1 RENA Chip!" << endl;
        throw(-1);
    }
    */

    Config::numUsbPerPanelConstant = NUM_DAQ_PER_CART*NUM_CART_PER_PANEL;
    Config::numUsbPP = Config::numUsbPerPanelConstant;

    configReady = true;

 
}


void Config::setupDirectory()
{
    // This should be called when user needs a new directory
    // In the current implementation it should be called only
    // on the first click of the START button.

  /*
    if (DATE_STAMP) {
        if (dateStamp.empty()) {
            dateStamp = getDateStamp();
        }

        directoryName = masterDirPath + "/" + dateStamp + "/";
    }
    else {
        dateStamp.clear();
        directoryName = masterDirPath + "/" + DEFAULT_DIRNAME + "/";
    }

    if (forceDirname) {
        directoryName = masterDirPath + "/" + forcedDirectoryName + "/";
    }

    if (QDir(QString(directoryName.c_str())).exists()) {
        cout << "Directory \"" << directoryName << "\" already exists!" << endl;
        cout << "Data will be overwritten!" << endl;
        clearDir(directoryName);
    }
    else {
        QDir().mkdir(QString(directoryName.c_str()));
    }
    directoryNameOrig = directoryName;

    directoryTechName = directoryName + "/Tech/";

    if (!QDir(QString(directoryTechName.c_str())).exists()) {
        if (!QDir().mkpath(QString(directoryTechName.c_str()))) {
            cout << "Failed to create path \"" << directoryTechName << "\"" << endl;
            throw(-1);
        }
    }
    else {
        cout << "Directory \"" << directoryName << "\" already exists!" << endl;
        cout << "Data will be overwritten!" << endl;
        clearDir(directoryTechName);
    }

    directoryReady = true;
  */
}

void Config::updateFilenames()
{
    // This should be called when user hits START button.

    if (masterMode == Config::MODULE_TEST ) {
        // Filenames for the module test mode are handled by a different function
        // Config::updateFilenames4ModuleTest()
        return;
    }

    stringstream ss;

    if (DATE_STAMP) {
        dateStamp = getDateStamp();
    }

    if (!directoryReady) {
        setupDirectory();
    }
    else {
        directoryName = directoryNameOrig; // Incase it is changed by FILE Mode.
    }


    if (masterMode == Config::TRANS_STAGE) {
        // In this mode the directory is created the normal way and the filenames are handled by
        // another function Config::updateFilenames4TransStage()
        return;
    }

    // Set up filenames
    string delim = "";
    if (DATE_STAMP) {
        delim = "_";
    }

    string modePrefix = "";
    if (runMode == Config::DAQ) {
        modePrefix = DAQ_MODE_FILENAME_PRE;
    }
    else if (runMode == Config::PEDESTAL){
        modePrefix = PED_MODE_FILENAME_PRE;
    }
    else if (runMode == Config::FILE) {
        modePrefix = FILE_MODE_FILENAME_PRE;
    }
    else {
        modePrefix = "";
    }

    runIndexFilename = directoryTechName + modePrefix + "_" + RUN_INDEX_FILENAME_PRE + delim + Config::dateStamp + RUN_INDEX_FILENAME_EXT;
    binaryDataFilename = directoryName + modePrefix + "_" + BINARY_DATA_FILENAME_PRE + delim + Config::dateStamp + BINARY_DATA_FILENAME_EXT;
    binaryDataLeftFilename = directoryName + modePrefix + "_" + BINARY_DATA_FILENAME_PRE + delim + Config::dateStamp + "_L" + BINARY_DATA_FILENAME_EXT;
    binaryDataRightFilename = directoryName + modePrefix + "_" + BINARY_DATA_FILENAME_PRE + delim + Config::dateStamp + "_R" + BINARY_DATA_FILENAME_EXT;
    readoutMapFilename = directoryTechName + modePrefix + "_" + READOUT_MAP_FILENAME_PRE + delim + Config::dateStamp + READOUT_MAP_FILENAME_EXT;
    pedestalFilename = directoryTechName + modePrefix + "_" + PEDESTAL_FILENAME_PRE + delim + Config::dateStamp + PEDESTAL_FILENAME_EXT;
    crystalLocFilename = directoryTechName + modePrefix + "_" + CRYSTAL_LOC_FILENAME_PRE + delim + Config::dateStamp + CRYSTAL_LOC_FILENAME_EXT;
    crystalGainFilename = directoryTechName + modePrefix + "_" + CRYSTAL_GAIN_FILENAME_PRE + delim + Config::dateStamp + CRYSTAL_GAIN_FILENAME_EXT;
    dataFilename = directoryTechName + modePrefix + "_" + DATA_FILENAME_PRE + delim + Config::dateStamp + DATA_FILENAME_EXT;
    dataLeftFilename = directoryTechName + modePrefix + "_" + DATA_FILENAME_PRE + delim + Config::dateStamp + "_L" + DATA_FILENAME_EXT;
    dataRightFilename = directoryTechName + modePrefix + "_" + DATA_FILENAME_PRE + delim + Config::dateStamp + "_R" + DATA_FILENAME_EXT;
    histogramFilename = directoryTechName + modePrefix + "_" + HISTOGRAM_FILENAME_PRE + delim + Config::dateStamp + HISTOGRAM_FILENAME_EXT;

    if (multUsbPP) {
        dataLeftFilenameVec.clear();
        dataRightFilenameVec.clear();
        binaryDataLeftFilenameVec.clear();
        binaryDataRightFilenameVec.clear();
        for (int ii = 0 ; ii<numUsbPP; ii++) {
            stringstream ssLeftFilename;
            stringstream ssRightFilename;
            stringstream ssBinaryLeftFilename;
            stringstream ssBinaryRightFilename;
            ssLeftFilename << directoryTechName << modePrefix << "_" << DATA_FILENAME_PRE << delim << Config::dateStamp << "_L" << ii << DATA_FILENAME_EXT;
            ssRightFilename << directoryTechName << modePrefix << "_" << DATA_FILENAME_PRE << delim << Config::dateStamp << "_R" << ii << DATA_FILENAME_EXT;
            ssBinaryLeftFilename << directoryName << modePrefix << "_" << BINARY_DATA_FILENAME_PRE << delim << Config::dateStamp << "_L" << ii << BINARY_DATA_FILENAME_EXT;
            ssBinaryRightFilename << directoryName << modePrefix << "_" << BINARY_DATA_FILENAME_PRE << delim << Config::dateStamp << "_R" << ii << BINARY_DATA_FILENAME_EXT;
            dataLeftFilenameVec.push_back(ssLeftFilename.str());
            dataRightFilenameVec.push_back(ssRightFilename.str());
            binaryDataLeftFilenameVec.push_back(ssBinaryLeftFilename.str());
            binaryDataRightFilenameVec.push_back(ssBinaryRightFilename.str());
        }
    }

    writeRunIndex(runIndexFilename);
}

string Config::getDateStamp()
{
  //   QDateTime dateTime = QDateTime::currentDateTime();
  //  QString stamp;

  //  stamp = dateTime.toString("yyMMddhhmm");
  //  return stamp.toStdString();
  return "7919060123";
}

string Config::getDateStampMT()
{/*
    QDateTime dateTime = QDateTime::currentDateTime();
    QString stamp;

    stamp = dateTime.toString("MM-dd-yy");
    return stamp.toStdString(); */
  return "06-19-79";
}

void Config::clearDir(string dirname)
{ /*
    if (!QDir(QString(directoryName.c_str())).exists()) {
        return;
    }
    QDir dir(QString(dirname.c_str()));
    QStringList fileList = dir.entryList();
    for (int ii=0; ii<fileList.size(); ii++) {
        //cout << ii << ": " << fileList.at(ii).toStdString() << endl;
        dir.remove(fileList.at(ii));
    }
  */
}


void Config::writeRunIndex(string filename)
{/*
    using namespace tinyxml2;
    stringstream ss;
    XMLDocument doc;
    XMLDeclaration *declaration = doc.NewDeclaration("xml version=\"1.0\" ");
    XMLElement *root = doc.NewElement("RunIndex");
    doc.InsertEndChild(declaration);
    doc.InsertEndChild(root);

    XMLElement *xmlCoinMode = doc.NewElement("CoincidenceMode");
    xmlCoinMode->SetAttribute("VALUE", COINCIDENCE_MODE);
    root->InsertEndChild(xmlCoinMode);

    XMLElement *xmlSplitFiles = doc.NewElement("SplitFiles");
    xmlSplitFiles->SetAttribute("VALUE", splitFiles);
    root->InsertEndChild(xmlSplitFiles);

    if (splitFiles) {
        XMLElement *xmlFileSizeMax = doc.NewElement("FileSizeMax");
        xmlFileSizeMax->SetAttribute("UINT", (unsigned int)fileSizeMax);
        root->InsertEndChild(xmlFileSizeMax);
    }

    if (!COINCIDENCE_MODE) {
        XMLElement *xmlDataFilename = doc.NewElement("DataFilename");
        xmlDataFilename->SetAttribute("VALUE", extractFilename(dataFilename).c_str());
        root->InsertEndChild(xmlDataFilename);

        XMLElement *xmlBinaryDataFilename = doc.NewElement("BinaryDataFilename");
        xmlBinaryDataFilename->SetAttribute("VALUE", extractFilename(binaryDataFilename).c_str());
        root->InsertEndChild(xmlBinaryDataFilename);

    }
    else {
        if (!multUsbPP) {
            XMLElement *xmlDataLeftFilename = doc.NewElement("DataLeftFilename");
            xmlDataLeftFilename->SetAttribute("VALUE", extractFilename(dataLeftFilename).c_str());
            root->InsertEndChild(xmlDataLeftFilename);

            XMLElement *xmlDataRightFilename = doc.NewElement("DataRightFilename");
            xmlDataRightFilename->SetAttribute("VALUE", extractFilename(dataRightFilename).c_str());
            root->InsertEndChild(xmlDataRightFilename);

            XMLElement *xmlBinaryDataLeftFilename = doc.NewElement("BinaryDataLeftFilename");
            xmlBinaryDataLeftFilename->SetAttribute("VALUE", extractFilename(binaryDataLeftFilename).c_str());
            root->InsertEndChild(xmlBinaryDataLeftFilename);

            XMLElement *xmlBinaryDataRightFilename = doc.NewElement("BinaryDataRightFilename");
            xmlBinaryDataRightFilename->SetAttribute("VALUE", extractFilename(binaryDataRightFilename).c_str());
            root->InsertEndChild(xmlBinaryDataRightFilename);
        }
        else {
            XMLElement *xmlDataLeftFilenameVec = doc.NewElement("DataLeftFilenameVec");
            root->InsertEndChild(xmlDataLeftFilenameVec);
            XMLElement *xmlDataRightFilenameVec = doc.NewElement("DataRightFilenameVec");
            root->InsertEndChild(xmlDataRightFilenameVec);
            XMLElement *xmlBinaryDataLeftFilenameVec = doc.NewElement("BinaryDataLeftFilenameVec");
            root->InsertEndChild(xmlBinaryDataLeftFilenameVec);
            XMLElement *xmlBinaryDataRightFilenameVec = doc.NewElement("BinaryDataRightFilenameVec");
            root->InsertEndChild(xmlBinaryDataRightFilenameVec);
            for (int ii=0; ii<dataLeftFilenameVec.size(); ii++) {
                stringstream ssElementNameLeft;
                stringstream ssElementNameRight;
                stringstream ssBinaryElementNameLeft;
                stringstream ssBinaryElementNameRight;
                ssElementNameLeft << "L" << ii;
                ssElementNameRight << "R" << ii;
                ssBinaryElementNameLeft << "L" << ii;
                ssBinaryElementNameRight << "R" << ii;
                XMLElement *xmlDataLeftFilenameCurrent = doc.NewElement(ssElementNameLeft.str().c_str());
                XMLElement *xmlDataRightFilenameCurrent = doc.NewElement(ssElementNameRight.str().c_str());
                XMLElement *xmlBinaryDataLeftFilenameCurrent = doc.NewElement(ssBinaryElementNameLeft.str().c_str());
                XMLElement *xmlBinaryDataRightFilenameCurrent = doc.NewElement(ssBinaryElementNameRight.str().c_str());
                xmlDataLeftFilenameCurrent->SetAttribute("VALUE", extractFilename(dataLeftFilenameVec[ii]).c_str());
                xmlDataRightFilenameCurrent->SetAttribute("VALUE", extractFilename(dataRightFilenameVec[ii]).c_str());
                xmlBinaryDataLeftFilenameCurrent->SetAttribute("VALUE", extractFilename(binaryDataLeftFilenameVec[ii]).c_str());
                xmlBinaryDataRightFilenameCurrent->SetAttribute("VALUE", extractFilename(binaryDataRightFilenameVec[ii]).c_str());
                xmlDataLeftFilenameVec->InsertEndChild(xmlDataLeftFilenameCurrent);
                xmlDataRightFilenameVec->InsertEndChild(xmlDataRightFilenameCurrent);
                xmlBinaryDataLeftFilenameVec->InsertEndChild(xmlBinaryDataLeftFilenameCurrent);
                xmlBinaryDataRightFilenameVec->InsertEndChild(xmlBinaryDataRightFilenameCurrent);
            }
        }
    }

    XMLElement *xmlReadoutMapFilename = doc.NewElement("ReadoutMapFilename");
    xmlReadoutMapFilename->SetAttribute("VALUE", extractFilename(readoutMapFilename).c_str());
    root->InsertEndChild(xmlReadoutMapFilename);

    XMLElement *xmlPedestalFilename = doc.NewElement("PedestalFilename");
    xmlPedestalFilename->SetAttribute("VALUE", extractFilename(pedestalFilename).c_str());
    root->InsertEndChild(xmlPedestalFilename);

    XMLElement *xmlCrystalLocFilename = doc.NewElement("CrystalLocFilename");
    xmlCrystalLocFilename->SetAttribute("VALUE", extractFilename(crystalLocFilename).c_str());
    root->InsertEndChild(xmlCrystalLocFilename);

    XMLElement *xmlCrystalGainFilename = doc.NewElement("CrystalGainFilename");
    xmlCrystalGainFilename->SetAttribute("VALUE", extractFilename(crystalGainFilename).c_str());
    root->InsertEndChild(xmlCrystalGainFilename);

    // ALSO SAVE THE DEVICE USED FOR PROGRAMMING

    doc.SaveFile(filename.c_str());
 */
}


bool Config::loadRunIndex(string filename)
{
  /*
    using namespace tinyxml2;
    XMLDocument doc;
    if (doc.LoadFile(filename.c_str()) != 0) {
        cout << "Failed to load file \"" << filename << "\". Check to see if it exists!" << endl;
        return false;
    }
    XMLElement *root = doc.FirstChildElement("RunIndex");
    if (!root) {
        cout << "Invalid file structure in file \"" << filename << "\"" << endl;
        return false;
    }

    XMLElement *xmlSplitFiles = root->FirstChildElement("SplitFiles");
    if (!xmlSplitFiles) {
        cout << "In file \"" << filename << "\"" << endl;
        cout << "\tTag not found: \"SplitFiles\"" << endl;
        cout << "\tAssuming files are not split!" << endl;
        Config::splitFiles = false;
    }
    else {
        Config::splitFiles = xmlSplitFiles->BoolAttribute("VALUE");
    }

    if (Config::splitFiles) {
        XMLElement *xmlFileSizeMax = root->FirstChildElement("FileSizeMax");
        if (!xmlFileSizeMax) {
            cout << "In file \"" << filename << "\"" << endl;
            cout << "\tTag not found: \"FileSizeMax\"" << endl;
            return false;
        }
        Config::fileSizeMax = (unsigned long long int)xmlFileSizeMax->UnsignedAttribute("UINT");
    }

    QDir dir; // By default this contains PWD
    string relativePath = dir.relativeFilePath(QString(filename.c_str())).toStdString();
    directoryName = extractDirname(relativePath);



    // Look for DataFilename (if exists COINCIDENCE_MODE should be false otherwise true)
    XMLElement *xmlDataFilename = root->FirstChildElement("DataFilename");
    if (xmlDataFilename) {
        // COINCIDENCE_MODE = false
        if (COINCIDENCE_MODE != false) {
            cout << "Coincidence Mode does not match! Change coinciende mode in \"config.h\"." << endl;
            return false;
        }
        dataFilename = directoryName + xmlDataFilename->Attribute("VALUE");
        XMLElement *xmlBinaryDataFilename = root->FirstChildElement("BinaryDataFilename");
        binaryDataFilename = directoryName + xmlBinaryDataFilename->Attribute("VALUE");
    }
    else {
        // COINCIDENCE_MODE = true
        if (COINCIDENCE_MODE != true) {
            cout << "Coincidence Mode does not match! Change coinciende mode in \"config.h\"." << endl;
            return false;
        }
    }


    // Look for DataLeftFilename (if exists multUsbPP should be false, otherwise true)
    XMLElement *xmlDataLeftFilename = root->FirstChildElement("DataLeftFilename");
    if (xmlDataLeftFilename) {
        // COINCIDENCE_MODE = true;
        multUsbPP = false;
        dataLeftFilename = directoryName + xmlDataLeftFilename->Attribute("VALUE");
        XMLElement *xmlDataRightFilename = root->FirstChildElement("DataRightFilename");
        dataRightFilename = directoryName + xmlDataRightFilename->Attribute("VALUE");

        XMLElement *xmlBinaryDataLeftFilename = root->FirstChildElement("BinaryDataLeftFilename");
        binaryDataLeftFilename = directoryName + xmlBinaryDataLeftFilename->Attribute("VALUE");
        XMLElement *xmlBinaryDataRightFilename = root->FirstChildElement("BinaryDataRightFilename");
        binaryDataRightFilename = directoryName + xmlBinaryDataRightFilename->Attribute("VALUE");
    }



    XMLElement *xmlDataLeftFilenameVec = root->FirstChildElement("DataLeftFilenameVec");
    if (xmlDataLeftFilenameVec) {
        multUsbPP = true;
        XMLElement *xmlDataRightFilenameVec = root->FirstChildElement("DataRightFilenameVec");
        XMLElement *xmlBinaryDataLeftFilenameVec = root->FirstChildElement("BinaryDataLeftFilenameVec");
        XMLElement *xmlBinaryDataRightFilenameVec = root->FirstChildElement("BinaryDataRightFilenameVec");

        XMLElement *xmlCurrentFilenameElement = xmlDataLeftFilenameVec->FirstChildElement();
        int numFilenames = 0;
        while (xmlCurrentFilenameElement) {
            numFilenames++;
            xmlCurrentFilenameElement = xmlCurrentFilenameElement->NextSiblingElement();
        }

        numUsbPP = numFilenames;

        dataLeftFilenameVec.clear();
        dataRightFilenameVec.clear();
        binaryDataLeftFilenameVec.clear();
        binaryDataRightFilenameVec.clear();
        for (int ii=0; ii<numFilenames; ii++) {
            stringstream ssLeftId;
            stringstream ssRightId;
            ssLeftId << "L" << ii;
            ssRightId << "R" << ii;
            XMLElement *xmlCurrentFilenameLeft = xmlDataLeftFilenameVec->FirstChildElement(ssLeftId.str().c_str());
            XMLElement *xmlCurrentFilenameRight = xmlDataRightFilenameVec->FirstChildElement(ssRightId.str().c_str());
            XMLElement *xmlCurrentBinaryFilenameLeft = xmlBinaryDataLeftFilenameVec->FirstChildElement(ssLeftId.str().c_str());
            XMLElement *xmlCurrentBinaryFilenameRight = xmlBinaryDataRightFilenameVec->FirstChildElement(ssRightId.str().c_str());
            if (!xmlCurrentFilenameLeft || !xmlCurrentFilenameRight) {
                cout << "Missing filename in \"" << filename << "\"! Check \"DataLeftFilenameVec\" and \"DataRightFilenameVec\" tags." << endl;
                return false;
            }
            if (!xmlCurrentBinaryFilenameLeft || !xmlCurrentBinaryFilenameRight) {
                cout << "Missing filename in \"" << filename << "\"! Check \"BinaryDataLeftFilenameVec\" and \"BinaryDataRightFilenameVec\" tags." << endl;
                return false;
            }
            dataLeftFilenameVec.push_back(directoryName + xmlCurrentFilenameLeft->Attribute("VALUE"));
            dataRightFilenameVec.push_back(directoryName + xmlCurrentFilenameRight->Attribute("VALUE"));
            binaryDataLeftFilenameVec.push_back(directoryName + xmlCurrentBinaryFilenameLeft->Attribute("VALUE"));
            binaryDataRightFilenameVec.push_back(directoryName + xmlCurrentBinaryFilenameRight->Attribute("VALUE"));
        }

    }


    XMLElement *xmlReadoutMapFilename = root->FirstChildElement("ReadoutMapFilename");
    readoutMapFilename = directoryName + xmlReadoutMapFilename->Attribute("VALUE");

    XMLElement *xmlPedestalFilename = root->FirstChildElement("PedestalFilename");
    pedestalFilename = directoryName + xmlPedestalFilename->Attribute("VALUE");

    XMLElement *xmlCrystalLocFilename = root->FirstChildElement("CrystalLocFilename");
    crystalLocFilename = directoryName + xmlCrystalLocFilename->Attribute("VALUE");

    XMLElement *xmlCrystalGainFilename = root->FirstChildElement("CrystalGainFilename");
    crystalGainFilename = directoryName + xmlCrystalGainFilename->Attribute("VALUE");

    // ALSO LOAD THE DEVICE USED FOR PROGRAMMING
    return verifyFilenames();
  */
}



string Config::extractFilename(string path)
{
    return path.substr(path.find_last_of('/')+1);
}



string Config::extractDirname(string path)
{
    return path.substr(0, path.find_last_of('/')).append("/");
}


bool Config::verifyFilenames()
{
    ifstream inputFile;
    string filenameTemp;
    if (!COINCIDENCE_MODE) {
        if (Config::splitFiles) {
            filenameTemp = Util::buildSplitFilename(dataFilename, 0);
        }
        else {
            filenameTemp = dataFilename;
        }
        inputFile.open(filenameTemp.c_str(), ios::in);
        if (!inputFile.is_open()) {
            cout << "File \"" << filenameTemp << "\" is missing!" << endl;
            return false;
        }
        inputFile.close();
    }
    else {
        if (!multUsbPP) {
            if (Config::splitFiles) {
                filenameTemp = Util::buildSplitFilename(dataLeftFilename, 0);
            }
            else {
                filenameTemp = dataLeftFilename;
            }
            inputFile.open(filenameTemp.c_str(), ios::in);
            if (!inputFile.is_open()) {
                cout << "File \"" << filenameTemp << "\" is missing!" << endl;
                return false;
            }
            inputFile.close();

            if (Config::splitFiles) {
                filenameTemp = Util::buildSplitFilename(dataRightFilename, 0);
            }
            else {
                filenameTemp = dataRightFilename;
            }
            inputFile.open(filenameTemp.c_str(), ios::in);
            if (!inputFile.is_open()) {
                cout << "File \"" << filenameTemp << "\" is missing!" << endl;
                return false;
            }
            inputFile.close();
        }
        else {
            for (int ii = 0; ii < numUsbPP; ii++) {
                if (Config::splitFiles) {
                    filenameTemp = Util::buildSplitFilename(dataLeftFilenameVec[ii], 0);
                }
                else {
                    filenameTemp = dataLeftFilenameVec[ii];
                }
                inputFile.open(filenameTemp.c_str(), ios::in);
                if (!inputFile.is_open()) {
                    cout << "File \"" << filenameTemp << "\" is missing!" << endl;
                    return false;
                }
                inputFile.close();

                if (Config::splitFiles) {
                    filenameTemp = Util::buildSplitFilename(dataRightFilenameVec[ii], 0);
                }
                else {
                    filenameTemp = dataRightFilenameVec[ii];
                }
                inputFile.open(filenameTemp.c_str(), ios::in);
                if (!inputFile.is_open()) {
                    cout << "File \"" << filenameTemp << "\" is missing!" << endl;
                    return false;
                }
                inputFile.close();
            }
        }
    }

    inputFile.open(readoutMapFilename.c_str(), ios::in);
    if (!inputFile.is_open()) {
        cout << "File \"" << readoutMapFilename << "\" is missing!" << endl;
        return false;
    }
    inputFile.close();

    return true;
}

bool Config::setForceDirname(bool value, string dirname)
{
    if (directoryReady) {
        return false; // Directory is already set up
    }

    forceDirname = value;

    if (dirname.empty()) {
        forcedDirectoryName = DEFAULT_DIRNAME;
    }
    else {
        forcedDirectoryName = dirname;
    }

    return true;
}

void Config::updateFilenames4ModuleTest(bool acquireDateStamp)
{ /*
    //Module name is stored in "string Config::moduleNameMT"

    if (acquireDateStamp) {
        dateStampMT = getDateStampMT();
    }

    // DEBUG
    //moduleNameMT = "moduleNameMT";
    //resistorMT = "10k";
    //suffixMT = "_0";
    // DEBUG_END

    stringstream ssVoltage;
    ssVoltage << setfill('0');
    ssVoltage << setw(4) << voltageMT << "V";


    string directoryNameClean = masterDirPath + "/ModuleTest/";
    string directoryNameTech = directoryNameClean + "/Tech/";

    if (!QDir(QString(directoryNameClean.c_str())).exists()) {
        if (!QDir().mkpath(QString(directoryNameClean.c_str()))) {
            cout << "Failed to create path \"" << directoryNameClean << "\"" << endl;
            throw(-1);
        }
    }

    if (!QDir(QString(directoryNameTech.c_str())).exists()) {
        if (!QDir().mkpath(QString(directoryNameTech.c_str()))) {
            cout << "Failed to create path \"" << directoryNameTech << "\"" << endl;
            throw(-1);
        }
    }

    binaryDataFilename = directoryNameClean + moduleNameMT + "_" + dateStampMT + "_" + resistorMT + "_" + ssVoltage.str() + suffixMT + ".dat";
    moduleTestLogFilename = directoryNameClean + moduleNameMT + "_" + dateStampMT + "_" + resistorMT + "_" + ssVoltage.str() + suffixMT + ".txt";
    runIndexFilename = directoryNameTech + moduleNameMT + "_" + dateStampMT + "_" + resistorMT + "_" + ssVoltage.str() + suffixMT + "_" + RUN_INDEX_FILENAME_PRE + RUN_INDEX_FILENAME_EXT;
    //binaryDataLeftFilename = directoryNameTech + moduleNameMT + "_" + dateStampMT + "_" + resistorMT + "_" + ssVoltage.str() + suffixMT + "_" + BINARY_DATA_FILENAME_PRE + "_L" + BINARY_DATA_FILENAME_EXT;
    //binaryDataRightFilename = directoryNameTech + moduleNameMT + "_" + dateStampMT + "_" + resistorMT + "_" + ssVoltage.str() + suffixMT + "_" + BINARY_DATA_FILENAME_PRE + "_R" + BINARY_DATA_FILENAME_EXT;
    readoutMapFilename = directoryNameTech + moduleNameMT + "_" + dateStampMT + "_" + resistorMT + "_" + ssVoltage.str() + suffixMT + "_" + READOUT_MAP_FILENAME_PRE + READOUT_MAP_FILENAME_EXT;
    pedestalFilename = directoryNameTech + moduleNameMT + "_" + dateStampMT + "_" + resistorMT + "_" + ssVoltage.str() + suffixMT + "_" + PEDESTAL_FILENAME_PRE + PEDESTAL_FILENAME_EXT;
    crystalLocFilename = directoryNameTech + moduleNameMT + "_" + dateStampMT + "_" + resistorMT + "_" + ssVoltage.str() + suffixMT + "_" + CRYSTAL_LOC_FILENAME_PRE + CRYSTAL_LOC_FILENAME_EXT;
    crystalGainFilename = directoryNameTech + moduleNameMT + "_" + dateStampMT + "_" + resistorMT + "_" + ssVoltage.str() + suffixMT + "_" + CRYSTAL_GAIN_FILENAME_PRE + CRYSTAL_GAIN_FILENAME_EXT;
    dataFilename = directoryNameTech + moduleNameMT + "_" + dateStampMT + "_" + resistorMT + "_" + ssVoltage.str() + suffixMT + "_" + DATA_FILENAME_PRE + DATA_FILENAME_EXT;
    //dataLeftFilename = directoryNameTech + moduleNameMT + "_" + dateStampMT + "_" + resistorMT + "_" + ssVoltage.str() + suffixMT + "_" + DATA_FILENAME_PRE + "_L" + DATA_FILENAME_EXT;
    //dataRightFilename = directoryNameTech + moduleNameMT + "_" + dateStampMT + "_" + resistorMT + "_" + ssVoltage.str() + suffixMT + "_" + DATA_FILENAME_PRE + "_R" + DATA_FILENAME_EXT;
    histogramFilename = directoryNameTech + moduleNameMT + "_" + dateStampMT + "_" + resistorMT + "_" + ssVoltage.str() + suffixMT + "_" + HISTOGRAM_FILENAME_PRE + HISTOGRAM_FILENAME_EXT;

    writeRunIndex(runIndexFilename);
  */
}

void Config::updateFilenames4TransStage()
{/*
    if (Config::masterMode != Config::TRANS_STAGE) {
        cout << "WARNING! Config::updateDilenames4TransStage() is called but Config::masterMode != Config::TRANS_STAGE" << endl;
    }
    updateFilenames(); // To prepare the directory.


    // Set up filenames
    string delim = "";
    if (DATE_STAMP) {
        delim = "_";
    }

    stringstream ssPositionUM;
    ssPositionUM << setfill('0');
    ssPositionUM << setw(6) << positionUM << "u";

    stringstream ssPosStepUM;
    ssPosStepUM << setfill('0');
    ssPosStepUM << setw(6) << posStepUM << "u";

    runIndexFilename = directoryTechName + modePrefixTS0 + "_" + modePrefixTS1 + "_" + RUN_INDEX_FILENAME_PRE + delim + Config::dateStamp + "_" + ssPositionUM.str() + "_" + ssPosStepUM.str() + RUN_INDEX_FILENAME_EXT;
    binaryDataFilename = directoryName + modePrefixTS0 + "_" + modePrefixTS1 + "_" + BINARY_DATA_FILENAME_PRE + delim + Config::dateStamp + "_" + ssPositionUM.str() + "_" + ssPosStepUM.str() + BINARY_DATA_FILENAME_EXT;
    binaryDataLeftFilename = directoryName + modePrefixTS0 + "_" + modePrefixTS1 + "_" + BINARY_DATA_FILENAME_PRE + delim + Config::dateStamp + "_" + ssPositionUM.str() + "_" + ssPosStepUM.str() + "_L" + BINARY_DATA_FILENAME_EXT;
    binaryDataRightFilename = directoryName + modePrefixTS0 + "_" + modePrefixTS1 + "_" + BINARY_DATA_FILENAME_PRE + delim + Config::dateStamp + "_" + ssPositionUM.str() + "_" + ssPosStepUM.str() + "_R" + BINARY_DATA_FILENAME_EXT;
    readoutMapFilename = directoryTechName + modePrefixTS0 + "_" + modePrefixTS1 + "_" + READOUT_MAP_FILENAME_PRE + delim + Config::dateStamp + "_" + ssPositionUM.str() + "_" + ssPosStepUM.str() + READOUT_MAP_FILENAME_EXT;
    pedestalFilename = directoryTechName + modePrefixTS0 + "_" + modePrefixTS1 + "_" + PEDESTAL_FILENAME_PRE + delim + Config::dateStamp + "_" + ssPositionUM.str() + "_" + ssPosStepUM.str() + PEDESTAL_FILENAME_EXT;
    crystalLocFilename = directoryTechName + modePrefixTS0 + "_" + modePrefixTS1 + "_" + CRYSTAL_LOC_FILENAME_PRE + delim + Config::dateStamp + "_" + ssPositionUM.str() + "_" + ssPosStepUM.str() + CRYSTAL_LOC_FILENAME_EXT;
    crystalGainFilename = directoryTechName + modePrefixTS0 + "_" + modePrefixTS1 + "_" + CRYSTAL_GAIN_FILENAME_PRE + delim + Config::dateStamp + "_" + ssPositionUM.str() + "_" + ssPosStepUM.str() + CRYSTAL_GAIN_FILENAME_EXT;
    dataFilename = directoryTechName + modePrefixTS0 + "_" + modePrefixTS1 + "_" + DATA_FILENAME_PRE + delim + Config::dateStamp + "_" + ssPositionUM.str() + "_" + ssPosStepUM.str() + DATA_FILENAME_EXT;
    dataLeftFilename = directoryTechName + modePrefixTS0 + "_" + modePrefixTS1 + "_" + DATA_FILENAME_PRE + delim + Config::dateStamp + "_" + ssPositionUM.str() + "_" + ssPosStepUM.str() + "_L" + DATA_FILENAME_EXT;
    dataRightFilename = directoryTechName + modePrefixTS0 + "_" + modePrefixTS1 + "_" + DATA_FILENAME_PRE + delim + Config::dateStamp + "_" + ssPositionUM.str() + "_" + ssPosStepUM.str() + "_R" + DATA_FILENAME_EXT;
    histogramFilename = directoryTechName + modePrefixTS0 + "_" + modePrefixTS1 + "_" + HISTOGRAM_FILENAME_PRE + delim + Config::dateStamp + "_" + ssPositionUM.str() + "_" + ssPosStepUM.str() + HISTOGRAM_FILENAME_EXT;

    if (multUsbPP) {
        dataLeftFilenameVec.clear();
        dataRightFilenameVec.clear();
        binaryDataLeftFilenameVec.clear();
        binaryDataRightFilenameVec.clear();
        for (int ii = 0 ; ii<numUsbPP; ii++) {
            stringstream ssLeftFilename;
            stringstream ssRightFilename;
            stringstream ssBinaryLeftFilename;
            stringstream ssBinaryRightFilename;
            ssLeftFilename << directoryTechName << modePrefixTS0 << "_" << modePrefixTS1 << "_" << DATA_FILENAME_PRE << delim << Config::dateStamp << "_" << ssPositionUM.str() << "_" << ssPosStepUM.str() << "_L" << ii << DATA_FILENAME_EXT;
            ssRightFilename << directoryTechName << modePrefixTS0 << "_" << modePrefixTS1 << "_" << DATA_FILENAME_PRE << delim << Config::dateStamp << "_" << ssPositionUM.str() << "_" << ssPosStepUM.str() << "_R" << ii << DATA_FILENAME_EXT;
            ssBinaryLeftFilename << directoryName << modePrefixTS0 << "_" << modePrefixTS1 << "_" << BINARY_DATA_FILENAME_PRE << delim << Config::dateStamp << "_" << ssPositionUM.str() << "_" << ssPosStepUM.str() << "_L" << ii << BINARY_DATA_FILENAME_EXT;
            ssBinaryRightFilename << directoryName << modePrefixTS0 << "_" << modePrefixTS1 << "_" << BINARY_DATA_FILENAME_PRE << delim << Config::dateStamp << "_" << ssPositionUM.str() << "_" << ssPosStepUM.str() << "_R" << ii << BINARY_DATA_FILENAME_EXT;
            dataLeftFilenameVec.push_back(ssLeftFilename.str());
            dataRightFilenameVec.push_back(ssRightFilename.str());
            binaryDataLeftFilenameVec.push_back(ssBinaryLeftFilename.str());
            binaryDataRightFilenameVec.push_back(ssBinaryRightFilename.str());
        }
    }

    writeRunIndex(runIndexFilename);
 */
}



void Config::setRunMode(int runMode)
{
    Config::runMode = runMode;
    if (runMode != Config::FILE) {
        // Set default values for global variables
        binaryOutput = BINARY_OUTPUT;
        splitFiles = SPLIT_FILES;
        fileSizeMax = FILE_SIZE_MAX;
        multUsbPP = MULT_USB_PER_PANEL;
        numUsbPP = numUsbPerPanelConstant;
    }
}

int Config::getRunMode()
{
    return runMode;
}

void Config::createNewDirectory()
{
    directoryReady = false;
}
