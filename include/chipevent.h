#ifndef CHIPEVENT_H
#define CHIPEVENT_H

struct chipevent {
    long ct;
    short chip;
    short panel;
    short cartridge;
    short module;
    short com1;
    short com2;
    short com1h;
    short com2h;
    short u1;
    short v1;
    short u2;
    short v2;
    short u1h;
    short v1h;
    short u2h;
    short v2h;
    short a;
    short b;
    short c;
    short d;
    int pos;
};


#endif // CHIPEVENT_H
