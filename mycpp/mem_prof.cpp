#include <cstdio>

#include <mycpp/types.h>
#include <mycpp/mutex.h>
#include <mycpp/util.h>

#include <mycpp/mem_prof.h>

namespace MyCpp {

namespace MemProf_priv {}
using namespace MemProf_priv;

namespace MemProf_priv {

    Ref< Mutex_<SimplyReferenced> > mem_prof_mutex;

    class MemProfEntry_Comparator__CompData
    {
    public:
	const char    *source_file;
	unsigned long  source_line;
	const char    *backtrace;

	MemProfEntry_Comparator__CompData (const char    *source_file,
					   unsigned long  source_line,
					   const char    *backtrace)
	{
	    this->source_file = source_file;
	    this->source_line = source_line;
	    this->backtrace   = backtrace;
	}
    };

    class MemProfEntry_Comparator
    {
    public:
	static bool greater (ConstPointer<MemProfEntry> const &left,
			     MemProfEntry_Comparator__CompData const &right)
	{
	    abortIf (left.isNull ());

	    ComparisonResult res = orderStrings (left->source_file, right.source_file);
	    if (res != ComparisonEqual)
		return res == ComparisonGreater;

	    if (left->source_line != right.source_line)
		return left->source_line > right.source_line;

	    return orderStrings (left->backtrace, right.backtrace) == ComparisonGreater;
	}

	static bool equals (ConstPointer<MemProfEntry> const &left,
			    MemProfEntry_Comparator__CompData const &right)
	{
	    abortIf (left.isNull ());

	    return (orderStrings (left->source_file, right.source_file) == ComparisonEqual) &&
		   (left->source_line == right.source_line) &&
		   (orderStrings (left->backtrace, right.backtrace) == ComparisonEqual);
	}
    };

    static Map < MemProfEntry,
		 DirectExtractor<MemProfEntry&>,
		 MemProfEntry_Comparator >
	    mem_prof_map;

}

MemProfLink
addMemProfEntry (void          *ptr,
		 const char    *source_file,
		 unsigned long  source_line,
		 const char    *type_name,
		 const char    *backtrace)
{
    mem_prof_mutex->lock ();

    MemProfEntry_Comparator__CompData comp_data (source_file, source_line, NULL /* backtrace */);
    MapBase<MemProfEntry>::Entry map_entry = mem_prof_map.lookup (comp_data);
    if (map_entry.isNull ()) {
	map_entry = mem_prof_map.addFor (comp_data);
	abortIf (map_entry.isNull ());
	MemProfEntry &entry = map_entry.getData ();

	entry.ptr = ptr;
	entry.source_file.set (source_file);
	entry.source_line = source_line;
	entry.type_name.set (type_name);
//	entry.backtrace.set (backtrace);

	entry.count = 1;
    } else {
	MemProfEntry &entry = map_entry.getData ();
	entry.count ++;
    }

    mem_prof_mutex->unlock ();

    return MemProfLink (map_entry);
}

void
removeMemProfEntry (MemProfLink const &link)
{
    if (!link.map_entry.isNull ()) {
	MemProfEntry &entry = link.map_entry.getData ();
	abortIf (entry.count < 1);
	entry.count --;
	if (entry.count == 0)
	    mem_prof_map.remove (link.map_entry);
    }
}

void
dumpMemProf ()
{
    fprintf (stderr, "MyCpp.dumpMemProf:\n");

    MapBase<MemProfEntry>::Iterator map_iter = mem_prof_map.createIterator ();
    while (!map_iter.done ()) {
	MemProfEntry const &entry = map_iter.next ().getData ();

	libraryLock ();
	fprintf (stderr, "%lu %s:%lu %s\n",
		 entry.count,
		 (const char*) entry.source_file,
		 entry.source_line,
		 (const char*) entry.type_name);
//	fprintf (stderr, "---> %s\n", (const char*) entry.backtrace);
	libraryUnlock ();
    }

    libraryLock ();
    fflush (stderr);
    libraryUnlock ();
}

void
memProfInit ()
{
    mem_prof_mutex = new Mutex_<SimplyReferenced>;
    mem_prof_mutex->unref (); // instead of grab()
}

}

