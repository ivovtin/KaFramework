#ifndef EMCBR_H
# define EMCBR_H

# ifndef __CINT__
#  include "Rtypes.h"
# endif

//energy - MeV, dimensions - cm, angles - degrees

struct EMCBranch {
	Int_t ncls, ncls_trk, nlkr, ncsi;
	Int_t nstr_cls, nstr_trk;
	Float_t total_energy, lkr_energy, csi_energy;
};

struct TowerClusterBranch {
	Int_t cluster;
	Int_t lkr, csi;
	Int_t ncells;
	Float_t energy;
	Float_t x, y, z;
	Float_t vx, vy, vz;
	Float_t theta, phi, rho;
	Float_t dtheta, dphi, drho;
	Float_t theta_str, phi_str;
	Int_t qlty;
	Int_t ncells_bad;
	Int_t str_ncls;
	Int_t str_ntrk;
	Int_t dc_ntrk;
	Int_t emc_ncls;
};

struct StripClusterBranch {
	Int_t cluster;
	Int_t ncells;
	Int_t type_phi, type_z;
	Int_t layer;
	Float_t energy;
	Float_t rho;
	Float_t x;
};

struct StripTrackBranch {
	Int_t track;
	Int_t nvec;
	Float_t x0, y0, z0;
	Float_t vx, vy, vz;
	Float_t t1, t2;
};


static const char* emcBranchList="ncls/I:ncls_trk:nlkr:ncsi:nstrcls:nstrtrk:energy/F:elkr:ecsi";

static const char* towerClusterBranchList="c/I:lkr:csi:ncells:e/F:x:y:z:vx:vy:vz:theta:phi:rho"
":dtheta:dphi:drho:thetastr:phistr:qlty/I:ncellsbad:str_ncls:str_ntrk:dc_ntrk:emc_ncls";

static const char* stripClusterBranchList="c/I:ncells:type_fi:type_z:nl:energy/F:rho:x";

static const char* stripTrackBranchList="t/I:nvec:x0/F:y0:z0:vx:vy:vz:t1:t2";

extern void copy(EMCBranch*);
//extern void copy(EMCBranch* bemc,int t);
extern void copy(TowerClusterBranch* btcls,int c);
extern void copy(StripClusterBranch* bstrcls,int c);
extern void copy(StripTrackBranch* bstrtrk,int t);

#endif
