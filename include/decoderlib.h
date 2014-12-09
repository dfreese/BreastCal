#ifndef DECODERLIB_H
#define DECODERLIB_H

struct DaqPacket;

#include <vector>

int DecodePacketByteStream(const std::vector<char> & packet_byte_stream,
                           DaqPacket & packet_info);

#endif // DECODERLIB_H
