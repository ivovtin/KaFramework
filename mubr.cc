#include <assert.h>
#include <string.h>
#include <iostream.h>
#include "mubr.h"
#include "KrVDDCMu/dcmu.h"
#include "KrMu/mu_system.h"
#include "KrMu/mu_event.h"

void copy(MUBranch* bmu, int t, int nhits)
{
     assert(bmu!=0);
     memset(bmu,0,sizeof(MUBranch));

     bmu->nmuhits=0; bmu->octant=0; bmu->llayer=0; bmu->dcmuhits=0;

     bmu->nmuhits=nhits;
     int doct=0;
     //        float mufl=0;
     //        for (int i=0; i<mu_event.n; i++)
     //        {
     doct=abs(mu_hit_octant(0)-mu_hit_octant(mu_event.n-1));
     //            mufl=doct!=1?(doct==7?(mufl>1?mufl:1):doct):(mufl>1?mufl:1);
     //        }
     //        cout<<"D_oct="<<doct<<endl;
     bmu->octant=mu_hit_octant(mu_event.n-1); //only for cosmic tracks
     bmu->llayer=mu_hit_layer(abs(mu_event.n-1));
     bmu->status=dcmu_hits_status[t];
}
