#ifndef DAQPACKET_H
#define DAQPACKET_H

#include <vector>

struct DaqPacket {
    int backend_address;
    int daq_board;
    int fpga;
    int rena;
    int no_modules_triggered;
    bool module_trigger_flags[4];
    long timestamp;
    std::vector<short> adc_values;
};

#endif // DAQPACKET_H
