
#include "TROOT.h"
#include "Riostream.h"
#include "TTree.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TSpectrum.h"
#include "TSpectrum2.h"
#include "TH2F.h"
#include "TVector.h"
#include "TMath.h"
#include "TF1.h"
#include "Apd_Fit.h"
#include "./decoder.h"
#include "./ModuleCal.h"
#include "./CoincEvent.h"

//void usage(void);
Int_t  getmintime(Double_t a,Double_t b,Double_t c, Double_t d);
//void usage(void){
// cout << " mergecal -fl [filename] -fr [filename] [-t [threshold] -v ]" <<endl;
//  return;}

int main(int argc, Char_t *argv[])
{
  cout << "Welcome to Merge_coinc."; 

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	Char_t		filenamel[FILENAMELENGTH] = "";
	Char_t		filenamer[FILENAMELENGTH] = "";
	Int_t		verbose = 0, threshold=-1000;
	Int_t		ix,ascii;
	//module UNIT0,UNIT1,UNIT2,UNIT3;
        CoincEvent       *evt = new CoincEvent();

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
		if(strncmp(argv[ix], "-fl", 3) == 0) {
			if(strlen(argv[ix + 1]) < FILENAMELENGTH) {
				sprintf(filenamel, "%s", argv[ix + 1]);
			}
			else {
				cout << "Filename " << argv[ix + 1] << " too long !" << endl;
				cout << "Exiting.." << endl;
				return -99;
			}
		}

		if(strncmp(argv[ix], "-fr", 3) == 0) {
			if(strlen(argv[ix + 1]) < FILENAMELENGTH) {
				sprintf(filenamer, "%s", argv[ix + 1]);
			}
			else {
				cout << "Filename " << argv[ix + 1] << " too long !" << endl;
				cout << "Exiting.." << endl;
				return -99;
			}
		}

	}

        if (verbose) cout << endl;
        rootlogon(verbose);



        Char_t filebase[FILENAMELENGTH],rootfile[FILENAMELENGTH]; 
        Char_t asciifile[FILENAMELENGTH]; 
	//       Char_t tmpname[20],tmptitle[50];
	//        Int_t i,j,k,m,lines;
	//        Double_t aa, bb;
	//        Int_t augment;
        ifstream infile;
        Int_t evts=0;
	//        strncpy(filebase,filename,strlen(filename)-17);
	//        filebase[strlen(filename)-17]='\0';
	//        sprintf(rootfile,"%s",filename);
	//        strcat(rootfile,".root");
              
	//	cout << "Rootfile to open :: " << rootfile << endl;


        if (verbose) cout << " Opening file " << filenamel << endl;
        TFile *file_left = new TFile(filenamel,"OPEN");
        if (!file_left || file_left->IsZombie()) {  cout << "problems opening file " << filenamel << "\n.Exiting" << endl; return -11;} 
        TTree *car_l = (TTree *) file_left->Get("cartridge");
        ModuleCal *EL=0;
        ModuleCal *ER=0;

	//        car_l->SetBranchAddress("event",&EL);
	//        car_l->SetBranchAddress("event",&EL);
        car_l->SetBranchAddress("CalData",&EL);
	/*
	car_l->SetBranchAddress("ct",&EL.ct);
        car_l->SetBranchAddress("ft",&EL.ft);
          car_l->SetBranchAddress("E",&EL.E);
	car_l->SetBranchAddress("Ec",&EL.Ec);
        car_l->SetBranchAddress("Ech",&EL.Ech);
        car_l->SetBranchAddress("x",&EL.x);
        car_l->SetBranchAddress("y",&EL.y);
        car_l->SetBranchAddress("chip",&EL.chip);
        car_l->SetBranchAddress("fin",&EL.fin);
        car_l->SetBranchAddress("m",&EL.m);
        car_l->SetBranchAddress("apd",&EL.apd);
        car_l->SetBranchAddress("id",&EL.id);
        car_l->SetBranchAddress("pos",&EL.pos);

	*/

        if (verbose) cout << " Opening file " << filenamer <<endl;
        TFile *file_right = new TFile(filenamer,"OPEN");
        if (!file_right || file_right->IsZombie()) {  cout << "problems opening file " << filenamel << "\n.Exiting" << endl; return -11;} 
        TTree *car_r = (TTree *) file_right->Get("cartridge");
	ofstream asciiout;

	//        car_r->SetBranchAddress("event",&ER);
	//        car_r->SetBranchAddress("event",&ER);
        car_r->SetBranchAddress("CalData",&ER);
	/*
	car_r->SetBranchAddress("ct",&ER.ct);
        car_r->SetBranchAddress("ft",&ER.ft);
          car_r->SetBranchAddress("E",&ER.E);
	car_r->SetBranchAddress("Ec",&ER.Ec);
        car_r->SetBranchAddress("Ech",&ER.Ech);
        car_r->SetBranchAddress("x",&ER.x);
        car_r->SetBranchAddress("y",&ER.y);
        car_r->SetBranchAddress("chip",&ER.chip);
        car_r->SetBranchAddress("fin",&ER.fin);
        car_r->SetBranchAddress("m",&ER.m);
        car_r->SetBranchAddress("apd",&ER.apd);
        car_r->SetBranchAddress("id",&ER.id);
        car_r->SetBranchAddress("pos",&ER.pos);
	*/

	  // Open Calfile //
        strncpy(filebase,filenamel,strlen(filenamel)-13);
        filebase[strlen(filenamel)-13]='\0';
        sprintf(rootfile,"%s",filebase);
        strcat(rootfile,".merged.root");
        if (ascii){
	  sprintf(asciifile,"%s.merged.ascii",filebase);
          asciiout.open(asciifile);
	    }

  

        cout << " Opening file " << rootfile << " for writing " << endl;
        TFile *calfile = new TFile(rootfile,"RECREATE");
          // Create Tree //

       TTree *merged = new  TTree("merged","Merged and Calibrated LYSO-PSAPD data ");
       //       merged->Branch("event",&evt.dtc,"dtc/L:dtf/D:chip1/I:fin1/I:m1/I:apd1/I:crystal1/I:E1/D:Ec1/D:Ech1/D:ft1/D:chip2/I:fin2/I:m2/I:apd2/I:crystal2/I:E2/D:Ec2/D:Ech2/D:ft2/D:x1/D:y1/D:x2/D:y2/D:pos/I");
       //       merged->Branch("event",&evt.dtc,"dtc/L:dtf/D:E1/D:Ec1/D:Ech1/D:ft1/D:E2/D:Ec2/D:Ech2/D:ft2/D:x1/D:y1/D:x2/D:y2/D:chip1/I:fin1/I:m1/I:apd1/I:crystal1/I:chip2/I:fin2/I:m2/I:apd2/I:crystal2/I:pos/I");
       merged->Branch("Event",&evt);

       Long64_t entries_car_r = car_r->GetEntries();
       Long64_t entries_car_l = car_l->GetEntries();

       if (verbose){
       cout << " Right entries: " << entries_car_r  << endl;
       cout << " Left  entries: " << entries_car_l  << endl;
       }



       //       Int_t l1,r1;

       //       Double_t leftfine,rightfine;

