#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <regex.h>
#include <list>
#include "mknatfilelist.h"

using namespace std;

//static const char* defrundirs[]={"/runs/runs","/space/data2/KEDR_RUNS/runs",
//"/space_old/data2/KEDR_RUNS/runs",(char*)0};
static const char* defrundirs[]={"/runs/runs","/space/runs",
"/space_old/data2/KEDR_RUNS/runs",(char*)0};

static const char* user_natdir=0;
static const char* user_natpatt=0;

static regex_t preg_nat, preg_runnum, preg_range, preg_events, preg_blank;

static size_t getfilelist(const list<string>& arglist, flist_exev_t& filelist,bool verbose);
static bool   check_runfile(int run,string& natfile);

int mknatfilelist(int nargs,char* args[], flist_exev_t& filelist,bool verbose)
{
	static char errbuf[100];

	regex_t *preg;
	int errcode=0;
	errcode=regcomp(&preg_nat,"^.+\\.[nd]at(\\.bz2)?",REG_EXTENDED|REG_ICASE);
	if( errcode ) { preg=&preg_nat; goto ERROR; }
	errcode=regcomp(&preg_runnum,"^ *[0-9]+",REG_EXTENDED);
	if( errcode ) { preg=&preg_runnum; goto ERROR; }
	errcode=regcomp(&preg_range,"^ *([0-9]+)-([0-9]+) *$",REG_EXTENDED);
	if( errcode ) { preg=&preg_range; goto ERROR; }
	errcode=regcomp(&preg_events,":([0-9]*:?[0-9]+[, ]*)+$",REG_EXTENDED);
	if( errcode ) { preg=&preg_events; goto ERROR; }
	errcode=regcomp(&preg_blank,"^[:blank:]*$",REG_EXTENDED);
	if( errcode ) { preg=&preg_blank; goto ERROR; }

	if( errcode ) {
ERROR:
		regerror(errcode,preg,errbuf,100);
		cerr<<__func__<<" internal error: "<<errbuf<<endl;
		return -1;
	}

	filelist.clear();

	list<string> arglist;

	for(int i=0; i<nargs; i++)
		arglist.push_back(args[i]);

	if( verbose ) cout<<"Making up NAT file list..."<<endl;

	int nentries=getfilelist(arglist,filelist,verbose);

	if( verbose ) cout<<nentries<<" files assigned"<<endl;

	return nentries;
}

int mknatfilelist(int nruns, int runs[], flist_exev_t& filelist,bool verbose)
{
	filelist.clear();

	if( verbose ) cout<<"Making up run file list..."<<endl;

	string natfile;
	for(int i=0; i<nruns; i++) {
		if( check_runfile(runs[i],natfile) ) {
			filelist.insert(flist_exev_t::value_type(natfile,eventlist_t()));
			if( verbose ) cout<<' '<<natfile;
		} else {
			cerr<<__func__<<": No file for run "<<runs[i]<<" exists"<<endl;
			continue;
		}
	}

	int nentries=filelist.size();

	if( verbose ) cout<<nentries<<" files assigned"<<endl;

	return nentries;
}

void mknatfl_setdir(const char* dir) { user_natdir=dir; }

void mknatfl_setpatt(const char* patt) { user_natpatt=patt; }

static bool check_runfile(int run,string& natfile)
{
	char srun[10];
	sprintf(srun,"%06d",run);

	if( user_natdir ) {
		natfile=string(user_natdir)+"/daq"+srun+".nat";
	} else if( user_natpatt ) {
		natfile=user_natpatt;
		natfile.replace(natfile.find_first_of('*'),1,srun);
		if( natfile.substr(natfile.length()-4)==".bz2" ) natfile.erase(natfile.length()-4);
	} else { //use default dirs
		int i=0;
		while( defrundirs[i]!=0 ) {
			natfile=string(defrundirs[i])+"/daq"+srun+".nat";
			if( access(natfile.c_str(),R_OK)==0 || access(string(natfile+".bz2").c_str(),R_OK)==0 )
				return true;
			i++;
		}
		return false;
	}

	if( access(natfile.c_str(),R_OK)==0 || access(string(natfile+".bz2").c_str(),R_OK)==0 )
		return true;

	return false;
}

