#include <unistd.h>
#include <libgen.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <set>
#include <list>
#include <sys/time.h>
#include <csignal>

#include "ReadNat/re_def.h"
#include "ReadNat/rr_def.h"
#include "ReadNat/read_nat_c.h"
#include "KDB/kdb.h"
#include "VDDCRec/kdcswitches.h"
#include "VDDCRec/kdcvd.h"
#include "VDDCRec/ktracks.h"
#include "VDDCRec/kglobparam.h"
#ifdef DCPID
#include "KrdEdxPId/KrdEdxPId.hh"
#endif
#include "KrToF/tof_system.h"
#include "KEmcRec/emc_system.h"
#include "KrAtc/atcrec.h"
#include "KrAtc/atc_sim.h"
#include "KrAtc/AtcRec.hh"
#include "KrMu/mu_system.h"
#include "KrMu/mu_res.h"
#include "KrVDDCMu/dcmu.h"
#include "KDisplay/kdisplay_event.h"

#include "runtypes.h"
#include "mknatfilelist.h"
#include "kf_event_stat.h"
#include "kframework.h"

//Reconstruction record unpack routine
extern "C" void kedr_unpack_();

using namespace std;

//Signal handler replacing flag. Set by kf_install_signal_handler.
static bool install_signal_handler=false;

//System's use on/off flags. Changed with kf_use().
static bool use_dc=false,
			use_tof=false,
			use_atc=false,
			use_emc=false,
			use_mu=false;

//List of system's idetificators specifying an order of
//system's reconstruction and user's rejection calls. Changed by kf_use().
static list<int> sys_list;

//Cosmic processing flag. Set with kf_cosmic().
static int cosmic=0;

int MCCalibRunNumber=19862;   //1/06/2017

int kdisp_ev=0;              //17/04/2017

//Out NAT mode: 0 - single file (mSingleFile in ReadNat), 1 - one file per run (mRunFiles).
//Set by kf_set_out_nat().
static int out_nat_mode=1;

//NAT file name (in case of single out file) or NAT path prefix (a file per run)
//to store skimmed files. Set by kf_set_out_nat().
static const char* out_nat=(char*)NULL;

//Obtain reconstruction information for VDDC and EMC from file.
//Set by kf_reco_from_file().
static bool reco_from_file=false;

//Display every event in KDisplay. Set with kf_display_all().
static bool display_all=false;

//Minimum interval in seconds between event display updates. Set with kf_display_update().
static float display_update=1.0;

//Process only events in exclude list
static bool process_only=false;

//If set the program pauses processing until the Enter key is pressed
static bool wait_user=false;

//Job event statistics
static kf_event_stat runStat, jobStat;

//User routines registered by kf_set_before_run and kf_set_after_run,
//called by kf_process before and after processing each run, respectively.
static int (*before_run)(int)=0;    //called before run
static int (*after_run)(int)=0;     //called after run

//User selection routines, registered with kf_register_selection.
//If event is rejected routine must return non-zero code
//corresponding to a cut numbering index (from 1) in a vector above,
//otherwise zero must be returned.
static int (*pre_reject)()=0;    //rejection before system reconstructions
static int (*reject_dc)()=0;     //VDDC event selection
static int (*reject_tof)()=0;    //TOF event selection
static int (*reject_emc)()=0;    //EMC event selection
static int (*reject_atc)()=0;    //ATC event selection
static int (*reject_mu)()=0;     //MU event selection
static int (*post_reject)()=0;   //rejection after system reconstructions

//User analysis routines, registered with kf_register_analysis().
//Return 1 if an event is wanted to be drawn.
static int  (*analyse_event)()=0;

//Modify header flag. REDE energy read from DB will be written to header.
static bool modify_header=0;

//Set custom signal handler for printing position in the input event stream.
void kf_install_signal_handler(int onoff)
{ install_signal_handler=onoff; }

//Turn on/off a system's reconstruction in the analysis. The order of kf_use calls
//determine the order of calling systems' reconstruction and rejections.
void kf_onoff(int sys,int on)
{
	if( on )
		sys_list.push_back(sys);
	else
		sys_list.remove(sys);

	if     ( sys==KF_VDDC_SYSTEM ) use_dc =on;
	else if( sys==KF_TOF_SYSTEM )  use_tof=on;
	else if( sys==KF_EMC_SYSTEM )  use_emc=on;
	else if( sys==KF_ATC_SYSTEM )  use_atc=on;
	else if( sys==KF_MU_SYSTEM )   use_mu=on;
}

