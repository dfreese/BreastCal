#ifndef DECODERLIB_H
#define DECODERLIB_H

struct DaqPacket;
struct chipevent;
class ModuleDat;
class EventCal;

#include <vector>
#include <string>
#include "Syspardef.h"

void getfinmodule(
        int panel,
        int chip,
        int local_module,
        short & module,
        short & fin);

int DecodePacketByteStream(const std::vector<char> & packet_byte_stream,
                           DaqPacket & packet_info);

int PacketToRawEvents(
        const DaqPacket & packet_info,
        std::vector<chipevent> & raw_events,
        int cartridge_id,
        int sourcepos = 0);

int RawEventToModuleDat(
        const chipevent & rawevent,
        ModuleDat & event,
        float pedestals[CARTRIDGES_PER_PANEL]
                       [RENAS_PER_CARTRIDGE]
                       [MODULES_PER_RENA]
                       [CHANNELS_PER_MODULE],
        int threshold,
        int nohit_threshold,
        int panel_id);


int RawEventToEventCal(
        const chipevent & rawevent,
        EventCal & event,
        float pedestals[SYSTEM_PANELS]
                       [CARTRIDGES_PER_PANEL]
                       [RENAS_PER_CARTRIDGE]
                       [MODULES_PER_RENA]
                       [CHANNELS_PER_MODULE],
        float centers_u[SYSTEM_PANELS]
                       [CARTRIDGES_PER_PANEL]
                       [FINS_PER_CARTRIDGE]
                       [MODULES_PER_FIN]
                       [APDS_PER_MODULE],
        float centers_v[SYSTEM_PANELS]
                       [CARTRIDGES_PER_PANEL]
                       [FINS_PER_CARTRIDGE]
                       [MODULES_PER_FIN]
                       [APDS_PER_MODULE],
        float gain_spat[SYSTEM_PANELS]
                       [CARTRIDGES_PER_PANEL]
                       [FINS_PER_CARTRIDGE]
                       [MODULES_PER_FIN]
                       [APDS_PER_MODULE]
                       [CRYSTALS_PER_APD],
        float gain_comm[SYSTEM_PANELS]
                       [CARTRIDGES_PER_PANEL]
                       [FINS_PER_CARTRIDGE]
                       [MODULES_PER_FIN]
                       [APDS_PER_MODULE]
                       [CRYSTALS_PER_APD],
        float eres_spat[SYSTEM_PANELS]
                       [CARTRIDGES_PER_PANEL]
                       [FINS_PER_CARTRIDGE]
                       [MODULES_PER_FIN]
                       [APDS_PER_MODULE]
                       [CRYSTALS_PER_APD],
        float eres_comm[SYSTEM_PANELS]
                       [CARTRIDGES_PER_PANEL]
                       [FINS_PER_CARTRIDGE]
                       [MODULES_PER_FIN]
                       [APDS_PER_MODULE]
                       [CRYSTALS_PER_APD],
        float crystal_x[SYSTEM_PANELS]
                       [CARTRIDGES_PER_PANEL]
                       [FINS_PER_CARTRIDGE]
                       [MODULES_PER_FIN]
                       [APDS_PER_MODULE]
                       [CRYSTALS_PER_APD],
        float crystal_y[SYSTEM_PANELS]
                       [CARTRIDGES_PER_PANEL]
                       [FINS_PER_CARTRIDGE]
                       [MODULES_PER_FIN]
                       [APDS_PER_MODULE]
                       [CRYSTALS_PER_APD],
        bool use_crystal[SYSTEM_PANELS]
                        [CARTRIDGES_PER_PANEL]
                        [FINS_PER_CARTRIDGE]
                        [MODULES_PER_FIN]
                        [APDS_PER_MODULE]
                        [CRYSTALS_PER_APD],
        int threshold,
        int nohit_threshold,
        int panel_id);

int ReadPedestalFile(const std::string & filename,
                     float pedestals[CARTRIDGES_PER_PANEL]
                                    [RENAS_PER_CARTRIDGE]
                                    [MODULES_PER_RENA]
                                    [CHANNELS_PER_MODULE]);

int WritePedestalFile(const std::string & filename,
                      double mean[CARTRIDGES_PER_PANEL]
                                 [RENAS_PER_CARTRIDGE]
                                 [MODULES_PER_RENA]
                                 [CHANNELS_PER_MODULE],
                      double rms[CARTRIDGES_PER_PANEL]
                                [RENAS_PER_CARTRIDGE]
                                [MODULES_PER_RENA]
                                [CHANNELS_PER_MODULE],
                      int events[CARTRIDGES_PER_PANEL]
                                [RENAS_PER_CARTRIDGE]
                                [MODULES_PER_RENA]);

int ReadCalibrationFile(
        const std::string & filename,
        bool use_crystal[SYSTEM_PANELS]
                        [CARTRIDGES_PER_PANEL]
                        [FINS_PER_CARTRIDGE]
                        [MODULES_PER_FIN]
                        [APDS_PER_MODULE]
                        [CRYSTALS_PER_APD],
        float gain_spat[SYSTEM_PANELS]
                       [CARTRIDGES_PER_PANEL]
                       [FINS_PER_CARTRIDGE]
                       [MODULES_PER_FIN]
                       [APDS_PER_MODULE]
                       [CRYSTALS_PER_APD],
        float gain_comm[SYSTEM_PANELS]
                       [CARTRIDGES_PER_PANEL]
                       [FINS_PER_CARTRIDGE]
                       [MODULES_PER_FIN]
                       [APDS_PER_MODULE]
                       [CRYSTALS_PER_APD],
        float eres_spat[SYSTEM_PANELS]
                       [CARTRIDGES_PER_PANEL]
                       [FINS_PER_CARTRIDGE]
                       [MODULES_PER_FIN]
                       [APDS_PER_MODULE]
                       [CRYSTALS_PER_APD],
        float eres_comm[SYSTEM_PANELS]
                       [CARTRIDGES_PER_PANEL]
                       [FINS_PER_CARTRIDGE]
                       [MODULES_PER_FIN]
                       [APDS_PER_MODULE]
                       [CRYSTALS_PER_APD],
        float crystal_x[SYSTEM_PANELS]
                       [CARTRIDGES_PER_PANEL]
                       [FINS_PER_CARTRIDGE]
                       [MODULES_PER_FIN]
                       [APDS_PER_MODULE]
                       [CRYSTALS_PER_APD],
        float crystal_y[SYSTEM_PANELS]
                       [CARTRIDGES_PER_PANEL]
                       [FINS_PER_CARTRIDGE]
                       [MODULES_PER_FIN]
                       [APDS_PER_MODULE]
                       [CRYSTALS_PER_APD]);

int ReadUVCirclesFile(
        const std::string & filename,
        float circles_u[SYSTEM_PANELS]
                       [CARTRIDGES_PER_PANEL]
                       [FINS_PER_CARTRIDGE]
                       [MODULES_PER_FIN]
                       [APDS_PER_MODULE],
        float circles_v[SYSTEM_PANELS]
                       [CARTRIDGES_PER_PANEL]
                       [FINS_PER_CARTRIDGE]
                       [MODULES_PER_FIN]
                       [APDS_PER_MODULE]);

int ReadPedestalFile(
        const std::string & filename,
        float pedestals[SYSTEM_PANELS]
                       [CARTRIDGES_PER_PANEL]
                       [RENAS_PER_CARTRIDGE]
                       [MODULES_PER_RENA]
                       [CHANNELS_PER_MODULE]);

#endif // DECODERLIB_H
