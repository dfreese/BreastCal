#ifndef SEGMENTATION_H
#define SEGMENTATION_H

class TGraph;
class TH2F;

int PeakSearch(
        TGraph * peaks_sorted,
        TH2F * flood,
        int verbose,
        int &validflag,
        float &cost,
        bool APD);

#endif // SEGMENTATION_H
