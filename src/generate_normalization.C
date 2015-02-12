#include "Syspardef.h"
#include "CoincEvent.h"
#include <string>
#include <cal_helper_functions.h>
#include <iostream>
#include <cmath>
#include <fstream>
#include <stdlib.h>
#include <TFile.h>
#include <PixelCal.h>

#define INCHTOMM 25.4
#define XCRYSTALPITCH 1
#define YCRYSTALPITCH 1
#define XMODULEPITCH 0.405*INCHTOMM
#define YOFFSET 1.51 // mm
#define YDISTANCEBETWEENAPDS (0.32+0.079)*INCHTOMM  // 10.1346 mm
#define ZPITCH 0.0565*INCHTOMM //

using namespace std;

void usage(void) {
    cout << "generate_normalization [-v] -f [filename] -p [panelseparation]" << endl;
    cout << " Options:\n"
         << " -calleft [filename]: specify a calibration file for the left side\n"
         << " -calright [filename]: specify a calibration file for the right side\n"
         << " -r [0.0 - 1.0]: randomly subsample the output by a given ratio\n"
         << " -s [value]: set the seed of the random number generator\n";
    return;
}

struct lor_buffer {
    float x1;
    float y1;
    float z1;
    float cdt;
    float ri;
    float x2;
    float y2;
    float z2;
    float si;
    float Si;
};

int EventToLorCoordinates(
        const CoincEvent & data,
        const double total_panel_distance,
        double & x1,
        double & x2,
        double & y1,
        double & y2,
        double & z1,
        double & z2)
{

    x1 = (XMODULEPITCH - 8 * XCRYSTALPITCH) / 2
         + (data.m1 - 8) * XMODULEPITCH
         + (std::floor(data.crystal1/8) + 0.5) * XCRYSTALPITCH;
    x2 = (XMODULEPITCH - 8 * XCRYSTALPITCH) / 2
         + (data.m2 - 8) * XMODULEPITCH 
         + (7 - std::floor(data.crystal2 / 8) + 0.5) * XCRYSTALPITCH;

    y1 = -total_panel_distance / 2
         - data.apd1 * YDISTANCEBETWEENAPDS
         - ((7 - std::floor(data.crystal1 % 8) * YCRYSTALPITCH) + 0.5);
    y2 = total_panel_distance / 2
         + data.apd2*YDISTANCEBETWEENAPDS
         + ((7 - std::floor(data.crystal2 % 8) * YCRYSTALPITCH) + 0.5);                              

    z1 = (((data.cartridge1 - CARTRIDGES_PER_PANEL/2.0) * FINS_PER_CARTRIDGE) + data.fin1 + 0.5) * ZPITCH;
    z2 = (((data.cartridge2 - CARTRIDGES_PER_PANEL/2.0) * FINS_PER_CARTRIDGE) + data.fin2 + 0.5) * ZPITCH;

    return(0);
}

