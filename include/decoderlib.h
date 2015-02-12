#ifndef DECODERLIB_H
#define DECODERLIB_H

struct DaqPacket;
struct chipevent;
class ModuleDat;

#include <vector>
#include <string>
#include "Syspardef.h"

void getfinmodule(
        short panel,
        short chip,
        short local_module,
        short & module,
        short & fin);

int DecodePacketByteStream(const std::vector<char> & packet_byte_stream,
                           DaqPacket & packet_info);

int PacketToRawEvents(
        const DaqPacket & packet_info,
        std::vector<chipevent> & raw_events,
        int cartridge_id,
        int sourcepos);

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

#endif // DECODERLIB_H