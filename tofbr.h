#ifndef TOFBR_H
# define TOFBR_H

#include "Rtypes.h"

struct ToFBranch{
    Int_t nhits;            //общее число хитов
    Int_t dchits;
    Int_t namps;
    Int_t ntimes;
    Float_t time[2];          //время срабатывания счетчика на трек
    Float_t length[2];        //длина трека
    Float_t beta[2];          //v/c
    Float_t tof_phi[2];
    Float_t type[2];
};

static const char* ToFBranchList="nhits/i:dchits/i:namps/i:ntimes/i:time[2]/f:length[2]/f:beta[2]/f:phi[2]/f:type[2]/i";

extern void copy(ToFBranch* btof,int t);

#endif
