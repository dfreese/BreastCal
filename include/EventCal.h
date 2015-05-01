#ifndef EVENTCAL_H
#define EVENTCAL_H

//#ifndef ROOT_TObject
//#include "TObject.h"
//#endif
//class EventCal
//        :
//        public TObject
//{
//public:
//    Long64_t ct;
//    Float_t ft;
//    Float_t E;
//    Float_t Espat;
//    Float_t x;
//    Float_t y;
//    Int_t crystal;
//    EventCal();
//    virtual ~EventCal();

//    ClassDef(EventCal,5);
//};

struct EventCal {
    // The course timestamp recorded by the FPGA controlling the rena
    long ct;
    // The fine timestamp calculated from the UV timing circle
    float ft;
    // Calibrated Energy of the event
    float E;
    // The denominator for the anger logic in adc values
    float anger_denom;
    // The x and y positions of the event within the flood histogram
    float x;
    float y;
    // The ID of the crystal within the system.  Since only 19 bits are required
    // for the crystal ID, the 12 MSBs may be used to flag events for things.
    int crystal;
};

#endif
