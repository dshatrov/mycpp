#include <mycpp/mycpp.h>

using namespace MyCpp;

class MyClass : public virtual SimplyReferenced
{
public:
    int key;

    MyClass (int key)
    {
	this->key = key;
    }
};

int main (void)
{
    myCppInit ();

    {
	Map<int> map;
	Map<int>::Entry entry = map.add (10);
	entry = map.lookup (10);
	abortIf (entry.isNull ());
	map.remove (entry);
	map.clear ();
    }

    {
	Map< Ref<String>,
	     AccessorExtractor< String,
				MemoryDesc,
			       	&String::getMemoryDesc >,
	     MemoryComparator<> >
		map;
//	Map< Ref<String> >::Entry entry = map.add (String::forData ("123"));
	Map< Ref<String> >::Entry entry = map.addFor (ConstMemoryDesc::forString ("123"));
#if 0
	entry = map.add (String::forData ("ABC"));
	entry = map.lookup (ConstMemoryDesc::"123");
	abortIf (entry.isNull ());
	map.remove (entry);
	map.clear ();
#endif
    }

    {
#if 0
	Map< Ref<MyClass>, RefMemberComparator< MyClass, int, &MyClass::key > > map;
	map.add (grab (new MyClass (1)));
	MapBase< Ref<MyClass> >::Entry entry = map.lookup (1);
	abortIf (entry.isNull ());
	map.remove (entry);
	map.clear ();
#endif
    }

    return 0;
}

