#ifndef __MYCPP__MEM_UTIL_H__
#define __MYCPP__MEM_UTIL_H__


#include <libmary/util_dev.h>

#include <mycpp/memory_desc.h>


namespace MyCpp {

static inline void
copyMemory (MemoryDesc const &to,
	    ConstMemoryDesc const &from)
{
//    Size tocopy = to.getLength() >= from.getLength() ? to.getLength() : from.getLength();
    assert (to.getLength() >= from.getLength());
    memcpy (to.getMemory(), from.getMemory(), from.getLength());
}

static inline void
copyMemory (MemoryDesc const &to,
	    ConstMemoryDesc const &from,
	    Size len)
{
    copyMemory (to, from.getRegion (0, len));
}

static inline void
zeroMemory (MemoryDesc const &mem)
{
    memset (mem.getMemory(), 0, mem.getLength());
}

// TODO Redundant with MemoryDesc::forObject()
template <class T>
void zeroMemory (T &obj)
{
    zeroMemory (MemoryDesc ((Byte*) &obj, sizeof obj));
}

//char* rawCollectBacktrace ();
using M::rawCollectBacktrace;

}


#endif /* __MYCPP__MEM_UTIL_H__ */