//Turn on/off a systems' reconstruction in the analysis.
//Set default order of systems' calls.
void kf_use(int sysmask)
{
	use_dc=false;
	use_tof=false;
	use_atc=false;
	use_emc=false;
	use_mu=false;

	sys_list.clear();

	if( sysmask&KF_VDDC_SYSTEM ) {
		use_dc=true;
		sys_list.push_back(KF_VDDC_SYSTEM);
	}
	if( sysmask&KF_TOF_SYSTEM ) {
		use_tof=true;
		sys_list.push_back(KF_TOF_SYSTEM);
	}
	if( sysmask&KF_EMC_SYSTEM ) {
		use_emc=true;
		sys_list.push_back(KF_EMC_SYSTEM);
	}
	if( sysmask&KF_ATC_SYSTEM ) {
		use_atc=true;
		sys_list.push_back(KF_ATC_SYSTEM);
	}
	if( sysmask&KF_MU_SYSTEM ) {
		use_mu=true;
		sys_list.push_back(KF_MU_SYSTEM);
	}
}

//Set cosmic processing flag.
//cosm==0 - beam run, default
//cosm>0 - cosmic run
//cosm<0 - auto cosmic run determination
void kf_cosmic(int cosm)
{ cosmic=cosm; }

void kf_kdisplay_cut(int kdisp_ev1)
{ kdisp_ev=kdisp_ev1; }

void kf_MCCalibRunNumber(int MCCalibRunNumber1)
{ MCCalibRunNumber=MCCalibRunNumber1; }

//Set out NAT file name and write mode.
void kf_set_out_nat(const char* fn,int mode)
{ out_nat=fn; out_nat_mode=mode; }

//Set flag to read VDDC&EMC reconstruction records from NAT file.
void kf_reco_from_file(int on)
{ reco_from_file=on; }

//Manage all display settings.
void kf_set_display(int all,int wait,float interval)
{
	display_all=all;
	wait_user=wait;
	display_update=interval;
}

//Display every processed event.
void kf_display_all(int choice)
{ display_all=choice; }

//Set event display update interval in seconds. 1.0 sec by default.
void kf_display_update(float interval)
{ display_update = interval>0?interval:0; }

//To be called when there's a need to synchronize output with kdisplay_event.
void kf_display_sync()
{
	if( display_update!=0 ) {
		//Wait remaining time and then draw the event
		struct itimerval cur;
		getitimer(ITIMER_REAL,&cur);
		if( cur.it_value.tv_sec>0 || cur.it_value.tv_usec>0 )
			pause(); //wait until SIGALRM is caught
	}
}

//If wait is true next event won't be drawn until user presses the Enter key.
void kf_display_wait_user(int wait)
{ wait_user=wait; }

//Set/unset process_only flag, if set then events in exclude list will be processed exclusively.
//Each event from list is to be drawn and debugging information is to be shown.
void kf_process_only(int only)
{ process_only=only; }

//Add a cut description. Do not register a cut with index 0,
//it is reserved for reconstruction error cut in case of system's cuts or
//left empty in case of PRE/POST cuts.
void kf_add_cut(int sel_id,uint cut_id,const char* cut)
{
	static const char* sel_names[7]={"Pre-rejection","VD&DC rejection",
	"ToF rejection","EMC rejection","ATC rejection","MU rejection","Post-rejection"};

	if( sel_id<KF_PRE_SEL || sel_id>KF_POST_SEL  ) {
		cerr<<"Invalid selection class "<<sel_id<<endl;
		return;
	}
	if( !cut ) {
		cerr<<"Invalid cut given."<<endl;
		return;
	}
	if( !jobStat.get_class(sel_id) ) {
		jobStat.add_class(sel_id,sel_names[sel_id]);
		runStat.add_class(sel_id,sel_names[sel_id]);
	}
	jobStat.add_cut(sel_id,cut_id,cut);
	runStat.add_cut(sel_id,cut_id,cut);
}

//Set routine called before processing each run.
void kf_set_before_run(int (*routine)(int))
{ before_run=routine; }

//Set routine called after processing each run.
void kf_set_after_run(int (*routine)(int))
{ after_run=routine; }

