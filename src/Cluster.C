#include "Cluster.h"
#include "TMath.h"
#include <iostream>

Cluster::Cluster() {}

void Cluster::Add(float x, float y,int binval)
{
    for (int i=0; i<binval; i++) {
        newmeanx = ( x + newmeanx*newpoints) / (newpoints+1);
        newmeany = ( y + newmeany*newpoints) / (newpoints+1);
        wcss +=  TMath::Power(curmeanx-x,2)+TMath::Power(curmeany-y,2);
        newpoints++;
    }
}

void Cluster::NewIteration(void)
{
    curmeanx=newmeanx;
    curmeany=newmeany;
    newmeanx=0;
    newmeany=0;
    newpoints=0;
    wcss=0;
}

void Cluster::Init(float x,float y)
{
    Cluster::NewIteration();
    curmeanx=x;
    curmeany=y;
}


float Cluster::Distance(float x,float y)
{
    return TMath::Power(curmeanx-x,2)+TMath::Power(curmeany-y,2);
}

void Cluster::Print(void)
{
    std::cout << newmeanx << " " << newmeany << " " << newpoints << std::endl;
}
