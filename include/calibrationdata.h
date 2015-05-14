#ifndef CALIBRATIONDATA_H
#define CALIBRATIONDATA_H

#include <Syspardef.h>

struct CalibrationData {
    float pedestals[SYSTEM_PANELS]
                   [CARTRIDGES_PER_PANEL]
                   [RENAS_PER_CARTRIDGE]
                   [MODULES_PER_RENA]
                   [CHANNELS_PER_MODULE];
    float centers_u[SYSTEM_PANELS]
                   [CARTRIDGES_PER_PANEL]
                   [FINS_PER_CARTRIDGE]
                   [MODULES_PER_FIN]
                   [APDS_PER_MODULE];
    float centers_v[SYSTEM_PANELS]
                   [CARTRIDGES_PER_PANEL]
                   [FINS_PER_CARTRIDGE]
                   [MODULES_PER_FIN]
                   [APDS_PER_MODULE];
    float gain_spat[SYSTEM_PANELS]
                   [CARTRIDGES_PER_PANEL]
                   [FINS_PER_CARTRIDGE]
                   [MODULES_PER_FIN]
                   [APDS_PER_MODULE]
                   [CRYSTALS_PER_APD];
    float gain_comm[SYSTEM_PANELS]
                   [CARTRIDGES_PER_PANEL]
                   [FINS_PER_CARTRIDGE]
                   [MODULES_PER_FIN]
                   [APDS_PER_MODULE]
                   [CRYSTALS_PER_APD];
    float eres_spat[SYSTEM_PANELS]
                   [CARTRIDGES_PER_PANEL]
                   [FINS_PER_CARTRIDGE]
                   [MODULES_PER_FIN]
                   [APDS_PER_MODULE]
                   [CRYSTALS_PER_APD];
    float eres_comm[SYSTEM_PANELS]
                   [CARTRIDGES_PER_PANEL]
                   [FINS_PER_CARTRIDGE]
                   [MODULES_PER_FIN]
                   [APDS_PER_MODULE]
                   [CRYSTALS_PER_APD];
    float crystal_x[SYSTEM_PANELS]
                   [CARTRIDGES_PER_PANEL]
                   [FINS_PER_CARTRIDGE]
                   [MODULES_PER_FIN]
                   [APDS_PER_MODULE]
                   [CRYSTALS_PER_APD];
    float crystal_y[SYSTEM_PANELS]
                   [CARTRIDGES_PER_PANEL]
                   [FINS_PER_CARTRIDGE]
                   [MODULES_PER_FIN]
                   [APDS_PER_MODULE]
                   [CRYSTALS_PER_APD];
    bool use_crystal[SYSTEM_PANELS]
                    [CARTRIDGES_PER_PANEL]
                    [FINS_PER_CARTRIDGE]
                    [MODULES_PER_FIN]
                    [APDS_PER_MODULE]
                    [CRYSTALS_PER_APD];
    float time_offset_cal[SYSTEM_PANELS]
                         [CARTRIDGES_PER_PANEL]
                         [FINS_PER_CARTRIDGE]
                         [MODULES_PER_FIN]
                         [APDS_PER_MODULE]
                         [CRYSTALS_PER_APD];
};

#endif // CALIBRATIONDATA_H
