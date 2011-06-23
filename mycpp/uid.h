#ifndef __MYCPP__UID_H__
#define __MYCPP__UID_H__

#include <mycpp/types.h>
#include <mycpp/base_util.h>

#include <mycpp/id_mapper.h>


//#define MYCPP__ROBUST_SLOW_UID


namespace MyCpp {

/** .cat uid */

/*t*/
typedef Uint64 UidType;

/*c*/
class UidMapper
{
private:
    typedef IdMapper<void*> IdMapperType;

public:
    /*c*/
    class UidEntry {
	friend class UidMapper;

    private:
	UidType uid;
	IdMapperType::Node *node;

	UidEntry (UidType const uid,
		  IdMapperType::Node * const node)
	    : uid (uid),
	      node (node)
	{
	}

    public:
	/*m*/
	UidType getUid () const
	{
	    return uid;
	}

	/*m*/
	UidEntry ()
	{
	}
    };

private:
    IdMapperType id_mapper;

    // Non-copyable
    UidMapper (UidMapper const &);
    UidMapper& operator = (UidMapper const &);

public:
    /*m*/
    UidEntry grabUid ()
    {
	IdMapperType::Node * const node = id_mapper.add (NULL);
	abortIf (node == NULL);
	return UidEntry (node->value, node);
    }

    /*m*/
    void releaseUid (UidEntry const uid_entry)
    {
	id_mapper.remove (uid_entry.node);
    }

    /*m*/
    static UidMapper& getGlobUidMapper ();

    /*m*/
    UidMapper ()
    {
    }
};

#ifdef MYCPP__ROBUST_SLOW_UID
// Slower variant without implementation-defined pointer casts.
/*c*/
class UidBinding
{
private:
    UidMapper &uid_mapper;

    Bool mutable got_uid;
    UidMapper::UidEntry mutable uid_entry;

    // Non-copyable
    UidBinding (UidBinding const &);
    UidBinding& operator = (UidBinding const &);

public:
    /*m*/
    UidType getUid () const
    {
	if (!got_uid) {
	    uid_entry = uid_mapper.grabUid ();
	    got_uid = true;
	}

	return uid_entry.getUid ();
    }

    /*m*/
    UidBinding (UidMapper &uid_mapper)
	: uid_mapper (uid_mapper)
    {
    }

    ~UidBinding ()
    {
	if (got_uid)
	    uid_mapper.releaseUid (uid_entry);
    }
};
#else
// Faster variant, uses pointer casts.
class UidBinding
{
public:
    UidType getUid () const
    {
	return (UidType) this;
    }
};
#endif // MYCPP__ROBUST_SLOW_UID

/*c Base class for classes providing uids.
 */
class UidProvider
{
private:
#ifdef MYCPP__ROBUST_SLOW_UID
    UidBinding const uid_binding;
#else
    UidBinding uid_binding;
#endif

public:
    /*m*/
    UidType getUid () const
    {
#ifdef MYCPP__ROBUST_SLOW_UID
	return uid_binding.getUid ();
#else
// Assuming that returning 'this' is faster and does no harm.
//	return (UidType) &uid_binding;
	return (UidType) this;
#endif
    }

    /*m*/
    UidProvider (UidMapper &uid_mapper = UidMapper::getGlobUidMapper ())
#ifdef MYCPP__ROBUST_SLOW_UID
	: uid_binding (uid_mapper)
#endif
    {
	(void) uid_mapper;
    }
};

}

#endif /* __MYCPP__UID_H__ */

