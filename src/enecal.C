/*

This program fills the energy histograms for every crystal, needed to do energy calibration. If we don't have a valid segmnetation, the crystal id remains -1.

 A new file is created, this is a little redundant. Crystal id's are assigned.

 */


#include "enecal.h" 
//void usage(void);

//void usage(void){
// cout << " Parsetomodule -f [filename] [-p [pedfile] -v -o [outputfilename]] -n [nrfiles in loop] -t [threshold]" <<endl;
//  return;}


int main(int argc, Char_t *argv[])
{
 	cout << " Welcome to EneCal. Performs Crystal Binning." << endl;

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	Char_t		filename[FILENAMELENGTH] = "";
	Int_t		verbose = 0;
	Int_t		ix;
        Bool_t          fileset=0;
        ModuleDat *event = 0;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	for(ix = 1; ix < argc; ix++) {

		/*
		 * Verbose '-v'
		 */
		if(strncmp(argv[ix], "-v", 2) == 0) {
			cout << "Verbose Mode " << endl;
			verbose = 1;
		}
		/* filename '-f' */
		if(strncmp(argv[ix], "-f", 2) == 0) {
			if(strlen(argv[ix + 1]) < FILENAMELENGTH) {
				sprintf(filename, "%s", argv[ix + 1]);
				fileset=1;
			}
			else {
				cout << "Filename " << argv[ix + 1] << " too long !" << endl;
				cout << "Exiting.." << endl;
				return -99;
			}
		}
	}

        rootlogon(verbose);


	if (!fileset) { cout << " Please specify Filename. Exiting. " << endl; return -99;}
	TFile *rfile = new TFile(filename,"OPEN");
       
        Char_t filebase[FILENAMELENGTH],peaklocationfilename[FILENAMELENGTH],newrootfile[FILENAMELENGTH]; 
        Char_t tmpname[50],tmptitle[50];
        Int_t i,j,k,m,lines;
        Int_t validpeaks[RENACHIPS][4][2];
        Double_t U_x[RENACHIPS][4][2][64];
        Double_t U_y[RENACHIPS][4][2][64];
        Double_t aa, bb;
        ifstream infile;
        TH1F *Ehist[RENACHIPS][4][2][64];
        TH1F *Ehist_com[RENACHIPS][4][2][64];
	//        TF1 *Efits[4][2][64];
        TVector* ppVals = (TVector *) rfile->Get("pp_spat");
        TVector* ppVals_com = (TVector *) rfile->Get("pp_com");
        TVector* uu_c = (TVector *) rfile->Get("uu_c");
        TVector* vv_c = (TVector *) rfile->Get("vv_c");

        if (!((ppVals) && (ppVals_com) && (uu_c) && (vv_c))){
	    cout << " Problem reading ppVals and/or uu/cc circle centers.\nExiting. " << endl;
            return -3;
	  }

	//        TCanvas *c1 =new TCanvas();
        Char_t treename[40];
        TDirectory *subdir[RENACHIPS];

        strncpy(filebase,filename,strlen(filename)-5);
        filebase[strlen(filename)-5]='\0';
        if (verbose) cout << " filebase = " << filebase << endl;


      if (verbose) { 
        cout << " Circle centers :: " << endl;
     
       for (int kk=0;kk<RENACHIPS;kk++){
         cout << " Chip " << kk << " : " ;
           for (i=0;i<4;i++){
	     cout << " M" << i << ":: " ;
              for (j=0;j<2;j++){
		cout << "A" << j << "- " ;
                cout << (*uu_c)(kk+i*RENACHIPS+j*MODULES*RENACHIPS) << " " << (*vv_c)(kk+i*RENACHIPS+j*MODULES*RENACHIPS) << " ";
	      }
	   }
	   cout << endl;
       }
      } // verbose


	for (m=0;m<RENACHIPS;m++){
        for (j=0;j<4;j++){
	  for (i=0;i<2;i++){
	     validpeaks[m][j][i]=0;
	     sprintf(peaklocationfilename,"%s.RENA%d.unit%d_apd%d_peaks",filebase,m,j,i);
             strcat(peaklocationfilename,".txt");
             infile.open(peaklocationfilename);
             lines = 0;
              while (1){
               if (!infile.good()) break;
               infile >> k >>  aa >> bb;
               if (k < 64) { U_y[m][j][i][k]=aa; U_x[m][j][i][k]=bb;}
       //       cout << k << ", " << U_x[i][j][k]<< ", " << U_y[i][j][k] << endl;
               lines++;      
                }
	      if (verbose) cout << "Found " << lines-1 << " peaks in  file " << peaklocationfilename << endl;
       infile.close();
       if (lines==65){ if (verbose) cout << "Setting Validpeaks " << endl; validpeaks[m][j][i]=1; }
	}
	}
	}


	if (verbose){
	  for (m=0;m<RENACHIPS;m++){
        for (j=0;j<4;j++){
	  for (i=0;i<2;i++){
            cout << " m = " << m << " i = " << i << " j = " << j << " : " <<validpeaks[m][j][i] <<endl;
	  }}
	}
	}




	//	cout << " U_x[0][0][21] ="<< U_x[0][0][21] << " U_y[0][0][21] = " << U_y[0][0][21]<<endl;

	

       TFile *f;
       sprintf(newrootfile,"%s.enecal",filebase);
       strcat(newrootfile,".root");
       if (verbose) {
	 cout << "Creating New Root file : " << newrootfile << endl;}
       f = new TFile(newrootfile,"RECREATE");

	TTree *block;
	/* loop over the chips !! */

	for (m=0;m<RENACHIPS;m++){

       /* Creating histograms */
	  for (j=0;j<4;j++){
            for (i=0;i<2;i++){
	      for (k=0;k<64;k++){
                sprintf(tmpname,"Ehist[%d][%d][%d][%d]",m,j,i,k);
                sprintf(tmptitle,"RENA %d Unit %d Module %d Pixel %d",m,j,i,k);
                Ehist[m][j][i][k] = new TH1F(tmpname,tmptitle,Ebins,E_low,E_up);
                sprintf(tmpname,"Ehist_com[%d][%d][%d][%d]",m,j,i,k);
                sprintf(tmptitle,"RENA %d Unit %d Module %d Pixel %d Common",m,j,i,k);
                Ehist_com[m][j][i][k] = new TH1F(tmpname,tmptitle,Ebins_com,E_low_com,E_up_com);
		//                sprintf(tmpname,"Efits[%d][%d][%d]",i,j,k);
		//                sprintf(tmptitle,"Unit %d Module %d Pixel %d",i,j,k);
		//                Efits[i][j][k] = new TF1(tmpname,"gaus",EBINS,EMIN,EMAX);
	      }
            }
          }
	} // m


	  /*
	  if (m==0)  block = (TTree *) rfile->Get("block1");
          else  block = (TTree *) rfile->Get("block2");
	  */

	 sprintf(treename,"mdata");
         block = (TTree *) rfile->Get(treename);
         if (!block) {
	   cout << " Problem reading Tree " << treename  << " from file " << filename << endl;
           cout << " Exiting " << endl;
           return -10;}
	 //	 entries=block->GetEntries();

	 cout << " Looping over " << block->GetEntries() << " entries." ;
	block->SetBranchAddress("eventdata",&event);
	   /*
          block->SetBranchAddress("ct",&event.ct);
	  block->SetBranchAddress("chip",&event.chip);
	  block->SetBranchAddress("module",&event.module);
	  block->SetBranchAddress("apd",&event.apd);
	  block->SetBranchAddress("Ec",&event.Ec);
	  block->SetBranchAddress("Ech",&event.Ech);
	  block->SetBranchAddress("x",&event.x);
	  block->SetBranchAddress("y",&event.y);
	  block->SetBranchAddress("E",&event.E);
	  block->SetBranchAddress("ft",&event.ft);
	  block->SetBranchAddress("a",&event.a);
	  block->SetBranchAddress("b",&event.b);
	  block->SetBranchAddress("c",&event.c);
	  block->SetBranchAddress("d",&event.d);
	  block->SetBranchAddress("id",&event.id);
	  block->SetBranchAddress("pos",&event.pos);
	   */

         strncpy(filebase,filename,strlen(filename)-5);
         filebase[strlen(filename)-5]='\0';
  
	 if (verbose) cout << "\n. Cloning Tree " << endl;
          TTree *calblock = block->CloneTree(0);
	  sprintf(treename,"calblock");// Energy calibrated event data ");
	  /*
         if (m==0) calblock->SetName("calblock1");
         else calblock->SetName("calblock2");
	  */
          calblock->SetName(treename);

          int chip;
          int module;
          int apd;

          cout << " Looping over data .. ";
	  for (i=0;i<block->GetEntries();i++){
	    //	    if ((i%100000)==0) fprintf(stdout,"%d Events Processed\r",i);
	   //      	  for (i=0;i<1e5;i++){
	    block->GetEntry(i);
	    chip=event->chip;
            module=event->module;
            apd=event->apd; 

	   // perform time calibration
            if ((event->apd==0) || (event->apd==1) ) event->ft=finecalc(event->ft,(*uu_c)(event->chip+RENACHIPS*event->module+event->apd*MODULES*RENACHIPS),(*vv_c)(event->chip+RENACHIPS*event->module+event->apd*MODULES*RENACHIPS) );


	    if (validpeaks[chip][module][apd]){
               
	      // if (validpeaks[m][0][0]&&UNIT0.com1h<threshold)
	        event->id=getcrystal(event->x,event->y,U_x[chip][module][apd],U_y[chip][module][apd],verbose);
		if ((event->id)>=0){  
                    Ehist[chip][module][apd][event->id]->Fill(event->E); 
                    Ehist_com[chip][module][apd][event->id]->Fill(-event->Ec); 
		} // if valid crystalid 
            } // if validpeaks

             calblock->Fill();
             
	     //	     cout << validpeaks[0][0] << " "<<UNIT0.E << " " << UNIT0.id<< endl;

	  }


	  cout << " .... Done looping over the events " <<endl;

          ppVals->Write("pp_spat");
          ppVals_com->Write("pp_com");

          calblock->AutoSave();
	  //          f->Write();     
	  for (m=0;m<RENACHIPS;m++){
           sprintf(tmpname,"RENA%d",m);
           subdir[m] = f->mkdir(tmpname);
           subdir[m]->cd();
	  for (j=0;j<2;j++){
            for (i=0;i<4;i++){
	      for (k=0;k<64;k++){
                Ehist[m][i][j][k]->Write();
                Ehist_com[m][i][j][k]->Write();
	      }
            }
          }
	} // loop over m

	//	Ehist[1][0][0][34]->Draw(); c1->Print("test.png");
 
          f->Close();
         
	rfile->Close();


	return 0;}


