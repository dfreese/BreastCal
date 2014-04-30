#include "format_recon_dca_ana.h"
//void usage(void);

//void usage(void){
// cout << " mergecal -fl [filename] -fr [filename] [-t [threshold] -v ]" <<endl;
//  return;}


#define NROFPOINTSOURCES 7

int main(int argc, Char_t *argv[])
{
 	cout << "Welcome " << endl;

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	Char_t		filename[FILENAMELENGTH] = "";
	Int_t		verbose = 0, threshold=-1000;
	Int_t		ix,ascii;
	 buf buffer; 
         Int_t PANELDISTANCE=-1; // mm
         Bool_t RANDOMS=0;
	 Float_t FINETIMEWINDOW=40;
         Double_t x0[NROFPOINTSOURCES]={-55.62,-35.62,-15.70,4.28,24.23,44.1,64.39};
         Double_t y0[NROFPOINTSOURCES]={3.33,3.33,3.54,3.75,3.96,4.17,4.38};
         Double_t z0[NROFPOINTSOURCES]={0,0,0,0,0,0,0};

         Int_t pos;
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

		if (strncmp(argv[ix],"-pos",4)==0){
		  pos = atoi(argv[ix+1]);
		  ix++;
		}

		/*
                if(strncmp(argv[ix], "-x0",3)==0){
	          ix++; x0 = atof(argv[ix]);
		  ix++; y0 = atof(argv[ix]);
		  ix++; z0 = atof(argv[ix]);
		}
		*/ 	if(strncmp(argv[ix], "-r", 2) == 0) {
			cout << "RANDOMS SELECTION " << endl;
			RANDOMS = 1;
		}


                if (strncmp(argv[ix],"-p",2) ==0 ) {
                  ix++;
		  PANELDISTANCE=atoi(argv[ix]);
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


		if(strncmp(argv[ix], "-f", 2) == 0) {

		if(strncmp(argv[ix], "-ft", 3) == 0) {
                  threshold = atoi( argv[ix+1]);
		  cout << "Threshold =  " << threshold << " ( not implemented yet ) " << endl;
                  ix++;
		}

		else {

		/* filename '-f' */

			if(strlen(argv[ix + 1]) < FILENAMELENGTH) {
				sprintf(filename, "%s", argv[ix + 1]);
			}
			else {
				cout << "Filename " << argv[ix + 1] << " too long !" << endl;
				cout << "Exiting.." << endl;
				return -99;
			}
		}

                }
	} // loop over arguments



	//        rootlogon(verbose);


         TFile *f = new TFile("DCA.root","UPDATE");
         TH1D *DCA[NROFPOINTSOURCES];
         TString name;
         TString title;
          for (Int_t i=0;i<NROFPOINTSOURCES;i++){
	    name.Form("DCA[%d]",i);
            title.Form(" Source position %d",i);
	    DCA[i] = new TH1D(name.Data(),title.Data(),200,0,20);   
	  }


        if (PANELDISTANCE<0) { cout << "please specify paneldistance: -p [DISTANCE].\nExiting." << endl;return -2;}

        if (RANDOMS) cout << " Reformatting for RANDOMS " << endl;
        else {  cout << " Fine Time window =  " << FINETIMEWINDOW << "  " << endl;}

        Char_t filebase[FILENAMELENGTH],rootfile[FILENAMELENGTH]; 
        Char_t asciifile[FILENAMELENGTH], outfile[FILENAMELENGTH]; 

        ifstream infile;

        cout << " Opening file " << filename << endl;
        TFile *file = new TFile(filename,"OPEN");
        TTree *m = (TTree *) file->Get("merged");
        CoincEvent *data = new CoincEvent();

        if (!m) {            m  = (TTree *) file->Get("mana"); }  




        if (!m) { cout << " Problem reading branch 'merged' or 'mana'  from file "<<  filename << endl; 
	  cout << " Exiting. " << endl; return -99;}

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
     
        sprintf(outfile,"%s.cuda",filebase);
	//        outputfile.open(outfile);

       Long64_t entries_m = m->GetEntries();


       cout << " Processing " <<  entries_m  <<  " Events " << endl;

      Long64_t i;
 
      Double_t x1,y1,z1,x2,y2,z2;
      Double_t x3,y3,z3;
      Double_t x10,y10,z10;
      Double_t x21,y21,z21;

      Double_t x10_2,x21_2,x10x21,d;

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

      for (i=0;i<entries_m;i++){

	m->GetEntry(i);

        Int_t sourcepos = TMath::Floor((data->pos-15e3)/20e3);

	// YOUR CODE HERE -- YOU HAVE ACCESS TO THE STRUCT DATA AND ITS MEMBERS TO DO WHATEVER CONSTRAINTS //

	// The different constraint here compared to merged_ana, is that we assume that merged_ana already did a valid random selection on dtc,
        // the selection is only converted to CUDA readable format. 
        if ((RANDOMS) || ( TMath::Abs(data->dtf)<FINETIMEWINDOW)) {  
	  if ( ( data->E1<700 ) && (data->E1> 400 ) ) {
        	  if ( ( data->E2<700 ) && (data->E2> 400 ) ) {
		    if ( (data->crystal1>=0 ) && (data->crystal1<64 )) {
           		    if ( (data->crystal2>=0 ) && (data->crystal2<64 )) {
				//	if ( data->apd1 ) continue;


                              y1 = TOTALPANELDISTANCE/2;
                              y2 = -TOTALPANELDISTANCE/2;
                              y1 +=   data->apd1*YDISTANCEBETWEENAPDS;
                              y2 -=   data->apd2*YDISTANCEBETWEENAPDS;
			      y1 +=  (( 7-TMath::Floor(data->crystal1%8) * YCRYSTALPITCH ) + 0.5  );
			      y2 -=  (( 7-TMath::Floor(data->crystal2%8) * YCRYSTALPITCH ) + 0.5  );                              
			
			      x1 = (XMODULEPITCH-8*XCRYSTALPITCH)/2+(data->m1-8)*XMODULEPITCH;  
			      x2 = (XMODULEPITCH-8*XCRYSTALPITCH)/2+(7-data->m2)*XMODULEPITCH;  
			      x1 +=  ( TMath::Floor(data->crystal1/8)  + 0.5  )*XCRYSTALPITCH;
			      x2 +=  ( 7-TMath::Floor(data->crystal2/8)  + 0.5  )*XCRYSTALPITCH;
                              
                              
                              
                              z1 = -ZPITCH/2+(4-data->fin1)*ZPITCH;
          		      z2 = -ZPITCH/2+(4-data->fin2)*ZPITCH;
			      // z1=0;
			      //  z2=0;
 

                              x10 = x1- x0[sourcepos];
                              y10 = y1- y0[sourcepos];
                              z10 = z1- z0[sourcepos];

                              x21 = x2 - x1;
                              y21 = y2 - y1;
                              z21 = z2 - z1;
 

                              x10_2 = x10*x10 + y10*y10 + z10*z10;
			      x21_2 = x21*x21 + y21*y21 + z21*z21;
                              x10x21 = ( x10*x21 + y10*y21 + z10*z21);

                              d = ( x10_2*x21_2 - x10x21*x10x21) / ( x21*x21 + y21*y21 +z21*z21 ) ;
  
			      DCA[sourcepos]->Fill(d);
 

             // this is a good data now ..
             // do something+ write to disk
			      buffer.x1 = x1;
                              buffer.x2 = x2;
                              buffer.y1 = y1;
			      buffer.y2 = y2;
			      buffer.z1 = z1;
			      buffer.z2 = z2;

                              lines++;

			      if (ascii) asciiout << x1 << " " <<y1 << " " << z1 << " " << x2 << " " << y2 << " " << z2 << endl;

			      //			      outputfile.write( (char *) &buffer, sizeof(buffer));
                 }



			    }
		    }
		  }
	  }
      
           
      }


      /*
      TCanvas *c1 = new TCanvas();
      DCA->Draw();
      c1->Print("DCA.png");
      */

      for (Int_t i=0;i<NROFPOINTSOURCES;i++){
	DCA[i]->Write();}
      f->Close();
 
       // loop over entries
      if (ascii) asciiout.close();

      cout << " wrote " << lines << " LORS. " << endl;


#define COARSEDIFF 100       



  
  

      //       calfile->Close();



 
      return 0;}