static size_t getfilelist(const list<string>& arglist, flist_exev_t& filelist,bool verbose)
{
	static int level=0;

	regmatch_t pmatch[3];

	level++;
	string indent(level,' ');

	//check correctness of user_natpatt
	if( user_natpatt ) {
		char* astloc=strchr(user_natpatt,'*');
		if( astloc==0 ) {
			cerr<<__func__<<": Wrong format of filename pattern - no asteric"<<endl;
			return 0;
		}
	}

	string natfile;

	list<string>::const_iterator iter=arglist.begin(), last=arglist.end();

	for( ; iter!=last; iter++ ) {
		//take argument without excluded events list
		string arg(*iter,0,iter->find_first_of(':'));
		pair<flist_exev_t::iterator,bool> result;

		if( regexec(&preg_nat,arg.c_str(),0,0,0)==0 ) {
			//NAT filename
			if( arg.substr(arg.length()-4)==".bz2" ) arg.erase(arg.length()-4);
			result=filelist.insert(flist_exev_t::value_type(arg,eventlist_t()));
			if( verbose ) cout<<indent<<arg;
		} else if( regexec(&preg_range,arg.c_str(),3,pmatch,0)==0 ) {
			//Run number range
			string srun1(arg,pmatch[1].rm_so,pmatch[1].rm_eo-pmatch[1].rm_so);
			string srun2(arg,pmatch[2].rm_so,pmatch[2].rm_eo-pmatch[2].rm_so);
			int run1=atoi(srun1.c_str()), run2=atoi(srun2.c_str());
			list<string>::const_iterator fmti;
			for(int run=run1; run<=run2; run++) {
				if( check_runfile(run,natfile) ) {
					filelist.insert(flist_exev_t::value_type(natfile,eventlist_t()));
					if( verbose ) cout<<indent<<natfile<<endl;
				}
			}
			continue; //no excluded events list
		} else if( regexec(&preg_runnum,arg.c_str(),0,0,0)==0 ) {
			//Run number
			int run=atoi(arg.c_str());
			if( check_runfile(run,natfile) ) {
				result=filelist.insert(flist_exev_t::value_type(natfile,eventlist_t()));
				if( verbose ) cout<<indent<<natfile;
			} else {
				cerr<<__func__<<": No file for run "<<run<<" exist"<<endl;
				continue;
			}
		} else if( access(arg.c_str(),R_OK)==0 ) {
			//Text file with run/file list
			if( verbose ) cout<<"+Reading "<<arg<<" for run list:"<<endl;

			ifstream in(arg.c_str());
			if( !in.is_open() ) {
				cerr<<__func__<<": Can not open file "<<arg<<endl;
				continue;
			}

			string entry;
			list<string> entries;

			while ( 1 ) {
				getline(in,entry);
				if( in.eof() || in.fail() ) break;
				if( regexec(&preg_blank,entry.c_str(),0,0,0)!=0 )
					entries.push_back(entry);
			}

			in.close();

			//call recursively with the file content
			getfilelist(entries,filelist,verbose);

			continue; //no excluded events list
		} else {
			cerr<<__func__<<": Unrecognized input argument "<<arg<<endl;
			continue;
		}

		//Make list of excluded events
		string tail(*iter,arg.length());
		if( regexec(&preg_events,tail.c_str(),1,pmatch,0)!=0 ) {
			if( verbose ) cout<<endl;
			if( tail.length()>1 )
				cerr<<__func__<<": Wrong format of event list "<<tail.substr(1,string::npos)<<endl;
			continue;
		}
		tail.erase(0,1); //remove first column

		if( verbose ) cout<<" supplied events:";

		eventlist_t &exevlst=(*result.first).second;

		string::size_type p=0, lp=0;
		u_int64_t run=0;
		bool isrun=false;

		while( 1 ) {
			p=tail.find_first_not_of(", ",lp);
			if( p==string::npos ) break;

			if( tail[p]==':' ) { run=1; p++; } //hardware event number in current run

			lp=tail.find_first_of(":, ",p);
			if( tail[lp]==':' ) isrun=true; //the word is a run number

			string::size_type len = (lp==string::npos)?lp:lp-p; //length of the word

			u_int64_t num=atoi(string(tail,p,len).c_str());

			if( isrun ) {
				run=num;
				isrun=false;
			} else {
				if( verbose ) {
					cout<<" ";
					if( run==1 ) cout<<":";
					else if( run>1 ) cout<<run<<":";
					cout<<num;
				}
				num+=run*1000000;
				exevlst.insert(num);
			}

			if( lp==string::npos ) break;
			lp++;
		}

		if( verbose ) cout<<endl;
	}

	return filelist.size();
}

string exev_to_string(int run,int event)
{
	ostringstream ostr;
	if( run==0 )
		ostr<<"sequential event "<<event;
	else if( run==1 )
		ostr<<"run event "<<event;
	else
		ostr<<"run "<<run<<" event "<<event;
	return ostr.str();
}

string exev_to_string(const eventlist_t& exev)
{
	if( exev.empty() ) return string();

	ostringstream ostr;
	eventlist_t::const_iterator cur=exev.begin(), last=exev.end();
	for(; cur!=last; cur++) {
		u_int64_t run=(*cur)/1000000ULL;
		u_int64_t event=(*cur)%1000000ULL;
		if( run==1 )
			ostr<<":";
		else if( run>1 )
			ostr<<run<<":";
		ostr<<event<<" ";
	}

	return ostr.str();
}

