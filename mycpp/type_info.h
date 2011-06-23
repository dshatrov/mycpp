#ifndef __MYCPP__TYPE_INFO_H__
#define __MYCPP__TYPE_INFO_H__

#include <mycpp/atomic_pointer.h>

namespace MyCpp {

class TypeInfo
{
public:
    class TypeInfoEntry
    {
	friend class TypeInfo;

    private:
	// (no data fields)
    };

private:
    AtomicPointer type_entry;

protected:
    void initTypeInfo (TypeInfoEntry *entry);

public:
    bool sameType (TypeInfoEntry const *entry);

    bool sameType (TypeInfo const *type_info);
};

}

#endif /* __MYCPP__TYPE_INFO_H__ */

