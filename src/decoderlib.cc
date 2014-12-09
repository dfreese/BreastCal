#include "decoderlib.h"
#include "daqpacket.h"
//#include <iostream>

int DecodePacketByteStream(
        const std::vector<char> & packet_byte_stream,
        DaqPacket & packet_info)
{
    if (packet_byte_stream.empty()) {
        return(-1);
    }

    if ((int(packet_byte_stream[0] & 0xFF )) != 0x80) {
        // first byte of packet needs to be 0x80
        return(-2);
    }

    packet_info.backend_address = int((packet_byte_stream[1] & 0x7C) >> 2);
    packet_info.daq_board = int((packet_byte_stream[1] & 0x03) >> 0);
    packet_info.fpga = int((packet_byte_stream[2] & 0x30) >> 4);
    packet_info.rena = int((packet_byte_stream[2] & 0x40) >> 6);

    int trigCode = int(packet_byte_stream[2] & 0x0F);

    if (trigCode == 0) {
        return(-3);
    }

    packet_info.no_modules_triggered = 0;
    for (int ii = 0; ii < 4; ii++) {
        packet_info.no_modules_triggered += int((trigCode >> ii) & (0x01));
        packet_info.module_trigger_flags[ii] = bool((trigCode >> ii) & (0x01));
    }

    unsigned int expected_packet_size = 10 +
                                        32 * packet_info.no_modules_triggered;

    if (packet_byte_stream.size() != expected_packet_size) {
//        std::cout << "packet_byte_stream.size(): " << packet_byte_stream.size()
//                  << "expected_packet_size: " << expected_packet_size
//                  << std::endl;
        return(-4);
    }
    // Byte 1 is not used - 0x1f

    for (int ii = 3; ii < 9; ii++) {
        packet_info.timestamp = packet_info.timestamp << 7;
        packet_info.timestamp += long((packet_byte_stream[ii] & 0x7F));
    }

    // Remaining bytes are ADC data for each channel
    for (unsigned int counter = 9;
         counter < (expected_packet_size - 1);
         counter += 2)
    {
        short value(packet_byte_stream[counter] & 0x3F);
        value = value << 6;
        value += short(packet_byte_stream[counter + 1] & 0x3F);
        packet_info.adc_values.push_back(value);
    }
    return(0);
}
