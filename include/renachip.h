#ifndef RENACHIP_H
#define RENACHIP_H

#include "config.h"
#include "module.h"
#include "moduleaddress.h"
#include "readoutmap.h"

#include <string>

using namespace std;

class RENAChip
{
public:
    RENAChip(const string HKey);
    ~RENAChip();
    void set(const ModuleAddress &address, const string &channelCode, const string &parameter, int &value);
    void fillBuffConfig(const ModuleAddress &address, vector<char> &sendBuff);
    void fillBuffHitReg(vector<char> &sendBuff);
    void fillBuffOption(vector<char> &sendBuff, bool enableReadout = true, bool coinOverride = true, bool forceTrig = false, bool readTrigNotTime = false);
    void fillReadoutMap(ReadoutMap* readoutMap);
    //int sumVecUpto(vector<int> &vec, int index); // make private
    void fillModuleRMap(vector<int> &moduleRMap, const vector<int> &parseSizeVec, const int &offset); //make private
    int chipId;
    static int numChip;
    static Channel *unused;


//private:   //XXX: REMOVE AFTER DEBUGGING
    vector<Module*> module;
    //vector<Channel*> unused;
    vector<Channel*> channel;
private:
    int fillModuleMapExtended(map<int, map<int, vector<int> > > &moduleMapExtended, int trigCode);
};

#endif // RENACHIP_H
