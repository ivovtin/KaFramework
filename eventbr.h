#ifndef EVENTBR_H
# define EVENTBR_H

# ifndef __CINT__
#  include "Rtypes.h"
# endif

#include "kframework.h"

struct EventBranch {
	Int_t event, evdaq; //succesive event and DAQ event numbers
	Int_t run;     //guess what it is
	Int_t quality; //event quality number, set by user
        Float_t Ebeam;  //kedrraw_.Header.Energy - beam energy
};

static const char* eventBranchList="event/I:evdaq:run:quality:ebeam/F";

#include <assert.h>
#include <string.h>
#include "ReadNat/re_def.h"

inline void copy(EventBranch* bevent)
{
	assert(bevent!=0);
	memset(bevent,0,sizeof(EventBranch));
	//do not copy bevent->event as it will be filled by user
	bevent->evdaq=kedrraw_.Header.Number;
	bevent->run=kedrraw_.Header.RunNumber;
        //bevent->Ebeam=kedrraw_.Header.Energy;
	bevent->Ebeam=beam_energy;
}

#endif
