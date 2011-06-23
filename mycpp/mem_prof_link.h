#ifndef __MYCPP__MEM_PROF_LINK_H__
#define __MYCPP__MEM_PROF_LINK_H__

#include <mycpp/mycpp_config.h>
#include <mycpp/types.h>
#include <mycpp/map.h>

namespace MyCpp {

class MemProfEntry;

class MemProfLink
{
public:
    MapBase<MemProfEntry>::Entry map_entry;

    bool isNull ()
    {
	return map_entry.isNull ();
    }

    MemProfLink (MapBase<MemProfEntry>::Entry const &map_entry)
    {
	this->map_entry = map_entry;
    }

    MemProfLink ()
    {
    }
};

MemProfLink addMemProfEntry (void          *ptr,
			     const char    *source_file,
			     unsigned long  source_line,
			     const char    *type_name,
			     const char    *backtrace);

void removeMemProfEntry (MemProfLink const &link);

void dumpMemProf ();

void memProfInit ();

}

#endif /* __MYCPP__MEM_PROF_LINK_H__ */

