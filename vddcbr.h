#ifndef VDDCBR_H
# define VDDCBR_H

# ifndef __CINT__
#  include "Rtypes.h"
# endif

//momentum - MeV/c, dimensions - cm, angles - degrees

struct VertexBranch {
	Int_t ntrk, nip, nbeam;
	Float_t x, y, z;
	Float_t sig_x, sig_y, sig_z;
	Float_t theta2t, phi2t;
};

struct TrackBranch {
	Int_t track;
	Int_t charge;
	Int_t ip;
	Int_t nvec, nvecxy, nvecz, nhits, nhitsxy, nhitsz, nhitsvd;
	Float_t p, pt, theta, phi, chi2;
	Float_t rc, xc, yc, zc, za;
	Float_t ph0, ph1, ph2;
	Float_t x0, x1, x2;
	Float_t y0, y1, y2;
	Float_t z0, z1, z2;
	Float_t vx, vy, vz;
	Int_t emc_ncls;
	Int_t atc_ncnt;
};

static const char* vertexBranchList="ntrk/I:nip:nbeam:xv/F:yv:zv:sigxv:sigyv:sigzv:theta2t:phi2t";

static const char* trackBranchList="t/I:q:ip:nvec:nvecxy:nvecz:nhits:nhitsxy:nhitsz:nhitsvd"
":p/F:pt:theta:phi:chi2:rc:xc:yc:zc:za:ph0:ph1:ph2:x0:x1:x2:y0:y1:y2:z0:z1:z2:vx:vy:vz"
":emc_ncls/I:atc_ncnt";

extern void copy(VertexBranch*);
extern void copy(TrackBranch* btrack,int t);

#endif