int main(int argc, char ** argv)
{
    if (argc == 1) {
        usage();
        return(0);
    }

    string filename("");
    bool verbose(false);
    float panel_distance(-1);
    string left_calibration_filename;
    string right_calibration_filename;

    bool randomly_subsample_flag(false);
    double subsample_ratio(-1);

    long random_seed(time(NULL));

    for (int ix = 1; ix < argc; ix++) {
		if(strcmp(argv[ix], "-v") == 0) {
			cout << "Verbose Mode " << endl;
			verbose = true;
		}
		if(strcmp(argv[ix], "-h") == 0) {
		  usage();
		  return(0);
		}
		if(strcmp(argv[ix], "--help") == 0) {
		  usage();
		  return(0);
		}
    }

    for (int ix = 1; ix < (argc - 1); ix++) {
        if (strcmp(argv[ix], "-p") == 0) {
            panel_distance = atof(argv[ix + 1]);
            cout << " Using panel distance " << panel_distance << " mm." << endl;
		}
        if(strcmp(argv[ix], "-f") == 0) {
            filename = string(argv[ix + 1]);
        }
        if(strcmp(argv[ix], "-calleft") == 0) {
            left_calibration_filename = string(argv[ix + 1]);
        }
        if(strcmp(argv[ix], "-calright") == 0) {
            right_calibration_filename = string(argv[ix + 1]);
        }
        if(strcmp(argv[ix], "-r") == 0) {
            randomly_subsample_flag = true;
            subsample_ratio = atof(argv[ix + 1]);
        }
        if(strcmp(argv[ix], "-s") == 0) {
            random_seed = atol(argv[ix + 1]);
        }
    }

    if (panel_distance <= 0) {
        cerr << "Panel Distance not specified or invalid" << endl;
        cerr << "Exiting" << endl;
        return(-2);
    }

    if (randomly_subsample_flag) {
        if ((subsample_ratio < 0) || (subsample_ratio > 1)) {
            cerr << "Subsample ratio specified invalid.  Must be 0.0 to 1.0." << endl;
            cerr << "Exiting" << endl;
            return(-3);
        }
        srand(random_seed);
        cout << " random_seed: " << random_seed << endl;
    }

    PixelCal * left_cal = 0;
    if (left_calibration_filename != "") {
        if (verbose) {
            cout << "Opening calibration file" << endl;
        }
        TFile * left_calibration_file = new TFile(left_calibration_filename.c_str());

        if (left_calibration_file->IsZombie()) {
            cerr << "Error opening file : "
                 << left_calibration_filename << endl;
            cerr << "Exiting..." << endl;
            return(-5);
        }

        left_cal = (PixelCal *) left_calibration_file->Get("CrysCalPar");
        if (!left_cal) {
            cerr << "Problem reading CrystalCalPar from "
                 << left_calibration_filename << endl;
            cerr << "Exiting." << endl;
            return(-6);
        }
        if (verbose) {
            cout << "Finished opening left calibration file" << endl;
        }
    }

    PixelCal * right_cal = 0;
    if (right_calibration_filename != "") {
        if (verbose) {
            cout << "Opening right calibration file" << endl;
        }
        TFile * right_calibration_file = new TFile(right_calibration_filename.c_str());

        if (right_calibration_file->IsZombie()) {
            cerr << "Error opening file : "
                 << right_calibration_filename << endl;
            cerr << "Exiting..." << endl;
            return(-5);
        }

        right_cal = (PixelCal *) right_calibration_file->Get("CrysCalPar");
        if (!right_cal) {
            cerr << "Problem reading CrystalCalPar from "
                 << right_calibration_filename << endl;
            cerr << "Exiting." << endl;
            return(-6);
        }
        if (verbose) {
            cout << "Finished opening right calibration file" << endl;
        }
    }


    cout << " Opening file " << filename << endl;

    ofstream outputfile;

    outputfile.open(filename.c_str(), ios::binary);

    long lines(0);
    double total_panel_distance = panel_distance + 2 * YOFFSET;


    for (int cartridge1 = 0; cartridge1 < CARTRIDGES_PER_PANEL; cartridge1++) {
        for (int fin1 = 0; fin1 < FINS_PER_CARTRIDGE; fin1++) {
            for (int module1 = 0; module1 < MODULES_PER_FIN; module1++) {
                for (int apd1 = 0; apd1 < APDS_PER_MODULE; apd1++) {
                    // If the peaks valid flag in the calibration file is false
                    // then skip all of those crystals
                    if (!left_cal) {
                        if (!left_cal->validpeaks[cartridge1][fin1][module1][apd1]) {
                            continue;
                        }
                    }
                    for (int cartridge2 = 0; cartridge2 < CARTRIDGES_PER_PANEL; cartridge2++) {
                        for (int fin2 = 0; fin2 < FINS_PER_CARTRIDGE; fin2++) {
                            for (int module2 = 0; module2 < MODULES_PER_FIN; module2++) {
                                for (int apd2 = 0; apd2 < APDS_PER_MODULE; apd2++) {
                                    // If the peaks valid flag in the calibration file is false
                                    // then skip all of those crystals
                                    if (!right_cal) {
                                        if (!right_cal->validpeaks[cartridge2][fin2][module2][apd2]) {
                                            continue;
                                        }
                                    }
                                    for (int crystal1 = 0; crystal1 < CRYSTALS_PER_APD; crystal1++) {
                                        for (int crystal2 = 0; crystal2 < CRYSTALS_PER_APD; crystal2++) {
                                            // If the user has chosen it, only write out subsample_ratio * total
                                            // events, by generating a random number between 0 and 1 and accepting
                                            // the event if it is less than that.
                                            if (randomly_subsample_flag) {
                                                double random_value((double) rand() / (double) RAND_MAX);
                                                if (random_value > subsample_ratio) {
                                                    continue;
                                                }
                                            }
//                                            CoincEvent event;
//                                            event.cartridge1 = cartridge1;
//                                            event.cartridge2 = cartridge2;
//                                            event.fin1 = fin1;
//                                            event.fin2 = fin2;
//                                            event.m1 = module1;
//                                            event.m2 = module2;
//                                            event.apd1 = apd1;
//                                            event.apd2 = apd2;
//                                            event.crystal1 = crystal1;
//                                            event.crystal2 = crystal2;



                                            double x1 = (XMODULEPITCH-8*XCRYSTALPITCH)/2+(module1-8)*XMODULEPITCH;  
                                            double x2 = (XMODULEPITCH-8*XCRYSTALPITCH)/2+(module2-8)*XMODULEPITCH;

                                            x1 +=  ( TMath::Floor(crystal1/8)  + 0.5  )*XCRYSTALPITCH;
                                            x2 +=  ( 7-TMath::Floor(crystal2/8)  + 0.5  )*XCRYSTALPITCH;

                                            double y1 = -total_panel_distance/2;
                                            double y2 = total_panel_distance/2;
                                            y1 -= apd1*YDISTANCEBETWEENAPDS;
                                            y2 += apd2*YDISTANCEBETWEENAPDS;
                                            y1 -= (( 7-TMath::Floor(crystal1%8) * YCRYSTALPITCH ) + 0.5  );
                                            y2 += (( 7-TMath::Floor(crystal2%8) * YCRYSTALPITCH ) + 0.5  );                              

                                            double z1 = (((cartridge1 - CARTRIDGES_PER_PANEL/2.0) * FINS_PER_CARTRIDGE) + fin1 + 0.5) * ZPITCH;
                                            double z2 = (((cartridge2 - CARTRIDGES_PER_PANEL/2.0) * FINS_PER_CARTRIDGE) + fin2 + 0.5) * ZPITCH;

//                                            double x1(0);
//                                            double x2(0);
//                                            double y1(0);
//                                            double y2(0);
//                                            double z1(0);
//                                            double z2(0);
//
//
//                                            EventToLorCoordinates(event,
//                                                    total_panel_distance,
//                                                    x1, x2, y1, y2, z1, z2);

                                            lor_buffer buffer; 
                                            buffer.x1 = x1;
                                            buffer.x2 = x2;
                                            buffer.y1 = y1;
                                            buffer.y2 = y2;
                                            buffer.z1 = z1;
                                            buffer.z2 = z2;
                                            buffer.cdt = 0;
                                            buffer.ri = 0;
                                            buffer.si = 0;
                                            buffer.Si = 0;

                                            lines++;

                                            outputfile.write((char *) & buffer, sizeof(buffer));
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    cout << " LORS Written: " << lines << endl;

    return(0);
}

