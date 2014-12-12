#ifndef DECODERLIB_H
#define DECODERLIB_H

struct DaqPacket;

#include <vector>
#include <string>
#include "Syspardef.h"

int DecodePacketByteStream(const std::vector<char> & packet_byte_stream,
                           DaqPacket & packet_info);

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
