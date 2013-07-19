/*************

 ************/
#include "TROOT.h"
#include "Riostream.h"
#include "TTree.h"
#include "TTreeIndex.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TSpectrum.h"
#include "TSpectrum2.h"
#include "TH2F.h"
#include "TVector.h"
#include "TMath.h"
#include "TF1.h"
//#include "apd_fit.h"
//#include "apd_peaktovalley.h"
//#include "/Users/arne/root/macros/myrootlib.h"
//##include "/home/miil/root/libInit_avdb.h"
#include "./decoder.h"
#include "time.h"
#include "ModuleCal.h"

int main(int argc, Char_t *argv[])
{
  	cout << "Welcome to Merge_4up. Sorting data from each 4up. " << endl;

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	Char_t		filenamel[FILENAMELENGTH] = "";
	Int_t		verbose = 0;
        Int_t ncuts=0;
        Long64_t timeinterval=0,mintime;
	Int_t		ix,rb;
        rb=-99;
        Int_t    left=-9999;
	//        modulecal       UL0,UL1,UL2,UL3;
        ModuleCal *event = new ModuleCal();
        ModuleCal *evt = new ModuleCal(); 
        ModuleCal *unsrt_evt=0;
        TTree*           cal;
        Long64_t lasttime=-1;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

     

	for(ix = 1; ix < argc; ix++) {

		if(strncmp(argv[ix], "-h", 2) == 0) {
			cout << " Usage:  " << endl;
                        cout << " ./merge_panel -f [Filename] -rb [renaboard] --L/--R [-v] " << endl;
                        cout << " Renaboard is either 0,1,2 or 3" << endl;
                        cout << " Specify which panel: --L for left; --R for right " << endl;
			return -1;
		}


		/*
		 * Verbose '-v'
		 */
		if(strncmp(argv[ix], "-v", 2) == 0) {
			cout << "Verbose Mode " << endl;
			verbose = 1;
		}

		if(strncmp(argv[ix], "-rb", 3) == 0) {
			rb = atoi(argv[ix+1]);
                        if (verbose) cout << "Rena board : " << rb <<endl;
                         ix++;
		}


		if(strncmp(argv[ix], "-nc", 3) == 0) {
			ncuts = atoi(argv[ix+1]);
                        if (verbose) cout << "Number of splits : " << ncuts <<endl;
                         ix++;
		}

		if(strncmp(argv[ix], "-ts", 3) == 0) {
		  timeinterval = (Long64_t) atol(argv[ix+1]);
                        if (verbose) cout << "Split at time : " << timeinterval <<endl;
                         ix++;
		}

		if(strncmp(argv[ix], "-lt", 3) == 0) {
		  lasttime = (Long64_t) atol(argv[ix+1]);
                         if (verbose) cout << "Last time : " << lasttime <<endl;
                         ix++;
		}



		/* filename '-f' */
		if(strncmp(argv[ix], "-f", 2) == 0) {
			if(strlen(argv[ix + 1]) < FILENAMELENGTH) {
				sprintf(filenamel, "%s", argv[ix + 1]);
			}
			else {
				cout << "Filename " << argv[ix + 1] << " too long !" << endl;
				cout << "Exiting.." << endl;
				return -99;
			}
			ix++;
                    }
		

                if (strncmp(argv[ix],"--L",3) ==0 ){
                  if (verbose) cout << " Left panel used " << endl;
                  left=1;
		}

                if (strncmp(argv[ix],"--R",3) ==0 ){
                  if (left==1) { cout << "Please specify --l OR --r not both !\n Exiting.\n"; return -99;}
                  if (verbose) cout << " Right panel used " << endl;
                  left=0;

		}
	}

	if (left==-9999) {
          cout << "Please specify which panel we're using: Add --L or --R to command line"  << endl;  return -1;}         

	if ((rb<0)||(rb>3)) {
	    cout << " Please specify a valid renaboard  ( -rb [Renaboard] ). Options are 0,1,2, or 3. Nothing else.\nExiting." ;
            return -1;

	  }           

	if ( lasttime<0) {
	  cout << "Please specify last time as calculated by get_opt_split :: -lt [value]" << endl;
          return -1;}
      
        if ((ncuts>1)&&(timeinterval==0)) {
          cout << " Please specify the time at which to perform the splits:  -ts [timestamp] " << endl ;
          cout << " Exiting " << endl;
          return -1;
	}

             TCanvas *c1;
             c1 = (TCanvas*)gROOT->GetListOfCanvases()->FindObject("c1");
             if (!c1) c1 = new TCanvas("c1","c1",10,10,1000,1000);


        rootlogon(verbose);


        Char_t filebase[FILENAMELENGTH],rootfile[FILENAMELENGTH]; 
        Int_t m;
        ifstream infile;
        strncpy(filebase,filenamel,strlen(filenamel)-9);
        filebase[strlen(filenamel)-9]='\0';
        

        if (verbose){
	cout << " filename = " << filenamel << endl;
	cout << " filebase = " << filebase << endl; 
        cout << " Opening file " << filenamel << endl;}
        TFile *file_left = new TFile(filenamel,"OPEN");

        if (!file_left->IsOpen()) 
	  { cout << "problems opening file " << filenamel ;
            cout << "\n Exiting " << endl; 
            return -11;}


	if (verbose ) {cout << " File Content : " << endl; 
	file_left->ls(); }
        
        


        Long64_t entries;

	// Long64_t maxentries=0;
	//        Int_t maxchip=0;

        Char_t treename[40];

	//        for (m=0;m<RENACHIPS;m++){

	  sprintf(treename,"cal");
          cal = (TTree *) file_left->Get(treename);
          if (!(cal))    {
	    cout << " Problem reading " << treename << " from file. Exiting." << endl;
	    return -11; //            continue;
	  }

          cal->SetBranchAddress("Calibrated Event Data",&unsrt_evt);
	  /*
	  cal->SetBranchAddress("ct",&unsrt_evt.ct);
	  cal->SetBranchAddress("chip",&unsrt_evt.chip);
	  cal->SetBranchAddress("module",&unsrt_evt.module);
	  cal->SetBranchAddress("apd",&unsrt_evt.apd);
	  cal->SetBranchAddress("Ecal",&unsrt_evt.Ecal);
	  cal->SetBranchAddress("E",&unsrt_evt.E);
	  cal->SetBranchAddress("Ec",&unsrt_evt.Ec);
	  cal->SetBranchAddress("Ech",&unsrt_evt.Ech);
	  cal->SetBranchAddress("x",&unsrt_evt.x);
	  cal->SetBranchAddress("y",&unsrt_evt.y);
	  cal->SetBranchAddress("ft",&unsrt_evt.ft);
	  cal->SetBranchAddress("pos",&unsrt_evt.pos);
	  cal->SetBranchAddress("id",&unsrt_evt.id);
	  */
	  /*	  
        cal[m]->SetBranchAddress("U0",&UL[m*4+0]);
        cal[m]->SetBranchAddress("U1",&UL[m*4+1]);
        cal[m]->SetBranchAddress("U2",&UL[m*4+2]);
        cal[m]->SetBranchAddress("U3",&UL[m*4+3]);
	  */


        // Create Tree //
        entries=cal->GetEntries();
	/*       if (verbose) cout << " Entries " << treename << ": " << entries << endl; 
	//        if (entries[m]>maxentries) {maxentries=entries[m];maxchip=m;}
        maxentries=entries;
        cal->GetEntry(entries-1);
        lasttime=unsrt_evt.ct;
*/
	// 	} // loop m

        if (verbose) cout << " Last timestamp = " << lasttime << endl;

	//        Long64_t lasttimes[RENACHIPS];
	//   Int_t skipchip[RENACHIPS];
        Long64_t l,curevent=0;

/*
//        for (m=0;m<RENACHIPS;m++){
        if(entries[m]) {
	  cal[m]->GetEntry(entries[m]-1);
          if (verbose) cout << " Last Timestamp Chip " << m  << ": " << UL[m*4+2].ct << endl;
          lasttimes[m]=UL[m*4+2].ct;
          if ( lasttime <  UL[m*4+2].ct ) { lasttime= UL[m*4+2].ct;}
	  skipchip[m]=0;
          curevent[m]=0;
         }
        else skipchip[m]=1;
//}
 
	//#define MAXHITS 20000000
	//#define MAXCUTS 50

        cout << " Last timestamp = " << lasttime << endl;

        cout << " Maximum entries : " << maxentries << " (chip " << maxchip << ")." << endl;	

*/   

        if ( ncuts > MAXCUTS ) {
          cout << " ERROR ! THERE ARE OVER " << MAXCUTS*MAXHITS << " EVENTS TO PROCESS. " << endl;
          cout << " This is too many !\n Not even trying.\n Bye. " << endl;
          return -999;}

        cout << " Anticipating " << ncuts << " iterations. " << endl;
        
        Int_t k;
        Int_t nrrollovers=0;
        Long64_t timecut[MAXCUTS];

	/*
	for (k=0;k<cuts;k++) {
          cal[maxchip]->GetEntry(TMath::Min((Long64_t) (k+1)*MAXHITS,entries[maxchip]-1));
          timecut[k]=UL[maxchip*4+2].ct;
	  cout << " Time cut " << k << ": " << timecut[k]  << endl;}
	*/
	time_t time_before_filling, time_before_sorting, time_after_sorting, time_after_filling,thistime; 
        Long64_t curtime=0;
       
	//	Long64_t timestep=48e6;
        Long64_t thischiptime;
 	//        lasttime=1e10;
	//	timestep=TMath::Floor(lasttime/10); //1e10;
	//	            timestep=TMath::Floor(lasttime/1000); //1e10;
 
	//	  cout << " Timestep = " << timestep << endl;
  

	  for (k=0; k<ncuts; k++) { timecut[k]=timeinterval*(k+1);}
          
          if (ncuts>0){
            // make sure that timecut[ncuts-1] > last possible time; used to be:
	         timecut[ncuts-1]=lasttime;
           	  }
          else timecut[0]=lasttime;

	  for (Int_t k=0;k<ncuts;k++) {	  cout << " Time cut " << k << ": " << timecut[k]  << endl;}
	  Int_t kk=0,stop,finincr,extra;

	  // do 2 criterion   and while loop  :: while  !((criteria1) and (criteria2))
          // criteria1 set to false if  (firsttime >  timecut ),
          // criteria2 is set to false if criteria1 is false :
	  // here we'll look for a number of events past the event criterion 1 was set
          // if the time of the event is smaller than the cutofftime, we fill the histogram,
	  // IMPORTANT: on next iteration we need to go back to event where criterion 1 happened ( so it seems we need a dual check before filling if  time > prevcutoff && time > prevcutoff
 

       //  while (curtime<lasttimes[6]) {
       //    while ((curtime<lasttime)&&(kk<2)) {
         while ((curtime<lasttime)) {
	   cout << "\n Split number :: " << kk << ", timecut = " << timecut[kk] << endl;
	   //           if (kk > 42) verbose=1;
          time_before_filling = clock();

	  // Open Calfile //
	//        strncpy(filebase,filenamel,strlen(filenamel)-13);
	//        filebase[strlen(filenamel)-13]='\0';
	   sprintf(rootfile,"%s.4up%d_part%d.root",filebase,rb,kk);
	//        strcat(rootfile,".panel.root");

        cout << " Opening file " << rootfile << " for writing " << endl;
        TFile *calfile = new TFile(rootfile,"RECREATE");
        TTree *panel = new TTree("panel","Sorted Panel Data") ;
        Long64_t lasteventtime=0;
        panel->SetDirectory(0); 

        panel->Branch("Sorted data",&event);

	//        panel->Branch("event",&event.ct,"ct/L:ft/D:E/D:Ec/D:Ech/D:x/D:y/D:chip/I:m/I:apd/I:id/I",2);
	//        panel->Branch("event",&event.ct,"ct/L:ft/D:E/D:Ec/D:Ech/D:x/D:y/D:chip/I:m/I:apd/I:id/I");
	/*
        panel->Branch("ct",&event.ct,"ct/L");
        panel->Branch("ft",&event.ft,"ft/F");
        panel->Branch("E",&event.E,"E/F");
	panel->Branch("Ec",&event.Ec,"Ec/F");
        panel->Branch("Ech",&event.Ech,"Ech/F");
        panel->Branch("x",&event.x,"x/F");
        panel->Branch("y",&event.y,"y/F");
        panel->Branch("chip",&event.chip,"chip/S");
        panel->Branch("fin",&event.fin,"fin/S");
        panel->Branch("m",&event.m,"m/S");
        panel->Branch("apd",&event.apd,"apd/S");
        panel->Branch("id",&event.id,"id/S");
        panel->Branch("pos",&event.pos,"pos/I");
	*/
	/*
        panel->Branch("ct",&event.ct,"ct/L");
        panel->Branch("ft",&event.ft,"ft/D");
        panel->Branch("E",&event.E,"E/D");
	panel->Branch("Ec",&event.Ec,"Ec/D");
        panel->Branch("Ech",&event.Ech,"Ech/D");
        panel->Branch("x",&event.x,"x/D");
        panel->Branch("y",&event.y,"y/D");
        panel->Branch("chip",&event.chip,"chip/I");
        panel->Branch("fin",&event.fin,"fin/I");
        panel->Branch("m",&event.m,"m/I");
        panel->Branch("apd",&event.apd,"apd/I");
        panel->Branch("id",&event.id,"id/I");
        panel->Branch("pos",&event.pos,"pos/I");
	*/

	// this could be simple: we just split the file according to the timestamp 
	// check whether timestamp[unsrt_evt.chip]  

	//          for (m=0;m<RENACHIPS;m++){ 
	//  if (skipchip[m]) continue;
	//            if (verbose) cout << " Processing chip " << m << endl;
	thischiptime=0; l=curevent; curtime=0;  stop=0;extra=0;
	    // setting cutime to 0 here ensures we don't skip the next while loop 
	    //            while (thischiptime<timestep) {           
	    //            while ((curtime<(timestep*kk))&&(stop)) {           
            while (!(stop)){
	    //            while ((curtime<(timecut[kk]))&&(stop)) {           
	      cal->GetEntry(l);
	      //              thischiptime=UL[m*4+2].ct ;
              curtime=unsrt_evt->ct ;
	      if (( curtime+1e12 ) < lasteventtime ) { 
                // rollover occured !! 
		// FIXME :: only one rollover supported as of now. 
		nrrollovers=1;}
              if (nrrollovers) { 
		curtime+=nrrollovers*ROLLOVERTIME;}

	      //              for ( j=0;j<4;j++) {
	      //                if (UL[m*4+j].apd >=0 ) {
                  if (verbose) cout << " thischiptime = " << curtime ; //<< endl;
		  //                  if (UL[m*4+j].apd == 0 ) { event.ft=UL[m*4+j].ft1 ;}// event.Ec=UL[m*4}
		  //                  else { event.ft=UL[m*4+j].ft2 ;}
		  //                    event.ct=unsrt_evt.ct;
        	    event->ct=curtime;
		    event->Ecal=unsrt_evt->Ecal;
                    event->ft=unsrt_evt->ft;
		    event->Ec=unsrt_evt->Ec;
                    event->E=unsrt_evt->E;
		    event->Ech=unsrt_evt->Ech;
		    event->x=unsrt_evt->x;
		    event->y=unsrt_evt->y;
                    /* assigning chip number = from 0 - > 32 for each cartridge so 0->7; 8->15; 16->23; 24->32; 
                       the chip number will be more for debugging */
                    event->chip=unsrt_evt->chip+(rb)*8;
		    //                    event.m=m*4+j+(rb)*32;
                    /* Assigning fin number */
                    m=unsrt_evt->chip;
                    if (left==1) { if (rb<2) finincr=0; else finincr=1;} 
                    else { if (rb<2) finincr=0; else finincr=1;}
                    if (m<2) { event->fin = 6+finincr;} 
                    else { if (m<4) { event->fin = 4+finincr;}
		      else { if (m<6) { event->fin = 2+finincr;}
			else { event->fin = finincr; }}}
		    /* Assigning module number */ 
                    event->m=unsrt_evt->m +(unsrt_evt->chip%2)*4+(rb%2)*8;
		    /* note :: we can plot using  event.m+(event.fin)*16; */
                    event->apd=unsrt_evt->apd;
                    event->id=unsrt_evt->id;
                    event->pos=unsrt_evt->pos;
                    if (kk > 0 ) mintime=timecut[kk-1];
                    else mintime=0;
                    if ( (curtime<=timecut[kk])&&curtime>mintime )  { panel->Fill();}
                    else {
		      // we're going to need to go back a number of steps
                      extra++;
		      if (( curtime-timecut[kk])>10000 ) { stop=1;} }   
		     // we stop the while loop and close the file if the timedifference is larger than 10

    	      if (verbose) {cout << " filling chip " << unsrt_evt->chip << " UNIT " << event->m ;
	      cout << " (l="<<l<<")"<< endl;}
		    //	    } // if mod >= 0
	    //   } // loop over 4 modules in chip
	      l++;
              if (l>=entries) stop=1;
     //              if (verbose) cout << " Extra entries :: " << extra << "; l = " << l << endl;
	      //              l-=extra;
              if (l<0 ) l=0; // safety
	      //              if ( l >= entries[m] ) { skipchip[m]=1; stop=0; continue;}
              if ((l>0)&&((l%5000000)==0)) cout << " Processed " << l/10e6 << " million events " << endl;
              lasteventtime=curtime;  
			 	 }  // while !stop

             if (verbose) cout << " Extra entries :: " << extra << "; l = " << l << endl;
	       l-=extra;

	    curevent=l;
            if (verbose) cout << " Curevent[" << m << "] = " << curevent << "; curtime = " << curtime << endl;
//	  } // loop m
	    kk++;
	  //          if (kk >10 ) break;

	    // Fill tree with first 10% of events, then sort. 
           
      
	  //	  cout << " time after filling tree :: "  << endl;
        
	    time_before_sorting=clock();
	  // sorting

	    //	    panel->BuildIndex("event.ct >> 32","(event.ct & 0xFFFFFFFF)");
	    //            TTreeIndex *index = (TTreeIndex *) panel->GetTreeIndex();
	    // for (Int_t t=0;t<index->GetN(); t++){ p->GetEntry(index->GetIndex()[t]); cout << evt.ct << endl;}
            
	    //	    Long64_t index;
            Long64_t N = panel->GetEntries();

             Long64_t *table = new Long64_t[N];
             Long64_t *sorted = new Long64_t[N];

	    //  std::cout<<"Reading Run numbers"<<std::endl;
            for (l=0;l<N;l++)     {
                panel->GetEntry(l);
                table[l] = event->ct;
		//             cout << " event->ct = " << event->ct <<  endl;
	    } // loop over l 

	    std::cout<<"Sorting Run numbers :: " ; //<<std::endl;

           TMath::Sort(N,table,sorted,kFALSE);

          time_after_sorting=clock();
        

          printf("Time to merge  8 chips (sec): %f",(double)(time_before_sorting-time_before_filling)/(CLOCKS_PER_SEC));
          printf("Time to sort the merged chips (sec): %f\n",(double)(time_after_sorting-time_before_sorting)/(CLOCKS_PER_SEC));

	  if (verbose) cout << N << " entries in tree; cloning tree :: " << endl;

	//  TTree *fourup = panel->CloneTree(0); 
	//  cout << " Cloning Successfull !! " << endl;
	//  fourup->SetName("fourup");

        TTree *fourup = new TTree("fourup","Time Sorted data one 4-up board");
        fourup->Branch("Time Sorted Data",&evt);
	/*
        fourup->Branch("ct",&evt.ct,"ct/L");
        fourup->Branch("ft",&evt.ft,"ft/D");
         fourup->Branch("E",&evt.E,"E/D");
	fourup->Branch("Ec",&evt.Ec,"Ec/D");
         fourup->Branch("Ech",&evt.Ech,"Ech/D");
         fourup->Branch("x",&evt.x,"x/D");
         fourup->Branch("y",&evt.y,"y/D");
         fourup->Branch("chip",&evt.chip,"chip/I");
         fourup->Branch("fin",&evt.fin,"fin/I");
         fourup->Branch("m",&evt.m,"m/I");
         fourup->Branch("apd",&evt.apd,"apd/I");
        fourup->Branch("id",&evt.id,"id/I");
       fourup->Branch("pos",&evt.pos,"pos/I");
	*/

        if (verbose)	  cout << " Loading Baskets ... " << endl;
  panel->LoadBaskets(2328673567232LL);
	  thistime=clock();  

       for (l=0;l<N;l++){
	 if ((((l)%5000000)==0)&&(l>0))  { cout << l << " entries processed; " ;
	      cout << " time spent = " <<(double)  (clock()-thistime)/CLOCKS_PER_SEC ;
              cout << "; since start: " <<(double) (clock()-time_after_sorting)/CLOCKS_PER_SEC <<endl; 
   	    thistime = clock(); }
            
            panel->GetEntry(sorted[l]);
	    //     panel->GetEntry(l);
	     //  cout << " event->ct = " << event->ct <<  " ( sorted[l] = " << sorted[l] << " )" << endl;
 	     evt=event;
             fourup->Fill();
          }
 
          time_after_filling=clock();
 
          printf("Time to clone the Tree (sec): %f\n",(double)(time_after_filling-time_after_sorting)/(CLOCKS_PER_SEC));

	 	  	 	fourup->Write();
	//        panel->Write();
          calfile->Close();

          delete panel;
          delete calfile;

          if (kk >= ncuts  ) break;
      } // while loop 
	

       return 0;}

