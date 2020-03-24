#ifndef MKFILELIST_H
# define MKFILELIST_H

# ifdef __cplusplus
#  include <string>
#  include <map>
#  include <set>
#  include <sys/types.h>

typedef std::set<u_int64_t> eventlist_t;

typedef std::map<std::string, eventlist_t> flist_exev_t;

extern int mknatfilelist(int nargs,char* args[], flist_exev_t& filelist,bool verbose);

extern int mknatfilelist(int nruns,int runs[], flist_exev_t& filelist,bool verbose);

extern std::string exev_to_string(const eventlist_t&);
extern std::string exev_to_string(int run,int event);

# endif

# ifdef __cplusplus
extern "C" {
# endif
extern void mknatfl_setdir(const char* dir);

extern void mknatfl_setpatt(const char* patt);
# ifdef __cplusplus
}
# endif

#endif
