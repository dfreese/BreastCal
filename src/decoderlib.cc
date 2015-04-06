#include "decoderlib.h"
#include "daqpacket.h"
#include "chipevent.h"
#include "ModuleDat.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cmath>

#define SHIFTFACCOM 4
#define SHIFTFACSPAT 12
#define BYTESPERCOMMON 12 // ( 4 commons per module * 3 values per common )
#define VALUESPERSPATIAL 4


void getfinmodule(
        short panel,
        short chip,
        short local_module,
        short & module,
        short & fin)
{
    short fourup = std::floor(chip / 8);
    short localchip = chip % 8;
    fin = 6 - 2 * std::floor(localchip / 2);
    if (panel) {
        if (chip < 16) {
            fin++;
        }
        module = (3 - local_module) % 4;
        if (!(chip%2)) {
            module += MODULES_PER_RENA;
        }
        if (!(fourup % 2)) {
            module += (MODULES_PER_FIN / 2);
        }
    } else {
        if (chip >= 16) {
            fin++;
        }
        module = local_module % 4;
        if (chip % 2) {
            module += MODULES_PER_RENA;
        }
        if (fourup % 2) {
            module += (MODULES_PER_FIN / 2);
        }
    }
}

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
        packet_info.module_trigger_flags[ii] = bool((trigCode >> ii) & (0x01));
        if (packet_info.module_trigger_flags[ii]) {
            packet_info.no_modules_triggered++;
        }
    }

    unsigned int expected_packet_size =
            10 + 32 * packet_info.no_modules_triggered;

    if (packet_byte_stream.size() != expected_packet_size) {
        return(-4);
    }

    packet_info.timestamp = 0;
    for (int ii = 3; ii < 9; ii++) {
        packet_info.timestamp = packet_info.timestamp << 7;
        packet_info.timestamp += long((packet_byte_stream[ii] & 0x7F));
    }

    // Remaining bytes are ADC data for each channel
    //packet_info.adc_values.reserve((expected_packet_size - 1 - 9)/2);
    int adc_value(0);
    for (unsigned int counter = 9;
         counter < (expected_packet_size - 1);
         counter += 2)
    {
        short value(packet_byte_stream[counter] & 0x3F);
        value = value << 6;
        value += short(packet_byte_stream[counter + 1] & 0x3F);
        //packet_info.adc_values.push_back(value);
        packet_info.adc_values[adc_value] = value;
        adc_value++;
    }
    return(0);
}

int PacketToRawEvents(
        const DaqPacket & packet_info,
        std::vector<chipevent> & raw_events,
        int cartridge_id,
        int sourcepos)
{
    raw_events.reserve(packet_info.no_modules_triggered);
    int current_module_count(0);
    for (int module = 0; module < 4; module++) {
        if (packet_info.module_trigger_flags[module]) {
            chipevent rawevent;

            rawevent.ct = packet_info.timestamp;
            rawevent.chip = packet_info.rena + 2 * packet_info.fpga
                    + packet_info.daq_board * RENAS_PER_FOURUPBOARD;
            rawevent.cartridge = cartridge_id;
            rawevent.module = module;

            int even = rawevent.chip % 2;

            int channel_offset = current_module_count * BYTESPERCOMMON
                                 + even *
                                   packet_info.no_modules_triggered *
                                   SHIFTFACCOM;

            int spatial_offset = BYTESPERCOMMON *
                                 packet_info.no_modules_triggered
                                 + current_module_count * VALUESPERSPATIAL
                                 - even *
                                   packet_info.no_modules_triggered *
                                   SHIFTFACSPAT;

            rawevent.com1h = packet_info.adc_values[0 + channel_offset];
            rawevent.u1h = packet_info.adc_values[1 + channel_offset];
            rawevent.v1h = packet_info.adc_values[2 + channel_offset];
            rawevent.com1 = packet_info.adc_values[3 + channel_offset];
            rawevent.u1 = packet_info.adc_values[4 + channel_offset];
            rawevent.v1 = packet_info.adc_values[5 + channel_offset];
            rawevent.com2h = packet_info.adc_values[6 + channel_offset];
            rawevent.u2h = packet_info.adc_values[7 + channel_offset];
            rawevent.v2h = packet_info.adc_values[8 + channel_offset];
            rawevent.com2 = packet_info.adc_values[9 + channel_offset];
            rawevent.u2 = packet_info.adc_values[10 + channel_offset];
            rawevent.v2 = packet_info.adc_values[11 + channel_offset];

            rawevent.a = packet_info.adc_values[0 + spatial_offset];
            rawevent.b = packet_info.adc_values[1 + spatial_offset];
            rawevent.c = packet_info.adc_values[2 + spatial_offset];
            rawevent.d = packet_info.adc_values[3 + spatial_offset];
            rawevent.pos = sourcepos;

            current_module_count++;
            raw_events.push_back(rawevent);
        }
    }
    return(0);
}

int RawEventToModuleDat(
        const chipevent & rawevent,
        ModuleDat & event,
        float pedestals[CARTRIDGES_PER_PANEL]
                       [RENAS_PER_CARTRIDGE]
                       [MODULES_PER_RENA]
                       [CHANNELS_PER_MODULE],
        int threshold,
        int nohit_threshold,
        int panel_id)
{
    float * module_pedestals =
            pedestals[rawevent.cartridge][rawevent.chip][rawevent.module];

    if ((rawevent.com1h - module_pedestals[5]) < threshold) {
        if ((rawevent.com2h - module_pedestals[7]) > nohit_threshold) {
            event.apd = 0;
            event.ft = (((rawevent.u1h & 0xFFFF) << 16) | (rawevent.v1h & 0xFFFF));
            event.Ec = rawevent.com1 - module_pedestals[4];
            event.Ech = rawevent.com1h - module_pedestals[5];
        } else {
            return(-2);
        }
    } else if ((rawevent.com2h - module_pedestals[7]) < threshold ) {
        if ( (rawevent.com1h - module_pedestals[5]) > nohit_threshold ) {
            event.apd = 1;
            event.ft = (((rawevent.u2h & 0xFFFF) << 16) | (rawevent.v2h & 0xFFFF));
            event.Ec = rawevent.com2 - module_pedestals[6];
            event.Ech = rawevent.com2h- module_pedestals[7];
        } else {
            return(-2);
        }
    } else {
        return(-1);
    }

    event.ct = rawevent.ct;
    event.chip = rawevent.chip;
    event.cartridge = rawevent.cartridge;
    event.a = rawevent.a - module_pedestals[0];
    event.b = rawevent.b - module_pedestals[1];
    event.c = rawevent.c - module_pedestals[2];
    event.d = rawevent.d - module_pedestals[3];

    event.module = rawevent.module;

    getfinmodule(panel_id,
                 rawevent.chip,
                 rawevent.module,
                 event.module,
                 event.fin);

    event.E = event.a + event.b + event.c + event.d;
    event.x = float(event.c + event.d - (event.b + event.a)) / float(event.E);
    event.y = float(event.a + event.d - (event.b + event.c)) / float(event.E);

    if (event.apd == 1) {
        event.y *= -1;
    }

    event.pos = rawevent.pos;

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
