int SETUP()
{
    if (gROOT->ProcessLine(".L Sel_GetFloods.cc+") !=0 ) return -1; 
  //  if (gROOT->ProcessLine(".L ~/MODULE_ANA/ANA_V5/SpeedUp/lib/libModuleAna.so") !=0 ) return -1; 
  return 0;
}
