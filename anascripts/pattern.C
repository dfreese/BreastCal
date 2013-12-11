
Int_t setpattern(void){
  //http://root.cern.ch/phpBB3/viewtopic.php?f=3&t=14597

  Int_t colors[64];
  for (i=0;i<64;i++) {
   switch (i%5) {
    case 0: colors[i]=kRed; break;
    case 1: colors[i]=kMagenta+2; break;
    case 2: colors[i]=kBlue; break;
    case 3: colors[i]=kGreen; break;
    case 4: colors[i]=kOrange; break;
  }
  }
gStyle->SetPalette((sizeof(colors)/sizeof(Int_t)), colors);

   return 0;}
