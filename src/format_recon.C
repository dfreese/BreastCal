#include "format_recon.h"
#include <string>
#include <cal_helper_functions.h>

#define INCHTOMM 25.4
#define XCRYSTALPITCH 1
#define YCRYSTALPITCH 1
#define XMODULEPITCH 0.405*INCHTOMM
#define YOFFSET 1.51 // mm
#define YDISTANCEBETWEENAPDS (0.32+0.079)*INCHTOMM  // 10.1346 mm
#define ZPITCH 0.0565*INCHTOMM //

void usage(void){
    cout << " format_recon [-r -v -a] -f [filename] -p [panelseparation] -t [finetimewindow] -mt [maxtime]]" <<endl;
    cout << " -r : needed for randoms " << endl;
    cout << " -v : verbose " << endl;
    cout << " -a : ascii  " << endl;
    cout << " -mt :: only give the first maxtime minutes " << endl;
    cout << " -eh: Set Upper Energy Window - Default is 700keV\n"
         << " -el: Set Lower Energy Window - Default is 400keV\n"
         << " -dtc: Enable Course Timestamp Windowing\n"
         << " -dtcl: Set Lower Course Timestamp Window - Default 0\n"
         << " -dtcl: Set Lower Course Timestamp Window - Default 4\n"
         << " -rcal (filename):  read in initial per crystal calibration\n";
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
    int ascii(0);
    Float_t PANELDISTANCE(-1); // mm
    Bool_t RANDOMS(0);
    Float_t FINETIMEWINDOW(40);
    Int_t MAXTIME(999999999);
    float energy_gate_high(700);
    float energy_gate_low(400);
    int dtc_gate_low(0);
    int dtc_gate_high(4);
    bool use_dtc_gating_flag(false);
    bool read_per_crystal_correction(false);
    string input_crystal_cal_filename;

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
        if(strcmp(argv[ix], "-r") == 0) {
			cout << "RANDOMS SELECTION " << endl;
			RANDOMS = 1;
		}
        if(strcmp(argv[ix], "-a") == 0) {
            cout << "Ascii output file generated" << endl;
            ascii = 1;
        }
        if(strcmp(argv[ix], "-dtc") == 0) {
            use_dtc_gating_flag = true;
        }
    }

    // Options requiring input
    for(int ix = 1; ix < (argc - 1); ix++) {
        if (strcmp(argv[ix],"-p") ==0 ) {
            PANELDISTANCE=atof(argv[ix + 1]);
            cout << " Using panel distance " << PANELDISTANCE << " mm." << endl;
		}
        if(strcmp(argv[ix], "-t") == 0) {
            FINETIMEWINDOW = atoi(argv[ix + 1]);
        }
        if(strcmp(argv[ix], "-mt") == 0) {
            MAXTIME = atoi( argv[ix+1]);
            cout <<"Only events within first " << MAXTIME << " minutes." << endl;
        }
        if(strcmp(argv[ix], "-f") == 0) {
            filename = string(argv[ix + 1]);
        }
        if(strcmp(argv[ix], "-eh") == 0) {
            stringstream ss;
            ss << argv[ix + 1];
            ss >> energy_gate_high;
        }
        if(strcmp(argv[ix], "-el") == 0) {
            stringstream ss;
            ss << argv[ix + 1];
            ss >> energy_gate_low;
        }
        if(strcmp(argv[ix], "-dtcl") == 0) {
            stringstream ss;
            ss << argv[ix + 1];
            ss >> dtc_gate_low;
            use_dtc_gating_flag = true;
        }
        if(strcmp(argv[ix], "-dtch") == 0) {
            stringstream ss;
            ss << argv[ix + 1];
            ss >> dtc_gate_high;
            use_dtc_gating_flag = true;
        }
        if(strcmp(argv[ix], "-rcal") == 0) {
            read_per_crystal_correction = true;
            input_crystal_cal_filename = string(argv[ix + 1]);
        }
    }

    rootlogon(verbose);

    if (PANELDISTANCE<0) {
        cerr << "Panel Distance not specified or invalid" << endl;
        cerr << "Exiting" << endl;
        return(-2);
    }

    float crystal_cal[SYSTEM_PANELS]
            [CARTRIDGES_PER_PANEL]
            [FINS_PER_CARTRIDGE]
            [MODULES_PER_FIN]
            [APDS_PER_MODULE]
            [CRYSTALS_PER_APD] = {{{{{{0}}}}}};

    if (read_per_crystal_correction) {
        int cal_read_status(ReadPerCrystalCal(
                input_crystal_cal_filename,crystal_cal));
        if (cal_read_status < 0) {
            cerr << "Error in reading input calibration file: "
                 << cal_read_status << endl;
            cerr << "Exiting.." << endl;
            return(-3);
        }
    }

    if (RANDOMS) {
        cout << " Reformatting for RANDOMS " << endl;
    } else {
        cout << " Fine Time window =  " << FINETIMEWINDOW << "  " << endl;
    }


    cout << " Opening file " << filename << endl;
    TFile *file = new TFile(filename.c_str(),"OPEN");
    TTree *m = (TTree *) file->Get("merged");
    CoincEvent *data = new CoincEvent();

    if (!m) {
        m  = (TTree *) file->Get("mana");
    }

    if (!m) {
        cout << " Problem reading branch 'merged' or 'mana'  from file "<<  filename << endl; 
        cout << " Exiting. " << endl;
        return(-99);
    }

    m->SetBranchAddress("Event",&data);

    ofstream asciiout,outputfile;

    // Open output file - matching required format for ALEX //
    size_t root_pos(filename.rfind(".root"));
    
    if (root_pos == string::npos) {
        cout << "Root file does not end in \".root\".  Exiting..." << endl;
        return(-5);
    }

    string filebase(filename.substr(0,root_pos));

    cout << "Filebase: " << filebase << endl;
    

    string rootfile(filebase+string(".merged.root"));
    string asciifile(filebase+string(".merged.ascii"));
    if (ascii){
        asciiout.open(asciifile.c_str());
    }
    cout << " Opening file " << rootfile << " for writing " << endl;
    // OPEN YOUR OUTPUTFILE HERE ! 


    char * temp_name = new char[filebase.length() + 100];
    if (RANDOMS) {
        sprintf(temp_name,"%s_p%.2f_random.cuda",filebase.c_str(),PANELDISTANCE);
    } else {
        sprintf(temp_name,"%s_p%.2f_t%.0f_el%.0f_eh%.0f.cuda",filebase.c_str(),PANELDISTANCE, FINETIMEWINDOW, energy_gate_low, energy_gate_high);
    }
    string outfile(temp_name);
    delete[] temp_name;
    cout << "Outfile: " << outfile << endl;
    outputfile.open(outfile.c_str());

    Long64_t entries_m = m->GetEntries();

    cout << " Processing " <<  entries_m  <<  " Events " << endl;

    Long64_t lines=0;
    Double_t TOTALPANELDISTANCE=PANELDISTANCE+2*YOFFSET;

    cout << "X:: PITCH : " << XMODULEPITCH << " mm." << endl;
    cout << "Y:: PANELDISTANCE : " << PANELDISTANCE << " mm, APD0 @ " << PANELDISTANCE+YOFFSET;
    cout << " mm, APD1 @ " << PANELDISTANCE+YOFFSET+YDISTANCEBETWEENAPDS << " mm."<<endl ;
    cout << "Z:: ZPITCH : " << ZPITCH << " mm."<<endl;

    Long64_t firsteventtime=0;
    Long64_t firsteventtimeset=kFALSE;

    Long64_t events_dropped_fine_time_window(0);
    Long64_t events_dropped_energy_gating(0);
    Long64_t events_dropped_invalid_index(0);

    for (Long64_t i=0; i<entries_m; i++) {
        m->GetEntry(i);

        // YOUR CODE HERE -- YOU HAVE ACCESS TO THE STRUCT DATA AND ITS MEMBERS TO DO WHATEVER CONSTRAINTS //

        // The different constraint here compared to merged_ana, is that we assume that merged_ana already did a valid random selection on dtc,
        // the selection is only converted to CUDA readable format. 
        if (BoundsCheckEvent(*data) == 0) {
            if (EnergyGateEvent(*data, energy_gate_low, energy_gate_high) == 0) {
                data->dtf -= GetEventOffset(*data, crystal_cal);
                if (((TMath::Abs(data->dtf)<FINETIMEWINDOW) && (!use_dtc_gating_flag))
                        || ((data->dtc >= dtc_gate_low) && (data->dtc <= dtc_gate_high) && (use_dtc_gating_flag))
                        || (RANDOMS)) 
                {
                    if (!firsteventtimeset){ 
                        firsteventtime=data->ct;
                        firsteventtimeset=kTRUE;
                        cout << "firsteventtime : " << firsteventtime/(60*COARSECLOCKFREQUENCY) << " min" << endl;
                    }

                    Double_t x1 = (XMODULEPITCH-8*XCRYSTALPITCH)/2+(data->m1-8)*XMODULEPITCH;  
                    Double_t x2 = (XMODULEPITCH-8*XCRYSTALPITCH)/2+(data->m2-8)*XMODULEPITCH;

                    x1 +=  ( TMath::Floor(data->crystal1/8)  + 0.5  )*XCRYSTALPITCH;
                    x2 +=  ( 7-TMath::Floor(data->crystal2/8)  + 0.5  )*XCRYSTALPITCH;

                    Double_t y1 = -TOTALPANELDISTANCE/2;
                    Double_t y2 = TOTALPANELDISTANCE/2;
                    y1 -= data->apd1*YDISTANCEBETWEENAPDS;
                    y2 += data->apd2*YDISTANCEBETWEENAPDS;
                    y1 -= (( 7-TMath::Floor(data->crystal1%8) * YCRYSTALPITCH ) + 0.5  );
                    y2 += (( 7-TMath::Floor(data->crystal2%8) * YCRYSTALPITCH ) + 0.5  );                              

                    Double_t z1 = (((data->cartridge1 - CARTRIDGES_PER_PANEL/2.0) * FINS_PER_CARTRIDGE) + data->fin1 + 0.5) * ZPITCH;
                    Double_t z2 = (((data->cartridge2 - CARTRIDGES_PER_PANEL/2.0) * FINS_PER_CARTRIDGE) + data->fin2 + 0.5) * ZPITCH;

                    buf buffer; 
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

                    if (ascii) {
                        asciiout << x1 << " " <<y1 << " " << z1 << " " << x2 << " " << y2 << " " << z2 << endl;
                    }

                    outputfile.write( (char *) &buffer, sizeof(buffer));
                } else {
                    events_dropped_fine_time_window++;
                }
            } else {
                events_dropped_energy_gating++;
            }
        } else {
            events_dropped_invalid_index++;
        }
    }
    // loop over entries
    if (ascii) {
        asciiout.close();
    }

    cout << " Processed Events: " <<  entries_m << endl;
    cout << " LORS Written: " << lines << endl;
    cout << " Dropped Events:   " << events_dropped_fine_time_window + 
                                     events_dropped_energy_gating + 
                                     events_dropped_invalid_index
                                  << endl;
    cout << "      Time Window: "  << events_dropped_fine_time_window << endl;
    cout << "    Energy Window: " << events_dropped_energy_gating << endl;
    cout << "    Invalid Index: " << events_dropped_invalid_index << endl;

    return(0);
}

