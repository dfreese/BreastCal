#include "TROOT.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TSpectrum.h"
#include "TSpectrum2.h"
#include "TH2F.h"
#include "TSpectrum.h"
#include "Syspardef.h"
#include <string>
#include "CoincEvent.h"

Int_t drawmod(
        TH1F *hi[FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE],
        TCanvas *ccc,
        const std::string & filename);
Int_t drawmod_crys2(
        TH1F *hi[MODULES_PER_FIN][APDS_PER_MODULE][CRYSTALS_PER_APD],
        TCanvas *ccc,
        const std::string & filename);
Int_t writ(TH1D *hi[PEAKS], TCanvas *ccc, Char_t filename[MAXFILELENGTH]);
Int_t writ2d(TH2F *hi[PEAKS], TCanvas *ccc, Char_t filename[MAXFILELENGTH]);
Int_t writval(
        Float_t mean_crystaloffset[CARTRIDGES_PER_PANEL][FINS_PER_CARTRIDGE][MODULES_PER_FIN][APDS_PER_MODULE][CRYSTALS_PER_APD],
        const std::string & outfile);
TH2F *get2dcrystal(Float_t vals[CRYSTALS_PER_APD], Char_t title[40]);
Float_t getmax (TSpectrum *s, Int_t npeaks, Int_t verbose);

int ReadPerCrystalCal(
        std::string filename,
        float crystal_correction[SYSTEM_PANELS]
                                [CARTRIDGES_PER_PANEL]
                                [FINS_PER_CARTRIDGE]
                                [MODULES_PER_FIN]
                                [APDS_PER_MODULE]
                                [CRYSTALS_PER_APD]);

int ReadPerCrystalEnergyCal(
        std::string filename,
        float crystal_correction[SYSTEM_PANELS]
                                [CARTRIDGES_PER_PANEL]
                                [FINS_PER_CARTRIDGE]
                                [MODULES_PER_FIN]
                                [APDS_PER_MODULE]
                                [CRYSTALS_PER_APD][2]);

int WritePerCrystalCal(
        std::string filename,
        float crystal_correction[SYSTEM_PANELS]
                                [CARTRIDGES_PER_PANEL]
                                [FINS_PER_CARTRIDGE]
                                [MODULES_PER_FIN]
                                [APDS_PER_MODULE]
                                [CRYSTALS_PER_APD]);

int WritePerApdAsPerCrystalCal(
        std::string filename,
        float apd_correction[SYSTEM_PANELS]
                            [CARTRIDGES_PER_PANEL]
                            [FINS_PER_CARTRIDGE]
                            [MODULES_PER_FIN]
                            [APDS_PER_MODULE]);

int WritePerCrystalEnergyCal(
        std::string filename,
        float crystal_correction[SYSTEM_PANELS]
                                [CARTRIDGES_PER_PANEL]
                                [FINS_PER_CARTRIDGE]
                                [MODULES_PER_FIN]
                                [APDS_PER_MODULE]
                                [CRYSTALS_PER_APD]
                                [2]);

int WritePerCartridgeAsPerCrystalEnergyCal(
        std::string filename,
        float crystal_correction[SYSTEM_PANELS]
                                [CARTRIDGES_PER_PANEL]
                                [2]);

int AddPerCrystalCal(
        float augend[SYSTEM_PANELS]
                    [CARTRIDGES_PER_PANEL]
                    [FINS_PER_CARTRIDGE]
                    [MODULES_PER_FIN]
                    [APDS_PER_MODULE]
                    [CRYSTALS_PER_APD],
        float addend[SYSTEM_PANELS]
                    [CARTRIDGES_PER_PANEL]
                    [FINS_PER_CARTRIDGE]
                    [MODULES_PER_FIN]
                    [APDS_PER_MODULE]
                    [CRYSTALS_PER_APD],
        float sum[SYSTEM_PANELS]
                 [CARTRIDGES_PER_PANEL]
                 [FINS_PER_CARTRIDGE]
                 [MODULES_PER_FIN]
                 [APDS_PER_MODULE]
                 [CRYSTALS_PER_APD]);

int AddPerApdAsPerCrystalCal(
        float augend[SYSTEM_PANELS]
                    [CARTRIDGES_PER_PANEL]
                    [FINS_PER_CARTRIDGE]
                    [MODULES_PER_FIN]
                    [APDS_PER_MODULE]
                    [CRYSTALS_PER_APD],
        float addend[SYSTEM_PANELS]
                    [CARTRIDGES_PER_PANEL]
                    [FINS_PER_CARTRIDGE]
                    [MODULES_PER_FIN]
                    [APDS_PER_MODULE],
        float sum[SYSTEM_PANELS]
                 [CARTRIDGES_PER_PANEL]
                 [FINS_PER_CARTRIDGE]
                 [MODULES_PER_FIN]
                 [APDS_PER_MODULE]
                 [CRYSTALS_PER_APD]);

int AddPerCrystalLocationAsPerCrystalCal (
        float augend[SYSTEM_PANELS]
                    [CARTRIDGES_PER_PANEL]
                    [FINS_PER_CARTRIDGE]
                    [MODULES_PER_FIN]
                    [APDS_PER_MODULE]
                    [CRYSTALS_PER_APD],
        float addend[CRYSTALS_PER_APD],
        float sum[SYSTEM_PANELS]
                 [CARTRIDGES_PER_PANEL]
                 [FINS_PER_CARTRIDGE]
                 [MODULES_PER_FIN]
                 [APDS_PER_MODULE]
                 [CRYSTALS_PER_APD]);

int BoundsCheckEvent(const CoincEvent & event);
int EnergyGateEvent(
        const CoincEvent & event,
        float energy_low,
        float energy_high);

float GetEventOffset(
        const CoincEvent & event,
        float crystal_cal[SYSTEM_PANELS]
                         [CARTRIDGES_PER_PANEL]
                         [FINS_PER_CARTRIDGE]
                         [MODULES_PER_FIN]
                         [APDS_PER_MODULE]
                         [CRYSTALS_PER_APD]);

float GetEventOffsetEdep(
        const CoincEvent & event,
        float crystal_cal[SYSTEM_PANELS]
                         [CARTRIDGES_PER_PANEL]
                         [FINS_PER_CARTRIDGE]
                         [MODULES_PER_FIN]
                         [APDS_PER_MODULE]
                         [CRYSTALS_PER_APD]
                         [2]);
