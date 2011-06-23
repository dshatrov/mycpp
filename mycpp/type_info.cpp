#include <mycpp/util.h>
#include <mycpp/io.h>

#include <mycpp/type_info.h>

namespace MyCpp {

void
TypeInfo::initTypeInfo (TypeInfoEntry *entry)
{
    abortIf (entry == NULL);

    if (type_entry.get () == entry) {
	errf->print ("MyCpp.TypeInfo.initTypeInfo: warning: initTypeInfo() has been called "
		     "moore than once for the same TypeInfoEntry")
	     .pendl ();
    }

    type_entry.set (static_cast <void*> (entry));
}

bool
TypeInfo::sameType (TypeInfoEntry const *entry)
{
    abortIf (entry == NULL);

    TypeInfoEntry *ptr1 = static_cast <TypeInfoEntry*> (type_entry.get ());

    if (ptr1 == NULL)
	abortIfReached_str ("initTypeInfo() has not been called");

    return ptr1 == entry;
}

bool
TypeInfo::sameType (TypeInfo const *type_info)
{
    TypeInfoEntry *ptr1 = static_cast <TypeInfoEntry*> (type_info->type_entry.get ());
    TypeInfoEntry *ptr2 = static_cast <TypeInfoEntry*> (type_entry.get ());

    if (ptr1 == NULL || ptr2 == NULL)
	abortIfReached_str ("initTypeInfo() has not been called");

    return ptr2 == ptr2;
}

}