//Register rejection routine.
//Called just after corresponding system's reconstruction
//or before/after all system's calls.
void kf_register_selection(int sel_id,int (*reject)())
{
	if     ( sel_id==KF_PRE_SEL )  pre_reject=reject;
	else if( sel_id==KF_VDDC_SEL ) reject_dc=reject;
	else if( sel_id==KF_TOF_SEL )  reject_tof=reject;
	else if( sel_id==KF_EMC_SEL )  reject_emc=reject;
	else if( sel_id==KF_ATC_SEL )  reject_atc=reject;
	else if( sel_id==KF_MU_SEL )   reject_mu=reject;
	else if( sel_id==KF_POST_SEL ) post_reject=reject;
	else cerr<<"Unknown selection ID "<<sel_id<<endl;
}

//Register user's event analysis routine
void kf_register_analysis(int (*event)())
{
	analyse_event=event;
}

//Catch various signals, print location in input data.
static void sighandler(int sig)
{
	//Ignore SIGALRM because it is sent by timer for drawing
	if( sig==SIGALRM ) return;

	const char* desc=0;
	bool abrt=false;
	switch( sig ) {
		case SIGHUP:  desc="Hangup"; break;
		case SIGINT:  desc="Interrupted"; break;
		case SIGQUIT: desc="Quited"; break;
		case SIGILL:  desc="Illegal instruction"; break;
		case SIGFPE:  desc="Floating point exception"; break;
		case SIGSEGV: desc="Segmentation fault"; abrt=true; break;
		case SIGPIPE: desc="Broken pipe"; break;
		case SIGTERM: desc="Terminated"; break;
		default:      desc="Unknown signal";
	}
	cerr<<desc<<" in run "<<kedrraw_.Header.RunNumber<<" at event "
		<<kedrraw_.Header.Number<<endl;

	if( abrt ) abort();

	exit(EXIT_FAILURE);
}

//Set modify event header
void kf_modify_header(int choice)
{ modify_header=choice; }

static int last_event=0;
static float lum_e=0, lum_p=0;
//static float beam_energy=0;
float beam_energy=0;
static float dbbeam_energy=0;

//Get last valid event and RDM energy for the run from DB
static bool get_run_data(int run)                                         //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
{
	static const int lum_table_id=2007, lum_table_length=7;
	static const int e_table_id=2119, e_table_length=16;

	bool rc=false;
	last_event=0;
	beam_energy=0;

	int buflum[lum_table_length];
	int bufenergy[e_table_length];

	KDBconn *conn = kdb_open();

	if (!conn) {
		cerr<<"Can not connect to database"<<endl;
		return false;
	}
	if( kdb_read_for_run(conn,lum_table_id,run,buflum,lum_table_length) ) {
		lum_p=buflum[0]*1E-3;
		lum_e=buflum[1]*1E-3;
		last_event=buflum[2];

	//cout<<"1-->"<<buflum[0]<<"  2-->"<<buflum[1]<<" 3-->"<<buflum[2]<<" 4-->"<<buflum[3]<<" 5-->"<<buflum[4]<<" 6-->"<<buflum[5]<<" 7-->"<<buflum[6]<<endl;
	//	if(run==19654)
	//	{ last_event=307251; }
		rc=true;
	}
	//kdb_setver(conn,0);
	if( kdb_read_for_run(conn,e_table_id,run,bufenergy,e_table_length) ) {
		beam_energy=bufenergy[1]*1E-6;
		rc=true;
	}

        /*
	void get_run_energy(int run, double *en, double *en_err) {
	    int array[8];
	    KdDbConn* conn;
	    int id;

	    conn = kd_db_open(NULL);
	    id = kd_db_get_id("runenergy",conn);
	    if (kd_db_get_for_run_int(id, run, conn, array, 8)) {
		*en = array[1]/1e6;
		*en_err = array[2]/1e6;
	    }
	    kd_db_close(conn);
        */



        //===========================================================
	int RunStatus=1;                                       //0-information in the start run; 1-information in the end run; 2 - information in the pause run
	KDBruninfo runinfo;
	if( kdb_run_get_info(conn,run,RunStatus,&runinfo) ) {
        last_event=runinfo.nread;
        dbbeam_energy=runinfo.E_setup*1E-3;
	cout<<" kdb_run_get_info   ===>>   E_setup="<<runinfo.E_setup*1E-3<<endl;
	cout<<" kdb_run_get_info   ===>>   NREAD="<<runinfo.nread<<endl;

	rc=true;
	}
        //===========================================================
	kdb_close(conn);

	if( rc )
		cout<<" RUN DB INFO: LastEvent="<<last_event<<", LumP="<<lum_p<<", LumE="<<lum_e<<", Energy="<<beam_energy<<endl;
	else
		cerr<<" No run info in lmrunscan for this run!"<<endl;

	return rc;
}

