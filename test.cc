#include <iostream>
#include "mknatfilelist.cc"
using namespace std;

int main(int argc, char* argv[])
{
	int run=argc>1?atoi(argv[1]):11694;
	string natfile;

	bool found=check_runfile(run,natfile);

	cout<<"file for run "<<run;
    if( found )
		cout<<": "<<natfile<<endl;
	else
		cout<<" not found"<<endl;

	return 0;
}
