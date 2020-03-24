#include <assert.h>
#include <string.h>
#include <math.h>
#include "vddcbr.h"
#include "VDDCRec/ktracks.h"
#include "KEmcRec/emc_struct.h"
#include "KrAtc/atcrec.h"
#include "KrAtc/atc_geometry.h"
#include "KrAtc/atc_regions.h"
#include "KrAtc/AtcHit.hh"
#include "KrAtc/AtcPar.hh"
#include "KrAtc/AtcRec.hh"

#include "ReadNat/re_def.h"
#include "ReadNat/rr_def.h"
#include "ReadNat/read_nat_c.h"


void copy(VertexBranch* bvertex)
{
	assert(bvertex!=0);
	memset(bvertex,0,sizeof(VertexBranch));

	bvertex->ntrk=eTracksAll;            //����� ������������������ ������
	bvertex->nip=eTracksIP;              //����� ������ �� ����� ��������������
	bvertex->nbeam=eTracksBeam;          //����� �������� ������

	bvertex->x=eVertexX;                 //���������� ��������������� ������� � ����-�� ������
	bvertex->y=eVertexY;
	bvertex->z=eVertexZ;
	bvertex->sig_x=eSigmaX;
	bvertex->sig_y=eSigmaY;
	bvertex->sig_z=eSigmaZ;
	bvertex->theta2t=acos(CThe2t)*180./M_PI;  //���� (� ������������) ����� ������� (��������� �����������) ��� ������������ (eTracksAll=2) �������
	bvertex->phi2t=acos(CPhi2t)*180./M_PI;    //���� (XY ��) ����� ������� (��������� �����������) ��� ������������ (eTracksAll=2) �������
}

void copy(TrackBranch* btrack,int t)
{
	assert(btrack!=0&&t<eTracksAll);
	memset(btrack,0,sizeof(TrackBranch));

	btrack->track=t;
	if( t<0 ) return; //do not copy the rest
	btrack->charge=-(int)tCharge(t); //so that electron will have negative charge
	btrack->ip=tVertex(t);
	btrack->nvec=tVectors(t);
	btrack->nvecxy=tVectorsXY(t);
	btrack->nvecz=tVectorsZ(t);
	btrack->nhits=tHits(t);
	btrack->nhitsxy=tHitsXY(t);
	btrack->nhitsz=tHits(t)-tHitsXY(t);
	btrack->nhitsvd=tHitsVD(t);
	btrack->p=tP(t);
	btrack->pt=tPt(t);
	btrack->theta=tTeta(t);
	btrack->phi=ktrrec_.FITRAK[t]+(ktrrec_.FITRAK[t]<0?360:0);
	btrack->chi2=tCh2(t);
	btrack->rc=tRc(t);
	btrack->xc=tXc(t);
	btrack->yc=tYc(t);
	btrack->zc=tZc(t);
	btrack->za=tZa(t);
	btrack->ph0=tPh0(t);
	btrack->ph1=tPh1(t);
	btrack->ph2=tPh2(t);
	btrack->x0=tX0(t);           //���������� ������ �����
	btrack->y0=tY0(t);
	btrack->z0=tZ0(t);
	btrack->x1=tX1(t);           //���������� ������ ����� �� �����
	btrack->y1=tY1(t);
	btrack->z1=tZ1(t);
	btrack->x2=tX2(t);           //���������� ��������� ����� �� �����
	btrack->y2=tY2(t);
	btrack->z2=tZ2(t);
	btrack->vx=tVx(t);           //����������� ���������� �������
	btrack->vy=tVy(t);
	btrack->vz=tVz(t);
	btrack->emc_ncls=semc.dc_emc_ncls[t];
	btrack->atc_ncnt=atc_track.ncnt_on_track[t];
}
