#ifndef __MYCPP__MEM_PROF_H__
#define __MYCPP__MEM_PROF_H__

#include <mycpp/mycpp_config.h>
#include <mycpp/string.h>
#include <mycpp/mem_prof_link.h>

namespace MyCpp {

class MemProfEntry
{
public:
    void          *ptr;
    String         source_file;
    unsigned long  source_line;
    String         type_name;
    String         backtrace;

    unsigned long  count;
};

}

#endif /* __MYCPP__MEM_PROF_H__ */

