#include <assert.h>
#include <string.h>
#include <iostream.h>
#include "atcbr.h"
#include "vddcbr.h"
#include "KrAtc/atcrec.h"
#include "KrAtc/atc_geometry.h"
#include "KrAtc/atc_regions.h"
//#include "KrAtc/atc_to_track.h"
#include "KrAtc/AtcHit.hh"
#include "KrAtc/AtcPar.hh"
#include "KrAtc/AtcRec.hh"
#include "VDDCRec/ktracks.h"
#include "ReadNat/re_def.h"
#include "ReadNat/rr_def.h"
#include "ReadNat/read_nat_c.h"

void copy(ATCBranch* batc)
{
	assert(batc!=0);
	memset(batc,0,sizeof(ATCBranch));
	for(int i=0; i<NATC; i++) if( atc_track.cnt_ntrk[i] ) batc->natc_cr++;
	batc->natc_hits=atcRec->hits.size();
	batc->rawdt=atc_rec.rawDeltaT;
	batc->dt=atc_rec.deltaT;
}

void copy(ATCCounterBranch* bcnt,int icnt, int t)
{
	assert(bcnt!=0 && icnt>=-1 && icnt<NATC && t<eTracksAll);      //если какое-либо выражение не верно - сообщение об ошибке и завершение программы
	memset(bcnt,0,sizeof(ATCCounterBranch));

	bcnt->icnt=icnt;
	if( icnt<0 ) return; //do not copy the rest

	//counter hit parameters
//        cout<<"Rec atc_rec.htyp="<<atc_rec.htyp[icnt]<<endl;        //!!!
        bcnt->triggered=atc_rec.trg[icnt];
	bcnt->zero=(!bcnt->triggered || (atc_rec.htyp[icnt]&AtcHit::Pedestal))?1:0;
	bcnt->fitted=(atc_rec.htyp[icnt]&AtcHit::Fitted)?1:0;
	bcnt->estimated=(atc_rec.htyp[icnt]&AtcHit::Estimated)?1:0;

//   if( !(atc_rec.htyp[icnt]&AtcHit::Pedestal) )                   //!!!!!!!!!
	bcnt->amp=atc_rec.amp[icnt];
	bcnt->rtime=atc_rec.rawtime[icnt];
	bcnt->time=atc_rec.time[icnt];
	bcnt->chi2=atc_rec.chi2[icnt];
	bcnt->ntrk=atc_track.cnt_ntrk[icnt];                      //кол-во треков пересекаемых счетчики

	// if none track hits the counter stop here
	if( !bcnt->ntrk ) return;

        int tlast=atc_track.cnt_ntrk[icnt];
//	int t=atc_track.cnt_tracks[icnt][0]-1;                            //номер трека
	if(atc_track.cnt_tracks[icnt][0]-1==t)
	{
	    bcnt->track=t;

	    //cout<<"==========================================================="<<endl;

	    // find index of this counter on the track t
	    int i;
	    for(i=0; i<atc_track.ncnt_on_track[t]; i++)                    //цикл по числу пересеченных счетчиков на трек
		if( atc_track.cnt_cross[t][i]==icnt+1 ) break;         //если номер пересеченного счетчика (из списка счетчиков) равен номеру счетчика то останавливаем цикл
//           i=atc_track.ncnt_on_track[t]-1;

	    //whether track goes near or right to wls
	    bcnt->wlshit=atc_track.wls_hit[t][i];
	    bcnt->nearwls=atc_track.near_wls[t][i];
	    //geometric parameters of track crossing in mm&rad
	    bcnt->tlen=atc_track.tlen_in_aer[t][i]*0.1;    //tlen in cm   !!!
	    bcnt->pathwls=atc_track.tlen_in_wls[t][i]*0.1;
//
	    int last=atc_track.ncnt_on_track[t];                      //кол-во счет-ов пересеченных треком
	    int ncall;
	    ncall=atc_track.track_end[t][last];                       //какой конец трека пересек счетчик (для космики)
	    // set 1pe amplitude to default (30 cnts) if not available but the counter working
	    if( !cntPar[icnt+1].isOffline() && cntPar[icnt+1].getA1pe()==0 && kedrrun_cb_.Header.RunType != 64)  //64 - simulation run
		bcnt->npe=bcnt->amp/30.;
	    else
		bcnt->npe=atc_rec.npe[icnt];

	    //cout<<"   atcbr"<<"\t"<<"icnt="<<icnt<<"\t"<<"i="<<i<<"\t"<<"npe="<<atc_rec.npe[icnt]<<endl;
	    //cout<<"t="<<t<<"\t"<<"last="<<last<<"\t"<<"icnt+1="<<icnt+1<<endl;

	    //find normalized amplitude in photoelectrons per track length in aerogel
	    // and find if the track goes within inner or test region as determined in regions.h
	    if( bcnt->tlen!=0 ) {
		if( atc_is_endcap(icnt+1) )
		    bcnt->npen=ThicknessEndcap*0.1*bcnt->npe/bcnt->tlen;
		else if( atc_is_barrel_1layer(icnt+1) )
		    bcnt->npen=ThicknessBarrel1*0.1*bcnt->npe/bcnt->tlen;
		else if( atc_is_barrel_2layer(icnt+1) )
		    bcnt->npen=ThicknessBarrel2*0.1*bcnt->npe/bcnt->tlen;
	    }

	    bcnt->rin=atc_track.rin[t][i]*0.1;                        //локальные координаты в см и радианах
	    bcnt->rout=atc_track.rout[t][i]*0.1;
	    bcnt->phiin=atc_track.phiin[t][i];
	    bcnt->phiout=atc_track.phiout[t][i];
	    bcnt->zin=atc_track.zin[t][i]*0.1;
	    bcnt->zout=atc_track.zout[t][i]*0.1;
	    bcnt->rwls=atc_track.rwls[t][i]*0.1;
	    bcnt->phiwls=atc_track.phiwls[t][i];        //*rad2deg - для перевода в градусы
	    bcnt->zwls=atc_track.zwls[t][i]*0.1;

	    double rin_gl,phiin_gl,zin_gl,rout_gl,phiout_gl,zout_gl;
	    atc_get_global(icnt+1,atc_track.rin[t][i],atc_track.phiin[t][i],atc_track.zin[t][i],rin_gl, phiin_gl,zin_gl);
	    atc_get_global(icnt+1,atc_track.rout[t][i],atc_track.phiout[t][i],atc_track.zout[t][i],rout_gl, phiout_gl,zout_gl);
	    bcnt->Rin_gl=rin_gl*0.1;
	    bcnt->Rout_gl=rout_gl*0.1;                    //глобальные координаты   - получаем преобразованием
	    bcnt->Phiin_gl=phiin_gl;
	    bcnt->Phiout_gl=phiout_gl;
	    bcnt->Zin_gl=zin_gl*0.1;
	    bcnt->Zout_gl=zout_gl*0.1;
	    //===================================================ATC_DOUBLE_CROSS===================================
	    //если есть пересечение области аэрогеля с разным отступом от стенок и от шифтера
	    if( atc_track_cross_region(t,i,ATC_AEROGEL_REGION,ATC_DOUBLE_CROSS) )  bcnt->aerogel_REGION=1;
	    else  bcnt->aerogel_REGION=0;

	    if( atc_track_cross_region(t,i,ATC_AEROGEL_REGION0,ATC_DOUBLE_CROSS) )  bcnt->aerogel_REGION0=1;
	    else  bcnt->aerogel_REGION0=0;

	    if( atc_track_cross_region(t,i,ATC_AEROGEL_REGION5,ATC_DOUBLE_CROSS) )  bcnt->aerogel_REGION5=1;
	    else  bcnt->aerogel_REGION5=0;

	    if( atc_track_cross_region(t,i,ATC_AEROGEL_REGION20,ATC_DOUBLE_CROSS) )  bcnt->aerogel_REGION20=1;
	    else  bcnt->aerogel_REGION20=0;

	    //если есть пересечение области аэрогеля и шифтера с разным отступом от стенок
	    if( atc_track_cross_region(t,i,ATC_ACTIVE_REGION,ATC_DOUBLE_CROSS) )  bcnt->active_REGION=1;
	    else  bcnt->active_REGION=0;

	    if( atc_track_cross_region(t,i,ATC_ACTIVE_REGION0,ATC_DOUBLE_CROSS) )  bcnt->active_REGION0=1;
	    else  bcnt->active_REGION0=0;

	    if( atc_track_cross_region(t,i,ATC_ACTIVE_REGION5,ATC_DOUBLE_CROSS) )  bcnt->active_REGION5=1;
	    else  bcnt->active_REGION5=0;

	    if( atc_track_cross_region(t,i,ATC_ACTIVE_REGION20,ATC_DOUBLE_CROSS) )  bcnt->active_REGION20=1;
	    else  bcnt->active_REGION20=0;

	    if( atc_track_cross_region(t,i,ATC_CRT_REGION,ATC_DOUBLE_CROSS) ) bcnt->testreg=1;
	    else  bcnt->testreg=0;


	    //===================================================ATC_SINGLE_CROSS===================================
	    //если есть пересечение области аэрогеля с разным отступом от стенок и от шифтера
	    if( atc_track_cross_region(t,i,ATC_AEROGEL_REGION,ATC_SINGLE_CROSS) )  bcnt->single_aerogel_REGION=1;
	    else  bcnt->single_aerogel_REGION=0;

	    if( atc_track_cross_region(t,i,ATC_AEROGEL_REGION0,ATC_SINGLE_CROSS) )  bcnt->single_aerogel_REGION0=1;
	    else  bcnt->single_aerogel_REGION0=0;

	    if( atc_track_cross_region(t,i,ATC_AEROGEL_REGION5,ATC_SINGLE_CROSS) )  bcnt->single_aerogel_REGION5=1;
	    else  bcnt->single_aerogel_REGION5=0;

	    if( atc_track_cross_region(t,i,ATC_AEROGEL_REGION20,ATC_SINGLE_CROSS) )  bcnt->single_aerogel_REGION20=1;
	    else  bcnt->single_aerogel_REGION20=0;

	    //если есть пересечение области аэрогеля и шифтера с разным отступом от стенок
	    if( atc_track_cross_region(t,i,ATC_ACTIVE_REGION,ATC_SINGLE_CROSS) )  bcnt->single_active_REGION=1;
	    else  bcnt->single_active_REGION=0;

	    if( atc_track_cross_region(t,i,ATC_ACTIVE_REGION0,ATC_SINGLE_CROSS) )  bcnt->single_active_REGION0=1;
	    else  bcnt->single_active_REGION0=0;

	    if( atc_track_cross_region(t,i,ATC_ACTIVE_REGION5,ATC_SINGLE_CROSS) )  bcnt->single_active_REGION5=1;
	    else  bcnt->single_active_REGION5=0;

	    if( atc_track_cross_region(t,i,ATC_ACTIVE_REGION20,ATC_SINGLE_CROSS) )  bcnt->single_active_REGION20=1;
	    else  bcnt->single_active_REGION20=0;

	    if( atc_track_cross_region(t,i,ATC_CRT_REGION,ATC_SINGLE_CROSS) ) bcnt->single_testreg=1;
	    else  bcnt->single_testreg=0;

	    //===================================================ATC_IN_CROSS===================================
	    //если есть пересечение области аэрогеля с разным отступом от стенок и от шифтера
	    if( atc_track_cross_region(t,i,ATC_AEROGEL_REGION,ATC_IN_CROSS) )  bcnt->in_aerogel_REGION=1;
	    else  bcnt->in_aerogel_REGION=0;

	    if( atc_track_cross_region(t,i,ATC_AEROGEL_REGION0,ATC_IN_CROSS) )  bcnt->in_aerogel_REGION0=1;
	    else  bcnt->in_aerogel_REGION0=0;

	    if( atc_track_cross_region(t,i,ATC_AEROGEL_REGION5,ATC_IN_CROSS) )  bcnt->in_aerogel_REGION5=1;
	    else  bcnt->in_aerogel_REGION5=0;

	    if( atc_track_cross_region(t,i,ATC_AEROGEL_REGION20,ATC_IN_CROSS) )  bcnt->in_aerogel_REGION20=1;
	    else  bcnt->in_aerogel_REGION20=0;

	    //если есть пересечение области аэрогеля и шифтера с разным отступом от стенок
	    if( atc_track_cross_region(t,i,ATC_ACTIVE_REGION,ATC_IN_CROSS) )  bcnt->in_active_REGION=1;
	    else  bcnt->in_active_REGION=0;

	    if( atc_track_cross_region(t,i,ATC_ACTIVE_REGION0,ATC_IN_CROSS) )  bcnt->in_active_REGION0=1;
	    else  bcnt->in_active_REGION0=0;

	    if( atc_track_cross_region(t,i,ATC_ACTIVE_REGION5,ATC_IN_CROSS) )  bcnt->in_active_REGION5=1;
	    else  bcnt->in_active_REGION5=0;

	    if( atc_track_cross_region(t,i,ATC_ACTIVE_REGION20,ATC_IN_CROSS) )  bcnt->in_active_REGION20=1;
	    else  bcnt->in_active_REGION20=0;

	    if( atc_track_cross_region(t,i,ATC_CRT_REGION,ATC_IN_CROSS) ) bcnt->in_testreg=1;
	    else  bcnt->in_testreg=0;

	    //===================================================ATC_OUT_CROSS===================================
	    //если есть пересечение области аэрогеля с разным отступом от стенок и от шифтера
	    if( atc_track_cross_region(t,i,ATC_AEROGEL_REGION,ATC_OUT_CROSS) )  bcnt->out_aerogel_REGION=1;
	    else  bcnt->out_aerogel_REGION=0;

	    if( atc_track_cross_region(t,i,ATC_AEROGEL_REGION0,ATC_OUT_CROSS) )  bcnt->out_aerogel_REGION0=1;
	    else  bcnt->out_aerogel_REGION0=0;

	    if( atc_track_cross_region(t,i,ATC_AEROGEL_REGION5,ATC_OUT_CROSS) )  bcnt->out_aerogel_REGION5=1;
	    else  bcnt->out_aerogel_REGION5=0;

	    if( atc_track_cross_region(t,i,ATC_AEROGEL_REGION20,ATC_OUT_CROSS) )  bcnt->out_aerogel_REGION20=1;
	    else  bcnt->out_aerogel_REGION20=0;

	    //если есть пересечение области аэрогеля и шифтера с разным отступом от стенок
	    if( atc_track_cross_region(t,i,ATC_ACTIVE_REGION,ATC_OUT_CROSS) )  bcnt->out_active_REGION=1;
	    else  bcnt->out_active_REGION=0;

	    if( atc_track_cross_region(t,i,ATC_ACTIVE_REGION0,ATC_OUT_CROSS) )  bcnt->out_active_REGION0=1;
	    else  bcnt->out_active_REGION0=0;

	    if( atc_track_cross_region(t,i,ATC_ACTIVE_REGION5,ATC_OUT_CROSS) )  bcnt->out_active_REGION5=1;
	    else  bcnt->out_active_REGION5=0;

	    if( atc_track_cross_region(t,i,ATC_ACTIVE_REGION20,ATC_OUT_CROSS) )  bcnt->out_active_REGION20=1;
	    else  bcnt->out_active_REGION20=0;

	    if( atc_track_cross_region(t,i,ATC_CRT_REGION,ATC_OUT_CROSS) ) bcnt->out_testreg=1;
	    else  bcnt->out_testreg=0;

	    //find if any of neighbouring counters triggered in the event and get
	    // maximum amplitude for neighbours
	    int nbrs[3]={(icnt%20==0)?icnt+19:icnt-1, (icnt%20==19)?icnt-19:icnt+1,
		(icnt>=20&&icnt<60)?(icnt<40?icnt+20:icnt-20):-1 };

	    for(int k=0; k<3; k++) {
		if( nbrs[k]<0 ) continue;
		if( atc_rec.trg[nbrs[k]] ) {
		    bcnt->neightrig=1;
		    if( atc_rec.npe[nbrs[k]]>bcnt->neighnpe )
			bcnt->neighnpe=atc_rec.npe[nbrs[k]];
		}
	    }
	}
}

