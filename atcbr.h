#ifndef ATCBR_H
# define ATCBR_H

# ifndef __CINT__
#  include "Rtypes.h"
# endif

//dimensions - cm, angles - degrees

struct ATCBranch {
	Int_t natc_cr;   //number of counters crossed by tracks
	Int_t natc_hits; //number of counters triggered in the event
	Int_t natc_thr;  //number of counters with amplitude above threshold
	Int_t rawdt;     //DeltaT reading
	Float_t dt;      //DeltaT correction in ticks -0.5..0.5
};

struct ATCCounterBranch {
	Int_t icnt;					//ATC number (from 0)
	Int_t track;
	Int_t ntrk;					//number of tracks crossed this counter in event
	Int_t triggered;			//there's data from this counter in the event
	Int_t zero;					//hit is of type Pedestal (defined in AtcHit class)
	Int_t fitted;				//hit is of type Fitted
	Int_t estimated;			//hit is of type Estimated
	Int_t neightrig;			//some neighbouring counters fired
	Int_t wlshit;				//WLS crossed
	Int_t nearwls;				//track goes close to WLS
        Int_t aerogel_REGION;
        Int_t aerogel_REGION0;                  //ATC_DOUBLE_CROSS
        Int_t aerogel_REGION5;
        Int_t aerogel_REGION20;
        Int_t active_REGION;
        Int_t active_REGION0;
        Int_t active_REGION5;
        Int_t active_REGION20;
 	Int_t testreg;				//cosmic test region of counter crossed at total thickness
        Int_t single_aerogel_REGION;
        Int_t single_aerogel_REGION0;           //ATC_SINGLE_CROSS
        Int_t single_aerogel_REGION5;
        Int_t single_aerogel_REGION20;
        Int_t single_active_REGION;
        Int_t single_active_REGION0;
        Int_t single_active_REGION5;
        Int_t single_active_REGION20;
 	Int_t single_testreg;				//cosmic test region of counter crossed at total thickness
        Int_t in_aerogel_REGION;
        Int_t in_aerogel_REGION0;                //ATC_IN_CROSS
        Int_t in_aerogel_REGION5;
        Int_t in_aerogel_REGION20;
        Int_t in_active_REGION;
        Int_t in_active_REGION0;
        Int_t in_active_REGION5;
        Int_t in_active_REGION20;
 	Int_t in_testreg;				//cosmic test region of counter crossed at total thickness
        Int_t out_aerogel_REGION;
        Int_t out_aerogel_REGION0;                //ATC_OUT_CROSS
        Int_t out_aerogel_REGION5;
        Int_t out_aerogel_REGION20;
        Int_t out_active_REGION;
        Int_t out_active_REGION0;
        Int_t out_active_REGION5;
        Int_t out_active_REGION20;
 	Int_t out_testreg;				//cosmic test region of counter crossed at total thickness
	Float_t amp, rtime;			//raw amplitude (ADC counts) and raw time of pulse maximum (ticks)
	Float_t time;				//time corrected on DeltaT (ticks)
	Float_t chi2;               //chi square of pulse fit
	Float_t npe, npen;			//number of photoelectrons and that normalized by track length
	Float_t tlen, pathwls;		//track length in aerogel volume and that in WLS
	Float_t rin, phiin, zin;	//local cylindric coordinates of track in-point       //локальные координаты
	Float_t rout, phiout, zout; //local cylindric coordinates of track out-point
	Float_t rwls, phiwls, zwls;	//position of WLS crossing (middle plane of the counter)
	Float_t neighnpe;           //maximum amplitude of neighbouring counters

	Float_t Rin_gl, Phiin_gl, Zin_gl;	//global cylindric coordinates of track in-point       //глобальные координаты
	Float_t Rout_gl, Phiout_gl, Zout_gl;     //global cylindric coordinates of track out-point

};

static const char* atcBranchList="natc_cr/I:natc_hits:natc_thr:rawdt:dt/F";

static const char* atcCounterBranchList="i/I:t:ntrk"                                   //то что кладем в ветку
":triggered:zero:fitted:estimated:neightrig:wlshit:nearwls:aerogel_region:aerogel_region0:aerogel_region5:aerogel_region20"
":active_region:active_region0:active_region5:active_region20:test"
":single_aerogel_region:single_aerogel_region0:single_aerogel_region5:single_aerogel_region20:single_active_region:single_active_region0"
":single_active_region5:single_active_region20:single_test"
":in_aerogel_region:in_aerogel_region0:in_aerogel_region5:in_aerogel_region20:in_active_region:in_active_region0:in_active_region5:in_active_region20:in_test"
":out_aerogel_region:out_aerogel_region0:out_aerogel_region5:out_aerogel_region20:out_active_region:out_active_region0:out_active_region5:out_active_region20:out_test"
":amp/F:rtime:time:chi2:npe:npen:tlen:pathwls:rin:phiin:zin:rout:phiout:zout:rwls:phiwls:zwls:neighnpe:Rin_gl:Phiin_gl:Zin_gl:Rout_gl:Phiout_gl:Zout_gl";

extern void copy(ATCBranch*);
extern void copy(ATCCounterBranch* bcnt,int icnt,int t);

#endif
