#include "format_recon.h"
void usage(void);

void usage(void){
    cout << " format_recon [-r -v -a] -f [filename] -p [panelseparation] -t [finetimewindow] -ft [threshold] -mt [maxtime]]" <<endl;
    cout << " -r : needed for randoms " << endl;
    cout << " -v : verbose " << endl;
    cout << " -a : ascii  " << endl;
    cout << " -mt :: only give the first maxtime minutes " << endl;
    cout << " -ap : asymmetric panel separation.  Assume p is near wall, ap is room side" << endl;
    return;
}




int main(int argc, Char_t *argv[])
{
    cout << "Welcome " << endl;

    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    Char_t		filename[FILENAMELENGTH] = "";
    Int_t		verbose = 0, threshold=-1000;
    Int_t		ix,ascii;
    buf buffer; 
    Float_t PANELDISTANCE=-1; // mm
    Bool_t RANDOMS=0;
    Float_t FINETIMEWINDOW=40;
    Int_t MAXTIME=999999999;
    bool use_asym_panel_sep(false);
    float panel_sep_wall(0);
    float panel_sep_room(0);
    float panel_sep_avg(0);
    float panel_sep_delta(0);
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    ascii=0;

    for(ix = 1; ix < argc; ix++) {

        /*
         * Verbose '-v'
		 */
		if(strncmp(argv[ix], "-v", 2) == 0) {
			cout << "Verbose Mode " << endl;
			verbose = 1;
		}

		if(strncmp(argv[ix], "-h", 2) == 0) {
		  usage();
		  return 0;
		}

        if(strncmp(argv[ix], "-r", 2) == 0) {
			cout << "RANDOMS SELECTION " << endl;
			RANDOMS = 1;
		}

        if (strncmp(argv[ix],"-p",2) ==0 ) {
            ix++;
            PANELDISTANCE=atof(argv[ix]);
            cout << " Using panel distance " << PANELDISTANCE << " mm." << endl;
		}

        if(strncmp(argv[ix], "-a", 2) == 0) {
            cout << "Ascii output file generated" << endl;
            ascii = 1;
        }

        if(strncmp(argv[ix], "-t", 2) == 0) {
            FINETIMEWINDOW = atoi( argv[ix+1]);
            ix++;
        }

        if(strncmp(argv[ix], "-mt", 3) == 0) {
            MAXTIME = atoi( argv[ix+1]);
            cout <<"Only events within first " << MAXTIME << " minutes." << endl;
            ix++;
        }

        if(strncmp(argv[ix], "-ap", 3) == 0) {
            use_asym_panel_sep = true;
            panel_sep_room = atoi( argv[ix+1]);
            ix++;
        }


        if(strncmp(argv[ix], "-f", 2) == 0) {
            if(strncmp(argv[ix], "-ft", 3) == 0) {
                threshold = atoi( argv[ix+1]);
                cout << "Threshold =  " << threshold << " ( not implemented yet ) " << endl;
                ix++;
            } else {
                /* filename '-f' */

                if(strlen(argv[ix + 1]) < FILENAMELENGTH) {
                    sprintf(filename, "%s", argv[ix + 1]);
                } else {
                    cout << "Filename " << argv[ix + 1] << " too long !" << endl;
                    cout << "Exiting.." << endl;
                    return(-99);
                }
            }

        }
    } // loop over arguments

    rootlogon(verbose);

    if (PANELDISTANCE<0) {
        cout << "please specify paneldistance: -p [DISTANCE].\nExiting." << endl;
        return(-2);
    }

    if (RANDOMS) {
        cout << " Reformatting for RANDOMS " << endl;
    } else {
        cout << " Fine Time window =  " << FINETIMEWINDOW << "  " << endl;
    }

    Char_t filebase[FILENAMELENGTH],rootfile[FILENAMELENGTH]; 
    Char_t asciifile[FILENAMELENGTH], outfile[FILENAMELENGTH]; 

    ifstream infile;

    cout << " Opening file " << filename << endl;
    TFile *file = new TFile(filename,"OPEN");
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
    strncpy(filebase,filename,strlen(filename)-5);
    filebase[strlen(filename)-5]='\0';
    sprintf(rootfile,"%s",filebase);
    strcat(rootfile,".merged.root");
    if (ascii){
        sprintf(asciifile,"%s.merged.ascii",filebase);
        asciiout.open(asciifile);
    }
    cout << " Opening file " << rootfile << " for writing " << endl;
    // OPEN YOUR OUTPUTFILE HERE ! 

    if (RANDOMS) {
        sprintf(outfile,"%s_p%.2f_random.cuda",filebase,PANELDISTANCE);
    } else {
        sprintf(outfile,"%s_p%.2f_t%.0f.cuda",filebase,PANELDISTANCE, FINETIMEWINDOW);
    }
    outputfile.open(outfile);

    Long64_t entries_m = m->GetEntries();


    cout << " Processing " <<  entries_m  <<  " Events " << endl;

    Long64_t i;

    Double_t x1,y1,z1,x2,y2,z2;

    buffer.cdt =0;
    buffer.ri=0;
    buffer.si=0;
    buffer.Si=0;


#define INCHTOMM 25.4

#define XCRYSTALPITCH 1
#define YCRYSTALPITCH 1
#define XMODULEPITCH 0.405*INCHTOMM
    // #define YOFFSET 0.057*INCHTOMM // 1.4478 mm   -  1.51 -> 0.057 is distance to the hole, distance to module is 0.0595
#define YOFFSET 1.51 // mm
#define YDISTANCEBETWEENAPDS (0.32+0.079)*INCHTOMM  // 10.1346 mm
#define ZPITCH 0.0565*INCHTOMM //

    Long64_t lines=0;
    Double_t TOTALPANELDISTANCE=PANELDISTANCE+2*YOFFSET;

    cout << "X:: PITCH : " << XMODULEPITCH << " mm." << endl;
    cout << "Y:: PANELDISTANCE : " << PANELDISTANCE << " mm, APD0 @ " << PANELDISTANCE+YOFFSET;
    cout << " mm, APD1 @ " << PANELDISTANCE+YOFFSET+YDISTANCEBETWEENAPDS << " mm."<<endl ;
    cout << "Z:: ZPITCH : " << ZPITCH << " mm."<<endl;

    Int_t p2=0;
    Int_t p1=0;

    if (use_asym_panel_sep) {
        panel_sep_wall = TOTALPANELDISTANCE;
        panel_sep_room += 2*YOFFSET;
        panel_sep_avg = (panel_sep_wall + panel_sep_room) / 2;
        panel_sep_delta = (panel_sep_room - panel_sep_wall ) / 164.5;
    }


    Long64_t firsteventtime=0;
    Long64_t firsteventtimeset=kFALSE;

    for (i=0;i<entries_m;i++) {

        m->GetEntry(i);

        // YOUR CODE HERE -- YOU HAVE ACCESS TO THE STRUCT DATA AND ITS MEMBERS TO DO WHATEVER CONSTRAINTS //

        // The different constraint here compared to merged_ana, is that we assume that merged_ana already did a valid random selection on dtc,
        // the selection is only converted to CUDA readable format. 
        if ((RANDOMS) || ( TMath::Abs(data->dtf)<FINETIMEWINDOW)) {  
            if ( ( data->E1<700 ) && (data->E1> 400 ) ) {
                if ( ( data->E2<700 ) && (data->E2> 400 ) ) {
                    if ( (data->crystal1>=0 ) && (data->crystal1<64 )) {
                        if ( (data->crystal2>=0 ) && (data->crystal2<64 )) {

                            if (!firsteventtimeset){ 
                                firsteventtime=data->ct;
                                firsteventtimeset=kTRUE;
                                cout << "firsteventtime : " << firsteventtime/(60*COARSECLOCKFREQUENCY) << " min" << endl;
                            }


                            // NOTE:: X was pointing differently before !

                            x1 = (XMODULEPITCH-8*XCRYSTALPITCH)/2+(data->m1-8)*XMODULEPITCH;  
                            x2 = (XMODULEPITCH-8*XCRYSTALPITCH)/2+(data->m2-8)*XMODULEPITCH;

			    x1 +=  ( TMath::Floor(data->crystal1/8)  + 0.5  )*XCRYSTALPITCH;
			    x2 +=  ( 7-TMath::Floor(data->crystal2/8)  + 0.5  )*XCRYSTALPITCH;
			   
                            // This "x2" is for ROTATED SETUP:: 
                            // x2 = (XMODULEPITCH-8*XCRYSTALPITCH)/2+(7-data->m2)*XMODULEPITCH;  
                           
                            if (use_asym_panel_sep) {
                                y1 = (panel_sep_avg + x1 * panel_sep_delta)/2;
                                y2 = (panel_sep_avg + x2 * panel_sep_delta)/-2;
                            } else {
                                y1 = TOTALPANELDISTANCE/2;
                                y2 = -TOTALPANELDISTANCE/2;
                            }
                            y1 +=   data->apd1*YDISTANCEBETWEENAPDS;
                            y2 -=   data->apd2*YDISTANCEBETWEENAPDS;
                            y1 +=  (( 7-TMath::Floor(data->crystal1%8) * YCRYSTALPITCH ) + 0.5  );
                            y2 -=  (( 7-TMath::Floor(data->crystal2%8) * YCRYSTALPITCH ) + 0.5  );                              

			    // David commented out the below 2 lines to test something.
			    //                            z1 = ZPITCH/2+(data->fin1-4)*ZPITCH;
			    //                            z2 = ZPITCH/2+(data->fin2-4)*ZPITCH;

			    z1 = ZPITCH/2+(7-data->fin1)*ZPITCH - (data->cartridge1)*ZPITCH*8;
			    z2 = ZPITCH/2+(7-data->fin2)*ZPITCH - (data->cartridge2)*ZPITCH*8;



                            // this is a good data now ..
                            // do something+ write to disk
                            buffer.x1 = x1;
                            buffer.x2 = x2;
                            buffer.y1 = y1;
                            buffer.y2 = y2;
                            buffer.z1 = z1;
                            buffer.z2 = z2;

                            lines++;

                            if (ascii) {
                                asciiout << x1 << " " <<y1 << " " << z1 << " " << x2 << " " << y2 << " " << z2 << endl;
                            }

                            outputfile.write( (char *) &buffer, sizeof(buffer));
                        }
                    }
                }
            }
        }
    }
    cout << " p1 : "<< p1 <<" p2 : " << p2 << endl;
    // loop over entries
    if (ascii) {
        asciiout.close();
    }
    cout << " wrote " << lines << " LORS. " << endl;
#define COARSEDIFF 100       
    return(0);
}

