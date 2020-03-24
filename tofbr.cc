#include <assert.h>
#include <string.h>
#include <iostream.h>
#include "tofbr.h"
#include "VDDCRec/mtofhits.h"
#include "VDDCRec/ToFTrack.hh"

#include "KrToF/tof_system.h"

void copy(ToFBranch* btof, int t)
{
     assert(btof!=0);
     memset(btof,0,sizeof(ToFBranch));

     int dcschits=0; int amphits=0; int timhits=0;
     float tphi=0;

     btof->nhits=kschit_.nScHits;               //общее колличество хитов
     btof->time[t]=kschit_.time_ns[t];
     btof->beta[t]=kscBhit_.Beta[t][0];         //v/c
     btof->length[t]=kscBhit_.len[t][0];        //длина трека
     btof->type[t]=kscBhit_.ToFhit[t][0];       //0-если нет счетчика,1 - EC left, 2 -B

     for (int i=0; i<kschit_.nScHits; i++)
     {
	 if ( kschit_.numtrk[i]==t )
	 {
	     dcschits++;
	     if ( kschit_.amp_ch[i]!=0 ) amphits++;            //kschit_.amp_ch[i] - амплитуда в каналах
	     if ( kschit_.time_ch[i]!=0) timhits++;            //time_ch[i] - время в каналах
	     tphi+=(kschit_.phi[i]);
	     //                    if(tphi < 0.) tphi +=(2*3.14159);
	 }
     }
     if(dcschits>0) tphi=tphi/dcschits;
     if(tphi < 0.) tphi +=(2*3.14159);
     btof->tof_phi[t]=kschit_.phi[t];          //угол phi
     btof->dchits=dcschits;
     btof->namps=amphits;
     btof->ntimes=timhits;

  //   cout<<"Track="<<t<<"\t"<<"tof time="<<btof->time[t]<<"\t"<<"length="<<btof->length[t]<<endl;
}
