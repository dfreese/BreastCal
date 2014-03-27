int SETUP()
{
  //  if (gROOT->ProcessLine(".L Sel_GetEhis.cc+") !=0 ) return -1; 
//    if (gROOT->ProcessLine(".L /home/miil/MODULE_ANA/ANA_V5/SpeedUp/lib/libModuleAna.so") !=0 ) return -1; 
    if (gROOT->ProcessLine(".L /home/products/BreastCal/lib/libModuleAna.so") !=0 ) return -1; 
  return 0;
}
