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
    long ct;
    float ft;
    float E;
    float Espat;
    float x;
    float y;
    int crystal;
};

#endif
