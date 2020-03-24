#ifndef MUBR_H
# define MUBR_H

#include "Rtypes.h"

struct MUBranch{
        Int_t nmuhits; //Number of hits in MU-chamber
        Int_t dcmuhits; //Number of DC attached hits in MU-chamber
        Int_t octant; //Number of non neabor octants
        Int_t llayer;  //Last hit layer in MU chambers
        Char_t status; //mu_hit status: is binded to track or not
    };

static const char* MUBranchList="nhits/i:dcmuhits/i:octant/i:layer/i:status/b";

extern void copy(MUBranch* bmu,int t,int nhits);

#endif
