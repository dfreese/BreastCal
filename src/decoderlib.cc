#include "decoderlib.h"
#include "daqpacket.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cmath>

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


int ReadPedestalFile(
        const std::string & filename,
        float pedestals[CARTRIDGES_PER_PANEL]
                       [RENAS_PER_CARTRIDGE]
                       [MODULES_PER_RENA]
                       [CHANNELS_PER_MODULE])
{
    std::ifstream pedestal_value_filestream;

    pedestal_value_filestream.open(filename.c_str());
    if (!pedestal_value_filestream) {
        return(-1);
    }

    int lines(0);
    std::string fileline;
    while (std::getline(pedestal_value_filestream, fileline)) {
        lines++;
        std::stringstream line_stream(fileline);
        std::string id_string;
        if ((line_stream >> id_string).fail()) {
            return(-2);
        }
        int cartridge;
        int chip;
        int module;
        int sscan_status(sscanf(id_string.c_str(),
                                "C%dR%dM%d",
                                &cartridge,&chip,&module));

        if (sscan_status != 3) {
            return(-3);
        }

        int events;
        if((line_stream >> events).fail()) {
            return(-4);
        }

        if ((chip >= RENAS_PER_CARTRIDGE) || (chip < 0) ||
                (module >= MODULES_PER_RENA) || (module < 0) ||
                (cartridge >= CARTRIDGES_PER_PANEL) || (cartridge < 0))
        {
            return(-5);
        }
        for (int ii = 0; ii < CHANNELS_PER_MODULE; ii++) {
            float channel_pedestal_value;
            if ((line_stream >> channel_pedestal_value).fail()) {
                return(-6);
            } else {
                pedestals[cartridge][chip][module][ii] = channel_pedestal_value;
            }

            float channel_pedestal_rms;
            if ((line_stream >> channel_pedestal_rms).fail()) {
                return(-7);
            }
        }
    }

    const int expected_lines(CARTRIDGES_PER_PANEL *
                             RENAS_PER_CARTRIDGE *
                             MODULES_PER_RENA);

    if (expected_lines != lines) {
        return(-8);
    } else {
        return(0);
    }
}


int WritePedestalFile(
        const std::string & filename,
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
                  [MODULES_PER_RENA])
{
    std::ofstream file_stream;
    file_stream.open(filename.c_str());

    if (!file_stream) {
        return(-1);
    }

    for (int c=0; c<CARTRIDGES_PER_PANEL; c++) {
        for (int r=0; r<RENAS_PER_CARTRIDGE; r++) {
            for (int i=0; i<MODULES_PER_RENA; i++) {
                // Write out the name of the module assuming there can never
                // be more than 999 renas or 9 modules (hardwired for 4).
                file_stream << std::setfill('0');
                file_stream << "C" << std::setw(1) << c
                        << "R" << std::setw(3) << r
                        << "M" << std::setw(1) << i;
                file_stream << std::setfill(' ') << std::setw(9)
                        << events[c][r][i];
                for (int jjj=0; jjj<CHANNELS_PER_MODULE; jjj++) {
                    file_stream << std::setprecision(0)
                                << std::fixed << std::setw(7);
                    file_stream << mean[c][r][i][jjj];

                    file_stream <<  std::setprecision(2)
                                << std::fixed << std::setw(8);
                    if (events[c][r][i]) {
                        file_stream << std::sqrt(rms[c][r][i][jjj] /
                                                 double(events[c][r][i]));
                    } else {
                        file_stream << 0;
                    }
                }
                file_stream << std::endl;
            }
        }
    }
    if (file_stream.fail()) {
        return(-2);
    } else {
        file_stream.close();
        return(0);
    }
}
