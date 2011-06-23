#include <mycpp/mycpp.h>
#include <mycpp/io.h>

using namespace MyCpp;

class A : public SimplyReferenced
{
public:
    int key;

    A (int const key)
	: key (key)
    {
    }

    A ()
	: key (0)
    {
    }
};

int main (void)
{
    myCppInit ();

    bool success = true;

    {
	MultiMap<int> multi_map;

	{
	    MultiMap<int>::Entry orig_entry = multi_map.add (1);
	    MultiMap<int>::Entry found_entry = multi_map.lookupValue (1);

	    if (!MultiMap<int>::Entry::equal (orig_entry, found_entry)) {
		errf->print ("#1.1 failed").pendl ();
		success = false;
	    }

	    if (found_entry.isNull ()) {
		errf->print ("#1.2 failed").pendl ();
		success = false;
	    }

	    multi_map.remove (orig_entry);
	    multi_map.clear ();
	}

	{
	    MultiMap<int>::Entry orig_entry = multi_map.addFor ((unsigned char) 2);
	    orig_entry.getData () = 2;

	    MultiMap<int>::Entry found_entry = multi_map.lookup ((unsigned char) 2);

	    if (!MultiMap<int>::Entry::equal (orig_entry, found_entry)) {
		errf->print ("#1.3 failed").pendl ();
		success = false;
	    }

	    if (multi_map.isEmpty ()) {
		errf->print ("#1.4 failed").pendl ();
		success = false;
	    }
	}

	{
	    MultiMap<int>::Entry unused_entry;
	    if (!unused_entry.isNull ()) {
		errf->print ("#1.5 failed").pendl ();
		success = false;
	    }
	}
    }

    {
	typedef MultiMap< Ref<A>,
			  MemberExtractor< A,
					   int const,
					   &A::key >,
			  DirectComparator<int> >
		TestMap_type;

	TestMap_type multi_map;

	{
	    TestMap_type::Entry orig_entry = multi_map.add (grab (new A (1)));
	    TestMap_type::Entry found_entry = multi_map.lookupValue (grab (new A (1)));

	    if (!TestMap_type::Entry::equal (orig_entry, found_entry)) {
		errf->print ("#2.1 failed").pendl ();
		success = false;
	    }

	    if (found_entry.isNull ()) {
		errf->print ("#2.2 failed").pendl ();
		success = false;
	    }

	    multi_map.remove (orig_entry);
	    multi_map.clear ();
	}

	{
	    TestMap_type::Entry orig_entry = multi_map.addFor ((unsigned char) 2);
	    orig_entry.getData () = grab (new A(2));

	    TestMap_type::Entry found_entry = multi_map.lookup ((unsigned char) 2);

	    if (!TestMap_type::Entry::equal (orig_entry, found_entry)) {
		errf->print ("#2.3 failed").pendl ();
		success = false;
	    }

	    if (multi_map.isEmpty ()) {
		errf->print ("#2.4 failed").pendl ();
		success = false;
	    }
	}

	{
	    TestMap_type::Entry unused_entry;
	    if (!unused_entry.isNull ()) {
		errf->print ("#2.5 failed").pendl ();
		success = false;
	    }
	}
    }

    {
	typedef MultiMap< A,
			  MemberExtractor< A const,
					   int const,
					   &A::key >,
			  DirectComparator<int> >
		TestMap_type;

	TestMap_type multi_map;

	{
	    TestMap_type::Entry orig_entry = multi_map.add (1);
	    TestMap_type::Entry found_entry = multi_map.lookupValue (1);

	    if (!TestMap_type::Entry::equal (orig_entry, found_entry)) {
		errf->print ("#3.1 failed").pendl ();
		success = false;
	    }

	    if (found_entry.isNull ()) {
		errf->print ("#3.2 failed").pendl ();
		success = false;
	    }

	    multi_map.remove (orig_entry);
	    multi_map.clear ();
	}

	{
	    TestMap_type::Entry orig_entry = multi_map.addFor ((unsigned char) 2);
	    orig_entry.getData () = 2;

	    TestMap_type::Entry found_entry = multi_map.lookup ((unsigned char) 2);

	    if (!TestMap_type::Entry::equal (orig_entry, found_entry)) {
		errf->print ("#3.3 failed").pendl ();
		success = false;
	    }

	    if (multi_map.isEmpty ()) {
		errf->print ("#3.4 failed").pendl ();
		success = false;
	    }
	}

	{
	    TestMap_type::Entry unused_entry;
	    if (!unused_entry.isNull ()) {
		errf->print ("#3.5 failed").pendl ();
		success = false;
	    }
	}

	{
	    multi_map.clear ();

	    multi_map.add (1);
	    multi_map.add (2);
	    multi_map.add (2);
	    multi_map.add (2);
	    multi_map.add (3);

	    {
		size_t i = 0;

		TestMap_type::Iterator iter (multi_map);
		while (!iter.done ()) {
		    TestMap_type::Entry entry = iter.next ();
		    if (entry.isNull ()) {
			errf->print ("#4.1 failed").pendl ();
			success = false;
			break;
		    }

		    i++;
		}

		if (i != 5) {
		    errf->print ("#4.2 failed").pendl ();
		    success = false;
		}
	    }

	    {
		size_t i = 0;

		TestMap_type::DataIterator iter (multi_map);
		while (!iter.done ()) {
		    A &a = iter.next ();
		    (void) a;

		    i++;
		}

		if (i != 5) {
		    errf->print ("#4.3 failed").pendl ();
		    success = false;
		}
	    }

	    {
		size_t i = 0;

		TestMap_type::SameKeyIterator iter = multi_map.lookup (2);
		while (!iter.done ()) {
		    TestMap_type::Entry entry = iter.next ();
		    if (entry.isNull ()) {
			errf->print ("#4.4 failed").pendl ();
			success = false;
			break;
		    }

		    i ++;
		}

		if (i != 3) {
		    errf->print ("#4.5 failed").pendl ();
		    success = false;
		}
	    }

	    {
		size_t i = 0;

		TestMap_type::SameKeyDataIterator iter = multi_map.lookup (2);
		while (!iter.done ()) {
		    A &a = iter.next ();
		    if (a.key != 2) {
			errf->print ("#4.6 failed").pendl ();
			success = false;
			break;
		    }

		    i ++;
		}

		if (i != 3) {
		    errf->print ("#4.7 failed").pendl ();
		    success = false;
		}
	    }
	}
    }

    {
	typedef MultiMap< Ref<String>,
			  AccessorExtractor< String,
					     MemoryDesc,
					     &String::getMemoryDesc >,
			  MemoryComparator<> >
		TestMap_type;

	TestMap_type multi_map;

	TestMap_type::Entry entry = multi_map.add (String::forData ("123"));
	multi_map.addFor (ConstMemoryDesc::forString ("123")).getData () =
		String::forData ("123");

	entry = multi_map.add (String::forData ("DEF"));
	entry = multi_map.add (String::forData ("ABC"));
	entry = multi_map.lookup ("123");
	if (entry.isNull ()) {
	    errf->print ("#5.1 failed").pendl ();
	    success = false;
	}

	multi_map.remove (entry);
	multi_map.clear ();
    }

    if (success)
	errf->print ("SUCCESS").pendl ();
    else
	errf->print ("FAILURE").pendl ();

    return success ? EXIT_SUCCESS : EXIT_FAILURE;
}

