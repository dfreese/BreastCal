#ifndef SYSPARDEF_H
#define SYSPARDEF_H

#define PAULS_PANELID true
#define DAQ_BEFORE01042013 false
#define MODULE_BASED_READOUT true

#define MINENTRIES 10000
#define MINEHISTENTRIES 10

//FIXME
#define CHANPERMODULE 8
#define MODULES 4
#define NUM_RENA_PER_4UPBOARD 8
#define FOURUPBOARDS_PER_DAQ 1
#define RENACHIPS NUM_RENA_PER_4UPBOARD*FOURUPBOARDS_PER_DAQ
#define MINENTRIES 10000
#define MINEHISTENTRIES 10



#define SYSTEM_PANELS 2
#define CARTRIDGES_PER_PANEL 2
#define FINS_PER_CARTRIDGE 8
#define FOURUPBOARDS_PER_CARTRIDGE 4
#define RENAS_PER_FOURUPBOARD 8
#define RENAS_PER_CARTRIDGE RENAS_PER_FOURUPBOARD*FOURUPBOARDS_PER_CARTRIDGE
#define MODULES_PER_RENA 4
#define CHANNELS_PER_MODULE 8

#define MODULES_PER_FIN 16
#define XTALS_PER_APD 64
#define APDS_PER_MODULE 2



#define MAXFILELENGTH      160
#define E_up 3000
#define E_low -200
#define Ebins 320
#define Ebins_pixel 160
#define E_up_com 1400
#define E_low_com -200
#define Ebins_com 160
#define Ebins_com_pixel 160
#define PP_LOW_EDGE 700
#define PP_UP_EDGE 2800
#define PP_LOW_EDGE_COM 350
#define PP_UP_EDGE_COM 1200
#define SATURATIONPEAK 1200


#define MINHISTENTRIES 200

#define FILENAMELENGTH	120
#define MAXFILELENGTH	160
#define PEAKS 64

#define EFITMIN  0 
#define EFITMAX  2400

#define EFITMIN_COM  0 
#define EFITMAX_COM  1200


#define MAXHITS 10000000

//FIXME David commented out below line and changed it to 150 to accomodate long dataset
//#define MAXCUTS 50
#define MAXCUTS 150

#define DEFAULTTHRESHOLD -600;
#define DEFAULT_NOHIT_THRESHOLD -50;
#define INFTY 1e99;

#define EGATEMIN 400
#define EGATEMAX 650
// rollovertime = 2^42 
#define ROLLOVERTIME 4398046511104 

#define UVFREQUENCY 980e3
#define COARSECLOCKFREQUENCY 12e6



#endif // SYSPARDEF
