int SETUP()
{
#  if ( gSystem->Load("~/MODULE_ANA/ANA_V5/SpeedUp/lib/libModuleDatDict.so")  == -1 )   return -1;
  if ( gSystem->Load("libModuleDatDict.so")  == -1 )   return -1;
  return 0;
		}
