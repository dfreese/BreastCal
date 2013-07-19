#include "format_recon.h"
//void usage(void);

//void usage(void){
// cout << " mergecal -fl [filename] -fr [filename] [-t [threshold] -v ]" <<endl;
//  return;}

int main(int argc, Char_t *argv[])
{
 	cout << "Welcome " << endl;

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	Char_t		filename[FILENAMELENGTH] = "";
	Int_t		verbose = 0, threshold=-1000;
	Int_t		ix,ascii;
	//module UNIT0,UNIT1,UNIT2,UNIT3;
	//        event           evt;

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


		if(strncmp(argv[ix], "-a", 2) == 0) {
			cout << "Ascii output file generated" << endl;
			ascii = 1;
		}


		if(strncmp(argv[ix], "-t", 2) == 0) {
                  threshold = atoi( argv[ix+1]);
		  cout << "Threshold =  " << threshold << " ( not implemented yet ) " << endl;
                  ix++;
		}

		/* filename '-f' */
		if(strncmp(argv[ix], "-f", 3) == 0) {
			if(strlen(argv[ix + 1]) < FILENAMELENGTH) {
				sprintf(filename, "%s", argv[ix + 1]);
			}
			else {
				cout << "Filename " << argv[ix + 1] << " too long !" << endl;
				cout << "Exiting.." << endl;
				return -99;
			}
		}


	} // loop over arguments

        rootlogon(verbose);



        Char_t filebase[FILENAMELENGTH],rootfile[FILENAMELENGTH]; 
        Char_t asciifile[FILENAMELENGTH]; 
	//       Char_t tmpname[20],tmptitle[50];
	//        Int_t i,j,k,m,lines;
	//        Double_t aa, bb;
	//        Int_t augment;
        ifstream infile;
	//        Int_t evts=0;
	//        strncpy(filebase,filename,strlen(filename)-17);
	//        filebase[strlen(filename)-17]='\0';
	//        sprintf(rootfile,"%s",filename);
	//        strcat(rootfile,".root");
              
	//	cout << "Rootfile to open :: " << rootfile << endl;


        cout << " Opening file " << filename << endl;
        TFile *file = new TFile(filename,"OPEN");
        TTree *m = (TTree *) file->Get("merged");
        CoincEvent *data;

	m->SetBranchAddress("event",&data);
	/*
	data.dtc   - Coarse time difference
        data.dtf   - Fine Time difference
        data.E1    - Energy spatial channel L
	data.Ec1   - Energy common L
        data.Ech1  - Energy common high L
        data.ft1   - fine time L
        data.E2    
	data.Ec2
        data.Ech2
        data.ft2
        data.x1 - don't use
        data.y1 - don't use
        data.x2 - don't use
        data.y2 - don't use
        data.chip1   - Chip number - not important
        data.fin1   - Fin number
        data.m1     - Module number (  0-> 15 )
        data.apd1   - Apd number ( 0 or 1, 0 is front, 1 is back )
        data.crystal1 - Crystal number ( 0 to 63, -1 or >64 is some error )
        data.chip2
        data.fin2
        data.m2
        data.apd2
        data.crystal2
        data.pos  - position of point source
	*/


	ofstream asciiout;

	  // Open output file - matching required format for ALEX //
	strncpy(filebase,filename,strlen(filename)-17);
         filebase[strlen(filename)-5]='\0';
         sprintf(rootfile,"%s",filebase);
         strcat(rootfile,".merged.root");
        if (ascii){
	  sprintf(asciifile,"%s.merged.ascii",filebase);
          asciiout.open(asciifile);
	    }
        cout << " Opening file " << rootfile << " for writing " << endl;
	// OPEN YOUR OUTPUTFILE HERE ! 
     


       Long64_t entries_m = m->GetEntries();


       cout << " Processing " <<  entries_m  <<  " Events " << endl;

      Long64_t i;
 
      Double_t x1,y1,z1,x2,y2,z2;

      for (i=0;i<entries_m;i++){

	m->GetEntry(i);

	// YOUR CODE HERE -- YOU HAVE ACCESS TO THE STRUCT DATA AND ITS MEMBERS TO DO WHATEVER CONSTRAINTS //
        if ( TMath::Abs(data->dtc)<6) {  
	  if ( ( data->E1<700 ) && (data->E1> 400 ) ) {
        	  if ( ( data->E2<700 ) && (data->E2> 400 ) ) {
		    if ( (data->crystal1>0 ) && (data->crystal1<64 )) {
           		    if ( (data->crystal2>0 ) && (data->crystal2<64 )) {

#define PANELDISTANCE 60 // mm

                              y1 = PANELDISTANCE/2;
                              y2 = -PANELDISTANCE/2;
                              y1 +=   data->apd1*10;
                              y2 -=   data->apd2*10;
			      y1 +=  (( TMath::Floor(data->crystal1%8)- 4 )*0.5  );
			      y2 +=  (( TMath::Floor(data->crystal1%8)- 4 )*0.5  );                              
			
			      x1 = (data->m1-8)*0.405*25.4;  
                              x2 = (8-data->m2)*0.405*25.4;  
			      x1 +=  (( TMath::Floor(data->crystal1/8)- 4 )*0.5  );
                              x2 +=  (( 4 - TMath::Floor(data->crystal1/8))*0.5  );
                              
                              z1 = data->fin1*0.056*25.4;
          		      z2 = data->fin2*0.056*25.4;
                             
 
             // this is a good data now ..
             // do something+ write to disk


		  }
         	  }
	}
	  }
	} 
           
          
      } // loop over entries
             

#define COARSEDIFF 100       



  
  

      //       calfile->Close();



 
	  return 0;}


