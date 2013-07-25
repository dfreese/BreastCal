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
//#include "Apd_Fit.h"
//#include "Apd_Peaktovalley.h"
//#include "/Users/arne/root/macros/myrootlib.h"
//#include "/home/miil/root/libInit_avdb.h"
#include "./decoder.h"
#include "./ModuleCal.h"
#include "time.h"
#include <sstream>

int main(int argc, Char_t *argv[])
{
  	cout << "\nWelcome to merge_panel, merging data from 4 4-up boards" << endl;

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	Char_t		filenamel[FILENAMELENGTH] = "";
	Int_t		verbose = 0;
	Int_t		ix,nb;
        Int_t           partnumber = -99;
        nb=0;

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

     

	for(ix = 1; ix < argc; ix++) {

		if(strncmp(argv[ix], "-h", 2) == 0) {
			cout << " Usage:  " << endl;
                        cout << " ./merge_panel -f [Filename] -rb [renaboard] [-v] " << endl;
                        cout << " Renaboard is either 1,2,3 or 4." << endl;
			return -1;
		}


		/*
		 * Verbose '-v'
		 */
		if(strncmp(argv[ix], "-v", 2) == 0) {
			cout << "Verbose Mode " << endl;
			verbose = 1;
		}

		if(strncmp(argv[ix], "-nb", 3) == 0) {
			nb = atoi(argv[ix+1]);
			if (verbose) cout << nb << " 4up boards " << endl;
                        ix++;
		}

		if(strncmp(argv[ix], "-pn", 3) == 0) {
			partnumber = atoi(argv[ix+1]);
			cout << "Partnumber : " << partnumber << endl;
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
		

	}

	if ((nb<1)||(nb>4)) {
	    cout << " Please specify a valid number of renaboards  ( -nb [number 4ups] ). Options are 1,2,3 or 4. Nothing else.\nExiting." ;
            return -1;
	  }           

             TCanvas *c1;
             c1 = (TCanvas*)gROOT->GetListOfCanvases()->FindObject("c1");
             if (!c1) c1 = new TCanvas("c1","c1",10,10,1000,1000);


        rootlogon(verbose);


        Char_t filebase[FILENAMELENGTH],rootfile[FILENAMELENGTH]; 
        
        Int_t i,m;
        ifstream infile;

        string _filenamel(filenamel) ;
        string _filebase,_filedir;

	if (verbose) cout << "File name read : " << _filenamel << endl;
        int pos1=_filenamel.find(".4up");
        int pos2=_filenamel.find(".root");
        int pos3=_filenamel.find("./L");
        unsigned int panpos=pos2-pos1+1+5;   
	if (verbose) cout << " p1: " << pos1 << ", p2: " << pos2 << ", p3: " << pos3 << endl;
        if ( panpos > strlen(filenamel) ) {
	  cout << " Check input file name ! " << endl;
          return -12; }
        strncpy(filebase,filenamel,strlen(filenamel)-panpos);
        filebase[strlen(filenamel)-panpos]='\0';
        if (verbose){
        cout << " filebase :: " << filebase << endl;
        cout << " length of partnumber :: " << pos2-pos1-10 << endl;}

	// if (pos3)  _filebase = _filenamel.substr(pos3+2,strlen(filenamel)-panpos-2
        _filebase=_filenamel.substr(0,pos1-2);
        _filedir=_filenamel.substr(pos1-2,1);

	if(verbose){	cout << " _filebase = " << _filebase << endl;       }

        int part=atoi( (_filenamel.substr(pos1+10,pos2-pos1-10)).c_str());
        if (partnumber==-99){
        strncpy(filebase,filenamel,strlen(filenamel)-11);
        filebase[strlen(filenamel)-11]='\0';
	}
        else {
        strncpy(filebase,filenamel,strlen(filenamel)-17);
        filebase[strlen(filenamel)-17]='\0';
	}
        if (verbose){
	cout << " filename = " << filenamel << endl;
	cout << " filebase = " << filebase << endl;}
        char * pch = strstr (filenamel,"4up");
        char * pch2 = strstr (pch,"part");
	if (verbose){  cout << " pch = " << pch <<endl; 
	cout << " pch2 = " << pch2 <<endl; }
         char cset[] = "1234567890";
	 //char numstr[21];
         i = strspn (pch2,cset);
	   //           strncpy (pch,"",6);
	 //        int part= atoi ( &filenamel[strlen(filenamel)-6] );
         if (verbose) {cout << " part = " << part <<  endl;
	   cout << filenamel[strlen(filenamel)-6] << endl;}
	/* 
       this is a trial .. making searching more better
          
	*/
	//	    return 0;

	TTree *panel[FOURUPBOARDS];
        ModuleCal *event[FOURUPBOARDS];
        ModuleCal *evt = new ModuleCal();
        ModuleCal *evnt = new ModuleCal();

	for (i=0;i<FOURUPBOARDS;i++){
	  event[i] = new ModuleCal();}


        Long64_t entries[FOURUPBOARDS];
		stringstream fnam;

        for ( i=0;i<(nb);i++){
	  fnam.str("");
	  fnam << "./" << _filedir << i << "/" << _filebase  << _filedir << i << ".4up" << i << "_part" << part << ".root";
	      _filenamel=fnam.str();
             
	  //	  _filenamel =  _filebase  + itoa(nb,numstr,10) + ".4up"  + itoa(nb,numstr,10) + "_part" + itoa(part,numstr,10) + ".root";
       
	      //   cout << " file to open : _filenamel : " << _filenamel << endl;
	  if (partnumber==-99){ sprintf(filenamel,"%s%d.4up%d.root",filebase,i,i);}
	  else {	  sprintf(filenamel,"%s%d.4up%d_part%d.root",filebase,i,i,partnumber); }
        cout << " Opening file " << _filenamel << endl;
        TFile *file_left = new TFile(_filenamel.c_str(),"OPEN");
	//  
	//	continue;
        if (!file_left || file_left->IsZombie()) { 
	   cout << "problems opening file " << _filenamel ;
	   //     cout << "\n Exiting " << endl; 
           entries[i]=0;
           continue; } // return -11;}


	//	return -99;

	if (verbose){
	cout << " File Content : " << endl;
        file_left->ls();}
	//	} return 0;}

        panel[i]= (TTree *)file_left->Get("fourup");
	panel[i]->SetBranchAddress("Time Sorted Data",&event[i]);
	/*
	panel[i]->SetBranchAddress("ct",&event[i].ct);
        panel[i]->SetBranchAddress("ft",&event[i].ft);
        panel[i]->SetBranchAddress("E",&event[i].E);
	panel[i]->SetBranchAddress("Ec",&event[i].Ec);
        panel[i]->SetBranchAddress("Ech",&event[i].Ech);
        panel[i]->SetBranchAddress("x",&event[i].x);
        panel[i]->SetBranchAddress("y",&event[i].y);
        panel[i]->SetBranchAddress("chip",&event[i].chip);
        panel[i]->SetBranchAddress("fin",&event[i].fin);
        panel[i]->SetBranchAddress("m",&event[i].m);
        panel[i]->SetBranchAddress("apd",&event[i].apd);
        panel[i]->SetBranchAddress("id",&event[i].id);
        panel[i]->SetBranchAddress("pos",&event[i].pos);
	*/
        if (!(panel[i]))    {
	    cout << " Problem reading Tree panel[" << i << "] from file." << endl;
            continue;
	  }

        entries[i]=panel[i]->GetEntries();
        cout << " Entries Tree " << i << ": " << entries[i] << endl; 

        } // loop over i


       
        sprintf(rootfile,"%spart%d_%s.panel.root",_filebase.c_str(),part,_filedir.c_str());

        cout << " Opening file " << rootfile << " for writing " << endl;
        TFile *calfile = new TFile(rootfile,"RECREATE");
        TTree *cartr = new TTree("cartr","Cartridge Data") ;
        cartr->SetDirectory(0); 
	//        cartr->Branch("event",&evt.ct,"ct/L:ft/D:E/D:Ec/D:Ech/D:x/D:y/D:chip/I:m/I:apd/I:id/I");
        cartr->Branch("TMP CalData",&evt);
	/*
        cartr->Branch("ct",&evt.ct,"ct/L");
        cartr->Branch("ft",&evt.ft,"ft/D");
        cartr->Branch("E",&evt.E,"E/D");
	cartr->Branch("Ec",&evt.Ec,"Ec/D");
        cartr->Branch("Ech",&evt.Ech,"Ech/D");
        cartr->Branch("x",&evt.x,"x/D");
        cartr->Branch("y",&evt.y,"y/D");
        cartr->Branch("chip",&evt.chip,"chip/I");
        cartr->Branch("m",&evt.m,"m/I");
        cartr->Branch("fin",&evt.fin,"m/I");
        cartr->Branch("apd",&evt.apd,"apd/I");
        cartr->Branch("id",&evt.id,"id/I");
        cartr->Branch("pos",&evt.pos,"pos/I");

        */
   

        Long64_t lasttime=0;

        Long64_t lasttimes[FOURUPBOARDS];
        Int_t skipfourup[FOURUPBOARDS];
        Int_t curevent[FOURUPBOARDS];

        for (m=0;m<FOURUPBOARDS;m++){ skipfourup[m]=1;}

        for (m=0;m<nb;m++){
        if(entries[m]) {
	  panel[m]->GetEntry(entries[m]-1);
          cout << " Last Timestamp Panel " << m  << ": " << event[m]->ct << endl;
          lasttimes[m]=event[m]->ct;
          if ( lasttime <  event[m]->ct ) lasttime= event[m]->ct;
	  skipfourup[m]=0;
          curevent[m]=0;
         }
        else skipfourup[m]=1;
}
 
        cout << " Last timestamp = " << lasttime << endl;
	time_t time_before_filling, time_before_sorting, time_after_sorting, time_after_filling,thistime; 
        Long64_t l,curtime=0;
       
	Long64_t timestep=48e6;
        Long64_t this4uptime;
 	//        lasttime=1e10;
	timestep=TMath::Floor(lasttime/10); //1e10;
	//	            timestep=TMath::Floor(lasttime/1000); //1e10;
 
	  cout << " Timestep = " << timestep << endl;
  
	  Int_t kk=1,stop;
           Int_t skipsum=0;
	  for (m=0;m<nb;m++){ if (skipfourup[m]) skipsum++;}
          time_before_filling = clock();

       //        while (curtime<lasttimes[6]) {
	  //	  	  while ((curtime<lasttime)&&(kk<2)) {
	 	  while ((curtime<lasttime)&&(skipsum!=4)) {
		    //   while ((curtime<lasttime)) {
		    if (verbose)    cout << " Looping kk = " << kk << endl;
          for (m=0;m<nb;m++){ 
            if (skipfourup[m]) continue;
	    if (verbose)   cout << " Processing 4 up board " << m << endl;
            this4uptime=0; l=curevent[m]; curtime=0;  stop=1;
	    // setting cutime to 0 here ensures we don't skip the next while loop 
	    //            while (thischiptime<timestep) {           
            while ((curtime<(timestep*kk))&&(stop)) {           
	      panel[m]->GetEntry(l);
	      //              thischiptime=UL[m*4+2].ct ;
               curtime=event[m]->ct ;
	       evt=event[m];
               cartr->Fill();
	      l++;
              if ( l >= entries[m] ) {  skipsum++; skipfourup[m]=1; stop=0; continue;}
              if (verbose) if ((l>0)&&((l%250000)==0)) cout << " Processed " << l << " events " << " chip " << m << endl;
	    } // while curtime< timestep
	    curevent[m]=l;
              if (verbose) cout << " Curevent[" << m << "] = " << curevent[m] << "; curtime = " << curtime << endl;
	  } // loop m
	  kk++;
          }


	  //	  cout << " time after filling tree :: "  << endl;
        
	    time_before_sorting=clock();
	  // sorting

	    //	    panel->BuildIndex("event.ct >> 32","(event.ct & 0xFFFFFFFF)");
	    //            TTreeIndex *index = (TTreeIndex *) panel->GetTreeIndex();
	    // for (Int_t t=0;t<index->GetN(); t++){ p->GetEntry(index->GetIndex()[t]); cout << evt.ct << endl;}
            
	    //             Long64_t index;
             Long64_t N = cartr->GetEntries();

             Long64_t *table = new Long64_t[N];
             Long64_t *sorted = new Long64_t[N];
          
	    //  std::cout<<"Reading Run numbers"<<std::endl;
            for (l=0;l<N;l++)     {
                cartr->GetEntry(l);
                table[l] = evt->ct;
		//             cout << " event->ct = " << event->ct <<  endl;
  }

           std::cout<<"Sorting Run numbers"<<std::endl;

           TMath::Sort(N,table,sorted,kFALSE);

          time_after_sorting=clock();
        

          printf("Time to merge  8 chips (sec): %f\n",(double)(time_before_sorting-time_before_filling)/(CLOCKS_PER_SEC));
          printf("Time to sort the merged chips (sec): %f\n",(double)(time_after_sorting-time_before_sorting)/(CLOCKS_PER_SEC));

	  cout << N << " entries in tree " << endl;

	  //          TTree *cartridge = cartr->CloneTree(0);
	  //          cartridge->SetName("cartridge");
	  //          cartridge->SetTitle("Sorted Cartridge Data");
	     TTree *cartridge = new TTree("cartridge","Time Sorted Cartridge data");

        cartridge->Branch("CalData",&evnt);
	/*
        cartridge->Branch("ct",&evnt.ct,"ct/L");
        cartridge->Branch("ft",&evnt.ft,"ft/D");
        cartridge->Branch("E",&evnt.E,"E/D");
	cartridge->Branch("Ec",&evnt.Ec,"Ec/D");
        cartridge->Branch("Ech",&evnt.Ech,"Ech/D");
        cartridge->Branch("x",&evnt.x,"x/D");
        cartridge->Branch("y",&evnt.y,"y/D");
        cartridge->Branch("chip",&evnt.chip,"chip/I");
        cartridge->Branch("fin",&evnt.fin,"fin/I");
        cartridge->Branch("m",&evnt.m,"m/I");
        cartridge->Branch("apd",&evnt.apd,"apd/I");
        cartridge->Branch("id",&evnt.id,"id/I");
        cartridge->Branch("pos",&evnt.pos,"pos/I");
	*/

	  //   fourup->Branch("evt",&evt.ct,"ct/L:ft/D:E/D:Ec/D:Ech/D:x/D:y/D:chip/I:m/I:apd/I:id/I");
 

 //loop branch by branch on the input Tree and fill the output Tree
	  /*
  TIter next(panel->GetListOfBranches());
  TBranch *bin, *bout;
  while ((bin = (TBranch*)next())) {
    bout = fourup->GetBranch(bin->GetName());
    printf("processing branch: %s\n",bin->GetName());
    //load all baskets of this branch in memory (for performance)
    bin->LoadBaskets();
    //loop on entries and fill output Tree
    for (index=0;index<N;index++) {
      bin->GetEntry(sorted[index]);
      bout->Fill();
    }
    bin->DropBaskets();
    fourup->AutoSave();
  }
	  */
         cartr->LoadBaskets(2328673567232LL);
	  thistime=clock();  

       for (l=0;l<N;l++){
	 if ((((l)%5000000)==0)&&(l>0))  { cout << l << " entries processed ";// << endl;
	      cout << "; time spent = " <<(double)  (clock()-thistime)/CLOCKS_PER_SEC ;
              cout << "; since start: " <<(double) (clock()-time_after_sorting)/CLOCKS_PER_SEC <<endl; 
   	    thistime = clock(); }
            
	         cartr->GetEntry(sorted[l]);
	    //     panel->GetEntry(l);
	     //  cout << " event.ct = " << event.ct <<  " ( sorted[l] = " << sorted[l] << " )" << endl;
		 //	     evt=event;
		 evnt=evt;
             cartridge->Fill();
          }
 
       cout << cartridge->GetEntries() << " Entries in the new tree. " << endl;

          time_after_filling=clock();
 
          printf("Time to clone the Tree (sec): %f\n",(double)(time_after_filling-time_after_sorting)/(CLOCKS_PER_SEC));

	  	cartridge->Write();
	//        panel->Write();
  calfile->Close();

	

       return 0;}

