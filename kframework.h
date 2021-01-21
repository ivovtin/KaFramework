#ifndef KFRAMEWORK_H
# define KFRAMEWORK_H

# include <sys/types.h>

/*KEDR system identificators*/
enum kf_systems_t {
	KF_VDDC_SYSTEM= 1<<0,
	KF_TOF_SYSTEM = 1<<1,
	KF_EMC_SYSTEM = 1<<2,
	KF_ATC_SYSTEM = 1<<3,
	KF_MU_SYSTEM  = 1<<4,
	KF_ALL_SYSTEMS= 0x1f
};

/*Selection ID constants*/
enum kf_selections_t {
    KF_PRE_SEL,
	KF_VDDC_SEL,
	KF_TOF_SEL,
	KF_EMC_SEL,
	KF_ATC_SEL,
	KF_MU_SEL,
	KF_POST_SEL
};

extern float beam_energy;

# ifdef __cplusplus
extern "C" {
# endif

/*Insall custom signal handler for printing position in the input event stream
  when program stopped by a terminating signal.*/
extern void kf_install_signal_handler(int onoff);

/*Turn on/off a system's reconstruction in the analysis. The order of kf_use calls
  determine the order of calling systems' reconstruction and rejections.*/
extern void kf_onoff(int sys,int on=1);
/*Turn on/off a systems' reconstruction in the analysis.
  Set default order of systems' calls.*/
extern void kf_use(int sysmask);

/*Set cosmic processing flag.
  cosm==0 - beam run, default
  cosm >0 - cosmic run,
  cosm <0 - auto cosmic run determination.*/
extern void kf_cosmic(int cosm);

extern void kf_MCCalibRunNumber(bool simOn,int MCCalibRunNumber1,int MCCalibRunNumber2,int NsimRate1,float scale1,float ascale1,float zscale1);
extern void kf_kdisplay_cut(int kdisp_ev1);

/*Set out NAT file name and write mode.
Write mode: 0 - single file (mSingleFile in ReadNat), 1 - one file per run (mRunFiles).*/
extern void kf_set_out_nat(const char* fn,int mode=1);

/*Set flag to read VDDC&EMC reconstruction records from NAT file.*/
extern void kf_reco_from_file(int on=1);

/*Manage all display settings.
  all      - if =1 display all events,
  wait     - if =1 wait for input from user before displayng next event,
  interval - if wait==0 specifies minimum interval in seconds between subsequent display updates.*/
extern void kf_set_display(int all,int wait,float interval=1.0);

/*Display every processed event.*/
extern void kf_display_all(int choice=1);

/*Set event display update interval in seconds. 1.0 sec by default.*/
extern void kf_display_update(float interval);

/*To be called when user wishes to synchronize his output with kdisplay_event*/
extern void kf_display_sync();

/*If wait is true next event won't be drawn until user presses the Enter key.*/
extern void kf_display_wait_user(int wait=1);

/*Set/unset process_only flag, if set then events in exclude list will be processed exclusively.
  Each event from list is to be drawn and debugging information is to be shown.*/
extern void kf_process_only(int only=1);

/*Add description of a cut for a system.*/
extern void kf_add_cut(int sel_id,uint i,const char* cut);

/*Set routine called before processing each run.*/
extern void kf_set_before_run(int (*routine)(int));

/*Set routine called after processing each run.*/
extern void kf_set_after_run(int (*routine)(int));

/*Register rejection routine for a system.
  It's called immediately after system's reconstruction.*/
extern void kf_register_selection(int sel_id,int (*select)());

/*Register user's event analysis routine.
  Routine must return 1 if events wanted to be shown by kdisplay_event.*/
extern void kf_register_analysis(int (*event)());

/*Set modify event header flag. REDE beam energy is written to energy par in header,*/
extern void kf_modify_header(int choice=1);

/*Main call of analysis job with string arguments.*/
extern int kf_process(int nfiles,char* files[],long long nevents=0);

/*Main call of analysis job with run arguments.*/
extern int kf_process_runs(int nruns,int runs[],long long nevents=0);

# ifdef __cplusplus
}
# endif

# include "mknatfilelist.h"

/*Set directory to look up NAT-files by run number*/
inline void kf_set_nat_dir(const char* dir) { mknatfl_setdir(dir); }

/*Set pattern to look up NAT-files by run number*/
inline void kf_set_nat_pattern(const char* patt) { mknatfl_setpatt(patt); }

#endif
