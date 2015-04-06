#ifndef EVENTCAL_H
#define EVENTCAL_H

#include "TObject.h"

class ModuleCal :
        public TObject
{
public:
    Long64_t ct;
    Float_t ft;
    Float_t E;
    Float_t Espat;
    Float_t x;
    Float_t y;
    Int_t crystal;

    EventCal();
    virtual ~EventCal();

    ClassDef(EventCal,1)
};

#endif