//Finish processing the run

static void finish_run(int run)
{
	static clock_t clock_prev;

	if( run==0 ) {
		clock_prev=clock();
		return;
	}

	//Call user routine
	if( after_run ) after_run(run);

	if( runStat.total ) {
		//Print down statistics for the run.
		cout<<"================================================================\n"
			<<"                RUN "<<run<<" SUMMARY"<<endl;
		double elapsed=double(clock()-clock_prev)/CLOCKS_PER_SEC;
		runStat.set_period(elapsed);
		runStat.print();
		cout<<"================================================================"<<endl;
		jobStat+=runStat;
		runStat.reset();
		clock_prev=clock();
	}
}

//Internal call of analysis processing
static int process(flist_exev_t& filelist, long long nevents)
{
	if( filelist.empty() ) return -1;

	int ostr=0;
	if( out_nat ) {
		ostr=write_nat(out_nat,out_nat_mode); //open output stream
		if( ostr==0 ) {
			cerr<<"Error opening output stream with name "<<out_nat<<" and mode "<<out_nat_mode<<endl;
			return -1;
		}
		nat_filter(ostr,fAll);
	}

	char pwd[100];
	getcwd(pwd,100);
	cout<<"Current directory is "<<pwd<<endl;

	if( install_signal_handler ) {
		//Set the custom signal handler for some terminating signals
		signal(SIGHUP,sighandler);
		signal(SIGINT,sighandler);
		signal(SIGQUIT,sighandler);
		signal(SIGILL,sighandler);
		signal(SIGFPE,sighandler);
		signal(SIGSEGV,sighandler);
		signal(SIGPIPE,sighandler);
		signal(SIGTERM,sighandler);
	}

	//Ignore SIGALRM because it is sent by timer for drawing
	signal(SIGALRM,sighandler);

	//Initialization of VD&DC
	if( use_dc ) {
		kf_add_cut(KF_VDDC_SEL,0,"reconstruction error");

		//Set flag for DC if reconstruction of cosmic tracks is required
		if( cosmic>0 )
			kdcvdcosmic();
		else if( cosmic==0 )
			kdcvdnocosmic();
	}

	//Initialization of ToF
	if( use_tof ) {
		kf_add_cut(KF_TOF_SEL,0,"reconstruction error");
	}

	//Initialization of EM calorimeters
	bool callEMCexplicitly=false;
	if( use_emc ) {
		if( !use_dc || use_dc && kdcswitches_.KemcAllowed<-1 ) {
			callEMCexplicitly=true;
			emc_init();
		}

		//turn on debugging in EMC for exclusive events
		if( process_only ) semc_cards.EMC_DEBUG=2;

		kf_add_cut(KF_EMC_SEL,0,"reconstruction error");
	}

	//Initialization of ATC
	if( use_atc ) {
		atc_init(); //Unneccessary. Done in atc_event().

		//turn on debugging in ATC for exclusive events
//		if( process_only ) atcRec->debug=4;                            //!!!!!!!!!!!!!!!!!!!!!!!!!!!   - debug information for earch counter atc - ampl, type, npe ...

		kf_add_cut(KF_ATC_SEL,0,"reconstruction error");
	}

	//Initialization of MU
	if( use_mu ) {
		mu_default_init(1);

		kf_add_cut(KF_MU_SEL,0,"reconstruction error");
	}

	//Get references to selection classes statistics
	kf_cut_class *pre_stat =runStat.get_class(KF_PRE_SEL),
				 *dc_stat  =runStat.get_class(KF_VDDC_SEL),
				 *tof_stat =runStat.get_class(KF_TOF_SEL),
				 *emc_stat =runStat.get_class(KF_EMC_SEL),
				 *atc_stat =runStat.get_class(KF_ATC_SEL),
				 *mu_stat  =runStat.get_class(KF_MU_SEL),
				 *post_stat=runStat.get_class(KF_POST_SEL);

	//Check that selection routines were registered by user as indicated by cuts
	if( pre_stat && !pre_reject ) {
		cerr<<"Cuts are declared but not defined for pre-rejection"<<endl;
		return 1;
	}
	if( dc_stat && dc_stat->size()>1 && !reject_dc ) {
		cerr<<"Cuts are declared but not defined for VDDC rejection"<<endl;
		return 1;
	}
	if( tof_stat &&	tof_stat->size()>1 && !reject_tof ) {
		cerr<<"Cuts are declared but not defined for ToF rejection"<<endl;
		return 1;
	}
	if( emc_stat &&	emc_stat->size()>1 && !reject_emc ) {
		cerr<<"Cuts are declared but not defined for EMC rejection"<<endl;
		return 1;
	}
	if( atc_stat &&	atc_stat->size()>1 && !reject_atc ) {
		cerr<<"Cuts are declared but not defined for ATC rejection"<<endl;
		return 1;
	}
	if( post_stat && !post_reject ) {
		cerr<<"Cuts are declared but not defined for post-rejection"<<endl;
		return 1;
	}

	//Value of display timer. Interval is null so that timer stops when expired.
	const struct itimerval itimer={{0,0},{int(display_update),int(1e6*(display_update-int(display_update)))}};

	//an event drawn flag
	bool drawn=false;

	//track of runs processed
	set<int> runs;

	int daqRun=0;

	long long cur_event=0;
	int file_event=0;

	int next_exrun=0, next_exev=0;

	bool stop_job=false;

	bool simulation=false;

	flist_exev_t::iterator item=filelist.begin(), lastitem=filelist.end();

	//Start loop on files
	for(; item!=lastitem && !stop_job; item++)
	{
		const char* natFile=(item->first).c_str();

		if( !open_nat(natFile) ) {                                   //открыть nat файл
			cerr<<"Error opening NAT file "<<natFile<<endl;
			continue;
		}
		cout<<"----------------------------------------------------------------"<<endl;
		cout<<"File "<<natFile<<" opened"<<endl;
		nat_all_records(0); // get only event records   //выдавать сформированные события

		file_event=0;

		//copy excluding events list instead of referencing, for further modifications
		eventlist_t exev=item->second;

		if( !exev.empty() ) {
			cout<<"Events to "<<(process_only?"process":"exclude")<<": "<<exev_to_string(exev)<<endl;
			next_exrun=(*exev.begin())/1000000;
			next_exev=(*exev.begin())%1000000;
		} else if( process_only ) {
			cout<<"No events specified to process exclusively: assume all the events."<<endl;
		}

		//Start loop on events in the file
		while( next_event() )  //прочитать следующую запись (событие) из файла, возвращает нуль по концу файла или при ошибке
		{
			//required number of events processed
			if( nevents>0 && cur_event==nevents ) {
				stop_job=true;
				break;
			}

			//last valid event in the run reached
			if( !simulation )
				if( last_event>0 && kedrraw_.Header.Number>last_event ) break;

			cur_event++;
			file_event++;

			//check for excluding or exclusive processing
			if( !exev.empty() ) {
				int &number_to_check=next_exrun?kedrraw_.Header.Number:file_event;
				if( process_only ) {
					string eventstring=exev_to_string(next_exrun,next_exev);
					cout<<"Skip to the "<<eventstring<<" ..."<<endl;
					int eof=0;
					do {
						if( kedrraw_.Header.RunNumber==next_exrun || next_exrun<=1 ) {
							if( number_to_check==next_exev )
								break;
							if( number_to_check>next_exev ) {
								cout<<" "<<eventstring<<" not found"<<endl;
								exev.erase(exev.begin());
								if( exev.empty() ) break;
								next_exrun=(*exev.begin())/1000000;
								next_exev=(*exev.begin())%1000000;
							}
						}
						eof=!next_event();
						cur_event++;
						file_event++;
					} while( !eof );
					if( eof ) { //end of file encountered
						cout<<"End of file, following events not found: "<<exev_to_string(exev)<<endl;
						break;
					}
					if( exev.empty() ) {
						cout<<"Exlusive event list exhausted - drop the file"<<endl;
						break;
					}
					exev.erase(exev.begin());
					if( !exev.empty() ) {
						next_exrun=(*exev.begin())/1000000;
						next_exev=(*exev.begin())%1000000;
					}
				} else {
					if( (kedrraw_.Header.RunNumber==next_exrun || next_exrun<=1) &&
						number_to_check==next_exev ) {
						cout<<"Event "<<next_exev<<" was requested to skip"<<endl;
						exev.erase(exev.begin());
						if( !exev.empty() ) {
							next_exrun=(*exev.begin())/1000000;
							next_exev=(*exev.begin())%1000000;
						}
						continue;
					}
				}
			} else { // exev.empty() == true
				//if exclusive list exhausted drop the file
				if( process_only && !item->second.empty() ) {
					cout<<"Exlusive event list exhausted - drop the file"<<endl;
					break;
				}
			}

			if( kedrrun_cb_.Header.RunType == KRT_SIM ) {
				//Simulation file
				//cout<<" Processing simulation file"<<endl;

				simulation=true;
                                                                  //вызовы делаются перед началом чтения файла и вызовов подпрограмм реконструкции
				if( use_dc ) {
                                       // XTKey=0;   //05/06/2017 - попробовал
				       // kdcsimxt();               //моделирование X(t) c "бесконечно хорошим" пространственным разрешением"
				//	kdcsimsigma();            //при добавлении моделируется экспериментальное пространственное разрешение
				//	kdcsimsysterr();          //при добавлении моделируется систематическая ошибка калибровки

					ksimreal(1,MCCalibRunNumber,MCCalibRunNumber);     //1/06/2017

				}

				if( use_emc && callEMCexplicitly ) emc_run(1,0);

			} else if( kedrraw_.Header.RunNumber!=daqRun ) {
				//Experiment file

				//Do necessary job after last run processing
				finish_run(daqRun);

				daqRun=kedrraw_.Header.RunNumber;

				if( runs.find(daqRun)!=runs.end() ) { //this run has been already processed
					cerr<<"Skipping this run "<<daqRun<<": already met this number"<<endl;
					break;
				}

				runs.insert(daqRun);
				cout<<"             BEGIN RUN "<<daqRun<<endl;

				get_run_data(daqRun);

				if( use_dc ) {
					//automatic cosmic run determination
					if( cosmic<0 ) {
						if( kedrrun_cb_.Header.RunType==KRT_COSM ||
							kedrrun_cb_.Header.RunType==KRT_COSM_A ) {
							kdcvdcosmic();
						} else {
							kdcvdnocosmic();
						}
					}
#ifdef DCPID
					dcdedxpidinit(&daqRun);
#endif
				}
				if( use_mu ) dcmu_init(daqRun,0);

				if( use_emc && callEMCexplicitly ) emc_run(daqRun,0);

				//Note: DC, ToF, ATC reconstructions do initializations for runs
				// by themselves in per-event calls

				//Call user's run-initialization routine
				if( before_run )
					if( before_run(daqRun)!=0 ) //skip this run
						break;
			}

			runStat.total++;

			if( runStat.total%1000==0 )
				cout<<setw(6)<<"event "<<runStat.total<<endl;

			//read reconstruction records from file
			if( !simulation && reco_from_file )
				kedr_unpack_();

			if( modify_header && beam_energy>0 )
				kedrraw_.Header.Energy=(int)rint(beam_energy);

			int irej=0;

			//Apply pre-event rejections
			if( pre_reject ) {
				irej=pre_reject();
				if( irej>0 ) {
					if( !pre_stat ) pre_stat=&runStat[KF_PRE_SEL];
					(*pre_stat)[irej]++;
					continue;
				}
			}

			int res=0;
			list<int>::const_iterator iter=sys_list.begin();

			//loop on registered systems
			for( ; iter!=sys_list.end(); iter++ )
			{
				//VD&DC event reconstruction
				if( *iter==KF_VDDC_SYSTEM ) {
					if( !reco_from_file ) {
						//due to pre-rejection kdcenum_.EvNum isn't equal to event number
						kdcenum_.EvNum=cur_event-1;
						kdcvdrec(0,&res);
						if( res<0 ) {
							(*dc_stat)[0]++;
							break;
						}
#ifdef DCPID
						if( !simulation ) pidevent();
#endif
						res=0; //zero res if everything is Ok
					}
					//User's VDDC event rejection
					if( reject_dc ) {
						irej=reject_dc();
						if( irej>0 ) {
							(*dc_stat)[irej]++;
							break;
						}
					}
				}

				//ToF event selection
				if( *iter==KF_TOF_SYSTEM ) {
					res=tof_event();
					if( res ) {
						(*tof_stat)[0]++;
						break;
					}
					//User's ToF event rejection
					if( reject_tof ) {
						irej=reject_tof();
						if( irej>0 ) {
							(*tof_stat)[irej]++;
							break;
						}
					}
				}

				//EMC event selection
				if( *iter==KF_EMC_SYSTEM ) {
					if( !reco_from_file && callEMCexplicitly ) {
						res=emc_event();
						if( res ) {
							(*emc_stat)[0]++;
							break;
						}
					}
					//User's EMC event rejection
					if( reject_emc ) {
						irej=reject_emc();
						if( irej>0 ) {
							(*emc_stat)[irej]++;
							break;
						}
					}
				}

				//ATC event recontruction
				if( *iter==KF_ATC_SYSTEM ) {
					res=atc_event();
					if( res ) {
						(*atc_stat)[0]++;
						break;
					}
					//User's ATC event rejection
					if( reject_atc ) {
						irej=reject_atc();
						if( irej>0 ) {
							(*atc_stat)[irej]++;
							break;
						}
					}
				}

				//MU event recontruction
				if( *iter==KF_MU_SYSTEM ) {
					if( use_dc && simulation )
						dcmu_simrun(1);
					res=mu_next_event_good();
					if( res<0 ) {
						(*mu_stat)[0]++;
						break;
					}
					if( use_dc )
						dcmu_next_tracks();
					res=0;
					//User's MU event rejection
					if( reject_mu ) {
						irej=reject_mu();
						if( irej>0 ) {
							(*mu_stat)[irej]++;
							break;
						}
					}
				}

			} //end of loop on systems

			//Event was wrongly reconstructed or rejected by one of systems
			if( res || irej ) continue;

			//Apply post-event rejections
			if( post_reject ) {
				irej=post_reject();
				if( irej>0 ) {
					if( !post_stat ) post_stat=&runStat[KF_POST_SEL];
					(*post_stat)[irej]++;
					continue;
				}
			}

			runStat.passed++;

			res=0;

			//Call user's analysis for event
			if( analyse_event ) {
				res=analyse_event();
			}

//			if( res==1 || display_all ) {
//                        cout<<"kdcenum_.EvNum="<<kdcenum_.EvNum%1000<<endl;
			if( kdcenum_.EvNum>=kdisp_ev && (res==1 || display_all) ) {                            //17/04/2018
				if( drawn ) {
					if( wait_user )
						getchar(); //Wait user
					else
						kf_display_sync(); //Wait until delay timer expires
				}
				//Draw this event
				kdisplay_event();
				//Reset timer
				if( display_update!=0 && !wait_user ) setitimer(ITIMER_REAL,&itimer,0);
				drawn=true;
			}

			//Write event to NAT file if requested
			if( ostr && res>=0 ) {
				put_event(ostr);
				runStat.written++;
			}

		} //loop on events

		close_nat();

		finish_run(daqRun);
	} //loop on files

	if( drawn ) {
		if( wait_user )
			getchar();
		else
			kf_display_sync(); //Wait until delay timer expires
	//	kdisplay_stop();                                                                          !!!!!!!!!!!!!!!!!!!!!!-закоментил
	}

	if( runs.size()>1 ) {
		cout<<"<<<<<<<<<<<<<<<<<<<<<<<<<< JOB SUMMARY >>>>>>>>>>>>>>>>>>>>>>>>>>\n"
			<<runs.size()<<" runs processed:";
		for(set<int>::iterator it=runs.begin(); it!=runs.end(); it++)
			cout<<" "<<*it;
		cout<<"\n";
		jobStat.print();
		cout<<"<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"<<endl;
	}

	//free EMC reconstruction
	if( use_emc ) emc_close();

	//free ATC reconstruction
	if( use_atc ) atc_stop();

	//free MU reconstruction
	if( use_mu ) dcmu_stop();

	return 0;
}

//Main call of analysis job with string arguments.
int kf_process(int nfiles, char* files[], long long nevents)
{
	flist_exev_t filelist;

	int nf=mknatfilelist(nfiles,files,filelist,true);

	int rc=process(filelist,nevents);

	return rc;
}

//Main call of analysis job with run arguments.
int kf_process_runs(int nruns, int runs[], long long nevents)
{
	flist_exev_t filelist;

	int nf=mknatfilelist(nruns,runs,filelist,true);

	int rc=process(filelist,nevents);

	return rc;
}

