#ifndef KF_EVENT_STAT_H
# define KF_EVENT_STAT_H

# include <iostream>
# include <string>
# include <map>
# include <list>
# include <sys/types.h>

//Cut description and number of applying
class kf_cut_stat {
public:
	std::string name;
	long counter;

	kf_cut_stat& operator++(int) { counter++; return *this; }
	kf_cut_stat& operator+=(const kf_cut_stat& oth) { counter+=oth.counter; return *this; } //sum method
	void reset() { counter=0; }

	kf_cut_stat() : name(), counter(0) {}
	explicit kf_cut_stat(const std::string &nm) : name(nm), counter(0) {}
};

//Class of cuts
class kf_cut_class : public std::map<uint,class kf_cut_stat>
{
public:
	std::string name;
	explicit kf_cut_class(const std::string& _name="Unknown") : name(_name) {}
	long sum() const;
};

//Statistics of events
class kf_event_stat
{
public:
	typedef std::map<int,class kf_cut_class> stats_type;
	typedef std::pair<int,class kf_cut_class> sval_type;
	typedef std::list<int> order_type;

	long total, passed, written; //event counters
private:
	stats_type stats;
	order_type order; //for specific order of classes
	size_t maxstrlen; //for nicer printing
    double period; //CPU time spent for event processing

public:

	kf_event_stat() : total(0),passed(0),written(0),stats(),order(),maxstrlen(0),period(0) {} //default constructor

	kf_cut_stat& add_cut(int class_id,uint cut_id,const std::string& cut_name);
	kf_cut_class& add_class(int class_id,const std::string& class_name);
	kf_cut_class* get_class(int class_id)
	{
		stats_type::iterator it=stats.find(class_id);
		return it!=stats.end() ? &(*it).second : NULL;
	}

	kf_event_stat& operator+=(const kf_event_stat&); //sum method
	kf_cut_class&  operator[](int class_id) //get element method
	{
		kf_cut_class *c=get_class(class_id);
		if( !c ) c=&add_class(class_id,"Unknown");
		return *c;
	}

	void reset(); //reset counters
	void print() const; //print statistics

	void sort(const order_type& ord); //sort cut classes according to given order
	void set_period(double t) { period=t; }
};

#endif
