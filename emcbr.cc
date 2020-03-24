#include <assert.h>
#include <string.h>
#include <iostream.h>
#include "emcbr.h"
#include "ReadNat/rr_def.h"
#include "ReadNat/re_def.h"
#include "ReadNat/ss_def.h"
#include "KEmcRec/emc_struct.h"
#include "KEmcRec/GEmcRec.h"
#include "VDDCRec/ktracks.h"

void copy(EMCBranch* bemc)          //энергия выделившаяся в калориметре
{
        assert(bemc!=0);
	memset(bemc,0,sizeof(EMCBranch));

	bemc->ncls=semc.emc_ncls;
	bemc->nstr_cls=semc.str_ncls;
	bemc->nstr_trk=semc.str_ntracks;
	bemc->nlkr=emcRec->lkrClusters.size();
	bemc->ncsi=emcRec->csiClusters.size();
        //bemc->total_energy=kedrraw_.Header.Energy*1e-3;
	for(int c=0; c<semc.emc_ncls; c++) {
		bemc->total_energy+=semc.emc_energy[c];
		if( semc.emc_type[c]==1 )
			bemc->lkr_energy+=semc.emc_energy[c];
		else if( semc.emc_type[c]==2 )
			bemc->csi_energy+=semc.emc_energy[c];
	}
}

void copy(TowerClusterBranch* btcls,int c)               //data tower&cluster fill
{
	//assert(btcls!=0&&c<semc.emc_ncls);
	assert(btcls!=0);
 	memset(btcls,0,sizeof(TowerClusterBranch));

	btcls->cluster=c;                                //number cluster
	if( c<0 ) return; //do not copy the rest
	if( semc.emc_type[c]==1 ) {                      //type cluster -> csi if lkr(tower)
		btcls->lkr=1;
	} else if( semc.emc_type[c]==2 ) {
		btcls->csi=1;
	}
	btcls->ncells=semc.emc_ncells[c];                //число ячеек в кластере
	btcls->energy=semc.emc_energy[c];                //энергия кластера
	btcls->x=semc.emc_x[c];                          //координата по x в см.
	btcls->y=semc.emc_y[c];
	btcls->z=semc.emc_z[c];
	//cout<<"Event="<<eDaqNumber<<"\t"<<"c="<<c<<"\t"<<"energy="<<semc.emc_energy[c]<<"\t"<<semc.emc_x[c]<<"\t"<<semc.emc_y[c]<<"\t"<<semc.emc_z[c]<<endl;
        //cout<<"tP(0)="<<tP(0)<<"\t"<<"tP(1)="<<tP(1)<<endl;
	btcls->vx=semc.emc_vx[c];                        //единичный вектор направления
	btcls->vy=semc.emc_vy[c];
	btcls->vz=semc.emc_vz[c];
	btcls->theta=semc.emc_theta[c];                  //угол theta в градусах
	btcls->phi=semc.emc_phi[c];
	btcls->rho=semc.emc_rho[c];                      //rho=sqrt(x*x+y*y) в см
	btcls->dtheta=semc.emc_dtheta[c];
	btcls->dphi=semc.emc_dphi[c];
	btcls->drho=semc.emc_drho[c];
	btcls->theta_str=semc.emc_theta_str[c];          //theta уточненный по полоскам
	btcls->phi_str=semc.emc_phi_str[c];
	btcls->qlty=semc.emc_qlty[c];                    //=1 если максимальная ячейка кластера на краю калориметра
	btcls->ncells_bad=semc.emc_ncells_bad[c];        //число плохих ячеек с которыми соседствуют ячейки кластера
	btcls->str_ncls=semc.emc_str_ncls[c];            //число полосочных кластреов, соответствующих данному emc кластеру
	btcls->str_ntrk=semc.emc_str_ntrk[c];            //число полосочных треков, соответствующих данному emc кластеру
	btcls->dc_ntrk=semc.emc_dc_ntrk[c];              //число треков соответствующих данному emc кластеру
	btcls->emc_ncls=semc.emc_emc_ncls[c];            //число кластеров соответствующих данному emc кластеру
}

void copy(StripClusterBranch* bstrcls,int c)
{
	assert(bstrcls!=0&&c<semc.str_ncls);
	memset(bstrcls,0,sizeof(StripClusterBranch));

	bstrcls->cluster=c;
	if( c<0 ) return; //do not copy the rest
	bstrcls->ncells=semc.str_ncells[c];
    if( semc.str_type[c]==3 )
		bstrcls->type_phi=1;
    else if( semc.str_type[c]==4 )
		bstrcls->type_z=1;
	bstrcls->layer=semc.str_nl[c];
	bstrcls->rho=semc.str_rho[c];
	bstrcls->x=semc.str_xrec[c];
}

void copy(StripTrackBranch* bstrtrk,int t)
{
	assert(bstrtrk!=0&&t<semc.str_ntracks);
	memset(bstrtrk,0,sizeof(StripTrackBranch));

	bstrtrk->track=t;
	if( t<0 ) return; //do not copy the rest
	bstrtrk->nvec=semc.str_track_nvect[t];
	bstrtrk->x0=semc.str_track_x0[t];
	bstrtrk->y0=semc.str_track_y0[t];
	bstrtrk->z0=semc.str_track_z0[t];
	bstrtrk->vx=semc.str_track_vx[t];
	bstrtrk->vy=semc.str_track_vy[t];
	bstrtrk->vz=semc.str_track_vz[t];
	bstrtrk->t1=semc.str_track_t1[t];
	bstrtrk->t2=semc.str_track_t2[t];
}

