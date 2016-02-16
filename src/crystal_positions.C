#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <Syspardef.h>
#include <cmath> 
#include <stdlib.h>

using namespace std;

#define INCHTOMM 25.4
#define XCRYSTALPITCH 1
#define YCRYSTALPITCH 1
#define XMODULEPITCH 0.405*INCHTOMM
#define YOFFSET 1.51 // mm
#define YDISTANCEBETWEENAPDS (0.32+0.079)*INCHTOMM  // 10.1346 mm
#define ZPITCH 0.0565*INCHTOMM //

int crystal_position(
        int panel,
        int cartridge,
        int fin,
        int module,
        int apd,
        int crystal,
        float TOTALPANELDISTANCE,
        float & x,
        float & y,
        float & z)
{
    if (panel == 0) {
        x = (XMODULEPITCH - 8 * XCRYSTALPITCH) / 2 + (module - 8) * XMODULEPITCH;  
        x += (std::floor(crystal / 8)  + 0.5) * XCRYSTALPITCH;

        y = -TOTALPANELDISTANCE / 2;
        y -= apd * YDISTANCEBETWEENAPDS;
        y -= ((7 - std::floor(crystal % 8) * YCRYSTALPITCH) + 0.5);

        z = (((cartridge - CARTRIDGES_PER_PANEL / 2.0) * FINS_PER_CARTRIDGE) + fin + 0.5) * ZPITCH;
    } else {
        x = (XMODULEPITCH - 8 * XCRYSTALPITCH) / 2 + (module - 8) * XMODULEPITCH;
        x += (7 - std::floor(crystal / 8) + 0.5) * XCRYSTALPITCH;
        y = TOTALPANELDISTANCE / 2;
        y += apd * YDISTANCEBETWEENAPDS;
        y += ((7 - std::floor(crystal % 8) * YCRYSTALPITCH) + 0.5);                              
        z = (((cartridge - CARTRIDGES_PER_PANEL / 2.0) * FINS_PER_CARTRIDGE) + fin + 0.5) * ZPITCH;
    }
    return(0);
}

void usage(void){
    cout << " crystal_positions [-v] -f [filename] -p [panelseparation]" << endl;
    return;
}

int main(int argc, char ** argv)
{
    if (argc == 1) {
        usage();
        return(0);
    }
    cout << "Welcome " << endl;

    string filename;
    int verbose(0);
    float PANELDISTANCE(-1);

    // Options not requiring input
    for(int ix = 1; ix < argc; ix++) {
		if(strncmp(argv[ix], "-v", 2) == 0) {
			cout << "Verbose Mode " << endl;
			verbose = 1;
		}
		if(strcmp(argv[ix], "-h") == 0) {
		  usage();
		  return(0);
		}
    }

    // Options requiring input
    for(int ix = 1; ix < (argc - 1); ix++) {
        if (strcmp(argv[ix],"-p") ==0 ) {
            PANELDISTANCE=atof(argv[ix + 1]);
            cout << " Using panel distance " << PANELDISTANCE << " mm." << endl;
		}
        if(strcmp(argv[ix], "-f") == 0) {
            filename = string(argv[ix + 1]);
        }
    }

    if (PANELDISTANCE<0) {
        cerr << "Panel Distance not specified or invalid" << endl;
        cerr << "Exiting" << endl;
        return(-2);
    }

    ofstream outputfile(filename.c_str());

    float crystal_cal[SYSTEM_PANELS]
            [CARTRIDGES_PER_PANEL]
            [FINS_PER_CARTRIDGE]
            [MODULES_PER_FIN]
            [APDS_PER_MODULE]
            [CRYSTALS_PER_APD] = {{{{{{0}}}}}};
    float TOTALPANELDISTANCE = PANELDISTANCE + 2 * YOFFSET;
    for (int p = 0; p < SYSTEM_PANELS; p++) {
    for (int c = 0; c < CARTRIDGES_PER_PANEL; c++) {
    for (int f = 0; f < FINS_PER_CARTRIDGE; f++) {
    for (int m = 0; m < MODULES_PER_FIN; m++) {
    for (int a = 0; a < APDS_PER_MODULE; a++) {
    for (int x = 0; x < CRYSTALS_PER_APD; x++) {
        float x_pos, y_pos, z_pos;
        crystal_position(p, c, f, m, a, x, TOTALPANELDISTANCE, x_pos, y_pos, z_pos);
        outputfile << x_pos << " " << y_pos << " " << z_pos << endl;
    }
    }
    }
    }
    }
    }

    return(0);
}

