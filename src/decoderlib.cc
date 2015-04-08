#include "decoderlib.h"
#include "daqpacket.h"
#include "chipevent.h"
#include "ModuleDat.h"
#include "eventcal.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <iterator>
#include <TMath.h>

#define SHIFTFACCOM 4
#define SHIFTFACSPAT 12
#define BYTESPERCOMMON 12 // ( 4 commons per module * 3 values per common )
#define VALUESPERSPATIAL 4


void getfinmodule(
        int panel,
        int chip,
        int local_module,
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
    packet_info.adc_values.clear();
    packet_info.adc_values.reserve((expected_packet_size - 1 - 9)/2);
    //int adc_value(0);
    for (unsigned int counter = 9;
         counter < (expected_packet_size - 1);
         counter += 2)
    {
        //short value(packet_byte_stream[counter] & 0x3F);
        //value = value << 6;
        //value += short(packet_byte_stream[counter + 1] & 0x3F);

        short value(((packet_byte_stream[counter] & 0x3F) << 6) +
                     (packet_byte_stream[counter + 1] & 0x3F));

        packet_info.adc_values.push_back(value);
        //packet_info.adc_values[adc_value] = value;
        //adc_value++;
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


float FineCalc(short u, short v, float u_cent, float v_cent) {
    float tmp = TMath::ATan2(u - u_cent, v - v_cent);
    if (tmp < 0.0) {
        tmp += 2 * 3.141592;
    }
    return(tmp);
}

int GetCrystalID(
        float x,
        float y,
        float loc_x[CRYSTALS_PER_APD],
        float loc_y[CRYSTALS_PER_APD])
{
    double min(DBL_MAX);
    int crystal_id(-1);

    if ((TMath::Abs(x) > 1) || (TMath::Abs(y) > 1)) {
        return(-2);
    }
    for (int crystal = 0; crystal < CRYSTALS_PER_APD; crystal++) {
        double dist = TMath::Power(loc_x[crystal] - x, 2) +
                      TMath::Power(loc_y[crystal] - y, 2);
        if (dist < min) {
            crystal_id = crystal;
            min = dist;
        }
    }
    return(crystal_id);
}

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
        int threshold,
        int nohit_threshold,
        int panel_id)
{
    float * module_pedestals =
            pedestals[panel_id][rawevent.cartridge]
                     [rawevent.chip][rawevent.module];

    int apd = -1;
    if ((rawevent.com1h - module_pedestals[5]) < threshold) {
        if ((rawevent.com2h - module_pedestals[7]) > nohit_threshold) {
            apd = 0;
        } else {
            return(-2);
        }
    } else if ((rawevent.com2h - module_pedestals[7]) < threshold ) {
        if ( (rawevent.com1h - module_pedestals[5]) > nohit_threshold ) {
            apd = 1;
        } else {
            return(-2);
        }
    } else {
        return(-1);
    }

    event.ct = rawevent.ct;

    short a = rawevent.a - module_pedestals[0];
    short b = rawevent.b - module_pedestals[1];
    short c = rawevent.c - module_pedestals[2];
    short d = rawevent.d - module_pedestals[3];

    short module = rawevent.module;
    short fin = 0;
    getfinmodule(panel_id,
                 rawevent.chip,
                 rawevent.module,
                 module,
                 fin);

    float * apd_spat_gain =
            gain_spat[panel_id][rawevent.cartridge][fin][module][apd];

    float * apd_spat_eres =
            eres_spat[panel_id][rawevent.cartridge][fin][module][apd];

    float * apd_crystal_x =
            crystal_x[panel_id][rawevent.cartridge][fin][module][apd];

    float * apd_crystal_y =
            crystal_y[panel_id][rawevent.cartridge][fin][module][apd];

    float module_centers_u =
            centers_u[panel_id][rawevent.cartridge][fin][module][apd];

    float module_centers_v =
            centers_v[panel_id][rawevent.cartridge][fin][module][apd];

    int E = a + b + c + d;
    event.x = float(c + d - (b + a)) / float(E);
    event.y = float(a + d - (b + c)) / float(E);
    if (apd == 1) {
        event.y *= -1;
        event.ft = FineCalc(rawevent.u1h,
                            rawevent.v1h,
                            module_centers_u,
                            module_centers_v);
    } else {
        event.ft = FineCalc(rawevent.u2h,
                            rawevent.v2h,
                            module_centers_u,
                            module_centers_v);
    }

    int crystal = GetCrystalID(event.x, event.y, apd_crystal_x, apd_crystal_y);

    if (crystal < 0) {
        return(-3);
    }

    event.crystal = ((((panel_id * CARTRIDGES_PER_PANEL + rawevent.cartridge)
                                 * FINS_PER_CARTRIDGE + fin)
                                 * MODULES_PER_FIN + module)
                                 * APDS_PER_MODULE + apd)
                                 * CRYSTALS_PER_APD + crystal;

    event.Espat = E / apd_spat_gain[crystal] * 511;
    event.E = event.Espat;

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

int ReadCalibrationFile(
        const std::string & filename,
        float use_crystal[SYSTEM_PANELS]
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
                       [CRYSTALS_PER_APD])
{
    std::ifstream calibration_filestream;

    calibration_filestream.open(filename.c_str());
    if (!calibration_filestream) {
        return(-1);
    }


    const int expected_lines(SYSTEM_PANELS *
                             CARTRIDGES_PER_PANEL *
                             FINS_PER_CARTRIDGE *
                             APDS_PER_MODULE *
                             MODULES_PER_FIN *
                             CRYSTALS_PER_APD);

    std::vector<bool> use_crystal_read(expected_lines, 0);
    std::vector<float> crystal_x_read(expected_lines, 0);
    std::vector<float> crystal_y_read(expected_lines, 0);
    std::vector<float> gain_spat_read(expected_lines, 0);
    std::vector<float> gain_comm_read(expected_lines, 0);
    std::vector<float> eres_spat_read(expected_lines, 0);
    std::vector<float> eres_comm_read(expected_lines, 0);

    int lines(0);
    std::string fileline;
    while (std::getline(calibration_filestream, fileline)) {
        if (lines >= expected_lines) {
            return(-2);
        }

        std::stringstream line_stream(fileline);

        bool use_crystal_val;
        if((line_stream >> use_crystal_val).fail()) {
            return(-3);
        } else {
            use_crystal_read[lines] = use_crystal_val;
        }

        if((line_stream >> crystal_x_read[lines]).fail()) {
            return(-4);
        }

        if((line_stream >> crystal_y_read[lines]).fail()) {
            return(-5);
        }

        if((line_stream >> gain_spat_read[lines]).fail()) {
            return(-6);
        }

        if((line_stream >> gain_comm_read[lines]).fail()) {
            return(-7);
        }

        if((line_stream >> eres_spat_read[lines]).fail()) {
            return(-8);
        }

        if((line_stream >> eres_comm_read[lines]).fail()) {
            return(-9);
        }

        lines++;
    }

    if (expected_lines != lines) {
        return(-10);
    } else {
        std::copy(use_crystal_read.begin(),
                  use_crystal_read.end(),
                  &use_crystal[0][0][0][0][0][0]);
        std::copy(crystal_x_read.begin(), crystal_x_read.end(), &crystal_x[0][0][0][0][0][0]);
        std::copy(crystal_y_read.begin(), crystal_y_read.end(), &crystal_y[0][0][0][0][0][0]);
        std::copy(gain_spat_read.begin(), gain_spat_read.end(), &gain_spat[0][0][0][0][0][0]);
        std::copy(gain_comm_read.begin(), gain_comm_read.end(), &gain_comm[0][0][0][0][0][0]);
        std::copy(eres_spat_read.begin(), eres_spat_read.end(), &eres_spat[0][0][0][0][0][0]);
        std::copy(eres_comm_read.begin(), eres_comm_read.end(), &eres_comm[0][0][0][0][0][0]);
        return(0);
    }
}

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
                       [APDS_PER_MODULE])
{
    std::ifstream calibration_filestream;

    calibration_filestream.open(filename.c_str());
    if (!calibration_filestream) {
        return(-1);
    }

    const int expected_lines(SYSTEM_PANELS *
                             CARTRIDGES_PER_PANEL *
                             FINS_PER_CARTRIDGE *
                             APDS_PER_MODULE *
                             MODULES_PER_FIN);

    std::vector<float> circles_u_read(expected_lines, 0);
    std::vector<float> circles_v_read(expected_lines, 0);

    int lines(0);
    std::string fileline;
    while (std::getline(calibration_filestream, fileline)) {
        if (lines >= expected_lines) {
            return(-2);
        }

        std::stringstream line_stream(fileline);

        if((line_stream >> circles_u_read[lines]).fail()) {
            return(-3);
        }

        if((line_stream >> circles_v_read[lines]).fail()) {
            return(-4);
        }

        lines++;
    }

    if (expected_lines != lines) {
        return(-5);
    } else {
        std::copy(circles_u_read.begin(),
                  circles_u_read.end(),
                  &circles_u[0][0][0][0][0]);
        std::copy(circles_v_read.begin(),
                  circles_v_read.end(),
                  &circles_v[0][0][0][0][0]);
        return(0);
    }
}
