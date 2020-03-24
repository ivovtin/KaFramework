#include <iostream>
#include <iomanip>
#include "kf_event_stat.h"

using namespace std;

long kf_cut_class::sum() const
{
	long s=0;
	for(const_iterator it=begin(); it!=end(); it++)
		s+=it->second.counter;
	return s;
}

kf_cut_stat& kf_event_stat::add_cut(int class_id,uint cut_id,const string& cut_name)
{
	kf_cut_stat& cut=(*this)[class_id][cut_id];
	cut=kf_cut_stat(cut_name);
	if( maxstrlen<cut_name.length() ) maxstrlen=cut_name.length();
	return cut;
}

kf_cut_class& kf_event_stat::add_class(int class_id,const string& class_name)
{
	pair<stats_type::iterator,bool> res=stats.insert(sval_type(class_id,kf_cut_class(class_name)));
	kf_cut_class &cut_class=(*res.first).second;
	if( !res.second ) //if the class exists already change its name
		cut_class.name=class_name;
    else //if added for the first time put it in the order list
		order.push_back(class_id);
	if( maxstrlen<class_name.length() ) maxstrlen=class_name.length();
	return cut_class;
}

kf_event_stat& kf_event_stat::operator+=(const kf_event_stat& r)
{
	total+=r.total;
	passed+=r.passed;
	written+=r.written;
	period+=r.period;

	stats_type::const_iterator rit=r.stats.begin(), r_last=r.stats.end();
	for( ; rit!=r_last; rit++) {
		pair<stats_type::iterator,bool> insres=stats.insert(sval_type(*rit));
		if( insres.second ) {
            order.push_back((*rit).first);
			continue;
		}
		kf_cut_class &lc=(*insres.first).second;
		kf_cut_class::const_iterator rcit=(*rit).second.begin(), rc_last=(*rit).second.end();
		for( ; rcit!=rc_last; rcit++) {
			pair<kf_cut_class::iterator,bool> insres2=lc.insert(kf_cut_class::value_type(*rcit));
			if( insres2.second ) continue;
			kf_cut_stat &ls=(*insres2.first).second;
			ls+=rcit->second;
		}
	}

	if( maxstrlen<r.maxstrlen ) maxstrlen=r.maxstrlen;

	return *this;
}

void kf_event_stat::reset()
{
	total=0;
	passed=0;
	period=0;
	written=0;

	for(stats_type::iterator it=stats.begin(); it!=stats.end(); it++)
		for(kf_cut_class::iterator it2=(it->second).begin(); it2!=(it->second).end(); it2++)
			(it2->second).reset();
}

void kf_event_stat::print() const                                               //print statistic
{
	static const char *sym_ordinary="\033[0m", *sym_head="\033[5m";

	ostream out(cout.rdbuf());

	out.precision(3);
	out.setf(ios::fixed);

	out <<left<<setw(27)<<"Total number of events:"<<right<<setw(7)<<total<<"\n"
		<<left<<setw(27)<<"Number of selected events:"<<right<<setw(7)<<passed<<" "<<100.*passed/total<<"%\n";
	if( written>0 )
		out <<left<<setw(27)<<"Number of written events:"<<right<<setw(7)<<written<<" "<<100.*written/total<<"%\n";
	if( period>0 )
		out <<left<<setw(27)<<"Average processing rate:"<<right<<setw(7)
			<<setprecision(1)<<total/period<<" Hz\n"<<setprecision(3);
	out <<"---\n";

	order_type::const_iterator ikey=order.begin();
	for( ; ikey!=order.end(); ikey++) {
		const kf_cut_class& cut_class=stats.find(*ikey)->second;
		if( cut_class.empty() ) continue; //no cuts defined for this class
		long sumstat=cut_class.sum();
		out <<sym_head<<left<<setw(maxstrlen+2)<<cut_class.name
			<<right<<setw(8)<<sumstat<<" "<<100.*sumstat/total<<"%\n"<<sym_ordinary;
		kf_cut_class::const_iterator icut=cut_class.begin();
		for( ; icut!=cut_class.end(); icut++) {
			const kf_cut_stat& cut=icut->second;
			if( cut.name.empty() && !cut.counter ) continue;
			out <<left<<" "<<setw(maxstrlen+1)<<cut.name
				<<right<<setw(8)<<cut.counter<<" "<<100.*cut.counter/total<<"%\n";
		}
	}
	out<<flush;
}
void kf_event_stat::sort(const order_type& ord)
{
	order_type::const_reverse_iterator irev=ord.rbegin();
	for( ; irev!=ord.rend(); irev++) {
		if( stats.find(*irev)==stats.end() ) {
			cerr<<"kf_event_stat::sort: No cut class with index "<<*irev<<" is declared"<<endl;
			continue;
		}
		order.remove(*irev);
		order.push_front(*irev);
	}
}