#define COARSEDIFF 10       

       Int_t l=0;
       Int_t car_ri=0;
       Int_t car_li=0;

       Long64_t lefttime=0;
       Long64_t righttime=0;

       Long64_t prev_lefttime=0;
       Long64_t prev_righttime=0;
       //       Long64_t prev_righttime_2=0;

       Int_t skipevt=0;
       Int_t endlessloop=0;
       Int_t dontaugmentmask=0;


       cout << rootfile ;

       while (1){

#ifdef DEBUG2
       cout << "\n Getting Tree Entries :: " << endl;
       cout << " This event: dontaugmentmask = " << dontaugmentmask << endl;
#endif

       car_r->GetEntry(car_ri);
       car_l->GetEntry(car_li);


       if ( ( (double) car_ri / entries_car_r ) >  l*0.25 ) { 
       if ( ( (double) car_li / entries_car_l ) >  l*0.25 ) { 
	 cout <<  " " << l*25 << " % done .. ;  "; l++;}}


#ifdef DEBUG2
       cout << " Prev Left  times: " << setprecision(12) << lefttime << endl ;
       cout << " Prev Right times: " << righttime << endl ;
#endif
  
       if ((!(dontaugmentmask&=0x1))&&(!(skipevt))) prev_lefttime=lefttime; 
       //       if ((!(dontaugmentmask&=0x2))&&(!(skipevt))) prev_lefttime_2=lefttime_2; 
       if ((!(dontaugmentmask&=0x4))&&(!(skipevt))) prev_righttime=righttime;
       //       if ((!(dontaugmentmask&=0x8))&&(!(skipevt))) prev_righttime_2=righttime_2;


       lefttime=EL->ct;
       righttime=ER->ct;

#ifdef DEBUG2
       cout << endl;
       cout << " This event::  left: " << car_li ;
       cout <<             ", right: " << car_ri << endl ;
#endif



#ifdef DEBUG2
              cout << " This event Left  times: " << setprecision(12) << lefttime <<  endl;
              cout << " This event Right times: " << righttime <<  endl;
#endif

       if (((prev_lefttime > lefttime)||(car_li>=entries_car_l))&&(dontaugmentmask!=0xf)) { dontaugmentmask|=0x1; } 
       else { dontaugmentmask&=(0xE); }
       //       if (((prev_lefttime_2 > lefttime_2)||(ch2_li>=entries_ch2_l))&&(dontaugmentmask!=0xf)) { dontaugmentmask|=0x2; } 
       //       else { dontaugmentmask&=(0xD); }
       if (((prev_righttime > righttime)||(car_ri>=entries_car_r))&&(dontaugmentmask!=0xf)) { dontaugmentmask|=0x4; } 
       else { dontaugmentmask&=(0xB); }
       //       if (((prev_righttime_2 > righttime_2)||(ch2_ri>=entries_ch2_r))&&(dontaugmentmask!=0xf)) { dontaugmentmask|=0x8; } 
       //       else { dontaugmentmask&=(0x7); }

#ifdef DEBUG2
      cout << "\t Assigning dontaugmentmask = " << dontaugmentmask << endl;
#endif

       if ( dontaugmentmask&0x1) lefttime+=INFTY;
       //       if ( dontaugmentmask&0x2) lefttime_2+=INFINITY;
       if ( dontaugmentmask&0x4) righttime+=INFTY;
       //       if ( dontaugmentmask&0x8) righttime_2+=INFINITY;

#ifdef DEBUG2
       cout << " Left  times: " << setprecision(12) << lefttime << endl ; // _1 << " and " << lefttime_2 << endl;
       cout << " Right times: " << righttime << endl ; //" and " << righttime_2 << endl;
#endif

       //       break;
       //       if (lefttime_1  < lefttime_2 ) { l1=0; lefttime=lefttime_1;  } else {l1=1;lefttime=lefttime_2; }
       //       if (righttime_1 < righttime_2) { r1=0;righttime=righttime_1; } else {r1=1;righttime=righttime_2;}

#ifdef DEBUG2
              cout << " Left  time: " << setprecision(12) << lefttime ;
              cout << " Right time: " << righttime  << endl;
#endif

     

       if ((TMath::Abs(lefttime-righttime) < COARSEDIFF ) && (!(skipevt))){
	 evt->dtc= lefttime-righttime;
         evt->dtf= EL->ft-ER->ft;
#ifdef DEBUG2
           cout << " ::::  COARSE COINCIDENCE :::: Delta T = " << evt->dtc << endl;
#endif
               evt->chip1=EL->chip;
               evt->fin1=EL->fin;
               evt->m1=EL->m;
               evt->apd1=EL->apd;
               evt->crystal1=EL->id;
               evt->E1=EL->Ecal;
               evt->Ec1=EL->Ec;
               evt->Ech1=EL->Ech;
               evt->ft1=EL->ft;
               evt->x1=EL->x;
               evt->y1=EL->y;
               evt->chip2=ER->chip;
               evt->fin2=ER->fin;
               evt->m2=ER->m;
               evt->apd2=ER->apd;
               evt->crystal2=ER->id;
               evt->E2=ER->Ecal;
               evt->Ec2=ER->Ec;
               evt->Ech2=ER->Ech;
               evt->ft2=ER->ft;
               evt->x2=ER->x;
               evt->y2=ER->y;

               evt->pos = ER->pos;

           if (evt->dtf < -TMath::Pi() ) evt->dtf+=2*TMath::Pi();
           if (evt->dtf >  TMath::Pi() ) evt->dtf-=2*TMath::Pi();
           evt->dtf*=1e9;
           evt->dtf/=(2*TMath::Pi()*UVFREQUENCY);
	   //	   cout << "Filling Tree : (rightfine=" << rightfine << ",leftfine=" << leftfine << ")" << endl;
           if  (((EL->ft>-1)&&(ER->ft>-1))&& (ER->pos == EL->pos ) )  { evts++; merged->Fill();

	     if (ascii){
	       asciiout << evt->dtc << " " <<evt->dtf << " ";
               asciiout << evt->chip1 << " " << evt->m1 << " " << evt->apd1 << " " << evt->crystal1 << " ";
               asciiout << evt->E1 << " " << evt->Ec1 << " " << evt->Ech1 << " " << evt->ft1 << " ";
               asciiout << evt->chip2 << " " << evt->m2 << " " << evt->apd2 << " " << evt->crystal2 << " ";
               asciiout << evt->E2 << " " << evt->Ec2 << " " << evt->Ech2 << " " << evt->ft2 << " ";
               asciiout << evt->x1 << " " << evt->y1  << " " << evt->x2 << "  " << evt->y2 << "  " << evt->pos << "  ";
               asciiout << evt->fin1 << "  " << evt->fin2  << endl;

			   // FLAU edited this to print out position.
			   // FLAU edited this to print out fin number (Dec2,2012)
	     }

#ifdef DEBUG2
	     cout << "  Filling tree :: " << endlessloop << " " << evt->dtc << " " << evt->dtf << " " << evt->chip1 << " " << evt->m1 << " " << evt->apd1;
             cout << " " << evt->crystal1 << " " << evt->E1 << " " << evt->chip2 << " "<<evt->m2 << " "<< evt->apd2 << " ";
             cout << evt->crystal2 << " " <<evt->E2 << endl;
#endif
	   }
           // overlap :: 
	   //   if (l1==0) { 
           if (!(dontaugmentmask&0x1)) car_li++; else skipevt=1; 
       //	   else { if (!(dontaugmentmask&0x2)) ch2_li++; else skipevt=1;}
       //  if (r1==0) { 
           if (!(dontaugmentmask&0x4)) car_ri++; else skipevt=1;
           // else { if (!(dontaugmentmask&0x8)) ch2_ri++; else skipevt=1;} 
       } // COARSE TIME DIFFERENCE
	 else {
#ifdef DEBUG2
           cout << " ::::  NO COINCIDENCE :::: " <<endl;
	 // no overlap :: need to augment only one  
        //determine which one should augment:
         cout << " dontaugmentmask = " << dontaugmentmask << endl;
#endif
	   skipevt=0;
     
     if (dontaugmentmask!= 0xF ){
       if ( lefttime < righttime ) car_li++;
       else car_ri++;}
     //       car_li++;car_ri++; }
     /*
 	 augment = getmintime(lefttime_1+(dontaugmentmask&0x1)*INFINITY,lefttime_2+(dontaugmentmask&0x2)*INFINITY,
			      righttime_1+(dontaugmentmask&0x4)*INFINITY,righttime_2+(dontaugmentmask&0x8)*INFINITY);
#ifdef DEBUG2
         cout << " To augment :: " << augment << endl;
#endif
	 switch (augment){
	 case 1:
           ch1_li++; 
	   break;
        case 2:
  	   ch2_li++;
           break;
        case 3:
  	   ch1_ri++;
           break;
        case 4:
  	   ch2_ri++;
           break;
	 } //switch
	 } // dontaugmentmask !=F  */
         else { dontaugmentmask=0;
	   lefttime=0;
	   //	   lefttime_2=0;
	   righttime=0;
	   //	   righttime_2=0;
	 }

#ifdef DEBUG2
     cout << " After augmentation::  left: " << car_li << " " ; //<< ch2_li;
     cout <<                      ", right: " << car_ri << endl; // " " << ch2_ri << endl;
#endif
	 } // else - no COINCIDENCE FOUND
         endlessloop++;
         if (endlessloop > 200e9) { cout << " Program caught on endless loop\n Exiting. " << endl; return -99; }
	 //	 	 if ( ch1_ri > 20e3) break;  
	 //	 	 if ( ch1_li > 20e3) break;  

	 //  if (((ch1_ri >= entries_ch1_r)&&(ch2_ri>=entries_ch2_r))||((ch1_li >= entries_ch1_l)&&(ch2_li >= entries_ch2_l))){
     if ((car_ri >= entries_car_r)||(car_li >= entries_car_l)){
           break;}



       } // while loop 	

       cout << "Filled tree with " << evts << " events  ( car_ri = " << car_ri  ; // ", car2_ri = " << ch2_ri ;
       cout << "; car_li = " << car_li << " )" <<endl ; //", ch2_li = " << ch2_li << ")" << endl;

        merged->Write();


  
  

       calfile->Close();


       
 
       /*
   TCanvas *c1;
   c1 = (TCanvas*)gROOT->GetListOfCanvases()->FindObject("c1");
  if (!c1) c1 = new TCanvas("c1","c1",10,10,1000,1000);
   c1->SetCanvasSize(700,700);
       */

	  return 0;}


Int_t  getmintime(Double_t a,Double_t b,Double_t c, Double_t d){
  if ( a < b ){
      if (c<d) {
	if (a<c) return 1;
	else return 3;
      } else {
	if (a<d) return 1;
	else return 4;
      } 
  } else {
      if (c<d){
	if (b<c) return 2;
        else return 3;
      } else {
        if (b<d) return 2;
        else return 4;
      }
  }
}
 

  
 
