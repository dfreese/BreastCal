#ifndef DAQBOARDMAP_H
#define DAQBOARDMAP_H

#include <string>

class DaqBoardID {
public:
    DaqBoardID() :
        panel(-1),
        cartridge(-1),
        id_valid(false)
    {}

    int panel;
    int cartridge;
    bool id_valid;
};

class DaqBoardMap {
public:
    DaqBoardMap();
    int loadMap(const std::string & filename);
    DaqBoardID ids[32];
    bool isValid(int address);
    int getPanelCartridgeNumber(int address, int & panel, int & cartridge);
};

#endif // DAQBOARDMAP_H