Int_t getcrystal(Double_t x, Double_t y, Double_t xpos[64], Double_t ypos[64], Int_t verbose){
  //  cout << " Getcrystal :: " << " xpos[21] = " << xpos[21] << " ypos[21] = " << ypos[21] << endl;
  //  cout << " x = " << x << " y = " << y << endl;
  
  Double_t dist,min;
  Int_t histnr;

  histnr=9999;
  min=100000;
  if ((TMath::Abs(x)>1)||(TMath::Abs(y)>1)) return -3;
   for (Int_t k=0;k<PEAKS;k++){
      //      dist=(*(*xpeaks+k)-xdata)*(*(*xpeaks+k)-xdata)+(*(*ypeaks+k)-ydata)*(*(*ypeaks+k)-ydata);

      dist=TMath::Power((Float_t) ypos[k]-y,2)+TMath::Power((Float_t) xpos[k]-x,2);

     //if (debvar) cout << "k = " << k << "dist = " << dist << endl;

     if (dist<min) {histnr=k;min=dist;

       //   if (debvar) cout << "--------> k = " << k <<" min = " << min <<endl;
       }
     } //for loop 
   //      if (verbose) cout << "FINAL :: histnr = " << histnr <<" min = " << min << endl;
     if (histnr!=9999) { 
       return histnr;
	}
     
  
     else { if (verbose) 
	 {cout << "No associated histogram found !" << endl;
	   cout << " Entry :  x = " << x << " y = " << y <<endl;}
     }
    min=10000;
    histnr=9999;
    
  return -2;}




Double_t finecalc(Double_t uv, Float_t u_cent, Float_t v_cent){
  Double_t tmp;
  Int_t UV = (Int_t) uv;
  Int_t u = (( UV & 0xFFFF0000 ) >> 16 );
  Int_t v = ( UV & 0xFFFF );
  // cout << " finecalc : u = " << u << " v = " << v  << " ( center :: " << u_cent << ","<< v_cent << ")";
  tmp=TMath::ATan2(u-u_cent,v-v_cent);
  //  cout << " tmp = " << tmp << endl;
  if (tmp < 0. ) { tmp+=2*3.141592;}
  return tmp;///(2*3.141592*CIRCLEFREQUENCY);
}

