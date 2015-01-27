#include "daqboardmap.h"
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

DaqBoardMap::DaqBoardMap()
{
}

int DaqBoardMap::loadMap(const std::string &filename)
{
    // read in cartridge map
    ifstream carmap;
    carmap.open(filename.c_str());
    if (!carmap.good()) {
        return(-1);
    }

    string fileline;
    while(getline(carmap, fileline)) {
        if (fileline.empty() > 0) {
            continue;
        } else if (fileline[0] == '#') {
            // If a line starts with a pound sign, it is ignored
            continue;
        }

        std::stringstream linestream(fileline);
        string c_id;
        linestream >> c_id;
        string dummy;
        linestream >> dummy;
        int id_val;
        if (!(linestream >> id_val)) {
            cerr << " Error parsing " << fileline << " from file " << filename << endl;
            return(-2);
        }
        int panel_id;
        int cartridge_id;
        sscanf(c_id.c_str(),"P%dC%d",&panel_id,&cartridge_id);
        cout << " PANEL :: " << panel_id  << " CARTRIDGE :: " << cartridge_id
             << " ID : " << id_val << endl;
        if (id_val < 0 || id_val >= 32) {
            cerr << "Error: Invalid Cartridge map id_val in \""
                 << filename << "\"" << endl;
            return(-3);
        } else {
            ids[id_val].panel = panel_id;
            ids[id_val].cartridge = cartridge_id;
            ids[id_val].id_valid = true;
        }
    }
    carmap.close();
    return(0);
}


bool DaqBoardMap::isValid(int address)
{
    if ((address < 0) || (address >= 32)) {
        return(false);
    } else {
        return(ids[address].id_valid);
    }
}

int DaqBoardMap::getPanelCartridgeNumber(
        int address,
        int & panel,
        int & cartridge)
{
    if ((address < 0) || (address >= 32)) {
        return(-1);
    } else if (!ids[address].id_valid) {
        return(-2);
    } else {
        panel = ids[address].panel;
        cartridge = ids[address].cartridge;
        return(0);
    }
}
